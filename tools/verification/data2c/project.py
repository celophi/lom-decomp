"""Where this project keeps each image's inputs.

An image is the executable ("slus") or an overlay ("field", "wmap", ...). The
make rules and report_all.py both go through these functions, so the layout
is described once.
"""
from __future__ import annotations

import pathlib

REPO = pathlib.Path(__file__).resolve().parents[3]
EXECUTABLE = "slus"


def symbol_files(version: str, image: str) -> list[str]:
    """The image's own symbols, then splat's lists of every name it refers to."""
    symbols = REPO / f"config/{version}/symbols"
    if image == EXECUTABLE:
        own = [symbols / "shared_symbol_addrs.txt"] + sorted(symbols.glob("slus_*_symbol_addrs.txt"))
        linker = REPO / f"linker/{version}"
    else:
        own = [symbols / f"{image}_symbol_addrs.txt"]
        linker = REPO / f"linker/{version}/overlays/{image}"
    files = [p for p in own if p.exists()]
    files += [linker / "undefined_syms_auto.txt", linker / "undefined_funcs_auto.txt"]
    return [str(p.relative_to(REPO)) for p in files]


def source_globs(image: str) -> list[str]:
    """The C files whose declarations type the image's data."""
    if image == EXECUTABLE:
        return ["src/main/*.c", "src/main/audio/*.c", "src/psyq/*/*.c"]
    return [f"src/overlays/{image}/**/*.c"]


def include_dirs(image: str) -> list[str]:
    return ["src/main/internal", "src/main/audio/internal"] if image == EXECUTABLE else [f"src/overlays/{image}/internal"]


def data_files(version: str):
    """(image, data .s, assembled object) for every splat data file with a .data section.

    The object path is where a normal build writes it; report_all.py reads it
    back for data assembly.
    """
    asm_root = REPO / f"asm/{version}"
    for asm in sorted(asm_root.rglob("data/*.s")):
        if not any(line.startswith(".section .data") for line in asm.read_text(errors="replace").split("\n")):
            continue
        rel = asm.relative_to(asm_root)
        if rel.parts[0] == "overlays":
            image = rel.parts[1]
            obj = REPO / f"build/{version}/overlays/{image}/asm/{version}" / rel.with_suffix(".o")
        else:
            image = EXECUTABLE
            obj = REPO / f"build/{version}/asm/{version}" / rel.with_suffix(".o")
        yield image, str(asm.relative_to(REPO)), obj
