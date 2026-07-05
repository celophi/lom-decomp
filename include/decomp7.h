#ifndef _DECOMP7_H
#define _DECOMP7_H

#include "common.h"
#include "psyq/libgte.h"
#include "psyq/libgpu.h"
#include "scene_state.h"

/** @brief Fixed-address scene-selection state block; see S_801ED480. */
#define SCENE_STATE ((S_801ED480*)0x801ED480)

/** @brief One 0x7CC4-byte half of the field overlay's double-buffered render context. */
typedef struct obj_struct
{
  u8 pad0[0x4040];
  DISPENV disp_env;   /**< 0x4040 */
  DRAWENV draw_env;   /**< 0x4054 */
  u8 pad2[0x40B8 - 0x40B0];
  u32 unk40B8;
  u8 pad1[0x7CC4 - 0x40BC];
} FieldRenderHalf;

typedef struct {
    u8   _pad0[0x406A];                 /* padding up to offset 0x406A */
    u8   unk406A;                       /* offset 0x406A */
    u8   _pad1[0x40B0 - 0x406B];        /* padding to offset 0x40B0 */
    s16  unk40B0;                       /* offset 0x40B0 */
    s16  unk40B2;                       /* offset 0x40B2 */
    s16  unk40B4;                       /* offset 0x40B4 */
    s16  unk40B6;                       /* offset 0x40B6 */
    u8   _pad2[0xBD2E - 0x40B8];        /* padding to offset 0xBD2E */
    u8   unkBD2E;                       /* offset 0xBD2E */
    u8   _pad3[0xBD74 - 0xBD2F];        /* padding to offset 0xBD74 */
    s16  unkBD74;                       /* offset 0xBD74 */
    s16  unkBD76;                       /* offset 0xBD76 */
    s16  unkBD78;                       /* offset 0xBD78 */
    s16  unkBD7A;                       /* offset 0xBD7A */
} ArgStruct;

typedef struct {
    u8   _pad0[0x7CBF];                /* padding up to offset 0x7CBF */
    u8   unk7CBF;                      /* offset 0x7CBF */
    u32  unk7CC0;                      /* offset 0x7CC0 */
    u8   _pad2[0xF983 - 0x7CC4];       /* padding to offset 0xF983 */
    u8   unkF983;                      /* offset 0xF983 */
    u32  unkF984;                      /* offset 0xF984 */
} ArgStruct2;

extern u32 *FUN_80015c28(void);
extern void akao_cmd_f0(void);
extern void akao_cmd_f1(void);
void field_run_frame_loop(void*);
void func_80015F88(void*);
extern void func_80067EB4(s32, s32, s32, s32);
extern void func_8009AFE0(s32, s32, u32, s32, s32, s32);
extern void func_800A379C(void);
extern s32 g_scene_mode;
extern s32 g_field_entry_flag;
extern u32 D_8003EC88;
extern s32 g_layout_flag;
extern s32 g_layout_option;
extern s32 g_layout_sub_mode;
extern s32 g_pending_game_state;
extern s32 D_801158A4;
extern void *D_800473F4;
extern void *D_800473EC;
extern s32 D_80035248;
extern s32 D_800473E8;
extern s32 D_80047400;
extern s32 D_80047404;
extern s32 D_80047408;

#endif