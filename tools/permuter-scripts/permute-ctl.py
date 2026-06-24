#!/usr/bin/env python3
"""
permute-ctl.py — start/stop/query decomp-permuter sessions.

Usage (run from /staging):
    python3 tools/permuter-scripts/permute-ctl.py start  <dir> [-j N] [--best-only]
    python3 tools/permuter-scripts/permute-ctl.py stop   <dir>
    python3 tools/permuter-scripts/permute-ctl.py status <dir>
    python3 tools/permuter-scripts/permute-ctl.py best   <dir>
    python3 tools/permuter-scripts/permute-ctl.py clean  <dir>
"""

import argparse
import os
import re
import signal
import subprocess
import sys
from pathlib import Path

PERMUTER_PY = "tools/decomp-permuter/permuter.py"
PID_FILE = "permuter.pid"
LOG_FILE = "permuter.log"
SCORE_RE = re.compile(r"\] found new best score! \((\d+) vs")


def pid_path(pdir: Path) -> Path:
    return pdir / PID_FILE


def log_path(pdir: Path) -> Path:
    return pdir / LOG_FILE


def read_pid(pdir: Path) -> int | None:
    p = pid_path(pdir)
    if not p.exists():
        return None
    try:
        return int(p.read_text().strip())
    except ValueError:
        return None


def is_running(pid: int) -> bool:
    try:
        os.kill(pid, 0)
        return True
    except ProcessLookupError:
        return False
    except PermissionError:
        return True  # exists but not ours to signal


def best_score_from_log(pdir: Path) -> int | None:
    lp = log_path(pdir)
    if not lp.exists():
        return None
    best = None
    for line in lp.read_text(errors="replace").splitlines():
        m = SCORE_RE.search(line)
        if m:
            score = int(m.group(1))
            if best is None or score < best:
                best = score
    return best


def best_output_dir(pdir: Path) -> Path | None:
    candidates = sorted(pdir.glob("output-*-*"))
    if not candidates:
        return None
    # Sort by score (first number after "output-")
    def score_key(p: Path) -> int:
        try:
            return int(p.name.split("-")[1])
        except (IndexError, ValueError):
            return 999999
    return min(candidates, key=score_key)


# ── Commands ─────────────────────────────────────────────────────────────────

def cmd_start(pdir: Path, jobs: int, best_only: bool) -> None:
    pid = read_pid(pdir)
    if pid and is_running(pid):
        print(f"Already running (pid {pid}).")
        return

    log = log_path(pdir)
    log_fd = log.open("a")

    cmd = [
        "python3", PERMUTER_PY,
        f"-j{jobs}",
        str(pdir),
    ]
    if best_only:
        cmd.insert(-1, "--best-only")

    proc = subprocess.Popen(
        cmd,
        stdout=log_fd,
        stderr=log_fd,
        start_new_session=True,
    )
    log_fd.close()

    pid_path(pdir).write_text(str(proc.pid))
    print(f"Started permuter (pid {proc.pid}), logging to {log}")
    print(f"Watch: tail -f {log}")


def cmd_stop(pdir: Path) -> None:
    pid = read_pid(pdir)
    if pid is None:
        print("No PID file found — permuter may not be running.")
        return
    if not is_running(pid):
        print(f"Process {pid} is not running. Cleaning up PID file.")
        pid_path(pdir).unlink(missing_ok=True)
        return

    os.kill(pid, signal.SIGTERM)
    print(f"Sent SIGTERM to pid {pid}.")
    pid_path(pdir).unlink(missing_ok=True)


def cmd_status(pdir: Path) -> None:
    pid = read_pid(pdir)
    if pid and is_running(pid):
        print(f"Running  (pid {pid})")
    elif pid:
        print(f"Stopped  (stale PID {pid})")
        pid_path(pdir).unlink(missing_ok=True)
    else:
        print("Stopped  (no PID file)")

    best = best_score_from_log(pdir)
    if best is not None:
        print(f"Best score so far: {best}")

    lp = log_path(pdir)
    if lp.exists():
        lines = lp.read_text(errors="replace").splitlines()
        tail = lines[-20:] if len(lines) > 20 else lines
        print(f"\n--- last {len(tail)} lines of {lp} ---")
        print("\n".join(tail))


def cmd_best(pdir: Path) -> None:
    best_dir = best_output_dir(pdir)
    if best_dir is None:
        print("No output candidates found yet.")
        return

    score_part = best_dir.name.split("-")[1] if "-" in best_dir.name else "?"
    print(f"Best candidate: {best_dir}  (score {score_part})\n")

    c_file = best_dir / "source.c"
    if not c_file.exists():
        # Some permuter versions write the source directly in the output dir
        candidates = list(best_dir.glob("*.c"))
        c_file = candidates[0] if candidates else None

    if c_file and c_file.exists():
        print(c_file.read_text())
    else:
        print(f"(No .c file found in {best_dir})")


def cmd_clean(pdir: Path) -> None:
    pid = read_pid(pdir)
    if pid and is_running(pid):
        os.kill(pid, signal.SIGTERM)
        print(f"Stopped permuter (pid {pid}).")
        pid_path(pdir).unlink(missing_ok=True)

    removed = 0
    for candidate in pdir.glob("output-*"):
        if candidate.is_dir():
            import shutil
            shutil.rmtree(candidate)
            removed += 1

    log_path(pdir).unlink(missing_ok=True)
    print(f"Removed {removed} candidate director{'y' if removed == 1 else 'ies'} and log.")
    print(f"Kept: base.c, target.o, compile.sh, settings.toml")


# ── Entry point ───────────────────────────────────────────────────────────────

def main() -> None:
    parser = argparse.ArgumentParser(
        description="Control a decomp-permuter session.",
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    sub = parser.add_subparsers(dest="command", required=True)

    p_start = sub.add_parser("start", help="Start the permuter in the background")
    p_start.add_argument("dir", help="Permuter directory (e.g. /permute/cdrom_complete_command)")
    p_start.add_argument("-j", type=int, default=os.cpu_count() or 4, metavar="N",
                         help="Number of parallel workers (default: nproc)")
    p_start.add_argument("--best-only", action="store_true",
                         help="Only write candidates that beat the current best score")

    p_stop = sub.add_parser("stop", help="Stop the permuter")
    p_stop.add_argument("dir")

    p_status = sub.add_parser("status", help="Show permuter status and recent log")
    p_status.add_argument("dir")

    p_best = sub.add_parser("best", help="Print the best candidate source found so far")
    p_best.add_argument("dir")

    p_clean = sub.add_parser("clean", help="Stop permuter and remove all candidate output")
    p_clean.add_argument("dir")

    args = parser.parse_args()
    pdir = Path(args.dir).resolve()

    if not pdir.exists():
        print(f"ERROR: directory not found: {pdir}", file=sys.stderr)
        sys.exit(1)

    if args.command == "start":
        cmd_start(pdir, args.j, args.best_only)
    elif args.command == "stop":
        cmd_stop(pdir)
    elif args.command == "status":
        cmd_status(pdir)
    elif args.command == "best":
        cmd_best(pdir)
    elif args.command == "clean":
        cmd_clean(pdir)


if __name__ == "__main__":
    main()
