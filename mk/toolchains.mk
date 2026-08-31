# ============================================================================
# Toolchains
# ============================================================================

# Naming convention:
#   - Standard, unqualified Make names select the default GCC 2.8.0 toolchain.
#   - Alternate toolchains use a <version>_<flavor> suffix.
#   - Flag variables begin with the tool that consumes them.
#
# Compiler commands include -B so each GCC finds its matching internal tools.
CC             := /opt/psx-gcc-2.8.0/gcc -B/opt/psx-gcc-2.8.0/
CC_272_CDK     := /opt/psx-gcc-2.7.2-cdk/gcc -B/opt/psx-gcc-2.7.2-cdk/
CC_272_GNU     := /opt/psx-gcc-2.7.2-gnuas/gcc -B/opt/psx-gcc-2.7.2-gnuas/
CC_260         := /opt/psx-gcc-2.6.0/gcc -B/opt/psx-gcc-2.6.0/
AS_272_GNU     := /opt/psx-gcc-2.7.2-gnuas/as

# Modern GNU binutils link, convert, and inspect the generated objects.
CROSS_COMPILE  := mipsel-linux-gnu-
LD             := $(CROSS_COMPILE)ld
OBJCOPY        := $(CROSS_COMPILE)objcopy
OBJDUMP        := $(CROSS_COMPILE)objdump

# Shared header search paths for compilers, assemblers, and maspsx.
INCLUDE_FLAGS := -Iinclude -Iinclude/sdk

# GCC 2.8.0 is the default compiler. Sources are routed to G0 or G4 in the
# source lists; G controls the maximum size of gp-relative data.
CFLAGS_G0 := -O2 -G0 -gcoff -fsigned-char -fno-builtin
CFLAGS_G4 := -O2 -G4 -gcoff -fsigned-char

# Alternate compiler and assembler flags.
# The GCC 2.7.2 CDK G0 pipeline optionally disables individual optimization
# passes for source files whose target code requires it. These variables are
# empty by default and receive target-specific assignments from mk/overlays.mk.
# Recursive assignment keeps the target-specific values visible here.
CFLAGS_272_CDK_SCHED_FLAG :=
CFLAGS_272_CDK_STRENGTH_FLAG :=
CFLAGS_272_CDK_G0 = -O2 -G0 -msoft-float -gcoff $(CFLAGS_272_CDK_SCHED_FLAG) $(CFLAGS_272_CDK_STRENGTH_FLAG)
CFLAGS_272_GNU_G0 := -O2 -G0
ASFLAGS_272_GNU    := -O -EL
CFLAGS_260_G0      := -O2 -G0 -gcoff -msoft-float
# Same GCC 2.6.0 pipeline as CFLAGS_260_G0 but at -O1. field_runtime_glyph.c's
# two glyph helpers only match at -O1; the rest of the 260 list stays at -O2.
CFLAGS_260_G0_O1   := -O1 -G0 -gcoff -msoft-float

# maspsx preprocesses assembly syntax and can invoke GNU as directly.
MASPSX          := python3 tools/maspsx/maspsx.py
MASPSX_AS       := $(MASPSX) --run-assembler
# Inject include/macro.inc for splat-generated assembly directives.
MASPSX_PP_FLAGS := --macro-inc

# Each maspsx-backed compiler family must emulate its original ASPSX version.
MASPSX_FLAGS         := -no-pad-sections --aspsx-version=2.77 --expand-div
MASPSX_FLAGS_260     := -no-pad-sections --aspsx-version=2.34 --expand-div

# GCC 2.7.2 CDK div expansion varies by source, like the G4 case below. The
# flag defaults to --expand-div and is cleared with a target-specific
# assignment for the no-expand subset. Recursive assignment keeps that value
# visible here, so the CDK C rule must reference this with $$(...) (deferred).
MASPSX_CDK_DIV_FLAG := --expand-div
MASPSX_FLAGS_272_CDK = -no-pad-sections --aspsx-version=2.67 $(MASPSX_CDK_DIV_FLAG)

# GCC 2.8.0 G4 division expansion varies by source. This flag is cleared with
# a target-specific assignment for objects whose original code used bare div.
# Recursive assignment keeps that target-specific value visible here.
MASPSX_DIV_FLAG_G4 := --expand-div
MASPSX_FLAGS_G4 = -no-pad-sections --aspsx-version=2.77 $(MASPSX_DIV_FLAG_G4)
