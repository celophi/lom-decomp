#include "common.h"

typedef struct RecC98D4
{
    u8 pad0[0x24];
    u8 unk24;
    u8 pad25;
    u8 unk26;
} RecC98D4;

typedef struct OutC98D4
{
    u8 unk0;
    u8 unk1;
    u8 unk2;
    u8 unk3;
} OutC98D4;

extern s32 g_gosub_result_count;
extern s32 g_gosub_result_values;
extern u8 g_menuLayoutBuffer[];
extern OutC98D4 D_80122C01;

/**
 * @brief Pack the current gosub result's color/attribute bytes into D_80122C01.
 *
 * If there are gosub results, reads the selected result's 0x40-byte layout record
 * (at @c g_menuLayoutBuffer + 0xCE0) for its 0x24 and 0x26 fields and a 6-bit
 * attribute from the 0xCF4 word; otherwise defaults to 0xFF. The 0x26 field is
 * clamped to 0..0x63 and all four bytes are written to @c D_80122C01.
 *
 * @note gcc280_g0, 100% match.
 */
void func_800C98D4(void)
{
    s32 result_value;
    s32 attr;
    s32 flags;
    s32 clamp_src;
    s32 out;
    RecC98D4 *rec;

    result_value = 0xFF;
    attr = 0xFF;
    flags = 0xFF;
    clamp_src = 0;
    if (g_gosub_result_count != 0)
    {
        u8 *buf = g_menuLayoutBuffer;
        u8 *base;
        u8 *recbase;
        result_value = g_gosub_result_values;
        base = buf + result_value * 0x40;
        recbase = buf + 0xCE0;
        rec = (RecC98D4 *)(recbase + result_value * 0x40);
        flags = rec->unk24;
        attr = (*(u32 *)(base + 0xCF4) >> 10) & 0x3F;
        clamp_src = rec->unk26;
    }
    if (clamp_src >= 0)
    {
        out = 0x63;
        if (clamp_src < 0x64)
        {
            out = clamp_src;
        }
    }
    else
    {
        out = 0;
    }
    D_80122C01.unk0 = result_value;
    D_80122C01.unk1 = attr;
    D_80122C01.unk2 = flags;
    D_80122C01.unk3 = out;
}
