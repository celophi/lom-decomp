#include "common.h"
/** @brief Layout buffer view for active records, identity words, and packed metadata. */
typedef struct Layout
{
    u8 pad0[0x640];
    u8 unk640;
    u8 pad641[0x37];
    u32 unk678, unk67C;
    u8 pad680[0x660];
    u8 unkCE0;
    u8 padCE1[0x37];
    u32 unkD18, unkD1C;
    u8 padD20[0x2440];
    u8 unk3160;
    u8 pad3161[0x13];
    union
    {
        u32 word;
        u16 halves[2];
    } packed;
    u8 pad3178[0x20];
    u32 unk3198, unk319C;
} Layout;
extern void func_800B2844(s32, u8 *, s32);
extern u8 D_80046138[], D_800F0E98[], g_menuLayoutBuffer[];
extern u8 D_80122C02, D_80122C03, D_80122C04, D_80122C06;
/**
 * @brief Count available records, detect duplicate identities, and prepare the selected display.
 * @note Status is 1 for an empty selection and 2 for an identity found in either table.
 */
void func_800C905C(void)
{

    s32 var_v1;
    s32 temp_s0;
    s32 temp_s0_2;
    s32 temp_v0;
    s32 temp_v0_3;
    s32 var_a1;
    s32 var_a3;
    s32 temp_s2;
    s32 var_a0;
    s32 var_s1;
    u32 temp_v0_2;
    u8 *temp_a2;
    u8 *temp_v1;

    s32 selected;
    selected = D_80122C02;
    var_a0 = 0;
    var_a1 = var_a0;
    var_v1 = (s32)g_menuLayoutBuffer + var_a1 * 0x40;
    do
    {
        if (((Layout *)var_v1)->unk3160 != 0)
        {
            var_a0 += 1;
        }
        var_a1 += 1;
        var_v1 = (s32)g_menuLayoutBuffer + var_a1 * 0x40;
    } while (var_a1 < 4);
    var_a1 = (s32)&D_80122C06;
    *(u8 *)var_a1 = var_a0;
    var_a0 = (s32)g_menuLayoutBuffer;
    temp_v1 = (selected << 6) + g_menuLayoutBuffer;
    ((u8 *)var_a1)[-3] = 0;
    var_a3 = 0;
    if (((Layout *)temp_v1)->unk3160 == 0)
    {
        ((u8 *)var_a1)[-3] = 1;
        return;
    }
    goto search_first;
found_first:
    var_a3 = 1;
    goto search_second;
found_second:
    var_a3 = 1;
    goto searches_done;
search_first:
    var_a1 = var_a3;
    temp_a2 = temp_v1;
first_loop:
    if (((Layout *)var_a0)->unkCE0 != 0 &&
        ((Layout *)var_a0)->unkD18 == ((Layout *)temp_a2)->unk3198 &&
        ((Layout *)var_a0)->unkD1C == ((Layout *)temp_a2)->unk319C)
    {
        goto found_first;
    }
    var_a1++;
    var_a0 += 0x40;
    if (var_a1 < 100)
    {
        goto first_loop;
    }
search_second:
    var_a1 = 0;
    var_v1 = (s32)g_menuLayoutBuffer;
    temp_a2 = (u8 *)((selected << 6) + var_v1);
    var_a0 = var_v1;
second_loop:
    if (((Layout *)var_a0)->unk640 != 0 &&
        ((Layout *)var_a0)->unk678 == ((Layout *)temp_a2)->unk3198 &&
        ((Layout *)var_a0)->unk67C == ((Layout *)temp_a2)->unk319C)
    {
        goto found_second;
    }
    var_a1++;
    var_a0 += 0x40;
    if (var_a1 < 8)
    {
        goto second_loop;
    }
searches_done:
    temp_v0 = selected << 6;
    if (var_a3 == 0)
    {
        var_v1 = (s32)g_menuLayoutBuffer;
        temp_v0_2 = ((Layout *)(temp_v0 + var_v1))->packed.word;
        temp_s2 = (temp_v0_2 >> 8) & 3;
        var_s1 = (temp_v0_2 >> 0xA) & 0x3F;
        if (temp_s2 == 1)
        {
            var_s1 += 0xB;
        }
        else if (temp_s2 == 2)
        {
            var_s1 += 0x17;
        }
        var_v1 = (s32)g_menuLayoutBuffer;
        temp_v0_3 = selected << 6;
        temp_s0 = ((Layout *)(temp_v0_3 + var_v1))->packed.halves[1] & 0x3F;
        func_800B2844(0, temp_v0_3 + ((u8 *)var_v1 + 0x3160), 0xFF);
        D_80122C04 = temp_s2;
        temp_s0_2 = temp_s0 * 2;
        (&D_80122C04)[1] = var_s1;
        func_800B2844(1, D_800F0E98[temp_s0_2] + (D_800F0E98[temp_s0_2 + 1] << 8) + D_800F0E98,
                      0xFF);
        return;
    }
    func_800B2844(0, temp_v0 + D_80046138, 0xFF);
    D_80122C03 = 2;
}
