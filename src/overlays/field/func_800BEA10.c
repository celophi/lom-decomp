#include "common.h"
extern void func_800BF2F0(s32);
extern void func_800BFA34(void);
extern u8 *func_800C1E40(s32);
extern void func_800C21C0(s32);
/** @brief Resource table view exposing the command halfword at offset 0x244. */
typedef struct ResourceRow
{
    u8 padding[0x244];
    u16 command;
} ResourceRow;
/** @brief Effect setup header followed by packed state bytes. */
typedef struct Header
{
    u8 *owner;
    u8 mode;
    u8 rest[0x2D];
} Header;
extern u8 *D_80122B78, *D_80123FC0, *g_field_script;
extern Header *D_80123FC4;
/**
 * @brief Initialize effect state, run its command, and copy the resulting parameters.
 * @param arg0 Destination record receiving bytes at offsets 0x24 through 0x26.
 * @param arg1 Effect setup mode.
 * @param arg2 Resource subentry selector.
 * @param arg3 Primary resource selector.
 * @param arg4 Command selector relative to 0x40.
 */
void func_800BEA10(u8 *arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4)
{
    Header **state;
    u8 **resource;
    s32 command;
    u8 *clamp_base;
    s32 temp_a0;
    u8 *temp_s1;
    u8 *temp_v0_3;
    s32 var_a0;
    s32 var_a0_3;
    s32 var_a1;
    u8 temp_v1;
    s32 temp_v1_2;
    s32 temp_v1_3;
    u8 *temp_v0;
    u8 *temp_v0_2;

    func_800C21C0(arg3);
    func_800C21C0(arg4);
    D_80123FC4->owner = arg0;
    D_80123FC4->mode = arg1;
    ((u8 *)D_80123FC4)[0x5] = arg2;
    ((u8 *)D_80123FC4)[0x6] = (s8)arg3;
    var_a0 = 0;
    ((u8 *)D_80123FC4)[0x7] = (s8)arg4;
    do
    {
        temp_v0 = (u8 *)D_80123FC4 + var_a0;
        temp_v1 = temp_v0[0x20];
        var_a0 += 1;
        temp_v0[0x20] = (s8)((temp_v1 & 0xF0) | 4);
    } while (var_a0 < 8);
    var_a0 = 0;
    do
    {
        temp_v0_2 = (u8 *)D_80123FC4 + var_a0;
        var_a0 += 1;
        temp_v0_2[0x28] = 0xFF;
    } while (var_a0 < 6);
    resource = &D_80123FC0;
    temp_v0_3 = func_800C1E40(0xF);
    state = &D_80123FC4;
    temp_a0 = (arg2 * 2) + (arg3 * 8);
    *resource = temp_v0_3;
    ((u8 *)(*state))[0x2F] = (s8)((temp_v0_3 + temp_a0)[0x44] & 7);
    ((u8 *)(*state))[0x30] = (s8)((u8)(*resource + temp_a0)[0x44] >> 3);
    ((u8 *)(*state))[0x31] = (u8)(*resource + temp_a0)[0x45];
    command = ((ResourceRow *)(*resource + ((arg4 - 0x40) * 2)))->command;
    temp_s1 = g_field_script;
    g_field_script = D_80122B78 + 0xD98;
    func_800BF2F0(command);
    g_field_script = temp_s1;
    func_800BFA34();
    arg0[0x24] = (u8)((u8 *)(*state))[0x2E];
    clamp_base = (u8 *)D_80123FC4;
    temp_v1_2 = clamp_base[0x2F];
    if ((s8)clamp_base[0x2F] >= 0)
    {
        var_a1 = 7;
        if (temp_v1_2 < 8U)
        {
            var_a1 = temp_v1_2 & 0xFF;
        }
    }
    else
    {
        var_a1 = 0;
    }
    clamp_base = (u8 *)D_80123FC4;
    temp_v1_3 = clamp_base[0x30];
    if ((s8)clamp_base[0x30] >= 0)
    {
        var_a0_3 = 7;
        if (temp_v1_3 < 8U)
        {
            var_a0_3 = temp_v1_3 & 0xFF;
        }
    }
    else
    {
        var_a0_3 = 0;
    }
    arg0[0x25] = (u8)(D_80123FC0 + (var_a1 + (var_a0_3 * 8)))[4];
    arg0[0x26] = (u8)((u8 *)D_80123FC4)[0x31];
}
