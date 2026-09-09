#include "common.h"
/** @brief Script command result, opcode, and three word-sized arguments. */
typedef struct Command
{
    u16 result, opcode;
    s32 arg4, arg8;
    u32 argC;
} Command;
extern u8 *func_800A9060(void);
extern void func_800BD520(s32, s32, s32);
extern void func_800BE888(u8 *, s32, s32, u32);
extern void func_800BEC44(u8 *, s32);
extern s32 func_800BF68C(s32, s32, u32);
extern void func_800BF880(s32);
extern void func_800BF9A0(s32);
extern s32 func_800BF9F0(s32);
extern void func_800C1EC8(s32, u8 *, s32);
extern u8 *D_80122B74, *D_80122B78, *D_80123FC4, *g_field_script;
/**
 * @brief Dispatch record setup, counter consumption, and related script operations.
 * @param arg0 Unused dispatcher context.
 * @param arg1 Command whose result halfword receives an index or error code.
 */
void func_800BDCA4(s32 arg0, Command *arg1)
{
    u8 *temp_a1;

    u8 **context;
    s32 temp_v0_4;
    s32 temp_v0_5;
    u16 temp_v1;
    u32 temp_v1_2;
    u32 var_v0;
    u8 *temp_s1;
    s32 temp_v0_2;
    s32 temp_v0_3;
    u8 *temp_a1_2;
    u8 *temp_v1_3;
    u8 *temp_v1_4;

    temp_v1 = arg1->opcode;
    switch (temp_v1)
    {
    case 0:
        temp_a1 = D_80122B78 + 0x104;
        D_80123FC4 = temp_a1;
        func_800C1EC8(0, temp_a1, 0x60);
        return;
    case 1:
        temp_s1 = func_800A9060();
        var_v0 = 0xFA;
        if (temp_s1 != 0)
        {
            context = &D_80122B74;
            temp_v1_2 = arg1->argC;
            if (temp_v1_2 < 0x40U)
            {
                temp_v1_3 = *context + temp_v1_2;
                temp_v0_2 = temp_v1_3[0x25E0];
                if (temp_v0_2 != 0)
                {
                    temp_v1_3[0x25E0] = (u8)(temp_v0_2 - 1);
                    func_800BE888(temp_s1, arg1->arg4, arg1->arg8, arg1->argC);
                    var_v0 = (u32)temp_s1 - 0xCE0;
                    var_v0 = (var_v0 - (u32)*context) >> 12;
                }
                else
                {
                    var_v0 = 0xFD;
                }
            }
            else
            {
                var_v0 = 0xFE;
            }
        }
        goto block_22;
    case 2:
        temp_s1 = D_80122B74 + ((arg1->arg4 << 6) + 0xCE0);
        var_v0 = 0xFB;
        if (*temp_s1 != 0)
        {
            temp_v1_4 = D_80122B74 + arg1->arg8;
            temp_v0_3 = temp_v1_4[0x25E0];
            if (temp_v0_3 != 0)
            {
                temp_v1_4[0x25E0] = (u8)(temp_v0_3 - 1);
                func_800BEC44(temp_s1, arg1->arg8);
                arg1->result = (u16)arg1->arg4;
                return;
            }
            var_v0 = 0xFD;
            goto block_22;
        }
        goto block_22;
    case 3:
        if (arg1->arg8 != 0)
        {
            do
            {
                func_800BF9A0(arg1->arg4);
                temp_v0_4 = arg1->arg8 - 1;
                arg1->arg8 = temp_v0_4;
            } while (temp_v0_4 != 0);
            return;
        }
        break;
    case 4:
        if (arg1->arg8 != 0)
        {
            do
            {
                func_800BF880(arg1->arg4);
                temp_v0_5 = arg1->arg8 - 1;
                arg1->arg8 = temp_v0_5;
            } while (temp_v0_5 != 0);
            return;
        }
        break;
    case 5:
        temp_v0_4 = func_800BF9F0(arg1->arg4);
        temp_a1_2 = g_field_script + (*(s32 *)(g_field_script + 4) * 0xC);
        *(s32 *)(temp_a1_2 + 0xC) = (s32)((*(s32 *)(temp_a1_2 + 0xC) & ~1) | (temp_v0_4 & 1));
        return;
    case 6:
        func_800BD520(0, 0x7100, func_800BF68C(arg1->arg4, arg1->arg8, arg1->argC));
        return;
    case 7:
    default:
        var_v0 = 0xFF;
        goto block_22;
    }
    return;
block_22:
    arg1->result = (u16)var_v0;
}
