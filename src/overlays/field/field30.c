#include "common.h"

typedef struct
{
    s32 unk0;
    s32 unk4;
    s32 unk8;
    s32 unkC;
    s32 unk10;
} UnkStruct14;

void func_800832F0(UnkStruct14 *arg0, UnkStruct14 *arg1)
{
    arg0->unk0 = arg1->unk0;
    arg0->unk4 = arg1->unk4;
    arg0->unk8 = arg1->unk8;
    arg0->unkC = arg1->unkC;
    arg0->unk10 = arg1->unk10;
}

typedef struct
{
    u32 unk0;         /* 0x00 */
    s32 unk4;         /* 0x04 */
    u8  unk8[3];      /* 0x08 */
    u8  unkB;         /* 0x0B */
    u8  unkC[6];      /* 0x0C */
    u8  unk12;        /* 0x12 */
    u8  unk13[0x35];  /* 0x13, stride 0x48 */
} UnkPartEntry;

/**
 * @see decomp.me (100%) local match - no scratch link created.
 */
void func_8008332C(u8 *arg0, UnkPartEntry *arg1, s32 arg2)
{
    s32 var_s1;
    u32 temp_a3;
    u8 temp_v0_2;
    u8 temp_test;
    s32 temp_a0;

    for (var_s1 = 0; var_s1 < arg2; var_s1++)
    {
        temp_test = (arg1[var_s1].unkB + 9) & 0xFF;
        if ((u32) temp_test < 3U)
        {
            if (arg1[var_s1].unk4 & 1)
            {
                temp_a3 = arg1[var_s1].unk0;
                func_8008343C(0xF9 - arg1[var_s1].unkB, temp_a3 & 3, arg0, ((temp_a3 >> 8) & 7) + 1);
            }
            if (((u32) arg1[var_s1].unk0 >> 0x16) & 1)
            {
                if ((arg1[var_s1].unk12 != 0) && (field_get_track_counter_modulo(arg0, arg1[var_s1].unk12) == 0))
                {
                    temp_a0 = 0xF9 - arg1[var_s1].unkB;
                    temp_v0_2 = (arg0 + var_s1)[0x2B] + 1;
                    (arg0 + var_s1)[0x2B] = temp_v0_2;
                    func_80083868(temp_a0, temp_v0_2 & 0xFF, arg0);
                }
            }
        }
    }
}
