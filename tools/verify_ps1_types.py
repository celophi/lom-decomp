#!/usr/bin/env python3
"""Check PS1 storage layouts and native pointer operations using Clang.

Cross-target checks emit real object files without a target SDK. The Linux
runtime checks link two original game translation units and execute their
script consumers against mapped memory above 0x80000000.
"""

from __future__ import annotations

import argparse
from pathlib import Path
import platform
import shutil
import subprocess
import tempfile


ROOT = Path(__file__).resolve().parents[1]
TARGETS = (
    "x86_64-unknown-linux-gnu",
    "aarch64-unknown-linux-gnu",
    "x86_64-pc-windows-msvc",
    "aarch64-pc-windows-msvc",
    "x86_64-apple-macos11",
    "arm64-apple-macos11",
)


def run(command: list[str], *, source: str | None = None) -> subprocess.CompletedProcess[str]:
    """Run one compiler or test command, reporting its complete failure output."""
    result = subprocess.run(command, cwd=ROOT, input=source, capture_output=True, text=True)
    if result.returncode:
        raise RuntimeError(f"{' '.join(command)}\n{result.stdout}{result.stderr}")
    return result


def main() -> int:
    """Run all available checks and return a process status."""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--clang", default="clang", help="Clang executable")
    parser.add_argument("--no-runtime", action="store_true", help="Only run SDK-free cross-target checks")
    args = parser.parse_args()
    clang = shutil.which(args.clang)
    if clang is None:
        parser.error(f"Clang not found: {args.clang}")

    includes = ["-Iinclude", "-Iinclude/sdk", "-Isrc/overlays/field"]
    common = [clang, "-fms-extensions", "-DPS1_32BIT_STORAGE", "-DM2CTX", "-fno-builtin",
              "-Werror", "-Wno-deprecated-non-prototype", *includes]
    try:
        # Explicitly disable extensions: Windows Clang enables them by default.
        missing_flag = subprocess.run(
            [clang, "-fno-ms-extensions", "-DPS1_32BIT_STORAGE", "-Iinclude", "-x", "c", "-fsyntax-only", "-"],
            cwd=ROOT, input='#include "ps1_types.h"\n', text=True, capture_output=True,
        )
        if missing_flag.returncode == 0 or "requires -fms-extensions" not in missing_flag.stderr:
            raise RuntimeError("The missing -fms-extensions diagnostic was not produced")

        # PS1_CALL must refuse anything that isn't a code slot, such as a plain
        # function pointer, instead of calling it as if it were resolved.
        not_a_slot = subprocess.run(
            [clang, "-fms-extensions", "-DPS1_32BIT_STORAGE", "-Iinclude", "-x", "c", "-fsyntax-only", "-"],
            cwd=ROOT, text=True, capture_output=True,
            input='#include "ps1_types.h"\nvoid f(void (*handler)(void)) { PS1_CALL(handler)(); }\n',
        )
        if not_a_slot.returncode == 0:
            raise RuntimeError("PS1_CALL accepted a plain function pointer")

        with tempfile.TemporaryDirectory(prefix="lom-ps1-types-") as directory:
            work = Path(directory)
            # These callers pass the address of a stored cursor through the
            # rendering helpers. Both levels of the pointer must agree.
            run([*common, "-std=gnu89", "-Wno-pointer-sign", "-Wno-pointer-to-int-cast",
                 "-Wno-int-to-pointer-cast", "-Wno-int-conversion", "-fsyntax-only",
                 "src/overlays/field/field_scene_load.c", "src/overlays/field/field_scene_build.c"])
            print("PASS stored render-cursor callers and helpers", flush=True)
            for target in TARGETS:
                for version in ("US", "JP"):
                    # Actor tables have two deliberately distinct C views that
                    # cannot share one translation unit. Verify both separately.
                    for actor_tables in (False, True):
                        definitions = [f"-DVERSION_{version}"]
                        if actor_tables:
                            definitions.append("-DTEST_ACTOR_TABLES")
                        run([*common, "-std=gnu11", "-O2", f"--target={target}", *definitions,
                             "-c", "tools/tests/ps1_pointer_layout.c", "-o", str(work / "layout.o")])
                print(f"PASS layouts and code generation: {target} (US and JP)", flush=True)

            if args.no_runtime or platform.system() != "Linux":
                print("Runtime checks skipped; run on Linux without --no-runtime to exercise mapped RAM")
            else:
                for optimization in ("-O0", "-O2"):
                    executable = work / f"pointer-test{optimization}"
                    run([*common, "-std=gnu89", optimization, "-DVERSION_US",
                         "-ffunction-sections", "-fdata-sections", "-Wno-return-type",
                         "-Wno-pointer-to-int-cast", "-Wno-int-to-pointer-cast",
                         "tools/tests/ps1_pointer_ops.c", "tools/tests/ps1_pointer_runtime.c",
                         "src/overlays/field/field_script_operands.c", "src/akao_xa_stream.c",
                         "-Wl,--gc-sections", "-o", str(executable)])
                    result = run([str(executable)])
                    print(f"PASS runtime {optimization}: {result.stdout.strip()}", flush=True)
    except (OSError, RuntimeError) as error:
        parser.exit(1, f"{error}\n")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
