import sys, struct, re, bisect
from collections import defaultdict
from elftools.elf.elffile import ELFFile

ROOT = "/Users/zcanann/Documents/Projects/SFA-Decomp"
SPLITS = ROOT + "/config/GSAE01/splits.txt"
SYMBOLS = ROOT + "/config/GSAE01/symbols.txt"
DOL = ROOT + "/orig/GSAE01/sys/main.dol"

def parse_splits():
    units = {}
    cur = None
    for line in open(SPLITS):
        m = re.match(r'^(\S+\.c):', line)
        if m:
            cur = m.group(1); units[cur] = {}
            continue
        m = re.match(r'^\s+(\.\w+|\w+)\s+start:(0x[0-9A-Fa-f]+)\s+end:(0x[0-9A-Fa-f]+)', line)
        if m and cur:
            units[cur][m.group(1)] = (int(m.group(2),16), int(m.group(3),16))
    return units

def parse_symbols():
    name2addr = {}
    addr2name = {}
    for line in open(SYMBOLS, errors='replace'):
        m = re.match(r'^(\S+)\s*=\s*(\S+):(0x[0-9A-Fa-f]+);(.*)$', line)
        if not m: continue
        name, sect, addr = m.group(1), m.group(2), int(m.group(3),16)
        sz = re.search(r'size:(0x[0-9A-Fa-f]+)', m.group(4))
        name2addr[name] = (sect, addr, int(sz.group(1),16) if sz else None)
        addr2name.setdefault(addr, name)
    return name2addr, addr2name

class Dol:
    def __init__(self, path):
        d = open(path,'rb').read()
        self.d = d
        offs = struct.unpack('>18I', d[0:0x48])
        addrs = struct.unpack('>18I', d[0x48:0x90])
        sizes = struct.unpack('>18I', d[0x90:0xd8])
        self.secs = [(addrs[i], sizes[i], offs[i]) for i in range(18) if sizes[i]]
    def read(self, addr, n):
        for a,s,o in self.secs:
            if a <= addr and addr+n <= a+s:
                return self.d[o+addr-a:o+addr-a+n]
        return None

def global_section_ranges(units):
    r = defaultdict(lambda: [0xffffffff, 0])
    for u in units.values():
        for s,(a,b) in u.items():
            r[s][0] = min(r[s][0], a); r[s][1] = max(r[s][1], b)
    return {s:(a,b) for s,(a,b) in r.items()}

def audit(unit_key, objpath, units, name2addr, gsr, dol):
    claims = units[unit_key]
    f = open(objpath,'rb')
    elf = ELFFile(f)
    symtab = elf.get_section_by_name('.symtab')
    secnames = {i: elf.get_section(i).name for i in range(elf.num_sections())}
    syms = list(symtab.iter_symbols())
    text_start = claims['.text'][0]
    # function table: STT_FUNC symbols in .text
    funcs = []
    for s in syms:
        if s['st_info']['type']=='STT_FUNC' and s['st_shndx'] not in ('SHN_UNDEF','SHN_ABS'):
            if secnames.get(s['st_shndx'])=='.text':
                funcs.append((s['st_value'], s['st_size'], s.name))
    funcs.sort()
    fstarts = [f0 for f0,_,_ in funcs]
    def off2func(off):
        i = bisect.bisect_right(fstarts, off)-1
        if i<0: return '?'
        return funcs[i][2]
    def classify(addr):
        for s,(a,b) in gsr.items():
            if a<=addr<b: return s
        return '?'
    # relocs
    rela = elf.get_section_by_name('.rela.text')
    perfunc = defaultdict(lambda: defaultdict(set))  # func -> sect -> set(addr)
    unknown = []
    for r in rela.iter_relocations():
        sym = syms[r['r_info_sym']]
        rtype = r['r_info_type']
        if sym['st_shndx'] not in ('SHN_UNDEF','SHN_ABS'):
            sect = secnames.get(sym['st_shndx'])
            if sect not in claims:
                continue
            addr = claims[sect][0] + sym['st_value'] + r['r_addend']
        else:
            info = name2addr.get(sym.name)
            if info is None:
                m = re.match(r'lbl_(80[0-9A-Fa-f]{6})$', sym.name)
                if m: addr = int(m.group(1),16) + r['r_addend']
                else:
                    unknown.append(sym.name); continue
            else:
                addr = info[1] + r['r_addend']
        sect = classify(addr)
        fn = off2func(r['r_offset'])
        perfunc[fn][sect].add(addr)
    return funcs, perfunc, unknown

def fmt_val(b):
    if b is None: return '????'
    u = struct.unpack('>I', b)[0]
    fv = struct.unpack('>f', b)[0]
    return f"{u:08x} ({fv!r})"

if __name__ == '__main__':
    unit_key = sys.argv[1]
    objpath = sys.argv[2]
    units = parse_splits()
    name2addr, addr2name = parse_symbols()
    gsr = global_section_ranges(units)
    dol = Dol(DOL)
    funcs, perfunc, unknown = audit(unit_key, objpath, units, name2addr, gsr, dol)
    text_start = units[unit_key]['.text'][0]
    print(f"== {unit_key} .text 0x{units[unit_key]['.text'][0]:08x}-0x{units[unit_key]['.text'][1]:08x}")
    print(f"global .sdata2 range: {gsr.get('.sdata2')}")
    if unknown: print("UNKNOWN SYMS:", sorted(set(unknown))[:20])
    # per-function pool refs in text order
    all_pool = set()
    for off, sz, name in funcs:
        refs = perfunc.get(name, {})
        pool = sorted(refs.get('.sdata2', set()))
        all_pool.update(pool)
        if pool:
            print(f"0x{text_start+off:08x} {name:44s} sdata2: {' '.join(f'{a:08x}' for a in pool)}")
    print("\n-- pool words (addr value):")
    vals = defaultdict(list)
    for a in sorted(all_pool):
        b = dol.read(a,4)
        print(f"  {a:08x}  {fmt_val(b)}")
        if b: vals[b].append(a)
    print("\n-- duplicated values at distinct addresses:")
    for b, addrs in sorted(vals.items(), key=lambda kv: kv[1]):
        if len(addrs)>1:
            print(f"  {fmt_val(b):30s} at {' '.join(f'{a:08x}' for a in addrs)}")
