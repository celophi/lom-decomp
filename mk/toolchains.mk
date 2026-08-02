# ============================================================================
# Toolchains
# ============================================================================

# ─── Toolchain ──────────────────────────────────────────────────────────────────
#
# CC       — GCC 2.8.0 cross-compiler for PSX (installed in /opt/psx-gcc-2.8.0/)
# LD       — modern mipsel linker (binutils from apt)
# OBJCOPY  — converts ELF ↔ raw binary

CROSS        	:= mipsel-linux-gnu-
CC           	:= /opt/psx-gcc-2.8.0/gcc -B/opt/psx-gcc-2.8.0/
CC_CDK       	:= /opt/psx-gcc-2.7.2-cdk/gcc -B/opt/psx-gcc-2.7.2-cdk/
CC_GNU       	:= /opt/psx-gcc-2.7.2-gnuas/gcc -B/opt/psx-gcc-2.7.2-gnuas/
CC_260_PSX		:= /opt/psx-gcc-2.6.0/gcc -B/opt/psx-gcc-2.6.0/
AS_GNU       	:= /opt/psx-gcc-2.7.2-gnuas/as
LD           	:= $(CROSS)ld
OBJCOPY      	:= $(CROSS)objcopy


# ─── Compiler & Assembler Flags ─────────────────────────────────────────────────
#
# -O2            Optimization level that matches the original compiler output.
# -G0 / -G4     Controls the "small data" threshold. -G0 means nothing goes in
#                the $gp-relative section; -G4 allows data ≤4 bytes to use $gp.
#                Most files use -G0, but cdrom.c needs -G4 to match the original.
# -g             Emit debug info (doesn't affect code generation on this GCC).
# -fsigned-char  Treat bare 'char' as signed (PSX SDK convention).
#
# MASPSX_AS_FLAGS:
#   --run-assembler     Have maspsx invoke the system assembler directly.
#   -no-pad-sections    Don't pad sections to 16-byte alignment (matches ASPSX).
#   --aspsx-version     Target ASPSX 2.77 behavior for asm translation.
#   --macro-inc         Enable ASPSX directive macros (nonmatching, dlabel, etc.)
#                       Only used for hand-written .s files, NOT for GCC output.

CFLAGS_G0       := -O2 -G0 -gcoff -fsigned-char -fno-builtin
CFLAGS_G4       := -O2 -G4 -gcoff -fsigned-char

# CDK (GCC 2.7.2-970404) flags: no -g/-fsigned-char; uses -msoft-float and COFF debug.
CFLAGS_CDK_G0   := -O2 -G0 -msoft-float -gcoff

# GCC 2.6.0-psx flags.
CFLAGS_260_G0   := -O2 -G0 -gcoff -msoft-float
MASPSX_AS_FLAGS_260 := -no-pad-sections --aspsx-version=2.34 --expand-div

# PSX GNU GCC 2.7.2 flags: compiles to .s, then assembled with its own 'as'.
CFLAGS_GNU_G0   := -O2 -G0
AS_GNU_FLAGS    := -O -EL

INCLUDE_FLAGS   := -Iinclude -Iinclude/psyq

MASPSX_AS       	:= python3 tools/maspsx/maspsx.py --run-assembler
MASPSX_AS_FLAGS 	:= -no-pad-sections --aspsx-version=2.77 --expand-div
# GCC 2.8.0 -G4: whether maspsx expands `div $reg,...` into the div-by-zero /
# overflow break-check sequence is per-file. Most G4 objects (cdrom.c,
# overlay_memory.c, decomp4.c, controller.c) need --expand-div; some have bare
# `div $zero,...` with no checks and must omit it. MASPSX_DIV_FLAG_G4 defaults
# to --expand-div and is overridden to empty per-object for the exceptions via
# a target-specific variable (see SRCS_G4_NOEXPAND / *_gcc_g4_noexpand_srcs).
MASPSX_DIV_FLAG_G4  := --expand-div
MASPSX_AS_FLAGS_G4   = -no-pad-sections --aspsx-version=2.77 $(MASPSX_DIV_FLAG_G4)
MASPSX_AS_FLAGS_CDK := -no-pad-sections --aspsx-version=2.67 --expand-div
MASPSX_PP       	:= python3 tools/maspsx/maspsx.py
MASPSX_PP_FLAGS 	:= --macro-inc
