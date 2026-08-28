#include "common.h"

/**
 * @brief Layout placeholder for the object at D_80122C0D. Only the bytes at
 *        offset 0 and 0x12 are read here.
 */
typedef struct {
    u8 unk0;
    u8 pad[0x11];
    u8 unk12;
} StructC0D;

extern StructC0D D_80122C0D;
extern u8 g_menuLayoutBuffer[];

/**
 * @brief Reset one menu record's status word and its eight trailing bytes.
 *
 * @details Selects a g_menuLayoutBuffer record from D_80122C0D.unk0 and
 * D_80122C0D.unk12, sets its status byte at +0x26F4 to 0xFF, clears the word at
 * +0x26F0 and the eight bytes at +0x26F8..+0x26FF, then forces the upper three
 * bytes of the +0x26F4 word to 0xFF.
 *
 * @note Matched under GCC 2.7.2 CDK. Folding the -4 adjustment into @c va
 *       before constructing the record address makes GCC load unk0 before
 *       materializing the global base, matching the target schedule exactly.
 */
void func_800C87B4(void)
{
    u8 *base;
    u8 *buf;
    s32 va;

    va = D_80122C0D.unk0 - 4;
    buf = g_menuLayoutBuffer;
    base = buf + (va * 0x10 + D_80122C0D.unk12 * 0x8C);
    base[0x26F4] = 0xFF;
    *(s32 *)(base + 0x26F0) = 0;
    base[0x26F8] = 0;
    base[0x26F9] = 0;
    base[0x26FA] = 0;
    base[0x26FB] = 0;
    base[0x26FC] = 0;
    base[0x26FD] = 0;
    base[0x26FE] = 0;
    base[0x26FF] = 0;
    *(s32 *)(base + 0x26F4) |= ~0xFF;
}
