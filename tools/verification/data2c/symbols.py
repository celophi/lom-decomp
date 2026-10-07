"""Symbol names and addresses for one image (the executable or an overlay).

Addresses come from the same files the linker uses: the image's
`<image>_symbol_addrs.txt` and splat's `undefined_syms_auto.txt` and
`undefined_funcs_auto.txt`, which list every external name the image refers
to. Unnamed symbols carry their address in their name (func_8006FBDC,
D_800D9370), so they need no entry at all.
"""
from __future__ import annotations

import bisect
import collections
import pathlib
import re

# The PS1's main RAM as the CPU addresses it (KSEG0).
RAM_START = 0x80000000
RAM_END = 0x80200000

SYMBOL_LINE = re.compile(r"^\s*([A-Za-z_]\w*)\s*=\s*(0x[0-9A-Fa-f]+)\s*;(.*)$")
ADDRESS_NAME = re.compile(r"(func|D)_([0-9A-Fa-f]{8})")


class SymbolTable:
    def __init__(self):
        self.address = {}                         # name -> address
        self.functions = set()                    # addresses that start a function
        self.function_names = set()               # names declared as functions in C
        self.preferred = set()                    # names to define first at their address
        self._names_at = None                     # address -> names, built on first lookup

    # -- loading

    def load(self, path: pathlib.Path | str):
        """Read a `name = 0xADDR;` file. `// type:func` or undefined_funcs marks functions."""
        path = pathlib.Path(path)
        all_functions = "undefined_funcs" in path.name
        for line in path.read_text(errors="replace").split("\n"):
            m = SYMBOL_LINE.match(line)
            if not m:
                continue
            name, addr, comment = m.group(1), int(m.group(2), 16), m.group(3)
            if not RAM_START <= addr < RAM_END:
                continue
            self.address.setdefault(name, addr)
            if all_functions or "type:func" in comment or name.startswith("func_"):
                self.functions.add(addr)
        self._names_at = None

    def define(self, name: str, addr: int):
        """Add or override a name, e.g. a label from the data's own object file."""
        self.address[name] = addr
        self._names_at = None

    def add_function_names(self, names):
        """Names the C declares as functions; pointers to them are code pointers."""
        self.function_names |= set(names)
        self.functions |= {a for n, a in self.address.items() if n in self.function_names}

    # -- lookup

    def address_of(self, name: str) -> int | None:
        """A name's address; unnamed func_/D_ symbols carry it in the name."""
        if name in self.address:
            return self.address[name]
        m = ADDRESS_NAME.fullmatch(name)
        if not m:
            return None
        addr = int(m.group(2), 16)
        self.define(name, addr)
        if m.group(1) == "func":
            self.functions.add(addr)
        return addr

    def is_function(self, name: str) -> bool:
        return name in self.function_names or self.address.get(name) in self.functions

    def names_at(self, addr: int) -> list[str]:
        """All names at an address, the one to use first."""
        self._index()
        return self.order(self._names_at.get(addr, []))

    def order(self, names) -> list[str]:
        """Order names at one address: the data's own label first, then descriptive
        names before D_ names, so generated code reads like the decomp."""
        return sorted(set(names), key=lambda n: (n not in self.preferred, n.startswith("D_"), n))

    def resolve(self, addr: int, within: int = 0x10000):
        """(name, offset) for an address inside a symbol, or None if nothing is close."""
        self._index()
        i = bisect.bisect_right(self._sorted, addr) - 1
        if i < 0 or addr - self._sorted[i] >= within:
            return None
        base = self._sorted[i]
        return self.order(self._names_at[base])[0], addr - base

    def in_range(self, lo: int, hi: int) -> list[int]:
        """Addresses with at least one name, in [lo, hi), in order."""
        self._index()
        return [a for a in self._sorted if lo <= a < hi]

    def _index(self):
        if self._names_at is None:
            self._names_at = collections.defaultdict(list)
            for n, a in self.address.items():
                self._names_at[a].append(n)
            self._sorted = sorted(self._names_at)
