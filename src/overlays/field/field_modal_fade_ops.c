#include "common.h"

/*
 * Modal fade transitions. D_8012269C is the field's modal-state code (see
 * field_overlay_launchers.c); the two fade starters here set it to 6 and 7.
 */

typedef struct
{
    u8 unk0;   /* 0x0 */
    u8 unk1;   /* 0x1 */
} StructEC400;

/** @brief Caller-owned block whose handle word at 0x40B8 func_800AB690 refreshes. */
typedef struct
{
    u8 pad0[0x40B8];
    s32 unk40B8; /* 0x40B8 */
} ArgA;

void field_set_fade_target_only(s16 red, s16 green, s16 blue, s16 duration);
void func_800A3938(s32 sound_id, s32 pan);
void func_800AE9E0(void);
s32 func_800A88A0(void *arg0, void *arg1, void *arg2, s32 arg3, s32 arg4, s32 arg5, s32 arg6);

extern StructEC400 D_800EC400;
extern s32 D_8011F3D4;
extern s32 D_8012269C;
extern s32 D_801227E0;
extern s32 D_80122990;
extern s32 D_80122B08;

/**
 * @brief Enter modal state 6: fade toward a red tint, play sound 0xC7, and latch arg0.
 * @param arg0 Stored to D_801227E0; func_800AB690 skips its refresh while this is nonzero.
 */
void func_800AB638(s32 arg0)
{
    D_8012269C = 6;
    field_set_fade_target_only(0xC0, 0x80, 0x80, 8);
    func_800A3938(0xC7, 0x80);
    D_801227E0 = arg0;
}

/**
 * @brief Refresh a handle at arg0->unk40B8 unless the global gate is set.
 *
 * Builds the D_800EC400 address from its own first two bytes (low byte plus
 * a high byte shifted by 8) offset by -0x3C, then hands it to func_800A88A0.
 *
 * @param arg0 Block holding the handle at 0x40B8.
 */
void func_800AB690(ArgA *arg0)
{
    s32 handle;
    s32 low;
    s32 offset;
    u8 *base;

    handle = arg0->unk40B8;
    if (D_801227E0 == 0)
    {
        low = D_800EC400.unk0;
        offset = (D_800EC400.unk1 << 8) + (s32)(base = (u8 *)&D_800EC400 - 0x3C);
        handle = func_800A88A0(
            handle,
            arg0,
            (void *)(low + offset),
            4,
            0xA0,
            0x68,
            2);
    }
    arg0->unk40B8 = handle;
}

/**
 * @brief Enter modal state 7: fade toward white, play sound 0x125, and arm a 500-frame counter.
 */
void func_800AB710(void)
{
    D_8012269C = 7;
    field_set_fade_target_only(0xC0, 0xC0, 0xC0, 8);
    func_800AE9E0();
    func_800A3938(0x125, 0x80);
    D_8011F3D4 = 0;
    D_80122B08 = 0;
    D_80122990 = 0x1F4;
}
