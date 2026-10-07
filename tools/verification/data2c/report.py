"""What data2c noticed while typing a region.

Counts say how much of each thing happened; notes keep a few examples. Most
entries are not errors for the PS1 build, which stays byte-exact regardless,
but they are the places where the decomp's declarations are not yet good
enough for a build with wider pointers.
"""
from __future__ import annotations

import collections
import json
import pathlib

EXAMPLES_KEPT = 8


class Report:
    def __init__(self, source: str):
        self.source = source
        self.counts = collections.Counter()
        self.examples = collections.defaultdict(list)

    def count(self, what: str, n: int = 1):
        self.counts[what] += n

    def note(self, what: str, example: str):
        """Count one occurrence and keep it as an example."""
        self.counts[what] += 1
        if len(self.examples[what]) < EXAMPLES_KEPT:
            self.examples[what].append(example)

    def summary(self, what: str, total: int, examples):
        """Record a total with ready-made examples."""
        self.counts[what] += total
        self.examples[what].extend(list(examples)[:EXAMPLES_KEPT])

    def print(self, header: str):
        print(header)
        for what, n in self.counts.most_common():
            print(f"  {n:7}  {what}")
        for what, examples in self.examples.items():
            print(f"  e.g. {what}: " + "; ".join(examples[:4]))

    def save(self, path: pathlib.Path, **extra):
        path.write_text(json.dumps({"source": self.source, **extra, "counts": dict(self.counts),
                                    "examples": dict(self.examples)}, indent=1))
