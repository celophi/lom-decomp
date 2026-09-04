#include "common.h"
#include "field_types.h"
#include "cd_resources.h"

s32 cdrom_stream(s32 resourceIndex, u32 destination);
void cdrom_wait_queue_empty(void);
extern void func_80084240(void); /* Fixed prototype */
void func_80140004(s32 cdLoadAddr, s32 imageResourceIndex, s32 musicResourceIndex, s32 audioClipIndex);
void func_800A74E8();
void func_800AA02C();

typedef struct
{
    char pad_00[0x1EC];
    u16 unk1EC[35]; /* 0x1EC - Array size bound by 0x232 offset */
    u8 unk232;      /* 0x232 */
    char pad_233;   /* 0x233 */
    u16 unk234;     /* 0x234 */
    u16 unk236;     /* 0x236 */
    u16 unk238;     /* 0x238 */
    u8 unk23A;      /* 0x23A */
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
    u8 pad0[2];                    /* 0x00 - 0x01 */
    u16 map0_action1_animation_id; /* 0x02 - 0x03 */
    u8 pad1[6];                    /* 0x04 - 0x09 */
    u16 map0_action5_animation_id; /* 0x0A - 0x0B */
    u8 pad2[11];                   /* 0x0C - 0x16 */
    u8 map0_action1_disabled;      /* 0x17 */
    u8 pad3[3];                    /* 0x18 - 0x1A */
    u8 map0_action5_disabled;      /* 0x1B */
    u8 pad4[8];                    /* 0x1C - 0x23 */
    u16 map1_action1_animation_id; /* 0x24 - 0x25 */
    u8 pad5[6];                    /* 0x26 - 0x2B */
    u16 map1_action5_animation_id; /* 0x2C - 0x2D */
    u8 pad6[11];                   /* 0x2E - 0x38 */
    u8 map1_action1_disabled;      /* 0x39 */
    u8 pad7[3];                    /* 0x3A - 0x3C */
    u8 map1_action5_disabled;      /* 0x3D */
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
    u32 unk0; /* 0x00 */
    u32 unk4; /* 0x04 */
    u8 unk8; /* 0x08 */
    u8 unk9; /* 0x09 */
    u8 padA;
    u8 unkB; /* 0x0B */
    u8 unkC; /* 0x0C */
    u8 unkD; /* 0x0D */
    u8 unkE; /* 0x0E */
    u8 unkF; /* 0x0F */
    u8 unk10; /* 0x10 */
    u8 unk11; /* 0x11 */
    u8 pad12[0x14 - 0x12];
    union
    {
        u32 w; /* 0x14 */
        struct
        {
            u16 lo; /* 0x14 */
            s16 hi; /* 0x16 */
        } h;
    } u14;
    s16 unk18; /* 0x18 */
    u8 pad1A[0x23 - 0x1A];
    u8 unk23; /* 0x23 */
    union
    {
        u32 w; /* 0x24 */
        struct
        {
            u8 unk24; /* 0x24 */
            u8 unk25; /* 0x25 */
            u8 unk26; /* 0x26 */
            u8 unk27; /* 0x27 */
        } b;
    } u24;
    u32 unk28; /* 0x28 */
    u8 unk2C; /* 0x2C */
    u8 pad2D;
    u8 unk2E; /* 0x2E */
    u8 pad2F[0x31 - 0x2F];
    u8 unk31; /* 0x31 */
    u8 unk32; /* 0x32 */
    u8 unk33; /* 0x33 */
    u32 unk34; /* 0x34 */
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
} Struct_801ED600;

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
} Struct_Unk28;

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
    s32 unk0; /* 0x00 */
    s32 unk4; /* 0x04 */
    s32 unk8; /* 0x08 */
    u32 unkC; /* 0x0C */
    s16 unk10; /* 0x10 */
    s16 unk12; /* 0x12 */
    s16 unk14; /* 0x14 */
    s16 unk16; /* 0x16 */
    u8 unk18; /* 0x18 */
    u8 unk19; /* 0x19 */
    u8 unk1A; /* 0x1A */
    u8 unk1B; /* 0x1B */
    s32 unk1C; /* 0x1C */
    u8 pad20[0x21 - 0x20];
    u8 unk21; /* 0x21 */
    u8 unk22; /* 0x22 */
    u8 unk23; /* 0x23 */
    u8 unk24; /* 0x24 */
    u8 unk25; /* 0x25 */
    u8 pad26[0x27 - 0x26];
    u8 unk27; /* 0x27 */
    u8 unk28; /* 0x28 */
    u8 pad29[0x2A - 0x29];
    s16 unk2A; /* 0x2A */
    s16 unk2C; /* 0x2C */
    u16 unk2E; /* 0x2E */
    s16 unk30; /* 0x30 */
    u8 unk32; /* 0x32 */
    u8 unk33; /* 0x33 */
    u8 unk34; /* 0x34 */
    u8 unk35; /* 0x35 */
    u8 unk36; /* 0x36 */
    u8 unk37; /* 0x37 */
    u8 unk38; /* 0x38 */
    u8 pad39[0x3A - 0x39];
    u8 unk3A; /* 0x3A */
    u8 unk3B; /* 0x3B */
    u32 unk3C; /* 0x3C */
    s32 unk40; /* 0x40 */
    u32 unk44; /* 0x44 */
    u32 unk48; /* 0x48 */
    u32 unk4C; /* 0x4C */
    u8 pad50[0x54 - 0x50];
} Struct_D800FDF58;

typedef struct
{
    u8 pad0[0xC];
    u32 unkC; /* 0x0C */
    u32 unk10; /* 0x10 */
    u8 pad14[0x12C - 0x14];
    u32 unk12C; /* 0x12C */
    u8 pad130[0x140 - 0x130];
    s16 unk140; /* 0x140 */
    s16 unk142; /* 0x142 */
    s16 unk144; /* 0x144 */
    s16 unk146; /* 0x146 */
    u8 pad148[0x174 - 0x148];
    s32 unk174; /* 0x174 */
    union
    {
        s32 unk178;
        struct
        {
            u8 pad[2];
            u8 unk17A;
            u8 pad2;
        } b;
    } u; /* 0x178 */
    u8 pad17C[0x18E - 0x17C];
    u8 unk18E; /* 0x18E */
    u8 pad18F[0x19C - 0x18F];
    s32 unk19C; /* 0x19C */
    s32 unk1A0; /* 0x1A0 */
    u8 pad1A4[0x1A8 - 0x1A4];
    u8 unk1A8; /* 0x1A8 */
    u8 unk1A9; /* 0x1A9 */
    u8 unk1AA; /* 0x1AA */
    u8 pad1AB[0x23C - 0x1AB];
} Struct_D80105AE0;

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
    Struct_Unk28 unk28;
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
        s16 red;   /* offset 0 */
        s16 green; /* offset 2 */
        s16 blue;  /* offset 4 */
    } s;
    u32 w; /* covers red and green as a 32-bit word */
} FieldColorScale;

/*
 * Per-element structure (stride 0x268). D_800FD818 is a 3-element array; the
 * absolute offsets previously used by func_8006A324 (0x268, 0x4BC, 0x4D0,
 * 0x724, ...) are elements [1] and [2] of this array.
 */
typedef struct
{
    union
    {
        u16 h; /* offset 0x00 as a halfword (flags) */
        struct
        {
            u8 unk0; /* offset 0x00 */
            u8 unk1; /* offset 0x01 */
        } b;
    } u0;
    u8 unk2;                /* offset 0x02 */
    u8 unk3;                /* offset 0x03 */
    u8 pad0[0x254 - 4];     /* 0x04 .. 0x253 */
    u16 unk254;             /* offset 0x254 */
    u8 unk256;              /* offset 0x256 */
    u8 pad1[0x268 - 0x257]; /* 0x257 .. 0x267 */
} D_800FD818_type;

extern D_800FD818_type D_800FD818[];

extern FieldColorScale g_field_color_scale;
extern s8 g_field_color_scale_active;

extern s32 D_801227C8;
extern s32 D_800F22B8;
extern s32 D_800F22BC;
extern s32 D_800F22C0;
extern s32 D_800F22C4;
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
extern Struct_D800FDF58 D_800FDF58[];
extern Struct_D80105AE0 D_80105AE0[];
extern u8 D_800FF59C;

/* Forward declarations for the scene-transition helpers folded in from the
 * former func_80067fb0.c / func_800681c0.c translation units (same TU). */
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
void field_open_return_to_title_prompt(void);
void cdrom_queue_seek(s32);
void akao_cmd_c1(s32, s32, s32);
void akao_cmd_a9(s32, s32);

/**
 * @brief Reset the field text scratch state, optionally saving current state.
 * @see decomp.me (100%) TODO
 */
void func_80067FB0(s32 arg0)
{
    if (D_800F229C != 0)
    {
        func_800A710C();
        if (D_800F229C != 0)
        {
            field_text_reset_scratch();
            if (D_800F229C != 0)
            {
                func_800A8880(arg0);
            }
            func_80063194();
        }
    }
}

/**
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
            func_8006C3FC(&D_800FDF58[i], (void*)~0x1FF);
        }
    }

    field_restore_default_action_animation_mappings(0);
    D_8012291C = 0;
    func_80092124();
}

/**
 * @brief Open the return-to-title confirmation prompt if arg0 is zero.
 * @param arg0 When 0, opens the prompt; otherwise returns immediately.
 * @see decomp.me (100%) TODO
 */
void func_800681C0(s32 arg0)
{
    if (arg0 == 0)
    {
        field_open_return_to_title_prompt();
    }
}

/**
 * @see decomp.me (100%) TODO
 */
void func_800681E4(s32 arg0, s32 arg1, s32 arg2)
{
    s32 fade_time;

    if (D_800F22C0 == 0)
    {
        D_800F22B8 = arg0;
        D_800F22BC = arg1;
        D_800F22C4 = arg2;
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
        D_800F22C0 = fade_time;
        akao_cmd_c1(0, 0x78, 0);
        akao_cmd_a9(0x78, 0);
    }
}

/**
 * decomp.me (100%) https://decomp.me/scratch/9Ady0
 */
void field_update_gover_load(void)
{
    Struct_801ED600* ptr = (Struct_801ED600*)0x801ED600;

    if (D_800F22C0 == 0)
    {
        return;
    }

    if (--D_800F22C0 == 0)
    {
        ptr->unk140 = 0;
        ptr->unk92 = 0;
        ptr->unk13F = 0;
        ptr->unk91 = 0;
        cdrom_stream(CD_RES_GOVER_BIN, 0x80140000);
        cdrom_wait_queue_empty();
        func_80140004(0x80160000, D_800F22B8, D_800F22BC, D_800F22C4);
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
 * decomp.me (100%) https://decomp.me/scratch/KDXt0
 */
void field_update_audio_timer(void)
{
    s32 temp_v0;

    if (g_field_audio_timer != 0)
    {
        temp_v0 = g_field_audio_timer - 1;
        g_field_audio_timer = temp_v0;
        if (temp_v0 == 0)
        {
            akao_song_cmd_12c();
            func_800A380C();
            func_800A3904(0, 1, D_8011588C);
        }
    }
}

/**
 * decomp.me (100%) https://decomp.me/scratch/Xn30r
 */
s32 field_get_track_counter_modulo(s32 animation_data, s32 divisor)
{
    return ((FieldTrackCounterView*)((u8*)animation_data + g_field_track_index * 2))->track_frame % divisor;
}

/**
 * decomp.me (100%) https://decomp.me/scratch/X9uyL
 */
void field_interpolate_palette_track(FieldAnimationData* animation_data, s32 palette_sequence, s32 palette_table, s16* output)
{
    s32 var_t9 = 0;
    s32 var_t1 = 4;
    s16* var_t7;
    s32 var_t6;
    s32 new_var;
    FieldParameterCurve* temp_v0;
    s32 var_t0 = (&animation_data->curves[palette_sequence & 0xF])->segment_count_flags & 0x7F;
    u16* var_t5 = &animation_data->segments[(&animation_data->curves[palette_sequence & 0xF])->segment_offset];
    new_var = palette_sequence;
    if (var_t0 != 0)
    {
        do
        {
            u16 temp = *var_t5;
            s32 temp_v1 = var_t9 + (temp & 0x3FF);
            if (((FieldTrackCounterView*)(((u8*)animation_data) + (g_field_track_index * 2)))->track_frame < temp_v1)
            {
                break;
            }
            var_t9 = temp_v1;
            var_t5++;
            var_t1 += 4;
            var_t0--;
            if (var_t1 == 16)
            {
                var_t1 = 4;
            }
        } while (var_t0 != 0);
    }
    if (var_t0 != 0)
    {
        s32 temp_a1 = new_var & 0xFFFF;
        u16* var_t8 = (u16*)(palette_table + (((temp_a1 >> var_t1) & 0xF) << 5));
        u16* var_t4;
        s32 temp_v1_2;
        if ((var_t1 + 4) != 16)
        {
            var_t4 = (u16*)(palette_table + (((temp_a1 >> (var_t1 + 4)) & 0xF) << 5));
        }
        else
        {
            var_t4 = (u16*)(palette_table + ((temp_a1 << 1) & 0x1E0));
        }
        var_t7 = output;
        var_t6 = 0;
        do
        {
            u16 a = *var_t8;
            u16 b = *var_t4;
            s32 low_a = a & 0x1F;
            s32 low_b = b & 0x1F;
            s32 diff0 = low_b - low_a;
            s32 temp_t0 = ((FieldTrackCounterView*)(((u8*)animation_data) + (g_field_track_index * 2)))->track_frame - var_t9;
            s32 temp_a1_2 = (*var_t5) & 0x3FF;
            s32 temp_t3 = (diff0 * temp_t0) / temp_a1_2;
            s32 mid_a = (a >> 5) & 0x1F;
            s32 mid_b = (b >> 5) & 0x1F;
            s32 diff1 = mid_b - mid_a;
            s32 temp_t2 = (diff1 * temp_t0) / temp_a1_2;
            s32 high_a = (a >> 10) & 0x1F;
            s32 high_b = (b >> 10) & 0x1F;
            s32 diff2 = high_b - high_a;
            s32 temp_a1_3 = (diff2 * temp_t0) / temp_a1_2;
            *var_t7 = (((a & 0x8000) | (low_a + temp_t3)) | ((mid_a + temp_t2) << 5)) | ((high_a + temp_a1_3) << 10);
            var_t7++;
            var_t6++;
            var_t8++;
            var_t4++;
        } while (var_t6 < 16);
    }
}

/**
 * decomp.me (100%) https://decomp.me/scratch/t5bIj
 */
s32 field_evaluate_parameter_track(FieldAnimationData* animation_data, s32 curve_index)
{
    s32 var_s4;
    FieldParameterCurve* temp_s3;
    s32 var_s2;
    s32 temp_s1;
    u16* var_s0;
    s32 var_a1;
    u16 var_a2;
    u16 temp_a0;
    s32 temp_v1_2;
    u16 temp_v1_3;
    s32 rand_val;
    var_s4 = 0;
    temp_s3 = animation_data->curves + curve_index;
    var_s2 = var_s4;
    var_a1 = temp_s3->segment_count_flags & 0x7F;
    var_s0 = animation_data->segments + temp_s3->segment_offset;
    if (var_a1 != 0)
    {
        var_a2 = *((u16*)((((u8*)animation_data) + (g_field_track_index * 2)) + 0x1EC));
        do
        {
            if (!animation_data)
            {
            }
            temp_a0 = *var_s0;
            temp_v1_2 = var_s4 + (temp_a0 & 0x3FF);
            if (((s32)var_a2) < temp_v1_2)
            {
                break;
            }
            var_s4 = temp_v1_2;
            var_s2 = temp_a0 >> 10;
            var_a1--;
            var_s0++;
        } while (var_a1 != 0);
    }
    temp_s1 = temp_s3->end_value - temp_s3->start_value;
    var_s2 = (temp_s1 * var_s2) >> 5;
    if (var_a1 == 0)
    {
        return temp_s3->start_value;
    }
    if (((*((u16*)temp_s3)) & 0x80) && (D_801227C8 == 0))
    {
        rand_val = rand();
        return temp_s3->start_value +
               (((var_s2 + (((((temp_s1 * ((*var_s0) >> 10)) >> 5) - var_s2) * ((*((u16*)((((u8*)animation_data) + (g_field_track_index * 2)) + 0x1EC))) - var_s4)) /
                            ((*var_s0) & 0x3FF))) *
                 rand_val) >>
                15);
    }
    else
    {
        temp_v1_3 = *var_s0;
        return (temp_s3->start_value + var_s2) +
               (((((temp_s1 * (temp_v1_3 >> 10)) >> 5) - var_s2) * ((*((u16*)((((u8*)animation_data) + (g_field_track_index * 2)) + 0x1EC))) - var_s4)) / (temp_v1_3 & 0x3FF));
    }
}

/**
 * decomp.me (100%) https://decomp.me/scratch/MCyYP
 */
s32 field_finalize_actor_animation(FieldActorState* actor)
{
    s32 found;
    u8 temp_a0;
    s32 i;
    FieldActorState* var;
    s16 temp_v1;
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
                temp_v1 = D_800FDF58[actor->owner_object_index].unk2A;
                if (((temp_v1 != 0x90) && (temp_v1 != 0x94)) || (D_80105AE0[actor->owner_object_index].unkC & 0x200))
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
                    temp_a0 = D_80105AE0[actor->unk229[found]].u.unk178;
                    if ((temp_a0 & 1) && (D_80105AE0[actor->unk229[found]].u.b.unk17A == actor->unk233))
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
            var = g_field_actor_slots;
            for (j = 0; j < 80; j++, var++)
            {
                found = 0;
                if (((actor != var) && (var->unk24 != 0)) && ((var->unkC->unkC >> 8) & 4))
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
            func_8006D21C(actor);
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
        temp_a0 = actor->unk2A;
        if (temp_a0 != 0)
        {
            actor->unk222 = 0;
            actor->unk24 = 0;
            func_80084424(actor->owner_object_index);
            for (i = 0; i < 80; i++)
            {
                if (((g_field_actor_slots[i].unk24 != 0) && (g_field_actor_slots[i].owner_object_index == actor->owner_object_index)) &&
                    ((temp_a0 = g_field_actor_slots[i].u224.unk224) & 1))
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
 * decomp.me (100%) https://decomp.me/scratch/7d7kv
 */
unsigned int field_evaluate_parameter_track_at_time(FieldAnimationData* animation_data, s32 curve_index, s32 frame)
{
    FieldParameterCurve* entry;
    int new_var2;
    u16* data_ptr;
    s32 count;
    s32 accumulated_val;
    s32 v1_reg;
    s32 diff;
    u16 val;
    int new_var3;
    s32 fraction;
    int new_var;
    s32 result;
    s32 local_arg2;

    accumulated_val = 0;
    v1_reg = 0;
    local_arg2 = frame;
    entry = &animation_data->curves[curve_index];

    count = entry->segment_count_flags & 0x7F;
    data_ptr = &animation_data->segments[entry->segment_offset];

    if (count != 0)
    {
        do
        {
            u16 current = *data_ptr;
            s32 sum = accumulated_val + (current & 0x3FF);

            if (local_arg2 < sum)
            {
                break;
            }

            accumulated_val = sum;
            v1_reg = current >> 10;
            data_ptr++;
        } while ((--count) != 0);
    }

    diff = entry->end_value - entry->start_value;
    new_var2 = 0x3FF;
    v1_reg = (diff * v1_reg) >> 5;
    new_var = new_var2;

    if (count == 0)
    {
        return entry->start_value;
    }

    if (((*((u16*)entry)) & 0x80) && (D_801227C8 == 0))
    {
        s32 rand_val = rand();
        val = *data_ptr;
        fraction = ((((diff * (val >> 10)) >> 5) - v1_reg) * (local_arg2 - accumulated_val)) / (val & new_var);
        new_var3 = entry->start_value + (((v1_reg + fraction) * rand_val) >> 15);
        result = ((v1_reg + fraction) * rand_val) >> 15;
        return new_var3;
    }
    else
    {
        val = *data_ptr;
        fraction = ((((diff * (val >> 10)) >> 5) - v1_reg) * (local_arg2 - accumulated_val)) / (val & new_var2);
        result = fraction;
        return (entry->start_value + v1_reg) + result;
    }
}

/**
 * decomp.me (100%) https://decomp.me/scratch/eRVUu
 */
void field_reset_actor_track_state(FieldActorResetState* actor)
{
    s32 i;
    s32 j;

    /* Changing this to use 'j' binds 'j' to register a1, */
    /* which forces 'i' to take register a3 later on.     */
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
 * decomp.me (100%) https://decomp.me/scratch/PjqLA
 */
void field_advance_actor_tracks(FieldActorTrackState* actor)
{
    s32 var_a0;

    if (field_finalize_actor_animation((FieldActorState*)actor) == 0)
    {
        /* Removed the redundant track-count wrapper. */
        for (var_a0 = 0; var_a0 < actor->unk232; var_a0++)
        {
            if (((s32)actor->unk23A >> var_a0) & 1)
            {
                actor->unk1EC[var_a0]++;
            }
        }

        /* Incremented before checking conditions to match delay slot scheduling */
        actor->unk236++;

        if (actor->unk238 != 0 && actor->unk232 != 0)
        {
            if ((actor->unk236 % actor->unk238) == 0)
            {
                s32 temp_a0 = actor->unk236 / actor->unk238;
                if (temp_a0 < actor->unk232)
                {
                    actor->unk23A |= (1 << temp_a0);
                }
            }
        }
    }
}

/**
 * decomp.me (100%) https://decomp.me/scratch/3KrRM
 */
void field_initialize_actor_slots(void)
{
    s32 var_a0;
    s32 var_a0_2;
    FieldActorState* new_var;
    volatile u8* var_v1_2;
    var_a0 = 0;
    do
    {
        D_80105AE0[var_a0].u.unk178 &= ~1;
        var_a0++;
    } while (var_a0 < 0xD);
    var_a0_2 = 0;
    new_var = g_field_actor_slots;
    var_v1_2 = ((u8*)new_var) + 0x238;
    do
    {
        var_v1_2[-5] = var_a0_2;
        var_a0_2++;
        var_v1_2[-0x213] = 0;
        var_v1_2[-0x214] = 0;
        *((volatile u16*)(var_v1_2 - 0x16)) = 0;
        *((volatile u16*)var_v1_2) = 0;
        var_v1_2 += 0x244;
    } while (var_a0_2 < 0x50);
}

/**
 * decomp.me (100%) https://decomp.me/scratch/0JqYo
 */
void field_clear_actor_slots(void)
{
    s32 var_a0 = 0;
    FieldActorState* var_v1 = g_field_actor_slots;

    do
    {
        var_a0 += 1;
        var_v1->unk24 = 0;
        var_v1->unk232 = 0;
        var_v1->unk23A = 0;
        var_v1->unk23B = 0;
        var_v1->unk27 = 0;
        var_v1->unk28 = 0;
        var_v1++;
    } while (var_a0 < 0x50);
}

/**
 * decomp.me (100%) https://decomp.me/scratch/BUS6C
 */
void field_start_actor_animation(s32 slot_index, int target_count, u8* targets)
{
    s32 i;
    s32 k;
    s32 j;
    s32 m;
    FieldActorState* temp_s0;
    u8* src;
    m = slot_index;
    temp_s0 = &g_field_actor_slots[m];
    temp_s0->unk28 = 0;
    temp_s0->unk27 = 0;
    if (temp_s0->unk25 == 0)
    {
        return;
    }
    temp_s0->unkC->unk14 &= 0x7F;
    temp_s0->unk23A = 0;
    temp_s0->unk23B = 1;
    temp_s0->unk229[0] = 0;
    temp_s0->unk232 = target_count;
    ((u8*)&temp_s0->u224.h.lo)[1] = 0;
    if (target_count != 0)
    {
        src = targets;
        j = 0;
        i = 0;
        if (target_count > 0)
        {
            do
            {
                temp_s0->unk229[j] = *src;
                j++;
                src += 4;
                i++;
            } while (i < target_count);
        }
        if (j != 0)
        {
            temp_s0->unk232 = j;
        }
        else
        {
            field_finalize_actor_animation(temp_s0);
            return;
        }
    }
    else
    {
        temp_s0->unk232 = 1;
        temp_s0->unk229[0] = 0xFF;
    }
    func_8006D21C(temp_s0);
    for (i = 8; i >= 0; i--)
    {
        temp_s0->unk1EC[i] = 0;
    }

    temp_s0->unk236 = 0;
    temp_s0->unk234 = 0;
    for (k = 0; k < temp_s0->unk25; k++)
    {
        temp_s0->unk0[k].unk32 = k;
        for (m = 0; m < 9; m++)
        {
            temp_s0->unk2B[k] = (temp_s0->unkCC[m][k] = (temp_s0->unk3B[m][k] = 0));
        }
    }

    field_dispatch_actor_audio_event(temp_s0, 1, 0);
    for (i = 0; i < temp_s0->unk25; i++)
    {
        temp_s0->unk0[i].unk32 = i;
        for (j = 0; j < 9; j++)
        {
            temp_s0->unk2B[i] = (temp_s0->unkCC[j][i] = (temp_s0->unk3B[j][i] = 0));
        }
    }
}

/**
 * decomp.me (100%) https://decomp.me/scratch/XDOcQ
 */
void field_dispatch_actor_audio_event(void* actor, s32 event_type, s32 event_subtype)
{
    u8* ptr;
    Struct_D800FDF58* global_table;
    s32 i;
    s32 idx;
    u8* table;
    u16 val;
    s32 temp;
    u8* new_var;
    Struct_D800FDF58* entry;
    u8 d;

    for (i = 0; i < 2; i++)
    {
        ptr = (u8*)actor + i;
        idx = i << 1;
        if (ptr[0x27] == 0)
        {
            table = *((u8**)((u8*)actor + 0x0C));
            if ((*((u16*)(idx + (u32)table + 4))) == event_type)
            {
                if (event_type != 1)
                {
                    if ((*((table + i) + 2)) != event_subtype)
                    {
                        continue;
                    }
                }
                temp = func_8006CE70(((u8*)actor)[0x228]);
                table = (new_var = *((u8**)((u8*)actor + 0x0C)));
                val = *((u16*)((table + idx) + 8));
                switch (val >> 10)
                {
                case 0:
                    func_800A3938(val & 0x3FF, temp);
                    break;

                case 1:
                    if (((u8*)actor)[0x228] < 2)
                    {
                        func_800A3A90(val & 0x3FF, temp, ((u8*)actor)[0x228]);
                    }
                    else
                    {
                        global_table = D_800FDF58;
                        entry = &global_table[((u8*)actor)[0x228]];
                        d = entry->unk3B;
                        if (((u32)(d - 3)) < 3)
                        {
                            if (D_800FF59C != 0)
                            {
                                func_800A39A8(val & 0x3FF, temp, 0, ((u8*)actor)[0x228]);
                            }
                            else
                            {
                                func_800A39A8(val & 0x3FF, temp, entry->unk3B - 3, ((u8*)actor)[0x228]);
                            }
                        }
                    }
                    break;

                case 2:
                    if ((val & 0x3FF) < 2)
                    {
                        s32* field = (s32*)(((val & 0x3FF) << 2) + (u32)actor + 0x1C);
                        func_800A3E10(*field, temp, ((u8*)actor)[0x228]);
                    }
                    break;
                }

                if (event_type == 5)
                {
                    ptr[0x27] |= 0x80;
                }
                ((u8*)actor + i)[0x27] |= 1;
            }
        }
    }
}

/**
 * decomp.me (100%) https://decomp.me/scratch/tXpD1
 */
s32 field_is_actor_animation_active(s32 slot_index)
{
    FieldActorState* temp_v1;

    temp_v1 = &g_field_actor_slots[slot_index];
    return (temp_v1->unk23A | temp_v1->unk23B) != 0;
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
 * decomp.me (100%) https://decomp.me/scratch/etUW8
 */
void field_update_actor_animations(void)
{
    Struct_801ED600* ptr_801ED600 = (Struct_801ED600*)0x801ED600;
    FieldActorStateWithTracks* var_s3;
    Struct_Unk28* var_s1;
    FieldActorAnimationDef* temp_v1_3;
    s32 var_s0;
    s32 var_s6;
    u8* new_var;
    s32 i;
    s32 tmp;
    u16 temp_a0_4;
    u32 div_num;
    u32 div_den;
    var_s3 = g_field_actor_slots;
    var_s6 = 0;
    ptr_801ED600->unk140 = 0U;
    ptr_801ED600->unk92 = 0U;
    do
    {
        var_s1 = &var_s3->unk28;
        if ((&var_s3->unk28)->unk1FA != 0)
        {
            if ((&var_s3->unk28)->unk213 != 0)
            {
                field_reset_actor_track_mask(var_s3);
            }
            if ((&var_s3->unk28)->unk212 != (var_s3->unkC->unk15 * 0))
            {
                func_8007100C(var_s3);
                i = var_s3->unkC->unk14;
                if (((unsigned char)i) == 2)
                {
                    if ((&var_s3->unk0[var_s3->unkC->unk15])->unk34 & 0x04000000)
                    {
                        for (var_s0 = 0; var_s0 < (&var_s3->unk28)->unk20A; var_s0++)
                        {
                            u16 unk1ec = (&var_s3->unk28)->unk1C4[var_s0];
                            if (((&var_s3->unk0[var_s3->unkC->unk15])->unk31 < unk1ec) &&
                                ((&var_s3->unk28)->unk1C4[var_s0] < ((&var_s3->unk0[var_s3->unkC->unk15])->unk31 + (&var_s3->unk0[var_s3->unkC->unk15])->unkD)))
                            {
                                g_field_track_index = var_s0;
                                func_80099A48(var_s3, &var_s3->unk0[var_s3->unkC->unk15]);
                            }
                        }
                    }
                    else if ((&var_s3->unk0[var_s3->unkC->unk15])->unk31 == 0xFF)
                    {
                        for (var_s0 = 0; var_s0 < (&var_s3->unk28)->unk20A; var_s0++)
                        {
                            if ((&var_s3->unk0[var_s3->unkC->unk15])->unkD > (&var_s3->unk28)->unk1C4[var_s0])
                            {
                                g_field_track_index = var_s0;
                                func_80099A48(var_s3, &var_s3->unk0[var_s3->unkC->unk15]);
                            }
                        }
                    }
                    else if (((&var_s3->unk0[var_s3->unkC->unk15])->unk31 < (&var_s3->unk28)->unk1C4[0]) &&
                             ((&var_s3->unk28)->unk1C4[0] < ((&var_s3->unk0[var_s3->unkC->unk15])->unk31 + (&var_s3->unk0[var_s3->unkC->unk15])->unkD)))
                    {
                        g_field_track_index = 0;
                        func_80099A48(var_s3, &var_s3->unk0[var_s3->unkC->unk15]);
                    }
                }
                temp_v1_3 = var_s3->unkC;
                temp_a0_4 = temp_v1_3->unkE;
                if ((temp_a0_4 & 0x8000) && (((u8*)temp_v1_3)[0xE] == (&var_s3->unk28)->unk1C4[0]))
                {
                    func_8005A67C((temp_a0_4 >> 8) & 0x7F, 0);
                }
                func_8006D270(var_s3);
                if (field_finalize_actor_animation(var_s3) == 0)
                {
                    for (i = 0; i < (&var_s3->unk28)->unk20A; i++)
                    {
                        if (((&var_s3->unk28)->unk212 >> i) & 1)
                        {
                            (&var_s3->unk28)->unk1C4[i] += 1;
                        }
                    }

                    (&var_s3->unk28)->unk20E += 1;
                    if (((&var_s3->unk28)->unk210 != 0) && ((&var_s3->unk28)->unk20A != 0))
                    {
                        div_num = var_s1->unk20E;
                        div_den = var_s1->unk210;
                        if ((div_num % div_den) == 0)
                        {
                            i = (div_num / div_den) & 0xFFFF;
                            if (i < (&var_s3->unk28)->unk20A)
                            {
                                (&var_s3->unk28)->unk212 |= 1 << i;
                            }
                        }
                    }
                }
                g_field_track_index = 0;
                for (var_s0 = 0; var_s0 < 2; var_s0++)
                {
                    u8 temp_a1 = (new_var = var_s3->unkC->unk0)[var_s0];
                    if ((new_var[var_s0] != 0xFF) && (temp_a1 < 0x10U))
                    {
                        if (var_s0 != 0)
                        {
                            u8 temp_v1_6 = field_evaluate_parameter_track((FieldAnimationData*)var_s3, temp_a1 & 0xF) | ptr_801ED600->unk92;
                            ptr_801ED600->unk92 = temp_v1_6;
                            ptr_801ED600->unk140 = temp_v1_6;
                        }
                        else
                        {
                            u8 temp_v0_2 = field_evaluate_parameter_track((FieldAnimationData*)var_s3, (tmp = new_var[0]) & 0xF);
                            ptr_801ED600->unk91 = (ptr_801ED600->unk13F = temp_v0_2);
                        }
                    }
                }
            }
            var_s3->unk27 &= 0xFE;
            (&var_s3->unk28)->unk0 &= 0xFE;
        }
        var_s6 += 1;
        var_s3 += 1;
    } while (var_s6 < 0x50);
}

/**
 * decomp.me (100%) https://decomp.me/scratch/VZWgF
 */
void field_reset_actor_track_mask(FieldActorTrackMaskState* actor)
{
    s32 i;
    u16 check238 = actor->unk238;

    // hack
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
 * decomp.me (100%) https://decomp.me/scratch/hvTSS
 */
void field_prepare_actor_render_commands(s32 render_context, s32 unused)
{
    func_80074D7C();
    field_build_actor_render_commands(render_context, unused);
}

/**
 * decomp.me (100%) https://decomp.me/scratch/Sgd61
 */
void field_build_actor_render_commands(void* render_context)
{
    u32 new_var;
    FieldRenderContext* arg0_1 = (FieldRenderContext*)render_context;
    s32 new_var4;
    u32* var_s7 = &arg0_1->unk40;
    u32* p_s4 = (u32*)arg0_1->unk40B8;
    FieldActorState* var_s2 = g_field_actor_slots;
    s32 var_s5 = 0;
    u32 sp_arr_word;
    int new_var6;
    int new_var3;
    int new_var2;
    do
    {
        new_var3 = var_s2->unk23A != 0;
        if (new_var3)
        {
            u16 temp_v1 = var_s2->unkC->unk18;
            if ((temp_v1 & 2) && (!(var_s2->unkC->unk18 & 8)))
            {
                u8 var_a1;
                s32 cond;
                g_field_track_index = 0;
                cond = field_evaluate_parameter_track(var_s2, (var_s2->unkC->unk18 >> 8) & 0xF);
                var_a1 = 0;
                if (cond != 0)
                {
                    var_a1 = 0xFE;
                }
                if (var_a1 != 0)
                {
                    D_800FDF58[var_s2->owner_object_index].unk25 = var_a1;
                    D_80105AE0[var_s2->owner_object_index].u.unk178 |= 1;
                    D_80105AE0[var_s2->owner_object_index].u.b.unk17A = var_s2->unk233;
                }
                else
                {
                    u8 temp_v1_2 = var_s2->owner_object_index;
                    s16 temp_a0 = D_800FDF58[temp_v1_2].unk2A;
                    if ((temp_a0 == 0x90) || (temp_a0 == 0x94))
                    {
                        if (D_80105AE0[temp_v1_2].unkC & 0x200)
                        {
                            D_800FDF58[var_s2->owner_object_index].unk25 = var_a1;
                        }
                    }
                    else
                    {
                        D_800FDF58[var_s2->owner_object_index].unk25 = var_a1;
                    }
                }
            }
            new_var2 = var_s2->unkC->unk18 & 4;
            if (new_var2 && (!(var_s2->unkC->unk18 & 0x10)))
            {
                s32 var_s1 = 0;
                if (var_s2->unk232 != 0)
                {
                    do
                    {
                        u8 var_a1_2;
                        s32 cond2;
                        g_field_track_index = var_s1;
                        cond2 = field_evaluate_parameter_track(var_s2, var_s2->unkC->unk18 >> 0xC);
                        var_a1_2 = 0;
                        if (cond2 != 0)
                        {
                            var_a1_2 = 0xFE;
                        }
                        if (var_s2->unk229[var_s1] != 0xFF)
                        {
                            if (var_a1_2 != 0)
                            {
                                D_800FDF58[var_s2->unk229[var_s1]].unk25 = var_a1_2;
                                do
                                {
                                    D_80105AE0[var_s2->unk229[var_s1]].u.unk178 |= 1;
                                    D_80105AE0[var_s2->unk229[var_s1]].u.b.unk17A = var_s2->unk233;
                                } while (0);
                                ((u8*)var_s2)[0x225] = 1;
                            }
                            else
                            {
                                new_var4 = var_s1;
                                D_800FDF58[var_s2->unk229[new_var4]].unk25 = 0;
                            }
                        }
                        var_s1++;
                    } while (var_s1 < var_s2->unk232);
                }
            }
        }
        var_s5++;
        var_s2++;
    } while (var_s5 < 0x50);
    g_field_track_index = 0;
    var_s2 = g_field_actor_slots;
    var_s5 = 0;
    do
    {
        if (var_s2->unk23A != 0)
        {
            FieldActorAnimationDef* temp_a0_2 = var_s2->unkC;
            if (temp_a0_2->unkC & 0x1000)
            {
                s32 new_var33 = temp_a0_2->unkC >> 0xD;
                switch (new_var33 & 3)
                {
                case 0:
                    D_800F2278 = field_evaluate_parameter_track(var_s2, temp_a0_2->unk10 >> 0xC);
                    break;

                case 1:
                    D_800F227C = field_evaluate_parameter_track(var_s2, temp_a0_2->unk10 >> 0xC);
                    break;

                case 2:
                    D_800F227C = (D_800F2278 = field_evaluate_parameter_track(var_s2, temp_a0_2->unk10 >> 0xC));
                    break;

                case 3:
                    D_800F2278 = field_evaluate_parameter_track(var_s2, temp_a0_2->unk10 >> 0xC);
                    D_800F227C = field_evaluate_parameter_track(var_s2, ((var_s2->unkC->unk10 >> 0xC) + 1) & 0xF);
                    break;
                }
            }
        }
        var_s5++;
        var_s2++;
    } while (var_s5 < 0x50);
    var_s2 = g_field_actor_slots;
    var_s5 = 0;
    do
    {
        new_var3 = 0x00FFFFFF;
        if ((var_s2->unk23A != 0) && (var_s2->unk24 != 0))
        {
            FieldActorAnimationDef* temp_v1_7 = var_s2->unkC;
            u8 temp_a1 = (u8)temp_v1_7->unkC;
            if (((u8)temp_v1_7->unkC) < 0x10)
            {
                if (((temp_v1_7->unkC >> 8) & 1) != 0)
                {
                    do
                    {
                        ((u8*)(&sp_arr_word))[0] = field_evaluate_parameter_track(var_s2, (u8)temp_v1_7->unkC);
                    } while (0);
                    ((u8*)(&sp_arr_word))[1] = field_evaluate_parameter_track(var_s2, (((u8)var_s2->unkC->unkC) + 1) & 0xF);
                    ((u8*)(&sp_arr_word))[2] = field_evaluate_parameter_track(var_s2, (((u8)var_s2->unkC->unkC) + 2) & 0xF);
                }
                else
                {
                    ((u8*)(&sp_arr_word))[0] = (((u8*)(&sp_arr_word))[1] = (((u8*)(&sp_arr_word))[2] = field_evaluate_parameter_track(var_s2, temp_a1 & 0xF)));
                }
                if ((var_s2->unkC->unkC >> 8) & 4)
                {
                    field_set_global_color_scale(((u8*)(&sp_arr_word))[0] * 2, ((u8*)(&sp_arr_word))[1] * 2, ((u8*)(&sp_arr_word))[2] * 2);
                }
                else
                {
                    u32 var_a0;
                    if ((var_s2->unkC->unkC >> 8) & 1)
                    {
                        if (((u8*)(&sp_arr_word))[0] == 0)
                        {
                            if (((u8*)(&sp_arr_word))[1] == 0)
                            {
                                if (((u8*)(&sp_arr_word))[2] == 0)
                                {
                                    goto block_59;
                                }
                            }
                        }
                    }
                    else if (((u8*)(&sp_arr_word))[0] == 0)
                    {
                        goto block_59;
                    }
                    var_a0 = 0xE1000005;
                    {
                        u8* p_s0 = ((u8*)p_s4) + 4;
                        new_var = sp_arr_word;
                        p_s0[-1] = 3;
                        *((u16*)(p_s0 + 8)) = 0x140;
                        *((u16*)(p_s0 + 0xA)) = 0xF0;
                        *((u16*)(p_s0 + 6)) = 0;
                        *((u16*)(p_s0 + 4)) = 0;
                        *((u32*)p_s0) = new_var;
                        p_s0[3] = 0x62;
                        p_s4[0] = (p_s4[0 ^ 0] & 0xFF000000) | ((*var_s7) & new_var3);
                        *var_s7 = ((*var_s7) & 0xFF000000) | (((u32)p_s4) & 0x00FFFFFF);
                        new_var6 = (((var_s2->unkC->unkC >> 9) & 3) + 1) & 3;
                        p_s4 += 4;
                        {
                            u8* p_s0_2 = ((u8*)p_s4) + 4;
                            p_s0_2[-1] = 1;
                            *((u32*)p_s0_2) = (new_var6 << 5) | var_a0;
                        }
                        var_a0 = 0x00FFFFFF;
                        p_s4[0] = (p_s4[0] & 0xFF000000) | ((*var_s7) & var_a0);
                        *var_s7 = ((*var_s7) & 0xFF000000) | (((u32)p_s4) & 0x00FFFFFF);
                        p_s4 += 2;
                    }
                }
            }
        }
    block_59:
        var_s5++;

        var_s2++;
    } while (var_s5 < 0x50);
    field_apply_global_color_scale();
    arg0_1->unk40B8 = p_s4;
}

/**
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
 * decomp.me (100%) https://decomp.me/scratch/iwb2J
 */
void field_set_global_color_scale(s16 red, s16 green, s16 blue)
{
    g_field_color_scale.s.red = red;
    g_field_color_scale.s.green = green;
    g_field_color_scale.s.blue = blue;
}

/**
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
        FieldColorScale* p = &g_field_color_scale; /* forces s0/s1 registers in else branch */
        if (p->w != 0x1000100UL || p->s.blue != 0x100)
        {
            func_8005A0D0(-1, p->s.red, p->s.green, p->s.blue);
            *active = 1;
        }
    }
}

/**
 * decomp.me (100%) https://decomp.me/scratch/a1YEc
 */
void func_8006A324(void)
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

extern void* bcopy(const void*, void*, int);

typedef struct
{
    u8* start; /* 0x00 */
    u8* end;   /* 0x04 */
    u8 unk8;   /* 0x08 */
    u8 slot_index; /* 0x09 */
    u8 padA[0xE - 0xA];
    s16 unkE; /* 0x0E */
    u32 flags; /* 0x10 */
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

extern u8* g_pad_ctx;

extern u8 g_field_resource_buffer[];
extern u16 D_800FDA80;
extern FieldActorPartDef D_800FE3A0[];
extern FieldActorAnimationDef D_800FE758;
extern s32 D_800FE774;
extern FieldResourceEntry g_field_resource_entries[];
extern void* g_field_resource_cursor;
extern s32 D_801158A0;

void field_relocate_resource_buffer(s32);
/*
 * func_8006C3FC is deliberately left implicitly declared: it is called with two
 * arguments by field_relocate_resource_buffer and with one by func_8006A9A4,
 * and both forms are required to match. A prototype makes the one-argument call
 * a hard error and costs field_relocate_resource_buffer 8 exact rows.
 *
 * field_restore_default_action_animation_mappings is likewise left implicitly
 * declared: field_initialize_actor_system calls it with one argument while the
 * definition below takes void, and both forms are required to match.
 */
s32 func_8006A88C(s32, D_800FD818_type*, s32);
void func_8006A9A4(s32, s32, s32, s32);
void func_8006B4D0(s32, s32);
void func_8006B7A0(s32, s32);
/*
 * func_8006CF88 is deliberately left implicitly declared: field_load_actor_slot
 * calls it with four arguments and func_8006AD04 with none, and both forms are
 * required to match. Same situation as func_8006C3FC above; see [TOOL-09].
 */
void func_80091438(s32);
void func_800A3D44(s32, u8);

/**
 * @brief Reset and re-initialize the field actor/voice state tables.
 * @note  Matching decompilation; the pointer arithmetic and forced addition
 *        ordering in the final loops are load-bearing for codegen.
 * @see decomp.me (100%) https://decomp.me/scratch/0wUsT
 */
void field_initialize_actor_system(void)
{
    s32 i;
    s32 j;
    unsigned int new_var4;
    int new_var2;
    u8* new_var;
    FieldActorAnimationDef* new_var3;
    int new_var5;
    FieldActorPartDef* ptr_D;
    u8* new_var7;
    FieldActorState* arg0_ptr;
    u8* ptr_a0;
    u8* ptr_a1;
    int new_var8;
    u8* new_var6;
    s32 temp_a2;
    u8* ptr_a3;
    u8* pad_base;
    int new_var9;
    u8* pad_ptr;
    u8* new_var10;
    u32 dest_addr; /* temporary for address computation */

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
                    new_var4 = func_8006A88C(2, &D_800FD818[2], 1);
                    i = new_var4;
                }
                else
                {
                    i = func_8006A88C(j, &D_800FD818[j], 0);
                }
                if (i != D_800FD818[j].unk254)
                {
                    D_800FD818[j].unk254 = i;
                    if (j == 2)
                    {
                        func_8006A9A4(2, 2, i, 1);
                    }
                    else
                    {
                        func_8006A9A4(j, j, i, 0);
                    }
                }
                else
                {
                    field_relocate_resource_buffer(j);
                    new_var2 = 2;
                    if (j == new_var2)
                    {
                        g_field_resource_entries[new_var2].flags = g_field_resource_entries[new_var2].flags | 1;
                    }
                }
                func_8006B4D0(j, j);

                pad_base = (u8*)g_pad_ctx;
                new_var8 = D_800FDF58[j].unk1C & (~0x1FF);
                pad_ptr = pad_base + (j * 0x250);
                D_800FDF58[j].unk1C = new_var8 | ((pad_ptr[0x608] >> 7) ^ 1);

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
                i = func_8006A88C(j, &D_800FD818[j], 0);
                if (i != D_800FD818[j].unk254)
                {
                    D_800FD818[j].unk254 = i;
                    func_8006A9A4(j, j, i, 0);
                }
                else
                {
                    field_relocate_resource_buffer(j);
                }
                func_8006B4D0(j, j);

                pad_base = (u8*)g_pad_ctx;
                new_var9 = D_800FDF58[j].unk1C & (~0x1FF);
                pad_ptr = pad_base + (j * 0x250);
                D_800FDF58[j].unk1C = new_var9 | ((pad_ptr[0x608] >> 7) ^ 1);
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
    new_var = (u8*)g_field_actor_slots;
    new_var3 = &D_800FE758;
    ptr_D = D_800FE3A0;
    j = 0x6CC0;

    for (; i < 0xD; i++)
    {
        arg0_ptr = (FieldActorState*)(((u32)j) + ((u32)new_var));
        arg0_ptr->unk0 = ptr_D;
        arg0_ptr->unkC = new_var3;
        func_8006B7A0(i, 0);
        ptr_D++;
        j += 0x244;
    }

    field_restore_default_action_animation_mappings(new_var5 = 0);

    for (i = 0; i < 0x10; i++)
    {
        j = 0;
        new_var7 = ((u8*)g_field_actor_slots) + i;
        ptr_a3 = new_var7 + 0xB327;
        temp_a2 = i * 2;
        new_var4 = 0xB3C8;

        {
            u8* base = (u8*)g_field_actor_slots;
            new_var10 = base;
            new_var6 = new_var10;
            ptr_a0 = new_var6;
            ptr_a1 = ptr_a0;
        }

        for (; j < 9; j++)
        {
            new_var6 = ptr_a1 + 0xB337;
            /* Force addition order: temp_a2 + (ptr_a0 + new_var4) */
            dest_addr = temp_a2;
            dest_addr += (u32)(ptr_a0 + new_var4);
            *((u16*)dest_addr) = (*(i + new_var6) = 0);
            ptr_a1 += 0x10;
            ptr_a0 += 0x20;
            *ptr_a3 = 0;
        }
    }

    func_8006CF88(ptr_a0, ptr_a1, temp_a2, ptr_a3);
}

/**
 * @brief TODO: relocate field entry buffer into the streaming window.
 *
 * Copies the entry's payload into the rolling destination at
 * g_field_resource_cursor,
 * repoints the entry's begin/end pointers at the new location, updates its
 * state flags, and hands the previous buffer to func_8006C3FC.
 *
 * @param resource_index Index into g_field_resource_entries / D_800FDF58.
 * @note WIP - not yet byte-matching. Currently 99.52%; the only residue is the
 *       a1/a2 load order in the first bcopy argument setup.
 * @see decomp.me (99.52%) https://decomp.me/scratch/iTv8i
 */
void field_relocate_resource_buffer(s32 resource_index)
{
    u32 new_var4;
    Struct_D800FDF58* new_var;
    u8* old_start;

    g_field_resource_cursor += 0;
    bcopy((void*)(((u32)0x80180000 - (u32)g_field_resource_buffer) + ((u32)g_field_resource_entries[resource_index].start)), g_field_resource_cursor,
          g_field_resource_entries[resource_index].end - g_field_resource_entries[resource_index].start);
    new_var4 = g_field_resource_entries[resource_index].end;
    old_start = g_field_resource_entries[resource_index].start;
    new_var4 -= (u32)old_start;
    g_field_resource_entries[resource_index].start = (u8*)g_field_resource_cursor;
    g_field_resource_entries[resource_index].end = ((u8*)g_field_resource_cursor) + new_var4;
    g_field_resource_entries[resource_index].flags &= ~1;
    g_field_resource_entries[resource_index].slot_index = resource_index;
    g_field_resource_entries[resource_index].flags |= 2;
    g_field_resource_cursor = g_field_resource_entries[resource_index].end;
    new_var = &D_800FDF58[resource_index];
    func_8006C3FC(new_var, old_start);
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
 * @see decomp.me (100%) TODO
 */
s32 func_8006A88C(s32 arg0, D_800FD818_type* entry, s32 arg2)
{
    if (arg2 != 0)
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
 * @see decomp.me (100%) TODO
 */
void func_8006A958(s32 arg0)
{
    s32 i;

    for (i = 0; i < 0xD; i++)
    {
        func_8006B7A0(i, arg0);
    }
}

/**
 * @see decomp.me (100%) TODO
 */
void func_8006A9A4(s32 resource_index, s32 slot_index, s32 arg2, s32 arg3)
{
    g_field_resource_entries[resource_index].unkE = 0x2F;
    g_field_resource_entries[resource_index].slot_index = slot_index;
    g_field_resource_entries[resource_index].unk8 = 0;
    g_field_resource_entries[resource_index].flags =
        (g_field_resource_entries[resource_index].flags & ~1) | (arg3 & 1);
    g_field_resource_entries[resource_index].start = g_field_resource_cursor;
    func_8006CAFC(arg2, slot_index, resource_index, arg3 & 1);
    D_800FDF58[slot_index].unk21 &= 0x80;
    func_8006C3FC(&D_800FDF58[slot_index]);
    g_field_resource_entries[resource_index].end = g_field_resource_cursor;
    g_field_resource_entries[resource_index].flags |= 2;
}

/**
 * @see decomp.me (100%) TODO
 */
void func_8006AA7C(s32 arg0)
{
    s32 i;
    u8* pad_base;
    u8* pad_ptr;
    u32 new_var;
    s32 temp;

    if (arg0 < 2)
    {
        func_800B08FC(0, arg0);
        if (func_800B0850() == 0)
        {
            pad_base = (u8*)g_pad_ctx;

            for (i = 0; i < 2; i++)
            {
                if (D_800FD818[i].u0.b.unk0 & 1)
                {
                    temp = ~0x1FF;
                    new_var = D_800FDF58[i].unk1C & temp;
                    temp = i * 0x250;
                    pad_ptr = pad_base + temp;
                    D_800FDF58[i].unk1C = new_var | ((pad_ptr[0x608] >> 7) ^ 1);
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
 * @brief Compact the resource buffer by removing entry @p arg0 + 1's payload.
 *
 * Copies everything above the entry down over it, then subtracts the removed
 * size from every resource entry and every live actor buffer pointer that sat
 * above it, and finally drops the cursor and clears the entry's in-use flag.
 *
 * @param arg0 Resource slot index minus one; the entry acted on is arg0 + 1.
 *
 * @see decomp.me (100%)
 */

void func_8006AB38(s32 arg0)
{
    s32 idx;
    s32 i;
    u8* src;
    u8* dst;
    s32 size;
    Struct_D800FDF58* p;
    Struct_D800FDF58* basep;
    Struct_D800FDF58* slot;
    FieldResourceEntry* entry;

    idx = arg0;
    idx += 1;

    if (D_800FD818[idx].u0.b.unk0 & 1)
    {
        entry = &g_field_resource_entries[idx];

        while (D_800FD818[idx].u0.b.unk0 & 1)
        {
            basep = D_800FDF58;
            slot = &basep[idx];
            break;
        }

        src = entry->end;
        size = src - entry->start;
        dst = entry->start;
        slot->unk25 = 0xFF;

        D_800FD818[idx].unk256 = 0xFF;

        D_800FD818[idx].u0.h &= 0xFFFE;

        while (src != g_field_resource_cursor)
        {
            *dst = *src;
            src++;
            dst++;
        }

        for (i = 0; i < 8; i++)
        {
            if (g_field_resource_entries[i].start > g_field_resource_entries[idx].start)
            {
                g_field_resource_entries[i].start -= size;
                g_field_resource_entries[i].end -= size;
            }
        }

        i = 0;
        p = D_800FDF58;
        do
        {
            if ((p->unk25 != 0xFF) && (p->unk3B != 8))
            {
                if ((p->unk40 | 0x80000000) >
                    (((u32)g_field_resource_entries[idx].start) | 0x80000000))
                {
                    p->unk40 -= size;
                }
            }

            i++;
            p++;
        } while (i < 0xD);

        g_field_resource_cursor = ((u8*)g_field_resource_cursor) - size;
        g_field_resource_entries[idx].flags &= ~2;
    }
}


typedef struct
{
    s16 unk0; /* 0x00 */
    s16 unk2; /* 0x02 */
} Struct_D800EB254;

extern Struct_D800EB254 D_800EB254[];
extern u8 D_800FDA81;

Struct_D800FDF58* func_80087C9C(s32);

/**
 * @brief Bind a resource to actor slot @p arg2 + 1 and bring the slot online.
 *
 * Marks the slot in use, records its source kind, derives the resource id for
 * that kind, loads it, seeds the actor's buffer pointers from either a cleared
 * state (arg0 == -1), the template at D_800FDF58[0] (arg0 == -2), or a located
 * donor slot, then resets the render state and notifies the audio side.
 *
 * @param arg0 Source selector: >= 0 looks the resource up via func_80087C9C,
 *             -1 clears, -2 copies the template.
 * @param arg1 Value stored at the slot's unk2 and used for the >= 0x41 check.
 * @param arg2 Slot index minus one.
 * @return 0 if the slot was already in use, -1 if the lookup failed, else 1.
 *
 * @note NOT MATCHED. Instruction count and frame are exact. Required to match,
 *       each measured by reverting it:
 *       - `id` is s32, not s16 (+7 exact rows);
 *       - the mid-function switch reads through a pointer scoped to the switch
 *         ALONE (+4); widening that pointer to cover the `unk2` store before it
 *         or the `unk254` store after it both measure far worse;
 *       - `arg1` is s32, not u8, or the 0x41 test emits andi+sltiu (+2);
 *       - D_800FDF58[2] goes through its own pointer or the +0xA8 and +0x1C
 *         fold into one displacement (+7);
 *       - D_800EB254 has a 4-byte stride read as s16, not an s16 array;
 *       - g_pad_ctx is loaded ONCE into a local and reused at the func_8009C2E0
 *         call, which is what brings the insn count to exactly 295;
 *       - the post-call unk1C update indexes D_800FDF58[slot] directly and
 *         assigns `pad` inside the expression (+7); an `entry` pointer there
 *         makes the arg0 == -2 arm's pointer a global pseudo and its index
 *         chain leaves s0;
 *       - in the lookup arm, unk21 is stored after unk8 (+2).
 *       Residue is 19 rows: the ~0x1FF constant is materialized early, and
 *       in the arg0 == -2 arm the template pointer takes v1 instead of a0 so
 *       the D_800EB254 lui cannot fill the unk8 load delay (+1 nop).
 *       See working/func_8006AD04/STATUS.md.
 * @see decomp.me (97.53%) TODO
 */
s32 func_8006AD04(s32 arg0, s32 arg1, s32 arg2)
{
    s32 slot = arg2 + 1;
    Struct_D800FDF58* entry;
    Struct_D800FDF58* found;
    s32 args[3];
    Struct_D800EB254* def;
    D_800FD818_type* s;
    Struct_D800FDF58* two;
    u8* pad;
    Struct_D800FDF58* first;
    s32 id;

    if (D_800FD818[slot].u0.b.unk0 & 1)
    {
        return 0;
    }

    if (arg0 >= 0)
    {
        found = func_80087C9C(arg0);
        if (found == (Struct_D800FDF58*)-1)
        {
            return -1;
        }
    }

    D_800FD818[slot].u0.h |= 1;
    if (arg0 == -2)
    {
        D_800FD818[slot].unk3 = 0;
    }
    else
    {
        D_800FD818[slot].unk3 = slot;
    }

    D_800FD818[slot].unk2 = arg1;

    s = &D_800FD818[slot];
    switch (s->unk3)
    {
    case 0:
        id = ((s->u0.h >> 1) & 1) + 0xAEB;
        break;

    case 1:
        id = s->unk2 + 0xAEE;
        break;

    case 2:
    default:
        id = s->unk2 + 0xB02;
        break;
    }

    D_800FD818[slot].unk254 = id;
    func_8006A9A4(slot, slot, id, 0);
    func_8006B4D0(slot, slot);

    D_800FDF58[slot].unk1C = (D_800FDF58[slot].unk1C & ~0x1FF) | (((pad = (u8*)g_pad_ctx)[(slot * 0x250) + 0x608] >> 7) ^ 1);

    if ((slot == 2) && (arg1 >= 0x41))
    {
        two = &D_800FDF58[2];
        two->unk1C = (two->unk1C & 0xFFFCFFFF) | (((pad[0x29D7] + 1) & 3) << 16);
    }
    else
    {
        D_800FDF58[slot].unk1C &= 0xFFFCFFFF;
    }

    if (arg0 == -1)
    {
        D_800FDF58[slot].unk0 = 0;
        D_800FDF58[slot].unk4 = 0;
        D_800FDF58[slot].unk8 = 0;
        D_800FDF58[slot].unk21 = 0;
    }
    else if (arg0 == -2)
    {
        first = D_800FDF58;
        entry = &D_800FDF58[slot];
        entry->unk0 = first->unk0;
        entry->unk4 = first->unk4;
        entry->unk8 = first->unk8;
        def = &D_800EB254[first->unk1B >> 5];
        args[0] = def->unk0;
        args[1] = 0;
        args[2] = def->unk0;
        func_8009C2E0(entry, args, pad);
        entry->unk21 = 0;
    }
    else
    {
        D_800FDF58[slot].unk0 = found->unk0;
        D_800FDF58[slot].unk4 = found->unk4;
        D_800FDF58[slot].unk8 = found->unk8;
        D_800FDF58[slot].unk21 = 0;
        found->unk25 = 0xFF;
        D_800FE774--;
    }

    D_800FDF58[slot].unk25 = 0;
    D_800FDF58[slot].unk2A = 0;
    D_800FDF58[slot].unk10 = 0;
    D_800FDF58[slot].unk28 = 0xFF;
    func_8006C3FC(&D_800FDF58[slot]);
    func_800AA90C(0);
    func_8006CF88();

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

/* ==== merged from field6.c (runtime/actor-resource TU) ==== */

/**
 * @brief Find the resource entry slot matching arg0, or the first free slot
 *        if none matches, then hand it off to func_8006B240.
 * @param arg0 Resource slot identifier to search for.
 * @param arg1 Passed through unchanged to func_8006B240.
 * @see decomp.me (100%) TODO
 */
void func_8006B1A0(s32 arg0, s32 arg1)
{
    s32 i;

    for (i = 0; i < 9; i++)
    {
        if (((g_field_resource_entries[i].flags >> 1) & 1) && g_field_resource_entries[i].slot_index == arg0)
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

    func_8006B240(arg0, arg1, i);
}



extern u8 D_800FDA83;
extern u8 D_800FDCEA;
extern s32 D_800FE754;
extern s32 D_80122710;
extern s32 D_80122714;
extern s32 D_80122B20;
extern Struct_D800FDF58 D_800FF658[];
extern s32 D_80105770;
extern s32 D_800F22A0;
extern s32 D_800F22A4;
extern s32 D_800F22A8;


/**
 * @brief Initialize resource entry arg2 for slot arg0, then notify every
 *        D_800FDF58 record that references it via func_8006C3FC.
 * @param arg0 Resource slot identifier, stored into the entry.
 * @param arg1 Base pointer; func_8006CAFC's first argument is arg1 + 0xB52.
 * @param arg2 Resource entry index to (re)initialize.
 * @see decomp.me (100%) TODO
 */
void func_8006B240(s32 arg0, u8 *arg1, s32 arg2)
{
    s32 i;
    FieldResourceEntry *entry;
    FieldResourceEntry *base;

    func_8006B354(arg2);

    base = g_field_resource_entries;
    entry = &base[arg2];
    entry->slot_index = arg0;
    entry->unk8 = 0;
    func_8009C434();
    entry->unkE = 0;
    entry->flags &= ~1;
    entry->start = g_field_resource_cursor;
    func_8006CAFC(arg1 + 0xB52, arg0, arg2);
    entry->end = g_field_resource_cursor;
    entry->flags |= 2;

    {
        Struct_D800FDF58 *rec;
        u8 *info;

        rec = D_800FDF58;
        info = (u8*)D_800FDF58 + 0x3B;
        i = 0;
        while (i < 0xD)
        {
            if (info[-0x16] != 0xFF && info[0] == arg2)
            {
                func_8006C3FC(rec);
            }
            i++;
            info += 0x54;
            rec++;
        }
    }
}

/**
 * @brief If resource entry arg0 is in use, compact its buffer region out of
 *        g_field_resource_cursor's arena and shift every later entry and
 *        D_800FDF58 record down by the freed size.
 * @param arg0 Resource entry index to release.
 * @see decomp.me (100%) TODO
 */
void func_8006B354(s32 arg0)
{
    s32 i;
    u8 *src;
    u8 *dst;
    s32 size;
    Struct_D800FDF58 *p;

    if ((g_field_resource_entries[arg0].flags >> 1) & 1)
    {
        src = g_field_resource_entries[arg0].end;
        size = src - g_field_resource_entries[arg0].start;
        dst = g_field_resource_entries[arg0].start;

        while (src != g_field_resource_cursor)
        {
            *dst = *src;
            src++;
            dst++;
        }

        for (i = 0; i < 8; i++)
        {
            if (((g_field_resource_entries[i].flags >> 1) & 1) && g_field_resource_entries[i].start > g_field_resource_entries[arg0].start)
            {
                g_field_resource_entries[i].start -= size;
                g_field_resource_entries[i].end -= size;
            }
        }

        p = D_800FDF58;
        for (i = 0; i < 0xD; i++)
        {
            if ((p->unk25 != 0xFF) && (p->unk3B != 8))
            {
                if ((p->unk40 | 0x80000000) > (((u32)g_field_resource_entries[arg0].start) | 0x80000000))
                {
                    p->unk40 -= size;
                }
            }
            p++;
        }

        g_field_resource_cursor = ((u8*)g_field_resource_cursor) - size;
    }
}

/**
 * @brief Reset D_800FDF58/D_80105AE0 entry arg0 to its default state for a
 *        newly-created field object tied to resource slot arg1, applying a
 *        few slot-specific special cases (slot 1's palette flag, slot 2's
 *        pad-timer flag).
 * @param arg0 D_800FDF58/D_80105AE0 entry index to (re)initialize.
 * @param arg1 Resource slot index whose g_field_resource_entries slot_index
 *        is copied into the new entry's unkC.
 * @see decomp.me (97.06%) TODO
 * @note NOT MATCHED. Instruction count is exact (180). Required to match,
 *       each measured by reverting it:
 *       - the loop counter i is u32 (else gcc reverses the zero-fill loop);
 *       - the three masks are separate locals assigned after the loop;
 *       - the unk10/12/14 and unk44/48/4C groups are cleared through a
 *         pointer to the first member (the target forms the group address
 *         as a value, `addiu v0, base, 0x10; addu`);
 *       - unk178 is cleared with two read-modify-write statements; reusing
 *         one `flags` variable for both the unk178 and unk1C chains gives
 *         the pseudo two deaths, which pushes it out of local-alloc;
 *       - unkC is stored before unk3B/unk21, and 0x80 is a literal;
 *       - the arg0==1 / arg0==2 cases use an explicit pointer to the
 *         constant-indexed entry.
 *       Residue (11 rows) is the unk1C chain landing in a0 instead of v1:
 *       our sched1 hoists its load above the unk22/unk28 stores while the
 *       target keeps it in place, so the small constants take v1 first.
 *       Measured inert: every position of the `flags` load statement,
 *       compound vs two-step forms, a variable for 0xFFFB0000.
 */
void func_8006B4D0(s32 arg0, s32 arg1)
{
    u32 i;
    u8 *p;
    s32 flags;
    s32 mask_a;
    s32 mask_b;
    s32 mask_c;
    s16 *q16;
    u32 *q32;

    p = (u8*)&D_800FDF58[arg0];
    i = 0;
    do
    {
        *p = 0;
        i++;
        p++;
    } while (i < 0x54);

    mask_a = 0xFFFF7FFF;
    mask_b = 0xEFFFFFFF;
    mask_c = 0xFFFBFFFF;

    D_80105AE0[arg0].unk19C = -1;
    D_80105AE0[arg0].unk1A0 = 0;
    D_80105AE0[arg0].unk18E = 0;
    D_80105AE0[arg0].u.unk178 &= ~0x80;
    D_80105AE0[arg0].u.unk178 &= ~0x40;

    D_800FDF58[arg0].unk22 = (s8)(arg0 + 0x30);
    D_800FDF58[arg0].unk28 = 0xFF;
    flags = D_800FDF58[arg0].unk1C;
    D_800FDF58[arg0].unk3A = arg0;
    D_800FDF58[arg0].unk24 = 0;
    D_800FDF58[arg0].unk25 = 0;
    D_800FDF58[arg0].unk27 = 0;
    D_800FDF58[arg0].unk2A = 0;
    D_800FDF58[arg0].unk2C = 0;
    D_800FDF58[arg0].unk2E = 0;
    D_800FDF58[arg0].unk30 = 0;
    D_800FDF58[arg0].unk32 = 0;
    D_800FDF58[arg0].unk33 = 0;
    D_800FDF58[arg0].unk4 = 0;
    D_800FDF58[arg0].unk8 = 0;
    flags = flags & ~0x1FF;
    flags = flags | 2;
    D_800FDF58[arg0].unk1C = flags;
    D_800FDF58[arg0].unk0 = 0xFFFB0000;
    D_800FDF58[arg0].unk1C = flags & mask_a & mask_b & mask_c;

    if (arg0 == 1 && D_800FDA83 == 0)
    {
        Struct_D800FDF58 *entry1 = &D_800FDF58[1];
        entry1->unk1C = (entry1->unk1C & 0xFF87FFFF) | 0x500000;
    }
    else
    {
        D_800FDF58[arg0].unk1C &= 0xFF87FFFF;
    }

    D_800FDF58[arg0].unk1C &= ~0x800;
    D_800FDF58[arg0].unkC = g_field_resource_entries[arg1].slot_index;
    D_800FDF58[arg0].unk3B = arg1;
    D_800FDF58[arg0].unk21 = 0;
    q16 = &D_800FDF58[arg0].unk10;
    q16[0] = 0;
    q16[1] = 0;
    q16[2] = 0;
    D_800FDF58[arg0].unk16 = 1;
    D_800FDF58[arg0].unk34 = 0;
    q32 = &D_800FDF58[arg0].unk44;
    q32[0] = 0;
    q32[1] = 0;
    q32[2] = 0;
    D_800FDF58[arg0].unk1A = 0x80;
    D_800FDF58[arg0].unk19 = 0x80;
    D_800FDF58[arg0].unk18 = 0x80;

    if (arg0 == 2 && D_800FDCEA >= 0x41)
    {
        Struct_D800FDF58 *entry2 = &D_800FDF58[2];
        entry2->unk1C = (entry2->unk1C & 0xFFFCFFFF) | (((g_pad_ctx[0x29D7] + 1) & 3) << 16);
        return;
    }

    D_800FDF58[arg0].unk1C &= 0xFFFCFFFF;
}





extern s32 D_800F2298;
extern s32 D_8012269C;
extern s32 D_80105760;

/**
 * @brief Zero-fill D_800FE3A0 entry arg0, then set it up as a default field
 *        actor part: fade timers, kind 8, a size/idle-timer sized by arg1,
 *        and a couple of render/state flag bits.
 * @param arg0 D_800FE3A0 entry index to (re)initialize.
 * @param arg1 Selects the unk2E/unk33 timer value (0x40 if zero, else 0x30).
 * @see decomp.me (88.23%) TODO
 * @note NOT MATCHED. Instruction count is exact (79). Required to match,
 *       each measured by reverting it:
 *       - every field access goes through D_800FE3A0[arg0] (no entry
 *         pointer): cse folds the address within each block and the target
 *         re-derives it after the if/else join;
 *       - the arg1 test is a real two-arm if/else that stores unk2E/unk33
 *         in BOTH arms (the tails cross-jump); a `val` temp compiles to the
 *         preload form and loses the join label;
 *       - the 0x16 halfword and 0x24/0x25 bytes are union members, not
 *         casts, or the address is re-derived per access;
 *       - `i = 0x12` is assigned before the loop pointer.
 *       Residue (16 rows) is local-alloc coloring in the two flag-word
 *       regions: the target hoists `lw unk4` above the `= 8` stores so the
 *       constant 8 takes v1, and keeps the unk14 chain in a1 with the ~0xF0
 *       mask in v0. Measured inert: loading the words into locals early,
 *       statement/operand order of the RMW expressions, reusing `i` or `arg1`
 *       for constants, do/while(0) wrappers. See working/func_8006B7A0/.
 */
void func_8006B7A0(s32 arg0, s32 arg1)
{
    s32 i;
    u32 *p;

    i = 0x12;
    p = (u32*)&D_800FE3A0[arg0];
    do
    {
        *p = 0;
        i--;
        p++;
    } while (i != 0);

    D_800FE3A0[arg0].unk10 = 0x80;
    D_800FE3A0[arg0].unkF = 0x80;
    D_800FE3A0[arg0].unkE = 0x80;
    D_800FE3A0[arg0].unk8 = 1;
    D_800FE3A0[arg0].unk9 = 0xFF;
    D_800FE3A0[arg0].u14.h.hi = 0x14;
    D_800FE3A0[arg0].unkD = 8;
    D_800FE3A0[arg0].u24.b.unk25 = 8;
    D_800FE3A0[arg0].u24.b.unk24 = 8;
    D_800FE3A0[arg0].unk23 = 8;
    D_800FE3A0[arg0].unk18 = 0x100;
    D_800FE3A0[arg0].unk4 = (D_800FE3A0[arg0].unk4 | 0x800) & 0xFF3FFFFF;
    D_800FE3A0[arg0].unk4 |= 0x400000;
    D_800FE3A0[arg0].unk0 = D_800FE3A0[arg0].unk0 & 0xFCFFFFFF;
    D_800FE3A0[arg0].unk0 |= 0x2000000;

    if (arg1 != 0)
    {
        D_800FE3A0[arg0].unk2E = 0x30;
        D_800FE3A0[arg0].unk33 = 0x30;
    }
    else
    {
        D_800FE3A0[arg0].unk2E = 0x40;
        D_800FE3A0[arg0].unk33 = 0x40;
    }

    D_800FE3A0[arg0].unk11 = 0xFF;
    D_800FE3A0[arg0].u14.w = (D_800FE3A0[arg0].u14.w & ~0xF0) | 0x20;
    D_800FE3A0[arg0].unk28 |= 0x2000000;
    D_800FE3A0[arg0].u24.w |= 0x100000;
}

/**
 * @brief Broadcast a shared render/config state to every field actor slot:
 *        stores arg0-arg2 into all 13 D_80105AE0 and D_800FE3A0 entries, folds
 *        arg4 into the 2-bit field at bits 22-23 of each part's unk4, and folds
 *        arg3's low bit into bit 23 of each D_800FDF58 record's unk1C.
 * @param arg0 Byte written to unk1A8 (D_80105AE0) and unkE (D_800FE3A0).
 * @param arg1 Byte written to unk1A9 (D_80105AE0) and unkF (D_800FE3A0).
 * @param arg2 Byte written to unk1AA (D_80105AE0) and unk10 (D_800FE3A0).
 * @param arg3 Low bit replaces bit 23 of every D_800FDF58 entry's unk1C.
 * @param arg4 Low 2 bits replace bits 22-23 of every D_800FE3A0 entry's unk4.
 * @see decomp.me (100%) TODO
 */
void func_8006B8DC(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4)
{
    s32 i;
    u32 mode;
    u32 flag;

    mode = (arg4 & 3) << 22;
    for (i = 0; i < 13; i++)
    {
        D_80105AE0[i].unk1A8 = arg0;
        D_80105AE0[i].unk1A9 = arg1;
        D_80105AE0[i].unk1AA = arg2;
        D_800FE3A0[i].unkE = arg0;
        D_800FE3A0[i].unkF = arg1;
        D_800FE3A0[i].unk10 = arg2;
        D_800FE3A0[i].unk4 = (D_800FE3A0[i].unk4 & 0xFF3FFFFF) | mode;
    }

    for (i = 0; i < 13; i++)
    {
        flag = (arg3 & 1) << 23;
        D_800FDF58[i].unk1C = (D_800FDF58[i].unk1C & 0xFF7FFFFF) | flag;
    }
}

Struct_D800FDF58* func_80087C9C(s32);

/**
 * @brief Apply a shared render/config state to the single field actor slot that
 *        owns record arg5: stores arg0-arg2 into that slot's D_80105AE0 and
 *        D_800FE3A0 entries, folds arg4 into the 2-bit field at bits 22-23 of
 *        the part's unk4, and arg3's low bit into bit 23 of the record's unk1C.
 * @param arg0 Byte written to unk1A8 (D_80105AE0) and unkE (D_800FE3A0).
 * @param arg1 Byte written to unk1A9 (D_80105AE0) and unkF (D_800FE3A0).
 * @param arg2 Byte written to unk1AA (D_80105AE0) and unk10 (D_800FE3A0).
 * @param arg3 Low bit replaces bit 23 of the record's unk1C.
 * @param arg4 Low 2 bits replace bits 22-23 of the part's unk4.
 * @param arg5 Record selector passed to func_80087C9C.
 * @return 0 on success, -1 if func_80087C9C found no record.
 * @see decomp.me (100%) TODO
 */
s32 func_8006B984(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5)
{
    Struct_D800FDF58 *rec;

    rec = func_80087C9C(arg5);
    if (rec == (Struct_D800FDF58*)-1)
    {
        return -1;
    }

    D_80105AE0[rec->unk3A].unk1A8 = arg0;
    D_80105AE0[rec->unk3A].unk1A9 = arg1;
    D_80105AE0[rec->unk3A].unk1AA = arg2;
    D_800FE3A0[rec->unk3A].unkE = arg0;
    D_800FE3A0[rec->unk3A].unkF = arg1;
    D_800FE3A0[rec->unk3A].unk10 = arg2;
    D_800FE3A0[rec->unk3A].unk4 = (D_800FE3A0[rec->unk3A].unk4 & 0xFF3FFFFF) | ((arg4 & 3) << 22);
    rec->unk1C = (rec->unk1C & 0xFF7FFFFF) | ((arg3 & 1) << 23);
    return 0;
}

typedef struct
{
    s16 x;
    s16 y;
    s16 w;
    s16 h;
} RECT;

/** @brief Staging buffer that field CD reads land in. */
typedef struct
{
    u32 unk0;  /* 0x00 */
    u32 unk4;  /* 0x04 */
    s32 unk8;  /* 0x08 byte offset of the second image inside the data area */
    u32 unkC;  /* 0x0C */
    u16 unk10; /* 0x10 */
    u16 unk12; /* 0x12 */
    u16 unk14; /* 0x14 start of the image data */
} FieldCdBuffer;

extern FieldCdBuffer* D_8010D038;

/*
 * Per-element structure (stride 0x268). D_800FD818 is a 3-element array; the
 * absolute offsets previously used by func_8006A324 (0x268, 0x4BC, 0x4D0,
 * 0x724, ...) are elements [1] and [2] of this array.
 */

extern u8 g_prim_rect_buf[];

/**
 * @brief Read a field texture set off the CD and upload it to VRAM: one
 *        optional full/partial strip at y = arg4 + 0x1F4, then the tile block
 *        for the current slot.
 * @param arg0 CD resource index (masked to 16 bits) to queue.
 * @param arg1 Slot index; < 2 selects the 0x380-based tile column.
 * @param arg2 Column index scaled by 0x40 into the tile block's x.
 * @param arg3 Selects the narrow (0x40-wide / 0x80-tall) variants.
 * @param arg4 Base row added to 0x1F4 for the first upload's y.
 * @see decomp.me (100%) TODO
 */
void func_8006BAF8(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4)
{
    RECT rect;
    FieldCdBuffer* buf;
    s32 second;
    s32 full_width;

    buf = D_8010D038;
    full_width = 0x100;
    cdrom_queue_read(arg0 & 0xFFFF, buf);
    cdrom_wait_queue_empty();
    second = buf->unk8;
    buf->unk14 = 0;

    if (arg2 == 0 || arg3 != 0)
    {
        if (arg3 != 0)
        {
            rect.x = 0xC0;
            rect.y = arg4 + 0x1F4;
            rect.w = 0x40;
            rect.h = 1;
        }
        else
        {
            rect.y = arg4 + 0x1F4;
            rect.w = full_width;
            rect.x = 0;
            rect.h = 1;
        }
        LoadImage(&rect, &buf->unk14);
    }

    if (arg1 >= 2)
    {
        s32 base = (arg2 << 6) + 0x340;
        s32 off = arg1 << 6;
        rect.x = base - off;
        rect.w = 0x40;
        rect.y = 0;
        rect.h = 0x100;
    }
    else if (arg3 != 0)
    {
        s32 base = (arg2 << 6) + 0x380;
        rect.x = base - (arg1 << 7);
        rect.y = 0x80;
        rect.w = 0x40;
        rect.h = 0x80;
    }
    else
    {
        s32 base = (arg2 << 6) + 0x380;
        s32 off = arg1 << 7;
        rect.x = base - off;
        rect.w = 0x40;
        rect.y = 0;
        rect.h = 0x100;
    }

    LoadImage(&rect, (u8*)(second + (s32)buf + 0x14));
    DrawSync(0);
}

/**
 * @see decomp.me (100%) TODO
 */
void func_8006BC50(void)
{
    Struct_D800FDF58 *rec;
    Struct_D80105AE0 *ent;
    s32 i;
    s32 mode;
    s32 count;
    s32 index;

    rec = &D_800FDF58[0];
    ent = &D_80105AE0[0];

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
        if (rec->unk25 != 0xFF)
        {
            if (!(rec->unk1C & 0x1FF) && count <= 0)
            {
                count++;
            }

            if (!(ent->unkC & 0x2000))
            {
                rec->unk40 = func_8006C168(rec);
            }
            else
            {
                rec->unk3C |= 0x1000000;
            }

            mode = ent->unk10 & 0xF;
            if (D_800FE754 == mode || mode == 0)
            {
                func_80086494(i);
                if (!(ent->u.unk178 & 0x81))
                {
                    if (D_800F229C == 0 && g_field_return_to_title_prompt_state == 0)
                    {
                        func_800880EC(rec);
                    }

                    mode = rec->unk1C & 0x1FF;
                    if (mode == 0)
                    {
                        if (D_80122714 == 0 && D_80122710 == 0 && D_800F229C == 0)
                        {
                            if (!(ent->unkC & 0x21E4))
                            {
                                func_8008DC54(rec, count);
                            }
                            else
                            {
                                func_8006BFC4(rec);
                            }
                        }
                    }
                    else if (mode == 1)
                    {
                        if (D_800F229C == 0 && g_field_return_to_title_prompt_state == 0)
                        {
                            if (!(ent->unkC & 0x21E4))
                            {
                                func_8008D29C(rec, index, 0x600 + (index * 0x400));
                                rec->unk1C &= ~0x800;
                            }
                            else
                            {
                                func_8006BFC4(rec);
                            }
                        }
                        index++;
                    }
                    else if (mode == 2)
                    {
                        if (g_field_return_to_title_prompt_state == 0 && D_800F229C == 0 && D_80122B20 == 0)
                        {
                            if (rec->unk2A != 0x93 && rec->unk2A != 0x94 && rec->unk2A != 0x90 &&
                                rec->unk2A != 0xAE && rec->unk2A != 0x8E && rec->unk2A != 0xB8)
                            {
                                func_80087564(rec);
                            }
                        }

                        if (!(ent->unkC & 0x21E4))
                        {
                            func_8008EF0C(rec);
                            if (g_field_return_to_title_prompt_state == 0 && D_800F229C == 0)
                            {
                                rec->unk1C |= 0x800;
                            }
                        }
                        else
                        {
                            func_8006BFC4(rec);
                        }
                    }

                    func_8008D174(rec);
                }
            }
        }

        i++;
        rec++;
        ent++;
    } while (i < 13);

    func_80091D7C();
}

/**
 * @see decomp.me (100%) TODO
 */
void func_8006BFC4(Struct_D800FDF58 *rec)
{
    if (rec->unk4 < 0)
    {
        rec->unk4 += 0x800;
        if (rec->unk4 > 0)
        {
            rec->unk4 = 0;
        }
    }
}


/**
 * @see decomp.me (100%) TODO
 */
void func_8006BFE8(FieldRenderContext *ctx)
{
    Struct_D800FDF58 *rec;
    Struct_D80105AE0 *ent;
    u32 *base;
    s32 cursor;
    s32 i;

    rec = &D_800FDF58[0];
    base = &ctx->unk40;
    i = 0;
    ent = &D_80105AE0[0];
    cursor = ctx->unk40B8;

    do
    {
        if (rec->unk25 != 0xFE && rec->unk25 != 0xFF)
        {
            if (rec->unk40 >= 0)
            {
                cursor = func_80077FB4(rec, cursor, base, rec->unk40, 0, &D_800FE3A0[i]);
            }
            else
            {
                cursor = func_80075C88(rec, cursor, base, rec->unk40, 0, &D_800FE3A0[i]);
            }
        }
        else if (rec->unk25 == 0xFE)
        {
            ent->unk12C = 0x100000;
            ent->unk140 = -8;
            ent->unk142 = -0xF;
            ent->unk144 = 8;
            ent->unk146 = 0;
        }
        else
        {
            ent->unk12C = 0;
        }

        i++;
        rec++;
        ent++;
    } while (i < 13);

    ctx->unk40B8 = cursor;
}

/**
 * @see decomp.me (100%) TODO
 */
void func_8006C11C(u16 arg0)
{
    s32 size;

    size = cdrom_queue_read(arg0);
    cdrom_wait_queue_empty();
    g_field_resource_cursor += (size + 3) & ~3;
}

/**
 * @see decomp.me (100%) TODO
 */
u8 *func_8006C168(Struct_D800FDF58 *rec)
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

    base = g_field_resource_entries[rec->unk3B].start;
    p = base + 4;
    mode = p[1] >> 1;
    wrap = p[1] & 1;
    if (mode & 0x40)
    {
        mode -= 0x40;
    }

    seq = rec->unk21;
    idx = seq & 0x7F;
    if (idx >= (s32)base[4])
    {
        p = base + 6;
        rec->unk21 = seq & 0x80;
    }
    else
    {
        p = p + (idx * 2 + 2);
    }

    off = p[0];
    hi = p[1];
    p = base + off + ((hi & 0x7F) << 8);
    shift = (hi >> 7) + 1;
    rec->unk3C |= 0x1000000;

    if (rec->unk24 != 0)
    {
        rec->unk16--;
        rec->unk34++;
    }

    if (rec->unk16 == 0 && rec->unk24 != 0)
    {
        cursor = rec->unk27 + 1;
        rec->unk27 = cursor;
        if (cursor >= p[0])
        {
            if (rec->unk2E == 0 || --rec->unk2E == 0)
            {
                if (rec->unk1C & 0x800)
                {
                    rec->unk16 = 1;
                    rec->unk34 = 1;
                    rec->unk36 = 0;
                    rec->unk27--;
                    return (u8 *)rec->unk40;
                }
            }
            rec->unk27 = 0;
        }

        pos = rec->unk27;
        rec->unk3C &= 0xFEFFFFFF;
        at_end = (pos + 1) >= (s32)p[0];
        p = p + ((pos << shift) + 1);
        dur = p[1];
        rec->unk16 = dur;
        rec->unk35 = dur;
        if (rec->unk16 == 0)
        {
            rec->unk16++;
            rec->unk35++;
        }
        rec->unk34 = 0;
        if (shift == 2)
        {
            rec->unk37 = p[2];
            rec->unk36 = p[3];
            if (at_end)
            {
                rec->unk38 = rec->unk37;
            }
            else
            {
                rec->unk38 = p[6];
            }
        }
        else
        {
            rec->unk38 = 0;
            rec->unk37 = 0;
            rec->unk36 = mode;
        }
    }
    else
    {
        p = p + ((rec->unk27 << shift) + 1);
    }

    q = base + base[2] + (base[3] << 8) + (p[0] * 2 + 2);
    if (wrap)
    {
        return (u8 *)((s32)(base + q[0] + (q[1] << 8)) & 0x7FFFFFFF);
    }
    return base + q[0] + (q[1] << 8);
}

/**
 * @see decomp.me (100%) TODO
 */
void func_8006C3FC(Struct_D800FDF58 *rec)
{
    rec->unk27 = 0;
    rec->unk1C &= ~0x800;
    rec->unk40 = func_8006C460(rec, g_field_resource_entries[rec->unk3B].start);
}

/**
 * @see decomp.me (100%) TODO
 */
u8 *func_8006C460(Struct_D800FDF58 *rec, u8 *base)
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

    p = base + 4;
    mode = p[1] >> 1;
    wrap = p[1] & 1;
    if (mode & 0x40)
    {
        mode -= 0x40;
    }

    seq = rec->unk21;
    idx = seq & 0x7F;
    if (idx >= (s32)base[4])
    {
        p = base + 6;
        rec->unk21 = seq & 0x80;
    }
    else
    {
        p = p + (idx * 2 + 2);
    }

    off = p[0];
    hi = p[1];
    p = base + off + ((hi & 0x7F) << 8);
    shift = (hi >> 7) + 1;

    at_end = (rec->unk27 + 1) >= (s32)p[0];
    p = p + 1;
    dur = p[1] * rec->unk24;
    rec->unk16 = dur;
    rec->unk35 = dur;
    if (rec->unk16 == 0)
    {
        rec->unk16++;
        rec->unk35++;
    }
    rec->unk34 = 0;
    if (shift == 2)
    {
        rec->unk37 = p[2];
        rec->unk36 = p[3];
        if (at_end)
        {
            rec->unk38 = 0;
        }
        else
        {
            rec->unk38 = p[6];
        }
    }
    else
    {
        rec->unk38 = 0;
        rec->unk37 = 0;
        rec->unk36 = mode;
    }

    if (rec->unk35 == 0)
    {
        rec->unk35 = 1;
    }
    rec->unk3C &= 0xFEFFFFFF;

    q = base + base[2] + (base[3] << 8) + (p[0] * 2 + 2);
    if (wrap)
    {
        return (u8 *)((s32)(base + q[0] + (q[1] << 8)) & 0x7FFFFFFF);
    }
    return base + q[0] + (q[1] << 8);
}

/**
 * @see decomp.me (100%) TODO
 */
void func_8006C5FC(Struct_D800FDF58 *rec)
{
    rec->unk1C |= 0x800;
    rec->unk40 = func_8006C658(rec, g_field_resource_entries[rec->unk3B].start);
}

/**
 * @see decomp.me (100%) TODO
 */
u8 *func_8006C658(Struct_D800FDF58 *rec, u8 *base)
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

    p = base + 4;
    mode = p[1] >> 1;
    wrap = p[1] & 1;
    if (mode & 0x40)
    {
        mode -= 0x40;
    }

    seq = rec->unk21;
    idx = seq & 0x7F;
    if (idx >= (s32)base[4])
    {
        p = base + 6;
        rec->unk21 = seq & 0x80;
    }
    else
    {
        p = p + (idx * 2 + 2);
    }

    off = p[0];
    hi = p[1];
    p = base + off + ((hi & 0x7F) << 8);
    shift = (hi >> 7) + 1;

    rec->unk27 = p[0] - 1;
    p = p + ((rec->unk27 << shift) + 1);
    dur = p[1] * rec->unk24;
    rec->unk16 = dur;
    rec->unk35 = dur;
    if (rec->unk16 == 0)
    {
        rec->unk16++;
        rec->unk35++;
    }
    rec->unk34 = 0;
    if (shift == 2)
    {
        rec->unk37 = p[2];
        rec->unk36 = p[3];
        rec->unk38 = 0;
    }
    else
    {
        rec->unk38 = 0;
        rec->unk37 = 0;
        rec->unk36 = mode;
    }

    rec->unk3C &= 0xFEFFFFFF;

    q = base + base[2] + (base[3] << 8) + (p[0] * 2 + 2);
    if (wrap)
    {
        return (u8 *)((s32)(base + q[0] + (q[1] << 8)) & 0x7FFFFFFF);
    }
    return base + q[0] + (q[1] << 8);
}

/**
 * @brief Return the frame count of the animation entry AFTER the record's
 *        current one, or 0 when that entry is past the end of the table.
 * @param rec Field record whose unk3B selects the resource and unk21 the entry.
 * @return Frame count byte of entry (unk21 & 0x7F) + 1, or 0 if out of range.
 * @note WIP - not byte-matching. Insn count and every opcode/offset are exact;
 *       the only defect is one coloring decision. The target keeps `rec` in a2
 *       behind an `addu a2, a0, zero` entry copy because the unk3B index temp
 *       takes a0; ours coalesces the entry copy so `rec` keeps a0 and the temp
 *       goes to a1, which also costs a load-delay nop where the target puts the
 *       base[4] read. Confirmed cause (see idioms.md [ENTRY-05]): in our compile
 *       `rec` is a block-local allocno, so local-alloc honours its a0
 *       copy-suggestion before the index temp is placed. The target coloring
 *       (rec a2, index temp a0, base a1) needs `rec` to be a GLOBAL allocno so
 *       that the index temp takes a0 first and prunes rec's a0 preference. A
 *       dummy `rec` read after the branch reproduces the whole entry sequence
 *       (+2 exact rows, +2 insns), but no natural spelling that keeps `rec`
 *       live past the branch without emitting a use has been found. Measured
 *       inert: aliases, named temps, statement order, K&R, if/else, result
 *       variable, for/while wrappers, volatile field read, u8* parameter.
 * @see decomp.me (89.19%) TODO
 */
u8 func_8006C7D8(Struct_D800FDF58 *rec)
{
    u8 *base;
    u8 *p;
    s32 idx;

    base = g_field_resource_entries[rec->unk3B].start;
    p = base + 4;
    idx = (rec->unk21 & 0x7F) + 1;
    if (idx >= (s32)base[4])
    {
        return 0;
    }
    p = p + (idx * 2 + 2);
    p = base + p[0] + ((p[1] & 0x7F) << 8);
    return p[0];
}

/**
 * @see decomp.me (100%) TODO
 */
u8 *func_8006C854(Struct_D800FDF58 *rec, u8 *base)
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

    p = base + 4;
    mode = p[1] >> 1;
    wrap = p[1] & 1;
    if (mode & 0x40)
    {
        mode -= 0x40;
    }

    idx = rec->unk21 & 0x7F;
    if (idx >= (s32)base[4])
    {
        p = base + 6;
    }
    else
    {
        p = p + (idx * 2 + 2);
    }

    off = p[0];
    hi = p[1];
    p = base + off + ((hi & 0x7F) << 8);
    rec->unk3C |= 0x1000000;
    shift = (hi >> 7) + 1;

    if (D_800F2298 == 0 && D_8012269C == 0 && D_801227C8 == 0 && rec->unk24 != 0)
    {
        rec->unk16--;
        rec->unk34++;
    }

    if (rec->unk16 == 0 && rec->unk24 != 0)
    {
        cursor = rec->unk27 + 1;
        rec->unk27 = cursor;
        if (cursor >= p[0])
        {
            part = &g_field_actor_slots[rec->unk22].unk0[rec->unk23];
            if (!((part->unk4 >> 4) & 3))
            {
                func_80071500(rec, part);
                return 0;
            }
            rec->unk27 = 0;
        }

        pos = rec->unk27;
        rec->unk3C &= 0xFEFFFFFF;
        at_end = (pos + 1) == (s32)p[0];
        p = p + ((pos << shift) + 1);
        dur = p[1] * rec->unk24;
        rec->unk34 = 0;
        rec->unk16 = dur;
        rec->unk35 = dur;
        if (shift == 2)
        {
            rec->unk37 = p[2];
            rec->unk36 = p[3];
            if (at_end)
            {
                rec->unk38 = 0;
            }
            else
            {
                rec->unk38 = p[6];
            }
        }
        else
        {
            rec->unk38 = 0;
            rec->unk37 = 0;
            rec->unk36 = mode;
        }
    }
    else
    {
        p = p + ((rec->unk27 << shift) + 1);
    }

    q = base + base[2] + (base[3] << 8) + (p[0] * 2 + 2);
    if (wrap)
    {
        return (u8 *)((s32)(base + q[0] + (q[1] << 8)) & 0x7FFFFFFF);
    }
    return base + q[0] + (q[1] << 8);
}

/**
 * @see decomp.me (100%) TODO
 */
void func_8006CAFC(u16 arg0, s32 arg1, s32 arg2)
{
    FieldCdBuffer *buf;
    s32 size;

    buf = D_8010D038;
    size = cdrom_queue_read(arg0, buf);
    cdrom_wait_queue_empty();
    func_8006CB6C(buf, size, arg1, arg2);
}

/**
 * @see decomp.me (100%) TODO
 */
void func_8006CB6C(FieldCdBuffer *buf, s32 size, s32 arg2, s32 arg3)
{
    switch (buf->unk0 >> 2)
    {
    case 2:
        func_8006CE00((u8 *)buf + buf->unk4, size - buf->unk4, arg2);
        func_8006CCAC((u8 *)buf + buf->unk0, arg2, 0, arg3);
        break;
    case 3:
        func_8006CE00((u8 *)buf + buf->unk8, size - buf->unk8, arg2);
        func_8006CCAC((u8 *)buf + buf->unk0, arg2, 0, arg3);
        func_8006CCAC((u8 *)buf + buf->unk4, arg2, 1, arg3);
        break;
    case 4:
        func_8006CE00((u8 *)buf + buf->unkC, size - buf->unkC, arg2);
        func_8006CCAC((u8 *)buf + buf->unk0, arg2, 0, arg3);
        func_8006CCAC((u8 *)buf + buf->unk4, arg2, 1, arg3);
        func_8006CCAC((u8 *)buf + buf->unk8, arg2, 2, arg3);
        break;
    }
}

/**
 * @see decomp.me (100%) TODO
 */
void func_8006CCAC(FieldCdBuffer *buf, s32 arg1, s32 arg2, s32 arg3)
{
    RECT rect;
    s32 second;

    second = buf->unk8;

    if (arg2 == 2)
    {
        rect.x = 0xC0;
        rect.y = arg3 + 0x1F4;
        rect.w = 0x40;
        rect.h = 1;
    }
    else
    {
        rect.y = arg3 + 0x1F4;
        rect.w = 0x100;
        rect.x = 0;
        rect.h = 1;
    }

    if (arg2 != 1)
    {
        LoadImage(&rect, &buf->unk14);
    }

    if (arg1 >= 2)
    {
        s32 base = 0x340;
        s32 off = arg1 << 6;
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

        if (arg2 == 2)
        {
            rect.x = 0x3C0 - (arg1 << 7);
            rect.y = 0x80;
            rect.w = 0x40;
            rect.h = 0x80;
        }
        else if (arg2 == 1)
        {
            rect.x = 0x3C0 - (arg1 << 7);
            rect.y = 0;
            rect.w = w;
            rect.h = h;
        }
        else
        {
            s32 base = (arg2 << 6) + 0x380;
            s32 off = arg1 << 7;
            rect.x = base - off;
            rect.w = 0x40;
            rect.y = 0;
            rect.h = 0x100;
        }
    }

    LoadImage(&rect, (u8*)(second + (s32)buf + 0x14));
}

/**
 * @see decomp.me (100%) TODO
 */
void func_8006CE00(u32 *src, s32 len, s32 slot)
{
    u32 *dst;
    u32 n;

    dst = g_field_resource_cursor;
    n = (u32)(len + 3) >> 2;
    while (n != 0)
    {
        *dst = *src;
        src++;
        n--;
        dst++;
    }

    g_field_resource_entries[slot].start = g_field_resource_cursor;
    g_field_resource_cursor += (len + 3) & ~3;
}

/**
 * @see decomp.me (100%) TODO
 */
s32 func_8006CE70(s32 arg0)
{
    Vec2s pos;
    s32 t;

    pos.x = 0xA0 + D_800F22A0 / 256 + D_800FDF58[arg0].unk0 / 256;
    pos.y = 0x70 + D_800F22A4 / 256 + D_800FDF58[arg0].unk4 / 256 - D_800FDF58[arg0].unk8 / 512 - D_800F22A8 / 512;

    t = pos.x;
    if (t >= 0x10)
    {
        if (t >= 0x131)
        {
            return 0x9F;
        }
        t = ((t - 0x10) * 63) / 288;
        return t + 0x60;
    }
    return 0x60;
}

/**
 * @see decomp.me (100%) TODO
 */
void func_8006CF88(void)
{
    u8 *slot1;
    u8 *slot2;
    u8 *pad;
    s32 idx1;
    s32 sel;

    if (D_800FD818[1].unk3 != 0)
    {
        idx1 = D_800FD818[1].unk2 + 2;
    }
    else
    {
        idx1 = (D_800FD818[1].u0.h >> 1) & 1;
    }

    if (D_800FD818[0].unk256 != ((D_800FD818[0].u0.h >> 1) & 1) ||
        D_800FD818[1].unk256 != idx1 ||
        D_800FD818[2].unk256 != D_800FD818[2].unk2 + 0xE)
    {
        if (D_800FD818[1].unk256 != idx1)
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

        slot1 = g_prim_rect_buf + 0x4A0;
        D_800FD818[1].unk256 = idx1;
        bcopy((u8 *)D_8010D038 + ((u32 *)D_8010D038)[D_800FD818[1].unk256 + 1], slot1, 0x4A0);

        slot2 = g_prim_rect_buf + 0x940;
        D_800FD818[2].unk256 = D_800FD818[2].unk2 + 0xE;
        bcopy((u8 *)D_8010D038 + ((u32 *)D_8010D038)[D_800FD818[2].unk256 + 1], slot2, 0x4A0);

        if (D_800FD818[1].unk3 == 0)
        {
            func_800A5638(slot1, (D_800FD818[1].u0.h >> 1) & 1);
        }

        if (g_pad_ctx[0xA90] != 0 && (*(u32 *)&g_pad_ctx[0xAA8] & 0x7F) == 4)
        {
            sel = *(s8 *)&g_pad_ctx[0x29D7];
            if (sel < 3)
            {
                pad = g_pad_ctx;
                pad += sel * 0x14C;
                func_800A55E4(slot2, *(s32 *)(pad + 0x2B54));
            }
        }
    }

    func_80084240();
}

/**
 * @see decomp.me (100%) TODO
 */
void func_8006D1EC(void)
{
    s32 i;
    s32 val;

    val = 0xFF;
    for (i = 0x102; i >= 0; i--)
    {
        D_800FF658[i].unk25 = val;
    }

    D_80105770 = 0;
}

/**
 * @see decomp.me (100%) TODO
 */
void func_8006D21C(FieldActorState *actor)
{
    s32 i;
    s32 owner;
    s32 val;
    Struct_D800FDF58 *base;
    u8 *p;

    owner = actor->unk233;
    i = 0;
    val = 0xFF;
    base = D_800FF658;
    p = (u8 *)base + 0x25;
    while (i < 0x103)
    {
        if (*p != val && p[-3] == owner)
        {
            *p = val;
        }
        i++;
        p += 0x54;
    }
}

/**
 * @see decomp.me (100%) TODO
 */
void func_8006D270(FieldActorState *actor)
{
    s32 i;

    if (actor->unk232 != 0)
    {
        for (i = 0; i < actor->unk232; i++)
        {
            if ((actor->unk23A >> i) & 1)
            {
                g_field_track_index = i;
                func_8006D310(actor);
            }
        }
    }
    else
    {
        g_field_track_index = 0;
        func_8006D310(actor);
    }
}

/**
 * @see decomp.me (100%) TODO
 */
void func_8006D310(FieldActorState *actor)
{
    FieldActorPartDef *part;
    s32 i;
    s32 val;
    s32 prev;
    s32 count;
    u8 kind;
    u32 flags;

    part = actor->unk0;
    i = 0;
    if (actor->unk25 != 0)
    {
        do
        {
            kind = part->unk31;
            if (kind != 0xFE &&
                (!(actor->unkC->unkC & 0x800) || ((actor->unk240[actor->unk29] >> i) & 1)) &&
                part->unkB != 0xFF &&
                (!(part->u14.w & 4) || g_field_track_index == 0))
            {
                if (actor->unk229[g_field_track_index] == 0xFF)
                {
                    if (kind == 0xFF || (part->unk34 & 0x4000000))
                    {
                        goto next;
                    }
                    if (part->unk31 > actor->unk1EC[g_field_track_index])
                    {
                        goto next;
                    }
                }
                else if (kind != 0xFF)
                {
                    if (part->unk31 > actor->unk1EC[g_field_track_index])
                    {
                        goto next;
                    }
                }

                if (((u8 *)part)[0x2B] & 1)
                {
                    val = part->unkC;
                }
                else
                {
                    val = field_evaluate_parameter_track(actor, part->unkC);
                }

                flags = part->unk28;
                if (((flags >> 30) & 1) && actor->unkCC[g_field_track_index][i] >= val)
                {
                    goto next;
                }

                if ((part->unk0 >> 15) & 1)
                {
                    if (((flags >> 24) & 1) && actor->unk3B[g_field_track_index][i] != 0)
                    {
                        goto next;
                    }
                    if (field_get_track_counter_modulo(actor, (((u8 *)part)[7] & 0xF) + 1) != 0)
                    {
                        goto next;
                    }
                    if (actor->unk3B[g_field_track_index][i] >= val)
                    {
                        goto next;
                    }
                    do
                    {
                        prev = actor->unk3B[g_field_track_index][i];
                        D_80105760 = 0;
                        if (func_8006D79C(actor, i, 0) == -1)
                        {
                            goto next;
                        }
                        if (actor->unk3B[g_field_track_index][i] == prev)
                        {
                            goto next;
                        }
                    } while (actor->unk3B[g_field_track_index][i] < val);
                }
                else
                {
                    if (actor->unk3B[g_field_track_index][i] < val &&
                        field_get_track_counter_modulo(actor, (((u8 *)part)[7] & 0xF) + 1) == 0)
                    {
                        count = (part->unk2C & 0x1F) + 1;
                        while (count != 0)
                        {
                            if (actor->unk3B[g_field_track_index][i] >= val)
                            {
                                break;
                            }
                            D_80105760 = 0;
                            if (func_8006D79C(actor, i, 0) == -1)
                            {
                                break;
                            }
                            count--;
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
 * @see decomp.me (100%) TODO
 */
void func_8006D6A8(Struct_D800FDF58 *dst, FieldActorPartDef *part, Struct_D800FDF58 *rec)
{
    if (!((part->unk4 >> 11) & 1) && !((part->unk28 >> 25) & 1) && (part->unk2C >> 5) == 0 &&
        (*(u32 *)&part->unkC & 0xFFFF0000) == 0x80800000 && part->unk10 == 0x80)
    {
        dst->unk1C |= 0x10008000;
        dst->unk18 = D_800FE3A0[rec->unk3A].unkE;
        dst->unk19 = D_800FE3A0[rec->unk3A].unkF;
        dst->unk1A = D_800FE3A0[rec->unk3A].unk10;
    }
}

