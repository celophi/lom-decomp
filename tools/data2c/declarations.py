"""What the decomp's C says each data symbol is.

The sources are parsed with libclang for the PS1 target (mipsel), so sizes,
offsets and alignment are the PS1 ones. Only declarations are used; function
bodies do not matter, except that every function name is collected so a
pointer to it can be written as a code pointer.

One symbol is often declared differently in different files (a u8[] view in
one, a struct in another). choose() picks one; the report lists the rest.
"""
from __future__ import annotations

import collections
import pathlib

import clang.cindex as ci

TypeKind = ci.TypeKind
CursorKind = ci.CursorKind

# Integer kinds libclang reports, and whether each is signed.
SIGNED = {
    TypeKind.CHAR_S: True, TypeKind.SCHAR: True, TypeKind.CHAR_U: False, TypeKind.UCHAR: False,
    TypeKind.SHORT: True, TypeKind.USHORT: False, TypeKind.INT: True, TypeKind.UINT: False,
    TypeKind.LONG: True, TypeKind.ULONG: False, TypeKind.LONGLONG: True, TypeKind.ULONGLONG: False,
    TypeKind.BOOL: False,
}
FUNCTION_KINDS = (TypeKind.FUNCTIONPROTO, TypeKind.FUNCTIONNOPROTO)


def clang_args(version: str, includes) -> list[str]:
    """Parse the C the way the PS1 build sees it (the header-only M2CTX mode)."""
    return (["--target=mipsel-unknown-linux-gnu", "-std=gnu89", f"-DVERSION_{version.upper()}", "-DM2CTX",
             "-Iinclude", "-Iinclude/sdk", "-Wno-everything", "-fms-extensions"]
            + [f"-I{d}" for d in includes])


def is_integer(t) -> bool:
    return t.kind in SIGNED or t.kind == TypeKind.ENUM


def is_signed(t) -> bool:
    """Enums are ints on the PS1 compiler."""
    return t.kind == TypeKind.ENUM or SIGNED.get(t.kind, True)


def is_union(t) -> bool:
    return t.get_declaration().kind == CursorKind.UNION_DECL


def points_to_function(t) -> bool:
    return t.get_pointee().get_canonical().kind in FUNCTION_KINDS


class Declarations:
    """Every declaration of the wanted names across a set of source files."""

    def __init__(self, sources, wanted, args):
        self.types = collections.defaultdict(list)       # name -> [(type, file name)]
        self.function_names = set()
        index = ci.Index.create()
        for src in sources:
            tu = index.parse(str(src), args=args)
            for cur in tu.cursor.walk_preorder():
                if cur.kind == CursorKind.VAR_DECL and cur.spelling in wanted:
                    self.types[cur.spelling].append((cur.type, pathlib.Path(str(src)).name))
                elif cur.kind == CursorKind.FUNCTION_DECL:
                    self.function_names.add(cur.spelling)

    def for_names(self, names):
        return [d for n in names for d in self.types.get(n, [])]


def layout_key(t) -> str:
    """A fingerprint of a type's PS1 layout: two types with the same key have the
    same size, field offsets and pointer positions."""
    t = t.get_canonical()
    k = t.kind
    if is_integer(t):
        return f"i{t.get_size()}{'s' if is_signed(t) else 'u'}"
    if k == TypeKind.POINTER:
        return "p"
    if k == TypeKind.CONSTANTARRAY:
        return f"[{t.element_count}]{layout_key(t.element_type)}"
    if k == TypeKind.INCOMPLETEARRAY:
        return f"[]{layout_key(t.element_type)}"
    if k == TypeKind.RECORD:
        fields = [f"{t.get_offset(f.spelling)}:{layout_key(f.type)}"
                  + (f":{f.get_bitfield_width()}" if f.is_bitfield() else "") for f in t.get_fields()]
        return ("U" if is_union(t) else "S") + "{" + ",".join(fields) + "}"
    if k in (TypeKind.FLOAT, TypeKind.DOUBLE):
        return f"f{t.get_size()}"
    return f"?{k}"


def choose(decls, extent: int, report, name: str):
    """One type for a symbol from all its declarations.

    Prefer, in order: a complete type that fits in the symbol's extent, one
    that has pointers (they matter for a native build), a struct over a plain
    array, and then the larger type.
    """
    variants = collections.OrderedDict()
    for t, f in decls:
        variants.setdefault(layout_key(t), (t, f))
    if len(variants) > 1:
        report.note("symbol with conflicting declarations", f"{name}: {len(variants)} layouts")
        if any("p" in k for k in variants):
            files = sorted({f.split('.')[0] for _, f in variants.values()})[:3]
            report.note("conflicting declarations that disagree about pointers", f"{name}: " + " vs ".join(files))

    def score(t):
        t = t.get_canonical()
        size = t.get_size()
        key = layout_key(t)
        return (size > 0 and size <= extent, "p" in key, t.kind == TypeKind.RECORD or "S{" in key, size)
    return max(variants.values(), key=lambda tf: score(tf[0]))[0]


def record_members(t):
    """A record's members in the order the generated type lays them out.

    Structs get explicit byte padding members wherever the declared fields
    leave a gap: C cannot initialize padding, and some of the game's padding
    bytes are not zero. Unions put the member covering the most bytes first,
    because C89 can only initialize a union's first member; member order does
    not change a union's layout.

    Returns [("field", cursor) | ("pad", offset, size)].
    """
    fields = list(t.get_fields())
    if is_union(t):
        def cover(f):
            ft = f.type.get_canonical()
            return (ft.get_size(), "p" in layout_key(ft))
        widest = max(fields, key=cover) if fields else None
        return [("field", widest)] + [("field", f) for f in fields if f is not widest]
    out = []
    pos = 0
    for f in fields:
        start_bits = t.get_offset(f.spelling)
        if f.is_bitfield():
            start, end = start_bits // 8, (start_bits + f.get_bitfield_width() + 7) // 8
        else:
            start = start_bits // 8
            end = start + f.type.get_canonical().get_size()
        if start > pos and not f.is_bitfield():
            out.append(("pad", pos, start - pos))
        out.append(("field", f))
        pos = max(pos, end)
    # Trailing padding is left alone next to bitfields, whose storage units cover it.
    if t.get_size() > pos and not any(f.is_bitfield() for f in fields):
        out.append(("pad", pos, t.get_size() - pos))
    return out
