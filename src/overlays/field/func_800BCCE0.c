#include "field_script.h"
/** @brief Pending layout transition state with overlapping control fields at 0x418. */
typedef struct
{
    u8 pad[0x404];
    s32 layout, option, third_selector;
    u8 pad410[8];
    union
    {
        s32 word;
        struct
        {
            s16 id;
            s8 mode;
            u8 flags;
        } fields;
    } control;
} State;
extern State *D_80122B78;
extern s32 g_layout_flag;
extern s32 g_layout_option;

/**
 * @brief Set pending layout selectors and reset execution to script record zero.
 * @param transition_id Transition identifier stored in the control word's low halfword.
 * @param mode Mode byte stored at offset 0x41A.
 * @param selectors Three packed selector bytes; 0xFE and 0xFF select sentinel behavior.
 * @param flags Five-bit transition flags stored at control bits 24 through 28.
 * @note 100% match with GCC 2.7.2 CDK: 109 instructions, 436 bytes.
 */
void func_800BCCE0(s16 transition_id, s8 mode, s32 selectors, s32 flags)
{
    s32 layout;
    s32 third_selector;
    s32 option;

    State *state = D_80122B78;
    option = selectors & 0xFF;
    state->control.word = (s32)(state->control.word | 0x40000000);
    state->control.fields.mode = mode;
    layout = (selectors >> 8) & 0xFF;
    third_selector = (selectors >> 0x10) & 0xFF;
    state->control.fields.id = transition_id;
    switch (option)
    {
    case 0xFE:
        D_80122B78->option = -2;
        break;
    case 0xFF:
        g_layout_option = -1;
        D_80122B78->option = -1;
        break;
    default:
        if (option == g_layout_option)
        {
            D_80122B78->option = -2;
        }
        else
        {
            D_80122B78->option = option;
        }
        break;
    }
    switch (layout)
    {
    case 0xFE:
        D_80122B78->layout = -2;
        break;
    case 0xFF:
        D_80122B78->layout = -1;
        break;
    default:
        if (layout == g_layout_flag)
        {
            D_80122B78->layout = -1;
        }
        else
        {
            D_80122B78->layout = layout;
        }
        break;
    }
    switch (third_selector)
    {
    case 0xFE:
        D_80122B78->third_selector = -2;
        break;
    case 0xFF:
        D_80122B78->third_selector = -1;
        break;
    default:
        D_80122B78->third_selector = third_selector;
        break;
    }
    D_80122B78->control.word =
        (s32)((D_80122B78->control.word & 0xE0FFFFFF) | ((flags & 0x1F) << 0x18));
    g_field_script->active_record = 0;
    g_field_script->status.word = (s32)(g_field_script->status.word & 0x7FFFFFFF);
    ((FieldScriptRecord *)((u8 *)g_field_script + ((g_field_script->active_record * 3) << 2)))->pc =
        0;
}
