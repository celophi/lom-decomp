#include "common.h"
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
typedef struct FieldActorPartDef
{
    u8 pad0[0xD];
    u8 unkD;
    u8 padE[0x31 - 0xE];
    u8 unk31;
    u8 unk32;
    u8 pad33[1];
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
    u8 pad29;
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
    u8 pad23C[0x244 - 0x23C];
} FieldActorState;

typedef struct
{
    u32 unk0;            /* 0x00 */
    u32 unk4;            /* 0x04 */
    u32 unk8;            /* 0x08 */
    u8 padC[0x1C - 0xC]; /* 0x0C */
    s32 unk1C;           /* 0x1C */
    u8 pad20[0x25 - 0x20];
    u8 unk25; /* 0x25 */
    u8 pad26[0x2A - 0x26];
    s16 unk2A; /* 0x2A */
    u8 pad2C[0x54 - 0x2C];
} Struct_D800FDF58;

typedef struct
{
    u8 pad0[0xC];
    u32 unkC;
    u8 pad10[0x178 - 0x10];
    union
    {
        u32 unk178;
        struct
        {
            u8 pad[2];
            u8 unk17A;
            u8 pad2;
        } b;
    } u;
    u8 pad17C[0x23C - 0x17C];
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
    u8 pad0[0x254 - 2];     /* 0x02 .. 0x253 */
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
                func_8006441C();
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
    func_800643E0();
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
s32 field_evaluate_parameter_track(FieldAnimationData* animation_data, s32 curve_index, u16 unused)
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

    if (field_finalize_actor_animation() == 0)
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
                        d = entry->pad2C[0xF];
                        if (((u32)(d - 3)) < 3)
                        {
                            if (D_800FF59C != 0)
                            {
                                func_800A39A8(val & 0x3FF, temp, 0, ((u8*)actor)[0x228]);
                            }
                            else
                            {
                                func_800A39A8(val & 0x3FF, temp, entry->pad2C[0xF] - 3, ((u8*)actor)[0x228]);
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
    u8 pad8[1];
    u8 slot_index; /* 0x09 */
    u8 padA[6];
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

extern PadContext* g_pad_ctx;

extern u8 g_field_resource_buffer[];
extern u16 D_800FDA80;
extern FieldActorPartDef D_800FE3A0[];
extern FieldActorAnimationDef D_800FE758;
extern s32 D_800FE774;
extern FieldResourceEntry g_field_resource_entries[];
extern void* g_field_resource_cursor;
extern s32 D_801158A0;

void field_relocate_resource_buffer(s32);
void func_8006C3FC(Struct_D800FDF58*, void*);
void field_restore_default_action_animation_mappings(s32);
s32 func_8006A88C(s32, D_800FD818_type*, s32);
void func_8006A9A4(s32, s32, s32, s32);
void func_8006B4D0(s32, s32);
void func_8006B7A0(s32, s32);
void func_8006CF88(void*, void*, s32, void*);
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
 * @see decomp.me (98.98%) https://decomp.me/scratch/iTv8i
 */
void field_relocate_resource_buffer(s32 resource_index)
{
    u32 new_var4;
    u32 new_var2;
    Struct_D800FDF58* new_var;
    u8* old_start;

    new_var2 = (u32)0x80180000;
    g_field_resource_cursor += 0;
    new_var4 = (u32)g_field_resource_buffer;
    bcopy((void*)((new_var2 - new_var4) + ((u32)g_field_resource_entries[resource_index].start)), g_field_resource_cursor,
          g_field_resource_entries[resource_index].end - g_field_resource_entries[resource_index].start);
    new_var4 = g_field_resource_entries[resource_index].end - g_field_resource_entries[resource_index].start;
    old_start = g_field_resource_entries[resource_index].start;
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
