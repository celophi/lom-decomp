#include "common.h"

#define CARDA_CARD_DIRECTORY_BYTES 0x320
#define CARDA_DIRECTORY_ENTRY_BYTES 0x28

typedef struct { s16 vx; s16 vy; } DVECTOR;
typedef struct CardaFallbackTextMatch { u8 pad[0x24]; u8 text[0x20]; } CardaFallbackTextMatch;

extern s32 D_801660FC;
extern s32 D_80166AE0;
extern s32 D_80165FEC;
extern s32 D_801660A0;
extern s32 D_80165FF4;
extern u8 D_80166440[];
extern u8 D_80166124[];
extern u8 D_80166120[];
extern u8 D_801662A0[];
extern u8 D_8016636F;
extern s32 D_8016606C;
extern s32 D_80166068;
extern s32 D_8003EC9C;
extern char D_800ECF7C[];
extern u16 D_8014B060;
extern u16 D_8014B092;
extern u16 D_8014B08C;
extern u16 D_8014CA6C[];
extern u8 D_800EC3F6[2];

s32 func_800A88A0();
s32 func_8001714C();
s32 func_80144CD0();
s32 func_800A8A78();
void func_80142508(void *);
s32 func_8014A900();

#define CARDA_GLYPH_SYM(sym, off) ((void *)(((u8 *)&(sym) - (off)) + (sym)))
#define CARDA_GLYPH_OFF(base, off) ((void *)((base) + *(u16 *)((base) + (off))))

/**
 * @brief Draw the selected memory-card entry's details.
 *
 * This is the CARDA counterpart of CLOAD's selected-entry renderer.  The
 * unused vector is retained because it is part of the original function's
 * stack layout under GCC 2.7.2 CDK.
 * @see matching: 100.00%
 */
s32 func_80141DF4(s32 *ot, s32 prim, s32 x_offset, s32 y_offset)
{
    s32 result;
    DVECTOR pos;
    u8 name[0x21];
    char unused_pad[212];
    s32 slot[3];
    DVECTOR unused_pos;

    result = prim;
    if (D_801660FC == 0)
    {
        return result;
    }
    if (D_80166AE0 != 0)
    {
        return result;
    }
    if (D_801660FC != 3 && D_80165FEC != 0xFA && D_80165FEC < 0x10)
    {
        if (D_801660FC == 2)
        {
            s32 x = -x_offset;
            u8 *base;

            result = func_800A88A0(prim, ot, CARDA_GLYPH_SYM(D_8014B060, 0x28), 4, x, -y_offset, 0);
            base = (u8 *)&D_8014B060 - 0x28;
            return func_800A88A0(result, ot, CARDA_GLYPH_OFF(base, 0x2A), 4, x, 0x10 - y_offset, 0);
        }
        else if (D_801660FC == 4)
        {
            return func_800A88A0(prim, ot, CARDA_GLYPH_SYM(D_8014B092, 0x5A), 4, -x_offset, -y_offset, 0);
        }
        else
        {
            s32 term1 = D_801660A0 * CARDA_CARD_DIRECTORY_BYTES;
            s32 term2 = (D_80165FF4 * CARDA_DIRECTORY_ENTRY_BYTES) + (s32)D_80166440;

            if (func_8001714C(D_800ECF7C, (void *)(term1 + term2), 0xC) == 0)
            {
                if (D_8003EC9C == 0xFF || D_8016636F == D_8003EC9C || D_8016636F == 0xFF)
                {
                    u8 *base90 = D_801662A0;
                    s32 present_count;
                    s32 i;
                    s32 j;
                    s32 step;
                    s32 half_step;
                    s32 base_x;
                    s32 base_y;
                    s32 total;
                    s32 hours;
                    s32 time_val;

                    total = 0;
                    slot[0] = (u32)(*(s32 *)(base90 + 0x18)) >> 0x19;
                    slot[1] = ((u32)(*(s32 *)(base90 + 0x20)) >> 0x12) & 0x7F;
                    slot[2] = (u32)(*(s32 *)(base90 + 0x20)) >> 0x19;
                    D_8016606C = (s32)base90[0x1F];

                    present_count = 0;
                    for (i = 0; i < 3; i++)
                    {
                        if (slot[i] != 0x7F)
                        {
                            present_count += 1;
                        }
                    }

                    switch (present_count)
                    {
                    case 2:
                        step = 0x20;
                        half_step = 0x10;
                        time_val = D_80166068;
                        if (D_80166068 < 0)
                        {
                            time_val = D_80166068 + 0x1F;
                        }
                        D_80166068 -= (time_val >> 5) << 5;
                        break;
                    case 3:
                        step = 0x10;
                        half_step = 0x20;
                        D_80166068 %= 0x60;
                        break;
                    default:
                        step = 0x10;
                        half_step = 0x20;
                        D_80166068 = 0x1F;
                        break;
                    }

                    i = 0;
                    j = i;
                    for (; j < 3; j++)
                    {
                        base_y = i * half_step;
                        base_x = base_y + half_step;
                        if (slot[j] != 0x7F)
                        {
                            s32 adjust = step;
                            s32 rem;
                            s32 hi;
                            s32 delta;

                            if ((D_80166068 >= base_y && D_80166068 < base_x && (delta = D_80166068 - base_y, 1))
                                || (rem = base_x % (half_step * present_count), D_80166068 >= rem && D_80166068 < (hi = rem + half_step) && (delta = hi - D_80166068, 1)))
                            {
                                adjust += delta;
                            }
                            result = func_80144CD0(result, ot, total - x_offset, -y_offset, adjust, slot[j], i, j);
                            total += adjust;
                            i += 1;
                        }
                    }

                    {
                        u8 *base90_2 = D_801662A0;
                        s32 x = -x_offset;
                        s32 y = -y_offset;

                        base_y = *(s32 *)(base90_2 + 0x30);

                        pos.vx = (s16)(x + 0x70);
                        pos.vy = (s16)y;
                        hours = base_y / 216000;
                        result = func_800A8A78(ot, result, hours, 4, &pos, 1);
                        result = func_800A88A0(result, ot, D_800EC3F6[0] + ((s32)&D_800EC3F6 - 0x32) + (D_800EC3F6[1] << 8), 4, x + 0x6F, y, 0);
                        base_y = (base_y / 3600) - (hours * 0x3C);
                        if (base_y < 0xA)
                        {
                            pos.vx = (s16)(x + 0x7D);
                            pos.vy = (s16)y;
                            result = func_800A8A78(ot, result, 0, 4, &pos, 1);
                        }
                        pos.vx = (s16)(x + 0x85);
                        pos.vy = (s16)y;
                        result = func_800A88A0(func_800A88A0(func_800A8A78(ot, result, base_y, 4, &pos, 1), ot, base90_2, 4, x + 0x54, y + 0x10, 0), ot, CARDA_GLYPH_OFF((u8 *)D_8014CA6C, (*(s32 *)(base90_2 + 0x20) & 0x3FFFF) * 2), 4, x + 0x54, y + 0x20, 0);
                    }
                }
                else
                {
                    result = func_800A88A0(result, ot, CARDA_GLYPH_SYM(D_8014B08C, 0x54), 4, -x_offset, -y_offset, 0);
                }
            }
            else
            {
                s32 j;

                {
                    u8 *text_base;
                    func_80142508(&D_80166124);
                    text_base = (u8 *)&D_80166124;
                    text_base -= 4;
                    if ((u32)(text_base[0x24] - 1) >= 0x7FU)
                    {
                        do {
do {
                        for (j = 0; j < 0x20; j++)
                        {
                            name[j] = text_base[j + 4];
                        }
                        name[j] = 0;
                        result = func_8014A900(result, ot, name, -x_offset, -y_offset, 4, 0);

                        for (j = 0; j < 0x20; j++)
                        {
                            name[j] = ((CardaFallbackTextMatch *)&D_80166120)->text[j];
                        }
                        name[j] = 0;
                        result = func_8014A900(result, ot, name, -x_offset, -y_offset + 0x10, 4, 0);
                        } while (0);
} while (0);
                    }
                }
            }
        }
    }
    return result;
}
