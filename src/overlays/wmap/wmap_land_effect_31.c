#include "wmap_model_render.h"
#include "wmap_land_effect_31.h"
#include "wmap_sprite_render.h"
#include "wmap_view_effects.h"
#include "wmap_resource_support.h"
#include "wmap_main.h"
#include "wmap_effect_primitives.h"
#include "wmap_sequence_runtime.h"
#include "wmap_effect_resources.h"
#include "cdrom.h"
#include "sdk/libgte.h"
#include "wmap_step_sequence.h"
#include "wmap_cells.h"

void wmap_land_effect_31_sequence_20_step_02(void);
void wmap_land_effect_31_sequence_20_step_04(void);
void wmap_land_effect_31_sequence_19_step_02(void);
void wmap_land_effect_31_sequence_19_step_04(void);
void wmap_land_effect_31_sequence_5_step_04(void);
void wmap_land_effect_31_sequence_6_step_04(void);
void wmap_land_effect_31_sequence_15_step_02(void);
void wmap_land_effect_31_sequence_15_step_04(void);
void wmap_land_effect_31_sequence_16_step_02(void);
void wmap_land_effect_31_sequence_16_step_04(void);
void wmap_land_effect_31_sequence_17_step_02(void);
void wmap_land_effect_31_sequence_18_step_04(void);
s32 wmap_land_effect_31_run_timeline(s32 reset);
s32 wmap_land_effect_31_run_sequence_1(s32 reset);
s32 wmap_land_effect_31_run_sequence_2(s32 reset);
s32 wmap_land_effect_31_run_sequence_3(s32 reset);
s32 wmap_land_effect_31_run_sequence_8(s32 reset);
s32 wmap_land_effect_31_run_sequence_9(s32 reset);
s32 wmap_land_effect_31_run_sequence_10(s32 reset);
s32 wmap_land_effect_31_run_sequence_11(s32 reset);
s32 wmap_land_effect_31_run_sequence_12(s32 reset);
s32 wmap_land_effect_31_run_sequence_13(s32 reset);
s32 wmap_land_effect_31_run_sequence_17(s32 reset);
s32 wmap_land_effect_31_run_sequence_18(s32 reset);
s32 wmap_land_effect_31_run_sequence_20(s32 reset);
s32 wmap_land_effect_31_run_sequence_4(s32 reset);
s32 wmap_land_effect_31_run_sequence_5(s32 reset);
s32 wmap_land_effect_31_run_sequence_6(s32 reset);
s32 wmap_land_effect_31_run_sequence_7(s32 reset);
s32 wmap_land_effect_31_run_sequence_14(s32 reset);
s32 wmap_land_effect_31_run_sequence_15(s32 reset);
s32 wmap_land_effect_31_run_sequence_16(s32 reset);
s32 wmap_land_effect_31_run_sequence_19(s32 reset);
void wmap_land_effect_31_sequence_1_step_04(void);
void wmap_land_effect_31_sequence_2_step_02(void);
void wmap_land_effect_31_sequence_3_step_02(void);
void wmap_land_effect_31_sequence_3_step_04(void);
void wmap_land_effect_31_step_02(void);
void wmap_land_effect_31_step_03(void);
void wmap_land_effect_31_step_04(void);
void wmap_land_effect_31_step_05(void);
void wmap_land_effect_31_sequence_1_step_02(void);
void wmap_land_effect_31_sequence_5_step_02(void);
void wmap_land_effect_31_sequence_6_step_02(void);

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s16 field_04;
} WmapShort3;

typedef struct
{
    s16 field_00;
    s16 field_02;
    u16 field_04;
    s16 field_06;
} WmapShort4;

typedef struct
{
    s32 field_00;
    s32 field_04;
    s32 field_08;
} WmapInt3;

typedef struct
{
    s32 words[9];
} WmapBlock36;

typedef struct
{
    s32 field_00;
    s32 field_04;
    s32 field_08;
    s32 field_0C;
    s32 field_10;
    s32 field_14;
    s32 field_18;
    s32 field_1C;
    s32 field_20;
    s32 state_24;
    s32 field_28;
    s32 field_2C;
    s32 field_30;
    s32 field_34;
    s32 field_38;
    s32 field_3C;
    s32 field_40;
    s32 field_44;
    s32 field_48;
    s32 tail_state;
} WmapState;

typedef void (*WmapHandler)(void);

/** @brief World-map 0x14-byte slot: only the leading halfword is cleared here. */
typedef struct
{
    s16 field_00;
    u8 pad_02[0x12];
} WmapSlot14;

extern WmapHandler D_800D7474[];
extern WmapHandler D_800D748C[];
extern WmapHandler D_800D753C[];
extern WmapHandler D_800D7554[];
extern WmapHandler D_800D7564[];
extern WmapHandler D_800D757C[];
extern WmapHandler D_800D7594[];
extern WmapHandler D_800D75AC[];
extern WmapHandler D_800D75C4[];
extern WmapHandler D_800D75D4[];
extern WmapHandler D_800D75E4[];
extern WmapHandler D_800D75F4[];
extern WmapHandler D_800D760C[];
extern WmapHandler D_800D7624[];
extern WmapHandler D_800D763C[];
extern WmapHandler D_800D7654[];
extern WmapHandler D_800D766C[];
extern WmapHandler D_800D7684[];
extern WmapHandler D_800D769C[];
extern WmapHandler D_800D76AC[];
extern WmapHandler D_800D76C4[];
extern WmapHandler D_800D76DC[];
extern WmapBlock36 D_800D06BC;
extern WmapBlock36 D_800D9240;
extern WmapShort3 D_800DCEB8;
extern s32 g_wmap_load_buffer[];
extern s32* g_wmap_effect_model_pack_1;
extern s32* g_wmap_effect_model_pack_2;
extern s32* g_wmap_effect_model_pack_3;
extern s32* D_8011CF2C;
extern s32* D_8011CF30;
extern s32* D_8011CF34;
extern s32* D_8011CF38;
extern s32* D_8011CF3C;
extern s32 g_wmap_selected_artifact;
extern u8 g_wmap_animation_bank_0;
extern u8 g_wmap_animation_bank_2;
extern s32 D_80139234;
extern WmapInt3 D_80139200;
extern WmapShort3 D_80139210;
extern s32 D_8013923C;
extern s32 D_80139240;
extern s32 D_8013924C;
extern s32 D_80139260;
extern s32 D_80139264;
extern s32 D_80139268;
extern s32 D_8013926C;
extern s32 g_wmap_view_mode;
extern WmapInt3 D_80139968;
extern s32 D_8013B29C;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern s32 D_80182DE4;
extern s32 D_801B24B4;
extern s32 D_801B25D8;
extern s32 D_801B25DC;
extern s32 D_801B25E0;
extern u32 g_wmap_land_effect_31_step;
extern s32 g_wmap_land_effect_31_timer;
extern u32 g_wmap_land_effect_31_timeline_step;
extern s32 g_wmap_land_effect_31_timeline_timer;
extern u32 g_wmap_land_effect_31_sequence_1_step;
extern s32 g_wmap_land_effect_31_sequence_1_timer;
extern u32 g_wmap_land_effect_31_sequence_2_step;
extern s32 g_wmap_land_effect_31_sequence_2_timer;
extern u32 g_wmap_land_effect_31_sequence_3_step;
extern s32 g_wmap_land_effect_31_sequence_3_timer;
extern u32 g_wmap_land_effect_31_sequence_4_step;
extern s32 g_wmap_land_effect_31_sequence_4_timer;
extern u32 g_wmap_land_effect_31_sequence_5_step;
extern s32 g_wmap_land_effect_31_sequence_5_timer;
extern u32 g_wmap_land_effect_31_sequence_6_step;
extern s32 g_wmap_land_effect_31_sequence_6_timer;
extern u32 g_wmap_land_effect_31_sequence_7_step;
extern s32 g_wmap_land_effect_31_sequence_7_timer;
extern u32 g_wmap_land_effect_31_sequence_8_step;
extern s32 g_wmap_land_effect_31_sequence_8_timer;
extern u32 g_wmap_land_effect_31_sequence_9_step;
extern s32 g_wmap_land_effect_31_sequence_9_timer;
extern u32 g_wmap_land_effect_31_sequence_10_step;
extern s32 g_wmap_land_effect_31_sequence_10_timer;
extern u32 g_wmap_land_effect_31_sequence_11_step;
extern s32 g_wmap_land_effect_31_sequence_11_timer;
extern u32 g_wmap_land_effect_31_sequence_12_step;
extern s32 g_wmap_land_effect_31_sequence_12_timer;
extern u32 g_wmap_land_effect_31_sequence_13_step;
extern s32 g_wmap_land_effect_31_sequence_13_timer;
extern u32 g_wmap_land_effect_31_sequence_14_step;
extern s32 g_wmap_land_effect_31_sequence_14_timer;
extern u32 g_wmap_land_effect_31_sequence_15_step;
extern s32 g_wmap_land_effect_31_sequence_15_timer;
extern u32 g_wmap_land_effect_31_sequence_16_step;
extern s32 g_wmap_land_effect_31_sequence_16_timer;
extern u32 g_wmap_land_effect_31_sequence_17_step;
extern s32 g_wmap_land_effect_31_sequence_17_timer;
extern u32 g_wmap_land_effect_31_sequence_18_step;
extern s32 g_wmap_land_effect_31_sequence_18_timer;
extern u32 g_wmap_land_effect_31_sequence_19_step;
extern s32 g_wmap_land_effect_31_sequence_19_timer;
extern u32 g_wmap_land_effect_31_sequence_20_step;
extern s32 g_wmap_land_effect_31_sequence_20_timer;
extern void wmap_land_effect_31_sequence_4_step_02(void);
extern void wmap_land_effect_31_sequence_4_step_04(void);
extern void wmap_land_effect_31_sequence_7_step_02(void);
extern void wmap_land_effect_31_sequence_8_step_02(void);
extern void wmap_land_effect_31_sequence_9_step_02(void);
extern void wmap_land_effect_31_sequence_10_step_02(void);
extern void wmap_land_effect_31_sequence_10_step_04(void);
extern void wmap_land_effect_31_sequence_11_step_02(void);
extern void wmap_land_effect_31_sequence_11_step_04(void);
extern void wmap_land_effect_31_sequence_12_step_02(void);
extern void wmap_land_effect_31_sequence_12_step_04(void);
extern void wmap_land_effect_31_sequence_13_step_02(void);
extern void wmap_land_effect_31_sequence_13_step_04(void);
extern s32* D_8011CF40;
extern s32 wmap_land_effect_31_run_timeline(s32);
extern s32 wmap_land_effect_31_run_sequence_1(s32);
extern s32 wmap_land_effect_31_run_sequence_2(s32);
extern s32 wmap_land_effect_31_run_sequence_3(s32);
extern s32 wmap_land_effect_31_run_sequence_4(s32);
extern s32 wmap_land_effect_31_run_sequence_5(s32);
extern void wmap_land_effect_31_sequence_5_step_04(void);
extern s32 wmap_land_effect_31_run_sequence_6(s32);
extern void wmap_land_effect_31_sequence_6_step_04(void);
extern s32 wmap_land_effect_31_run_sequence_7(s32);
extern s32 wmap_land_effect_31_run_sequence_8(s32);
extern s32 wmap_land_effect_31_run_sequence_9(s32);
extern s32 wmap_land_effect_31_run_sequence_10(s32);
extern s32 wmap_land_effect_31_run_sequence_11(s32);
extern s32 wmap_land_effect_31_run_sequence_12(s32);
extern s32 wmap_land_effect_31_run_sequence_13(s32);
extern s32 wmap_land_effect_31_run_sequence_14(s32);
extern s32 wmap_land_effect_31_run_sequence_15(s32);
extern void wmap_land_effect_31_sequence_15_step_02(void);
extern void wmap_land_effect_31_sequence_15_step_04(void);
extern s32 wmap_land_effect_31_run_sequence_16(s32);
extern void wmap_land_effect_31_sequence_16_step_02(void);
extern void wmap_land_effect_31_sequence_16_step_04(void);
extern s32 wmap_land_effect_31_run_sequence_17(s32);
extern void wmap_land_effect_31_sequence_17_step_02(void);
extern s32 wmap_land_effect_31_run_sequence_18(s32);
extern void wmap_land_effect_31_sequence_18_step_02(void);
extern void wmap_land_effect_31_sequence_18_step_04(void);
extern s32 wmap_land_effect_31_run_sequence_19(s32);
extern void wmap_land_effect_31_sequence_19_step_02();
extern void wmap_land_effect_31_sequence_19_step_04();
extern s32 wmap_land_effect_31_run_sequence_20(s32);
extern void wmap_land_effect_31_sequence_20_step_02(void);
extern void wmap_land_effect_31_sequence_20_step_04();

typedef struct
{
    s16 field_00;
    s16 field_02;
    u16 field_04;
    s16 field_06;
} WmapPair;

extern u8 g_wmap_animation_bank_1[];
extern WmapPair D_801B3118;

extern VECTOR g_wmap_camera_translation;


extern WmapAnimationSlot g_wmap_actor_animations[];

extern WmapScreenPosition g_wmap_focus_screen_position;

extern s32* g_wmap_effect_params;

extern WmapSlot14 g_wmap_actor_motions[];

extern SVECTOR D_801B2498;
extern SVECTOR D_801B2490;
extern SVECTOR D_801B2670;
extern SVECTOR D_801B2678;
extern SVECTOR D_801B3120;

void wmap_land_effect_31_timeline_step_17(void)
{
    g_wmap_view_mode = -1;
    g_wmap_backdrop_target_level = 0;
    D_80182D74.field_00 = 0xB4;
    D_80182D74.field_01 = 0;
    D_80182D74.field_02 = 0x96;
    D_80182D80.field_00 = 0xB4;
    D_80182D80.field_01 = 0;
    D_80182D80.field_02 = 0x96;
    D_80182D8C.field_00 = 0xB4;
    D_80182D8C.field_01 = 0;
    D_80182D8C.field_02 = 0x96;
    D_80182D94.field_00 = 0xB4;
    D_80182D94.field_01 = 0;
    D_80182D94.field_02 = 0x96;
    g_wmap_land_effect_31_timeline_timer = 0x38;
    g_wmap_land_effect_31_timeline_step++;
}

void wmap_land_effect_31_timeline_step_19(void)
{
    g_wmap_transition_mesh_hidden = 1;
    wmap_start_sequence(wmap_land_effect_31_run_sequence_7);
    g_wmap_backdrop_target_level = 0;
    D_80182D74.field_00 = 0xC8;
    D_80182D74.field_01 = 0xC8;
    D_80182D74.field_02 = 0xC8;
    D_80182D80.field_00 = 0xC8;
    D_80182D80.field_01 = 0xC8;
    D_80182D80.field_02 = 0xC8;
    D_80182D8C.field_00 = 0xC8;
    D_80182D8C.field_01 = 0xC8;
    D_80182D8C.field_02 = 0xC8;
    D_80182D94.field_00 = 0xC8;
    D_80182D94.field_01 = 0xC8;
    D_80182D94.field_02 = 0xC8;
    g_wmap_land_effect_31_timeline_timer = 0xF;
    g_wmap_land_effect_31_timeline_step++;
}

void wmap_land_effect_31_timeline_step_23(void)
{
    wmap_start_sequence(wmap_land_effect_31_run_sequence_15);
    wmap_start_sequence(wmap_land_effect_31_run_sequence_16);
    wmap_start_sequence(wmap_land_effect_31_run_sequence_14);
    wmap_start_sequence(wmap_land_effect_31_run_sequence_5);
    wmap_start_sequence(wmap_land_effect_31_run_sequence_4);
    g_wmap_backdrop_target_level = 0;
    D_80182D74.field_00 = 0x60;
    D_80182D74.field_01 = 0xC;
    D_80182D74.field_02 = 0xC8;
    D_80182D80.field_00 = 0x60;
    D_80182D80.field_01 = 0xC;
    D_80182D80.field_02 = 0xC8;
    D_80182D8C.field_00 = 0xA0;
    D_80182D8C.field_01 = 0xA0;
    D_80182D8C.field_02 = 0xA0;
    D_80182D94.field_00 = 0xA0;
    D_80182D94.field_01 = 0xA0;
    D_80182D94.field_02 = 0xA0;
    g_wmap_land_effect_31_timeline_timer = 0x14;
    g_wmap_land_effect_31_timeline_step++;
}

void wmap_land_effect_31_timeline_step_37(void)
{
    g_wmap_backdrop_target_level = 0;
    D_80182D74.field_00 = 0;
    D_80182D74.field_01 = 0x64;
    D_80182D74.field_02 = 0x78;
    D_80182D80.field_00 = 0;
    D_80182D80.field_01 = 0x64;
    D_80182D80.field_02 = 0x78;
    D_80182D8C.field_00 = 0;
    D_80182D8C.field_01 = 0x64;
    D_80182D8C.field_02 = 0x78;
    D_80182D94.field_00 = 0;
    D_80182D94.field_01 = 0x64;
    D_80182D94.field_02 = 0x78;
    D_8013B29C = 0;
    wmap_start_map_tint(0x701050);
    g_wmap_view_mode = 0;
    g_wmap_backdrop_target_level = 0;
    wmap_start_sequence(wmap_land_effect_31_run_sequence_19);
    g_wmap_land_effect_31_timeline_timer = 0x58;
    g_wmap_land_effect_31_timeline_step++;
}

void wmap_land_effect_31_timeline_step_41(void)
{
    wmap_start_sequence(wmap_land_effect_31_run_sequence_6);
    D_80182D74.field_00 = 0;
    D_80182D74.field_01 = 0;
    D_80182D74.field_02 = 0;
    D_80182D80.field_00 = 0;
    D_80182D80.field_01 = 0;
    D_80182D80.field_02 = 0;
    D_80182D8C.field_00 = 0;
    D_80182D8C.field_01 = 0;
    D_80182D8C.field_02 = 0;
    D_80182D94.field_00 = 0;
    D_80182D94.field_01 = 0;
    D_80182D94.field_02 = 0;
    D_800D9240 = D_800D06BC;
    g_wmap_land_effect_31_timeline_timer = 0x78;
    g_wmap_land_effect_31_timeline_step++;
}

/** @brief Configure the effect, reset its resource slots, and advance the sequence. */
void wmap_land_effect_31_sequence_1_step_01(void)
{
    s32 i;

    g_wmap_particle_intensity = 40;
    g_wmap_effect_params[0x1F] = -8;
    g_wmap_effect_params[0x20] = 20;
    g_wmap_effect_params[0x21] = 20;
    g_wmap_effect_params[0x22] = 30;
    g_wmap_effect_params[0x23] = 1;
    g_wmap_effect_params[0x24] = 3900;
    g_wmap_effect_params[0x25] = 60;
    g_wmap_effect_params[0x26] = 15;
    g_wmap_effect_params[0x27] = 2;
    g_wmap_effect_params[0x28] = 10000;
    for (i = 0; i < 40; i++)
    {
        g_wmap_actor_motions[i + g_wmap_effect_params[0x25]].field_00 = 0;
        g_wmap_actor_animations[i + 64].data = g_wmap_animation_bank_1;
    }
    g_wmap_land_effect_31_sequence_1_timer = 40;
    g_wmap_land_effect_31_sequence_1_step++;
    wmap_land_effect_31_sequence_1_step_02();
}

void wmap_land_effect_31_sequence_4_step_02(void)
{
    s32 value;
    s32 timer;

    func_8006AEE0();
    wmap_draw_model(g_wmap_effect_model_pack_1, (D_80139234 / 0x10) & 3, 0xA, 0x36, 0x7940, 0x1001, D_801B24B4, 0, 0, -1);
    D_80139234 += 0x10;
    value = D_801B24B4 + 2;
    D_801B24B4 = value;
    if (value >= 0x82)
    {
        D_801B24B4 = 0x81;
    }
    ((WmapShort4*)&D_801B2490)->field_04 += 0x40;
    timer = g_wmap_land_effect_31_sequence_4_timer - 1;
    g_wmap_land_effect_31_sequence_4_timer = timer;
    if (timer == 0)
    {
        g_wmap_land_effect_31_sequence_4_step++;
    }
}

void wmap_land_effect_31_sequence_4_step_04(void)
{
    s32 value;
    s32 timer;
    WmapShort4* position;

    func_8006AEE0();
    wmap_draw_model(g_wmap_effect_model_pack_1, (D_80139234 / 0x10) & 3, 0xA, 0x36, 0x7940, 0x1001, D_801B24B4, 0, 0, -1);
    value = D_801B24B4 - 2;
    D_801B24B4 = value;
    if (value < 0)
    {
        D_801B24B4 = 0;
    }
    position = (WmapShort4*)&D_801B2490;
    D_80139234 += 0x10;
    timer = g_wmap_land_effect_31_sequence_4_timer - 1;
    g_wmap_land_effect_31_sequence_4_timer = timer;
    position->field_04 += 0x40;
    if (timer == 0)
    {
        g_wmap_land_effect_31_sequence_4_step++;
    }
}

/** @brief Configure the effect and its resource slots, then advance the sequence. */
void wmap_land_effect_31_sequence_5_step_01(void)
{
    s32 i;

    g_wmap_effect_params[0] = 40;
    g_wmap_effect_params[1] = 20;
    g_wmap_effect_params[2] = -210;
    g_wmap_effect_params[3] = 420;
    g_wmap_effect_params[4] = -210;
    g_wmap_effect_params[5] = 420;
    g_wmap_effect_params[6] = 0;
    g_wmap_effect_params[7] = 80;
    g_wmap_effect_params[8] = 129;
    g_wmap_effect_params[9] = 1;
    g_wmap_effect_params[10] = 8;
    g_wmap_effect_params[11] = 80;
    g_wmap_effect_params[12] = 15;
    g_wmap_effect_params[13] = 0;
    D_801B3120 = g_wmap_zero_rotation;
    for (i = 0; i < 40; i++)
    {
        g_wmap_actor_animations[i + 20].data = g_wmap_animation_bank_1;
    }
    g_wmap_land_effect_31_sequence_5_timer = 204;
    g_wmap_land_effect_31_sequence_5_step++;
    wmap_land_effect_31_sequence_5_step_02();
}

/** @brief Configure the effect, reset its resource slots, and advance the sequence. */
void wmap_land_effect_31_sequence_6_step_01(void)
{
    s32 i;

    g_wmap_particle_intensity = 32;
    g_wmap_effect_params[0x15] = -3;
    g_wmap_effect_params[0x16] = 8;
    g_wmap_effect_params[0x17] = 20;
    g_wmap_effect_params[0x18] = 30;
    g_wmap_effect_params[0x19] = 1;
    g_wmap_effect_params[0x1A] = 1500;
    g_wmap_effect_params[0x1B] = 60;
    g_wmap_effect_params[0x1C] = 15;
    g_wmap_effect_params[0x1D] = 1;
    g_wmap_effect_params[0x1E] = 10000;
    for (i = 0; i < 32; i++)
    {
        g_wmap_actor_motions[i + g_wmap_effect_params[0x1B]].field_00 = 0;
        g_wmap_actor_animations[i + 64].data = g_wmap_animation_bank_1;
    }
    g_wmap_land_effect_31_sequence_6_timer = 32;
    g_wmap_land_effect_31_sequence_6_step++;
    wmap_land_effect_31_sequence_6_step_02();
}

void wmap_land_effect_31_sequence_7_step_02(void)
{
    s32 value;
    s32 fade;
    s32 timer;

    value = g_wmap_effect_model_a_position.vz - 0xDAC;
    g_wmap_effect_model_a_position.vz = value;
    if (value < 0x2710)
    {
        g_wmap_effect_model_a_position.vz = 0x2710;
    }
    PushMatrix();
    wmap_set_map_rotation(&g_wmap_effect_model_a_rotation);
    if (g_wmap_effect_fade_a != 0)
    {
        wmap_draw_model(D_8011CF34, 0, 4, 0x35, 0x7800, 1, g_wmap_effect_fade_a, 0, 0, -1);
        fade = g_wmap_effect_fade_a - 1;
        g_wmap_effect_fade_a = fade;
        if (fade < 0)
        {
            g_wmap_effect_fade_a = 0;
        }
    }
    PopMatrix();
    timer = g_wmap_land_effect_31_sequence_7_timer - 1;
    g_wmap_land_effect_31_sequence_7_timer = timer;
    if (timer == 0)
    {
        g_wmap_land_effect_31_sequence_7_step++;
    }
}

void wmap_land_effect_31_sequence_8_step_02(void)
{
    s32 value;
    s32 fade;
    s32 timer;

    value = g_wmap_effect_model_b_position.vz - 0xDAC;
    g_wmap_effect_model_b_position.vz = value;
    if (value < 0x2710)
    {
        g_wmap_effect_model_b_position.vz = 0x2710;
    }
    PushMatrix();
    wmap_set_map_rotation(&g_wmap_effect_model_b_rotation);
    if (g_wmap_effect_fade_b != 0)
    {
        wmap_draw_model(D_8011CF3C, 0, 4, 0x35, 0x7800, 1, g_wmap_effect_fade_b, 0, 0, -1);
        fade = g_wmap_effect_fade_b - 4;
        g_wmap_effect_fade_b = fade;
        if (fade < 0)
        {
            g_wmap_effect_fade_b = 0;
        }
    }
    PopMatrix();
    timer = g_wmap_land_effect_31_sequence_8_timer - 1;
    g_wmap_land_effect_31_sequence_8_timer = timer;
    if (timer == 0)
    {
        g_wmap_land_effect_31_sequence_8_step++;
    }
}

void wmap_land_effect_31_sequence_9_step_02(void)
{
    s32 value;
    s32 fade;
    s32 timer;

    value = g_wmap_effect_model_c_position.vz - 0xDAC;
    g_wmap_effect_model_c_position.vz = value;
    if (value < 0x2710)
    {
        g_wmap_effect_model_c_position.vz = 0x2710;
    }
    PushMatrix();
    wmap_set_map_rotation(&g_wmap_effect_model_c_rotation);
    if (g_wmap_effect_fade_c != 0)
    {
        wmap_draw_model(D_8011CF2C, 0, 4, 0x35, 0x7800, 1, g_wmap_effect_fade_c, 0, 0, -1);
        fade = g_wmap_effect_fade_c - 2;
        g_wmap_effect_fade_c = fade;
        if (fade < 0)
        {
            g_wmap_effect_fade_c = 0;
        }
    }
    PopMatrix();
    timer = g_wmap_land_effect_31_sequence_9_timer - 1;
    g_wmap_land_effect_31_sequence_9_timer = timer;
    if (timer == 0)
    {
        g_wmap_land_effect_31_sequence_9_step++;
    }
}

void wmap_land_effect_31_sequence_10_step_02(void)
{
    s32 value;
    s32 timer;

    wmap_set_map_rotation(&D_801B2498);
    wmap_draw_model(g_wmap_load_buffer, (D_8013923C / 0x10) & 7, 0xA, 0x36, 0x7880, 0x1001, D_80182DE4, 6, -0x18, -1);
    D_8013923C += 0x10;
    value = D_80182DE4 + 4;
    D_80182DE4 = value;
    if (value >= 0x82)
    {
        D_80182DE4 = 0x81;
    }
    D_801B2498.vz += 0x10;
    timer = g_wmap_land_effect_31_sequence_10_timer - 1;
    g_wmap_land_effect_31_sequence_10_timer = timer;
    if (timer == 0)
    {
        g_wmap_land_effect_31_sequence_10_step++;
    }
}

void wmap_land_effect_31_sequence_10_step_04(void)
{
    s32 value;
    s32 timer;

    wmap_set_map_rotation(&D_801B2498);
    wmap_draw_model(g_wmap_load_buffer, (D_8013923C / 0x10) & 7, 0xA, 0x36, 0x7880, 0x1001, D_80182DE4, 6, -0x18, -1);
    value = D_80182DE4 - 0x10;
    D_80182DE4 = value;
    if (value < 0)
    {
        D_80182DE4 = 0;
    }
    D_8013923C += 0x10;
    timer = g_wmap_land_effect_31_sequence_10_timer - 1;
    g_wmap_land_effect_31_sequence_10_timer = timer;
    D_801B2498.vz += 0x10;
    if (timer == 0)
    {
        g_wmap_land_effect_31_sequence_10_step++;
    }
}

void wmap_land_effect_31_sequence_11_step_02(void)
{
    s32 value;
    s32 timer;

    wmap_set_map_rotation(&D_801B3118);
    wmap_draw_model(g_wmap_load_buffer, (D_8013926C / 0x10) & 7, 0xA, 0x36, 0x7880, 0x1001, D_801B25E0, 4, -0x14, -1);
    D_8013926C -= 0x10;
    value = D_801B25E0 + 2;
    D_801B25E0 = value;
    if (value >= 0x82)
    {
        D_801B25E0 = 0x81;
    }
    ((u16*)&D_801B3118)[2] += 0x10;
    timer = g_wmap_land_effect_31_sequence_11_timer - 1;
    g_wmap_land_effect_31_sequence_11_timer = timer;
    if (timer == 0)
    {
        g_wmap_land_effect_31_sequence_11_step++;
    }
}

void wmap_land_effect_31_sequence_11_step_04(void)
{
    s32 value;
    s32 timer;

    wmap_set_map_rotation(&D_801B3118);
    wmap_draw_model(g_wmap_load_buffer, (D_8013926C / 0x10) & 7, 0xA, 0x36, 0x7880, 0x1001, D_801B25E0, 4, -0x14, -1);
    value = D_801B25E0 - 0x80;
    D_801B25E0 = value;
    if (value < 0)
    {
        D_801B25E0 = 0;
    }
    D_8013926C -= 0x10;
    timer = g_wmap_land_effect_31_sequence_11_timer - 1;
    g_wmap_land_effect_31_sequence_11_timer = timer;
    ((u16*)&D_801B3118)[2] += 0x10;
    if (timer == 0)
    {
        g_wmap_land_effect_31_sequence_11_step++;
    }
}

void wmap_land_effect_31_sequence_12_step_02(void)
{
    s32 value;
    s32 timer;

    wmap_set_map_rotation(&g_wmap_effect_model_d_rotation);
    wmap_draw_model(g_wmap_effect_model_pack_3, (D_80139260 / 0x10) & 3, 0xA, 0x36, 0x7900, 0x1001, g_wmap_effect_fade_d, 4, -0x14, -1);
    D_80139260 -= 0x20;
    value = g_wmap_effect_fade_d + 2;
    g_wmap_effect_fade_d = value;
    if (value >= 0x82)
    {
        g_wmap_effect_fade_d = 0x81;
    }
    g_wmap_effect_model_d_rotation.vz += 0x90;
    timer = g_wmap_land_effect_31_sequence_12_timer - 1;
    g_wmap_land_effect_31_sequence_12_timer = timer;
    if (timer == 0)
    {
        g_wmap_land_effect_31_sequence_12_step++;
    }
}

void wmap_land_effect_31_sequence_12_step_04(void)
{
    s32 value;
    s32 timer;

    wmap_set_map_rotation(&g_wmap_effect_model_d_rotation);
    wmap_draw_model(g_wmap_effect_model_pack_3, (D_80139260 / 0x10) & 3, 0xA, 0x36, 0x7900, 0x1001, g_wmap_effect_fade_d, 4, -0x14, -1);
    value = g_wmap_effect_fade_d - 8;
    g_wmap_effect_fade_d = value;
    if (value < 0)
    {
        g_wmap_effect_fade_d = 0;
    }
    D_80139260 -= 0x20;
    timer = g_wmap_land_effect_31_sequence_12_timer - 1;
    g_wmap_land_effect_31_sequence_12_timer = timer;
    g_wmap_effect_model_d_rotation.vz += 0x90;
    if (timer == 0)
    {
        g_wmap_land_effect_31_sequence_12_step++;
    }
}

void wmap_land_effect_31_sequence_13_step_02(void)
{
    s32 value;
    s32 timer;

    wmap_set_map_rotation(&D_801B2670);
    wmap_draw_model(D_8011CF38, (D_80139264 / 0x10) & 3, 0xA, 0x36, 0x7900, 0x1001, D_801B25D8, 6, -0x18, -1);
    D_80139264 += 0x10;
    value = D_801B25D8 + 2;
    D_801B25D8 = value;
    if (value >= 0x82)
    {
        D_801B25D8 = 0x81;
    }
    D_801B2670.vz += 0x60;
    timer = g_wmap_land_effect_31_sequence_13_timer - 1;
    g_wmap_land_effect_31_sequence_13_timer = timer;
    if (timer == 0)
    {
        g_wmap_land_effect_31_sequence_13_step++;
    }
}

void wmap_land_effect_31_sequence_13_step_04(void)
{
    s32 value;
    s32 timer;

    wmap_set_map_rotation(&D_801B2670);
    wmap_draw_model(D_8011CF38, (D_80139264 / 0x10) & 3, 0xA, 0x36, 0x7900, 0x1001, D_801B25D8, 6, -0x18, -1);
    value = D_801B25D8 - 0x10;
    D_801B25D8 = value;
    if (value < 0)
    {
        D_801B25D8 = 0;
    }
    D_80139264 += 0x10;
    timer = g_wmap_land_effect_31_sequence_13_timer - 1;
    g_wmap_land_effect_31_sequence_13_timer = timer;
    D_801B2670.vz += 0x60;
    if (timer == 0)
    {
        g_wmap_land_effect_31_sequence_13_step++;
    }
}

void wmap_land_effect_31_sequence_14_step_02(void)
{
    s32 value;
    s32 timer;

    func_8006AEE0();
    wmap_draw_model(D_8011CF40, (D_80139268 / 0x10) & 3, 0xA, 0x36, 0x7880, 1, D_801B25DC, 0, 0, -1);
    D_80139268 += 8;
    value = D_801B25DC + 2;
    D_801B25DC = value;
    if (value >= 0x82)
    {
        D_801B25DC = 0x81;
    }
    D_801B2678.vz += 0;
    timer = g_wmap_land_effect_31_sequence_14_timer - 1;
    g_wmap_land_effect_31_sequence_14_timer = timer;
    if (timer == 0)
    {
        g_wmap_land_effect_31_sequence_14_step++;
    }
}

void wmap_land_effect_31_sequence_14_step_04(void)
{
    s32 value;
    s32 timer;

    func_8006AEE0();
    wmap_draw_model(D_8011CF40, (D_80139268 / 0x10) & 3, 0xA, 0x36, 0x7880, 1, D_801B25DC, 0, 0, -1);
    value = D_801B25DC - 4;
    D_801B25DC = value;
    if (value < 0)
    {
        D_801B25DC = 0;
    }
    D_80139268 += 8;
    D_801B2678.vz += 0;
    timer = g_wmap_land_effect_31_sequence_14_timer - 1;
    g_wmap_land_effect_31_sequence_14_timer = timer;
    if (timer == 0)
    {
        g_wmap_land_effect_31_sequence_14_step++;
    }
}

void wmap_land_effect_31_sequence_20_step_01(void)
{
    s32 index;
    s32 config_offset;
    s32 screen_offset;
    u8* config_base;
    u8* screen_base;
    u8* resource;
    u8* screen_entry;
    s16* config_entry;

    index = 0;
    config_base = g_wmap_actor_motions;
    screen_base = (u8*)g_wmap_actor_animations;
    resource = &g_wmap_animation_bank_2;
    screen_offset = 0x640;
    config_offset = 0xFA0;
    g_wmap_effect_params[40] = 0x28;
    g_wmap_effect_params[41] = 0xC8;
    g_wmap_effect_params[42] = -0x136;
    g_wmap_effect_params[43] = 0x28;
    g_wmap_effect_params[44] = -0x64;
    g_wmap_effect_params[45] = 0x28;
    g_wmap_effect_params[46] = 0;
    g_wmap_effect_params[47] = 0x2D;
    g_wmap_effect_params[48] = 0x81;
    g_wmap_effect_params[49] = 1;
    g_wmap_effect_params[50] = 0x20;
    g_wmap_effect_params[51] = 4;
    g_wmap_effect_params[52] = 0xF;
    g_wmap_effect_params[53] = 0;

    do
    {
        screen_entry = (u8*)(screen_offset + (s32)screen_base);
        screen_offset += 8;
        config_entry = (s16*)(config_offset + (s32)config_base);
        config_offset += 0x14;
        index++;
        *config_entry = 0;
        *(u8**)(screen_entry + 4) = resource;
    } while (index < 0x28);

    g_wmap_land_effect_31_sequence_20_timer = 0x64;
    g_wmap_land_effect_31_sequence_20_step++;
    wmap_land_effect_31_sequence_20_step_02();
}

WMAP_STEP_RUNNER(wmap_land_effect_31_run, D_800D7474, 6, g_wmap_land_effect_31_step, g_wmap_land_effect_31_timer)

WMAP_STEP_RESET(wmap_land_effect_31_reset, g_wmap_land_effect_31_step, g_wmap_land_effect_31_timer)

WMAP_STEP_START_BLOCKING(wmap_land_effect_31_step_01, g_wmap_land_effect_31_step, wmap_run_land_focus, wmap_land_effect_31_step_02)

WMAP_STEP_WAIT_IDLE(wmap_land_effect_31_step_02, g_wmap_land_effect_31_step, wmap_land_effect_31_step_03)

WMAP_STEP_START_BLOCKING(wmap_land_effect_31_step_03, g_wmap_land_effect_31_step, wmap_land_effect_31_run_timeline, wmap_land_effect_31_step_04)

WMAP_STEP_WAIT_IDLE(wmap_land_effect_31_step_04, g_wmap_land_effect_31_step, wmap_land_effect_31_step_05)

WMAP_STEP_ADVANCE(wmap_land_effect_31_step_05, g_wmap_land_effect_31_step)

WMAP_STEP_RUNNER(wmap_land_effect_31_run_timeline, D_800D748C, 0x2C, g_wmap_land_effect_31_timeline_step, g_wmap_land_effect_31_timeline_timer)

WMAP_STEP_RESET(wmap_land_effect_31_timeline_reset, g_wmap_land_effect_31_timeline_step, g_wmap_land_effect_31_timeline_timer)

void wmap_land_effect_31_timeline_step_01(void)
{
    wmap_play_sound(0x34, 0x80);
    g_wmap_event_active = 1;
    wmap_start_sequence(wmap_land_effect_31_run_sequence_18);
    g_wmap_land_effect_31_timeline_timer = 0xF;
    g_wmap_land_effect_31_timeline_step++;
}

WMAP_STEP_WAIT(wmap_land_effect_31_timeline_step_02, g_wmap_land_effect_31_timeline_step, g_wmap_land_effect_31_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_31_timeline_step_03, g_wmap_land_effect_31_timeline_step, g_wmap_land_effect_31_timeline_timer,
                         wmap_land_effect_31_run_sequence_20, 0xF)

WMAP_STEP_WAIT(wmap_land_effect_31_timeline_step_04, g_wmap_land_effect_31_timeline_step, g_wmap_land_effect_31_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_31_timeline_step_05, g_wmap_land_effect_31_timeline_step, g_wmap_land_effect_31_timeline_timer,
                         wmap_land_effect_31_run_sequence_2, 0x16)

WMAP_STEP_WAIT(wmap_land_effect_31_timeline_step_06, g_wmap_land_effect_31_timeline_step, g_wmap_land_effect_31_timeline_timer)

void wmap_land_effect_31_timeline_step_07(void)
{
    wmap_start_sequence(wmap_land_effect_31_run_sequence_13);
    g_wmap_backdrop_target_level = 0xC;
    wmap_start_map_tint(0x704060);
    g_wmap_land_effect_31_timeline_timer = 0xB;
    g_wmap_land_effect_31_timeline_step++;
}

WMAP_STEP_WAIT(wmap_land_effect_31_timeline_step_08, g_wmap_land_effect_31_timeline_step, g_wmap_land_effect_31_timeline_timer)

void wmap_land_effect_31_timeline_step_09(void)
{
    g_wmap_placement_overlay_hidden = 1;
    g_wmap_land_effect_31_timeline_timer = 0x28;
    g_wmap_land_effect_31_timeline_step++;
}

WMAP_STEP_WAIT(wmap_land_effect_31_timeline_step_10, g_wmap_land_effect_31_timeline_step, g_wmap_land_effect_31_timeline_timer)

void wmap_land_effect_31_timeline_step_11(void)
{
    wmap_start_sequence(wmap_land_effect_31_run_sequence_17);
    wmap_start_map_tint(0x352030);
    wmap_start_sequence(wmap_land_effect_31_run_sequence_10);
    g_wmap_land_effect_31_timeline_timer = 0xA;
    g_wmap_land_effect_31_timeline_step++;
}

WMAP_STEP_WAIT(wmap_land_effect_31_timeline_step_12, g_wmap_land_effect_31_timeline_step, g_wmap_land_effect_31_timeline_timer)

void wmap_land_effect_31_timeline_step_13(void)
{
    g_wmap_screen_fade_mode = 2;
    D_8013B29C = 1;
    g_wmap_land_effect_31_timeline_timer = 4;
    g_wmap_land_effect_31_timeline_step++;
}

WMAP_STEP_WAIT(wmap_land_effect_31_timeline_step_14, g_wmap_land_effect_31_timeline_step, g_wmap_land_effect_31_timeline_timer)

void wmap_land_effect_31_timeline_step_15(void)
{
    wmap_start_map_tint(0);
    g_wmap_backdrop_target_level = 1;
    g_wmap_land_effect_31_timeline_timer = 0xF;
    g_wmap_land_effect_31_timeline_step++;
}

WMAP_STEP_WAIT(wmap_land_effect_31_timeline_step_16, g_wmap_land_effect_31_timeline_step, g_wmap_land_effect_31_timeline_timer)

WMAP_STEP_WAIT(wmap_land_effect_31_timeline_step_18, g_wmap_land_effect_31_timeline_step, g_wmap_land_effect_31_timeline_timer)

WMAP_STEP_WAIT(wmap_land_effect_31_timeline_step_20, g_wmap_land_effect_31_timeline_step, g_wmap_land_effect_31_timeline_timer)

void wmap_land_effect_31_timeline_step_21(void)
{
    wmap_install_callback(func_8006C0EC);
    D_80139210.field_00 = 0x132;
    D_80139210.field_02 = 0;
    D_80139210.field_04 = 0x1C2;
    D_80139968.field_00 = 0x26C;
    D_80139968.field_04 = 0;
    D_80139968.field_08 = 0;
    D_800DCEB8.field_00 = 0x132;
    D_800DCEB8.field_02 = 0;
    D_800DCEB8.field_04 = -0x1C2;
    D_80139200.field_00 = 0x26C;
    D_80139200.field_04 = 0;
    D_80139200.field_08 = 0;
    g_wmap_land_effect_31_timeline_timer = 0xA;
    g_wmap_land_effect_31_timeline_step++;
}

WMAP_STEP_WAIT(wmap_land_effect_31_timeline_step_22, g_wmap_land_effect_31_timeline_step, g_wmap_land_effect_31_timeline_timer)

WMAP_STEP_WAIT(wmap_land_effect_31_timeline_step_24, g_wmap_land_effect_31_timeline_step, g_wmap_land_effect_31_timeline_timer)

void wmap_land_effect_31_timeline_step_25(void)
{
    D_80139210.field_00 = 0;
    D_80139210.field_02 = 0;
    D_80139210.field_04 = 0;
    D_80139968.field_00 = 5;
    D_80139968.field_04 = 0;
    D_80139968.field_08 = 0;
    D_800DCEB8.field_00 = 0;
    D_800DCEB8.field_02 = 0;
    D_800DCEB8.field_04 = 0;
    D_80139200.field_00 = 0;
    D_80139200.field_04 = 0;
    D_80139200.field_08 = 0;
    g_wmap_land_effect_31_timeline_timer = 0x1E;
    g_wmap_land_effect_31_timeline_step++;
}

WMAP_STEP_WAIT(wmap_land_effect_31_timeline_step_26, g_wmap_land_effect_31_timeline_step, g_wmap_land_effect_31_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_31_timeline_step_27, g_wmap_land_effect_31_timeline_step, g_wmap_land_effect_31_timeline_timer,
                         wmap_land_effect_31_run_sequence_3, 0xA0)

WMAP_STEP_WAIT(wmap_land_effect_31_timeline_step_28, g_wmap_land_effect_31_timeline_step, g_wmap_land_effect_31_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_31_timeline_step_29, g_wmap_land_effect_31_timeline_step, g_wmap_land_effect_31_timeline_timer,
                         wmap_land_effect_31_run_sequence_8, 7)

WMAP_STEP_WAIT(wmap_land_effect_31_timeline_step_30, g_wmap_land_effect_31_timeline_step, g_wmap_land_effect_31_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_31_timeline_step_31, g_wmap_land_effect_31_timeline_step, g_wmap_land_effect_31_timeline_timer,
                         wmap_land_effect_31_run_sequence_11, 0x14)

WMAP_STEP_WAIT(wmap_land_effect_31_timeline_step_32, g_wmap_land_effect_31_timeline_step, g_wmap_land_effect_31_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_31_timeline_step_33, g_wmap_land_effect_31_timeline_step, g_wmap_land_effect_31_timeline_timer,
                         wmap_land_effect_31_run_sequence_1, 0x28)

WMAP_STEP_WAIT(wmap_land_effect_31_timeline_step_34, g_wmap_land_effect_31_timeline_step, g_wmap_land_effect_31_timeline_timer)

void wmap_land_effect_31_timeline_step_35(void)
{
    wmap_start_sequence(wmap_land_effect_31_run_sequence_12);
    g_wmap_backdrop_target_level = 0;
    D_80182D74.field_00 = 0;
    D_80182D74.field_01 = 0;
    D_80182D74.field_02 = 0;
    D_80182D80.field_00 = 0;
    D_80182D80.field_01 = 0;
    D_80182D80.field_02 = 0;
    D_80182D8C.field_00 = 0;
    D_80182D8C.field_01 = 0;
    D_80182D8C.field_02 = 0;
    D_80182D94.field_00 = 0;
    D_80182D94.field_01 = 0;
    D_80182D94.field_02 = 0;
    g_wmap_land_effect_31_timeline_timer = 0x1E;
    g_wmap_land_effect_31_timeline_step++;
}

WMAP_STEP_WAIT(wmap_land_effect_31_timeline_step_36, g_wmap_land_effect_31_timeline_step, g_wmap_land_effect_31_timeline_timer)

WMAP_STEP_WAIT(wmap_land_effect_31_timeline_step_38, g_wmap_land_effect_31_timeline_step, g_wmap_land_effect_31_timeline_timer)

void wmap_land_effect_31_timeline_step_39(void)
{
    wmap_start_sequence(wmap_land_effect_31_run_sequence_9);
    wmap_start_map_tint(0x808080);
    g_wmap_backdrop_target_level = 0x10;
    g_wmap_transition_mesh_hidden = 0;
    g_wmap_land_effect_31_timeline_timer = 0x1C;
    g_wmap_cells[g_wmap_focus_cell_x][g_wmap_focus_cell_y].land_id = g_wmap_selected_artifact | 0x100;
    g_wmap_land_effect_31_timeline_step++;
}

WMAP_STEP_WAIT(wmap_land_effect_31_timeline_step_40, g_wmap_land_effect_31_timeline_step, g_wmap_land_effect_31_timeline_timer)

WMAP_STEP_WAIT(wmap_land_effect_31_timeline_step_42, g_wmap_land_effect_31_timeline_step, g_wmap_land_effect_31_timeline_timer)

void wmap_land_effect_31_timeline_finish(void)
{
    g_wmap_sequence_busy = 0;
    g_wmap_land_effect_31_timeline_step++;
}

WMAP_STEP_RUNNER(wmap_land_effect_31_run_sequence_1, D_800D753C, 6, g_wmap_land_effect_31_sequence_1_step, g_wmap_land_effect_31_sequence_1_timer)

WMAP_STEP_RESET(wmap_land_effect_31_sequence_1_reset, g_wmap_land_effect_31_sequence_1_step, g_wmap_land_effect_31_sequence_1_timer)

void wmap_land_effect_31_sequence_1_step_02(void)
{
    s32 timer;

    func_8006AEE0();
    func_8006A2FC(&g_wmap_sprite_actors[64], &g_wmap_actor_animations[64], 0x28, 0, 0x7F, 4, 0, (u8*)g_wmap_effect_params + 0x78);
    timer = g_wmap_land_effect_31_sequence_1_timer - 1;
    g_wmap_land_effect_31_sequence_1_timer = timer;
    if (timer == 0)
    {
        g_wmap_land_effect_31_sequence_1_step++;
    }
}

void wmap_land_effect_31_sequence_1_step_03(void)
{
    g_wmap_land_effect_31_sequence_1_timer = 0x20;
    g_wmap_effect_params[35] = -1;
    g_wmap_land_effect_31_sequence_1_step++;
    wmap_land_effect_31_sequence_1_step_04();
}

void wmap_land_effect_31_sequence_1_step_04(void)
{
    s32 timer;

    func_8006AEE0();
    func_8006A2FC(&g_wmap_sprite_actors[64], &g_wmap_actor_animations[64], 0x28, 0, 0x7F, 4, 0, (u8*)g_wmap_effect_params + 0x78);
    timer = g_wmap_land_effect_31_sequence_1_timer - 1;
    g_wmap_land_effect_31_sequence_1_timer = timer;
    if (timer == 0)
    {
        g_wmap_land_effect_31_sequence_1_step++;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_31_sequence_1_end, g_wmap_land_effect_31_sequence_1_step)

WMAP_STEP_RUNNER(wmap_land_effect_31_run_sequence_2, D_800D7554, 4, g_wmap_land_effect_31_sequence_2_step, g_wmap_land_effect_31_sequence_2_timer)

WMAP_STEP_RESET(wmap_land_effect_31_sequence_2_reset, g_wmap_land_effect_31_sequence_2_step, g_wmap_land_effect_31_sequence_2_timer)

void wmap_land_effect_31_sequence_2_step_01(void)
{
    WmapSpriteActor* actor = &g_wmap_sprite_actors[5];

    g_wmap_actor_animations[5].data = g_wmap_animation_bank_1;
    actor->scale_index = 0xF;
    actor->sequence = 3;
    actor->previous_sequence = -1;
    actor->shade_step = 8;
    actor->target_shade = 0x81;
    actor->resource_index = 0;
    actor->shade = 1;
    g_wmap_land_effect_31_sequence_2_timer = 0x64;
    g_wmap_land_effect_31_sequence_2_step++;
    wmap_land_effect_31_sequence_2_step_02();
}

void wmap_land_effect_31_sequence_2_step_02(void)
{
    s32 timer;

    wmap_step_actor_animation(&g_wmap_sprite_actors[5], &g_wmap_actor_animations[5]);
    wmap_draw_actor_sprite(&g_wmap_sprite_actors[5], g_wmap_focus_screen_position.packed, 0xF, 2, 0);
    timer = g_wmap_land_effect_31_sequence_2_timer - 1;
    g_wmap_land_effect_31_sequence_2_timer = timer;
    if (timer == 0)
    {
        g_wmap_land_effect_31_sequence_2_step++;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_31_sequence_2_end, g_wmap_land_effect_31_sequence_2_step)

WMAP_STEP_RUNNER(wmap_land_effect_31_run_sequence_3, D_800D7564, 6, g_wmap_land_effect_31_sequence_3_step, g_wmap_land_effect_31_sequence_3_timer)

WMAP_STEP_RESET(wmap_land_effect_31_sequence_3_reset, g_wmap_land_effect_31_sequence_3_step, g_wmap_land_effect_31_sequence_3_timer)

void wmap_land_effect_31_sequence_3_step_01(void)
{
    WmapSpriteActor* actor = &g_wmap_sprite_actors[6];

    g_wmap_actor_animations[6].data = &g_wmap_animation_bank_0;
    actor->previous_sequence = -1;
    actor->shade_step = 2;
    actor->target_shade = 0x81;
    actor->resource_index = 0;
    actor->scale_index = 0;
    actor->sequence = 0;
    actor->shade = 1;
    g_wmap_land_effect_31_sequence_3_timer = 0x10E;
    g_wmap_land_effect_31_sequence_3_step++;
    wmap_land_effect_31_sequence_3_step_02();
}

void wmap_land_effect_31_sequence_3_step_02(void)
{
    s32 timer;

    wmap_step_actor_animation(&g_wmap_sprite_actors[6], &g_wmap_actor_animations[6]);
    wmap_draw_actor_sprite(&g_wmap_sprite_actors[6], g_wmap_focus_screen_position.packed, 0x10, 8, 1);
    timer = g_wmap_land_effect_31_sequence_3_timer - 1;
    g_wmap_land_effect_31_sequence_3_timer = timer;
    if (timer == 0)
    {
        g_wmap_land_effect_31_sequence_3_step++;
    }
}

void wmap_land_effect_31_sequence_3_step_03(void)
{
    WmapSpriteActor* actor = &g_wmap_sprite_actors[6];

    actor->shade_step = 2;
    actor->target_shade = 0;
    D_80139268 = 0;
    g_wmap_land_effect_31_sequence_3_timer = 0x40;
    g_wmap_land_effect_31_sequence_3_step++;
    wmap_land_effect_31_sequence_3_step_04();
}

void wmap_land_effect_31_sequence_3_step_04(void)
{
    s32 timer;

    wmap_step_actor_animation(&g_wmap_sprite_actors[6], &g_wmap_actor_animations[6]);
    wmap_draw_actor_sprite(&g_wmap_sprite_actors[6], g_wmap_focus_screen_position.packed, 0x10, 8, 1);
    g_wmap_sprite_actors[6].scale_index = D_80139268 >> 4;
    D_80139268 += 0x10;
    if (D_80139268 >= 0xF1)
    {
        D_80139268 = 0xF0;
    }
    timer = g_wmap_land_effect_31_sequence_3_timer - 1;
    g_wmap_land_effect_31_sequence_3_timer = timer;
    if (timer == 0)
    {
        g_wmap_land_effect_31_sequence_3_step++;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_31_sequence_3_end, g_wmap_land_effect_31_sequence_3_step)

WMAP_STEP_RUNNER(wmap_land_effect_31_run_sequence_4, D_800D757C, 6, g_wmap_land_effect_31_sequence_4_step, g_wmap_land_effect_31_sequence_4_timer)

WMAP_STEP_RESET(wmap_land_effect_31_sequence_4_reset, g_wmap_land_effect_31_sequence_4_step, g_wmap_land_effect_31_sequence_4_timer)

void wmap_land_effect_31_sequence_4_step_01(void)
{
    D_801B24B4 = 1;
    D_801B2490 = g_wmap_zero_rotation;
    D_80139234 = 0;
    g_wmap_land_effect_31_sequence_4_timer = 0x92;
    g_wmap_land_effect_31_sequence_4_step++;
    wmap_land_effect_31_sequence_4_step_02();
}

WMAP_STEP_ARM_TIMER(wmap_land_effect_31_sequence_4_step_03, g_wmap_land_effect_31_sequence_4_step, g_wmap_land_effect_31_sequence_4_timer, 0x40,
                    wmap_land_effect_31_sequence_4_step_04)

WMAP_STEP_ADVANCE(wmap_land_effect_31_sequence_4_end, g_wmap_land_effect_31_sequence_4_step)

WMAP_STEP_RUNNER(wmap_land_effect_31_run_sequence_5, D_800D7594, 6, g_wmap_land_effect_31_sequence_5_step, g_wmap_land_effect_31_sequence_5_timer)

WMAP_STEP_RESET(wmap_land_effect_31_sequence_5_reset, g_wmap_land_effect_31_sequence_5_step, g_wmap_land_effect_31_sequence_5_timer)

void wmap_land_effect_31_sequence_5_step_02(void)
{
    s32 timer;

    func_8006ADD0(&g_wmap_camera_translation, &D_801B3120);
    func_8006C448((WmapState*)g_wmap_effect_params);
    D_801B3120.vz += 0x18;
    timer = g_wmap_land_effect_31_sequence_5_timer - 1;
    g_wmap_land_effect_31_sequence_5_timer = timer;
    if (timer == 0)
    {
        g_wmap_land_effect_31_sequence_5_step++;
    }
}

void wmap_land_effect_31_sequence_5_step_03(void)
{
    s32 i;
    WmapSpriteActor* configs;

    configs = &g_wmap_sprite_actors[20];
    for (i = 0; i < 0x28; i++)
    {
        configs[i].target_shade = 0;
        configs[i].shade_step = 8;
    }
    g_wmap_land_effect_31_sequence_5_timer = 0x10;
    g_wmap_land_effect_31_sequence_5_step++;
    wmap_land_effect_31_sequence_5_step_04();
}

void wmap_land_effect_31_sequence_5_step_04(void)
{
    s32 timer;

    func_8006ADD0(&g_wmap_camera_translation, &D_801B3120);
    func_8006C448((WmapState*)g_wmap_effect_params);
    D_801B3120.vz += 0x18;
    timer = g_wmap_land_effect_31_sequence_5_timer - 1;
    g_wmap_land_effect_31_sequence_5_timer = timer;
    if (timer == 0)
    {
        g_wmap_land_effect_31_sequence_5_step++;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_31_sequence_5_end, g_wmap_land_effect_31_sequence_5_step)

WMAP_STEP_RUNNER(wmap_land_effect_31_run_sequence_6, D_800D75AC, 6, g_wmap_land_effect_31_sequence_6_step, g_wmap_land_effect_31_sequence_6_timer)

WMAP_STEP_RESET(wmap_land_effect_31_sequence_6_reset, g_wmap_land_effect_31_sequence_6_step, g_wmap_land_effect_31_sequence_6_timer)

void wmap_land_effect_31_sequence_6_step_02(void)
{
    s32 timer;

    func_8006AEE0();
    func_8006A2FC(&g_wmap_sprite_actors[64], &g_wmap_actor_animations[64], 0x20, 0, 0x7F, 2, 0, (WmapState*)g_wmap_effect_params + 1);
    timer = g_wmap_land_effect_31_sequence_6_timer - 1;
    g_wmap_land_effect_31_sequence_6_timer = timer;
    if (timer == 0)
    {
        g_wmap_land_effect_31_sequence_6_step++;
    }
}

void wmap_land_effect_31_sequence_6_step_03(void)
{
    g_wmap_land_effect_31_sequence_6_timer = 0x20;
    g_wmap_effect_params[25] = -1;
    g_wmap_land_effect_31_sequence_6_step++;
    wmap_land_effect_31_sequence_6_step_04();
}

void wmap_land_effect_31_sequence_6_step_04(void)
{
    s32 timer;

    func_8006AEE0();
    func_8006A2FC(&g_wmap_sprite_actors[64], &g_wmap_actor_animations[64], 0x20, 0, 0x7F, 2, 0, (WmapState*)g_wmap_effect_params + 1);
    timer = g_wmap_land_effect_31_sequence_6_timer - 1;
    g_wmap_land_effect_31_sequence_6_timer = timer;
    if (timer == 0)
    {
        g_wmap_land_effect_31_sequence_6_step++;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_31_sequence_6_end, g_wmap_land_effect_31_sequence_6_step)

WMAP_STEP_RUNNER(wmap_land_effect_31_run_sequence_7, D_800D75C4, 4, g_wmap_land_effect_31_sequence_7_step, g_wmap_land_effect_31_sequence_7_timer)

WMAP_STEP_RESET(wmap_land_effect_31_sequence_7_reset, g_wmap_land_effect_31_sequence_7_step, g_wmap_land_effect_31_sequence_7_timer)

WMAP_STEP_DROP_START(wmap_land_effect_31_sequence_7_step_01, g_wmap_land_effect_31_sequence_7_step, g_wmap_land_effect_31_sequence_7_timer, g_wmap_effect_model_a_rotation,
                     g_wmap_zero_rotation, g_wmap_effect_model_a_position, g_wmap_effect_fade_a, 0x80, 0xAFC8, 0x80, wmap_land_effect_31_sequence_7_step_02)

WMAP_STEP_ADVANCE(wmap_land_effect_31_sequence_7_end, g_wmap_land_effect_31_sequence_7_step)

WMAP_STEP_RUNNER(wmap_land_effect_31_run_sequence_8, D_800D75D4, 4, g_wmap_land_effect_31_sequence_8_step, g_wmap_land_effect_31_sequence_8_timer)

WMAP_STEP_RESET(wmap_land_effect_31_sequence_8_reset, g_wmap_land_effect_31_sequence_8_step, g_wmap_land_effect_31_sequence_8_timer)

WMAP_STEP_DROP_START(wmap_land_effect_31_sequence_8_step_01, g_wmap_land_effect_31_sequence_8_step, g_wmap_land_effect_31_sequence_8_timer, g_wmap_effect_model_b_rotation,
                     g_wmap_zero_rotation, g_wmap_effect_model_b_position, g_wmap_effect_fade_b, 0x80, 0xAFC8, 0x20, wmap_land_effect_31_sequence_8_step_02)

WMAP_STEP_ADVANCE(wmap_land_effect_31_sequence_8_end, g_wmap_land_effect_31_sequence_8_step)

WMAP_STEP_RUNNER(wmap_land_effect_31_run_sequence_9, D_800D75E4, 4, g_wmap_land_effect_31_sequence_9_step, g_wmap_land_effect_31_sequence_9_timer)

WMAP_STEP_RESET(wmap_land_effect_31_sequence_9_reset, g_wmap_land_effect_31_sequence_9_step, g_wmap_land_effect_31_sequence_9_timer)

WMAP_STEP_DROP_START(wmap_land_effect_31_sequence_9_step_01, g_wmap_land_effect_31_sequence_9_step, g_wmap_land_effect_31_sequence_9_timer, g_wmap_effect_model_c_rotation,
                     g_wmap_zero_rotation, g_wmap_effect_model_c_position, g_wmap_effect_fade_c, 0x80, 0xAFC8, 0x40, wmap_land_effect_31_sequence_9_step_02)

WMAP_STEP_ADVANCE(wmap_land_effect_31_sequence_9_end, g_wmap_land_effect_31_sequence_9_step)

WMAP_STEP_RUNNER(wmap_land_effect_31_run_sequence_10, D_800D75F4, 6, g_wmap_land_effect_31_sequence_10_step, g_wmap_land_effect_31_sequence_10_timer)

WMAP_STEP_RESET(wmap_land_effect_31_sequence_10_reset, g_wmap_land_effect_31_sequence_10_step, g_wmap_land_effect_31_sequence_10_timer)

void wmap_land_effect_31_sequence_10_step_01(void)
{
    D_80182DE4 = 1;
    D_801B2498 = g_wmap_zero_rotation;
    D_8013923C = 0;
    g_wmap_land_effect_31_sequence_10_timer = 0x5A;
    g_wmap_land_effect_31_sequence_10_step++;
    wmap_land_effect_31_sequence_10_step_02();
}

WMAP_STEP_ARM_TIMER(wmap_land_effect_31_sequence_10_step_03, g_wmap_land_effect_31_sequence_10_step, g_wmap_land_effect_31_sequence_10_timer, 8,
                    wmap_land_effect_31_sequence_10_step_04)

WMAP_STEP_ADVANCE(wmap_land_effect_31_sequence_10_end, g_wmap_land_effect_31_sequence_10_step)

WMAP_STEP_RUNNER(wmap_land_effect_31_run_sequence_11, D_800D760C, 6, g_wmap_land_effect_31_sequence_11_step, g_wmap_land_effect_31_sequence_11_timer)

WMAP_STEP_RESET(wmap_land_effect_31_sequence_11_reset, g_wmap_land_effect_31_sequence_11_step, g_wmap_land_effect_31_sequence_11_timer)

void wmap_land_effect_31_sequence_11_step_01(void)
{
    D_801B25E0 = 1;
    D_801B3118 = *(WmapPair*)&g_wmap_zero_rotation;
    D_8013926C = 0;
    g_wmap_land_effect_31_sequence_11_timer = 0xB6;
    g_wmap_land_effect_31_sequence_11_step++;
    wmap_land_effect_31_sequence_11_step_02();
}

WMAP_STEP_ARM_TIMER(wmap_land_effect_31_sequence_11_step_03, g_wmap_land_effect_31_sequence_11_step, g_wmap_land_effect_31_sequence_11_timer, 1,
                    wmap_land_effect_31_sequence_11_step_04)

WMAP_STEP_ADVANCE(wmap_land_effect_31_sequence_11_end, g_wmap_land_effect_31_sequence_11_step)

WMAP_STEP_RUNNER(wmap_land_effect_31_run_sequence_12, D_800D7624, 6, g_wmap_land_effect_31_sequence_12_step, g_wmap_land_effect_31_sequence_12_timer)

WMAP_STEP_RESET(wmap_land_effect_31_sequence_12_reset, g_wmap_land_effect_31_sequence_12_step, g_wmap_land_effect_31_sequence_12_timer)

void wmap_land_effect_31_sequence_12_step_01(void)
{
    g_wmap_effect_fade_d = 0x81;
    g_wmap_effect_model_d_rotation = g_wmap_zero_rotation;
    D_80139260 = 0;
    g_wmap_land_effect_31_sequence_12_timer = 0x68;
    g_wmap_land_effect_31_sequence_12_step++;
    wmap_land_effect_31_sequence_12_step_02();
}

WMAP_STEP_ARM_TIMER(wmap_land_effect_31_sequence_12_step_03, g_wmap_land_effect_31_sequence_12_step, g_wmap_land_effect_31_sequence_12_timer, 0x10,
                    wmap_land_effect_31_sequence_12_step_04)

WMAP_STEP_ADVANCE(wmap_land_effect_31_sequence_12_end, g_wmap_land_effect_31_sequence_12_step)

WMAP_STEP_RUNNER(wmap_land_effect_31_run_sequence_13, D_800D763C, 6, g_wmap_land_effect_31_sequence_13_step, g_wmap_land_effect_31_sequence_13_timer)

WMAP_STEP_RESET(wmap_land_effect_31_sequence_13_reset, g_wmap_land_effect_31_sequence_13_step, g_wmap_land_effect_31_sequence_13_timer)

void wmap_land_effect_31_sequence_13_step_01(void)
{
    D_801B25D8 = 1;
    D_801B2670 = g_wmap_zero_rotation;
    D_80139264 = 0;
    g_wmap_land_effect_31_sequence_13_timer = 0x87;
    g_wmap_land_effect_31_sequence_13_step++;
    wmap_land_effect_31_sequence_13_step_02();
}

WMAP_STEP_ARM_TIMER(wmap_land_effect_31_sequence_13_step_03, g_wmap_land_effect_31_sequence_13_step, g_wmap_land_effect_31_sequence_13_timer, 8,
                    wmap_land_effect_31_sequence_13_step_04)

WMAP_STEP_ADVANCE(wmap_land_effect_31_sequence_13_end, g_wmap_land_effect_31_sequence_13_step)

WMAP_STEP_RUNNER(wmap_land_effect_31_run_sequence_14, D_800D7654, 6, g_wmap_land_effect_31_sequence_14_step, g_wmap_land_effect_31_sequence_14_timer)

WMAP_STEP_RESET(wmap_land_effect_31_sequence_14_reset, g_wmap_land_effect_31_sequence_14_step, g_wmap_land_effect_31_sequence_14_timer)

void wmap_land_effect_31_sequence_14_step_01(void)
{
    D_801B25DC = 1;
    D_801B2678 = g_wmap_zero_rotation;
    D_80139268 = 0;
    g_wmap_land_effect_31_sequence_14_timer = 0xF0;
    g_wmap_land_effect_31_sequence_14_step++;
    wmap_land_effect_31_sequence_14_step_02();
}

WMAP_STEP_ARM_TIMER(wmap_land_effect_31_sequence_14_step_03, g_wmap_land_effect_31_sequence_14_step, g_wmap_land_effect_31_sequence_14_timer, 0x20,
                    wmap_land_effect_31_sequence_14_step_04)

WMAP_STEP_ADVANCE(wmap_land_effect_31_sequence_14_end, g_wmap_land_effect_31_sequence_14_step)

WMAP_STEP_RUNNER(wmap_land_effect_31_run_sequence_15, D_800D766C, 6, g_wmap_land_effect_31_sequence_15_step, g_wmap_land_effect_31_sequence_15_timer)

WMAP_STEP_RESET(wmap_land_effect_31_sequence_15_reset, g_wmap_land_effect_31_sequence_15_step, g_wmap_land_effect_31_sequence_15_timer)

void wmap_land_effect_31_sequence_15_step_01(void)
{
    D_80139240 = 0;
    g_wmap_land_effect_31_sequence_15_timer = 0xF8;
    g_wmap_land_effect_31_sequence_15_step++;
    wmap_land_effect_31_sequence_15_step_02();
}

void wmap_land_effect_31_sequence_15_step_02(void)
{
    s32 timer;

    func_8006AEE0();
    wmap_draw_model(D_8011CF30, 0, 0xB0, 0x35, 0x7800, 1, D_80139240, -0x32, 0, -1);
    D_80139240 += 8;
    if (D_80139240 >= 0x81)
    {
        D_80139240 = 0x80;
    }

    timer = g_wmap_land_effect_31_sequence_15_timer - 1;
    g_wmap_land_effect_31_sequence_15_timer = timer;
    if (timer == 0)
    {
        g_wmap_land_effect_31_sequence_15_step++;
    }
}

WMAP_STEP_ARM_TIMER(wmap_land_effect_31_sequence_15_step_03, g_wmap_land_effect_31_sequence_15_step, g_wmap_land_effect_31_sequence_15_timer, 0x10,
                    wmap_land_effect_31_sequence_15_step_04)

void wmap_land_effect_31_sequence_15_step_04(void)
{
    s32 timer;

    func_8006AEE0();
    wmap_draw_model(D_8011CF30, 0, 0xB0, 0x35, 0x7800, 1, D_80139240, -0x32, 0, -1);
    D_80139240 -= 8;
    if (D_80139240 < 0)
    {
        D_80139240 = 0;
    }

    timer = g_wmap_land_effect_31_sequence_15_timer - 1;
    g_wmap_land_effect_31_sequence_15_timer = timer;
    if (timer == 0)
    {
        g_wmap_land_effect_31_sequence_15_step++;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_31_sequence_15_end, g_wmap_land_effect_31_sequence_15_step)

WMAP_STEP_RUNNER(wmap_land_effect_31_run_sequence_16, D_800D7684, 6, g_wmap_land_effect_31_sequence_16_step, g_wmap_land_effect_31_sequence_16_timer)

WMAP_STEP_RESET(wmap_land_effect_31_sequence_16_reset, g_wmap_land_effect_31_sequence_16_step, g_wmap_land_effect_31_sequence_16_timer)

void wmap_land_effect_31_sequence_16_step_01(void)
{
    D_8013924C = 1;
    g_wmap_land_effect_31_sequence_16_timer = 0x70;
    g_wmap_land_effect_31_sequence_16_step++;
    wmap_land_effect_31_sequence_16_step_02();
}

void wmap_land_effect_31_sequence_16_step_02(void)
{
    s32 timer;

    func_8006AEE0();
    wmap_draw_model(g_wmap_effect_model_pack_2, 0, 0xA, 0x35, 0x7840, 1, D_8013924C, 0, 0, -1);
    D_8013924C += 8;
    if (D_8013924C >= 0x82)
    {
        D_8013924C = 0x81;
    }

    timer = g_wmap_land_effect_31_sequence_16_timer - 1;
    g_wmap_land_effect_31_sequence_16_timer = timer;
    if (timer == 0)
    {
        g_wmap_land_effect_31_sequence_16_step++;
    }
}

WMAP_STEP_ARM_TIMER(wmap_land_effect_31_sequence_16_step_03, g_wmap_land_effect_31_sequence_16_step, g_wmap_land_effect_31_sequence_16_timer, 0x20,
                    wmap_land_effect_31_sequence_16_step_04)

void wmap_land_effect_31_sequence_16_step_04(void)
{
    s32 timer;

    func_8006AEE0();
    wmap_draw_model(g_wmap_effect_model_pack_2, 0, 0xA, 0x35, 0x7840, 1, D_8013924C, 0, 0, -1);
    D_8013924C -= 4;
    if (D_8013924C < 0)
    {
        D_8013924C = 0;
    }

    timer = g_wmap_land_effect_31_sequence_16_timer - 1;
    g_wmap_land_effect_31_sequence_16_timer = timer;
    if (timer == 0)
    {
        g_wmap_land_effect_31_sequence_16_step++;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_31_sequence_16_end, g_wmap_land_effect_31_sequence_16_step)

WMAP_STEP_RUNNER(wmap_land_effect_31_run_sequence_17, D_800D769C, 4, g_wmap_land_effect_31_sequence_17_step, g_wmap_land_effect_31_sequence_17_timer)

WMAP_STEP_RESET(wmap_land_effect_31_sequence_17_reset, g_wmap_land_effect_31_sequence_17_step, g_wmap_land_effect_31_sequence_17_timer)

void wmap_land_effect_31_sequence_17_step_01(void)
{
    WmapSpriteActor* actor = &g_wmap_sprite_actors[7];

    g_wmap_actor_animations[7].data = &g_wmap_animation_bank_0;
    actor->scale_index = 0xF;
    actor->sequence = 1;
    actor->previous_sequence = -1;
    actor->shade_step = 4;
    actor->resource_index = 0;
    actor->target_shade = 0x61;
    actor->shade = 1;
    g_wmap_land_effect_31_sequence_17_timer = 0x64;
    g_wmap_land_effect_31_sequence_17_step++;
    wmap_land_effect_31_sequence_17_step_02();
}

void wmap_land_effect_31_sequence_17_step_02(void)
{
    s32 timer;
    WmapSpriteActor* config;

    config = &g_wmap_sprite_actors[7];
    wmap_step_actor_animation(config, &g_wmap_actor_animations[7]);
    wmap_draw_actor_sprite(config, g_wmap_focus_screen_position.packed, 0x2B, 8, 0);
    timer = g_wmap_land_effect_31_sequence_17_timer - 1;
    g_wmap_land_effect_31_sequence_17_timer = timer;
    if (timer == 0)
    {
        g_wmap_land_effect_31_sequence_17_step++;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_31_sequence_17_end, g_wmap_land_effect_31_sequence_17_step)

WMAP_STEP_RUNNER(wmap_land_effect_31_run_sequence_18, D_800D76AC, 6, g_wmap_land_effect_31_sequence_18_step, g_wmap_land_effect_31_sequence_18_timer)

WMAP_STEP_RESET(wmap_land_effect_31_sequence_18_reset, g_wmap_land_effect_31_sequence_18_step, g_wmap_land_effect_31_sequence_18_timer)

void wmap_land_effect_31_sequence_18_step_01(void)
{
    WmapSpriteActor* actor = &g_wmap_sprite_actors[8];

    g_wmap_actor_animations[8].data = &g_wmap_animation_bank_0;
    actor->scale_index = 0xF;
    actor->sequence = 2;
    actor->previous_sequence = -1;
    actor->shade_step = 2;
    actor->resource_index = 0;
    actor->target_shade = 0x81;
    actor->shade = 1;
    g_wmap_land_effect_31_sequence_18_timer = 0x6B;
    g_wmap_land_effect_31_sequence_18_step++;
    wmap_land_effect_31_sequence_18_step_02();
}

void wmap_land_effect_31_sequence_18_step_02(void)
{
    s32 timer;
    WmapSpriteActor* config;

    config = &g_wmap_sprite_actors[8];
    wmap_step_actor_animation(config, &g_wmap_actor_animations[8]);
    wmap_draw_actor_sprite(config, g_wmap_focus_screen_position.packed, 0x10, 8, 0);
    timer = g_wmap_land_effect_31_sequence_18_timer - 1;
    g_wmap_land_effect_31_sequence_18_timer = timer;
    if (timer == 0)
    {
        g_wmap_land_effect_31_sequence_18_step++;
    }
}

void wmap_land_effect_31_sequence_18_step_03(void)
{
    WmapSpriteActor* actor = &g_wmap_sprite_actors[8];

    actor->shade_step = 8;
    actor->target_shade = 0;
    g_wmap_land_effect_31_sequence_18_timer = 0x10;
    g_wmap_land_effect_31_sequence_18_step++;
    wmap_land_effect_31_sequence_18_step_04();
}

void wmap_land_effect_31_sequence_18_step_04(void)
{
    s32 timer;
    WmapSpriteActor* config;

    config = &g_wmap_sprite_actors[8];
    wmap_step_actor_animation(config, &g_wmap_actor_animations[8]);
    wmap_draw_actor_sprite(config, g_wmap_focus_screen_position.packed, 0x10, 8, 0);
    timer = g_wmap_land_effect_31_sequence_18_timer - 1;
    g_wmap_land_effect_31_sequence_18_timer = timer;
    if (timer == 0)
    {
        g_wmap_land_effect_31_sequence_18_step++;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_31_sequence_18_end, g_wmap_land_effect_31_sequence_18_step)

WMAP_STEP_RUNNER(wmap_land_effect_31_run_sequence_19, D_800D76C4, 6, g_wmap_land_effect_31_sequence_19_step, g_wmap_land_effect_31_sequence_19_timer)

WMAP_STEP_RESET(wmap_land_effect_31_sequence_19_reset, g_wmap_land_effect_31_sequence_19_step, g_wmap_land_effect_31_sequence_19_timer)

void wmap_land_effect_31_sequence_19_step_01(void)
{
    WmapSpriteActor* actor = &g_wmap_sprite_actors[9];

    g_wmap_actor_animations[9].data = &g_wmap_animation_bank_0;
    actor->scale_index = 0xF;
    actor->sequence = 3;
    actor->previous_sequence = -1;
    actor->shade_step = 2;
    actor->target_shade = 0x81;
    actor->resource_index = 0;
    actor->shade = 1;
    g_wmap_land_effect_31_sequence_19_timer = 0xB4;
    g_wmap_land_effect_31_sequence_19_step++;
    wmap_land_effect_31_sequence_19_step_02();
}

void wmap_land_effect_31_sequence_19_step_02(void)
{
    s32 timer;
    WmapSpriteActor* config;

    config = &g_wmap_sprite_actors[9];
    wmap_step_actor_animation(config, &g_wmap_actor_animations[9]);
    wmap_draw_actor_sprite(config, g_wmap_focus_screen_position.packed, 0x10, 8, 0);
    timer = g_wmap_land_effect_31_sequence_19_timer - 1;
    g_wmap_land_effect_31_sequence_19_timer = timer;
    if (timer == 0)
    {
        g_wmap_land_effect_31_sequence_19_step++;
    }
}

void wmap_land_effect_31_sequence_19_step_03(void)
{
    WmapSpriteActor* actor = &g_wmap_sprite_actors[9];

    actor->shade_step = 4;
    actor->target_shade = 0;
    g_wmap_land_effect_31_sequence_19_timer = 0x20;
    g_wmap_land_effect_31_sequence_19_step++;
    wmap_land_effect_31_sequence_19_step_04();
}

void wmap_land_effect_31_sequence_19_step_04(void)
{
    s32 timer;
    WmapSpriteActor* config;

    config = &g_wmap_sprite_actors[9];
    wmap_step_actor_animation(config, &g_wmap_actor_animations[9]);
    wmap_draw_actor_sprite(config, g_wmap_focus_screen_position.packed, 0x10, 8, 0);
    timer = g_wmap_land_effect_31_sequence_19_timer - 1;
    g_wmap_land_effect_31_sequence_19_timer = timer;
    if (timer == 0)
    {
        g_wmap_land_effect_31_sequence_19_step++;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_31_sequence_19_end, g_wmap_land_effect_31_sequence_19_step)

WMAP_STEP_RUNNER(wmap_land_effect_31_run_sequence_20, D_800D76DC, 6, g_wmap_land_effect_31_sequence_20_step, g_wmap_land_effect_31_sequence_20_timer)

WMAP_STEP_RESET(wmap_land_effect_31_sequence_20_reset, g_wmap_land_effect_31_sequence_20_step, g_wmap_land_effect_31_sequence_20_timer)

void wmap_land_effect_31_sequence_20_step_02(void)
{
    s32 timer;

    func_8006C448(&g_wmap_effect_params[40]);
    timer = g_wmap_land_effect_31_sequence_20_timer - 1;
    g_wmap_land_effect_31_sequence_20_timer = timer;
    if (timer == 0)
    {
        g_wmap_land_effect_31_sequence_20_step++;
    }
}

void wmap_land_effect_31_sequence_20_step_03(void)
{
    s32 i;
    WmapSpriteActor* actors = &g_wmap_sprite_actors[200];

    for (i = 0; i < 0x28; i++)
    {
        actors[i].target_shade = 0;
        actors[i].shade_step = 0x20;
    }
    g_wmap_land_effect_31_sequence_20_timer = 4;
    g_wmap_land_effect_31_sequence_20_step++;
    wmap_land_effect_31_sequence_20_step_04();
}

void wmap_land_effect_31_sequence_20_step_04(void)
{
    s32 timer;

    func_8006C448(&g_wmap_effect_params[40]);
    timer = g_wmap_land_effect_31_sequence_20_timer - 1;
    g_wmap_land_effect_31_sequence_20_timer = timer;
    if (timer == 0)
    {
        g_wmap_land_effect_31_sequence_20_step++;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_31_sequence_20_end, g_wmap_land_effect_31_sequence_20_step)
