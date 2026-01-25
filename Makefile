# Compiler
CC = mipsel-linux-gnu-gcc
AS = $(CC)

# Flags for PS1 / MIPS I
CFLAGS = -Iinclude -mips1 -mfp32 -mno-abicalls -fno-pic -mno-shared
ASFLAGS = -mips1 -mfp32 -mno-abicalls -fno-pic -mno-shared

# Source and build folders
SRC = $(wildcard src/*.c)
ASM = $(wildcard asm/nonmatchings/**/*.s asm/nonmatchings/*.s)
OBJ = $(patsubst src/%.c,build/asm/%.o,$(SRC)) $(patsubst asm/nonmatchings/%.s,build/asm/%.o,$(ASM))

# Output binary
BIN = build/lom.bin
LD_SCRIPT = linker/slus_010.13.ld

all: $(BIN)

# Compile C files
build/asm/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Assemble ASM files (handles nested folders)
build/asm/%.o: asm/nonmatchings/%.s
	@mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) -c $< -o $@

# Link
$(BIN): $(OBJ)
	$(CC) -T $(LD_SCRIPT) -o $@ $^

clean:
	rm -rf build/asm/*.o $(BIN)
