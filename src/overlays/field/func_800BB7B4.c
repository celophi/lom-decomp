#include "common.h"

/** @brief Reset fields addressed relative to successive 0x60-byte slots. */
typedef struct
{
    u8 pad[0x2F36];
    u16 count;
    s32 flags;
} Reset;
/** @brief Script header and 12-byte slot view used to locate the current bytecode. */
typedef struct
{
    s32 unk0;
    s32 unk4;
    u8 *unk8;
} Script;
extern Script *g_field_script;
extern u8 *D_80122B74;
extern u8 *D_80122B78;
extern void func_800B34D0(s32);
extern void func_800C1230(s32);
extern u8 *func_800C1E40(s32);
extern s32 *func_800C1EC8(s32 *, s32 *, s32);
/** @brief Dispatch a reset command and advance the current script by two bytes. */
void func_800BB7B4(void)
{
    s32 fill;
    s32 mask;
    s32 var_a0;
    s32 index;
    s32 var_a0_2;
    u8 temp_v1;
    Script *temp_a0;
    u8 *temp_v0;
    Script *temp_v1_2;
    u8 *var_v0;
    Reset *var_v1;

    temp_v1_2 = g_field_script;
    index = temp_v1_2->unk4;
    temp_v1_2 += index;
    temp_v1 = temp_v1_2->unk8[1];
    switch (temp_v1)
    {
    case 0:
        fill = 0xFFFFFF;
        var_a0 = 0xA;
        var_v0 = D_80122B74 + 0x28;
        do
        {
            *(s32 *)(var_v0 + 0x34) = fill;
            var_a0 -= 1;
            var_v0 -= 4;
        } while (var_a0 >= 0);
        *(s32 *)(D_80122B74 + 0x60) = 0x500;
        *(s32 *)(D_80122B74 + 0x64) = -0x8000;
        temp_a0 = g_field_script;
        *(s32 *)(D_80122B74 + 0x68) = 0x803F;
        goto block_7;
    case 1:
        func_800B34D0(1);
        goto block_16;
    case 3:
        fill = (s32)func_800C1E40(6);
        temp_v0 = D_80122B78;
        temp_a0 = g_field_script;
        *(s32 *)(temp_v0 + 0xF00) = fill;
    block_7:
        temp_a0 += temp_a0->unk4;
        temp_a0->unk8 = temp_a0->unk8 + 2;
        return;
    case 4:
        func_800C1EC8(0, (s32 *)(D_80122B74 + 0xE4), 0x200);
        goto block_16;
    case 5:
        func_800C1230(0);
        func_800C1230(1);
        func_800C1230(2);
        func_800C1230(3);
        func_800C1230(4);
        goto block_16;
    case 6:
        var_a0 = 0;
        mask = 0x7FFFFFFF;
        var_v1 = (Reset *)D_80122B74;
        do
        {
            var_a0 += 1;
            var_v1->count = 0;
            var_v1->flags = (s32)(var_v1->flags & mask);
            var_v1 = (Reset *)((u8 *)var_v1 + 0x60);
        } while (var_a0 < 5);
        goto block_16;
    case 7:
        var_a0 = 0;
        do
        {
            temp_v0 = D_80122B74 + var_a0;
            var_a0 += 1;
            *(u8 *)(temp_v0 + 0x25E0) = 0x63;
        } while (var_a0 < 0xFD);
        /* fallthrough */
    default:
        goto block_16;
    }
block_16:
    temp_v1_2 = g_field_script;
    index = temp_v1_2->unk4;
    temp_v1_2 += index;
    temp_v1_2->unk8 = temp_v1_2->unk8 + 2;
    return;
}
