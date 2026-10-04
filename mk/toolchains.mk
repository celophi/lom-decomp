# Toolchains

# CC and the unqualified flags use GCC 2.8.0. Other compilers have their
# version and flavor in the variable name. -B tells each GCC where to find
# its own internal tools; mixing these breaks the original code generation.
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
# source lists; G controls the maximum size of gp-relative data. Builtins stay
# enabled: code that calls abs() relies on the inline expansion, and every
# other source builds identically with or without -fno-builtin.
CFLAGS_G0 := -O2 -G0 -gcoff -fsigned-char
CFLAGS_G0_O0 := -O0 -G0 -gcoff -fsigned-char
CFLAGS_G4 := -O2 -G4 -gcoff -fsigned-char

# Alternate compiler and assembler flags.
CFLAGS_272_CDK_G0  := -O2 -G0 -msoft-float -gcoff
CFLAGS_272_GNU_G0  := -O2 -G0
ASFLAGS_272_GNU    := -O -EL
CFLAGS_260_G0      := -O2 -G0 -gcoff -msoft-float

# maspsx preprocesses assembly syntax and can invoke GNU as directly.
MASPSX          := python3 tools/maspsx/maspsx.py
MASPSX_AS       := $(MASPSX) --run-assembler
# Inject include/macro.inc for splat-generated assembly directives.
MASPSX_PP_FLAGS := --macro-inc

# These ASPSX versions affect instruction expansion and must match the
# assembler originally used with each compiler.
MASPSX_FLAGS         := -no-pad-sections --aspsx-version=2.77 --expand-div
MASPSX_FLAGS_272_CDK := -no-pad-sections --aspsx-version=2.67 --expand-div
MASPSX_FLAGS_260     := -no-pad-sections --aspsx-version=2.34 --expand-div

# GCC 2.8.0 G4 division expansion varies by source. This flag is cleared with
# a target-specific assignment for objects whose original code used bare div.
# Recursive assignment keeps that target-specific value visible here.
MASPSX_DIV_FLAG_G4 := --expand-div
MASPSX_FLAGS_G4 = -no-pad-sections --aspsx-version=2.77 $(MASPSX_DIV_FLAG_G4)

# DATA_AS_C=1 builds every .data region from C that tools/data2c generates at
# build time (see tools/data2c/README.md). The generated data is typed by the
# C, so it depends on these headers as well as each image's own sources.
DATA_AS_C ?=
DATA_AS_C_HEADERS := $(call rwildcard,include,*.h) $(call rwildcard,src/main,*.h) $(call rwildcard,src/overlays,*.h)
