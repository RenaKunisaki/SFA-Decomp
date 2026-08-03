#!/usr/bin/env python3
"""Prove that a source rewrite computes EXACTLY what the original computed.

Why this exists
---------------
Every gate this project owns -- the objdiff fuzzy score, tools/obj_equal.py,
tools/score_delta_gate.py, the forced link -- compares our output against
retail's BYTES.  None of them compares it against retail's MEANING.  So a
rewrite that changes the arithmetic and happens to land bytes closer to retail
is scored as a WIN and lands.  That already happened: a regex operand-swapper
turned

    verts + j * 12          (a base plus a scaled index)
into
    (j + verts) * 12        (a completely different computation)

because a regex cannot see precedence.  The score went up, the tool's own guard
passed it, and nothing downstream could have caught it.

This module is the missing gate.  It works on TEXT, independently of whatever
produced the rewrite, so a splicing bug in the generator cannot hide from it:
both spellings are re-parsed from scratch and the rewrite must be proved a
genuine algebraic identity over the operand types.

Two independent instruments must BOTH agree before a rewrite is cleared:

  1. a structural proof -- the two ASTs must be relatable by a closed list of
     identities that are true for every input (commutativity of + * & | ^ ==
     !=, the relational flip a<b == b>a, the ternary flip c?a:b == !c?b:a, and
     -- only with an explicit integer-type proof -- associativity of + * & | ^);

  2. a differential evaluation -- both spellings are evaluated under randomized
     inputs in an integer model (32-bit wrapping) and a floating-point model
     (compared BITWISE), with unknown sub-terms modelled as deterministic
     functions of their inputs.  Any disagreement rejects.

The two catch different mistakes.  The structural proof rejects
`verts + j * 12 -> (j + verts) * 12` at the root (an addition cannot equal a
multiplication).  The differential evaluation is what stops an identity being
mis-applied to a type it is not true for: floating-point addition and
multiplication are NOT associative, and it is the float model that says so.

Reordering is also observable through side effects and through short-circuit
evaluation, so any rule that moves one operand across another additionally
requires both operands to be pure (no call, assignment, ++/--, or comma), and
&& / || are never reordered.

Run `semantic_equivalence.py --self-test` for the control suite.  It carries
the exact `verts + j * 12` mutation as a positive control and MUST reject it.
"""
from __future__ import annotations

import argparse
import hashlib
import math
import random
import re
import struct
import sys
import zlib
from dataclasses import dataclass, field
from pathlib import Path
from typing import Dict, FrozenSet, List, Optional, Sequence, Tuple

sys.path.insert(0, str(Path(__file__).resolve().parent))
from cexpr import (Ambiguous, Node, ParseError, is_pure, normal_text,
                   parse_expression, strip_parens, tokenize)

# --------------------------------------------------------------- identities

# Commutative for every input in both the integer and the IEEE754 models.
# `-` and `/` are not commutative; `&&`/`||` are excluded because swapping them
# changes which side is evaluated at all.
COMMUTATIVE = {"+", "*", "&", "|", "^", "==", "!="}

# Associative in 32-bit wrapping integer arithmetic ONLY.  Never applied
# without a positive integer-type proof; float `+`/`*` are famously not.
ASSOCIATIVE_INT = {"+", "*", "&", "|", "^"}

RELATIONAL_FLIP = {"<": ">", ">": "<", "<=": ">=", ">=": "<="}

INT_TYPE_WORDS = {
    "u8", "u16", "u32", "u64", "s8", "s16", "s32", "s64",
    "int", "short", "long", "char", "unsigned", "signed", "size_t", "BOOL",
}
FLOAT_TYPE_WORDS = {"f32", "f64", "float", "double", "f80"}

CAST_WIDTH = {
    "u8": (8, False), "s8": (8, True), "char": (8, True),
    "u16": (16, False), "s16": (16, True), "short": (16, True),
    "u32": (32, False), "s32": (32, True), "int": (32, True),
    "long": (32, True), "unsigned": (32, False), "unsignedint": (32, False),
}


# ------------------------------------------------------------------ verdict


@dataclass
class Verdict:
    ok: bool
    reason: str
    rules: List[str] = field(default_factory=list)

    def __bool__(self):
        return self.ok

    def __str__(self):
        tag = "EQUIVALENT" if self.ok else "REJECTED"
        r = (" [" + ",".join(sorted(set(self.rules))) + "]") if self.rules else ""
        return f"{tag}: {self.reason}{r}"


# ------------------------------------------------------------- type oracle


class TypeOracle:
    """Positively proves a name integer-typed, or says nothing.

    Deliberately one-sided: everything it cannot establish is UNKNOWN, and the
    only rule that consults it (associativity) refuses on UNKNOWN.  A wrong
    "integer" answer would silently authorise a regrouping that is illegal for
    floats, so the parse is narrow on purpose -- plain declarations of scalar
    locals and parameters, nothing inferred through structs or headers.
    """

    DECL = re.compile(
        r"(?:^|[;{}(),])\s*(?:const\s+|volatile\s+|static\s+|register\s+)*"
        r"(?P<ty>[A-Za-z_]\w*)\s+(?P<stars>\**)\s*(?P<name>[A-Za-z_]\w*)\s*"
        r"(?=[;,)=\[])")

    def __init__(self, decl_text: str = ""):
        self.kind: Dict[str, str] = {}
        for m in self.DECL.finditer(decl_text or ""):
            ty, name = m.group("ty"), m.group("name")
            if m.group("stars"):
                self.kind.setdefault(name, "ptr")
            elif ty in INT_TYPE_WORDS:
                self.kind.setdefault(name, "int")
            elif ty in FLOAT_TYPE_WORDS:
                self.kind.setdefault(name, "float")

    def names(self) -> FrozenSet[str]:
        return frozenset(self.kind)

    def is_int_expr(self, n: Node) -> bool:
        n = strip_parens(n)
        if n.kind == "num":
            t = n.text.lower()
            return ("." not in t and "e" not in t.replace("0x", "")
                    and not t.endswith("f"))
        if n.kind == "id":
            return self.kind.get(n.text) == "int"
        if n.kind == "unary" and n.op in ("-", "+", "~"):
            return self.is_int_expr(n.kids[0])
        if n.kind == "bin" and n.op in ASSOCIATIVE_INT | {"-", "<<", ">>"}:
            return all(self.is_int_expr(k) for k in n.kids)
        if n.kind == "cast":
            return n.text.replace(" ", "") in CAST_WIDTH
        return False


# ---------------------------------------------------------- structural proof


def _split_sign(n: Node) -> Tuple[int, Node]:
    """(parity, node-with-every-minus-peeled-off-the-product)."""
    n = strip_parens(n)
    if n.kind == "unary" and n.op == "-":
        p, k = _split_sign(n.kids[0])
        return 1 - p, k
    if n.kind == "bin" and n.op == "*":
        p1, x = _split_sign(n.kids[0])
        p2, y = _split_sign(n.kids[1])
        if p1 == p2 == 0:
            return 0, n
        return p1 ^ p2, Node("bin", n.start, n.end, "*", "", [x, y])
    return 0, n


def _flatten(n: Node, op: str) -> List[Node]:
    n = strip_parens(n)
    if n.kind == "bin" and n.op == op:
        return _flatten(n.kids[0], op) + _flatten(n.kids[1], op)
    return [n]


class Prover:
    def __init__(self, sa: str, sb: str, oracle: TypeOracle, allow_assoc: bool):
        self.sa, self.sb = sa, sb
        self.oracle = oracle
        self.assoc = allow_assoc
        self.rules: List[str] = []
        self.why = ""

    def fail(self, msg: str) -> bool:
        if not self.why:
            self.why = msg
        return False

    def eq(self, a: Node, b: Node) -> bool:
        a, b = strip_parens(a), strip_parens(b)

        if self.eq_sign(a, b):
            return True
        if a.kind == "bin" and b.kind == "bin":
            return self.eq_bin(a, b)
        if a.kind == "cond" and b.kind == "cond":
            return self.eq_cond(a, b)

        if a.kind != b.kind:
            return self.fail(f"node kind {a.kind} vs {b.kind}"
                             f" ({a.src(self.sa)!r} vs {b.src(self.sb)!r})")

        if a.kind in ("id", "num", "str", "chr"):
            if a.text != b.text:
                return self.fail(f"leaf {a.text!r} vs {b.text!r}")
            return True
        if a.kind == "sizeof":
            if a.text != b.text or len(a.kids) != len(b.kids):
                return self.fail("sizeof operand differs")
            return all(self.eq(x, y) for x, y in zip(a.kids, b.kids))
        if a.kind == "member":
            if a.op != b.op or a.text != b.text:
                return self.fail(f"member {a.op}{a.text} vs {b.op}{b.text}")
            return self.eq(a.kids[0], b.kids[0])
        if a.kind == "cast":
            if a.text.replace(" ", "") != b.text.replace(" ", ""):
                return self.fail(f"cast {a.text!r} vs {b.text!r}")
            return self.eq(a.kids[0], b.kids[0])
        if a.kind in ("unary", "postincr", "assign", "comma"):
            if a.op != b.op:
                return self.fail(f"operator {a.op!r} vs {b.op!r}")
            if len(a.kids) != len(b.kids):
                return self.fail("arity")
            return all(self.eq(x, y) for x, y in zip(a.kids, b.kids))
        if a.kind in ("call", "index"):
            if len(a.kids) != len(b.kids):
                return self.fail("arity")
            return all(self.eq(x, y) for x, y in zip(a.kids, b.kids))
        return self.fail(f"unhandled node kind {a.kind}")

    def eq_sign(self, a: Node, b: Node) -> bool:
        """`-(x * y)` == `(-x) * y` == `x * (-y)`, and `(-x) * (-y)` == `x * y`.

        A product's sign is the exclusive-or of its factors' signs and its
        magnitude does not depend on them, in IEEE754 and in two's complement
        alike, so a leading minus may sit on the product or on either factor.
        The descent goes through `*` ONLY: `-(x + y)` is not `(-x) + y`, and
        integer division's rounding makes the same move worth refusing there.
        """
        pa, na = _split_sign(a)
        pb, nb = _split_sign(b)
        if pa != pb or (na is a and nb is b):
            return False
        if na.kind != "bin" or na.op != "*" or nb.kind != "bin" or nb.op != "*":
            return False
        saved = list(self.rules)
        why = self.why
        if self.eq(na, nb):
            self.rules.append("SIGN")
            return True
        self.rules = saved
        self.why = why
        return False

    def eq_bin(self, a: Node, b: Node) -> bool:
        if a.op == b.op:
            saved = list(self.rules)
            if self.eq(a.kids[0], b.kids[0]) and self.eq(a.kids[1], b.kids[1]):
                return True
            self.rules = saved
            # each alternative gets a clean slate for the explanation, so the
            # surfaced reason is the one from the rule that was actually being
            # tried, not from the plain structural attempt that ran first.
            self.why = ""
            if a.op in COMMUTATIVE:
                if not (is_pure(a.kids[0]) and is_pure(a.kids[1])):
                    return self.fail(f"`{a.op}` operands not pure -- reordering"
                                     " would change side-effect order")
                if self.eq(a.kids[0], b.kids[1]) and self.eq(a.kids[1], b.kids[0]):
                    self.rules.append("COMMUTE")
                    return True
                self.rules = saved
                self.why = ""
            if self.assoc and a.op in ASSOCIATIVE_INT:
                if self.eq_assoc(a, b):
                    return True
                self.rules = saved
            return self.fail(f"`{a.op}` operands do not correspond")

        if RELATIONAL_FLIP.get(a.op) == b.op:
            if not (is_pure(a.kids[0]) and is_pure(a.kids[1])):
                return self.fail("relational flip across impure operands")
            if self.eq(a.kids[0], b.kids[1]) and self.eq(a.kids[1], b.kids[0]):
                self.rules.append("RELFLIP")
                return True
            return self.fail("relational flip operands do not correspond")
        return self.fail(f"operator {a.op!r} vs {b.op!r}")

    def eq_assoc(self, a: Node, b: Node) -> bool:
        fa, fb = _flatten(a, a.op), _flatten(b, b.op)
        if len(fa) != len(fb) or len(fa) < 3:
            return self.fail("associative chains differ in length")
        if not all(self.oracle.is_int_expr(x) for x in fa + fb):
            return self.fail(f"`{a.op}` regrouping needs an integer proof for"
                             " every operand (FP + and * are not associative)")
        if not all(is_pure(x) for x in fa + fb):
            return self.fail("associative regrouping across impure operands")
        pool = list(fb)
        for x in fa:
            for i, y in enumerate(pool):
                saved = list(self.rules)
                if self.eq(x, y):
                    pool.pop(i)
                    break
                self.rules = saved
            else:
                return self.fail("associative chain operands do not correspond")
        self.rules.append("ASSOC-INT")
        return True

    def eq_cond(self, a: Node, b: Node) -> bool:
        saved = list(self.rules)
        if (self.eq(a.kids[0], b.kids[0]) and self.eq(a.kids[1], b.kids[1])
                and self.eq(a.kids[2], b.kids[2])):
            return True
        self.rules = saved
        ca, cb = strip_parens(a.kids[0]), strip_parens(b.kids[0])
        neg = None
        if cb.kind == "unary" and cb.op == "!":
            neg = (ca, strip_parens(cb.kids[0]))
        elif ca.kind == "unary" and ca.op == "!":
            neg = (strip_parens(ca.kids[0]), cb)
        if neg is None:
            return self.fail("ternary conditions differ and neither negates"
                             " the other")
        if not self.eq(neg[0], neg[1]):
            self.rules = saved
            return self.fail("ternary negated conditions do not correspond")
        if self.eq(a.kids[1], b.kids[2]) and self.eq(a.kids[2], b.kids[1]):
            self.rules.append("TERNFLIP")
            return True
        self.rules = saved
        return self.fail("ternary arms do not correspond under negation")


# ------------------------------------------------- differential evaluation


def _wrap32(v: int) -> int:
    v &= 0xFFFFFFFF
    return v - 0x100000000 if v >= 0x80000000 else v


class Evaluator:
    """Evaluate an expression with unknown sub-terms modelled as deterministic
    functions of their inputs.

    `a[i * j]` and `a[j * i]` must agree, so an index is not a bare opaque
    leaf: it is hash(base-name, index-value).  The same holds for members,
    calls and dereferences.  Every unknown is therefore a genuine function --
    equal inputs give equal outputs, different inputs may not -- which is the
    weakest assumption under which the comparison still has teeth.

    The model names what an UNKNOWN leaf is, not what every leaf is: a name the
    oracle proves integer is an integer in both models, and one it proves
    floating-point is a float in both.  Arithmetic then follows the values --
    two integers combine with 32-bit wrapping, anything else in double
    precision -- so integer associativity is exact while float associativity
    still breaks, which is the whole point of running the float model.
    """

    def __init__(self, model: str, seed: int, oracle: "TypeOracle" = None):
        self.model = model                      # "int" | "float"
        self.seed = seed
        self.oracle = oracle
        self.cache: Dict[str, object] = {}

    def leaf_model(self, name: str) -> str:
        k = self.oracle.kind.get(name) if self.oracle else None
        if k == "int" or k == "ptr":
            return "int"
        if k == "float":
            return "float"
        return self.model

    # -- unknowns
    def opaque(self, key: str, args: Sequence[object] = (), model: str = None):
        blob = f"{self.seed}|{key}|" + "|".join(
            struct.pack(">d", float(a)).hex() if isinstance(a, float)
            else str(int(a)) for a in args)
        h = int.from_bytes(hashlib.blake2b(blob.encode(), digest_size=8).digest(),
                           "big")
        if (model or self.model) == "int":
            return _wrap32(h)
        # A FULL 52-bit mantissa is the point: draw floats with fewer
        # significant bits than a double carries and the products of three of
        # them stay exactly representable, so associativity violations never
        # appear and the float model silently stops testing anything.
        mant = 1.0 + (h & ((1 << 52) - 1)) / float(1 << 52)
        # The EXPONENT range is deliberately narrow while the MANTISSA is full
        # width.  Rounding, not magnitude, is what breaks float associativity,
        # and a wide exponent only makes a chain of seven multiplies overflow
        # an f32 cast -- which the evaluator would then report as a semantic
        # difference that is really an artefact of its own sampling.
        expo = ((h >> 52) & 0x7) - 4
        return math.ldexp(mant, expo) * (-1.0 if (h >> 58) & 1 else 1.0)

    def num(self, text: str):
        t = text.rstrip("uUlLfF")
        try:
            if t.lower().startswith("0x"):
                return _wrap32(int(t, 16))
            if "." in t or "e" in t.lower():
                return float(t)
            return _wrap32(int(t, 8) if (len(t) > 1 and t[0] == "0") else int(t))
        except ValueError:
            return self.opaque("num:" + text)

    def as_int(self, v) -> int:
        return _wrap32(int(v)) if not isinstance(v, int) else v

    def ev(self, n: Node, s: str):
        n = strip_parens(n)
        k = n.kind
        if k == "num":
            return self.num(n.text)
        if k == "id":
            return self.opaque("id:" + n.text, model=self.leaf_model(n.text))
        if k in ("str", "chr"):
            return self.opaque(k + ":" + n.text, model="int")
        if k == "sizeof":
            return self.opaque("sizeof:" + (n.text or normal_text(n.kids[0], s)),
                               model="int")
        if k == "member":
            return self.opaque("member:" + n.op + n.text, [self.ev(n.kids[0], s)])
        if k == "index":
            return self.opaque("index", [self.ev(n.kids[0], s), self.ev(n.kids[1], s)])
        if k == "call":
            return self.opaque("call:" + normal_text(n.kids[0], s),
                               [self.ev(x, s) for x in n.kids[1:]])
        if k == "postincr":
            return self.opaque("postincr:" + normal_text(n.kids[0], s))
        if k == "assign":
            return self.ev(n.kids[1], s)
        if k == "comma":
            self.ev(n.kids[0], s)
            return self.ev(n.kids[1], s)
        if k == "cast":
            v = self.ev(n.kids[0], s)
            ty = n.text.replace(" ", "")
            if "*" in ty:
                return self.opaque("castptr:" + ty, [v])
            if ty in CAST_WIDTH:
                bits, signed = CAST_WIDTH[ty]
                iv = self.as_int(v) & ((1 << bits) - 1)
                if signed and iv >= (1 << (bits - 1)):
                    iv -= (1 << bits)
                return iv
            if ty in ("f32", "float"):
                f = float(v)
                try:
                    return struct.unpack(">f", struct.pack(">f", f))[0]
                except OverflowError:
                    return math.copysign(math.inf, f)
            if ty in ("f64", "double"):
                return float(v)
            return self.opaque("cast:" + ty, [v])
        if k == "unary":
            if n.op == "&":
                return self.opaque("addrof", [self.ev(n.kids[0], s)], model="int")
            if n.op == "*":
                return self.opaque("deref", [self.ev(n.kids[0], s)])
            if n.op in ("++", "--"):
                return self.opaque("preincr:" + normal_text(n.kids[0], s))
            v = self.ev(n.kids[0], s)
            if n.op == "-":
                # follow the VALUE, not the model: in the integer model an
                # unknown leaf is an int, but a float literal in the same
                # expression still makes the sub-term a float, and coercing it
                # here reported a true identity as a difference.
                return _wrap32(-v) if isinstance(v, int) else -v
            if n.op == "+":
                return v
            if n.op == "!":
                z = 0 if isinstance(v, int) else 0.0
                return 1 if v == z else 0
            if n.op == "~":
                return _wrap32(~self.as_int(v))
        if k == "cond":
            c = self.ev(n.kids[0], s)
            z = 0 if isinstance(c, int) else 0.0
            return self.ev(n.kids[1], s) if c != z else self.ev(n.kids[2], s)
        if k == "bin":
            return self.bin(n, s)
        raise ParseError(f"cannot evaluate node kind {k}")

    def bin(self, n: Node, s: str):
        op = n.op
        if op in ("&&", "||"):
            a = self.ev(n.kids[0], s)
            az = a != (0 if isinstance(a, int) else 0.0)
            if op == "&&" and not az:
                return 0
            if op == "||" and az:
                return 1
            b = self.ev(n.kids[1], s)
            return 1 if b != (0 if isinstance(b, int) else 0.0) else 0
        a = self.ev(n.kids[0], s)
        b = self.ev(n.kids[1], s)
        if op in ("<", ">", "<=", ">=", "==", "!="):
            if isinstance(a, float) != isinstance(b, float):
                a, b = float(a), float(b)
            return 1 if {"<": a < b, ">": a > b, "<=": a <= b, ">=": a >= b,
                         "==": a == b, "!=": a != b}[op] else 0
        if op in ("&", "|", "^", "<<", ">>", "%"):
            ia, ib = self.as_int(a), self.as_int(b)
            if op == "&":
                r = ia & ib
            elif op == "|":
                r = ia | ib
            elif op == "^":
                r = ia ^ ib
            elif op == "<<":
                r = ia << (ib & 31)
            elif op == ">>":
                r = ia >> (ib & 31)
            else:
                r = ia - (ib or 1) * int(ia / (ib or 1))
            return _wrap32(r)
        if isinstance(a, int) and isinstance(b, int):
            ia, ib = self.as_int(a), self.as_int(b)
            if op == "+":
                return _wrap32(ia + ib)
            if op == "-":
                return _wrap32(ia - ib)
            if op == "*":
                return _wrap32(ia * ib)
            if op == "/":
                ib = ib or 1
                q = abs(ia) // abs(ib)
                return _wrap32(-q if (ia < 0) != (ib < 0) else q)
        else:
            a, b = float(a), float(b)
            if op == "+":
                return a + b
            if op == "-":
                return a - b
            if op == "*":
                return a * b
            if op == "/":
                return a / (b if b != 0.0 else 1.0)
        raise ParseError(f"cannot evaluate operator {op!r}")


def _bits(v) -> bytes:
    if isinstance(v, int):
        return b"i" + struct.pack(">q", v)
    return b"f" + struct.pack(">d", v)


def differential(a: Node, sa: str, b: Node, sb: str, trials: int = 256,
                 oracle: "TypeOracle" = None) -> Tuple[bool, str]:
    for mi, model in enumerate(("int", "float")):
        for t in range(trials):
            ea = Evaluator(model, seed=mi * 7919 + t, oracle=oracle)
            eb = Evaluator(model, seed=mi * 7919 + t, oracle=oracle)
            try:
                va = ea.ev(a, sa)
                vb = eb.ev(b, sb)
            except (OverflowError, ValueError, ZeroDivisionError) as e:
                return False, f"{model} model raised {e}"
            if _bits(va) != _bits(vb):
                return False, (f"{model} model disagrees on trial {t}: "
                               f"{va!r} vs {vb!r}")
    return True, f"{trials} trials x 2 models agree"


# ------------------------------------------------------------------- driver


def prove(before: str, after: str, decl_text: str = "",
          known_types: FrozenSet[str] = frozenset(),
          known_ids: FrozenSet[str] = frozenset(),
          allow_assoc: bool = False, trials: int = 256) -> Verdict:
    """Is `after` the same computation as `before`?  Both are re-parsed."""
    if before.strip() == after.strip():
        return Verdict(True, "identical text")
    oracle = TypeOracle(decl_text)
    ids = frozenset(oracle.names()) | frozenset(known_ids)
    try:
        a = parse_expression(before, ids, known_types)
    except Ambiguous as e:
        return Verdict(False, f"PARSE-AMBIGUOUS before: {e}")
    except ParseError as e:
        return Verdict(False, f"PARSE-ERROR before: {e}")
    try:
        b = parse_expression(after, ids, known_types)
    except Ambiguous as e:
        return Verdict(False, f"PARSE-AMBIGUOUS after: {e}")
    except ParseError as e:
        return Verdict(False, f"PARSE-ERROR after: {e}")

    p = Prover(before, after, oracle, allow_assoc)
    if not p.eq(a, b):
        return Verdict(False, "structural proof failed: " + p.why, p.rules)
    if not p.rules:
        return Verdict(True, "structurally identical", [])
    ok, why = differential(a, before, b, after, trials, oracle)
    if not ok:
        return Verdict(False, "differential evaluation failed: " + why, p.rules)
    return Verdict(True, why, p.rules)


# ------------------------------------------------------------- control suite

# (before, after, expect_ok, note).  The FIRST entry is the mandated positive
# control: the exact mutation a regex operand-swapper produced and applied.
CONTROLS = [
    ("verts + j * 12", "(j + verts) * 12", False,
     "POSITIVE CONTROL: precedence-boundary splice changes the arithmetic", {}),
    ("verts + j * 12", "verts + 12 * j", True, "swap inside the multiply", {}),
    ("verts + j * 12", "j * 12 + verts", True, "swap the addition", {}),
    ("a * b + c * d", "b * a + c * d", True, "commute one factor pair", {}),
    ("a * b + c * d", "c * d + a * b", True, "commute the sum", {}),
    ("x < y", "y > x", True, "relational flip", {}),
    ("x <= y", "y >= x", True, "relational flip", {}),
    ("x < y", "y < x", False, "not a flip -- the wrong operand order", {}),
    ("c ? a : b", "!c ? b : a", True, "ternary flip", {}),
    ("c ? a : b", "c ? b : a", False, "arms swapped without negating", {}),
    ("(f32)a * b", "b * (f32)a", True, "cast operand -- refused by regex tools", {}),
    ("p->pos.x * s", "s * p->pos.x", True, "member chain", {}),
    ("m[i][j] + k", "k + m[i][j]", True, "subscript chain", {}),
    ("a - b", "b - a", False, "subtraction is not commutative", {}),
    ("a / b", "b / a", False, "division is not commutative", {}),
    ("a << b", "b << a", False, "shift is not commutative", {}),
    ("f(x) + y", "y + f(x)", False, "a call moved across an operand", {}),
    ("a && f(x)", "f(x) && a", False, "short-circuit order is observable", {}),
    ("i++ + n", "n + i++", False, "side effect moved", {}),
    ("a & b", "b & a", True, "bitwise and commutes", {}),
    ("a == b", "b == a", True, "equality commutes", {}),
    ("a | b | c", "a | (b | c)", False,
     "associativity without a type proof", {}),
    ("i + j + k", "i + (j + k)", True, "integer associativity, proved",
     {"decl_text": "int i; int j; int k;", "allow_assoc": True}),
    ("x + y + z", "x + (y + z)", False,
     "FLOAT associativity -- rejected by the float model",
     {"decl_text": "f32 x; f32 y; f32 z;", "allow_assoc": True}),
    ("x + y + z", "x + (y + z)", False,
     "associativity with UNKNOWN types stays refused", {"allow_assoc": True}),
    ("a * b * c", "b * a * c", True, "commute inside a chain (no regrouping)", {}),
    ("obj->speed * dt + obj->pos", "obj->pos + dt * obj->speed", True,
     "two commutes in one expression", {}),
    ("obj->speed * dt + obj->pos", "(obj->pos + dt) * obj->speed", False,
     "the same precedence-boundary splice on a member chain", {}),
    ("base + idx * stride", "(idx + base) * stride", False,
     "generalised positive control", {}),
    ("n * 4 + 8", "8 + n * 4", True, "literal-first is legal arithmetic", {}),
    ("n * 4 + 8", "(8 + n) * 4", False, "literal splice across precedence", {}),
    ("a ? b + c : d", "a ? c + b : d", True, "commute inside a ternary arm", {}),
    ("v.x * v.x + v.y * v.y", "v.y * v.y + v.x * v.x", True, "dot-product halves", {}),
    ("v.x * v.x + v.y * v.y", "v.x * v.y + v.y * v.x", False,
     "operands crossed between the two products", {}),
    ("count * sizeof(Thing)", "sizeof(Thing) * count", True, "sizeof operand", {}),
    ("~mask & flags", "flags & ~mask", True, "unary operand commutes", {}),
    # A mixed int/float product: the evaluator must follow the VALUE types, not
    # the model, or it reports this true identity as a difference.
    ("-(x * (-2.2078018e-15f * x2 + 0.000023945184f))",
     "-x * (-2.2078018e-15f * x2 + 0.000023945184f)", True,
     "sign move where one factor is float and the other unknown", {}),
    ("-(a * b)", "-a * b", True, "a product's sign may sit on a factor", {}),
    ("-(a * b)", "a * -b", True, "or on the other factor", {}),
    ("-a * -b", "a * b", True, "two sign flips cancel exactly", {}),
    ("-(a + b)", "-a + b", False, "a SUM's sign does not distribute that way", {}),
    ("-(a / b)", "-a / b", False,
     "division sign placement is refused, not proved", {}),
    ("-(a * b) + c", "-a * b + c", True, "sign move inside a larger expression", {}),
    ("-(a * b)", "-(b * a)", True, "sign move composed with a commute", {}),
    ("a + b", "a + b + 0", False, "an added term is not a permutation", {}),
    ("a * b", "a * b * 1", False, "an added factor is not a permutation", {}),
    # Adversarial probes: the shapes a buggy GENERATOR would plausibly emit.
    ("p[i] + n * 4", "(n + p[i]) * 4", False, "precedence splice on a subscript"),
    ("a * b + c", "a * (b + c)", False, "a parenthesis moved one operator over"),
    ("(a + b) * c", "a + b * c", False, "a parenthesis dropped"),
    ("a - b + c", "a - (b + c)", False, "a minus regrouped"),
    ("x / y * z", "x / (y * z)", False, "a division regrouped"),
    ("a << 2 | b", "b | a << 2", True, "commute around a shift"),
    ("!(a && b)", "!(b && a)", False, "short-circuit operands never reorder"),
    ("v[i].x * s + v[i].y", "v[i].y + s * v[i].x", True,
     "two commutes through subscript and member"),
    ("v[i].x * s + v[i].y", "(v[i].y + v[i].x) * s", False,
     "the splice on that same shape"),
    ("f(a) * b", "b * f(a)", False, "a call moved across an operand"),
    ("-x - y", "-(x - y)", False, "sign does not distribute over a minus"),
]
CONTROLS = [(c[0], c[1], c[2], c[3], c[4] if len(c) > 4 else {}) for c in CONTROLS]


def self_test(verbose: bool = True) -> int:
    bad = 0
    for i, (before, after, want, note, kw) in enumerate(CONTROLS):
        v = prove(before, after, **kw)
        good = (bool(v) == want)
        bad += not good
        if verbose or not good:
            flag = "ok  " if good else "FAIL"
            print(f"[{flag}] {before!r} -> {after!r}\n"
                  f"        expect={'EQUIV' if want else 'REJECT'}  {v}\n"
                  f"        {note}")
    # A second, independent control: break the TYPE ORACLE on purpose so it
    # swears three floats are integers, and check the differential evaluation
    # still refuses the regrouping.  Without this, the float model could quietly
    # rot into a no-op and the suite above would not notice, because every other
    # rejection there is delivered by the structural proof.
    print("\n--- NEGATIVE CONTROL ON THE TYPE ORACLE ---")
    lying = TypeOracle("")
    lying.kind = {"x": "int", "y": "int", "z": "int"}
    fa = parse_expression("x * y * z")
    fb = parse_expression("x * (y * z)")
    p = Prover("x * y * z", "x * (y * z)", lying, True)
    struct_ok = p.eq(fa, fb)
    real = TypeOracle("f32 x; f32 y; f32 z;")
    diff_ok, why = differential(fa, "x * y * z", fb, "x * (y * z)", 256, real)
    print(f"  structural proof under the lying oracle: "
          f"{'PASSED (as expected)' if struct_ok else 'failed'}")
    print(f"  differential evaluation with the true types: "
          f"{'PASSED' if diff_ok else 'REJECTED'} -- {why}")
    if not struct_ok or diff_ok:
        print("  *** the differential instrument has no teeth ***")
        bad += 1
    else:
        print("  the two instruments are independent, as required")

    # the positive control is reported separately: it is the reason this tool
    # exists, and a future edit must never be able to silence it.
    pc = CONTROLS[0]
    v = prove(pc[0], pc[1])
    print("\n--- POSITIVE CONTROL ---")
    print(f"  {pc[0]!r} -> {pc[1]!r}")
    print(f"  {v}")
    if v.ok:
        print("  *** THE GATE IS BROKEN: it cleared the known-bad mutation ***")
        bad += 1
    else:
        print("  rejected, as required")
    print(f"\n{len(CONTROLS) - bad}/{len(CONTROLS)} controls hold"
          f"{'' if not bad else f'  ({bad} FAILURES)'}")
    return 1 if bad else 0


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--self-test", action="store_true")
    ap.add_argument("-q", "--quiet", action="store_true")
    ap.add_argument("--before")
    ap.add_argument("--after")
    ap.add_argument("--decls", default="")
    ap.add_argument("--assoc", action="store_true")
    ap.add_argument("--trials", type=int, default=256)
    a = ap.parse_args()
    if a.self_test:
        return self_test(not a.quiet)
    if not (a.before and a.after):
        ap.error("give --before and --after, or --self-test")
    v = prove(a.before, a.after, a.decls, allow_assoc=a.assoc, trials=a.trials)
    print(v)
    return 0 if v.ok else 2


if __name__ == "__main__":
    sys.exit(main())
