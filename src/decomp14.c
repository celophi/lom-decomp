#include "common.h"

typedef struct
{
    u8   pad0[0x40];
    s32  unk40;      /* 0x40 - base scale value */
    u8   pad1[0x09]; /* 0x44 - 0x4C */
    u8   unk4D;      /* 0x4D - pan table index */
} AkaoXaTracker;

extern AkaoXaTracker g_akao_xa_tracker;
extern s32 g_akao_xa_pan_current;
extern u32 D_8004F754;
extern u32 D_8004F7B8;
extern s16 D_8003D47C;
extern s16 D_8003D37C[];

void spu_set_voice_volume(s32 voice, u32 vol_l, u32 vol_r, s32 scale);
void spu_set_voice_pitch(s32 voice, u32 pitch);
void spu_set_voice_start_addr(s32 voice, u32 addr);
void spu_set_voice_repeat_addr(s32 voice, u32 addr);
void spu_set_voice_attack(s32 voice, s32 attack_shift, u32 mode_bits);
void spu_set_voice_decay_shift(s32 voice, s32 decay_shift);
void spu_set_voice_sustain_level(s32 voice, s32 sustain_level);
void spu_set_voice_sustain_mode(s32 voice, s32 sustain_bits, u32 mode_bits);
void spu_set_voice_release_mode(s32 voice, s32 release_shift, u32 mode_bit);

/**
 * @see decomp.me (100%)
 */
void func_8002D4D8(s32 voice, s32 mode, u32 start, u32 end)
{
    s16 vol_l;
    s16 vol_r;

    if (D_8004F754 & 2)
    {
        s32 t = (g_akao_xa_pan_current * D_8003D47C) >> 16;
        vol_r = t;
        vol_l = t;
    }
    else if (mode == 1)
    {
        vol_r = 0;
        vol_l = (u32)g_akao_xa_pan_current >> 1;
    }
    else if (mode == 2)
    {
        vol_l = 0;
        vol_r = (u32)g_akao_xa_pan_current >> 1;
    }
    else if (mode == 3)
    {
        s32 t = (g_akao_xa_pan_current >> 1) << 16;
        vol_r = (t >> 17) + (t >> 18);
        vol_l = vol_r;
    }
    else
    {
        s32 pan = g_akao_xa_tracker.unk4D;
        s32 base = g_akao_xa_tracker.unk40;
        vol_l = (u32)(base * D_8003D37C[pan]) >> 16;
        pan ^= 0xFF;
        vol_r = (u32)(base * D_8003D37C[pan]) >> 16;
    }

    spu_set_voice_volume(voice, vol_l, vol_r, 0);
    spu_set_voice_pitch(voice, D_8004F7B8);
    spu_set_voice_start_addr(voice, start);
    spu_set_voice_repeat_addr(voice, end);
    spu_set_voice_attack(voice, 0, 1);
    spu_set_voice_decay_shift(voice, 0xF);
    spu_set_voice_sustain_level(voice, 0xF);
    spu_set_voice_sustain_mode(voice, 0x7F, 3);
    spu_set_voice_release_mode(voice, 6, 3);
}
