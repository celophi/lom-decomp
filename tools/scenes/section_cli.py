"""Command-line output shared by the scene section readers."""

from __future__ import annotations

import argparse
from pathlib import Path
import sys
from typing import Callable

from tools.scenes.scene_format import SceneHeader
from tools.scenes.presentation import dump_yaml


def run(reader: Callable[[bytes, SceneHeader], dict], description: str,
        argv: list[str] | None = None, *, kind: str,
        text_renderer: Callable[[dict], str] | None = None, text_encoding: bool = False) -> int:
    """Read a whole scene and print one section, or write a new inspection file."""
    parser = argparse.ArgumentParser(description=description)
    parser.add_argument("source", type=Path, help="original scene IMG")
    parser.add_argument("-o", "--output", type=Path, help="new output file (default: standard output)")
    if text_renderer is not None:
        parser.add_argument("--format", choices=("yaml", "text"), default="yaml",
                            help="output format (default: yaml)")
    if text_encoding:
        parser.add_argument("--text-encoding", choices=("us", "jp"), default="us",
                            help="text control/glyph dialect; no external files (default: us)")
    args = parser.parse_args(argv)
    try:
        data = args.source.read_bytes()
        options = {"text_encoding": args.text_encoding} if text_encoding else {}
        document = reader(data, SceneHeader.parse(data), **options)
        if text_renderer is not None and args.format == "text":
            text = text_renderer(document)
        else:
            text = dump_yaml(document, kind)
        if args.output is None:
            sys.stdout.write(text)
        else:
            with args.output.open("x", encoding="ascii") as output:
                output.write(text)
    except (OSError, ValueError) as error:
        print(f"error: {error}", file=sys.stderr)
        return 1
    return 0
