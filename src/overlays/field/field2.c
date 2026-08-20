#include "common.h"

void field_clear_actor_slots(void);                        /* extern */
void field_initialize_actor_slots(void);                   /* extern */
void field_reset_fade_state(void);                         /* extern */
void func_80067AA4(void);                                  /* extern */
void func_8006A324(void);                                  /* extern */
void func_80083948(void);                                  /* extern */
void func_8008396C(void);                                  /* extern */
void func_80084240(void);                                  /* extern */
void func_80084524(void);                                  /* extern */
void func_80086F20(void);                                  /* extern */
void func_80091410(void);                                  /* extern */
void func_800970B0(void);                                  /* extern */
void func_800A255C(void);                                  /* extern */
void func_800A2DFC(void);                                  /* extern */
void func_800A3EBC(void);                                  /* extern */
void func_800A43C0(void);                                  /* extern */
void func_800A6204(void);                                  /* extern */
void func_800A8CFC(void);                                  /* extern */
void func_800AA02C(void);                                  /* extern */
void func_800AA824(void);                                  /* extern */
void func_800AA90C(s32);                                 /* extern */
void func_800ADE2C(void);                                  /* extern */
void func_800B0094(s32);                                 /* extern */
void func_800B01FC(s32);                               /* extern */

/*
 * The extern block below is ordered (and carries a few duplicate/unused
 * declarations) to reproduce the original translation unit's symbol order.
 * gcc 2.7.2 keys constant materialisation off declaration order here, so the
 * set and sequence are required to match - do not sort, dedup, or prune.
 */
extern s32 D_800F229C[];
extern s32 D_800F22B0[];
extern s32 D_8010AE54[];
extern s32 D_8010AE78[];
extern s32 D_8010D034[];
extern s32 D_8010D038[];
extern s32 D_80115894[];
extern s32 D_801158A0[];
extern s32 D_801178C8[];
extern s32 D_8011F428[];
extern s32 D_80122710[];
extern s32 D_801227DC[];
extern s32 D_801227E8[];
extern s32 D_8011F3AC[];
extern s32 D_800F22C0[];
extern s32 D_801227F0[];
extern s32 g_field_return_to_title_prompt_delay[];
extern s32 g_field_return_to_title_prompt_state[];
extern s32 g_previousGameState[];
extern s32 g_field_audio_timer[];
extern u16 g_music_track_index[];
extern u8 g_music_track_table[];
extern s32 D_800473F8[];
extern s32 D_800F2288[];
extern s32 D_800F2298[];
extern s32 D_800F22C0[];
extern s32 D_800FE754[];
extern s32 D_80105764[];
extern s32 D_8010AE48[];
extern s32 D_8011F3AC[];
extern s32 D_8012269C[];
extern s32 D_801227C8[];

/**
 * @brief Reset every field subsystem for a freshly entered scene.
 * @param arg0 Render half to install as the active context.
 * @note Byte-matching under the field2.c toolchain (gcc272_cdk plus
 *       -fno-schedule-insns). The statement-expression in the music-track
 *       test is required to match: the otherwise-unused `tp` forces the table
 *       base into its own register before the indexed load, reproducing the
 *       original codegen. See working/field_initialize_subsystems/STATUS.md.
 * @see decomp.me (100%) https://decomp.me/scratch/fKHnZ
 */
void field_initialize_subsystems(s32 arg0)
{
    s32 prev;
    s32 base;

    base = 0x80158000;
    D_8010D038[0] = 0x80140000;
    D_8010D034[0] = base;
    D_801227E8[0] = 0;
    func_800B0094(base);
    func_800A8CFC();
    func_8006A324();
    D_800F22B0[0] = arg0;
    D_801158A0[0] = 0;
    D_801178C8[0] = 0;
    func_80083948();
    func_8008396C();
    field_initialize_actor_slots();
    D_8010AE78[0] = 0;
    D_80122710[0] = 0;
    func_800AA90C(0);
    field_clear_actor_slots();
    func_80067AA4();
    func_80084240();
    func_80084524();
    func_800A255C();
    func_80091410();
    func_800A2DFC();
    func_800AA824();
    field_reset_fade_state();
    func_800970B0();
    D_801227DC[0] = 0;
    func_800A6204();
    D_8011F3AC[0] = 0;
    D_800F22C0[0] = 0;
    D_800F229C[0] = 0;
    g_field_return_to_title_prompt_state[0] = 0;
    g_field_return_to_title_prompt_delay[0] = 0;
    D_8010AE48[0] = 0;
    D_8010AE54[0] = 0;
    D_8012269C[0] = 0;
    D_800F2298[0] = 0;
    D_8011F428[0] = 0;
    D_801227F0[0] = 0;
    g_field_audio_timer[0] = 0;
    prev = g_previousGameState[0];
    if ((prev == 1) && (({ u16 *ip = g_music_track_index; u8 *tp = g_music_track_table; g_music_track_table[*ip]; }) != 0xFF))
    {
        D_80115894[0] = prev;
    }
    else
    {
        D_80115894[0] = 0;
    }
    func_800B01FC(prev);
    func_800AA02C();
    func_80086F20();
    func_800A3EBC();
    func_800A43C0();
    func_800ADE2C();
}
