#!/usr/bin/env python3
"""A real C-expression tokenizer and parser, with source spans.

Every brute-force lane that rewrites source text has so far done it with
regular expressions over the raw characters.  A regex cannot see precedence, so
it cannot tell `verts + j * 12` (an offset added to a scaled index) from
`(j + verts) * 12` (a completely different computation) -- and because the
retail binary is compared BYTE-wise, a wrong computation whose bytes happen to
land closer scores as a win.  Nothing downstream can catch that: the score, the
object comparison and the forced link all measure agreement with retail's
bytes, never with retail's meaning.

This module is the parsing half of the fix.  semantic_equivalence.py is the
proving half.  Together they are the gate a rewrite must pass BEFORE it is
compiled, so that a rewrite which changes the arithmetic is rejected on its
meaning rather than rewarded for its score.

Nodes carry (start, end) offsets into the ORIGINAL text, so a caller can splice
a rewritten sub-expression back into a file while leaving every other byte --
including whitespace and comments -- exactly as it was.
"""
from __future__ import annotations

import re
from dataclasses import dataclass, field
from typing import List, Optional, Sequence, Tuple

# --------------------------------------------------------------------- errors


class ParseError(Exception):
    """The text is not an expression this parser can read."""


class Ambiguous(ParseError):
    """`(x) * y` -- a cast or a multiply?  Undecidable without a symbol table."""


# ------------------------------------------------------------------ tokenizer

PUNCT = [
    "<<=", ">>=", "...",
    "->", "++", "--", "<<", ">>", "<=", ">=", "==", "!=", "&&", "||",
    "+=", "-=", "*=", "/=", "%=", "&=", "|=", "^=",
    "(", ")", "[", "]", "{", "}", ".", ",", ";", "?", ":",
    "+", "-", "*", "/", "%", "&", "|", "^", "~", "!", "<", ">", "=",
]

_NUM = re.compile(
    r"(?:0[xX][0-9a-fA-F]+|(?:\d+\.\d*|\.\d+|\d+)(?:[eE][-+]?\d+)?)"
    r"[uUlLfF]*")
_IDENT = re.compile(r"[A-Za-z_]\w*")


@dataclass
class Tok:
    kind: str          # id | num | str | chr | punct | eof
    text: str
    start: int
    end: int


def tokenize(s: str, lo: int = 0, hi: Optional[int] = None) -> List[Tok]:
    if hi is None:
        hi = len(s)
    out: List[Tok] = []
    i = lo
    while i < hi:
        c = s[i]
        if c in " \t\r\n\\":
            i += 1
            continue
        if s.startswith("//", i):
            j = s.find("\n", i)
            i = hi if j < 0 or j > hi else j
            continue
        if s.startswith("/*", i):
            j = s.find("*/", i)
            if j < 0:
                raise ParseError("unterminated comment")
            i = j + 2
            continue
        if c in "\"'":
            j = i + 1
            while j < hi:
                if s[j] == "\\":
                    j += 2
                    continue
                if s[j] == c:
                    j += 1
                    break
                j += 1
            else:
                raise ParseError("unterminated literal")
            out.append(Tok("str" if c == '"' else "chr", s[i:j], i, j))
            i = j
            continue
        m = _NUM.match(s, i)
        if m and (c.isdigit() or (c == "." and i + 1 < hi and s[i + 1].isdigit())):
            out.append(Tok("num", m.group(0), i, m.end()))
            i = m.end()
            continue
        m = _IDENT.match(s, i)
        if m:
            out.append(Tok("id", m.group(0), i, m.end()))
            i = m.end()
            continue
        for p in PUNCT:
            if s.startswith(p, i):
                out.append(Tok("punct", p, i, i + len(p)))
                i += len(p)
                break
        else:
            raise ParseError(f"stray character {c!r} at {i}")
    out.append(Tok("eof", "", hi, hi))
    return out


# ----------------------------------------------------------------------- AST


@dataclass
class Node:
    kind: str
    start: int = 0
    end: int = 0
    op: str = ""
    text: str = ""                       # id / num / str / chr / cast type
    kids: List["Node"] = field(default_factory=list)

    def src(self, s: str) -> str:
        return s[self.start:self.end]

    def walk(self):
        yield self
        for k in self.kids:
            yield from k.walk()


def _n(kind, start, end, op="", text="", kids=()) -> Node:
    return Node(kind, start, end, op, text, list(kids))


# -------------------------------------------------------------------- parser

BIN_PREC = {
    "*": 13, "/": 13, "%": 13,
    "+": 12, "-": 12,
    "<<": 11, ">>": 11,
    "<": 10, ">": 10, "<=": 10, ">=": 10,
    "==": 9, "!=": 9,
    "&": 8,
    "^": 7,
    "|": 6,
    "&&": 5,
    "||": 4,
}
ASSIGN_OPS = {"=", "+=", "-=", "*=", "/=", "%=", "&=", "|=", "^=", "<<=", ">>="}

BASE_TYPE_WORDS = {
    "void", "char", "short", "int", "long", "float", "double", "signed",
    "unsigned", "const", "volatile", "struct", "union", "enum",
    "u8", "u16", "u32", "u64", "s8", "s16", "s32", "s64", "f32", "f64",
    "bool", "BOOL", "size_t", "u128", "f80",
}
_TYPEISH = re.compile(r"^(?:[usf]\d+|.*_t|[A-Z]\w*)$")


class Parser:
    """Precedence-climbing parser for the C expression grammar.

    `known_ids` is the set of names the caller knows to be OBJECTS (locals,
    parameters, globals).  It is what disambiguates `(x) * y`: if x is a known
    object the parens are a grouping, if x is type-shaped it is a cast, and if
    neither can be established the parse is refused rather than guessed.
    """

    def __init__(self, s: str, toks: Sequence[Tok], known_ids=frozenset(),
                 known_types=frozenset()):
        self.s = s
        self.t = list(toks)
        self.i = 0
        self.known_ids = set(known_ids)
        self.known_types = set(known_types)

    # -- token helpers
    @property
    def cur(self) -> Tok:
        return self.t[self.i]

    def at(self, text: str) -> bool:
        c = self.cur
        return c.kind == "punct" and c.text == text

    def eat(self, text: str) -> Tok:
        if not self.at(text):
            raise ParseError(f"expected {text!r} got {self.cur.text!r}")
        tok = self.cur
        self.i += 1
        return tok

    # -- type-name recognition
    def _is_type_name(self, j: int) -> bool:
        """Do the tokens starting at j spell a type-name followed by `)`?"""
        k = j
        saw = False
        while self.t[k].kind == "id" and (
                self.t[k].text in BASE_TYPE_WORDS or
                (not saw and self.t[k].text not in self.known_ids and
                 (self.t[k].text in self.known_types or
                  _TYPEISH.match(self.t[k].text)))):
            saw = True
            k += 1
        if not saw:
            return False
        while self.t[k].kind == "punct" and self.t[k].text == "*":
            k += 1
        if self.t[k].kind == "punct" and self.t[k].text == "[":
            return False
        # a function-pointer type name: `void (*)(f32, f32)`, `int (*)(int)`
        if self.t[k].kind == "punct" and self.t[k].text == "(":
            k = self._skip_group(k)
            if k < 0:
                return False
            if self.t[k].kind == "punct" and self.t[k].text == "(":
                k = self._skip_group(k)
                if k < 0:
                    return False
        return self.t[k].kind == "punct" and self.t[k].text == ")"

    def _skip_group(self, k: int) -> int:
        """Index just past the `)` matching the `(` at k, or -1."""
        depth = 0
        while k < len(self.t):
            tk = self.t[k]
            if tk.kind == "punct" and tk.text == "(":
                depth += 1
            elif tk.kind == "punct" and tk.text == ")":
                depth -= 1
                if depth == 0:
                    return k + 1
            elif tk.kind == "eof":
                return -1
            k += 1
        return -1

    def _cast_is_ambiguous(self, j: int) -> bool:
        """`(NAME)` where NAME is neither a known object nor type-shaped."""
        if self.t[j].kind != "id":
            return False
        if not (self.t[j + 1].kind == "punct" and self.t[j + 1].text == ")"):
            return False
        nm = self.t[j].text
        if nm in BASE_TYPE_WORDS or nm in self.known_types or nm in self.known_ids:
            return False
        if not _TYPEISH.match(nm):
            return False
        # type-shaped but never declared as either: only ambiguous when what
        # follows could begin a unary-expression.
        nxt = self.t[j + 2]
        return (nxt.kind in ("id", "num") or
                (nxt.kind == "punct" and nxt.text in ("(", "*", "&", "-", "~", "!", "+")))

    # -- grammar
    def parse(self) -> Node:
        n = self.expr()
        if self.cur.kind != "eof":
            raise ParseError(f"trailing {self.cur.text!r}")
        return n

    def expr(self) -> Node:
        n = self.assign()
        while self.at(","):
            self.i += 1
            r = self.assign()
            n = _n("comma", n.start, r.end, ",", kids=(n, r))
        return n

    def assign(self) -> Node:
        n = self.cond()
        c = self.cur
        if c.kind == "punct" and c.text in ASSIGN_OPS:
            self.i += 1
            r = self.assign()
            return _n("assign", n.start, r.end, c.text, kids=(n, r))
        return n

    def cond(self) -> Node:
        n = self.binary(4)
        if self.at("?"):
            self.i += 1
            a = self.expr()
            self.eat(":")
            b = self.cond()
            return _n("cond", n.start, b.end, "?:", kids=(n, a, b))
        return n

    def binary(self, min_prec: int) -> Node:
        n = self.unary()
        while True:
            c = self.cur
            if c.kind != "punct":
                return n
            p = BIN_PREC.get(c.text)
            if p is None or p < min_prec:
                return n
            self.i += 1
            r = self.binary(p + 1)
            n = _n("bin", n.start, r.end, c.text, kids=(n, r))

    def unary(self) -> Node:
        c = self.cur
        if c.kind == "punct" and c.text in ("+", "-", "!", "~", "*", "&", "++", "--"):
            self.i += 1
            k = self.unary()
            return _n("unary", c.start, k.end, c.text, kids=(k,))
        if c.kind == "id" and c.text == "sizeof":
            self.i += 1
            if self.at("("):
                op = self.eat("(")
                depth = 1
                j = self.i
                while depth:
                    tk = self.t[j]
                    if tk.kind == "eof":
                        raise ParseError("unterminated sizeof")
                    if tk.kind == "punct" and tk.text == "(":
                        depth += 1
                    elif tk.kind == "punct" and tk.text == ")":
                        depth -= 1
                    j += 1
                close = self.t[j - 1]
                self.i = j
                return _n("sizeof", c.start, close.end,
                          text=self.s[op.end:close.start].strip())
            k = self.unary()
            return _n("sizeof", c.start, k.end, kids=(k,))
        if c.kind == "punct" and c.text == "(":
            if self._cast_is_ambiguous(self.i + 1):
                raise Ambiguous(f"`({self.t[self.i + 1].text})` cast-or-group")
            if self._is_type_name(self.i + 1):
                # the type's closing paren is the one MATCHING the open paren,
                # not the first `)` in the stream: `(void (*)(f32, f32))` has
                # two nested groups before it.
                close = self._skip_group(self.i) - 1
                if close < 0:
                    raise ParseError("unterminated cast")
                ty = self.s[self.t[self.i + 1].start:self.t[close].start].strip()
                self.i = close + 1
                k = self.unary()
                return _n("cast", c.start, k.end, text=ty, kids=(k,))
        return self.postfix()

    def postfix(self) -> Node:
        n = self.primary()
        while True:
            c = self.cur
            if c.kind != "punct":
                return n
            if c.text == "(":
                self.i += 1
                args: List[Node] = []
                if not self.at(")"):
                    args.append(self.assign())
                    while self.at(","):
                        self.i += 1
                        args.append(self.assign())
                close = self.eat(")")
                n = _n("call", n.start, close.end, kids=[n] + args)
            elif c.text == "[":
                self.i += 1
                idx = self.expr()
                close = self.eat("]")
                n = _n("index", n.start, close.end, kids=(n, idx))
            elif c.text in (".", "->"):
                self.i += 1
                nm = self.cur
                if nm.kind != "id":
                    raise ParseError("member name expected")
                self.i += 1
                n = _n("member", n.start, nm.end, c.text, nm.text, kids=(n,))
            elif c.text in ("++", "--"):
                self.i += 1
                n = _n("postincr", n.start, c.end, c.text, kids=(n,))
            else:
                return n

    def primary(self) -> Node:
        c = self.cur
        if c.kind == "id":
            self.i += 1
            return _n("id", c.start, c.end, text=c.text)
        if c.kind in ("num", "str", "chr"):
            self.i += 1
            return _n(c.kind, c.start, c.end, text=c.text)
        if c.kind == "punct" and c.text == "(":
            self.i += 1
            inner = self.expr()
            close = self.eat(")")
            return _n("paren", c.start, close.end, kids=(inner,))
        raise ParseError(f"unexpected {c.text!r}")


_TYPEDEF_CACHE: Optional[frozenset] = None


def project_typedefs(root=None) -> frozenset:
    """Every typedef name declared under include/ and src/.

    `(MtxPtr)x` is a cast and `(count)*x` is a multiply, and nothing in the
    token stream distinguishes them -- so without the real typedef list the
    parser has to refuse both.  That refusal was the single largest cause of
    skipped expressions in the first sweep (48 of 65), and a region a tool
    cannot parse looks exactly like a region it examined and cleared.
    """
    global _TYPEDEF_CACHE
    if _TYPEDEF_CACHE is not None:
        return _TYPEDEF_CACHE
    import pathlib
    root = pathlib.Path(root or pathlib.Path(__file__).resolve().parent.parent)
    names = set()
    pat = re.compile(r"\btypedef\b")
    fnptr = re.compile(r"\(\s*\*+\s*([A-Za-z_]\w*)\s*\)")
    for sub in ("include", "src"):
        d = root / sub
        if not d.is_dir():
            continue
        for f in d.rglob("*.h"):
            names |= _typedefs_in(f, pat, fnptr)
        for f in d.rglob("*.c"):
            names |= _typedefs_in(f, pat, fnptr)
    _TYPEDEF_CACHE = frozenset(names)
    return _TYPEDEF_CACHE


def _typedefs_in(path, pat, fnptr) -> set:
    try:
        txt = path.read_text("latin-1")
    except OSError:
        return set()
    out = set()
    for m in pat.finditer(txt):
        i, depth = m.end(), 0
        while i < len(txt):
            c = txt[i]
            if c in "{([":
                depth += 1
            elif c in "})]":
                depth -= 1
            elif c == ";" and depth <= 0:
                break
            i += 1
        decl = txt[m.end():i]
        fp = fnptr.search(decl)
        if fp:
            out.add(fp.group(1))
            continue
        # `typedef struct {...} A, *B;` -- every declarator after the body
        tail = decl[decl.rfind("}") + 1:] if "}" in decl else decl
        for part in tail.split(","):
            ids = _IDENT.findall(part)
            if ids:
                out.add(ids[-1])
    return out


def parse_expression(text: str, known_ids=frozenset(),
                     known_types=frozenset(), lo: int = 0,
                     hi: Optional[int] = None) -> Node:
    toks = tokenize(text, lo, hi)
    return Parser(text, toks, known_ids, known_types).parse()


# ------------------------------------------------------------------- helpers


def strip_parens(n: Node) -> Node:
    while n.kind == "paren":
        n = n.kids[0]
    return n


IMPURE_KINDS = {"call", "assign", "postincr", "comma"}


def is_pure(n: Node) -> bool:
    """No call, no assignment, no ++/--, no comma: reordering is observable
    only through those, so a pure node may be evaluated in any order."""
    for k in n.walk():
        if k.kind in IMPURE_KINDS:
            return False
        if k.kind == "unary" and k.op in ("++", "--"):
            return False
    return True


def normal_text(n: Node, s: str) -> str:
    """Whitespace/comment-free spelling, used to key identical leaves."""
    return "".join(t.text for t in tokenize(n.src(s)) if t.kind != "eof")
