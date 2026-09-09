#include "common.h"

extern u8 *func_800C1E40(s32);
extern s32 rand(void);
extern u8 *D_80122B74;
/**
 * @brief Combine random table selections into a record effect and clear its payload.
 * @param arg0 Record index in the field context.
 * @param arg1 Effect destination index within the record.
 */
void func_800C0260(s32 arg0, s32 arg1)
{
    s32 temp_a1;
    s32 temp_s2;
    u8 *temp_s3;
    s32 temp_v0;
    s32 temp_v0_2;
    s32 temp_v0_3;
    s32 temp_v1;
    s32 temp_v1_2;
    s32 var_s0;
    s32 temp_s1;
    s32 var_s1;
    u8 *temp_a0;
    u8 *temp_a0_2;
    u8 *temp_a2;
    u8 *var_a0;

    temp_s3 = func_800C1E40(0xB);
    temp_s2 = arg0 * 0x8C;
    temp_a0 = D_80122B74 + temp_s2;
    if ((((u32) * (u32 *)(temp_a0 + 0x26E4) >> 0xC) & 0xF) == 1)
    {
        var_s0 = 0;
        temp_s1 =
            *(temp_s3 + ((rand() & 0xF) + ((D_80122B74[temp_s2 + 0x26EC] - 0x58) * 0x10)) + 4);
        var_s1 = temp_s1 |
                 *(temp_s3 + ((rand() & 0xF) + ((D_80122B74[temp_s2 + 0x26EC] - 0x58) * 0x10)) + 4);
    }
    else
    {
        var_s0 = 1;
        temp_v0 = rand();
        var_a0 = D_80122B74 + temp_s2;
        var_s1 = *(temp_s3 + ((temp_v0 & 0xF) + ((*(u8 *)(var_a0 + 0x26EC) - 0x58) * 0x10)) + 4);
        if ((((u32) * (u32 *)(var_a0 + 0x26E4) >> 0xC) & 0xF) > 1)
        {
            do
            {
                temp_v0 = rand() & 0xF;
                temp_v1 = var_s0 + temp_s2;
                var_s0 += 1;
                var_a0 = D_80122B74 + temp_s2;
                var_s1 |=
                    *(temp_s3 + (temp_v0 + ((D_80122B74[temp_v1 + 0x26EC] - 0x58) * 0x10)) + 4);
            } while (var_s0 < (((u32) * (u32 *)(var_a0 + 0x26E4) >> 0xC) & 0xF));
        }
        var_s0 = 0;
    }
    temp_v0_2 = arg1 * 0x10;
    temp_v1_2 = arg0 * 0x8C;
    temp_a1 = temp_v0_2 + temp_v1_2;
    temp_a2 = temp_s3 + var_s1;
    D_80122B74[temp_a1 + 0x26F4] = (s8)((temp_a2[0x84] & 0x3F) + 0x60);
    temp_a0_2 = D_80122B74 + temp_a1;
    *(u32 *)(temp_a0_2 + 0x26F4) =
        (s32)((*(u32 *)(temp_a0_2 + 0x26F4) & ~0x300) | (((u8)temp_a2[0x84] >> 6) << 8));
    do
    {
        temp_v0_3 = var_s0 + temp_v0_2;
        var_s0 += 1;
        D_80122B74[temp_v0_3 + temp_v1_2 + 0x26F8] = 0;
    } while (var_s0 < 8);
}
