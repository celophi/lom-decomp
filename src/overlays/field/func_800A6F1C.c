#include "common.h"

/** @brief Packed actor addresses and restored state in a 0x23C-byte record. */
typedef struct
{
    s32 unk0;
    s32 unk4;
    s32 unk8;
    u8 padC[0x48 - 12];
    s16 unk48;
    u8 pad4A[0x23C - 0x4A];
} Actor;
/** @brief Saved pad-context counter used when resuming the field. */
typedef struct
{
    u8 pad0[0x315C];
    s32 unk315C;
} Pad;
extern void bcopy(void *, void *, s32);
extern void field_begin_return_to_title_prompt_close(void);
extern void field_set_scene_parameters(s32, s32, s32, s32, s32, s32);
extern void func_800A3938(s32, s32);
extern void func_800AA90C(s32);
extern s32 func_800ADEEC(void);
extern void func_800ADF34(void);
extern u8 D_800FD818[], D_8011F430[];
extern Actor D_80105AE0[];
extern s32 D_80115888, D_80115898, D_8011589C, D_801178B0, D_801178BC, D_801178C0;
extern s32 D_801226D8, D_80122828, g_field_return_to_title_prompt_state, g_pad_input,
    g_pending_game_state;
extern Pad *g_pad_ctx;
/**
 * @brief Process the return-to-title choice or restore the saved field scene.
 */
void func_800A6F1C(void)
{
    volatile Actor *actor;
    s32 prompt_state;
    s32 resume_count;
    s32 actor_index;
    s32 packed, source, copied;
    unsigned low_mask, high_mask;
    s16 sentinel;
    u8 *actor_flags;

    u8 *state = (u8 *)0x801ED600;

    if (!(D_80122828 & 7))
    {
        prompt_state = g_field_return_to_title_prompt_state - 1;
        g_field_return_to_title_prompt_state = prompt_state;
        if (prompt_state == 0)
        {
            if (D_801226D8 != 0)
            {
                g_field_return_to_title_prompt_state = 1;
                g_pending_game_state = 2;
                return;
            }
            bcopy(D_8011F430, g_pad_ctx, 0x3268);
            resume_count = g_pad_ctx->unk315C;
            if (resume_count != -1)
            {
                g_pad_ctx->unk315C = (s32)(resume_count + 1);
            }
            state[0x13F] = 0;
            state[0x91] = 0;
            state[0x140] = 0;
            state[0x92] = 0;
            func_800AA90C(0);
            field_set_scene_parameters(D_801178B0, D_801178BC, D_80115888, D_80115898, D_801178C0,
                                       D_8011589C);
            actor_index = 0;
            low_mask = 0xFFFFFF;
            high_mask = 0xFF000000;
            sentinel = 0xFF;
            actor_flags = D_800FD818;
            actor = D_80105AE0;
            do
            {
                /* Preserve both address reads before updating the packed fields. */
                packed = actor->unk8;
                source = *(volatile s32 *)&actor->unk0;
                copied = *(volatile s32 *)&actor->unk0;
                actor->unk8 = (packed & high_mask) | (source & low_mask);
                actor->unk4 = copied & low_mask;
                if (*actor_flags & 1)
                {
                    actor->unk48 = sentinel;
                }
                actor_flags += 0x268;
                actor_index += 1;
                actor++;
            } while (actor_index < 3);
        }
    }
    else if (func_800ADEEC() == 0)
    {
        if (g_pad_input & 0xA20)
        {
            func_800A3938(0x7E, 0x80);
            func_800ADF34();
            field_begin_return_to_title_prompt_close();
            return;
        }
        if (g_pad_input & 0xF100)
        {
            func_800A3938(0x7D, 0x80);
            D_801226D8 ^= 1;
        }
    }
}
