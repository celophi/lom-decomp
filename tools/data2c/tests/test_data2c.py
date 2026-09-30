#!/usr/bin/env python3
"""Unit tests for tools/data2c, on small made-up data (never game data).

Each case declares a type in a scratch C file, pairs it with bytes and
symbols, generates C, and checks the C round-trips through verify.py (Clang
for mipsel). The GCC 2.8 side is covered by `make verify-data-as-c`.

Run: python3 -m unittest discover -s tools/data2c/tests
Needs clang and the libclang Python bindings.
"""
from __future__ import annotations

import contextlib
import io
import pathlib
import shutil
import struct
import subprocess
import sys
import tempfile
import unittest

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parents[1]))

from declarations import Declarations, clang_args  # noqa: E402
from region import DataRegion, load_assembled, load_databin  # noqa: E402
from report import Report  # noqa: E402
from symbols import SymbolTable  # noqa: E402
from verify import verify  # noqa: E402
from writer import RegionWriter  # noqa: E402

HAVE_CLANG = shutil.which("clang") is not None
BASE = 0x80100000
TYPES = ("typedef signed char s8; typedef unsigned char u8; typedef short s16; typedef unsigned short u16;\n"
         "typedef int s32; typedef unsigned int u32;\n")


def words(*values) -> bytes:
    return b"".join(struct.pack("<I", v & 0xFFFFFFFF) for v in values)


class Case:
    """One generated region: its C text, report and round-trip result."""

    def __init__(self, c_decls: str, data: bytes, names: dict, relocations: dict | None = None,
                 functions=(), start: int = BASE):
        self.tmp = tempfile.TemporaryDirectory()
        tmp = pathlib.Path(self.tmp.name)
        src = tmp / "decls.c"
        src.write_text(TYPES + c_decls)

        self.symbols = SymbolTable()
        for name, addr in names.items():
            self.symbols.define(name, addr)
        self.symbols.functions |= {names[f] for f in functions}

        relocations = relocations or {}
        linked = bytearray(data)
        for addr, (target, addend) in relocations.items():
            struct.pack_into("<I", linked, addr - start, names[target] + addend)
        self.region = DataRegion("test", start, data, bytes(linked), relocations, from_object=bool(relocations))

        wanted = {n for a in self.symbols.in_range(self.region.start, self.region.end)
                  for n in self.symbols.names_at(a)}
        decls = Declarations([src], wanted, clang_args("us", []))
        self.symbols.add_function_names(decls.function_names)
        self.report = Report("test")
        self.text = RegionWriter(self.region, self.symbols, decls, self.report).generate()
        self.out = tmp / "out.c"
        self.out.write_text(self.text)

    def round_trips(self) -> bool:
        with contextlib.redirect_stdout(io.StringIO()) as log:
            ok = verify(self.out, self.region, self.symbols)
        self.log = log.getvalue()
        return ok

    def close(self):
        self.tmp.cleanup()


@unittest.skipUnless(HAVE_CLANG, "needs clang")
class GenerationTests(unittest.TestCase):
    def generate(self, *args, **kwargs) -> Case:
        case = Case(*args, **kwargs)
        self.addCleanup(case.close)
        self.assertTrue(case.round_trips(), getattr(case, "log", "") + case.text)
        return case

    def test_data_pointer_is_symbolic(self):
        case = self.generate("typedef struct { s32 a; void *p; } T; extern T g_t; extern s32 g_target;",
                             words(5, BASE + 8, 7), {"g_t": BASE, "g_target": BASE + 8})
        self.assertIn("(void *)&g_target", case.text)
        self.assertEqual(case.report.counts["pointer to data"], 1)

    def test_function_pointer_is_symbolic(self):
        case = self.generate("typedef void (*Fn)(void); void handler(void); extern Fn g_fns[1];",
                             words(0x80010000), {"g_fns": BASE, "handler": 0x80010000})
        self.assertIn("(void (*)())handler", case.text)
        self.assertIn("extern void handler();", case.text)

    def test_nonzero_padding_is_kept(self):
        data = bytes([1, 0x55, 0, 0]) + words(9)
        case = self.generate("typedef struct { u8 a; s32 b; } T; extern T g_t;", data, {"g_t": BASE})
        self.assertIn("d2c_pad_01[3]", case.text)
        self.assertEqual(case.report.counts["nonzero padding kept as bytes"], 1)

    def test_union_initializes_its_widest_member(self):
        case = self.generate("typedef union { u8 small; s32 big; } U; extern U g_u;",
                             words(0x12345678), {"g_u": BASE})
        self.assertIn("0x12345678", case.text)

    def test_bitfield_crossing_a_word_boundary(self):
        # Packed, so b starts at bit 28 and runs into the next word.
        a, b = 0xABCDEF1, 0xB5
        data = struct.pack("<Q", a | (b << 28))[:5] + bytes(3)
        case = self.generate("typedef struct { u32 a : 28; u32 b : 8; } __attribute__((packed)) T; extern T g_t;",
                             data, {"g_t": BASE})
        self.assertIn(f"{{ {a}, {b} }}", case.text)       # bitfields are written in decimal

    def test_packed_struct_stays_packed(self):
        case = self.generate("typedef struct { u8 a; s32 b; } __attribute__((packed)) T; extern T g_t;",
                             bytes([1]) + words(2) + bytes(3), {"g_t": BASE})
        self.assertIn("} __attribute__((packed)) D2C_S000;", case.text)

    def test_incomplete_array_takes_the_symbol_extent(self):
        case = self.generate("extern s16 g_arr[]; extern s16 g_next;",
                             struct.pack("<4h", 1, 2, 3, 4), {"g_arr": BASE, "g_next": BASE + 6})
        self.assertIn("s16 g_arr[3]", case.text)

    def test_address_in_an_integer_field_survives(self):
        case = self.generate("extern u32 g_table[2]; extern s32 g_target;",
                             words(0, 0) + words(3), {"g_table": BASE, "g_target": BASE + 8},
                             relocations={BASE: ("g_target", 0)})
        self.assertIn("(u32)(void *)&g_target", case.text)
        self.assertEqual(case.report.counts["integer field holding an address"], 1)

    def test_address_in_undeclared_bytes_survives(self):
        case = self.generate("extern s32 g_target;", words(0, 5) + words(3),
                             {"D_80100000": BASE, "g_target": BASE + 8},
                             relocations={BASE + 4: ("g_target", 0)})
        self.assertIn("u32 D_80100000[2]", case.text)

    def test_unaligned_array_joins_a_packed_cluster(self):
        # GCC would word-align g_b, so it has to share one struct with g_a.
        case = self.generate("extern u8 g_a[2]; extern u8 g_b[2];", bytes([1, 2, 3, 4]),
                             {"g_a": BASE, "g_b": BASE + 2})
        self.assertIn("struct d2c_cluster_80100000_t", case.text)
        self.assertIn("#define g_b (d2c_cluster_80100000.m_g_b)", case.text)
        self.assertIn("g_b = d2c_cluster_80100000 + 0x2", case.text)

    def test_second_name_at_an_address_becomes_an_alias(self):
        case = self.generate("extern s32 g_value;", words(4), {"g_value": BASE, "D_80100000": BASE})
        self.assertIn("D_80100000 = g_value", case.text)

    def test_output_is_deterministic(self):
        args = ("typedef struct { s32 a; void *p; } T; extern T g_t; extern s32 g_target;",
                words(5, BASE + 8, 7), {"g_t": BASE, "g_target": BASE + 8})
        first, second = Case(*args), Case(*args)
        self.addCleanup(first.close)
        self.addCleanup(second.close)
        self.assertEqual(first.text, second.text)


class SymbolTableTests(unittest.TestCase):
    def test_symbol_file_lines(self):
        with tempfile.TemporaryDirectory() as tmp:
            path = pathlib.Path(tmp) / "x_symbol_addrs.txt"
            path.write_text("a = 0x80010000; // type:func\nb = 0x80020000;\n"
                            "scratch = 0x1F800000;\nfunc_80030000 = 0x80030000;\n")
            symbols = SymbolTable()
            symbols.load(path)
        self.assertEqual(symbols.address["b"], 0x80020000)
        self.assertNotIn("scratch", symbols.address)         # outside main RAM
        self.assertTrue({0x80010000, 0x80030000} <= symbols.functions)
        self.assertNotIn(0x80020000, symbols.functions)

    def test_unnamed_symbols_carry_their_address(self):
        symbols = SymbolTable()
        self.assertEqual(symbols.address_of("D_80040000"), 0x80040000)
        self.assertEqual(symbols.address_of("func_80050000"), 0x80050000)
        self.assertIn(0x80050000, symbols.functions)
        self.assertIsNone(symbols.address_of("g_unknown"))

    def test_resolve_and_name_order(self):
        symbols = SymbolTable()
        symbols.define("D_80100000", BASE)
        symbols.define("g_table", BASE)
        self.assertEqual(symbols.names_at(BASE), ["g_table", "D_80100000"])
        self.assertEqual(symbols.resolve(BASE + 0x10), ("g_table", 0x10))
        self.assertIsNone(symbols.resolve(BASE - 4))


class RegionTests(unittest.TestCase):
    def test_databin_is_placed_by_its_label(self):
        with tempfile.TemporaryDirectory() as tmp:
            repo = pathlib.Path(tmp)
            (repo / "blob.bin").write_bytes(b"\x01\x02\x03\x04")
            (repo / "x.s").write_text('.section .data, "wa"\ndlabel g_blob\n.incbin "blob.bin"\nenddlabel g_blob\n')
            symbols = SymbolTable()
            symbols.define("g_blob", BASE)
            region = load_databin(repo, "x.s", symbols)
        self.assertEqual((region.start, region.data, region.from_object), (BASE, b"\x01\x02\x03\x04", False))
        self.assertIn("g_blob", symbols.preferred)

    @unittest.skipUnless(HAVE_CLANG, "needs clang")
    def test_assembled_data_gives_relocations(self):
        with tempfile.TemporaryDirectory() as tmp:
            repo = pathlib.Path(tmp)
            (repo / "x.c").write_text("extern int target; int *g_p = &target; int g_v = 3;\n")
            subprocess.run(["clang", "--target=mipsel-unknown-linux-gnu", "-fno-common", "-c", str(repo / "x.c"),
                            "-o", str(repo / "x.o")], check=True)
            (repo / "x.s").write_text("")          # no address comment: the labels place it
            symbols = SymbolTable()
            symbols.define("g_p", BASE)
            symbols.define("target", 0x80020000)
            region = load_assembled(repo, "x.o", "x.s", symbols)
        self.assertEqual(region.start, BASE)
        self.assertEqual(region.relocations, {BASE: ("target", 0)})
        self.assertEqual(region.linked[:4], struct.pack("<I", 0x80020000))
        self.assertEqual(symbols.address["g_v"], BASE + 4)

    @unittest.skipUnless(HAVE_CLANG, "needs clang")
    def test_labels_that_disagree_are_an_error(self):
        with tempfile.TemporaryDirectory() as tmp:
            repo = pathlib.Path(tmp)
            (repo / "x.c").write_text("int g_a = 1; int g_b = 2;\n")
            subprocess.run(["clang", "--target=mipsel-unknown-linux-gnu", "-c", str(repo / "x.c"),
                            "-o", str(repo / "x.o")], check=True)
            (repo / "x.s").write_text("")
            symbols = SymbolTable()
            symbols.define("g_a", BASE)
            symbols.define("g_b", BASE + 0x100)
            with self.assertRaises(SystemExit):
                load_assembled(repo, "x.o", "x.s", symbols)

    @unittest.skipUnless(HAVE_CLANG, "needs clang")
    def test_address_comment_is_the_fallback(self):
        with tempfile.TemporaryDirectory() as tmp:
            repo = pathlib.Path(tmp)
            (repo / "x.c").write_text("int g_unplaced = 1;\n")
            subprocess.run(["clang", "--target=mipsel-unknown-linux-gnu", "-c", str(repo / "x.c"),
                            "-o", str(repo / "x.o")], check=True)
            (repo / "x.s").write_text("dlabel g_unplaced\n    /* 74919 800C4588 01000000 */ .word 0x00000001\n")
            region = load_assembled(repo, "x.o", "x.s", SymbolTable())
        self.assertEqual(region.start, 0x800C4588)


if __name__ == "__main__":
    unittest.main()
