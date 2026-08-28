#include "common.h"

typedef struct {
    u8 pad[0xCF4];
    s32 unkCF4;
} MenuRec;

extern s16 D_80122C10;
extern u8 g_menuLayoutBuffer[];
extern void akao_set_song_params(s32 flags, s32 duration, s32 field_id, s32 sub_id);

/**
 * @brief Resolve the current menu record's song slot and latch it.
 *
 * Reads the packed field @c unkCF4 of the record selected by @c D_80122C10,
 * decoding a base index (@c bits 10-15) offset by mode (@c bits 8-9): +0 for
 * mode 0, +0xB for mode 1, +0x17 otherwise. When the result is the 0xFF
 * sentinel it triggers akao_set_song_params and stores 0; otherwise the resolved
 * slot is written back to @c D_80122C10.
 *
 * @see decomp.me (100%) TODO
 */
void func_800C6E28(void)
{
    u32 rec;
    s32 mode;
    s32 v;

    rec = ((MenuRec *)(g_menuLayoutBuffer + D_80122C10 * 0x40))->unkCF4;
    mode = (rec >> 8) & 3;
    if (mode == 0)
    {
        v = (rec >> 10) & 0x3F;
    }
    else if (mode == 1)
    {
        v = ((rec >> 10) & 0x3F) + 0xB;
    }
    else
    {
        v = ((rec >> 10) & 0x3F) + 0x17;
    }
    if (v == 0xFF)
    {
        akao_set_song_params(0x8002, 0x22, 0, 0);
        v = 0;
    }
    D_80122C10 = v;
}
