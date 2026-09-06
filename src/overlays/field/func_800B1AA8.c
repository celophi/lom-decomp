#include "common.h"

typedef struct
{
    u8 pad0[0x404];
    s32 unk404;
    s32 unk408;
    s32 unk40C;
    u8 pad410[0x418 - 0x410];
    u16 unk418;
    u8 unk41A;
    u8 unk41B;
} FieldStateB1AA8;

void func_800BD520(s32 arg0, u32 arg1, s32 arg2);
s32 func_800BD414(s32 arg0, s32 arg1);
void field_set_scene_parameters(s32 arg0, s32 arg1, u32 arg2, s32 arg3, s32 arg4, s32 arg5);

extern FieldStateB1AA8 *D_80122B78;
extern s32 g_pending_game_state;
extern s32 g_layout_sub_mode;
extern s32 g_layout_option;

/**
 * @brief Apply the pending field scene state or dispatch the current scene parameters.
 */
void func_800B1AA8(void)
{
    u16 state;

    func_800BD520(0, 0xFE2, 0);
    state = D_80122B78->unk418;
    switch (state)
    {
    case 0xFFFE:
        g_pending_game_state = 4;
        D_80122B78->unk404 = 0xFFFF;
        g_layout_sub_mode = -1;
        g_layout_option = -1;
        return;
    case 0xFFFF:
        D_80122B78->unk404 = state;
        g_pending_game_state = 1;
        g_layout_sub_mode = -1;
        g_layout_option = -1;
        if (func_800BD414(0, 0xFFF) != 0)
        {
            g_pending_game_state = 0;
            D_80122B78->unk418 = 1;
            D_80122B78->unk404 = 0;
        }
        return;
    default:
        field_set_scene_parameters(D_80122B78->unk418, D_80122B78->unk41A, D_80122B78->unk41B & 0x1F, D_80122B78->unk404, D_80122B78->unk408,
                                   D_80122B78->unk40C);
        break;
    }
}
