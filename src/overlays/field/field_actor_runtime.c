/** @file field_actor_runtime.c
 * @brief Coordinate actor resources, animation tracks, and actor slot runtime state.
 */

#include "common.h"
#include "field_types.h"
#include "field_actor_runtime.h"
#include "cd_resources.h"
#include "sdk/libgpu.h"

s32 cdrom_stream(s32 resourceIndex, u32 destination);
void cdrom_wait_queue_empty(void);
extern void func_80084240(void);
void func_80140004(s32 cdLoadAddr, s32 imageResourceIndex, s32 musicResourceIndex, s32 audioClipIndex);
void func_800A74E8();
void func_800AA02C();

typedef struct
{
    char pad_00[0x1EC];
    u16 unk1EC[35];
    u8 unk232;
    char pad_233;
    u16 unk234;
    u16 unk236;
    u16 unk238;
    u8 unk23A;
} FieldActorTrackState;

typedef struct
{
    s16 red;
    s16 green;
    s16 blue;
    s16 duration;
} FieldFadeTarget;

typedef struct
{
    s16 red;
    s16 green;
    s16 blue;
    s16 unk6;
} FieldFadeRestoreColor;

typedef struct
{
    u8 pad[0x1EC];
    u16 track_frame;
} FieldTrackCounterView;

typedef struct
{
    char pad_00[0x32];
    u8 unk32;
    char pad_33[0x48 - 0x33];
} FieldActorPartState;
typedef struct
{
    FieldActorPartState* parts;
    char pad_04[0x21];
    u8 unk25;
    char pad_26[0x5];
    u8 unk2B[9];
    char pad_34[7];
    u8 inner1[9][16];
    char pad_CB[1];
    u16 inner2[9][16];
    u16 unk1EC[9];
    char pad_1FE[0x36];
    u16 unk234;
    u16 unk236;
} FieldActorResetState;

typedef struct
{
    u8 pad0[2];
    u16 map0_action1_animation_id;
    u8 pad1[6];
    u16 map0_action5_animation_id;
    u8 pad2[11];
    u8 map0_action1_disabled;
    u8 pad3[3];
    u8 map0_action5_disabled;
    u8 pad4[8];
    u16 map1_action1_animation_id;
    u8 pad5[6];
    u16 map1_action5_animation_id;
    u8 pad6[11];
    u8 map1_action1_disabled;
    u8 pad7[3];
    u8 map1_action5_disabled;
} FieldActionAnimationMaps;

extern FieldActionAnimationMaps g_field_action_animation_maps;

typedef struct FieldActorAnimationDef
{
    u8 unk0[2];
    u8 pad2[0xC - 2];
    u16 unkC;
    u16 unkE;
    u16 unk10;
    u8 pad10[0x14 - 0x12];
    u8 unk14;
    u8 unk15;
    u8 pad16[0x18 - 0x16];
    u16 unk18;
} FieldActorAnimationDef;
typedef struct
{
    union
    {
        u32 w;
        struct
        {
            unsigned pad : 24;
            unsigned mode : 2;
            unsigned rest : 6;
        } b;
    } u0;
    union
    {
        u32 w;
        struct
        {
            unsigned pad : 11;
            unsigned flag : 1;
            unsigned pad2 : 10;
            unsigned mode : 2;
            unsigned rest : 8;
        } b;
    } u4;
    u8 unk8;
    u8 unk9;
    u8 padA;
    u8 unkB;
    u8 unkC;
    u8 unkD;
    u8 unkE;
    u8 unkF;
    u8 unk10;
    u8 unk11;
    u8 pad12[0x14 - 0x12];
    union
    {
        u32 w;
        struct
        {
            unsigned pad : 4;
            unsigned mode : 4;
            unsigned rest : 24;
        } b;
        struct
        {
            u16 lo;
            s16 hi;
        } h;
    } u14;
    s16 unk18;
    u8 pad1A[0x23 - 0x1A];
    u8 unk23;
    union
    {
        u32 w;
        struct
        {
            u8 unk24;
            u8 unk25;
            u8 unk26;
            u8 unk27;
        } b;
    } u24;
    u32 unk28;
    u8 unk2C;
    u8 pad2D;
    u8 unk2E;
    u8 pad2F[0x31 - 0x2F];
    u8 unk31;
    u8 unk32;
    u8 unk33;
    u32 unk34;
    u8 pad38[0x48 - 0x38];
} FieldActorPartDef;

typedef struct
{
    u8 pad0[0x91];
    u8 unk91;
    u8 unk92;
    u8 pad93[0x13F - 0x93];
    u8 unk13F;
    u8 unk140;
} FieldGoverLoadState;

typedef struct
{
    u8 unk0;
    u8 pad1;
    u8 unk2;
    u8 unk3[16];
    u8 unk13[9][16];
    u8 padA3;
    u16 unkA4[9][16];
    u16 unk1C4[9];
    u8 pad1D6[0x1FA - 0x1D6];
    u16 unk1FA;
    union
    {
        u32 unk1FC;
        struct
        {
            u16 lo;
            u16 animation_id;
        } h;
    } u1FC;
    u8 owner_object_index;
    u8 unk201[9];
    u8 unk20A;
    u8 unk20B;
    u16 unk20C;
    u16 unk20E;
    u16 unk210;
    u8 unk212;
    u8 unk213;
    u8 pad214[0x21C - 0x214];
} FieldActorTrackData;

typedef struct
{
    FieldActorPartDef* unk0;
    u8 pad4[0xC - 4];
    FieldActorAnimationDef* unkC;
    u8 pad10[0x24 - 0x10];
    u8 unk24;
    u8 unk25;
    u8 unk26;
    u8 unk27;
    u8 unk28;
    u8 unk29;
    u8 unk2A;
    u8 unk2B[16];
    u8 unk3B[9][16];
    u8 padCB;
    u16 unkCC[9][16];
    u16 unk1EC[9];
    u8 pad1FE[0x222 - 0x1FE];
    u16 unk222;
    union
    {
        u32 unk224;
        struct
        {
            u16 lo;
            u16 animation_id;
        } h;
    } u224;
    u8 owner_object_index;
    u8 unk229[9];
    u8 unk232;
    u8 unk233;
    u16 unk234;
    u16 unk236;
    u8 pad238[2];
    u8 unk23A;
    u8 unk23B;
    u8 pad23C[0x240 - 0x23C];
    u16* unk240;
} FieldActorState;

typedef struct
{
    s32 unk0;
    s32 unk4;
    s32 unk8;
    u32 unkC;
    s16 unk10;
    s16 unk12;
    s16 unk14;
    s16 unk16;
    u8 unk18;
    u8 unk19;
    u8 unk1A;
    u8 unk1B;
    s32 unk1C;
    u8 pad20[0x21 - 0x20];
    u8 unk21;
    u8 unk22;
    u8 unk23;
    u8 unk24;
    u8 unk25;
    u8 pad26[0x27 - 0x26];
    u8 unk27;
    u8 unk28;
    u8 pad29[0x2A - 0x29];
    s16 unk2A;
    s16 unk2C;
    u16 unk2E;
    s16 unk30;
    u8 unk32;
    u8 unk33;
    u8 unk34;
    u8 unk35;
    u8 unk36;
    u8 unk37;
    u8 unk38;
    u8 pad39[0x3A - 0x39];
    u8 unk3A;
    u8 unk3B;
    u32 unk3C;
    s32 unk40;
    u32 unk44;
    u32 unk48;
    u32 unk4C;
    u8 pad50[0x54 - 0x50];
} FieldActorObjectRecord;

typedef struct
{
    u8 pad0[0xC];
    u32 unkC;
    u32 unk10;
    u8 pad14[0x12C - 0x14];
    u32 unk12C;
    u8 pad130[0x140 - 0x130];
    s16 unk140;
    s16 unk142;
    s16 unk144;
    s16 unk146;
    u8 pad148[0x174 - 0x148];
    s32 unk174;
    union
    {
        s32 unk178;
        struct
        {
            u8 pad[2];
            u8 unk17A;
            u8 pad2;
        } b;
    } u;
    u8 pad17C[0x18E - 0x17C];
    u8 unk18E;
    u8 pad18F[0x19C - 0x18F];
    s32 unk19C;
    s32 unk1A0;
    u8 pad1A4[0x1A8 - 0x1A4];
    u8 unk1A8;
    u8 unk1A9;
    u8 unk1AA;
    u8 pad1AB[0x23C - 0x1AB];
} FieldActorObjectState;

typedef struct
{
    u8 segment_count_flags;
    u8 segment_offset;
    s16 end_value;
    s16 start_value;
} FieldParameterCurve;

typedef struct
{
    u8 pad[4];
    FieldParameterCurve* curves;
    u16* segments;
} FieldAnimationData;

typedef struct
{
    FieldActorPartDef* unk0;
    u8 pad4[0xC - 4];
    FieldActorAnimationDef* unkC;
    u8 pad10[0x24 - 0x10];
    u8 unk24;
    u8 unk25;
    u8 unk26;
    u8 unk27;
    FieldActorTrackData unk28;
} FieldActorStateWithTracks;

typedef struct
{
    u8 pad000[0x232];
    u8 unk232;
    u8 pad233[0x5];
    u16 unk238;
    u8 unk23A;
    u8 unk23B;
} FieldActorTrackMaskState;

typedef struct
{
    u8 pad[0x40];
    u32 unk40;
    u8 pad44[0x40B8 - 0x44];
    void* unk40B8;
} FieldRenderContext;

typedef union
{
    struct
    {
        s16 red;
        s16 green;
        s16 blue;
    } s;
    u32 w; /* Packed red/green view. */
} FieldColorScale;

/** @brief Runtime descriptor for one of the three actor resource slots. */
typedef struct
{
    union
    {
        u16 h;
        struct
        {
            u8 unk0;
            u8 unk1;
        } b;
    } u0;
    u8 unk2;
    u8 unk3;
    u8 pad0[0x254 - 4];
    u16 unk254;
    u8 unk256;
    u8 pad1[0x268 - 0x257];
} FieldActorResourceSlot;

typedef struct
{
    u8* start;
    u8* end;
    u8 unk8;
    u8 slot_index;
    u8 padA[0xE - 0xA];
    s16 unkE;
    u32 flags;
} FieldResourceEntry;

typedef struct
{
    u8 _pad000[0x840];
    u8 inject_enable;
    u8 _pad841[0x858 - 0x841];
    u32 inject_flags;
    u8 pad85C[0x24C];
    u32 unkAA8;
} PadContext;

typedef struct
{
    s16 unk0;
    s16 unk2;
} Struct_D800EB254;

/** @brief Staging buffer that field CD reads land in. */
typedef struct
{
    u32 unk0;
    u32 unk4;
    s32 unk8;
    u32 unkC;
    u16 unk10;
    u16 unk12;
    u16 unk14;
} FieldCdBuffer;

extern FieldActorResourceSlot D_800FD818[];

extern FieldColorScale g_field_color_scale;
extern s8 g_field_color_scale_active;

extern s32 D_801227C8;
extern s32 g_field_gover_image_resource_id;
extern s32 g_field_gover_music_resource_id;
extern s32 g_field_gover_load_countdown;
extern s32 g_field_gover_audio_clip_id;
extern s32 g_field_return_to_title_prompt_delay;
extern s32 g_field_return_to_title_prompt_state;
extern s32 D_8012291C;
extern s32 g_field_audio_timer;
extern s32 g_field_track_index;
extern s32 D_8011588C;
extern FieldFadeTarget g_field_fade_target;
extern FieldFadeRestoreColor g_field_fade_restore_color;
extern s32 D_800F2278;
extern s32 D_800F227C;
extern s32 D_800F2280;
extern FieldActorState g_field_actor_slots[80];
extern FieldActorObjectRecord D_800FDF58[];
extern FieldActorObjectState D_80105AE0[];
extern u8 D_800FF59C;

extern void* bcopy(const void*, void*, int);
extern u8* g_pad_ctx;
extern u8 g_field_resource_buffer[];
extern u16 D_800FDA80;
extern FieldActorPartDef D_800FE3A0[];
extern FieldActorAnimationDef D_800FE758;
extern s32 D_800FE774;
extern FieldResourceEntry g_field_resource_entries[];
extern void* g_field_resource_cursor;
extern s32 D_801158A0;
extern Struct_D800EB254 D_800EB254[];
extern u8 D_800FDA81;
extern u8 D_800FDA83;
extern u8 D_800FDCEA;
extern s32 D_800FE754;
extern s32 D_80122710;
extern s32 D_80122714;
extern s32 D_80122B20;
extern FieldActorObjectRecord D_800FF658[];
extern s32 D_80105770;
extern s32 D_800F22A0;
extern s32 D_800F22A4;
extern s32 D_800F22A8;
extern s32 D_800F2298;
extern s32 D_8012269C;
extern s32 D_80105760;
extern FieldCdBuffer* D_8010D038;
extern u8 g_prim_rect_buf[];
FieldActorObjectRecord* func_80087C9C(s32);
void func_8009C2E0(FieldActorObjectRecord* entry, s32* args);
void field_relocate_resource_buffer(s32);
s32 field_get_actor_resource_id(s32, FieldActorResourceSlot*, s32);
void field_load_actor_resource_slot(s32, s32, s32, s32);
void field_initialize_actor_record(s32, s32);
void field_initialize_actor_part(s32, s32);
void func_80091438(s32);
void func_800A3D44(s32, u8);

extern s32 D_800F229C;
extern s32 D_8010D020[];
void func_800A710C(void);
void field_text_reset_scratch(void);
void func_800A8880(s32);
void func_80063194(void);
void func_800A9A5C(void);
void func_800A68B4(void);
void func_800A7434(void);
void func_800A74B8(void);
void field_text_reset_windows(void);
void func_80092124(void);
void cdrom_queue_seek(s32);
void akao_cmd_c1(s32, s32, s32);
void akao_cmd_a9(s32, s32);

/**
 * @brief Advance the active field dialog runtime and finish any pending text work.
 * @param update_mode Mode forwarded to the active dialog update helper.
 * @see decomp.me (100%) TODO
 */
void field_update_dialog_runtime(s32 update_mode)
{
    if (D_800F229C != 0)
    {
        func_800A710C();
        if (D_800F229C != 0)
        {
            field_text_reset_scratch();
            if (D_800F229C != 0)
            {
                func_800A8880(update_mode);
            }
            func_80063194();
        }
    }
}

/**
 * @brief Restore the standard field fade and refresh the active dialog/menu state.
 * @see decomp.me (100%) TODO
 */
void func_80068028(void)
{
    func_800AA02C();
    g_field_fade_target.red = 0xC0;
    g_field_fade_target.green = 0xC0;
    g_field_fade_target.blue = 0xC0;
    g_field_fade_target.duration = 5;
    if (D_8010D020[0] == 0)
    {
        func_800A9A5C();
        func_800A68B4();
        func_800A7434();
    }
    else
    {
        func_800A74B8();
    }
}

/**
 * @brief Close the active dialog state and restore the first two actor slots.
 * @see decomp.me (100%) TODO
 */
void func_8006809C(void)
{
    s32 i;

    g_field_fade_target.red = g_field_fade_restore_color.red;
    g_field_fade_target.green = g_field_fade_restore_color.green;
    g_field_fade_target.blue = g_field_fade_restore_color.blue;
    g_field_fade_target.duration = 5;
    field_text_reset_windows();

    D_800F229C = 0;

    for (i = 0; i < 2; i++)
    {
        if (D_800FD818[i].u0.b.unk0 & 1)
        {
            D_800FDF58[i].unk2A = 0x9A;
            D_800FDF58[i].unk2E = 1;
            D_800FDF58[i].unk27 = 0;
            D_800FDF58[i].unk24 = 1;
            D_800FDF58[i].unk1C &= ~0x1FF;
            D_800FDF58[i].unk21 = (D_800FDF58[i].unk21 & 0x80) + 0x12;
            D_80105AE0[i].unk174 &= ~0x1800;
            field_restart_actor_animation(&D_800FDF58[i], (void*)~0x1FF);
        }
    }

    field_restore_default_action_animation_mappings(0);
    D_8012291C = 0;
    func_80092124();
}

/**
 * @brief Open the return-to-title confirmation prompt when the command value is zero.
 * @param command_value Command value; nonzero values leave the prompt unchanged.
 * @see decomp.me (100%) TODO
 */
void func_800681C0(s32 command_value)
{
    if (command_value == 0)
    {
        field_open_return_to_title_prompt();
    }
}

/**
 * @brief Start the timed transition that loads the game-over overlay.
 * @param image_resource_index Image resource passed to the game-over overlay.
 * @param music_resource_index Music resource passed to the game-over overlay.
 * @param audio_clip_index Audio clip passed to the game-over overlay.
 * @see decomp.me (100%) TODO
 */
void field_begin_gover_transition(s32 image_resource_index, s32 music_resource_index, s32 audio_clip_index)
{
    s32 fade_time;

    if (g_field_gover_load_countdown == 0)
    {
        g_field_gover_image_resource_id = image_resource_index;
        g_field_gover_music_resource_id = music_resource_index;
        g_field_gover_audio_clip_id = audio_clip_index;
        func_80084240();
        g_field_fade_target.red = 0;
        g_field_fade_restore_color.red = 0;
        g_field_fade_target.green = 0;
        g_field_fade_restore_color.green = 0;
        g_field_fade_target.blue = 0;
        g_field_fade_restore_color.blue = 0;
        fade_time = 0x3C;
        g_field_fade_target.duration = fade_time;
        cdrom_queue_seek(0xA);
        g_field_gover_load_countdown = fade_time;
        akao_cmd_c1(0, 0x78, 0);
        akao_cmd_a9(0x78, 0);
    }
}

/**
 * @brief Advance the staged game-over resource load and transition.
 * decomp.me (100%) https://decomp.me/scratch/9Ady0
 */
void field_update_gover_load(void)
{
    FieldGoverLoadState* ptr = (FieldGoverLoadState*)0x801ED600;

    if (g_field_gover_load_countdown == 0)
    {
        return;
    }

    if (--g_field_gover_load_countdown == 0)
    {
        ptr->unk140 = 0;
        ptr->unk92 = 0;
        ptr->unk13F = 0;
        ptr->unk91 = 0;
        cdrom_stream(CD_RES_GOVER_BIN, 0x80140000);
        cdrom_wait_queue_empty();
        func_80140004(0x80160000, g_field_gover_image_resource_id, g_field_gover_music_resource_id, g_field_gover_audio_clip_id);
        func_80084240();
    }
}

/**
 * @brief Update and render the modal return-to-title confirmation prompt.
 * @param render_ctx Current field render context passed to the prompt renderer.
 * @see decomp.me (100%) https://decomp.me/scratch/Kws0l
 */
void field_update_return_to_title_prompt(s32 render_ctx)
{

    if (g_field_return_to_title_prompt_state != 0)
    {

        if (g_field_return_to_title_prompt_delay != 0)
        {
            g_field_return_to_title_prompt_delay--;
            if (g_field_return_to_title_prompt_delay == 0)
            {
                g_field_fade_target.red = 0xC0;
                g_field_fade_target.green = 0xC0;
                g_field_fade_target.blue = 0xC0;
                g_field_fade_target.duration = 5;
            }
        }
        else
        {
            func_800A6F1C();
            if (g_field_return_to_title_prompt_state != 0)
            {
                field_text_reset_scratch();
                if (g_field_return_to_title_prompt_state != 0)
                {
                    func_800A8880(render_ctx);
                }
                func_80063194();
            }
        }
    }
}

/**
 * @brief Initialize and open the return-to-title confirmation prompt.
 * @see decomp.me (100%) https://decomp.me/scratch/doJjR
 */
void field_open_return_to_title_prompt(void)
{
    func_800AA02C();
    func_800A74E8();
}

/**
 * @brief Begin closing the return-to-title prompt and reset its field effects.
 * @see decomp.me (100%) https://decomp.me/scratch/b8yys
 */
void field_begin_return_to_title_prompt_close(void)
{
    g_field_fade_target.red = 0;
    g_field_fade_restore_color.red = 0;
    g_field_fade_target.green = 0;
    g_field_fade_restore_color.green = 0;
    g_field_fade_target.blue = 0;
    g_field_fade_restore_color.blue = 0;
    g_field_fade_target.duration = 8;
    field_text_reset_windows();
    D_8012291C = 0;
}

/**
 * @brief Advance the deferred field audio timer and start playback when it expires.
 * decomp.me (100%) https://decomp.me/scratch/KDXt0
 */
void field_update_audio_timer(void)
{
    s32 remaining_frames;

    if (g_field_audio_timer != 0)
    {
        remaining_frames = g_field_audio_timer - 1;
        g_field_audio_timer = remaining_frames;
        if (remaining_frames == 0)
        {
            akao_song_cmd_12c();
            func_800A380C();
            func_800A3904(0, 1, D_8011588C);
        }
    }
}

/**
 * @brief Return the active animation track frame modulo a divisor.
 * @param animation_data Animation state containing the per-track frame counters.
 * @param divisor Divisor applied to the active track frame.
 * @return Current active-track frame modulo @p divisor.
 * decomp.me (100%) https://decomp.me/scratch/Xn30r
 */
s32 field_get_track_counter_modulo(s32 animation_data, s32 divisor)
{
    return ((FieldTrackCounterView*)((u8*)animation_data + g_field_track_index * 2))->track_frame % divisor;
}

/**
 * @brief Interpolate a 16-color palette from the active animation track.
 * @param animation_data Animation data containing the palette track.
 * @param palette_sequence Palette curve selector.
 * @param palette_table Palette table selector.
 * @param output Destination for the 16 interpolated colors.
 * decomp.me (100%) https://decomp.me/scratch/X9uyL
 */
void field_interpolate_palette_track(FieldAnimationData* animation_data, s32 palette_sequence, s32 palette_table, s16* output)
{
    s32 segment_start_frame = 0;
    s32 selector_shift = 4;
    s16* output_color;
    s32 color_index;
    s32 palette_selectors;
    s32 segments_remaining = (&animation_data->curves[palette_sequence & 0xF])->segment_count_flags & 0x7F;
    u16* segment = &animation_data->segments[(&animation_data->curves[palette_sequence & 0xF])->segment_offset];
    palette_selectors = palette_sequence;
    if (segments_remaining != 0)
    {
        do
        {
            u16 temp = *segment;
            s32 segment_end_frame = segment_start_frame + (temp & 0x3FF);
            if (((FieldTrackCounterView*)(((u8*)animation_data) + (g_field_track_index * 2)))->track_frame < segment_end_frame)
            {
                break;
            }
            segment_start_frame = segment_end_frame;
            segment++;
            selector_shift += 4;
            segments_remaining--;
            if (selector_shift == 16)
            {
                selector_shift = 4;
            }
        } while (segments_remaining != 0);
    }
    if (segments_remaining != 0)
    {
        s32 selectors = palette_selectors & 0xFFFF;
        u16* start_palette = (u16*)(palette_table + (((selectors >> selector_shift) & 0xF) << 5));
        u16* end_palette;
        if ((selector_shift + 4) != 16)
        {
            end_palette = (u16*)(palette_table + (((selectors >> (selector_shift + 4)) & 0xF) << 5));
        }
        else
        {
            end_palette = (u16*)(palette_table + ((selectors << 1) & 0x1E0));
        }
        output_color = output;
        color_index = 0;
        do
        {
            u16 a = *start_palette;
            u16 b = *end_palette;
            s32 low_a = a & 0x1F;
            s32 low_b = b & 0x1F;
            s32 diff0 = low_b - low_a;
            s32 frame_offset = ((FieldTrackCounterView*)(((u8*)animation_data) + (g_field_track_index * 2)))->track_frame - segment_start_frame;
            s32 segment_length = (*segment) & 0x3FF;
            s32 red_step = (diff0 * frame_offset) / segment_length;
            s32 mid_a = (a >> 5) & 0x1F;
            s32 mid_b = (b >> 5) & 0x1F;
            s32 diff1 = mid_b - mid_a;
            s32 green_step = (diff1 * frame_offset) / segment_length;
            s32 high_a = (a >> 10) & 0x1F;
            s32 high_b = (b >> 10) & 0x1F;
            s32 diff2 = high_b - high_a;
            s32 blue_step = (diff2 * frame_offset) / segment_length;
            *output_color = (((a & 0x8000) | (low_a + red_step)) | ((mid_a + green_step) << 5)) | ((high_a + blue_step) << 10);
            output_color++;
            color_index++;
            start_palette++;
            end_palette++;
        } while (color_index < 16);
    }
}

/**
 * @brief Evaluate an animation parameter curve at the active track frame.
 * @param animation_data Animation data containing the parameter curves.
 * @param curve_index Parameter curve index.
 * @return Evaluated parameter value.
 * decomp.me (100%) https://decomp.me/scratch/t5bIj
 */
s32 field_evaluate_parameter_track(FieldAnimationData* animation_data, s32 curve_index)
{
    s32 segment_start_frame;
    FieldParameterCurve* curve;
    s32 start_delta;
    s32 value_range;
    u16* segment;
    s32 segments_remaining;
    u16 current_frame;
    u16 segment_word;
    s32 segment_end_frame;
    u16 final_segment_word;
    s32 rand_val;
    segment_start_frame = 0;
    curve = animation_data->curves + curve_index;
    start_delta = segment_start_frame;
    segments_remaining = curve->segment_count_flags & 0x7F;
    segment = animation_data->segments + curve->segment_offset;
    if (segments_remaining != 0)
    {
        current_frame = *((u16*)((((u8*)animation_data) + (g_field_track_index * 2)) + 0x1EC));
        do
        {
            if (!animation_data)
            {
            }
            segment_word = *segment;
            segment_end_frame = segment_start_frame + (segment_word & 0x3FF);
            if (((s32)current_frame) < segment_end_frame)
            {
                break;
            }
            segment_start_frame = segment_end_frame;
            start_delta = segment_word >> 10;
            segments_remaining--;
            segment++;
        } while (segments_remaining != 0);
    }
    value_range = curve->end_value - curve->start_value;
    start_delta = (value_range * start_delta) >> 5;
    if (segments_remaining == 0)
    {
        return curve->start_value;
    }
    if (((*((u16*)curve)) & 0x80) && (D_801227C8 == 0))
    {
        rand_val = rand();
        return curve->start_value +
               (((start_delta + (((((value_range * ((*segment) >> 10)) >> 5) - start_delta) * ((*((u16*)((((u8*)animation_data) + (g_field_track_index * 2)) + 0x1EC))) - segment_start_frame)) /
                            ((*segment) & 0x3FF))) *
                 rand_val) >>
                15);
    }
    else
    {
        final_segment_word = *segment;
        return (curve->start_value + start_delta) +
               (((((value_range * (final_segment_word >> 10)) >> 5) - start_delta) * ((*((u16*)((((u8*)animation_data) + (g_field_track_index * 2)) + 0x1EC))) - segment_start_frame)) / (final_segment_word & 0x3FF));
    }
}

/**
 * @brief Process completion state for an actor animation.
 * @param actor Actor animation state to finalize.
 * @return Nonzero when the animation has completed, otherwise zero.
 * decomp.me (100%) https://decomp.me/scratch/MCyYP
 */
s32 field_finalize_actor_animation(FieldActorState* actor)
{
    s32 found;
    u8 state_value;
    s32 i;
    FieldActorState* other_actor;
    s16 object_state;
    if (actor->unk222 == actor->unk1EC[0])
    {
        if (actor->unkC->unk18 & 0x20)
        {
            for (i = 8; i >= 0; i--)
            {
                actor->unk1EC[i] = 0;
            }

            return 0;
        }
        actor->unk23A = 0;
    }
    if (actor->unk23A == 0)
    {
        actor->unk23B = 0;
        if (actor->unkC->unk18 & 2)
        {
            if (D_80105AE0[actor->owner_object_index].u.b.unk17A == actor->unk233)
            {
                object_state = D_800FDF58[actor->owner_object_index].unk2A;
                if (((object_state != 0x90) && (object_state != 0x94)) || (D_80105AE0[actor->owner_object_index].unkC & 0x200))
                {
                    D_800FDF58[actor->owner_object_index].unk25 = 0;
                }
                D_80105AE0[actor->owner_object_index].u.unk178 &= ~1;
            }
        }
        if (actor->unkC->unk18 & 4)
        {
            for (found = 0; found < ((s32)actor->unk232); found++)
            {
                if (actor->unk229[found] != 0xFF)
                {
                    state_value = D_80105AE0[actor->unk229[found]].u.unk178;
                    if ((state_value & 1) && (D_80105AE0[actor->unk229[found]].u.b.unk17A == actor->unk233))
                    {
                        D_800FDF58[actor->unk229[found]].unk25 = 0;
                        D_80105AE0[actor->unk229[found]].u.unk178 &= ~1;
                    }
                }
            }
        }
        if (actor->unkC->unkC & 0x1000)
        {
            D_800F2280 = 0;
            D_800F227C = 0;
            D_800F2278 = 0;
        }
        if ((actor->unkC->unkC >> 8) & 4)
        {
            s32 j;
            other_actor = g_field_actor_slots;
            for (j = 0; j < 80; j++, other_actor++)
            {
                found = 0;
                if (((actor != other_actor) && (other_actor->unk24 != 0)) && ((other_actor->unkC->unkC >> 8) & 4))
                {
                    found = 1;
                    break;
                }
            }

            if (found == 0)
            {
                field_set_global_color_scale(0x100, 0x100, 0x100);
            }
        }
        if ((*(u32*)&actor->u224.unk224 & 0xFFFF0001) != 0xC0000)
        {
            field_clear_actor_effects(actor);
        }
        if (!(actor->unkC->unkC & 0x800))
        {
            actor->unk222 = 0;
            actor->unk24 = 0;
            if (*(u32*)(&actor->u224.unk224) & 1)
            {
                func_80084424(actor->owner_object_index);
            }
            return 1;
        }
        state_value = actor->unk2A;
        if (state_value != 0)
        {
            actor->unk222 = 0;
            actor->unk24 = 0;
            func_80084424(actor->owner_object_index);
            for (i = 0; i < 80; i++)
            {
                if (((g_field_actor_slots[i].unk24 != 0) && (g_field_actor_slots[i].owner_object_index == actor->owner_object_index)) &&
                    ((state_value = g_field_actor_slots[i].u224.unk224) & 1))
                {
                    g_field_actor_slots[i].unk23A = 0;
                    field_finalize_actor_animation(&g_field_actor_slots[i]);
                }
            }

            actor->unk2A = 0;
        }
        else
        {
            actor->unk222 = 0;
            actor->unk24 = 0;
        }
        return 1;
    }
    return 0;
}

/**
 * @brief Evaluate an animation parameter curve at an explicit frame.
 * @param animation_data Animation data containing the parameter curves.
 * @param curve_index Parameter curve index.
 * @param frame Frame at which to evaluate the curve.
 * @return Evaluated parameter value.
 * decomp.me (100%) https://decomp.me/scratch/7d7kv
 */
unsigned int field_evaluate_parameter_track_at_time(FieldAnimationData* animation_data, s32 curve_index, s32 frame)
{
    FieldParameterCurve* entry;
    int segment_mask;
    u16* segment;
    s32 segments_remaining;
    s32 segment_start_frame;
    s32 start_delta;
    s32 value_range;
    u16 segment_word;
    int randomized_result;
    s32 interpolated_delta;
    int segment_mask_copy;
    s32 result;
    s32 sample_frame;

    segment_start_frame = 0;
    start_delta = 0;
    sample_frame = frame;
    entry = &animation_data->curves[curve_index];

    segments_remaining = entry->segment_count_flags & 0x7F;
    segment = &animation_data->segments[entry->segment_offset];

    if (segments_remaining != 0)
    {
        do
        {
            u16 current = *segment;
            s32 sum = segment_start_frame + (current & 0x3FF);

            if (sample_frame < sum)
            {
                break;
            }

            segment_start_frame = sum;
            start_delta = current >> 10;
            segment++;
        } while ((--segments_remaining) != 0);
    }

    value_range = entry->end_value - entry->start_value;
    segment_mask = 0x3FF;
    start_delta = (value_range * start_delta) >> 5;
    segment_mask_copy = segment_mask;

    if (segments_remaining == 0)
    {
        return entry->start_value;
    }

    if (((*((u16*)entry)) & 0x80) && (D_801227C8 == 0))
    {
        s32 rand_val = rand();
        segment_word = *segment;
        interpolated_delta = ((((value_range * (segment_word >> 10)) >> 5) - start_delta) * (sample_frame - segment_start_frame)) / (segment_word & segment_mask_copy);
        randomized_result = entry->start_value + (((start_delta + interpolated_delta) * rand_val) >> 15);
        result = ((start_delta + interpolated_delta) * rand_val) >> 15;
        return randomized_result;
    }
    else
    {
        segment_word = *segment;
        interpolated_delta = ((((value_range * (segment_word >> 10)) >> 5) - start_delta) * (sample_frame - segment_start_frame)) / (segment_word & segment_mask);
        result = interpolated_delta;
        return (entry->start_value + start_delta) + result;
    }
}

/**
 * @brief Reset per-track counters and part state for an actor animation.
 * @param actor Actor track state to reset.
 * decomp.me (100%) https://decomp.me/scratch/eRVUu
 */
static void field_reset_actor_track_state(FieldActorResetState* actor)
{
    s32 i;
    s32 j;

    for (j = 8; j >= 0; j--)
    {
        actor->unk1EC[j] = 0;
    }

    actor->unk236 = 0;
    actor->unk234 = 0;

    for (i = 0; i < actor->unk25; i++)
    {
        actor->parts[i].unk32 = i;
        for (j = 0; j < 9; j++)
        {
            actor->unk2B[i] = (actor->inner2[j][i] = (actor->inner1[j][i] = 0));
        }
    }
}

/**
 * @brief Advance active animation-track counters for one actor.
 * @param actor Actor track state to advance.
 * decomp.me (100%) https://decomp.me/scratch/PjqLA
 */
static void field_advance_actor_tracks(FieldActorTrackState* actor)
{
    s32 track_index;

    if (field_finalize_actor_animation((FieldActorState*)actor) == 0)
    {
        for (track_index = 0; track_index < actor->unk232; track_index++)
        {
            if (((s32)actor->unk23A >> track_index) & 1)
            {
                actor->unk1EC[track_index]++;
            }
        }

        actor->unk236++;

        if (actor->unk238 != 0 && actor->unk232 != 0)
        {
            if ((actor->unk236 % actor->unk238) == 0)
            {
                s32 next_track_index = actor->unk236 / actor->unk238;
                if (next_track_index < actor->unk232)
                {
                    actor->unk23A |= (1 << next_track_index);
                }
            }
        }
    }
}

/**
 * @brief Initialize the field actor slot table and associated object state.
 * decomp.me (100%) https://decomp.me/scratch/3KrRM
 */
void field_initialize_actor_slots(void)
{
    s32 object_index;
    s32 actor_index;
    FieldActorState* actor_slots;
    volatile u8* actor_cursor;
    object_index = 0;
    do
    {
        D_80105AE0[object_index].u.unk178 &= ~1;
        object_index++;
    } while (object_index < 0xD);
    actor_index = 0;
    actor_slots = g_field_actor_slots;
    actor_cursor = ((u8*)actor_slots) + 0x238;
    do
    {
        actor_cursor[-5] = actor_index;
        actor_index++;
        actor_cursor[-0x213] = 0;
        actor_cursor[-0x214] = 0;
        *((volatile u16*)(actor_cursor - 0x16)) = 0;
        *((volatile u16*)actor_cursor) = 0;
        actor_cursor += 0x244;
    } while (actor_index < 0x50);
}

/**
 * @brief Clear all field actor slots and release their active state.
 * decomp.me (100%) https://decomp.me/scratch/0JqYo
 */
void field_clear_actor_slots(void)
{
    s32 slot_index = 0;
    FieldActorState* actor = g_field_actor_slots;

    do
    {
        slot_index += 1;
        actor->unk24 = 0;
        actor->unk232 = 0;
        actor->unk23A = 0;
        actor->unk23B = 0;
        actor->unk27 = 0;
        actor->unk28 = 0;
        actor++;
    } while (slot_index < 0x50);
}

/**
 * @brief Start an animation on one actor slot and initialize its target list.
 * @param slot_index Actor slot to animate.
 * @param target_count Number of entries in @p targets.
 * @param targets Animation target indices.
 * decomp.me (100%) https://decomp.me/scratch/BUS6C
 */
void field_start_actor_animation(s32 slot_index, int target_count, u8* targets)
{
    s32 i;
    s32 k;
    s32 j;
    s32 m;
    FieldActorState* actor;
    u8* target_cursor;
    m = slot_index;
    actor = &g_field_actor_slots[m];
    actor->unk28 = 0;
    actor->unk27 = 0;
    if (actor->unk25 == 0)
    {
        return;
    }
    actor->unkC->unk14 &= 0x7F;
    actor->unk23A = 0;
    actor->unk23B = 1;
    actor->unk229[0] = 0;
    actor->unk232 = target_count;
    ((u8*)&actor->u224.h.lo)[1] = 0;
    if (target_count != 0)
    {
        target_cursor = targets;
        j = 0;
        i = 0;
        if (target_count > 0)
        {
            do
            {
                actor->unk229[j] = *target_cursor;
                j++;
                target_cursor += 4;
                i++;
            } while (i < target_count);
        }
        if (j != 0)
        {
            actor->unk232 = j;
        }
        else
        {
            field_finalize_actor_animation(actor);
            return;
        }
    }
    else
    {
        actor->unk232 = 1;
        actor->unk229[0] = 0xFF;
    }
    field_clear_actor_effects(actor);
    for (i = 8; i >= 0; i--)
    {
        actor->unk1EC[i] = 0;
    }

    actor->unk236 = 0;
    actor->unk234 = 0;
    for (k = 0; k < actor->unk25; k++)
    {
        actor->unk0[k].unk32 = k;
        for (m = 0; m < 9; m++)
        {
            actor->unk2B[k] = (actor->unkCC[m][k] = (actor->unk3B[m][k] = 0));
        }
    }

    field_dispatch_actor_audio_event(actor, 1, 0);
    for (i = 0; i < actor->unk25; i++)
    {
        actor->unk0[i].unk32 = i;
        for (j = 0; j < 9; j++)
        {
            actor->unk2B[i] = (actor->unkCC[j][i] = (actor->unk3B[j][i] = 0));
        }
    }
}

/**
 * @brief Dispatch an audio event encoded in an actor animation.
 * @param actor Actor animation state that produced the event.
 * @param event_type Audio event type.
 * @param event_subtype Event-specific value.
 * decomp.me (100%) https://decomp.me/scratch/XDOcQ
 */
void field_dispatch_actor_audio_event(void* actor, s32 event_type, s32 event_subtype)
{
    u8* event_flags;
    FieldActorObjectRecord* actor_records;
    s32 i;
    s32 event_offset;
    u8* event_table;
    u16 sound_command;
    s32 pan;
    u8* event_table_alias;
    FieldActorObjectRecord* actor_record;
    u8 resource_slot;

    for (i = 0; i < 2; i++)
    {
        event_flags = (u8*)actor + i;
        event_offset = i << 1;
        if (event_flags[0x27] == 0)
        {
            event_table = *((u8**)((u8*)actor + 0x0C));
            if ((*((u16*)(event_offset + (u32)event_table + 4))) == event_type)
            {
                if (event_type != 1)
                {
                    if ((*((event_table + i) + 2)) != event_subtype)
                    {
                        continue;
                    }
                }
                pan = field_get_actor_sound_pan(((u8*)actor)[0x228]);
                event_table = (event_table_alias = *((u8**)((u8*)actor + 0x0C)));
                sound_command = *((u16*)((event_table + event_offset) + 8));
                switch (sound_command >> 10)
                {
                case 0:
                    func_800A3938(sound_command & 0x3FF, pan);
                    break;

                case 1:
                    if (((u8*)actor)[0x228] < 2)
                    {
                        func_800A3A90(sound_command & 0x3FF, pan, ((u8*)actor)[0x228]);
                    }
                    else
                    {
                        actor_records = D_800FDF58;
                        actor_record = &actor_records[((u8*)actor)[0x228]];
                        resource_slot = actor_record->unk3B;
                        if (((u32)(resource_slot - 3)) < 3)
                        {
                            if (D_800FF59C != 0)
                            {
                                func_800A39A8(sound_command & 0x3FF, pan, 0, ((u8*)actor)[0x228]);
                            }
                            else
                            {
                                func_800A39A8(sound_command & 0x3FF, pan, actor_record->unk3B - 3, ((u8*)actor)[0x228]);
                            }
                        }
                    }
                    break;

                case 2:
                    if ((sound_command & 0x3FF) < 2)
                    {
                        s32* field = (s32*)(((sound_command & 0x3FF) << 2) + (u32)actor + 0x1C);
                        func_800A3E10(*field, pan, ((u8*)actor)[0x228]);
                    }
                    break;
                }

                if (event_type == 5)
                {
                    event_flags[0x27] |= 0x80;
                }
                ((u8*)actor + i)[0x27] |= 1;
            }
        }
    }
}

/**
 * @brief Check whether an actor slot has an active animation.
 * @param slot_index Actor slot to inspect.
 * @return Nonzero when the slot has an active animation, otherwise zero.
 * decomp.me (100%) https://decomp.me/scratch/tXpD1
 */
s32 field_is_actor_animation_active(s32 slot_index)
{
    FieldActorState* actor;

    actor = &g_field_actor_slots[slot_index];
    return (actor->unk23A | actor->unk23B) != 0;
}

/**
 * @brief Find an active actor using a special-attack animation.
 *
 * Scans active actor slots whose low status bit is clear for animation IDs
 * 0x1F through 0x23. These IDs form a combat-animation family with dedicated
 * hit and collision handling.
 *
 * @return The owning field-object index ORed with 0x200, or zero if none.
 * @see decomp.me (100%) https://decomp.me/scratch/8lvHC
 */
s32 field_find_active_special_attack_actor(void)
{
    int animation_id_limit;
    FieldActorState* actor;
    s32 slot_index;
    int first_animation_id;
    u16 animation_id;
    actor = g_field_actor_slots;
    for (slot_index = 0; slot_index < 80; slot_index++)
    {
        first_animation_id = 0x1F;
        if ((actor->unk24 != 0) && (!(actor->u224.unk224 & 1)))
        {
            animation_id = animation_id >> 16;
            animation_id_limit = 0x24;
            animation_id = actor->u224.h.animation_id;
            if ((animation_id < animation_id_limit) && (animation_id >= first_animation_id))
            {
                return actor->owner_object_index | 0x200;
            }
        }
        actor++;
    }

    return 0;
}

/**
 * @brief Advance all active field actor animations for the current frame.
 * decomp.me (100%) https://decomp.me/scratch/etUW8
 */
void field_update_actor_animations(void)
{
    FieldGoverLoadState* runtime_state = (FieldGoverLoadState*)0x801ED600;
    FieldActorStateWithTracks* actor;
    FieldActorTrackData* tracks;
    FieldActorAnimationDef* animation;
    s32 track_index;
    s32 actor_index;
    u8* curve_selectors;
    s32 i;
    s32 curve_index;
    u16 animation_event;
    u32 frame_counter;
    u32 track_interval;
    actor = g_field_actor_slots;
    actor_index = 0;
    runtime_state->unk140 = 0U;
    runtime_state->unk92 = 0U;
    do
    {
        tracks = &actor->unk28;
        if ((&actor->unk28)->unk1FA != 0)
        {
            if ((&actor->unk28)->unk213 != 0)
            {
                field_reset_actor_track_mask(actor);
            }
            if ((&actor->unk28)->unk212 != (actor->unkC->unk15 * 0))
            {
                func_8007100C(actor);
                i = actor->unkC->unk14;
                if (((unsigned char)i) == 2)
                {
                    if ((&actor->unk0[actor->unkC->unk15])->unk34 & 0x04000000)
                    {
                        for (track_index = 0; track_index < (&actor->unk28)->unk20A; track_index++)
                        {
                            u16 track_frame = (&actor->unk28)->unk1C4[track_index];
                            if (((&actor->unk0[actor->unkC->unk15])->unk31 < track_frame) &&
                                ((&actor->unk28)->unk1C4[track_index] < ((&actor->unk0[actor->unkC->unk15])->unk31 + (&actor->unk0[actor->unkC->unk15])->unkD)))
                            {
                                g_field_track_index = track_index;
                                func_80099A48(actor, &actor->unk0[actor->unkC->unk15]);
                            }
                        }
                    }
                    else if ((&actor->unk0[actor->unkC->unk15])->unk31 == 0xFF)
                    {
                        for (track_index = 0; track_index < (&actor->unk28)->unk20A; track_index++)
                        {
                            if ((&actor->unk0[actor->unkC->unk15])->unkD > (&actor->unk28)->unk1C4[track_index])
                            {
                                g_field_track_index = track_index;
                                func_80099A48(actor, &actor->unk0[actor->unkC->unk15]);
                            }
                        }
                    }
                    else if (((&actor->unk0[actor->unkC->unk15])->unk31 < (&actor->unk28)->unk1C4[0]) &&
                             ((&actor->unk28)->unk1C4[0] < ((&actor->unk0[actor->unkC->unk15])->unk31 + (&actor->unk0[actor->unkC->unk15])->unkD)))
                    {
                        g_field_track_index = 0;
                        func_80099A48(actor, &actor->unk0[actor->unkC->unk15]);
                    }
                }
                animation = actor->unkC;
                animation_event = animation->unkE;
                if ((animation_event & 0x8000) && (((u8*)animation)[0xE] == (&actor->unk28)->unk1C4[0]))
                {
                    func_8005A67C((animation_event >> 8) & 0x7F, 0);
                }
                field_update_actor_effects(actor);
                if (field_finalize_actor_animation(actor) == 0)
                {
                    for (i = 0; i < (&actor->unk28)->unk20A; i++)
                    {
                        if (((&actor->unk28)->unk212 >> i) & 1)
                        {
                            (&actor->unk28)->unk1C4[i] += 1;
                        }
                    }

                    (&actor->unk28)->unk20E += 1;
                    if (((&actor->unk28)->unk210 != 0) && ((&actor->unk28)->unk20A != 0))
                    {
                        frame_counter = tracks->unk20E;
                        track_interval = tracks->unk210;
                        if ((frame_counter % track_interval) == 0)
                        {
                            i = (frame_counter / track_interval) & 0xFFFF;
                            if (i < (&actor->unk28)->unk20A)
                            {
                                (&actor->unk28)->unk212 |= 1 << i;
                            }
                        }
                    }
                }
                g_field_track_index = 0;
                for (track_index = 0; track_index < 2; track_index++)
                {
                    u8 curve_selector = (curve_selectors = actor->unkC->unk0)[track_index];
                    if ((curve_selectors[track_index] != 0xFF) && (curve_selector < 0x10U))
                    {
                        if (track_index != 0)
                        {
                            u8 combined_value = field_evaluate_parameter_track((FieldAnimationData*)actor, curve_selector & 0xF) | runtime_state->unk92;
                            runtime_state->unk92 = combined_value;
                            runtime_state->unk140 = combined_value;
                        }
                        else
                        {
                            u8 base_value = field_evaluate_parameter_track((FieldAnimationData*)actor, (curve_index = curve_selectors[0]) & 0xF);
                            runtime_state->unk91 = (runtime_state->unk13F = base_value);
                        }
                    }
                }
            }
            actor->unk27 &= 0xFE;
            (&actor->unk28)->unk0 &= 0xFE;
        }
        actor_index += 1;
        actor += 1;
    } while (actor_index < 0x50);
}

/**
 * @brief Rebuild the active animation-track mask for an actor.
 * @param actor Actor track-mask state to reset.
 * decomp.me (100%) https://decomp.me/scratch/VZWgF
 */
void field_reset_actor_track_mask(FieldActorTrackMaskState* actor)
{
    s32 i;
    u16 check238 = actor->unk238;

    /* Preserve the 16-bit value before rebuilding the active-track mask. */
    check238++;
    check238--;

    actor->unk23A = 1;
    if (check238 == 0)
    {
        actor->unk23A = 0;
        for (i = 0; i < actor->unk232; i++)
        {
            actor->unk23A |= 1 << i;
        }
    }
    actor->unk23B = 0;
}

/**
 * @brief Prepare actor state and emit actor render commands.
 * @param render_context Field render context.
 * @param unused Unused legacy argument.
 * decomp.me (100%) https://decomp.me/scratch/hvTSS
 */
void field_prepare_actor_render_commands(s32 render_context, s32 unused)
{
    func_80074D7C();
    field_build_actor_render_commands(render_context, unused);
}

/**
 * @brief Build GPU command packets for all visible field actors.
 * @param render_context Field render context and packet cursor.
 * decomp.me (100%) https://decomp.me/scratch/Sgd61
 */
void field_build_actor_render_commands(void* render_context)
{
    u32 packed_color;
    FieldRenderContext* render_ctx = (FieldRenderContext*)render_context;
    s32 target_index_copy;
    u32* ordering_table = &render_ctx->unk40;
    u32* packet = (u32*)render_ctx->unk40B8;
    FieldActorState* actor = g_field_actor_slots;
    s32 actor_index = 0;
    u32 color_word;
    int blend_mode;
    int state_mask;
    int target_animation_enabled;
    do
    {
        state_mask = actor->unk23A != 0;
        if (state_mask)
        {
            u16 animation_flags = actor->unkC->unk18;
            if ((animation_flags & 2) && (!(actor->unkC->unk18 & 8)))
            {
                u8 object_state_value;
                s32 cond;
                g_field_track_index = 0;
                cond = field_evaluate_parameter_track(actor, (actor->unkC->unk18 >> 8) & 0xF);
                object_state_value = 0;
                if (cond != 0)
                {
                    object_state_value = 0xFE;
                }
                if (object_state_value != 0)
                {
                    D_800FDF58[actor->owner_object_index].unk25 = object_state_value;
                    D_80105AE0[actor->owner_object_index].u.unk178 |= 1;
                    D_80105AE0[actor->owner_object_index].u.b.unk17A = actor->unk233;
                }
                else
                {
                    u8 owner_index = actor->owner_object_index;
                    s16 owner_state = D_800FDF58[owner_index].unk2A;
                    if ((owner_state == 0x90) || (owner_state == 0x94))
                    {
                        if (D_80105AE0[owner_index].unkC & 0x200)
                        {
                            D_800FDF58[actor->owner_object_index].unk25 = object_state_value;
                        }
                    }
                    else
                    {
                        D_800FDF58[actor->owner_object_index].unk25 = object_state_value;
                    }
                }
            }
            target_animation_enabled = actor->unkC->unk18 & 4;
            if (target_animation_enabled && (!(actor->unkC->unk18 & 0x10)))
            {
                s32 target_index = 0;
                if (actor->unk232 != 0)
                {
                    do
                    {
                        u8 target_state_value;
                        s32 cond2;
                        g_field_track_index = target_index;
                        cond2 = field_evaluate_parameter_track(actor, actor->unkC->unk18 >> 0xC);
                        target_state_value = 0;
                        if (cond2 != 0)
                        {
                            target_state_value = 0xFE;
                        }
                        if (actor->unk229[target_index] != 0xFF)
                        {
                            if (target_state_value != 0)
                            {
                                D_800FDF58[actor->unk229[target_index]].unk25 = target_state_value;
                                do
                                {
                                    D_80105AE0[actor->unk229[target_index]].u.unk178 |= 1;
                                    D_80105AE0[actor->unk229[target_index]].u.b.unk17A = actor->unk233;
                                } while (0);
                                ((u8*)actor)[0x225] = 1;
                            }
                            else
                            {
                                target_index_copy = target_index;
                                D_800FDF58[actor->unk229[target_index_copy]].unk25 = 0;
                            }
                        }
                        target_index++;
                    } while (target_index < actor->unk232);
                }
            }
        }
        actor_index++;
        actor++;
    } while (actor_index < 0x50);
    g_field_track_index = 0;
    actor = g_field_actor_slots;
    actor_index = 0;
    do
    {
        if (actor->unk23A != 0)
        {
            FieldActorAnimationDef* animation = actor->unkC;
            if (animation->unkC & 0x1000)
            {
                s32 color_target = animation->unkC >> 0xD;
                switch (color_target & 3)
                {
                case 0:
                    D_800F2278 = field_evaluate_parameter_track(actor, animation->unk10 >> 0xC);
                    break;

                case 1:
                    D_800F227C = field_evaluate_parameter_track(actor, animation->unk10 >> 0xC);
                    break;

                case 2:
                    D_800F227C = (D_800F2278 = field_evaluate_parameter_track(actor, animation->unk10 >> 0xC));
                    break;

                case 3:
                    D_800F2278 = field_evaluate_parameter_track(actor, animation->unk10 >> 0xC);
                    D_800F227C = field_evaluate_parameter_track(actor, ((actor->unkC->unk10 >> 0xC) + 1) & 0xF);
                    break;
                }
            }
        }
        actor_index++;
        actor++;
    } while (actor_index < 0x50);
    actor = g_field_actor_slots;
    actor_index = 0;
    do
    {
        state_mask = 0x00FFFFFF;
        if ((actor->unk23A != 0) && (actor->unk24 != 0))
        {
            FieldActorAnimationDef* color_animation = actor->unkC;
            u8 curve_selector = (u8)color_animation->unkC;
            if (((u8)color_animation->unkC) < 0x10)
            {
                if (((color_animation->unkC >> 8) & 1) != 0)
                {
                    do
                    {
                        ((u8*)(&color_word))[0] = field_evaluate_parameter_track(actor, (u8)color_animation->unkC);
                    } while (0);
                    ((u8*)(&color_word))[1] = field_evaluate_parameter_track(actor, (((u8)actor->unkC->unkC) + 1) & 0xF);
                    ((u8*)(&color_word))[2] = field_evaluate_parameter_track(actor, (((u8)actor->unkC->unkC) + 2) & 0xF);
                }
                else
                {
                    ((u8*)(&color_word))[0] = (((u8*)(&color_word))[1] = (((u8*)(&color_word))[2] = field_evaluate_parameter_track(actor, curve_selector & 0xF)));
                }
                if ((actor->unkC->unkC >> 8) & 4)
                {
                    field_set_global_color_scale(((u8*)(&color_word))[0] * 2, ((u8*)(&color_word))[1] * 2, ((u8*)(&color_word))[2] * 2);
                }
                else
                {
                    u32 packet_word;
                    if ((actor->unkC->unkC >> 8) & 1)
                    {
                        if (((u8*)(&color_word))[0] == 0)
                        {
                            if (((u8*)(&color_word))[1] == 0)
                            {
                                if (((u8*)(&color_word))[2] == 0)
                                {
                                    goto block_59;
                                }
                            }
                        }
                    }
                    else if (((u8*)(&color_word))[0] == 0)
                    {
                        goto block_59;
                    }
                    packet_word = 0xE1000005;
                    {
                        u8* primitive = ((u8*)packet) + 4;
                        packed_color = color_word;
                        primitive[-1] = 3;
                        *((u16*)(primitive + 8)) = 0x140;
                        *((u16*)(primitive + 0xA)) = 0xF0;
                        *((u16*)(primitive + 6)) = 0;
                        *((u16*)(primitive + 4)) = 0;
                        *((u32*)primitive) = packed_color;
                        primitive[3] = 0x62;
                        packet[0] = (packet[0 ^ 0] & 0xFF000000) | ((*ordering_table) & state_mask);
                        *ordering_table = ((*ordering_table) & 0xFF000000) | (((u32)packet) & 0x00FFFFFF);
                        blend_mode = (((actor->unkC->unkC >> 9) & 3) + 1) & 3;
                        packet += 4;
                        {
                            u8* draw_mode_primitive = ((u8*)packet) + 4;
                            draw_mode_primitive[-1] = 1;
                            *((u32*)draw_mode_primitive) = (blend_mode << 5) | packet_word;
                        }
                        packet_word = 0x00FFFFFF;
                        packet[0] = (packet[0] & 0xFF000000) | ((*ordering_table) & packet_word);
                        *ordering_table = ((*ordering_table) & 0xFF000000) | (((u32)packet) & 0x00FFFFFF);
                        packet += 2;
                    }
                }
            }
        }
    block_59:
        actor_index++;

        actor++;
    } while (actor_index < 0x50);
    field_apply_global_color_scale();
    render_ctx->unk40B8 = packet;
}

/**
 * @brief Reset the global actor color scale to neutral.
 * decomp.me (100%) https://decomp.me/scratch/kl6PF
 */
void field_reset_global_color_scale(void)
{
    g_field_color_scale.s.red = 0x100;
    g_field_color_scale.s.green = 0x100;
    g_field_color_scale.s.blue = 0x100;
    g_field_color_scale_active = 0;
}

/**
 * @brief Set the global actor color scale.
 * @param red Red scale component.
 * @param green Green scale component.
 * @param blue Blue scale component.
 * decomp.me (100%) https://decomp.me/scratch/iwb2J
 */
void field_set_global_color_scale(s16 red, s16 green, s16 blue)
{
    g_field_color_scale.s.red = red;
    g_field_color_scale.s.green = green;
    g_field_color_scale.s.blue = blue;
}

/**
 * @brief Apply the global actor color scale when its state changes.
 * decomp.me (100%) https://decomp.me/scratch/eDBPu
 */
void field_apply_global_color_scale(void)
{
    u8* active = &g_field_color_scale_active;

    if (*active != 0)
    {
        func_8005A0D0(-1, g_field_color_scale.s.red, g_field_color_scale.s.green, g_field_color_scale.s.blue);
        if (g_field_color_scale.w == 0x1000100UL && g_field_color_scale.s.blue == 0x100)
        {
            *active = 0;
        }
    }
    else
    {
        FieldColorScale* p = &g_field_color_scale;
        if (p->w != 0x1000100UL || p->s.blue != 0x100)
        {
            func_8005A0D0(-1, p->s.red, p->s.green, p->s.blue);
            *active = 1;
        }
    }
}

/**
 * @brief Invalidate the three actor resource slots and their cached portraits.
 * decomp.me (100%) https://decomp.me/scratch/a1YEc
 */
void field_reset_actor_resource_slots(void)
{
    D_800FD818[0].unk254 = 0;
    D_800FD818[1].unk254 = 0;
    D_800FD818[2].unk254 = 0;

    D_800FD818[0].unk256 = 0xFF;
    D_800FD818[1].unk256 = 0xFF;
    D_800FD818[2].unk256 = 0xFF;

    D_800FD818[0].u0.h = (u16)(D_800FD818[0].u0.h & 0xFFFD);
    D_800FD818[1].u0.h = (u16)(D_800FD818[1].u0.h & 0xFFFE);
    D_800FD818[2].u0.h = (u16)(D_800FD818[2].u0.h & 0xFFFE);
}


/* Legacy call sites use both zero-argument and four-argument forms of field_refresh_actor_portraits. */

/**
 * @brief Reset and re-initialize the field actor/voice state tables.
 * @see decomp.me (100%) https://decomp.me/scratch/0wUsT
 */
void field_initialize_actor_system(void)
{
    s32 i;
    s32 j;
    unsigned int work_value;
    int slot_two_index;
    u8* actor_base;
    FieldActorAnimationDef* default_animation;
    int zero_arg;
    FieldActorPartDef* part;
    u8* slot_column_base;
    FieldActorState* actor;
    u8* table_cursor;
    u8* row_cursor;
    int input_flags;
    u8* scratch_base;
    s32 column_offset;
    u8* byte_cursor;
    u8* pad_base;
    int input_flags_alt;
    u8* pad_ptr;
    u8* actor_base_alias;
    u32 dest_addr;

    D_801227C8 = 0;
    D_8012291C = 0;

    for (i = 0; i < 0xD; i++)
    {
        D_800FDF58[i].unk25 = 0xFF;
        D_800FDF58[i].unk0 = 0xFFFB0000;
        D_800FDF58[i].unk4 = 0;
        D_800FDF58[i].unk8 = 0;
    }

    bcopy(g_field_resource_buffer, (void*)0x80180000, 0x10000);
    D_800FE774 = 0;
    g_field_resource_cursor = g_field_resource_buffer;

    for (i = 0; i < 9; i++)
    {
        g_field_resource_entries[i].flags &= ~2;
    }

    if (D_801158A0 != 0)
    {
        for (j = 0; j < 3; j++)
        {
            if (D_800FD818[j].u0.b.unk0 & 1)
            {
                D_800FE774++;
                if (j == 2)
                {
                    work_value = field_get_actor_resource_id(2, &D_800FD818[2], 1);
                    i = work_value;
                }
                else
                {
                    i = field_get_actor_resource_id(j, &D_800FD818[j], 0);
                }
                if (i != D_800FD818[j].unk254)
                {
                    D_800FD818[j].unk254 = i;
                    if (j == 2)
                    {
                        field_load_actor_resource_slot(2, 2, i, 1);
                    }
                    else
                    {
                        field_load_actor_resource_slot(j, j, i, 0);
                    }
                }
                else
                {
                    field_relocate_resource_buffer(j);
                    slot_two_index = 2;
                    if (j == slot_two_index)
                    {
                        g_field_resource_entries[slot_two_index].flags = g_field_resource_entries[slot_two_index].flags | 1;
                    }
                }
                field_initialize_actor_record(j, j);

                pad_base = (u8*)g_pad_ctx;
                input_flags = D_800FDF58[j].unk1C & (~0x1FF);
                pad_ptr = pad_base + (j * 0x250);
                D_800FDF58[j].unk1C = input_flags | ((pad_ptr[0x608] >> 7) ^ 1);

                if (j < 2)
                {
                    func_800A3D44(j, D_800FD818[j].u0.b.unk1);
                }
            }
        }
    }
    else
    {
        for (j = 0; (j < 3) & 0xFFFFFFFFu; j++)
        {
            if ((D_800FD818[j].u0.b.unk0 & 1) != 0)
            {
                D_800FE774++;
                i = field_get_actor_resource_id(j, &D_800FD818[j], 0);
                if (i != D_800FD818[j].unk254)
                {
                    D_800FD818[j].unk254 = i;
                    field_load_actor_resource_slot(j, j, i, 0);
                }
                else
                {
                    field_relocate_resource_buffer(j);
                }
                field_initialize_actor_record(j, j);

                pad_base = (u8*)g_pad_ctx;
                input_flags_alt = D_800FDF58[j].unk1C & (~0x1FF);
                pad_ptr = pad_base + (j * 0x250);
                D_800FDF58[j].unk1C = input_flags_alt | ((pad_ptr[0x608] >> 7) ^ 1);
            }
        }
    }

    func_80091438(0);
    if (D_800FDA80 & 1)
    {
        func_80091438(1);
    }

    D_800FE758.unk14 = 0;
    i = 0;
    actor_base = (u8*)g_field_actor_slots;
    default_animation = &D_800FE758;
    part = D_800FE3A0;
    j = 0x6CC0;

    for (; i < 0xD; i++)
    {
        actor = (FieldActorState*)(((u32)j) + ((u32)actor_base));
        actor->unk0 = part;
        actor->unkC = default_animation;
        field_initialize_actor_part(i, 0);
        part++;
        j += 0x244;
    }

    field_restore_default_action_animation_mappings(zero_arg = 0);

    for (i = 0; i < 0x10; i++)
    {
        j = 0;
        slot_column_base = ((u8*)g_field_actor_slots) + i;
        byte_cursor = slot_column_base + 0xB327;
        column_offset = i * 2;
        work_value = 0xB3C8;

        {
            u8* base = (u8*)g_field_actor_slots;
            actor_base_alias = base;
            scratch_base = actor_base_alias;
            table_cursor = scratch_base;
            row_cursor = table_cursor;
        }

        for (; j < 9; j++)
        {
            scratch_base = row_cursor + 0xB337;
            dest_addr = column_offset;
            dest_addr += (u32)(table_cursor + work_value);
            *((u16*)dest_addr) = (*(i + scratch_base) = 0);
            row_cursor += 0x10;
            table_cursor += 0x20;
            *byte_cursor = 0;
        }
    }

    field_refresh_actor_portraits(table_cursor, row_cursor, column_offset, byte_cursor);
}

/**
 * @brief Relocate a field resource payload into the streaming buffer.
 * @param resource_index Resource entry index to relocate.
 */
void field_relocate_resource_buffer(s32 resource_index)
{
    u32 resource_size;
    FieldActorObjectRecord* actor_record;
    u8* old_start;
    void** cursor_ref;
    u8* source_base;
    u8* buffer_base;

    buffer_base = g_field_resource_buffer;
    buffer_base++;
    buffer_base--;
    source_base = (u8*)0x80180000;
    cursor_ref = &g_field_resource_cursor;
    *(u32*)&g_field_resource_entries[resource_index].start += 0;
    bcopy((void*)(source_base - (u32)buffer_base + (u32)g_field_resource_entries[resource_index].start), *cursor_ref,
          g_field_resource_entries[resource_index].end - g_field_resource_entries[resource_index].start);
    resource_size = g_field_resource_entries[resource_index].end;
    old_start = g_field_resource_entries[resource_index].start;
    resource_size -= (u32)old_start;
    g_field_resource_entries[resource_index].start = (u8*)g_field_resource_cursor;
    g_field_resource_entries[resource_index].end = ((u8*)g_field_resource_cursor) + resource_size;
    g_field_resource_entries[resource_index].flags &= ~1;
    g_field_resource_entries[resource_index].slot_index = resource_index;
    g_field_resource_entries[resource_index].flags |= 2;
    g_field_resource_cursor = g_field_resource_entries[resource_index].end;
    actor_record = &D_800FDF58[resource_index];
    field_restart_actor_animation(actor_record, old_start);
}

/**
 * @brief Restore the default animation IDs for action mappings 1 and 5.
 *
 * The two 0x22-byte maps are selected by the input/action resolver. Each map
 * contains eleven animation IDs followed by per-action disable bytes. This
 * restores mappings 1 and 5 in both maps and makes those mappings available.
 *
 * @see decomp.me (100%) https://decomp.me/scratch/oaoFZ
 */
void field_restore_default_action_animation_mappings(void)
{
    g_field_action_animation_maps.map0_action1_animation_id = 0x185;
    g_field_action_animation_maps.map1_action1_animation_id = 0x185;
    g_field_action_animation_maps.map0_action1_disabled = 0;
    g_field_action_animation_maps.map1_action1_disabled = 0;
    g_field_action_animation_maps.map0_action5_disabled = 0;
    g_field_action_animation_maps.map0_action5_animation_id = 0x585;
    g_field_action_animation_maps.map1_action5_disabled = 0;
    g_field_action_animation_maps.map1_action5_animation_id = 0x585;
}

/**
 * @brief Resolve the CD resource id selected by an actor resource slot.
 * @param unused_slot_index Actor slot index retained by the original interface.
 * @param entry Actor resource slot to inspect.
 * @param alternate_set Selects the alternate resource-id table when nonzero.
 * @return CD resource id for the selected actor resource.
 * @see decomp.me (100%) TODO
 */
s32 field_get_actor_resource_id(s32 unused_slot_index, FieldActorResourceSlot* entry, s32 alternate_set)
{
    if (alternate_set != 0)
    {
        switch (entry->unk3)
        {
        case 0:
            if (entry->u0.h & 2)
            {
                return entry->u0.b.unk1 + 0xA17;
            }
            return entry->u0.b.unk1 + 0xA0C;

        case 1:
            return entry->unk2 + 0xA23;

        case 2:
        default:
            return entry->unk2 + 0xA4B;
        }
    }
    else
    {
        switch (entry->unk3)
        {
        case 0:
            return ((entry->u0.h >> 1) & 1) + 0xAEB;

        case 1:
            return entry->unk2 + 0xAEE;

        case 2:
        default:
            return entry->unk2 + 0xB02;
        }
    }
}

/**
 * @brief Reset all actor-part runtime records.
 * @param timer_mode Selects the default timer values used for each part.
 * @see decomp.me (100%) TODO
 */
void field_initialize_actor_parts(s32 timer_mode)
{
    s32 i;

    for (i = 0; i < 0xD; i++)
    {
        field_initialize_actor_part(i, timer_mode);
    }
}

/**
 * @brief Load one actor resource package into the field resource arena.
 * @param resource_index Resource-entry index to initialize.
 * @param slot_index Actor slot associated with the resource.
 * @param resource_id CD resource id to load.
 * @param alternate_layout Low bit selects the alternate texture layout.
 * @see decomp.me (100%) TODO
 */
void field_load_actor_resource_slot(s32 resource_index, s32 slot_index, s32 resource_id, s32 alternate_layout)
{
    g_field_resource_entries[resource_index].unkE = 0x2F;
    g_field_resource_entries[resource_index].slot_index = slot_index;
    g_field_resource_entries[resource_index].unk8 = 0;
    g_field_resource_entries[resource_index].flags =
        (g_field_resource_entries[resource_index].flags & ~1) | (alternate_layout & 1);
    g_field_resource_entries[resource_index].start = g_field_resource_cursor;
    field_load_resource_package(resource_id, slot_index, resource_index, alternate_layout & 1);
    D_800FDF58[slot_index].unk21 &= 0x80;
    field_restart_actor_animation(&D_800FDF58[slot_index]);
    g_field_resource_entries[resource_index].end = g_field_resource_cursor;
    g_field_resource_entries[resource_index].flags |= 2;
}

/**
 * @brief Refresh controller-derived actor flags after the selected slot changes.
 * @param actor_slot Actor slot selector; values two and above are ignored.
 * @see decomp.me (100%) TODO
 */
void func_8006AA7C(s32 actor_slot)
{
    s32 i;
    u8* pad_base;
    u8* pad_ptr;
    u32 state_flags;
    s32 work_value;

    if (actor_slot < 2)
    {
        func_800B08FC(0, actor_slot);
        if (func_800B0850() == 0)
        {
            pad_base = (u8*)g_pad_ctx;

            for (i = 0; i < 2; i++)
            {
                if (D_800FD818[i].u0.b.unk0 & 1)
                {
                    work_value = ~0x1FF;
                    state_flags = D_800FDF58[i].unk1C & work_value;
                    work_value = i * 0x250;
                    pad_ptr = pad_base + work_value;
                    D_800FDF58[i].unk1C = state_flags | ((pad_ptr[0x608] >> 7) ^ 1);
                }
            }

            func_8008C7A8();
            func_800B4684();
            func_80084240();
            func_800B01FC();
        }
    }
}

/**
 * @brief Release an auxiliary actor slot and compact its resource data.
 * @param slot_index_minus_one Auxiliary slot number minus one.
 * @see decomp.me (100%)
 */

void field_release_actor_resource_slot(s32 slot_index_minus_one)
{
    s32 slot_index;
    s32 i;
    u8* src;
    u8* dst;
    s32 size;
    FieldActorObjectRecord* record;
    FieldActorObjectRecord* records;
    FieldActorObjectRecord* actor_record;
    FieldResourceEntry* entry;

    slot_index = slot_index_minus_one;
    slot_index += 1;

    if (D_800FD818[slot_index].u0.b.unk0 & 1)
    {
        entry = &g_field_resource_entries[slot_index];

        while (D_800FD818[slot_index].u0.b.unk0 & 1)
        {
            records = D_800FDF58;
            actor_record = &records[slot_index];
            break;
        }

        src = entry->end;
        size = src - entry->start;
        dst = entry->start;
        actor_record->unk25 = 0xFF;

        D_800FD818[slot_index].unk256 = 0xFF;

        D_800FD818[slot_index].u0.h &= 0xFFFE;

        while (src != g_field_resource_cursor)
        {
            *dst = *src;
            src++;
            dst++;
        }

        for (i = 0; i < 8; i++)
        {
            if (g_field_resource_entries[i].start > g_field_resource_entries[slot_index].start)
            {
                g_field_resource_entries[i].start -= size;
                g_field_resource_entries[i].end -= size;
            }
        }

        i = 0;
        record = D_800FDF58;
        do
        {
            if ((record->unk25 != 0xFF) && (record->unk3B != 8))
            {
                if ((record->unk40 | 0x80000000) >
                    (((u32)g_field_resource_entries[slot_index].start) | 0x80000000))
                {
                    record->unk40 -= size;
                }
            }

            i++;
            record++;
        } while (i < 0xD);

        g_field_resource_cursor = ((u8*)g_field_resource_cursor) - size;
        g_field_resource_entries[slot_index].flags &= ~2;
    }
}


/**
 * @brief Load and activate an auxiliary actor slot.
 * @param source_selector Existing actor selector, -1 for a cleared position, or -2 for the primary actor template.
 * @param resource_variant Resource variant stored in the slot descriptor.
 * @param slot_index_minus_one Auxiliary slot number minus one.
 * @return 1 on activation, 0 when already active, or -1 when the source actor cannot be found.
 */
s32 field_activate_actor_resource_slot(s32 source_selector, s32 resource_variant, s32 slot_index_minus_one)
{
    s32 slot = slot_index_minus_one + 1;
    FieldActorObjectRecord* source_record;
    FieldActorResourceSlot* resource_slot;
    u8* pad_context;
    s32 resource_id;
    s32 flags;
    u8* slot_input;

    if (D_800FD818[slot].u0.b.unk0 & 1)
    {
        return 0;
    }

    if (source_selector >= 0)
    {
        source_record = func_80087C9C(source_selector);
        if (source_record == (FieldActorObjectRecord*)-1)
        {
            return -1;
        }
    }

    D_800FD818[slot].u0.h |= 1;
    if (source_selector == -2)
    {
        D_800FD818[slot].unk3 = 0;
    }
    else
    {
        D_800FD818[slot].unk3 = slot;
    }

    D_800FD818[slot].unk2 = resource_variant;

    resource_slot = &D_800FD818[slot];
    switch (resource_slot->unk3)
    {
    case 0:
        resource_id = ((resource_slot->u0.h >> 1) & 1) + 0xAEB;
        break;

    case 1:
        resource_id = resource_slot->unk2 + 0xAEE;
        break;

    case 2:
    default:
        resource_id = resource_slot->unk2 + 0xB02;
        break;
    }

    D_800FD818[slot].unk254 = resource_id;
    field_load_actor_resource_slot(slot, slot, resource_id, 0);
    field_initialize_actor_record(slot, slot);

    flags = D_800FDF58[slot].unk1C;
    pad_context = g_pad_ctx;
    slot_input = pad_context + slot * 0x250;
    D_800FDF58[slot].unk1C = (flags & ~0x1FF) | ((slot_input[0x608] >> 7) ^ 1);

    if ((slot == 2) && (resource_variant >= 0x41))
    {
        FieldActorObjectRecord* slot_two = &D_800FDF58[2];
        slot_two->unk1C = (slot_two->unk1C & 0xFFFCFFFF) | (((pad_context[0x29D7] + 1) & 3) << 16);
    }
    else
    {
        D_800FDF58[slot].unk1C &= 0xFFFCFFFF;
    }

    if (source_selector == -1)
    {
        D_800FDF58[slot].unk0 = 0;
        D_800FDF58[slot].unk4 = 0;
        D_800FDF58[slot].unk8 = 0;
        D_800FDF58[slot].unk21 = 0;
    }
    else if (source_selector == -2)
    {
        FieldActorObjectRecord* entry;
        FieldActorObjectRecord* template;
        Struct_D800EB254* definition;
        Struct_D800EB254* definitions;
        s32 args[3];

        template = D_800FDF58;
        entry = &D_800FDF58[slot];
        entry->unk0 = template->unk0;
        entry->unk4 = template->unk4;
        entry->unk8 = template->unk8;
        definitions = D_800EB254;
        definition = &definitions[template->unk1B >> 5];
        args[0] = definition->unk0;
        args[1] = 0;
        args[2] = definition->unk0;
        func_8009C2E0(entry, args);
        entry->unk21 = 0;
    }
    else
    {
        D_800FDF58[slot].unk0 = source_record->unk0;
        D_800FDF58[slot].unk4 = source_record->unk4;
        D_800FDF58[slot].unk8 = source_record->unk8;
        D_800FDF58[slot].unk21 = 0;
        source_record->unk25 = 0xFF;
        D_800FE774--;
    }

    D_800FDF58[slot].unk25 = 0;
    D_800FDF58[slot].unk2A = 0;
    D_800FDF58[slot].unk10 = 0;
    D_800FDF58[slot].unk28 = 0xFF;
    field_restart_actor_animation(&D_800FDF58[slot]);
    func_800AA90C(0);
    field_refresh_actor_portraits();

    if ((D_801158A0 != 0) && (slot == 1))
    {
        func_800A3D44(1, D_800FDA81);
    }

    func_8008C7A8();
    func_8009C434();

    switch (D_800FD818[slot].unk3)
    {
    case 1:
        func_800A5174(1, D_800FD818[slot].unk2 + 0xA37);
        func_80091438(1);
        break;

    case 2:
        func_800A5174(2, D_800FD818[slot].unk2 + 0xA9B);
        break;
    }

    return 1;
}

/**
 * @brief Reuse the matching resource entry, or select the first free entry, then load it.
 * @param resource_slot_id Resource slot identifier to find.
 * @param resource_base Resource base forwarded to the entry loader.
 * @see decomp.me (100%) TODO
 */
void field_find_or_load_resource_entry(s32 resource_slot_id, s32 resource_base)
{
    s32 i;

    for (i = 0; i < 9; i++)
    {
        if (((g_field_resource_entries[i].flags >> 1) & 1) && g_field_resource_entries[i].slot_index == resource_slot_id)
        {
            break;
        }
    }

    if (i == 9)
    {
        for (i = 0; i < 9; i++)
        {
            if (!((g_field_resource_entries[i].flags >> 1) & 1))
            {
                break;
            }
        }
    }

    field_load_resource_entry(resource_slot_id, resource_base, i);
}


/**
 * @brief Replace a resource entry and restart actors that reference it.
 * @param resource_slot_id Resource slot identifier stored in the entry.
 * @param resource_base Base used to derive the package resource id.
 * @param entry_index Resource entry to replace.
 * @see decomp.me (100%) TODO
 */
void field_load_resource_entry(s32 resource_slot_id, u8 *resource_base, s32 entry_index)
{
    s32 i;
    FieldResourceEntry *entry;
    FieldResourceEntry *base;

    field_release_resource_entry(entry_index);

    base = g_field_resource_entries;
    entry = &base[entry_index];
    entry->slot_index = resource_slot_id;
    entry->unk8 = 0;
    func_8009C434();
    entry->unkE = 0;
    entry->flags &= ~1;
    entry->start = g_field_resource_cursor;
    field_load_resource_package(resource_base + 0xB52, resource_slot_id, entry_index);
    entry->end = g_field_resource_cursor;
    entry->flags |= 2;

    {
        FieldActorObjectRecord *rec;
        u8 *record_data;

        rec = D_800FDF58;
        record_data = (u8*)D_800FDF58 + 0x3B;
        i = 0;
        while (i < 0xD)
        {
            if (record_data[-0x16] != 0xFF && record_data[0] == entry_index)
            {
                field_restart_actor_animation(rec);
            }
            i++;
            record_data += 0x54;
            rec++;
        }
    }
}

/**
 * @brief Release a resource entry and compact the shared resource arena.
 * @param entry_index Resource entry to release.
 * @see decomp.me (100%) TODO
 */
void field_release_resource_entry(s32 entry_index)
{
    s32 i;
    u8 *src;
    u8 *dst;
    s32 size;
    FieldActorObjectRecord *record;

    if ((g_field_resource_entries[entry_index].flags >> 1) & 1)
    {
        src = g_field_resource_entries[entry_index].end;
        size = src - g_field_resource_entries[entry_index].start;
        dst = g_field_resource_entries[entry_index].start;

        while (src != g_field_resource_cursor)
        {
            *dst = *src;
            src++;
            dst++;
        }

        for (i = 0; i < 8; i++)
        {
            if (((g_field_resource_entries[i].flags >> 1) & 1) && g_field_resource_entries[i].start > g_field_resource_entries[entry_index].start)
            {
                g_field_resource_entries[i].start -= size;
                g_field_resource_entries[i].end -= size;
            }
        }

        record = D_800FDF58;
        for (i = 0; i < 0xD; i++)
        {
            if ((record->unk25 != 0xFF) && (record->unk3B != 8))
            {
                if ((record->unk40 | 0x80000000) > (((u32)g_field_resource_entries[entry_index].start) | 0x80000000))
                {
                    record->unk40 -= size;
                }
            }
            record++;
        }

        g_field_resource_cursor = ((u8*)g_field_resource_cursor) - size;
    }
}

/**
 * @brief Reset an actor object record and bind it to a resource entry.
 * @param actor_index Actor object record to initialize.
 * @param resource_entry_index Resource entry used by the actor.
 * @see decomp.me (100%) TODO
 */
void field_initialize_actor_record(s32 actor_index, s32 resource_entry_index)
{
    u32 i;
    u8* bytes;
    s32 flags;
    s32 initial_flags;
    s32 mask_a;
    s32 mask_b;
    s32 mask_c;
    s16* q16;
    u32* q32;

    bytes = (u8*)&D_800FDF58[actor_index];
    i = 0;
    do
    {
        *bytes = 0;
        i++;
        bytes++;
    } while (i < 0x54);

    mask_a = 0xFFFF7FFF;
    mask_b = 0xEFFFFFFF;
    mask_c = 0xFFFBFFFF;

    D_80105AE0[actor_index].unk19C = -1;
    D_80105AE0[actor_index].unk1A0 = 0;
    D_80105AE0[actor_index].unk18E = 0;
    D_80105AE0[actor_index].u.unk178 &= ~0x80;
    D_80105AE0[actor_index].u.unk178 &= ~0x40;

    D_800FDF58[actor_index].unk22 = (s8)(actor_index + 0x30);
    D_800FDF58[actor_index].unk28 = 0xFF;

    D_800FDF58[actor_index].unk3A = actor_index;
    D_800FDF58[actor_index].unk24 = 0;
    D_800FDF58[actor_index].unk25 = 0;
    D_800FDF58[actor_index].unk27 = 0;
    D_800FDF58[actor_index].unk2A = 0;
    D_800FDF58[actor_index].unk2C = 0;
    D_800FDF58[actor_index].unk2E = 0;
    D_800FDF58[actor_index].unk30 = 0;
    D_800FDF58[actor_index].unk32 = 0;
    D_800FDF58[actor_index].unk33 = 0;
    D_800FDF58[actor_index].unk4 = 0;
    D_800FDF58[actor_index].unk8 = 0;
    initial_flags = (D_800FDF58[actor_index].unk1C & ~0x1FF) | 2;
    D_800FDF58[actor_index].unk1C = initial_flags;
    D_800FDF58[actor_index].unk0 = 0xFFFB0000;
    flags = D_800FDF58[actor_index].unk1C;
    flags &= mask_a;
    flags &= mask_b;
    flags &= mask_c;
    D_800FDF58[actor_index].unk1C = flags;
    D_800FDF58[actor_index].unk0 = 0xFFFB0000;

    if (actor_index == 1 && D_800FDA83 == 0)
    {
        FieldActorObjectRecord* entry1 = &D_800FDF58[1];
        entry1->unk1C = (entry1->unk1C & 0xFF87FFFF) | 0x500000;
    }
    else
    {
        D_800FDF58[actor_index].unk1C &= 0xFF87FFFF;
    }

    D_800FDF58[actor_index].unk1C &= ~0x800;
    D_800FDF58[actor_index].unkC = g_field_resource_entries[resource_entry_index].slot_index;
    D_800FDF58[actor_index].unk3B = resource_entry_index;
    D_800FDF58[actor_index].unk21 = 0;
    q16 = &D_800FDF58[actor_index].unk10;
    q16[0] = 0;
    q16[1] = 0;
    q16[2] = 0;
    D_800FDF58[actor_index].unk16 = 1;
    D_800FDF58[actor_index].unk34 = 0;
    q32 = &D_800FDF58[actor_index].unk44;
    q32[0] = 0;
    q32[1] = 0;
    q32[2] = 0;
    D_800FDF58[actor_index].unk1A = 0x80;
    D_800FDF58[actor_index].unk19 = 0x80;
    D_800FDF58[actor_index].unk18 = 0x80;

    if (actor_index == 2 && D_800FDCEA >= 0x41)
    {
        FieldActorObjectRecord* entry2 = &D_800FDF58[2];
        entry2->unk1C = (entry2->unk1C & 0xFFFCFFFF) | (((g_pad_ctx[0x29D7] + 1) & 3) << 16);
        return;
    }

    D_800FDF58[actor_index].unk1C &= 0xFFFCFFFF;
}


/**
 * @brief Reset one actor-part runtime record to its default render state.
 * @param part_index Actor-part record to initialize.
 * @param timer_mode Selects the default timer values.
 */
void field_initialize_actor_part(s32 part_index, s32 timer_mode)
{
    s32 words_remaining;
    u32* word_cursor;

    words_remaining = sizeof(D_800FE3A0[part_index]) / sizeof(*word_cursor);
    word_cursor = (u32*)&D_800FE3A0[part_index];
    do
    {
        *word_cursor = 0;
        words_remaining--;
        word_cursor++;
    } while (words_remaining != 0);

    D_800FE3A0[part_index].unk10 = 0x80;
    D_800FE3A0[part_index].unkF = 0x80;
    D_800FE3A0[part_index].unkE = 0x80;
    D_800FE3A0[part_index].unk8 = 1;
    D_800FE3A0[part_index].unk9 = 0xFF;
    D_800FE3A0[part_index].unkD = 8;
    D_800FE3A0[part_index].u14.h.hi = 20;
    D_800FE3A0[part_index].u24.b.unk25 = 8;
    D_800FE3A0[part_index].u24.b.unk24 = 8;
    D_800FE3A0[part_index].unk23 = 8;
    D_800FE3A0[part_index].unk18 = 0x100;
    D_800FE3A0[part_index].u4.b.flag = 1;
    D_800FE3A0[part_index].u4.b.mode = 1;
    D_800FE3A0[part_index].u0.b.mode = 2;

    if (timer_mode != 0)
    {
        D_800FE3A0[part_index].unk2E = 0x30;
        D_800FE3A0[part_index].unk33 = 0x30;
    }
    else
    {
        D_800FE3A0[part_index].unk2E = 0x40;
        D_800FE3A0[part_index].unk33 = 0x40;
    }

    D_800FE3A0[part_index].unk11 = 0xFF;
    D_800FE3A0[part_index].unk28 |= 0x2000000;
    D_800FE3A0[part_index].u14.b.mode = 2;
    D_800FE3A0[part_index].u24.w |= 0x100000;
}

/**
 * @brief Apply the same color and render mode to every actor object and part.
 * @param red Red component.
 * @param green Green component.
 * @param blue Blue component.
 * @param color_flag Low bit copied into the actor color-control flag.
 * @param render_mode Two-bit render mode copied into each actor part.
 * @see decomp.me (100%) TODO
 */
void field_set_all_actor_render_state(s32 red, s32 green, s32 blue, s32 color_flag, s32 render_mode)
{
    s32 i;
    u32 mode_bits;
    u32 color_flag_bits;

    mode_bits = (render_mode & 3) << 22;
    for (i = 0; i < 13; i++)
    {
        D_80105AE0[i].unk1A8 = red;
        D_80105AE0[i].unk1A9 = green;
        D_80105AE0[i].unk1AA = blue;
        D_800FE3A0[i].unkE = red;
        D_800FE3A0[i].unkF = green;
        D_800FE3A0[i].unk10 = blue;
        D_800FE3A0[i].u4.w = (D_800FE3A0[i].u4.w & 0xFF3FFFFF) | mode_bits;
    }

    for (i = 0; i < 13; i++)
    {
        color_flag_bits = (color_flag & 1) << 23;
        D_800FDF58[i].unk1C = (D_800FDF58[i].unk1C & 0xFF7FFFFF) | color_flag_bits;
    }
}


/**
 * @brief Apply color and render state to one actor selected by script handle.
 * @param red Red component.
 * @param green Green component.
 * @param blue Blue component.
 * @param color_flag Low bit copied into the actor color-control flag.
 * @param render_mode Two-bit render mode copied into the actor part.
 * @param actor_selector Selector passed to the actor lookup.
 * @return 0 on success, or -1 when no actor matches the selector.
 * @see decomp.me (100%) TODO
 */
s32 field_set_actor_render_state(s32 red, s32 green, s32 blue, s32 color_flag, s32 render_mode, s32 actor_selector)
{
    FieldActorObjectRecord *rec;

    rec = func_80087C9C(actor_selector);
    if (rec == (FieldActorObjectRecord*)-1)
    {
        return -1;
    }

    D_80105AE0[rec->unk3A].unk1A8 = red;
    D_80105AE0[rec->unk3A].unk1A9 = green;
    D_80105AE0[rec->unk3A].unk1AA = blue;
    D_800FE3A0[rec->unk3A].unkE = red;
    D_800FE3A0[rec->unk3A].unkF = green;
    D_800FE3A0[rec->unk3A].unk10 = blue;
    D_800FE3A0[rec->unk3A].u4.w = (D_800FE3A0[rec->unk3A].u4.w & 0xFF3FFFFF) | ((render_mode & 3) << 22);
    rec->unk1C = (rec->unk1C & 0xFF7FFFFF) | ((color_flag & 1) << 23);
    return 0;
}


/**
 * @brief Load an actor texture resource and upload its palette and image data to VRAM.
 * @param resource_id CD resource id to read.
 * @param slot_index Actor texture slot.
 * @param texture_column Texture column within the VRAM actor area.
 * @param narrow_layout Selects the narrow half-height layout when nonzero.
 * @param palette_row Palette row offset.
 * @see decomp.me (100%) TODO
 */
static void field_load_actor_texture_set(s32 resource_id, s32 slot_index, s32 texture_column, s32 narrow_layout, s32 palette_row)
{
    RECT rect;
    FieldCdBuffer* buf;
    s32 second;
    s32 full_width;

    buf = D_8010D038;
    full_width = 0x100;
    cdrom_queue_read(resource_id & 0xFFFF, buf);
    cdrom_wait_queue_empty();
    second = buf->unk8;
    buf->unk14 = 0;

    if (texture_column == 0 || narrow_layout != 0)
    {
        if (narrow_layout != 0)
        {
            rect.x = 0xC0;
            rect.y = palette_row + 0x1F4;
            rect.w = 0x40;
            rect.h = 1;
        }
        else
        {
            rect.y = palette_row + 0x1F4;
            rect.w = full_width;
            rect.x = 0;
            rect.h = 1;
        }
        LoadImage(&rect, &buf->unk14);
    }

    if (slot_index >= 2)
    {
        s32 base = (texture_column << 6) + 0x340;
        s32 off = slot_index << 6;
        rect.x = base - off;
        rect.w = 0x40;
        rect.y = 0;
        rect.h = 0x100;
    }
    else if (narrow_layout != 0)
    {
        s32 base = (texture_column << 6) + 0x380;
        rect.x = base - (slot_index << 7);
        rect.y = 0x80;
        rect.w = 0x40;
        rect.h = 0x80;
    }
    else
    {
        s32 base = (texture_column << 6) + 0x380;
        s32 off = slot_index << 7;
        rect.x = base - off;
        rect.w = 0x40;
        rect.y = 0;
        rect.h = 0x100;
    }

    LoadImage(&rect, (u8*)(second + (s32)buf + 0x14));
    DrawSync(0);
}

/**
 * @brief Advance and dispatch the per-frame runtime state for all field actor objects.
 * @see decomp.me (100%) TODO
 */
void field_update_actor_objects(void)
{
    FieldActorObjectRecord *record;
    FieldActorObjectState *actor_state;
    s32 i;
    s32 mode;
    s32 count;
    s32 index;

    record = &D_800FDF58[0];
    actor_state = &D_80105AE0[0];

    if (D_80122714 == 0)
    {
        func_8009184C();
        if (D_80122714 == 0 && D_800FE754 == 0)
        {
            func_8009A2A4(D_800FDF58);
        }
    }

    func_80096394();
    index = 0;
    i = 0;
    count = -1;

    do
    {
        if (record->unk25 != 0xFF)
        {
            if (!(record->unk1C & 0x1FF) && count <= 0)
            {
                count++;
            }

            if (!(actor_state->unkC & 0x2000))
            {
                record->unk40 = field_advance_actor_animation_frame(record);
            }
            else
            {
                record->unk3C |= 0x1000000;
            }

            mode = actor_state->unk10 & 0xF;
            if (D_800FE754 == mode || mode == 0)
            {
                func_80086494(i);
                if (!(actor_state->u.unk178 & 0x81))
                {
                    if (D_800F229C == 0 && g_field_return_to_title_prompt_state == 0)
                    {
                        func_800880EC(record);
                    }

                    mode = record->unk1C & 0x1FF;
                    if (mode == 0)
                    {
                        if (D_80122714 == 0 && D_80122710 == 0 && D_800F229C == 0)
                        {
                            if (!(actor_state->unkC & 0x21E4))
                            {
                                func_8008DC54(record, count);
                            }
                            else
                            {
                                field_settle_actor_vertical_offset(record);
                            }
                        }
                    }
                    else if (mode == 1)
                    {
                        if (D_800F229C == 0 && g_field_return_to_title_prompt_state == 0)
                        {
                            if (!(actor_state->unkC & 0x21E4))
                            {
                                func_8008D29C(record, index, 0x600 + (index * 0x400));
                                record->unk1C &= ~0x800;
                            }
                            else
                            {
                                field_settle_actor_vertical_offset(record);
                            }
                        }
                        index++;
                    }
                    else if (mode == 2)
                    {
                        if (g_field_return_to_title_prompt_state == 0 && D_800F229C == 0 && D_80122B20 == 0)
                        {
                            if (record->unk2A != 0x93 && record->unk2A != 0x94 && record->unk2A != 0x90 &&
                                record->unk2A != 0xAE && record->unk2A != 0x8E && record->unk2A != 0xB8)
                            {
                                func_80087564(record);
                            }
                        }

                        if (!(actor_state->unkC & 0x21E4))
                        {
                            func_8008EF0C(record);
                            if (g_field_return_to_title_prompt_state == 0 && D_800F229C == 0)
                            {
                                record->unk1C |= 0x800;
                            }
                        }
                        else
                        {
                            field_settle_actor_vertical_offset(record);
                        }
                    }

                    func_8008D174(record);
                }
            }
        }

        i++;
        record++;
        actor_state++;
    } while (i < 13);

    func_80091D7C();
}

/**
 * @brief Move a negative actor Y offset toward zero by one fixed step.
 * @param record Actor object record to update.
 * @see decomp.me (100%) TODO
 */
void field_settle_actor_vertical_offset(FieldActorObjectRecord *record)
{
    if (record->unk4 < 0)
    {
        record->unk4 += 0x800;
        if (record->unk4 > 0)
        {
            record->unk4 = 0;
        }
    }
}


/**
 * @brief Emit render packets for every active field actor object.
 * @param render_context Field render context and packet cursor.
 * @see decomp.me (100%) TODO
 */
void field_render_actor_objects(FieldRenderContext *render_context)
{
    FieldActorObjectRecord *record;
    FieldActorObjectState *actor_state;
    u32 *ordering_table;
    s32 packet_cursor;
    s32 i;

    record = &D_800FDF58[0];
    ordering_table = &render_context->unk40;
    i = 0;
    actor_state = &D_80105AE0[0];
    packet_cursor = render_context->unk40B8;

    do
    {
        if (record->unk25 != 0xFE && record->unk25 != 0xFF)
        {
            if (record->unk40 >= 0)
            {
                packet_cursor = func_80077FB4(record, packet_cursor, ordering_table, record->unk40, 0, &D_800FE3A0[i]);
            }
            else
            {
                packet_cursor = func_80075C88(record, packet_cursor, ordering_table, record->unk40, 0, &D_800FE3A0[i]);
            }
        }
        else if (record->unk25 == 0xFE)
        {
            actor_state->unk12C = 0x100000;
            actor_state->unk140 = -8;
            actor_state->unk142 = -0xF;
            actor_state->unk144 = 8;
            actor_state->unk146 = 0;
        }
        else
        {
            actor_state->unk12C = 0;
        }

        i++;
        record++;
        actor_state++;
    } while (i < 13);

    render_context->unk40B8 = packet_cursor;
}

/**
 * @brief Stream one CD resource into the current field resource cursor.
 * @param resource_id CD resource id to queue.
 * @see decomp.me (100%) TODO
 */
static void field_stream_resource_to_buffer(u16 resource_id)
{
    s32 size;

    size = cdrom_queue_read(resource_id);
    cdrom_wait_queue_empty();
    g_field_resource_cursor += (size + 3) & ~3;
}

/**
 * @brief Advance the current forward actor animation and resolve its frame data.
 * @param record Actor object record whose animation is advanced.
 * @return Pointer to the current decoded frame data.
 * @see decomp.me (100%) TODO
 */
u8 *field_advance_actor_animation_frame(FieldActorObjectRecord *record)
{
    u8 *base;
    u8 *p;
    u8 *q;
    u32 mode;
    s32 wrap;
    s32 shift;
    s32 idx;
    s32 pos;
    s32 at_end;
    u8 seq;
    u8 cursor;
    s32 dur;
    s32 off;
    u32 hi;

    base = g_field_resource_entries[record->unk3B].start;
    p = base + 4;
    mode = p[1] >> 1;
    wrap = p[1] & 1;
    if (mode & 0x40)
    {
        mode -= 0x40;
    }

    seq = record->unk21;
    idx = seq & 0x7F;
    if (idx >= (s32)base[4])
    {
        p = base + 6;
        record->unk21 = seq & 0x80;
    }
    else
    {
        p = p + (idx * 2 + 2);
    }

    off = p[0];
    hi = p[1];
    p = base + off + ((hi & 0x7F) << 8);
    shift = (hi >> 7) + 1;
    record->unk3C |= 0x1000000;

    if (record->unk24 != 0)
    {
        record->unk16--;
        record->unk34++;
    }

    if (record->unk16 == 0 && record->unk24 != 0)
    {
        cursor = record->unk27 + 1;
        record->unk27 = cursor;
        if (cursor >= p[0])
        {
            if (record->unk2E == 0 || --record->unk2E == 0)
            {
                if (record->unk1C & 0x800)
                {
                    record->unk16 = 1;
                    record->unk34 = 1;
                    record->unk36 = 0;
                    record->unk27--;
                    return (u8 *)record->unk40;
                }
            }
            record->unk27 = 0;
        }

        pos = record->unk27;
        record->unk3C &= 0xFEFFFFFF;
        at_end = (pos + 1) >= (s32)p[0];
        p = p + ((pos << shift) + 1);
        dur = p[1];
        record->unk16 = dur;
        record->unk35 = dur;
        if (record->unk16 == 0)
        {
            record->unk16++;
            record->unk35++;
        }
        record->unk34 = 0;
        if (shift == 2)
        {
            record->unk37 = p[2];
            record->unk36 = p[3];
            if (at_end)
            {
                record->unk38 = record->unk37;
            }
            else
            {
                record->unk38 = p[6];
            }
        }
        else
        {
            record->unk38 = 0;
            record->unk37 = 0;
            record->unk36 = mode;
        }
    }
    else
    {
        p = p + ((record->unk27 << shift) + 1);
    }

    q = base + base[2] + (base[3] << 8) + (p[0] * 2 + 2);
    if (wrap)
    {
        return (u8 *)((s32)(base + q[0] + (q[1] << 8)) & 0x7FFFFFFF);
    }
    return base + q[0] + (q[1] << 8);
}

/**
 * @brief Restart an actor animation from its first frame in forward playback.
 * @param record Actor object record to restart.
 * @see decomp.me (100%) TODO
 */
void field_restart_actor_animation(FieldActorObjectRecord *record)
{
    record->unk27 = 0;
    record->unk1C &= ~0x800;
    record->unk40 = field_begin_actor_animation_forward(record, g_field_resource_entries[record->unk3B].start);
}

/**
 * @brief Initialize forward playback and resolve the first frame of an animation resource.
 * @param record Actor object record to initialize.
 * @param resource_base Animation resource base.
 * @return Pointer to the first decoded frame data.
 * @see decomp.me (100%) TODO
 */
u8 *field_begin_actor_animation_forward(FieldActorObjectRecord *record, u8 *resource_base)
{
    u8 *p;
    u8 *q;
    u32 mode;
    s32 wrap;
    s32 shift;
    s32 idx;
    s32 at_end;
    s32 dur;
    s32 off;
    u32 hi;
    u8 seq;

    p = resource_base + 4;
    mode = p[1] >> 1;
    wrap = p[1] & 1;
    if (mode & 0x40)
    {
        mode -= 0x40;
    }

    seq = record->unk21;
    idx = seq & 0x7F;
    if (idx >= (s32)resource_base[4])
    {
        p = resource_base + 6;
        record->unk21 = seq & 0x80;
    }
    else
    {
        p = p + (idx * 2 + 2);
    }

    off = p[0];
    hi = p[1];
    p = resource_base + off + ((hi & 0x7F) << 8);
    shift = (hi >> 7) + 1;

    at_end = (record->unk27 + 1) >= (s32)p[0];
    p = p + 1;
    dur = p[1] * record->unk24;
    record->unk16 = dur;
    record->unk35 = dur;
    if (record->unk16 == 0)
    {
        record->unk16++;
        record->unk35++;
    }
    record->unk34 = 0;
    if (shift == 2)
    {
        record->unk37 = p[2];
        record->unk36 = p[3];
        if (at_end)
        {
            record->unk38 = 0;
        }
        else
        {
            record->unk38 = p[6];
        }
    }
    else
    {
        record->unk38 = 0;
        record->unk37 = 0;
        record->unk36 = mode;
    }

    if (record->unk35 == 0)
    {
        record->unk35 = 1;
    }
    record->unk3C &= 0xFEFFFFFF;

    q = resource_base + resource_base[2] + (resource_base[3] << 8) + (p[0] * 2 + 2);
    if (wrap)
    {
        return (u8 *)((s32)(resource_base + q[0] + (q[1] << 8)) & 0x7FFFFFFF);
    }
    return resource_base + q[0] + (q[1] << 8);
}

/**
 * @brief Restart an actor animation from its final frame in reverse playback.
 * @param record Actor object record to restart.
 * @see decomp.me (100%) TODO
 */
void field_restart_actor_animation_reverse(FieldActorObjectRecord *record)
{
    record->unk1C |= 0x800;
    record->unk40 = field_begin_actor_animation_reverse(record, g_field_resource_entries[record->unk3B].start);
}

/**
 * @brief Initialize reverse playback and resolve the final frame of an animation resource.
 * @param record Actor object record to initialize.
 * @param resource_base Animation resource base.
 * @return Pointer to the final decoded frame data.
 * @see decomp.me (100%) TODO
 */
u8 *field_begin_actor_animation_reverse(FieldActorObjectRecord *record, u8 *resource_base)
{
    u8 *p;
    u8 *q;
    u32 mode;
    s32 wrap;
    s32 shift;
    s32 idx;
    s32 off;
    u32 hi;
    u8 seq;
    s32 dur;

    p = resource_base + 4;
    mode = p[1] >> 1;
    wrap = p[1] & 1;
    if (mode & 0x40)
    {
        mode -= 0x40;
    }

    seq = record->unk21;
    idx = seq & 0x7F;
    if (idx >= (s32)resource_base[4])
    {
        p = resource_base + 6;
        record->unk21 = seq & 0x80;
    }
    else
    {
        p = p + (idx * 2 + 2);
    }

    off = p[0];
    hi = p[1];
    p = resource_base + off + ((hi & 0x7F) << 8);
    shift = (hi >> 7) + 1;

    record->unk27 = p[0] - 1;
    p = p + ((record->unk27 << shift) + 1);
    dur = p[1] * record->unk24;
    record->unk16 = dur;
    record->unk35 = dur;
    if (record->unk16 == 0)
    {
        record->unk16++;
        record->unk35++;
    }
    record->unk34 = 0;
    if (shift == 2)
    {
        record->unk37 = p[2];
        record->unk36 = p[3];
        record->unk38 = 0;
    }
    else
    {
        record->unk38 = 0;
        record->unk37 = 0;
        record->unk36 = mode;
    }

    record->unk3C &= 0xFEFFFFFF;

    q = resource_base + resource_base[2] + (resource_base[3] << 8) + (p[0] * 2 + 2);
    if (wrap)
    {
        return (u8 *)((s32)(resource_base + q[0] + (q[1] << 8)) & 0x7FFFFFFF);
    }
    return resource_base + q[0] + (q[1] << 8);
}

/**
 * @brief Return the frame count of the next animation entry.
 * @param record Actor object record that selects the animation sequence.
 * @return Frame count of the next entry, or zero when there is no next entry.
 */
u8 field_get_next_animation_frame_count(FieldActorObjectRecord* record)
{
    u8* base;
    u8* p;
    s32 idx;

    do
    {
        base = g_field_resource_entries[record->unk3B].start;
    } while (0);
    p = base + 4;
    idx = (record->unk21 & 0x7F) + 1;
    if (idx >= (s32)base[4])
    {
        return 0;
    }
    p = p + (idx * 2 + 2);
    p = base + p[0] + ((p[1] & 0x7F) << 8);
    record = (FieldActorObjectRecord*)(u32)p[0];
    return (u32)record;
}

/**
 * @brief Advance an actor-part animation and resolve its current frame data.
 * @param record Actor object record whose part animation is advanced.
 * @param resource_base Animation resource base.
 * @return Pointer to the current decoded frame data.
 * @see decomp.me (100%) TODO
 */
u8 *field_advance_actor_part_animation_frame(FieldActorObjectRecord *record, u8 *resource_base)
{
    u8 *p;
    u8 *q;
    FieldActorPartDef *part;
    u32 mode;
    s32 wrap;
    s32 shift;
    s32 idx;
    s32 pos;
    s32 at_end;
    s32 dur;
    s32 off;
    u32 hi;
    u8 cursor;

    p = resource_base + 4;
    mode = p[1] >> 1;
    wrap = p[1] & 1;
    if (mode & 0x40)
    {
        mode -= 0x40;
    }

    idx = record->unk21 & 0x7F;
    if (idx >= (s32)resource_base[4])
    {
        p = resource_base + 6;
    }
    else
    {
        p = p + (idx * 2 + 2);
    }

    off = p[0];
    hi = p[1];
    p = resource_base + off + ((hi & 0x7F) << 8);
    record->unk3C |= 0x1000000;
    shift = (hi >> 7) + 1;

    if (D_800F2298 == 0 && D_8012269C == 0 && D_801227C8 == 0 && record->unk24 != 0)
    {
        record->unk16--;
        record->unk34++;
    }

    if (record->unk16 == 0 && record->unk24 != 0)
    {
        cursor = record->unk27 + 1;
        record->unk27 = cursor;
        if (cursor >= p[0])
        {
            part = &g_field_actor_slots[record->unk22].unk0[record->unk23];
            if (!((part->u4.w >> 4) & 3))
            {
                func_80071500(record, part);
                return 0;
            }
            record->unk27 = 0;
        }

        pos = record->unk27;
        record->unk3C &= 0xFEFFFFFF;
        at_end = (pos + 1) == (s32)p[0];
        p = p + ((pos << shift) + 1);
        dur = p[1] * record->unk24;
        record->unk34 = 0;
        record->unk16 = dur;
        record->unk35 = dur;
        if (shift == 2)
        {
            record->unk37 = p[2];
            record->unk36 = p[3];
            if (at_end)
            {
                record->unk38 = 0;
            }
            else
            {
                record->unk38 = p[6];
            }
        }
        else
        {
            record->unk38 = 0;
            record->unk37 = 0;
            record->unk36 = mode;
        }
    }
    else
    {
        p = p + ((record->unk27 << shift) + 1);
    }

    q = resource_base + resource_base[2] + (resource_base[3] << 8) + (p[0] * 2 + 2);
    if (wrap)
    {
        return (u8 *)((s32)(resource_base + q[0] + (q[1] << 8)) & 0x7FFFFFFF);
    }
    return resource_base + q[0] + (q[1] << 8);
}

/**
 * @brief Read and unpack a field actor resource package.
 * @param resource_id CD resource id to read.
 * @param slot_index Actor slot associated with the package.
 * @param resource_entry_index Resource entry receiving the package.
 * @see decomp.me (100%) TODO
 */
void field_load_resource_package(u16 resource_id, s32 slot_index, s32 resource_entry_index)
{
    FieldCdBuffer *buf;
    s32 size;

    buf = D_8010D038;
    size = cdrom_queue_read(resource_id, buf);
    cdrom_wait_queue_empty();
    field_unpack_resource_package(buf, size, slot_index, resource_entry_index);
}

/**
 * @brief Split a staged actor resource package into animation data and VRAM textures.
 * @param buf Staged package header and data.
 * @param size Total package size in bytes.
 * @param slot_index Actor slot receiving the package.
 * @param palette_row Palette row used for texture upload.
 * @see decomp.me (100%) TODO
 */
void field_unpack_resource_package(FieldCdBuffer *buf, s32 size, s32 slot_index, s32 palette_row)
{
    switch (buf->unk0 >> 2)
    {
    case 2:
        field_append_resource_data((u8 *)buf + buf->unk4, size - buf->unk4, slot_index);
        field_upload_resource_texture((u8 *)buf + buf->unk0, slot_index, 0, palette_row);
        break;
    case 3:
        field_append_resource_data((u8 *)buf + buf->unk8, size - buf->unk8, slot_index);
        field_upload_resource_texture((u8 *)buf + buf->unk0, slot_index, 0, palette_row);
        field_upload_resource_texture((u8 *)buf + buf->unk4, slot_index, 1, palette_row);
        break;
    case 4:
        field_append_resource_data((u8 *)buf + buf->unkC, size - buf->unkC, slot_index);
        field_upload_resource_texture((u8 *)buf + buf->unk0, slot_index, 0, palette_row);
        field_upload_resource_texture((u8 *)buf + buf->unk4, slot_index, 1, palette_row);
        field_upload_resource_texture((u8 *)buf + buf->unk8, slot_index, 2, palette_row);
        break;
    }
}

/**
 * @brief Upload one texture section from a staged actor resource package.
 * @param buf Texture section header and data.
 * @param slot_index Actor texture slot.
 * @param texture_index Texture section index.
 * @param palette_row Palette row used by the upload.
 * @see decomp.me (100%) TODO
 */
void field_upload_resource_texture(FieldCdBuffer *buf, s32 slot_index, s32 texture_index, s32 palette_row)
{
    RECT rect;
    s32 second;

    second = buf->unk8;

    if (texture_index == 2)
    {
        rect.x = 0xC0;
        rect.y = palette_row + 0x1F4;
        rect.w = 0x40;
        rect.h = 1;
    }
    else
    {
        rect.y = palette_row + 0x1F4;
        rect.w = 0x100;
        rect.x = 0;
        rect.h = 1;
    }

    if (texture_index != 1)
    {
        LoadImage(&rect, &buf->unk14);
    }

    if (slot_index >= 2)
    {
        s32 base = 0x340;
        s32 off = slot_index << 6;
        rect.x = base - off;
        rect.w = 0x40;
        rect.y = 0;
        rect.h = 0x100;
    }
    else
    {
        FieldCdBuffer *hdr = (FieldCdBuffer *)(second + (s32)buf);
        s32 w = hdr->unk10;
        s32 h = hdr->unk12;

        if (texture_index == 2)
        {
            rect.x = 0x3C0 - (slot_index << 7);
            rect.y = 0x80;
            rect.w = 0x40;
            rect.h = 0x80;
        }
        else if (texture_index == 1)
        {
            rect.x = 0x3C0 - (slot_index << 7);
            rect.y = 0;
            rect.w = w;
            rect.h = h;
        }
        else
        {
            s32 base = (texture_index << 6) + 0x380;
            s32 off = slot_index << 7;
            rect.x = base - off;
            rect.w = 0x40;
            rect.y = 0;
            rect.h = 0x100;
        }
    }

    LoadImage(&rect, (u8*)(second + (s32)buf + 0x14));
}

/**
 * @brief Copy resource data into the shared actor-resource arena.
 * @param src Source words.
 * @param length Number of source bytes.
 * @param slot_index Resource entry that receives the copied data.
 * @see decomp.me (100%) TODO
 */
void field_append_resource_data(u32 *src, s32 length, s32 slot_index)
{
    u32 *dst;
    u32 n;

    dst = g_field_resource_cursor;
    n = (u32)(length + 3) >> 2;
    while (n != 0)
    {
        *dst = *src;
        src++;
        n--;
        dst++;
    }

    g_field_resource_entries[slot_index].start = g_field_resource_cursor;
    g_field_resource_cursor += (length + 3) & ~3;
}

/**
 * @brief Convert an actor screen X position to the field audio pan range.
 * @param actor_index Actor object index.
 * @return Pan value clamped to 0x60 through 0x9F.
 * @see decomp.me (100%) TODO
 */
s32 field_get_actor_sound_pan(s32 actor_index)
{
    Vec2s pos;
    s32 screen_x;

    pos.x = 0xA0 + D_800F22A0 / 256 + D_800FDF58[actor_index].unk0 / 256;
    pos.y = 0x70 + D_800F22A4 / 256 + D_800FDF58[actor_index].unk4 / 256 - D_800FDF58[actor_index].unk8 / 512 - D_800F22A8 / 512;

    screen_x = pos.x;
    if (screen_x >= 0x10)
    {
        if (screen_x >= 0x131)
        {
            return 0x9F;
        }
        screen_x = ((screen_x - 0x10) * 63) / 288;
        return screen_x + 0x60;
    }
    return 0x60;
}

/**
 * @brief Rebuild cached portrait images when any of the three actor portrait selections change.
 * @see decomp.me (100%) TODO
 */
void field_refresh_actor_portraits(void)
{
    u8 *portrait1;
    u8 *portrait2;
    u8 *pad;
    s32 portrait1_index;
    s32 selector;

    if (D_800FD818[1].unk3 != 0)
    {
        portrait1_index = D_800FD818[1].unk2 + 2;
    }
    else
    {
        portrait1_index = (D_800FD818[1].u0.h >> 1) & 1;
    }

    if (D_800FD818[0].unk256 != ((D_800FD818[0].u0.h >> 1) & 1) ||
        D_800FD818[1].unk256 != portrait1_index ||
        D_800FD818[2].unk256 != D_800FD818[2].unk2 + 0xE)
    {
        if (D_800FD818[1].unk256 != portrait1_index)
        {
            if (D_800FD818[1].unk3 != 0)
            {
                func_800A5174(1, D_800FD818[1].unk2 + 0xA37);
            }
            else
            {
                func_800A5174(1, 0xA37);
            }
        }
        if (D_800FD818[2].unk256 != D_800FD818[2].unk2 + 0xE)
        {
            func_800A5174(2, D_800FD818[2].unk2 + 0xA9B);
        }

        cdrom_stream(0x5E5, D_8010D038);
        cdrom_wait_queue_empty();

        D_800FD818[0].unk256 = (D_800FD818[0].u0.h >> 1) & 1;
        bcopy((u8 *)D_8010D038 + ((u32 *)D_8010D038)[D_800FD818[0].unk256 + 1], g_prim_rect_buf, 0x4A0);

        portrait1 = g_prim_rect_buf + 0x4A0;
        D_800FD818[1].unk256 = portrait1_index;
        bcopy((u8 *)D_8010D038 + ((u32 *)D_8010D038)[D_800FD818[1].unk256 + 1], portrait1, 0x4A0);

        portrait2 = g_prim_rect_buf + 0x940;
        D_800FD818[2].unk256 = D_800FD818[2].unk2 + 0xE;
        bcopy((u8 *)D_8010D038 + ((u32 *)D_8010D038)[D_800FD818[2].unk256 + 1], portrait2, 0x4A0);

        if (D_800FD818[1].unk3 == 0)
        {
            func_800A5638(portrait1, (D_800FD818[1].u0.h >> 1) & 1);
        }

        if (g_pad_ctx[0xA90] != 0 && (*(u32 *)&g_pad_ctx[0xAA8] & 0x7F) == 4)
        {
            selector = *(s8 *)&g_pad_ctx[0x29D7];
            if (selector < 3)
            {
                pad = g_pad_ctx;
                pad += selector * 0x14C;
                func_800A55E4(portrait2, *(s32 *)(pad + 0x2B54));
            }
        }
    }

    func_80084240();
}

/**
 * @brief Mark every field effect record free and reset the effect allocator.
 * @see decomp.me (100%) TODO
 */
void field_reset_effect_pool(void)
{
    s32 i;
    s32 unused_state;

    unused_state = 0xFF;
    for (i = 0x102; i >= 0; i--)
    {
        D_800FF658[i].unk25 = unused_state;
    }

    D_80105770 = 0;
}

/**
 * @brief Retire every active field effect owned by an actor.
 * @param actor Actor whose owned effects are retired.
 * @see decomp.me (100%) TODO
 */
void field_clear_actor_effects(FieldActorState *actor)
{
    s32 i;
    s32 owner_index;
    s32 unused_state;
    FieldActorObjectRecord *effects;
    u8 *p;

    owner_index = actor->unk233;
    i = 0;
    unused_state = 0xFF;
    effects = D_800FF658;
    p = (u8 *)effects + 0x25;
    while (i < 0x103)
    {
        if (*p != unused_state && p[-3] == owner_index)
        {
            *p = unused_state;
        }
        i++;
        p += 0x54;
    }
}

/**
 * @brief Update effect-emitting parts for each active animation track of an actor.
 * @param actor Actor animation state to process.
 * @see decomp.me (100%) TODO
 */
void field_update_actor_effects(FieldActorState *actor)
{
    s32 i;

    if (actor->unk232 != 0)
    {
        for (i = 0; i < actor->unk232; i++)
        {
            if ((actor->unk23A >> i) & 1)
            {
                g_field_track_index = i;
                field_update_actor_part_effects(actor);
            }
        }
    }
    else
    {
        g_field_track_index = 0;
        field_update_actor_part_effects(actor);
    }
}

/**
 * @brief Evaluate one animation track and emit its actor-part effects.
 * @param actor Actor animation state to process.
 * @see decomp.me (100%) TODO
 */
void field_update_actor_part_effects(FieldActorState *actor)
{
    FieldActorPartDef *part;
    s32 i;
    s32 target_count;
    s32 previous_count;
    s32 spawn_count;
    u8 part_kind;
    u32 flags;

    part = actor->unk0;
    i = 0;
    if (actor->unk25 != 0)
    {
        do
        {
            part_kind = part->unk31;
            if (part_kind != 0xFE &&
                (!(actor->unkC->unkC & 0x800) || ((actor->unk240[actor->unk29] >> i) & 1)) &&
                part->unkB != 0xFF &&
                (!(part->u14.w & 4) || g_field_track_index == 0))
            {
                if (actor->unk229[g_field_track_index] == 0xFF)
                {
                    if (part_kind == 0xFF || (part->unk34 & 0x4000000))
                    {
                        goto next;
                    }
                    if (part->unk31 > actor->unk1EC[g_field_track_index])
                    {
                        goto next;
                    }
                }
                else if (part_kind != 0xFF)
                {
                    if (part->unk31 > actor->unk1EC[g_field_track_index])
                    {
                        goto next;
                    }
                }

                if (((u8 *)part)[0x2B] & 1)
                {
                    target_count = part->unkC;
                }
                else
                {
                    target_count = field_evaluate_parameter_track(actor, part->unkC);
                }

                flags = part->unk28;
                if (((flags >> 30) & 1) && actor->unkCC[g_field_track_index][i] >= target_count)
                {
                    goto next;
                }

                if ((part->u0.w >> 15) & 1)
                {
                    if (((flags >> 24) & 1) && actor->unk3B[g_field_track_index][i] != 0)
                    {
                        goto next;
                    }
                    if (field_get_track_counter_modulo(actor, (((u8 *)part)[7] & 0xF) + 1) != 0)
                    {
                        goto next;
                    }
                    if (actor->unk3B[g_field_track_index][i] >= target_count)
                    {
                        goto next;
                    }
                    do
                    {
                        previous_count = actor->unk3B[g_field_track_index][i];
                        D_80105760 = 0;
                        if (func_8006D79C(actor, i, 0) == -1)
                        {
                            goto next;
                        }
                        if (actor->unk3B[g_field_track_index][i] == previous_count)
                        {
                            goto next;
                        }
                    } while (actor->unk3B[g_field_track_index][i] < target_count);
                }
                else
                {
                    if (actor->unk3B[g_field_track_index][i] < target_count &&
                        field_get_track_counter_modulo(actor, (((u8 *)part)[7] & 0xF) + 1) == 0)
                    {
                        spawn_count = (part->unk2C & 0x1F) + 1;
                        while (spawn_count != 0)
                        {
                            if (actor->unk3B[g_field_track_index][i] >= target_count)
                            {
                                break;
                            }
                            D_80105760 = 0;
                            if (func_8006D79C(actor, i, 0) == -1)
                            {
                                break;
                            }
                            spawn_count--;
                        }
                    }
                }
            }
        next:
            i++;
            part++;
        } while (i < actor->unk25);
    }
}

/**
 * @brief Copy the actor-part color into a compatible spawned effect record.
 * @param effect Spawned effect record.
 * @param part Actor-part definition controlling the effect.
 * @param actor_record Actor object record that owns the part color.
 * @see decomp.me (100%) TODO
 */
static void field_apply_actor_part_color(FieldActorObjectRecord *effect, FieldActorPartDef *part, FieldActorObjectRecord *actor_record)
{
    if (!((part->u4.w >> 11) & 1) && !((part->unk28 >> 25) & 1) && (part->unk2C >> 5) == 0 &&
        (*(u32 *)&part->unkC & 0xFFFF0000) == 0x80800000 && part->unk10 == 0x80)
    {
        effect->unk1C |= 0x10008000;
        effect->unk18 = D_800FE3A0[actor_record->unk3A].unkE;
        effect->unk19 = D_800FE3A0[actor_record->unk3A].unkF;
        effect->unk1A = D_800FE3A0[actor_record->unk3A].unk10;
    }
}
