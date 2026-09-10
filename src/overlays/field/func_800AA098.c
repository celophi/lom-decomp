#include "common.h"
#include "main.h"
/** @brief Connection, raw input, and feedback bytes for the two controller ports. */
typedef struct
{
    u8 connected;
    u8 unknown1;
    u16 buttons;
    u8 pad4[0x8D];
    u8 feedback91, feedback92;
    u8 pad93[0x1B];
    u8 connected2;
    u8 padaf[0x90];
    u8 feedback13f, feedback140;
} FieldInputHardware;
/** @brief Flags, presence, and state in a 0x54-byte field actor record. */
typedef struct
{
    u8 pad0[0x1C];
    u32 flags;
    u8 pad20[5];
    u8 presence;
    u8 pad26[4];
    s16 state;
    u8 tail[0x28];
} FieldInputActor;
/** @brief Value at offset 0x14 in a 0x23C-byte actor slot. */
typedef struct
{
    u8 pad0[0x14];
    s32 value;
    u8 tail[0x224];
} FieldInputSlot;
extern FieldInputActor D_800FDF58[];
extern FieldInputSlot D_80105AE0[];
extern s32 g_pending_game_state;
extern s32 D_8012269C, D_8010AE78, D_80122710, D_801227C8;
extern s32 D_800F2298, D_800F229C, g_field_return_to_title_prompt_state, D_80122714;
extern s32 D_80122984, D_8012291C, D_80122980;
extern u8 D_801227B8[2], D_801227B9;
extern void akao_cmd_98_9a_9c_9e(s32);
extern void func_800A3904(s32, s32, s32);
extern void func_8008C7A8(void);
extern void func_800AA858(s32);
extern s32 func_8006751C(s32);
extern s32 func_800B0850(void);
extern void func_800AA570(void *, s32);
extern s32 cdrom_get_error_status(void);
extern void func_800AA7A4(void);
extern s32 func_8005B218(void);
extern s32 func_800AA498(void);
extern void func_800AEE28(void);
extern void func_800A3938(s32, s32);

/**
 * @brief Process field controller status, reset input, and guarded menu actions.
 * @param arg0 Value forwarded to the alternate input handler when it is active.
 * @note Reset input and CD errors clear controller feedback bytes.
 * @note The first and final actor scans share their counter to preserve codegen.
 */
void func_800AA098(s32 arg0)
{
    FieldInputHardware *pad = (FieldInputHardware *)0x801ED600;
    u32 buttons;
    FieldInputActor *actor;
    FieldInputActor *actor2;
    FieldInputSlot *slot;
    s32 i;
    s32 count;
    s32 index;
    s32 mask;
    s32 absent;
    s32 injected;

    buttons = pad->buttons;
    buttons = (buttons >> 8) | ((buttons & 0xFF) << 8);
    buttons = ((buttons & 0x40) >> 1) | ((buttons & 0x20) << 1) |
              ((buttons & 0x80) >> 3) | ((buttons & 0x10) << 3) | (buttons & 0xFF0F);
    if (D_8012269C != 0)
    {
        return;
    }
    if (buttons == 0x90F)
    {
        g_pending_game_state = 4;
        pad->feedback91 = 0;
        pad->feedback92 = 0;
        pad->feedback13f = 0;
        pad->feedback140 = 0;
        akao_cmd_98_9a_9c_9e(0);
        func_800A3904(0, 0x3C, 0x7F);
        return;
    }
    if (D_8010AE78 != 0)
    {
        D_80122710 = 1;
        return;
    }
    if (D_80122710 != 0)
    {
        D_80122710 = 0;
        func_8008C7A8();
    }
    if (D_801227C8 != 0)
    {
        func_800AA858(arg0);
        return;
    }
    actor = D_800FDF58;
    i = 0;
    if (!(actor->flags & 0x1FF))
    {
        do
        {
            if (D_800FDF58[i].presence != 0xFF && D_800FDF58[i].state == 0x9A)
            {
                return;
            }
            i++;
            actor++;
        } while (i < 3);
        if (func_8006751C(0) == -1 && D_800F2298 == 0 && D_800F229C == 0 &&
            g_field_return_to_title_prompt_state == 0 && D_80122714 == 0 && func_800B0850() == 0)
        {
            if (D_801227B8[0] != 0xFF && pad->connected == 0xFF)
            {
                func_800AA570((void *)0x80170000, 0);
            }
            if (D_801227B9 != 0xFF && pad->connected2 == 0xFF)
            {
                func_800AA570((void *)0x80170000, 1);
            }
            if (cdrom_get_error_status() != 0)
            {
                D_80122984 = 1;
                pad->feedback91 = 0;
                pad->feedback92 = 0;
                pad->feedback13f = 0;
                pad->feedback140 = 0;
                func_800AA7A4();
                return;
            }
            D_801227B8[0] = pad->connected;
            D_801227B8[1] = pad->connected2;
            if (D_8012291C != 0)
            {
                if (func_8005B218() == 0)
                {
                    mask = 0x800;
                    if (g_pad_input == 0x800)
                    {
                        goto open_menu;
                    }
                    if ((g_pad_ctx->inject_flags & 0x80) && g_pad_ctx->inject_enable != 0)
                    {
                        injected = g_pad_input_inject;
                        goto check_injected;
                    }
                }
            }
            else
            {
                mask = 0x10;
                if (g_pad_input == 0x800 || g_pad_input == 0x10)
                {
                    goto open_menu;
                }
                if ((g_pad_ctx->inject_flags & 0x80) && g_pad_ctx->inject_enable != 0)
                {
                    injected = g_pad_input_inject;
                    if (injected == 0x800)
                    {
                        goto open_menu;
                    }
check_injected:
                    if (injected == mask)
                    {
open_menu:
                        func_800AA570((void *)0x80170000, 0);
                    }
                }
            }
            func_800AA498();
            if (g_pad_input & 0x80)
            {
                i = 0;
                if (D_80122980 != 0)
                {
                    index = i;
                    absent = 0xFF;
                    slot = D_80105AE0;
                    actor2 = D_800FDF58;
                    do
                    {
                        if (actor2->presence != absent && slot->value >= 0x14)
                        {
                            i++;
                        }
                        slot++;
                        index++;
                        actor2++;
                    } while (index < 13);
                    if (i < 5)
                    {
                        func_800AEE28();
                        return;
                    }
                    func_800A3938(0x78, 0x80);
                }
            }
        }
    }
}
