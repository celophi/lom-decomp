#!/usr/bin/env python3
"""Check that the game's memory layouts survive a 64-bit build that keeps PS1 storage.

A native build can run the game against PS1 memory: the retail image loaded
at its original addresses, with every stored pointer kept four bytes (the
lom-native port works this way). Every struct, union and global the game
keeps in memory must then have the same size and field offsets on the host as
on the PS1. This check proves it without building anything: it lays each
type out twice with libclang, once for the PS1 (mipsel) and once for x86-64
with the storage definitions in host_storage.h, and compares the two.

A type passes when its stored pointers use the storage typedefs of
include/ps1_storage.h (u8_ptr, <Type>Ptr, <Type>Slot, u_long), whose defaults
are plain C. Locals and parameters are not stored, so they are not checked.

What it reports:
- structs and unions whose size or field offsets differ, with the first
  field that differs;
- globals whose size differs;
- compile errors that only the storage definitions cause (a code slot
  called without PS1_CALL, a plain pointer assigned to a code slot). These
  always fail the check.

Only memory the port keeps in PS1 layout is checked: globals the C does not
define (the retail data) and the records they hold by value. A global the C
defines is the port's own object.

The differences left today are listed in baseline_<version>.txt, which CI
checks. The list may only shrink; --update-baseline rewrites it.

Usage (normally through `make storage-check`):
  storage_check.py --version us [--update-baseline] [--verbose] FILE...
"""
from __future__ import annotations

import argparse
import concurrent.futures
import pathlib
import sys

REPO = pathlib.Path(__file__).resolve().parents[2]
HERE = pathlib.Path(__file__).resolve().parent
sys.path.insert(0, str(REPO / "tools" / "data2c"))

import clang.cindex as ci  # noqa: E402
from declarations import clang_args  # noqa: E402

CursorKind = ci.CursorKind
RECORDS = (CursorKind.STRUCT_DECL, CursorKind.UNION_DECL)
HOST_TARGET = ["--target=x86_64-unknown-linux-gnu", "-fms-extensions", "-include", str(HERE / "host_storage.h")]


def parse_args_for(version: str, source: str, host) -> list[str]:
    """PS1 layout (host False), host with the storage definitions (True), or
    plain host ("plain"), whose errors are the host's own, not the storage's."""
    args = clang_args(version, [str(pathlib.Path(source).parent)])
    if host:
        target = HOST_TARGET if host is True else HOST_TARGET[:2]
        args = target + [a for a in args if not a.startswith("--target")]
    return args


def in_repo(cursor) -> str | None:
    """The repo-relative file a declaration is in, or None outside the repo."""
    f = cursor.location.file
    if f is None:
        return None
    path = pathlib.Path(f.name)
    if not path.is_absolute():
        path = REPO / path
    try:
        return str(path.resolve().relative_to(REPO))
    except ValueError:
        return None


def layout(t) -> tuple:
    """Size and every field's offset and size, nested records included."""
    t = t.get_canonical()
    fields = []
    for f in t.get_fields():
        offset = f.get_field_offsetof()
        ft = f.type.get_canonical()
        fields.append((f.spelling, offset, ft.get_size(), f.type.spelling))
    return t.get_size(), tuple(fields)


def strip_arrays(t):
    t = t.get_canonical()
    while t.kind in (ci.TypeKind.CONSTANTARRAY, ci.TypeKind.INCOMPLETEARRAY):
        t = t.element_type.get_canonical()
    return t


def record_key(decl, typedef_names):
    """(file, name) of a record definition, or None for an anonymous record."""
    path = in_repo(decl)
    if path is None:
        return None
    loc = (decl.location.file.name, decl.location.line, decl.location.column)
    name = typedef_names.get(loc) or decl.spelling
    if not name or name.startswith("("):
        return None
    # A view declared inside a function is named after it.
    if decl.semantic_parent.kind == CursorKind.FUNCTION_DECL:
        name = f"{decl.semantic_parent.spelling}.{name}"
    return path, name


def embedded(t, typedef_names, out: set):
    """The named records a record holds by value, through anonymous members too."""
    for f in t.get_canonical().get_fields():
        ft = strip_arrays(f.type)
        if ft.kind == ci.TypeKind.RECORD:
            key = record_key(ft.get_declaration(), typedef_names)
            if key:
                out.add(key)
            else:
                embedded(ft, typedef_names, out)
    return out


def collect(job):
    """Layouts of the records and globals one translation unit declares, and
    which records each of them holds by value."""
    version, source, host = job
    tu = ci.Index.create().parse(source, args=parse_args_for(version, source, host))
    typedef_names = {}  # record definition location -> the typedef that names it
    for c in tu.cursor.walk_preorder():
        if c.kind == CursorKind.TYPEDEF_DECL:
            decl = c.underlying_typedef_type.get_canonical().get_declaration()
            if decl.kind in RECORDS and decl.location.file:
                typedef_names.setdefault((decl.location.file.name, decl.location.line, decl.location.column), c.spelling)
    records, globals_ = {}, {}
    for c in tu.cursor.walk_preorder():
        path = in_repo(c)
        if path is None:
            continue
        if c.kind in RECORDS and c.is_definition() and c.semantic_parent.kind not in RECORDS:
            key = record_key(c, typedef_names)
            if key:
                records[key] = (layout(c.type), embedded(c.type, typedef_names, set()))
        elif c.kind == CursorKind.VAR_DECL and c.semantic_parent.kind == CursorKind.TRANSLATION_UNIT:
            t = strip_arrays(c.type)
            holds = set()
            if t.kind == ci.TypeKind.RECORD:
                key = record_key(t.get_declaration(), typedef_names)
                holds = {key} if key else embedded(t, typedef_names, set())
            # A tentative definition (no extern, no initializer) also defines it.
            defined = c.is_definition() or c.storage_class != ci.StorageClass.EXTERN
            globals_[(path, c.spelling)] = (t.get_size(), c.type.spelling, holds, defined)
    errors = set()
    for d in tu.diagnostics:
        if d.severity >= ci.Diagnostic.Error and d.location.file:
            path = in_repo(d)
            if path:
                errors.add(f"{path}:{d.location.line}: {d.spelling}")
    return records, globals_, errors


def first_difference(ps1, host) -> str:
    """Which field of a record moved or grew first."""
    for (name, off_a, size_a, spelling), (_, off_b, size_b, _) in zip(ps1[1], host[1]):
        if off_a != off_b or size_a != size_b:
            return f"{name or '(anonymous)'} ({spelling}): offset {off_a // 8:#x}->{off_b // 8:#x}, size {size_a}->{size_b}"
    return f"size {ps1[0]}->{host[0]}"


def run(version: str, files: list[str]):
    jobs = [(version, f, host) for f in files for host in (False, True, "plain")]
    ps1_records, ps1_globals, host_records, host_globals = {}, {}, {}, {}
    host_errors, plain_errors = set(), set()
    with concurrent.futures.ProcessPoolExecutor() as pool:
        for (_, _, host), (records, globals_, errors) in zip(jobs, pool.map(collect, jobs, chunksize=4)):
            if host == "plain":
                plain_errors.update(errors)
                continue
            (host_records if host else ps1_records).update(records)
            (host_globals if host else ps1_globals).update(globals_)
            if host:
                host_errors.update(errors)

    # A global the C defines is the port's own object; the rest stay in PS1
    # memory (the retail data a native port maps at its original addresses).
    defined = {name for (_, name), value in ps1_globals.items() if value[3]}
    in_ps1_memory = {key: value for key, value in ps1_globals.items() if key[1] not in defined}

    # Records in PS1 memory: held by such a global, directly or inside another record.
    stored = set()
    pending = [key for _, _, holds, _ in in_ps1_memory.values() for key in holds]
    while pending:
        key = pending.pop()
        if key not in stored:
            stored.add(key)
            pending.extend(ps1_records.get(key, (None, set()))[1])

    differences = {}
    for key in stored:
        ps1, host = ps1_records.get(key, (None,))[0], host_records.get(key, (None,))[0]
        if ps1 and host and (host[0] != ps1[0] or [f[1:3] for f in host[1]] != [f[1:3] for f in ps1[1]]):
            differences[f"{key[0]}: {key[1]}"] = first_difference(ps1, host)
    for key, (size, spelling, _, _) in in_ps1_memory.items():
        host = host_globals.get(key)
        if host is not None and host[0] != size:
            differences[f"{key[0]}: {key[1]}"] = f"global {spelling}: size {size}->{host[0]}"
    # Errors only the storage definitions cause, such as a code slot called without PS1_CALL.
    return len(stored), len(in_ps1_memory), differences, sorted(host_errors - plain_errors)


def read_baseline(path: pathlib.Path) -> set[str]:
    if not path.exists():
        return set()
    return {line.split("#")[0].strip() for line in path.read_text().split("\n")} - {""}


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--version", required=True)
    ap.add_argument("--update-baseline", action="store_true", help="rewrite the baseline from this run")
    ap.add_argument("--verbose", action="store_true", help="list every difference")
    ap.add_argument("files", nargs="+")
    args = ap.parse_args()

    records, globals_, differences, errors = run(args.version, sorted(set(args.files)))
    print(f"storage-check ({args.version}): {records} records and {globals_} globals in PS1 memory checked, "
          f"{len(differences)} lay out differently on a 64-bit host")
    if args.verbose:
        for key in sorted(differences):
            print(f"  {key}: {differences[key]}")

    for error in errors:
        print(f"  ERROR with the storage definitions: {error}")
    if errors:
        print(f"storage-check: {len(errors)} error(s) compiling with the storage definitions")
        sys.exit(1)

    baseline_path = HERE / f"baseline_{args.version}.txt"
    if args.update_baseline:
        lines = ["# Types and globals whose 64-bit layout still differs from the PS1 layout (make storage-check).",
                 "# This list may only shrink; see tools/storage_check/storage_check.py.", ""]
        lines += [f"{key}  # {differences[key]}" for key in sorted(differences)]
        baseline_path.write_text("\n".join(lines) + "\n")
        print(f"wrote {baseline_path.relative_to(REPO)} ({len(differences)} entries)")
        return
    baseline = read_baseline(baseline_path)
    new = sorted(set(differences) - baseline)
    fixed = sorted(baseline - set(differences))
    for key in fixed:
        print(f"  now matches, remove from the baseline: {key}")
    for key in new:
        print(f"  NEW DIFFERENCE: {key}: {differences[key]}")
    if new:
        print(f"storage-check: {len(new)} type(s) or global(s) no longer keep their PS1 layout")
        sys.exit(1)


if __name__ == "__main__":
    main()
