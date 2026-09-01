#include "common.h"

typedef struct { u32 word; } AddheroAttrWord;
typedef struct { AddheroAttrWord attr; u32 unk4; void (*draw_handler)(); } AddheroPacket;
typedef struct { AddheroPacket first; AddheroPacket second; } AddheroPacketBlock;
typedef struct { s32 unk0; s32 unk4; s16 unk8; s16 unkA; s16 unkC; u16 unkE; } AddheroGpuPacket;
typedef struct { s32 unk0; u8 pad4[0x40AE]; s16 unk40B2; u8 pad40B4[4]; AddheroGpuPacket *unk40B8; } AddheroDrawState;
typedef AddheroGpuPacket *(*AddheroElemDrawFunc)();

extern AddheroPacketBlock D_80160940;
extern s32 D_801609A4;
extern s32 D_80160928;
extern s32 D_80122988;

void func_80141F00(AddheroDrawState *arg0)
{
    AddheroGpuPacket *var_s0;
    AddheroDrawState *var_s5;
    volatile u32 *var_s3;
    s32 temp_s1;
    s32 temp_s2;
    s32 var_s6;
    s32 sp20[24];
    u32 temp_a0_2;
    s32 temp_v1_2;
    u32 temp_a1;
    u32 temp_a2;
    s32 temp_a0_3;
    s32 var_v1;
    s32 temp_a3_2;
    s32 var_v0;
    s32 temp_a3_3;
    u32 temp_v0_3;
    u32 temp_a0_4;
    s32 temp_a0_5;
    s32 var_v1_2;
    s32 temp_a3_5;
    s32 var_v0_2;
    s32 temp_a3_6;
    u32 temp_v0_5;
    u32 temp_v1_3;
    s32 count;

    var_s0 = arg0->unk40B8;
    var_s5 = arg0;

    count = D_801609A4;
    if ((count < 0x10) &&
        ((D_80160940.second.attr.word & 7) == 2) &&
        (((D_80160940.second.unk4 >> 9) & 1) != 0))
    {
        count *= 0xE;
        if ((D_80160928 + 0x58) < count)
        {
            var_s0 = (AddheroGpuPacket *)func_800AE76C(var_s0, var_s5, 0x114, 0x82, 0);
        }
        if (D_80160928 != 0)
        {
            var_s0 = (AddheroGpuPacket *)func_800AE76C(var_s0, var_s5, 0x114, 0x3A, 1);
        }
    }

    if (arg0->unk40B2 != 0)
    {
        func_8001C56C(sp20, 0, 0xF0, 0x140, 0xE0);
    }
    else
    {
        func_8001C56C(sp20, 0, 8, 0x140, 0xE0);
    }

    var_s3 = (volatile u32 *)&D_80160940;
    var_s6 = 0;

    for (; var_s6 < 8; var_s6++, var_s3 += 3)
    {
        if (*var_s3 & 7)
        {
            func_8001A5D4((s32)var_s0, sp20);

            var_s0->unk0 = (var_s0->unk0 & 0xFF000000) | (var_s5->unk0 & 0x00FFFFFF);
            var_s5->unk0 = (s32)((var_s5->unk0 & 0xFF000000) | ((s32)var_s0 & 0x00FFFFFF));

            temp_a0_2 = *var_s3;
            temp_v1_2 = temp_a0_2 & 7;

            var_s0 = (AddheroGpuPacket *)((u8 *)var_s0 + 0x40);

            switch (temp_v1_2)
            {
            case 1:
                temp_v0_3 = *var_s3;
                temp_a1 = *(u32 *)((u8 *)var_s3 + 4);
                temp_a0_4 = temp_v0_3 >> 24;
                temp_a2 = ((temp_a1 & 1) << 8) | temp_a0_4;
                temp_a0_3 = (temp_v0_3 >> 3) & 0xF;
                var_v1 = temp_a2 * temp_a0_3;
                D_80122988 = 0;
                if (var_v1 < 0)
                {
                    var_v1 += 7;
                }
                temp_a3_2 = (temp_a1 >> 1) & 0xFF;
                var_v0 = temp_a3_2 * temp_a0_3;
                temp_s1 = var_v1 >> 3;
                if (var_v0 < 0)
                {
                    var_v0 += 7;
                }
                temp_s2 = var_v0 >> 3;
                temp_a3_3 = (s32)(temp_a3_2 - temp_s2);

                var_s0 = (*(AddheroElemDrawFunc *)((u8 *)var_s3 + 8))(var_s5, var_s0, (s32)(temp_a2 - temp_s1) / 2, temp_a3_3 / 2);
                {
                    u32 post_word;
                    u32 field;
                    u32 high;
                    post_word = *var_s3;
                    field = (post_word >> 7) & 0x1FF;
                    high = post_word >> 24;
                    var_s0 = (AddheroGpuPacket *)func_800AD850(var_s0, var_s5,
                                           field + (s32)((((*(u32 *)((u8 *)var_s3 + 4) & 1) << 8) | high) - temp_s1) / 2,
                                           (*((u8 *)var_s3 + 2)) + ((s32)((*(u32 *)((u8 *)var_s3 + 4) >> 1) & 0xFF) - temp_s2) / 2,
                                           temp_s1, temp_s2, arg0->unk40B2, var_s6 == 0);
                }
                {
                    u32 old_word;
                    u32 new_word;
                    old_word = *var_s3;
                    new_word = (old_word & ~0x78) | (((((old_word >> 3) & 0xF) + 1) & 0xF) * 8);
                    *(u32 *)var_s3 = new_word;
                    if (((new_word >> 3) & 0xF) == 8)
                    {
                        func_800AA02C();
                        *(u32 *)var_s3 = (*var_s3 & ~7) | 2;
                    }
                }
                break;

            case 2:
                var_s0 = (*(AddheroElemDrawFunc *)((u8 *)var_s3 + 8))(var_s5, var_s0, 0, 0);
                {
                    u32 case_word;
                    u32 high;
                    case_word = *var_s3;
                    high = case_word >> 24;
                    var_s0 = (AddheroGpuPacket *)func_800AD850(var_s0, var_s5,
                                           (case_word >> 7) & 0x1FF, *((u8 *)var_s3 + 2),
                                           ((*(u32 *)((u8 *)var_s3 + 4) & 1) << 8) | high,
                                           (*(u32 *)((u8 *)var_s3 + 4) >> 1) & 0xFF, arg0->unk40B2, var_s6 == 0);
                }
                temp_v1_3 = *var_s3;
                if (((temp_v1_3 >> 3) & 0xF) != 0)
                {
                    *(u32 *)var_s3 = (temp_v1_3 & ~0x78) | (((((temp_v1_3 >> 3) & 0xF) - 1) & 0xF) * 8);
                }
                break;

            case 3:
                temp_a0_5 = *var_s3;
                temp_a1 = *(u32 *)((u8 *)var_s3 + 4);
                var_v1_2 = (u32)temp_a0_5 >> 24;
                temp_a2 = ((temp_a1 & 1) << 8) | var_v1_2;
                temp_a0_5 = (u32)temp_a0_5 >> 3;
                temp_a0_5 &= 0xF;
                var_v1_2 = temp_a2 * temp_a0_5;
                D_80122988 = 0;
                if (var_v1_2 < 0)
                {
                    var_v1_2 += 7;
                }
                temp_a3_5 = (temp_a1 >> 1) & 0xFF;
                var_v0_2 = temp_a3_5 * temp_a0_5;
                temp_s1 = var_v1_2 >> 3;
                if (var_v0_2 < 0)
                {
                    var_v0_2 += 7;
                }
                temp_s2 = var_v0_2 >> 3;
                temp_a3_6 = (s32)(temp_a3_5 - temp_s2);

                var_s0 = (*(AddheroElemDrawFunc *)((u8 *)var_s3 + 8))(var_s5, var_s0, (s32)(temp_a2 - temp_s1) / 2, temp_a3_6 / 2);
                {
                    u32 post_word;
                    u32 field;
                    u32 high;
                    post_word = *var_s3;
                    field = (post_word >> 7) & 0x1FF;
                    high = post_word >> 24;
                    var_s0 = (AddheroGpuPacket *)func_800AD850(var_s0, var_s5,
                                           field + (s32)((((*(u32 *)((u8 *)var_s3 + 4) & 1) << 8) | high) - temp_s1) / 2,
                                           (*((u8 *)var_s3 + 2)) + ((s32)((*(u32 *)((u8 *)var_s3 + 4) >> 1) & 0xFF) - temp_s2) / 2,
                                           temp_s1, temp_s2, arg0->unk40B2, var_s6 == 0);
                }
                {
                    u32 old_word;
                    old_word = *var_s3;
                    var_v1_2 = old_word & ~0x78;
                    old_word >>= 3;
                    old_word &= 0xF;
                    old_word--;
                    old_word &= 0xF;
                    old_word <<= 3;
                    var_v1_2 |= old_word;
                    *(u32 *)var_s3 = var_v1_2;
                    if (!(((u32)var_v1_2 >> 3) & 0xF))
                    {
                        *(u32 *)var_s3 = ((((u32)var_v1_2 & ~0x78) | 0x18) & ~7) | 4;
                    }
                }
                break;

            case 4:
                temp_v0_5 = *(u32 *)var_s3;
                D_80122988 = 0;
                temp_v1_3 = (temp_v0_5 & ~0x78) | (((((temp_v0_5 >> 3) & 0xF) - 1) & 0xF) * 8);
                *(u32 *)var_s3 = temp_v1_3;
                if (!((temp_v1_3 >> 3) & 0xF))
                {
                    *(u32 *)var_s3 = temp_v1_3 & ~7;
                }
                break;
            }
        }
    }

    arg0->unk40B8 = var_s0;
}

