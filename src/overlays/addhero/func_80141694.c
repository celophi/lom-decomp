#include "common.h"

typedef struct { s16 x; s16 y; } Vec2s;
typedef struct AddheroRecord {
    u8 pad0[0x17];
    u8 unk17;
    u8 pad18[0xCF - 0x18];
    u8 unkCF;
    u8 padD0[4];
    u16 unkD4;
    u16 unkD6;
} AddheroRecord;
typedef struct AddheroFallbackText {
    u8 pad[0x24];
    u8 text[0x20];
} AddheroFallbackText;
typedef struct { u8 data[0x28]; } AddheroEntry28;

extern s32 D_80160920;
extern s32 D_801609C0;
extern s32 D_801609A8;
extern s32 D_801609A4;
extern s32 D_801609AC;
extern s32 D_801609B8;
extern u8 D_80165208;
extern u8 D_8016520C;
extern AddheroRecord D_80165388;
extern u8 D_80165457;
extern s32 D_80164A60;
extern AddheroEntry28 D_80164B60[][20];
extern char D_800ECF7C[];
extern s32 D_8003EC9C;
extern AddheroRecord *D_8012271C;
extern u16 D_80146FCC;
extern u16 D_80146FF4;
extern u16 D_80146FF8;
extern u16 D_80147470[];
extern u8 D_800EC3F6[2];

s32 func_8001714C();
s32 func_800A88A0(s32 prim, s32 *ot, void *glyph, s32 a3, s32 x, s32 y, s32 mode);
s32 func_800A8A78(s32 *ot, s32 prim, s32 ch, s32 a3, Vec2s *pos, s32 mode);
void func_80141DF0(void *arg0);
s32 func_80143DC0(s32 result, s32 *ot, s32 x, s32 y, s32 adjust, s32 slot, s32 i, s32 j);
s32 func_8014686C(s32 result, s32 *ot, u8 *name, s32 x, s32 y, s32 a5, s32 a6);

#define GLYPH_SYM(sym, off) ((void *)(((u8 *)&(sym) - (off)) + (sym)))
#define GLYPH_OFF(base, off) ((void *)((base) + *(u16 *)((base) + (off))))

s32 func_80141694(s32 *ot, s32 prim, s32 arg2, s32 arg3)
{
    s32 result;
    Vec2s pos;
    u8 name[0x21];
    char unused_pad[212];
    s32 slot[3];

    result = prim;
    if (D_801609B8 == 0)
    {
        return result;
    }
    if (D_80164A60 != 0)
    {
        return result;
    }
    if (D_801609B8 != 3 && D_801609A4 < 0x10)
    {
        if (D_801609B8 == 2)
        {
            s32 x = -arg2;
            u8 *base;

            result = func_800A88A0(prim, ot, GLYPH_SYM(D_80146FCC, 0x28), 4, x, -arg3, 0);
            base = (u8 *)&D_80146FCC - 0x28;
            return func_800A88A0(result, ot, GLYPH_OFF(base, 0x2A), 4, x, 0x10 - arg3, 0);
        }
        else
        {
            s32 term1 = D_801609A8 * 0x320;
            s32 term2 = (D_801609AC * 0x28) + (s32)D_80164B60;

            if (func_8001714C(D_800ECF7C, (char *)(term1 + term2), 0xC) == 0)
            {
                if (D_8003EC9C == 0xFF || D_80165457 == D_8003EC9C)
                {
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

                    {
                        u8 *record = (u8 *)&D_80165388;
                        slot[0] = (u32)(*(s32 *)(record + 0x18)) >> 0x19;
                        slot[1] = ((u32)(*(s32 *)(record + 0x20)) >> 0x12) & 0x7F;
                        slot[2] = (u32)(*(s32 *)(record + 0x20)) >> 0x19;
                        D_801609C0 = (s32)record[0x1F];
                    }

                    total = 0;
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
                        time_val = D_80160920;
                        if (D_80160920 < 0)
                        {
                            time_val = D_80160920 + 0x1F;
                        }
                        D_80160920 -= (time_val >> 5) << 5;
                        break;
                    case 3:
                        step = 0x10;
                        half_step = 0x20;
                        D_80160920 %= 0x60;
                        break;
                    default:
                        step = 0x10;
                        half_step = 0x20;
                        D_80160920 = 0x1F;
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

                            if ((D_80160920 >= base_y && D_80160920 < base_x && (delta = D_80160920 - base_y, 1))
                                || (rem = base_x % (half_step * present_count), D_80160920 >= rem && D_80160920 < (hi = rem + half_step) && (delta = hi - D_80160920, 1)))
                            {
                                adjust += delta;
                            }
                            result = func_80143DC0(result, ot, total - arg2, -arg3, adjust, slot[j], i, j);
                            i += 1;
                            total += adjust;
                        }
                    }

                    {
                        u8 *base90 = (u8 *)&D_80165388;
                        s32 x = -arg2;
                        s32 y = -arg3;

                        base_y = *(s32 *)(base90 + 0x30);
                        pos.x = (s16)(x + 0x70);
                        pos.y = (s16)y;
                        hours = base_y / 216000;
                        result = func_800A8A78(ot, result, hours, 4, &pos, 1);
                        result = func_800A88A0(result, ot,
                            D_800EC3F6[0] + ((s32)&D_800EC3F6 - 0x32) + (D_800EC3F6[1] << 8), 4, x + 0x6F, y, 0);
                        base_y = (base_y / 3600) - (hours * 0x3C);
                        if (base_y < 0xA)
                        {
                            pos.x = (s16)(x + 0x7D);
                            pos.y = (s16)y;
                            result = func_800A8A78(ot, result, 0, 4, &pos, 1);
                        }
                        pos.x = (s16)(x + 0x85);
                        pos.y = (s16)y;
                        result = func_800A8A78(ot, result, base_y, 4, &pos, 1);
                        result = func_800A88A0(result, ot, base90, 4, x + 0x54, y + 0x10, 0);

                        if (*(u16 *)(base90 + 0xD4) == *(u16 *)((u8 *)D_8012271C + 0xD4))
                        {
                            do { result = func_800A88A0(result, ot, GLYPH_SYM(D_80146FF4, 0x50), 4, x + 0x54, y + 0x20, 0); } while (0);
                        }
                        else
                        {
                            result = func_800A88A0(result, ot, GLYPH_OFF((u8 *)D_80147470, (*(s32 *)(base90 + 0x20) & 0x3FFFF) * 2), 4,
                                x + 0x54, y + 0x20, 0);
                        }
                    }
                }
                else
                {
                    result = func_800A88A0(result, ot, GLYPH_SYM(D_80146FF8, 0x54), 4, -arg2, -arg3, 0);
                }
            }
            else
            {
                s32 j;
                u8 *record;

                func_80141DF0(&D_8016520C);
                record = &D_8016520C;
                record -= 4;
                if ((u32)(record[0x24] - 1) >= 0x7FU)
                {
                    for (j = 0; j < 0x20; j++)
                    {
                        name[j] = record[4 + j];
                    }
                    name[j] = 0;
                    result = func_8014686C(result, ot, name, -arg2, -arg3, 4, 0);

                    for (j = 0; j < 0x20; j++)
                    {
                        name[j] = ((AddheroFallbackText *)&D_80165208)->text[j];
                    }
                    name[j] = 0;
                    result = func_8014686C(result, ot, name, -arg2, -arg3 + 0x10, 4, 0);
                }
            }
        }
    }
    return result;
}
