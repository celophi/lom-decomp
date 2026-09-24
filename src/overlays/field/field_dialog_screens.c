/** @file field_dialog_screens.c
 * @brief Timed panels, actor text queue, ability progression and dialog screens.
 *
 * One translation unit covering 0x800A5638 .. 0x800A88A0 (formerly
 * field_timed_panel.c, field_actor_text_queue.c, field_ability_progression.c
 * and field_dialog_screens.c). Symbols whose reconstructed type varies between
 * functions (g_field_player_records, g_field_object_states, g_field_actors,
 * D_8010D038, g_pad_ctx, bcopy, func_800ADF84, func_800A88A0, func_800A8A78,
 * func_800A838C) are declared at block scope inside each user with that
 * function's original type; do not hoist them to file scope.
 */

#include "common.h"
#include "vector.h"
#include "sdk/libgte.h"
#include "sdk/libgpu.h"
#include "field_text.h"
#include "field_effect_render_state.h"
#include "field_ability_progression.h"
#include "saved_game.h"
#include "field_modal_runtime.h"
#include "field_scene_transition.h"
#include "cdrom.h"

/* ---- Timed panels (0x800A5638 .. 0x800A6204) ---- */

/** @brief One 0x20-byte record in the D_800EE6E8 lookup table. */
typedef struct
{
    u8 data[0x20];
} UnkTable800EE6E8Entry;

/**
 * @brief FIELD context field holding the current primitive-chain handle.
 */
typedef struct
{
    u8 pad0[0x40B8];
    s32 primitive;
} FieldContext;

/** @brief Position, texture coordinates, dimensions and effect flags of one quad. */
typedef struct
{
    u16 x, y;
    u8 u, v;
    u16 width, height, flags;
} FieldTransitionQuad;

extern UnkTable800EE6E8Entry D_800EE6E8[];
extern FieldTransitionQuad D_800EF124[][4];
extern u8 D_800EF85C[];
extern s32 D_800F2298;
extern Vec2s D_801077FC;
extern u16 D_80122720;
extern u16 D_80122722;
extern s32 D_801229F0;
extern s32 g_pad_input;

void field_load_vram_resource(s32 id, s16 *rect, s32 arg2);
void field_set_fade_target_only(s16 red, s16 green, s16 blue, s16 duration);
void field_restore_fade_target_with_duration(s16 duration);
void func_800A3938(s32 sound_id, s32 pan);
void func_80086F48(const void *src, s16 value);
s32 rcos(s32);
s32 rsin(s32);
POLY_FT4 *func_800A5960(POLY_FT4 *prim, u_long *ordering_table, s32 index);
POLY_FT4 *func_800A6060(POLY_FT4 *prim, u_long *ordering_table);

/**
 * @brief Copy one 0x20-byte record out of the D_800EE6E8 lookup table.
 * @param dest Destination buffer receiving the copied record.
 * @param index Record index into D_800EE6E8.
 */
void func_800A5638(void *dest, s32 index)
{
    extern void *bcopy(const unsigned char *, unsigned char *, int);
    bcopy((u8 *)&D_800EE6E8[index], dest, 0x20);
}

/**
 * @brief Initializes field resource state and selects its startup sound.
 * @param index Resource-table index used to select the field data and sound.
 */
void func_800A5670(s32 index)
{
    extern s16 D_8011F3D0;
    extern s16 D_80122904;
    s16 rect[4];
    u8 value;
    s32 sound_id;

    if (D_800F2298 != 0)
    {
        return;
    }

    rect[0] = 0x140;
    rect[1] = 0;
    rect[2] = 0;
    rect[3] = 0x1F2;
    field_load_vram_resource(index + 0x1060, rect, 1);

    D_801229F0 = index;
    D_80122720 = rect[0];
    D_80122722 = rect[1];
    field_set_fade_target_only(0x80, 0x80, 0x80, 0x14);
    D_80122904 = 0x96;
    D_8011F3D0 = 0;
    D_800F2298 = 1;

    value = D_800EF85C[D_801229F0];
    if (value & 0x80)
    {
        switch (value & 0x7F)
        {
        case 0:
            sound_id = 0x11C;
            break;
        case 1:
            sound_id = 0x11A;
            break;
        case 2:
            sound_id = 0x11E;
            break;
        default:
            return;
        }
    }
    else
    {
        sound_id = 0x118;
    }

    func_800A3938(sound_id, 0x80);
}

/**
 * @brief Advance a timed FIELD panel, update its fade, and render its contents.
 * @param context FIELD context containing the primitive-chain handle.
 * @note Selected pad buttons shorten the middle portion of the countdown.
 */
void func_800A5794(FieldContext *context)
{
    extern s16 D_8011F3D0;
    extern u16 D_80122904;
    s32 timer_value;
    s32 primitive;
    u16 decremented_timer;
    u32 timer;
    u8 event_selector;
    u8 render_selector;

    if (D_800F2298 != 0)
    {
        decremented_timer = D_80122904 - 1;
        D_80122904 = decremented_timer;
        if ((decremented_timer & 0xFFFF) == 0x14)
        {
            field_restore_fade_target_with_duration(0x14);
        }
        timer_value = D_80122904;
        timer = timer_value & 0xFFFF;
        if (timer == 0)
        {
            D_800F2298 = 0;
            return;
        }
        if (timer < 0x20U)
        {
            if (timer == 0x1F)
            {
                event_selector = D_800EF85C[D_801229F0];
                if (event_selector & 0x80)
                {
                    switch (event_selector & 0x7F)
                    {
                    case 0:
                        func_800A3938(0x11D, 0x80);
                        break;
                    case 1:
                        func_800A3938(0x11B, 0x80);
                        break;
                    case 2:
                        func_800A3938(0x11F, 0x80);
                        break;
                    }
                }
                else
                {
                    func_800A3938(0x119, 0x80);
                }
            }
            D_8011F3D0 = D_80122904 * 4;
        }
        else if (timer >= 0x76U)
        {
            /* Volatile: the reload keeps this store from cross-jumping with the one above. */
            D_8011F3D0 = (0x96 - *(volatile u16 *)&D_80122904) * 4;
        }
        else if ((timer_value >= 0x21 && timer_value < 0x67) && (g_pad_input & 0x220))
        {
            D_80122904 = 0x20;
        }
        D_801077FC.y = 0;
        D_801077FC.x = 0;
        render_selector = D_800EF85C[D_801229F0];
        primitive = context->primitive;
        if (render_selector != 0)
        {
            primitive = func_800A5960(primitive, context, render_selector & 0x7F);
        }
        else
        {
            primitive = func_800A6060(primitive, context);
        }
        context->primitive = primitive;
    }
}

/**
 * @brief Recover a transition descriptor from a pointer to its flags field.
 * @param flags_ptr Address of the descriptor's flags member.
 * @note The loop walks descriptors through this flags pointer; addressing every
 *       other field relative to it reproduces the original induction variables.
 */
#define QUAD_OF_FLAGS(flags_ptr) ((FieldTransitionQuad *)((u8 *)(flags_ptr) - 10))

/**
 * @brief Append up to four textured quads for a field transition effect.
 * @param prim Next available primitive packet.
 * @param ordering_table Ordering-table entry receiving each generated primitive.
 * @param index Selects a group of four transition descriptors.
 * @return First unused packet after the generated textured quads.
 * @note Effect flags select expansion, translation, rotation or brightness;
 *       disabled descriptors have X equal to 0xFFFF.
 * @note The do/while(0) blocks around the two width products in the rotation
 *       case raise their loop weight so width keeps its saved register.
 * @see decomp.me (100%)
 */
POLY_FT4 *func_800A5960(POLY_FT4 *prim, u_long *ordering_table, s32 index)
{
    extern u16 D_8011F3D0;
    extern u16 D_80122904;
    s32 quad_index;
    u32 half_width;
    s32 bottom_2;
    s32 x_sine;
    s32 y_cosine;
    s32 x_sine_2;
    s32 y_cosine_2;
    s32 rotated_y;
    s32 term;
    s32 x_cosine, y_sine, x_cosine_2, y_sine_2;
    s32 rotated_x;
    u8 brightness;
    s32 expansion;
    s32 angle;
    s32 negative_width;
    s32 offset_1;
    s32 scaled;
    s32 bottom_1;
    s32 scaled_x_2;
    s32 right_1;
    s32 height_term;
    s32 height_term_y;
    s32 height_term_2;
    s32 height_term_y_2;
    s32 adjusted;
    s32 width_term;
    s32 palette_mask;
    s32 width_term_y;
    s32 scaled_y;
    s32 width_term_2;
    s32 width_term_y_2;
    s32 scaled_y_2;
    FieldTransitionQuad *quad;
    u8 *flags_base;
    u8 *quad_flags;
    s32 raw_width;
    u32 half;
    u32 width;

    quad = D_800EF124[index];
    flags_base = (u8 *)&quad->flags;
    for (quad_index = 0; quad_index < 4; quad_index++, quad++)
    {
        quad_flags = flags_base + quad_index * 12;
        if (quad->x != 0xFFFF)
        {
            raw_width = QUAD_OF_FLAGS(quad_flags)->width;
            if (QUAD_OF_FLAGS(quad_flags)->flags & 0xF)
            {
                raw_width = (u16)(raw_width >> 1);
            }
            setPolyFT4(prim);
            width = raw_width & 0xFFFF;
            prim->r0 = prim->g0 = prim->b0 = D_8011F3D0;
            if ((D_8011F3D0 != 0x80) || (QUAD_OF_FLAGS(quad_flags)->flags & 0x100))
            {
                setSemiTrans(prim, 1);
            }
            expansion = 0x80 - D_8011F3D0;
            switch ((QUAD_OF_FLAGS(quad_flags)->flags >> 11) & 0xF)
            {
            case 0:
                term = width * expansion;
                scaled = term;
                if (term < 0)
                {
                    scaled = term + 0x7F;
                }
                scaled = scaled >> 7;
                prim->x0 = prim->x2 = quad->x - scaled;
                prim->x1 = prim->x3 = quad->x + width + scaled - 1;
                prim->y0 = prim->y1 = QUAD_OF_FLAGS(quad_flags)->y - QUAD_OF_FLAGS(quad_flags)->height * expansion / 128;
                scaled = *(volatile u16 *)&QUAD_OF_FLAGS(quad_flags)->height * expansion;
                bottom_1 = QUAD_OF_FLAGS(quad_flags)->y + QUAD_OF_FLAGS(quad_flags)->height;
                if (scaled < 0)
                {
                    scaled += 0x7F;
                }
                prim->y2 = prim->y3 = bottom_1 + (scaled >> 7) - 1;
                break;
            case 1:
                if (D_80122904 < 0x25)
                {
                    scaled = width * expansion;
                    adjusted = scaled;
                    if (scaled < 0)
                    {
                        adjusted = scaled + 0x7F;
                    }
                    offset_1 = adjusted >> 7;
                    prim->x0 = prim->x2 = quad->x - offset_1;
                    right_1 = (quad->x + width) - offset_1;
                }
                else
                {
                    scaled = width * expansion;
                    adjusted = scaled;
                    if (scaled < 0)
                    {
                        adjusted = scaled + 0x7F;
                    }
                    offset_1 = adjusted >> 7;
                    prim->x0 = prim->x2 = quad->x + offset_1;
                    right_1 = quad->x + width + offset_1;
                }
                prim->x1 = prim->x3 = right_1 - 1;
                prim->y0 = prim->y1 = QUAD_OF_FLAGS(quad_flags)->y;
                prim->y2 = prim->y3 = QUAD_OF_FLAGS(quad_flags)->y + QUAD_OF_FLAGS(quad_flags)->height - 1;
                break;
            case 2:
                angle = expansion * 50;
                x_cosine = rcos(angle);
                x_sine = rsin(angle);
                do
                {
                    width_term = width * x_cosine;
                } while (0);
                if (width_term < 0)
                {
                    width_term += 0x1FFF;
                }
                width_term >>= 13;
                height_term = QUAD_OF_FLAGS(quad_flags)->height * x_sine;
                if (height_term < 0)
                {
                    height_term += 0x1FFF;
                }
                rotated_x = width_term + (height_term >> 13);
                y_sine = rsin(angle);
                y_cosine = rcos(angle);
                do
                {
                    width_term_y = width * y_sine;
                } while (0);
                if (width_term_y < 0)
                {
                    width_term_y += 0x1FFF;
                }
                width_term_y >>= 13;
                height_term_y = QUAD_OF_FLAGS(quad_flags)->height * y_cosine;
                if (height_term_y < 0)
                {
                    height_term_y += 0x1FFF;
                }
                scaled = expansion * rotated_x;
                rotated_y = width_term_y - (height_term_y >> 13);
                if (scaled < 0)
                {
                    scaled += 0x7F;
                }
                scaled_y = expansion * rotated_y;
                rotated_x = rotated_x + (scaled >> 7);
                if (scaled_y < 0)
                {
                    scaled_y += 0x7F;
                }
                rotated_y = rotated_y + (scaled_y >> 7);
                half = width >> 1;
                half_width = half;
                prim->x0 = (u16)((quad->x + half) - rotated_x);
                prim->x3 = (s16)(quad->x + half + rotated_x);
                prim->y0 = (u16)(QUAD_OF_FLAGS(quad_flags)->y + (QUAD_OF_FLAGS(quad_flags)->height >> 1) + rotated_y);
                prim->y3 = (s16)((QUAD_OF_FLAGS(quad_flags)->y + (QUAD_OF_FLAGS(quad_flags)->height >> 1)) - rotated_y);
                x_cosine_2 = rcos(angle);
                x_sine_2 = rsin(angle);
                negative_width = -(s32)width;
                width_term_2 = negative_width * x_cosine_2;
                if (width_term_2 < 0)
                {
                    width_term_2 += 0x1FFF;
                }
                width_term_2 >>= 13;
                height_term_2 = QUAD_OF_FLAGS(quad_flags)->height * x_sine_2;
                if (height_term_2 < 0)
                {
                    height_term_2 += 0x1FFF;
                }
                rotated_x = width_term_2 + (height_term_2 >> 13);
                y_sine_2 = rsin(angle);
                y_cosine_2 = rcos(angle);
                width_term_y_2 = negative_width * y_sine_2;
                if (width_term_y_2 < 0)
                {
                    width_term_y_2 += 0x1FFF;
                }
                width_term_y_2 >>= 13;
                height_term_y_2 = QUAD_OF_FLAGS(quad_flags)->height * y_cosine_2;
                if (height_term_y_2 < 0)
                {
                    height_term_y_2 += 0x1FFF;
                }
                scaled_x_2 = expansion * rotated_x;
                rotated_y = width_term_y_2 - (height_term_y_2 >> 13);
                if (scaled_x_2 < 0)
                {
                    scaled_x_2 += 0x7F;
                }
                scaled_y_2 = expansion * rotated_y;
                rotated_x = rotated_x + (scaled_x_2 >> 7);
                if (scaled_y_2 < 0)
                {
                    scaled_y_2 += 0x7F;
                }
                prim->x1 = (s16)((quad->x + half_width) - rotated_x);
                rotated_y = rotated_y + (scaled_y_2 >> 7);
                prim->x2 = (u16)(quad->x + half_width + rotated_x);
                prim->y1 = (u16)(QUAD_OF_FLAGS(quad_flags)->y + (QUAD_OF_FLAGS(quad_flags)->height >> 1) + rotated_y);
                adjusted = QUAD_OF_FLAGS(quad_flags)->height;
                adjusted = (u32)adjusted >> 1;
                bottom_2 = QUAD_OF_FLAGS(quad_flags)->y + adjusted - rotated_y;
                prim->y2 = bottom_2;
                break;
            case 3:
                if (D_8011F3D0 < 0x40)
                {
                    brightness = D_8011F3D0 * 4;
                }
                else
                {
                    brightness = ~((D_8011F3D0 - 0x40) * 2);
                }
                prim->r0 = prim->g0 = prim->b0 = brightness;
                prim->x0 = prim->x2 = quad->x;
                prim->x1 = prim->x3 = quad->x + width - 1;
                prim->y0 = prim->y1 = QUAD_OF_FLAGS(quad_flags)->y;
                prim->y2 = prim->y3 = QUAD_OF_FLAGS(quad_flags)->y + QUAD_OF_FLAGS(quad_flags)->height - 1;
                break;
            case 4:
                term = width * expansion;
                scaled = term;
                if (term < 0)
                {
                    scaled = term + 0x3F;
                }
                scaled = scaled >> 6;
                prim->x0 = prim->x2 = quad->x - scaled;
                prim->x1 = prim->x3 = quad->x + width + scaled - 1;
                prim->y0 = prim->y1 = QUAD_OF_FLAGS(quad_flags)->y - QUAD_OF_FLAGS(quad_flags)->height * expansion / 128;
                scaled = *(volatile u16 *)&QUAD_OF_FLAGS(quad_flags)->height * expansion;
                bottom_1 = QUAD_OF_FLAGS(quad_flags)->y + QUAD_OF_FLAGS(quad_flags)->height;
                if (scaled < 0)
                {
                    scaled += 0x7F;
                }
                prim->y2 = prim->y3 = bottom_1 + (scaled >> 7) - 1;
                break;
            }
            prim->u0 = prim->u2 = QUAD_OF_FLAGS(quad_flags)->u;
            prim->u1 = prim->u3 = QUAD_OF_FLAGS(quad_flags)->u + width - 1;
            prim->v0 = prim->v1 = QUAD_OF_FLAGS(quad_flags)->v;
            palette_mask = 0xF0;
            prim->v2 = prim->v3 = QUAD_OF_FLAGS(quad_flags)->v + QUAD_OF_FLAGS(quad_flags)->height - 1;
            prim->clut = getClut(QUAD_OF_FLAGS(quad_flags)->flags & palette_mask, 0x1F2);
            prim->tpage = getTPage(QUAD_OF_FLAGS(quad_flags)->flags & 3, (QUAD_OF_FLAGS(quad_flags)->flags >> 9) & 3, 320, 0);
            addPrim(ordering_table, prim);
            if (D_8011F3D0 < 0x7C)
            {
                term = (QUAD_OF_FLAGS(quad_flags)->flags >> 11) & 15;
                switch (term)
                {
                case 0:
                    break;
                case 1:
                    func_80086F48(prim, 0);
                    break;
                case 2:
                    func_80086F48(prim, 0);
                    break;
                }
            }
            prim++;
        }
    }
    return prim;
}

/**
 * @brief Build and link a textured field quad using the current field dimensions.
 * @param prim Primitive buffer slot to populate.
 * @param ordering_table Ordering-table entry receiving the primitive.
 * @return Pointer to the next primitive buffer slot.
 */
POLY_FT4 *func_800A6060(POLY_FT4 *prim, u_long *ordering_table)
{
    extern s16 D_8011F3D0;
    s32 value0;
    s32 value1;
    s32 value2;
    s32 value3;
    s32 value4;
    s32 value5;
    u32 address_mask;

    value0 = 9;
    setlen(prim, value0);
    value0 = 0x2C;
    setcode(prim, value0);
    value0 = (u8)D_8011F3D0;
    prim->b0 = value0;
    prim->g0 = value0;
    prim->r0 = value0;
    value1 = getcode(prim);
    value1 |= 2;
    setcode(prim, value1);
    value1 = 0x80;
    value0 = (u16)D_8011F3D0;
    value4 = D_80122720;
    value5 = value1 - value0;
    value3 = value4 * value5;
    value2 = value3 / 32;
    value0 = 0xA0;
    value0 -= value2;
    value1 = value4 * 2;
    value0 -= value1;
    prim->x2 = value0;
    prim->x0 = value0;
    value0 = value4 * 4;
    value0 += 0xA0;
    /* Loop notes keep the scheduler from moving this subtract into the load delay slot. */
    do
    {
        value0 -= value1;
    } while (0);
    value3 = D_80122722;
    value1 = value3 * value5;
    value0 += value2;
    value0--;
    prim->x3 = value0;
    prim->x1 = value0;
    value0 = 0x70;
    value2 = (u32)value3 >> 1;
    value0 -= value2;
    value4 = value1;
    if (value1 < 0)
    {
        value4 = value1 + 0x7F;
    }
    value5 = value4 >> 7;
    value0 -= value5;
    prim->y1 = value0;
    prim->y0 = value0;
    value0 = value2 - 0x70;
    value0 = value3 - value0;
    address_mask = 0xFFFFFF;
    value0 += value5;
    value0--;
    prim->y3 = value0;
    prim->y2 = value0;
    value0 = 0x7C80;
    prim->u2 = 0;
    prim->u0 = 0;
    value1 = D_80122720;
    prim->v1 = 0;
    prim->v0 = 0;
    prim->clut = value0;
    value1 <<= 2;
    value1--;
    prim->u3 = value1;
    prim->u1 = value1;
    value0 = (u8)D_80122722;
    value1 = 0x25;
    prim->tpage = value1;
    value0--;
    prim->v3 = value0;
    prim->v2 = value0;
    setaddr(prim, *ordering_table & address_mask);
    setaddr(ordering_table, prim);
    if ((u16)D_8011F3D0 < 0x7C)
    {
        func_80086F48(prim, 0);
    }
    return prim + 1;
}

/* ---- Actor text queue (0x800A6204 .. 0x800A68B4) ---- */

/** @brief Text pointer and packed position/countdown state for one text slot. */
typedef struct Slot
{
    u8 *text;
    u32 flags;
} Slot;

/** @brief Fixed-point position at the start of a 0x54-byte actor entry. */
typedef struct Position
{
    s32 x, y, z;
    u8 rest[0x54 - 12];
} Position;

/** @brief Two-word view of a pending text entry used by the frame handler. */
typedef struct
{
    u32 unk0;   /* 0x0 */
    u32 unk4;   /* 0x4 */
} ArgB;

/** @brief FIELD context passed through to the text entry handler. */
typedef struct
{
    u8 pad0[0x40];
    u8 unk40;   /* 0x40 */
    u8 pad41[0x40B8 - 0x41];
    s32 unk40B8; /* 0x40B8 */
} ArgA;

extern u8 D_800ED064[];

extern s32 g_field_text_session_active;
extern s32 D_801227DC;

extern s32 func_800AE864(u8 *);



void func_800A6634(ArgA *arg0, ArgB *arg1);

SPRT *func_800AD658(s32 *ordering_table, SPRT *sprite_cursor, s32 count);

/**
 * @brief Clear the six-bit frame countdown of the first three text entries.
 */
void func_800A6204(void)
{
    extern s32 D_801226A0[];

    D_801226A0[5] &= 0xFF81FFFF;
    D_801226A0[3] &= 0xFF81FFFF;
    D_801226A0[1] &= 0xFF81FFFF;
}

/**
 * @brief Start an inactive text slot near its actor and clamp its screen position.
 * @param arg0 Actor/text slot index; indices at least two are ignored.
 * @param arg1 Text table index, or a negative packed selector into the pad context.
 */
void func_800A623C(s32 arg0, s32 arg1)
{
    extern u8 *g_pad_ctx;
    extern Position g_field_actors[];
    extern Slot D_801226A0[];

    s16 point[2];
    Slot *slot;
    Slot *initial;
    u8 *initial_base;
    u8 *actor_base;
    Slot *output;
    Slot *base;
    Position *actor;
    s32 offset;
    s32 xoff, x, y, width;
    s32 screen_y;
    s32 actor_screen_x;
    s32 actor_screen_y;

    s32 first;
    if (arg0 < 2)
    {
        initial_base = (u8 *)D_801226A0;
        initial = (Slot *)(arg0 * 8 + initial_base);
        if (!((initial->flags >> 17) & 0x3F))
        {
            do
            {
                first = arg1 * 2;
                if (arg1 < 0)
                {
                    s32 second;
                    s32 selector;
                    u8 **pad_context_ptr;
                    do
                    {
                        pad_context_ptr = &g_pad_ctx;
                    } while (0);
                    selector = (u32)arg1 >> 16;
                    selector &= 0xFF;
                    second = selector * 0x250 + 0x5F0;
                    first = (s32)*pad_context_ptr + second;
                    second = ((arg1 & 0xFF) << 6) + 0x150;
                    first += second;
                    initial->text = (u8 *)first;
                }
                else
                {
                    s32 second;
                    second = (u32)D_800ED064;
                    first = *(u16 *)(first + second);
                    do
                    {
                        first += second;
                        initial->text = (u8 *)first;
                    } while (0);
                }
            } while (0);
            xoff = g_field_view_offset_x;
            base = D_801226A0;
            offset = arg0 * 8;
            slot = (Slot *)(offset + (u8 *)base);
            slot->flags |= 0x7E0000;
            if (xoff < 0)
            {
                xoff += 255;
            }
            actor_base = (u8 *)g_field_actors;
            actor = (Position *)(arg0 * 0x54 + actor_base);
            x = actor->x;
            if (x < 0)
            {
                x += 255;
            }
            arg0 = xoff >> 8;
            xoff = g_field_view_offset_y;
            actor_screen_x = (x >> 8) + 160;
            point[0] = arg0 + actor_screen_x;
            if (xoff < 0)
            {
                xoff += 255;
            }
            x = xoff >> 8;
            y = actor->y;
            if (y < 0)
            {
                y += 255;
            }
            xoff = actor->z;
            actor_screen_y = (y >> 8) + 112;
            screen_y = x + actor_screen_y;
            if (xoff < 0)
            {
                xoff += 511;
            }
            x = g_field_view_offset_z;
            xoff = screen_y - (xoff >> 9);
            if (x < 0)
            {
                x += 511;
            }
            point[1] = xoff - (x >> 9);
            width = func_800AE864(slot->text) * 6;
            if (point[0] + width >= 321)
            {
                point[0] = 320 - width;
            }
            if (point[0] - width - 8 < 0)
            {
                point[0] = width + 8;
            }
            if (point[1] >= 177)
            {
                point[1] = 176;
            }
            if (point[1] < 50)
            {
                point[1] = 50;
            }
            output = (Slot *)((s32)offset + (s32)base);
            {
                s32 point_x;
                point_x = point[0];
                output->flags = (output->flags & ~0x1FF) | (point_x & 0x1FF);
            }
            output->flags = (output->flags & 0xFFFE01FF) | (((point[1] + 4) & 0xFF) << 9);
        }
    }
}

/**
 * @brief Report whether any of the first three text entries has a pending countdown.
 * @return 1 if an entry's six-bit countdown field is nonzero, otherwise 0.
 */
s32 func_800A6490(void)
{
    typedef struct
    {
        s32 unk0;
        s32 unk4;
    } UnkStruct801226A0;
    extern UnkStruct801226A0 D_801226A0[];

    s32 i;
    UnkStruct801226A0 *p;

    i = 0;
    p = D_801226A0;
    for (; i < 3; i++, p++)
    {
        if (((u32)p->unk4 >> 17) & 0x3F)
        {
            return 1;
        }
    }
    return 0;
}

/**
 * @brief Process pending text entries, or clear their countdowns when disabled.
 * @param arg0 FIELD context forwarded to each active entry's handler.
 * @note The previous active count controls window cleanup on the next frame.
 */
void func_800A64D0(ArgA *arg0)
{
    typedef struct
    {
        s32 unk0;
        u32 low : 17;
        u32 count : 6;
        u32 high : 9;
    } UnkStruct801226A0;
    extern UnkStruct801226A0 D_801226A0[];

    s32 i;
    s32 active_count;

    active_count = 0;
    if (g_field_text_session_active != 0)
    {
        i = 0;
        for (; i < 3; i++)
        {
            D_801226A0[i].count = 0;
        }
        D_801227DC = 0;
        return;
    }

    for (i = 0; i < 3; i++)
    {
        if (D_801226A0[i].count)
        {
            active_count += 1;
        }
    }

    if (active_count != 0)
    {
        field_text_reset_scratch();
        for (i = 0; i < 3; i++)
        {
            if (D_801226A0[i].count)
            {
                func_800A6634(arg0, &D_801226A0[i]);
                D_801226A0[i].count--;
            }
        }
        field_text_upload_immediate_cache();
    }
    else if (D_801227DC != 0)
    {
        field_text_reset_windows();
    }
    D_801227DC = active_count;
}

/**
 * @brief Schedule the next text draw for an active entry and store its handle.
 * @param arg0 FIELD context holding the scratch base and current draw handle.
 * @param arg1 Pending text entry supplying the packed position and text pointer.
 * @see decomp.me (100%) TODO
 */
void func_800A6634(ArgA *arg0, ArgB *arg1)
{
    s32 count;
    s32 val;
    u32 field4;
    s32 handle;
    void *p40;

    count = 0x100;
    handle = arg0->unk40B8;
    field4 = arg1->unk4;
    val = (field4 >> 0x11) & 0x3F;
    p40 = &arg0->unk40;
    if (val < 0x20)
    {
        count = val * 4;
    }
    arg0->unk40B8 = func_800A66B4(
        handle,
        p40,
        arg1->unk0,
        4,
        field4 & 0x1FF,
        (arg1->unk4 >> 9) & 0xFF,
        2,
        count);
}

/**
 * @brief Builds and links text sprite primitives into an ordering table.
 * @param sprite_cursor Primitive-buffer cursor used for generated sprites.
 * @param ordering_table Ordering-table entry that receives the generated primitives.
 * @param text Null-terminated text to render.
 * @param text_color Text style passed to the glyph builder.
 * @param x Horizontal origin used to position the rendered text.
 * @param y Vertical origin used to position the rendered text.
 * @param alignment Horizontal alignment mode for the generated glyphs.
 * @param color Sprite tint value, or 0x100 to use the neutral tint.
 * @return Primitive-buffer cursor immediately after the generated draw commands.
 */
void* func_800A66B4(SPRT* sprite_cursor, s32* ordering_table, u8* text, s32 text_color, s32 x, s32 y, s32 alignment, s32 color)
{
    s32 n, count, i, acc;
    SPRT* sprite;
    DR_TPAGE* tpage;

    if (*text == 0)
    {
        return sprite_cursor;
    }

    n = field_text_build_sprites(sprite_cursor, text, text_color);
    count = n;

    if (alignment != 1)
    {
        if (alignment == 2)
        {
            sprite = sprite_cursor;
            for (i = 0; i < count; i++)
            {
                x -= sprite[i].w >> 1;
            }
        }
    }
    else
    {
        sprite = sprite_cursor;
        for (i = 0; i < count; i++)
        {
            x -= sprite[i].w;
        }
    }

    acc = 0;

    if (count != 0)
    {
        do
        {
            setSprt(sprite_cursor);
            if (color != 0x100)
            {
                setRGB0(sprite_cursor, color, color, color);
                setSemiTrans(sprite_cursor, 1);
            }
            else
            {
                setRGB0(sprite_cursor, 0x80, 0x80, 0x80);
            }
            setXY0(sprite_cursor, x + acc, y);
            acc += sprite_cursor->w;

            addPrim(ordering_table, sprite_cursor);
            sprite_cursor++;
            count--;
        } while (count != 0);
    }

    sprite_cursor = func_800AD658(ordering_table, sprite_cursor, n);

    tpage = (DR_TPAGE*)sprite_cursor;
    setDrawTPage(tpage, 0, 0, 0x3F);
    addPrim(ordering_table, tpage);

    return tpage + 1;
}

/* ---- Party proficiency and ability/technique unlock progression ---- */

#define FIELD_TECHNIQUE_UNLOCK_RULE_COUNT 227
#define FIELD_ABILITY_PROFICIENCY_COUNT 88
#define FIELD_PROGRESSION_PARTY_SIZE 3
#define FIELD_PROGRESSION_ACTIVE_FLAG 1
#define FIELD_PROGRESSION_CHARACTER_MASK 0x7F
#define FIELD_PROGRESSION_PLAYER_COUNT 2U
#define FIELD_PROFICIENCY_MAX 100U
#define FIELD_PROFICIENCY_BONUS 4
#define FIELD_WEAPON_CATEGORY_COUNT 11
#define FIELD_EQUIPPED_ABILITY_COUNT 2
#define FIELD_WEAPON_CATEGORY_SHIFT 10
#define FIELD_WEAPON_CATEGORY_MASK 0x3F
#define FIELD_TECHNIQUE_GROUP_SHIFT 4
#define FIELD_TECHNIQUE_WEAPON_MASK 0x0F
#define FIELD_TECHNIQUE_INDEX_MASK 0x7F
#define FIELD_TECHNIQUE_SILENT_FLAG 0x80
#define FIELD_TECHNIQUES_PER_GROUP 24
#define FIELD_TECHNIQUE_UNLOCK_FLAG 0x8000
#define FIELD_UNLOCK_BITS_PER_WORD 32

/** @brief Four ability prerequisites and a packed technique/weapon selection. */
typedef struct
{
    FieldAbilityPrerequisite prerequisites[4];
    u8 weapon;
    u8 result;
    u8 weapon_proficiency;
} FieldTechniqueUnlockRule;

/** @brief Saved character fields used to identify equipped abilities and weapons. */
typedef struct
{
    u8 pad_0[0x18];
    u8 character;
    u8 unknown_0x19;
    u8 abilities[FIELD_EQUIPPED_ABILITY_COUNT];
    u8 pad_1c[0x64 - 0x1C];
    u32 equipment;
    u8 pad_68[SAVED_CHARACTER_SIZE - 0x68];
} FieldProgressionCharacter;

/** @brief Persistent unlock masks, proficiency counters, and party equipment. */
typedef struct
{
    u8 pad_0[0x34];
    u32 techniques[FIELD_WEAPON_CATEGORY_COUNT];
    u32 abilities[3];
    u8 ability_proficiency[FIELD_ABILITY_PROFICIENCY_COUNT];
    u8 weapon_proficiency[FIELD_WEAPON_CATEGORY_COUNT];
    u8 pad_cf[0x5F0 - 0xCF];
    FieldProgressionCharacter characters[FIELD_PROGRESSION_PARTY_SIZE];
} FieldProgressionContext;

/** @brief Presence flags at the start of each runtime party record. */
typedef struct
{
    u8 flags;
    u8 pad_1[0x268 - 1];
} FieldProgressionPartyRecord;

extern FieldTechniqueUnlockRule g_field_technique_unlock_rules[];
/* Scene image setup also uses this flag; its broader purpose is unresolved. */
extern s32 D_80115890;

/**
 * @brief Advance active player proficiency and queue newly learned abilities and techniques.
 * @note Proficiency saturates at 100, advancing by four when D_80115890 is set, or one otherwise.
 * @note A technique rule can unlock silently, without adding a dialog entry.
 * @see decomp.me (100%) TODO
 */
void field_advance_ability_progression(void)
{
    extern FieldProgressionContext* g_pad_ctx;
    extern FieldProgressionPartyRecord g_field_player_records[];
    s32 ability_index;
    FieldAbilityUnlockRule* ability_base;
    s32 technique_index;
    FieldTechniqueUnlockRule* technique_base;
    FieldTechniqueUnlockRule* active_rule;
    FieldProgressionContext* context;
    FieldAbilityUnlockRule* ability_rule;
    FieldTechniqueUnlockRule* technique_rule;
    s32 technique_mask;
    s32 ability_word;
    s32 unlocked_ability_word;
    s32 equipped_weapon;
    s32 party_index;
    s32 technique_party_index;
    u32 weapon_category;
    u32 technique_group;
    u8 ability_proficiency;
    u8 weapon_requirement;
    s32 ability;
    u8 weapon_proficiency;

    /* Advance the equipped weapon and both abilities for each active player. */
    party_index = 0;
    do
    {
        if (g_field_player_records[party_index].flags & FIELD_PROGRESSION_ACTIVE_FLAG)
        {
            if ((u32)(g_pad_ctx->characters[party_index].character & FIELD_PROGRESSION_CHARACTER_MASK) < FIELD_PROGRESSION_PLAYER_COUNT)
            {
                weapon_category = (g_pad_ctx->characters[party_index].equipment >> FIELD_WEAPON_CATEGORY_SHIFT) & FIELD_WEAPON_CATEGORY_MASK;
                if (weapon_category < FIELD_WEAPON_CATEGORY_COUNT)
                {
                    weapon_proficiency = g_pad_ctx->weapon_proficiency[weapon_category];
                    if (weapon_proficiency < FIELD_PROFICIENCY_MAX)
                    {
                        if (D_80115890 != 0)
                        {
                            g_pad_ctx->weapon_proficiency[weapon_category] = weapon_proficiency + FIELD_PROFICIENCY_BONUS;

                            if (g_pad_ctx->weapon_proficiency[((g_pad_ctx->characters[party_index].equipment >> FIELD_WEAPON_CATEGORY_SHIFT) &
                                                               FIELD_WEAPON_CATEGORY_MASK)] > FIELD_PROFICIENCY_MAX)
                            {
                                g_pad_ctx->weapon_proficiency[((g_pad_ctx->characters[party_index].equipment >> FIELD_WEAPON_CATEGORY_SHIFT) &
                                                               FIELD_WEAPON_CATEGORY_MASK)] = FIELD_PROFICIENCY_MAX;
                            }
                        }
                        else
                        {
                            g_pad_ctx->weapon_proficiency[weapon_category] = weapon_proficiency + 1;
                        }
                    }
                }
                for (ability = 0; ability < FIELD_EQUIPPED_ABILITY_COUNT; ability++)
                {
                    ability_proficiency = g_pad_ctx->ability_proficiency[g_pad_ctx->characters[party_index].abilities[ability]];
                    if (ability_proficiency < FIELD_PROFICIENCY_MAX)
                    {
                        if (D_80115890 != 0)
                        {
                            g_pad_ctx->ability_proficiency[g_pad_ctx->characters[party_index].abilities[ability]] =
                                ability_proficiency + FIELD_PROFICIENCY_BONUS;

                            if (g_pad_ctx->ability_proficiency[g_pad_ctx->characters[party_index].abilities[ability]] > FIELD_PROFICIENCY_MAX)
                            {
                                g_pad_ctx->ability_proficiency[g_pad_ctx->characters[party_index].abilities[ability]] = FIELD_PROFICIENCY_MAX;
                            }
                        }
                        else
                        {
                            g_pad_ctx->ability_proficiency[g_pad_ctx->characters[party_index].abilities[ability]] = ability_proficiency + 1;
                        }
                    }
                }
            }
        }
        party_index++;
    } while (party_index < FIELD_PROGRESSION_PARTY_SIZE);
    context = g_pad_ctx;
    /* Ability rules require each nonempty prerequisite to be learned and trained. */
    ability_index = 0;
    ability_base = g_field_ability_unlock_rules;
    g_field_progression_unlock_count = 0;
    do
    {
        ability_rule = &ability_base[ability_index];
        ability = ability_rule->result;
        ability_word = ability / FIELD_UNLOCK_BITS_PER_WORD;
        if (!(context->abilities[ability_word] & (1 << (ability % FIELD_UNLOCK_BITS_PER_WORD))))
        {
            ability = ability_rule->prerequisites[0].ability;
            if (ability == FIELD_ABILITY_PREREQUISITE_NONE ||
                ((context->abilities[ability / FIELD_UNLOCK_BITS_PER_WORD] & (1 << (ability % FIELD_UNLOCK_BITS_PER_WORD))) &&
                 (context->ability_proficiency[ability] >= ability_rule->prerequisites[0].proficiency)))
            {
                ability = ability_rule->prerequisites[1].ability;
                if (ability == FIELD_ABILITY_PREREQUISITE_NONE ||
                    ((context->abilities[ability / FIELD_UNLOCK_BITS_PER_WORD] & (1 << (ability % FIELD_UNLOCK_BITS_PER_WORD))) &&
                     (context->ability_proficiency[ability] >= ability_rule->prerequisites[1].proficiency)))
                {
                    ability = ability_rule->result;
                    unlocked_ability_word = ability / FIELD_UNLOCK_BITS_PER_WORD;
                    context->abilities[unlocked_ability_word] =
                        (s32)(context->abilities[unlocked_ability_word] | (1 << (ability % FIELD_UNLOCK_BITS_PER_WORD)));
                    g_field_progression_unlocks[g_field_progression_unlock_count] = (s16)ability;
                    g_field_progression_unlock_count += 1;
                }
            }
        }
        ability_index++;
    } while ((s32)&ability_base[ability_index] < (s32)&ability_base[FIELD_ABILITY_UNLOCK_RULE_COUNT]);
    context = g_pad_ctx;
    /* Techniques additionally require an active player with the matching weapon. */
    technique_index = 0;
    technique_base = g_field_technique_unlock_rules;
    do
    {
        technique_rule = &technique_base[technique_index];
        technique_group = technique_rule->weapon >> FIELD_TECHNIQUE_GROUP_SHIFT;
        ability = technique_rule->result & FIELD_TECHNIQUE_INDEX_MASK;
        if (!(context->techniques[technique_group] & (1 << (ability % FIELD_UNLOCK_BITS_PER_WORD))))
        {
            ability = technique_rule->prerequisites[0].ability;
            if (ability == FIELD_ABILITY_PREREQUISITE_NONE ||
                ((context->abilities[ability / FIELD_UNLOCK_BITS_PER_WORD] & (1 << (ability % FIELD_UNLOCK_BITS_PER_WORD))) &&
                 (context->ability_proficiency[ability] >= technique_rule->prerequisites[0].proficiency)))
            {
                ability = technique_rule->prerequisites[1].ability;
                if (ability == FIELD_ABILITY_PREREQUISITE_NONE ||
                    ((context->abilities[ability / FIELD_UNLOCK_BITS_PER_WORD] & (1 << (ability % FIELD_UNLOCK_BITS_PER_WORD))) &&
                     (context->ability_proficiency[ability] >= technique_rule->prerequisites[1].proficiency)))
                {
                    ability = technique_rule->prerequisites[2].ability;
                    if (ability == FIELD_ABILITY_PREREQUISITE_NONE ||
                        ((context->abilities[ability / FIELD_UNLOCK_BITS_PER_WORD] & (1 << (ability % FIELD_UNLOCK_BITS_PER_WORD))) &&
                         (context->ability_proficiency[ability] >= technique_rule->prerequisites[2].proficiency)))
                    {
                        ability = technique_rule->prerequisites[3].ability;
                        if (ability == FIELD_ABILITY_PREREQUISITE_NONE ||
                            ((context->abilities[ability / FIELD_UNLOCK_BITS_PER_WORD] & (1 << (ability % FIELD_UNLOCK_BITS_PER_WORD))) &&
                             (context->ability_proficiency[ability] >= technique_rule->prerequisites[3].proficiency)))
                        {
                            technique_party_index = 0;
                            active_rule = technique_rule;
                            ability = technique_rule->result & FIELD_TECHNIQUE_INDEX_MASK;
                            do
                            {
                                if ((g_field_player_records[technique_party_index].flags & FIELD_PROGRESSION_ACTIVE_FLAG) &&
                                    ((u32)(context->characters[technique_party_index].character & FIELD_PROGRESSION_CHARACTER_MASK) <
                                     FIELD_PROGRESSION_PLAYER_COUNT))
                                {
                                    weapon_requirement = active_rule->weapon;
                                    equipped_weapon =
                                        (context->characters[technique_party_index].equipment >> FIELD_WEAPON_CATEGORY_SHIFT) & FIELD_WEAPON_CATEGORY_MASK;
                                    if (equipped_weapon == (weapon_requirement & FIELD_TECHNIQUE_WEAPON_MASK))
                                    {
                                        if (context->weapon_proficiency[equipped_weapon] >= active_rule->weapon_proficiency)
                                        {
                                            if (!(context->techniques[technique_group] & (technique_mask = 1 << (ability % FIELD_UNLOCK_BITS_PER_WORD))))
                                            {
                                                technique_group = weapon_requirement >> FIELD_TECHNIQUE_GROUP_SHIFT;
                                                context->techniques[technique_group] = (s32)(context->techniques[technique_group] | technique_mask);
                                                if (!(active_rule->result & FIELD_TECHNIQUE_SILENT_FLAG))
                                                {
                                                    g_field_progression_unlocks[g_field_progression_unlock_count] =
                                                        ((technique_group * FIELD_TECHNIQUES_PER_GROUP) + ability) | FIELD_TECHNIQUE_UNLOCK_FLAG;
                                                    g_field_progression_unlock_count += 1;
                                                }
                                            }
                                        }
                                    }
                                }
                                technique_party_index += 1;
                            } while (technique_party_index < FIELD_PROGRESSION_PARTY_SIZE);
                        }
                    }
                }
            }
        }
        technique_index++;
    } while ((s32)&technique_base[technique_index] < (s32)&technique_base[FIELD_TECHNIQUE_UNLOCK_RULE_COUNT]);
}

/* ---- Dialog screens (0x800A6EEC .. 0x800A88A0) ---- */

/** @brief Opaque pad-context handle; some users view it as other record types. */
typedef struct PadContext PadContext;

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

/** @brief Menu display record with packed flags, state, offsets, and update callback. */
typedef struct
{
    union
    {
        u32 word;
        struct
        {
            u16 low;
            s8 priority;
            u8 high;
        } fields;
    } flags;
    union
    {
        u32 word;
        struct
        {
            u16 low;
            s16 height;
        } fields;
    } state;
    s16 scroll_offset;
    s16 scroll_target;
    s16 scroll_tick;
    s16 pad_e;
    void (*update)(void);
} FieldMenuRecord;

/** @brief 0x268-byte entry whose first halfword carries per-slot flag bits. */
typedef struct {
    union { u16 h; struct { u8 unk0; u8 unk1; } b; } u0;
    u8 pad2[0x268 - 2];
} Entry268;

/** @brief 0x54-byte record exposing the halfword at 0x2A. */
typedef struct {
    u8 pad0[0x2A];
    s16 unk2A;
    u8 pad2C[0x54 - 0x2C];
} Rec54;

/** @brief 0x23C-byte state record exposing words at 0xC and 0x178. */
typedef struct {
    u8 pad0[0xC];
    s32 unkC;
    u8 pad10[0x178 - 0x10];
    s32 unk178;
    u8 pad17C[0x23C - 0x17C];
} State23C;

/** @brief Packed prompt flags accessed as both a word and individual fields. */
typedef union
{
    u32 word;
    struct
    {
        u32 low : 3;
        u32 type : 4;
        u32 priority : 9;
        u32 value : 8;
        u32 high : 8;
    } bits;
} FieldPromptFlags;

/** @brief Packed prompt state with an enable bit and an eight-bit value. */
typedef union
{
    u32 word;
    struct
    {
        u32 enabled : 1;
        u32 value : 8;
        u32 high : 23;
    } bits;
} FieldPromptState;

/** @brief Callback record returned by func_800ADF84. */
typedef struct
{
    FieldPromptFlags flags;
    FieldPromptState state;
    u8 pad8[8];
    void (*callback)(void);
} FieldADF84Rec;

/** @brief Partial slot record exposing the halfword cleared by func_800A74E8. */
typedef struct
{
    u8 pad0[0x260];
    s16 unk260;
    u8 pad262[0x268 - 0x262];
} RecFD818;

/** @brief Actor record prefix containing the active flag. */
typedef struct
{
    u8 active;
    u8 pad1[0x268 - 1];
} FieldActorEntry;

/** @brief 0x268-byte record exposing the two count bytes at 0x25C/0x25D. */
typedef struct
{
    u8 pad0[0x25C];
    u8 unk25C;
    u8 unk25D;
    u8 pad25E[0x268 - 0x25E];
} FieldEntry268;

/** @brief Active byte and fixed-point score accessed at offsets 0x5F0 and 0x610. */
typedef struct
{
    u8 pad0[0x5F0];
    u8 active;
    u8 pad5f1[0x1F];
    u32 score;
} FieldRankStats;

/** @brief Displayed numeric fields in a 0x268-byte player record. */
typedef struct
{
    u8 pad0[0x25A];
    u8 first;
    u8 second;
    u8 tail[0xC];
} FieldRankPlayer;

/** @brief Drawing position and three sorted actor indices sharing the local work area. */
typedef struct
{
    Vec2s position;
    s32 unused;
    s32 indices[3];
} FieldRankWork;

/** @brief Two-byte relative offset into the shared FIELD string table. */
typedef struct
{
    u8 unk0;
    u8 unk1;
} StructEC;

/** @brief Packed little-endian offset into the shared header text block. */
typedef struct PackedOffset
{
    unsigned char low;
    unsigned char high;
} PackedOffset;

/** @brief Text resource header with offsets to both item-name tables. */
typedef struct TextResource
{
    s32 unused;
    s32 normal;
    s32 special;
} TextResource;

/** @brief Eight-word animation lookup table copied to local storage. */
typedef struct
{
    s32 words[8];
} FieldQuadAnimationTable;

/* --- Shared (non-conflicting) extern data --- */
extern u8 D_8011F430[];
extern u8 D_800EC3D6[];
extern u8 D_800EC3C6[], D_800EC3DA[];
extern u8 g_field_dialog_item_quantities[];
extern unsigned char D_800EC3C4[];
extern void *g_field_dialog_item_texts[];
extern s32 D_801229A0[];
extern Rec54 g_field_actors[];
extern StructEC D_800EC3D8;
extern PackedOffset D_800EC3CC;
extern PackedOffset D_800EC3CE;
extern FieldQuadAnimationTable D_800513E8, D_80051408, D_80051428, D_80051448;
extern s32 D_801227EC;
extern s32 g_field_text_session_active;
extern s32 g_field_dialog_item_count;
extern s32 D_800F229C;
extern s32 D_801226D8;
extern s32 D_80122828;
extern s32 D_8011F420;
extern s32 g_pad_input;
extern s32 g_pending_game_state;
extern s32 g_field_return_to_title_prompt_state;
extern s32 g_field_return_to_title_prompt_delay;
extern s32 g_frame_counter;
extern s32 g_field_pending_spawn_id, g_field_pending_music_id, g_field_pending_secondary_music_id, g_field_pending_scene_id, g_field_pending_object_id, g_field_pending_sound_bank_id;

/* --- Shared (non-conflicting) extern function prototypes --- */
extern void func_800ADEB0(void);
extern void field_reset_input_repeat(void);
extern s32 func_800ADEEC(void);
extern void func_800ADF34(void);
extern void func_800A3938(s32 sound_id, s32 pan);


extern void field_begin_return_to_title_prompt_close(void);
extern void func_8006809C(void);
extern void func_800AE8A8(void);
extern void func_800AED20(void);
extern void func_800B661C(s32 arg0, FieldADF84Rec *arg1);
extern void akao_cmd_f1(void);
extern void func_800AE9E0(void);
extern s32 func_800B0888(void *arg0);
extern s32 field_emit_actor_portrait(s32, s32, s32, Vec2s *);

/* --- Forward prototypes for members defined later (real signatures) --- */
void func_800A7384(void);
void func_800A764C(void);
void func_800A7724(void);
void func_800A788C(void *ot, void *cursor, s32 x_offset, s32 y_offset);
s32 func_800A7B54(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
s32 func_800A7FB4(s32 *ot, s32 prim, s32 arg2, s32 arg3);
s32 func_800A8128(s32 ordering_table, s32 cursor, s32 scroll_x, s32 scroll_y, s32 viewport_height);
void *func_800A8524(s32 *ordering_table, POLY_FT4 *prim, s32 x, s32 y);
void *func_800A8660(s32 *ordering_table, POLY_FT4 *prim, s32 x, s32 y);

/**
 * @brief Snapshot the pad context into the field save buffer.
 */
void func_800A6EEC(void)
{
    extern void *bcopy(const void *, void *, int);
    extern PadContext *g_pad_ctx;

    bcopy(g_pad_ctx, D_8011F430, 0x3268);
}

/**
 * @brief Process the return-to-title choice or restore the saved field scene.
 */
void func_800A6F1C(void)
{
    extern void bcopy(void *, void *, s32);
    extern Pad *g_pad_ctx;
    extern u8 g_field_player_records[];
    extern Actor g_field_object_states[];

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
            field_rebuild_party_actions(0);
            field_set_scene_parameters(g_field_pending_scene_id, g_field_pending_object_id, g_field_pending_spawn_id, g_field_pending_music_id, g_field_pending_sound_bank_id,
                                       g_field_pending_secondary_music_id);
            actor_index = 0;
            low_mask = 0xFFFFFF;
            high_mask = 0xFF000000;
            sentinel = 0xFF;
            actor_flags = g_field_player_records;
            actor = g_field_object_states;
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

/**
 * @brief Route a confirm/cancel pad press to the active field sub-dialog.
 *
 * When no modal is blocking (func_800ADEEC returns 0) and a confirm or cancel
 * button is held (@c g_pad_input & 0x220), dispatches on the current dialog
 * mode @c D_800F229C: mode 1 advances via func_800A7384 (gated by
 * @c D_801227EC); mode 3 backs out via func_800A764C when @c g_field_dialog_item_count is set,
 * else falls through to the mode-2 handler func_800A7724.
 *
 * @see decomp.me (100%) TODO
 */
void func_800A710C(void)
{
    if ((func_800ADEEC() == 0) && (g_pad_input & 0x220))
    {
        switch (D_800F229C)
        {
        case 1:
            if (D_801227EC == 0)
            {
                func_800A7384();
                return;
            }
            break;
        case 3:
            if (g_field_dialog_item_count != 0)
            {
                func_800A764C();
                return;
            }
            /* fallthrough */
        case 2:
            func_800A7724();
            break;
        }
    }
}

/**
 * @brief Load menu data and size its display record for both entry categories.
 */
void func_800A71CC(void)
{
    FieldMenuRecord *func_800ADF84(void);
    extern s32 D_8010D038;

    s16 raw_height;
    s32 height;
    u16 entry_count;
    s32 entry_limit;
    s32 padding;
    s32 index;
    s32 saw_special;
    s32 saw_normal;
    u16 *entry;
    u32 large_state;
    u32 state;
    u32 small_state;
    FieldMenuRecord *record;

    cdrom_queue_read(0x5DF, D_8010D038);
    cdrom_wait_queue_empty();
    func_800A3938(0xA1, 0x80);
    D_800F229C = 3;
    func_800ADF34();
    record = func_800ADF84();
    record->flags.word = (s32)((((record->flags.word & ~0x78) | 8) & 0xFFFF007F) | 0x1000);
    saw_normal = 0;
    saw_special = 0;
    index = 0;
    padding = 0;
    entry_count = g_field_progression_unlock_count;
    if (entry_count != 0)
    {
        entry_limit = entry_count;
        entry = g_field_progression_unlocks;
        do
        {
            if (*entry & 0x8000)
            {
                if (saw_special == 0)
                {
                    saw_special = 1;
                    padding += 0x10;
                }
            }
            else if (saw_normal == 0)
            {
                saw_normal = 1;
                padding += 0x10;
            }
            index += 1;
            entry++;
        } while (index < entry_limit);
    }

    record->update = &func_800A8128;
    record->flags.word = (s32)(record->flags.word & 0xFFFFFF);
    state = record->state.word;
    state |= 1;
    record->state.word = state;
    raw_height = (g_field_progression_unlock_count * 0x10) + padding;
    record->state.word = state | 0x200;
    record->state.fields.height = raw_height;
    height = raw_height;
    if (height < 0xA1)
    {
        small_state = (record->state.word & ~0x1FE) | ((height & 0xFF) * 2);
        record->state.word = small_state;
        record->flags.fields.priority = (s8)(0x70 - ((small_state >> 2) & 0x7F));
        return;
    }

    record->scroll_offset = 0;
    record->scroll_target = 0;
    record->scroll_tick = 0;
    large_state = (((record->state.word & ~0xC00) | 0x400) & ~0x1FE) | 0x140;
    record->state.word = large_state;
    record->flags.fields.priority = (s8)(0x70 - ((large_state >> 2) & 0x50));
}

/**
 * @brief Clear pending selection flags across the three actor slots.
 */
void func_800A7384(void)
{
    extern Entry268 g_field_player_records[];
    extern State23C g_field_object_states[];

    s32 i;

    g_field_text_session_active = 0;
    i = 0;
    do {
        if ((g_field_player_records[i].u0.b.unk0 & 1) && g_field_actors[i].unk2A == 0x8E) {
            g_field_actors[i].unk2A = 0;
        }
        i++;
    } while (i < 3);

    func_8006809C();

    i = 0;
    do {
        g_field_object_states[i].unkC = 0;
        g_field_object_states[i].unk178 &= ~0x20;
        i++;
    } while (i < 3);
    g_field_dialog_item_count = 0;
}

/**
 * @brief Reset field sub-state and dispatch to one of three handlers.
 *
 * Runs the shared reset (func_800ADEB0, then latches @c D_801227EC to 4 and
 * calls field_reset_input_repeat), then selects a handler by state: func_800A71CC when
 * @c g_field_progression_unlock_count is set, else func_800A764C when @c g_field_dialog_item_count is set, else
 * func_800A7724. Finishes with func_800B0A08(0).
 *
 * @see decomp.me (100%) TODO
 */
void func_800A7434(void)
{
    extern void func_800B0A08(s32 arg0);

    func_800ADEB0();
    D_801227EC = 4;
    field_reset_input_repeat();
    if (g_field_progression_unlock_count != 0)
    {
        func_800A71CC();
    }
    else if (g_field_dialog_item_count != 0)
    {
        func_800A764C();
    }
    else
    {
        func_800A7724();
    }
    func_800B0A08(0);
}

/**
 * @brief Finish a field sub-dialog and return to the base field loop.
 */
void func_800A74B8(void)
{
    field_reset_input_repeat();
    func_800B0A08(0);
    field_begin_duel_result();
}

/**
 * @brief Configure the return-to-title prompt and reset its three slot values.
 * @note The mixed word and bitfield updates preserve the original store widths.
 */
void func_800A74E8(void)
{
    extern FieldADF84Rec *func_800ADF84(void);
    extern RecFD818 g_field_player_records[];

    FieldADF84Rec *rec;
    u32 state;
    s32 i;

    func_800ADEB0();
    func_800A3938(0xB9, 0x80);

    rec = func_800ADF84();
    rec->callback = func_800AE8A8;
    rec->flags.bits.type = 1;
    rec->flags.bits.priority = 0x20;
    state = (rec->state.word | 1) & ~0x1FE;
    rec->flags.bits.value = 0x30;
    state |= 0x60;
    rec->state.word = state;
    rec->flags.word &= 0xFFFFFF;

    rec = func_800ADF84();
    rec->callback = func_800AED20;
    rec->flags.bits.type = 1;
    rec->flags.bits.priority = 0x50;
    rec->state.bits.enabled = 0;
    rec->state.bits.value = 0x22;
    rec->flags.bits.value = 0x80;

    rec->flags.word = (rec->flags.word & 0xFFFFFF) | 0xA0000000;

    /* Keep the shared -2 value in the target's argument register. */
    do
    {
        func_800B661C(-2, rec);
    } while (0);
    akao_cmd_f1();

    g_field_return_to_title_prompt_delay = 0x3C;
    g_field_return_to_title_prompt_state = 3;
    D_801226D8 = 0;
    func_800AE9E0();

    for (i = 2; i >= 0; i--)
    {
        g_field_player_records[i].unk260 = 0;
    }
}

/**
 * @brief Configure a field prompt record for the current selection state.
 */
void func_800A764C(void)
{
    extern FieldADF84Rec *func_800ADF84(void);

    FieldADF84Rec *rec;
    u32 state;

    D_800F229C = 2;
    func_800A3938(0xB9, 0x80);
    func_800ADF34();

    rec = func_800ADF84();
    rec->flags.bits.type = 1;
    rec->flags.bits.priority = 0x40;

    state = rec->state.word & ~0x1FE;
    state |= (((g_field_dialog_item_count << 4) + 0x10) & 0xFF) << 1;
    rec->flags.bits.value = 0x70 - ((state >> 2) & 0x78);

    rec->callback = func_800A7FB4;
    rec->state.word = state;
    rec->flags.word = (rec->flags.word & 0xFFFFFF) | 0xC0000000;
    rec->state.bits.enabled = 0;
}

/**
 * @brief Initialize two field processes and position the second from the active actor count.
 */
void func_800A7724(void)
{
    extern FieldADF84Rec *func_800ADF84(void);
    extern FieldActorEntry g_field_player_records[];

    FieldADF84Rec *record;
    s32 actor_index;
    s32 active_count;

    func_800ADF34();
    D_800F229C = 1;
    func_800A3938(0xB9, 0x80);

    active_count = 0;
    record = func_800ADF84();
    actor_index = active_count;
    record->callback = func_800A788C;
    record->flags.bits.type = 1;
    record->flags.bits.priority = 0x20;
    record->flags.bits.value = 0x30;
    record->state.bits.enabled = 1;
    record->state.bits.value = 0x20;
    record->flags.word &= 0xFFFFFF;

    for (actor_index = 0; actor_index < 3; actor_index++)
    {
        if (g_field_player_records[actor_index].active & 1)
        {
            active_count++;
        }
    }

    record = func_800ADF84();
    record->callback = func_800A7B54;
    record->flags.bits.type = 1;
    record->flags.bits.priority = 0x20;
    record->flags.bits.value = 0x58;
    record->state.bits.enabled = 1;
    record->state.bits.value = ((active_count * 28) + 16) & 0xFF;
    record->flags.word &= 0xFFFFFF;
}

/**
 * @brief Draw the field process/actor summary row.
 * @param ot Ordering-table address used by the drawing helpers.
 * @param cursor Current primitive-buffer cursor.
 * @param x_offset Horizontal drawing origin subtracted from each column.
 * @param y_offset Vertical drawing origin subtracted from each row.
 */
void func_800A788C(void *ot, void *cursor, s32 x_offset, s32 y_offset)
{
    extern u8 *g_pad_ctx;
    extern FieldEntry268 g_field_player_records[];
    void *func_800A88A0(void *sprite_cursor, void *ot, u8 *text, s32 color, s32 x, s32 y, s32 flags);
    void *func_800A8A78(void *ot, void *cursor, s32 value, s32 color, s16 *position, s32 flags);

    s32 i;
    s32 total_x;
    s32 total_y;
    u8 *entry;
    u8 *pad;
    volatile u8 *text_base;
    void *handle;
    void *first_cursor;
    s16 position[2];
    s16 y;

    if (D_801227EC != 0)
    {
        first_cursor = cursor;
        if ((x_offset | y_offset) != 0)
        {
            goto loop_setup;
        }
        D_801227EC--;
        if (D_801227EC != 0)
        {
            goto loop_setup;
        }
        if (func_800B0888(first_cursor) != 0)
        {
            D_801227EC = 1;
        }
    }

    first_cursor = cursor;
loop_setup:
    i = 0;
    total_x = i;
    total_y = i;
    entry = (u8 *)g_field_player_records;
    pad = g_pad_ctx;
    do
    {
        if (pad[0x5F0] != 0)
        {
            FieldEntry268 *record = (FieldEntry268 *)entry;

            total_x += record->unk25C;
            total_y += record->unk25D;
        }
        entry += sizeof(FieldEntry268) - sizeof(((FieldEntry268 *)0)->pad25E);
        entry += sizeof(((FieldEntry268 *)0)->pad25E);
        i++;
        pad += 0x250;
    } while (i < 3);

    handle = func_800A88A0(first_cursor, ot,
        D_800EC3D6[0] + (D_800EC3D6 - 0x12) + (D_800EC3D6[1] << 8),
        4, 0x10 - x_offset, -y_offset, 0);
    text_base = D_800EC3D6 - 0x12;

    y = 0x10 - y_offset;
    position[1] = y;
    handle = func_800A8660(ot, handle, 0x18 - x_offset, y - 3);

    position[0] = 0x28 - x_offset;
    handle = func_800A8A78(ot, handle, total_x, 4, position, 0);
    handle = func_800A88A0(handle, ot,
        text_base[0x16] + ((text_base[0x17] << 8) + text_base),
        4, 0x40 - x_offset, position[1], 0);
    handle = func_800A8524(ot, handle, 0x50 - x_offset, position[1] + 2);

    position[0] = 0x60 - x_offset;
    handle = func_800A8A78(ot, handle, total_y, 4, position, 0);
    handle = func_800A88A0(handle, ot,
        text_base[0x1A] + ((text_base[0x1B] << 8) + text_base),
        4, 0x80 - x_offset, position[1], 0);

    position[0] = 0xD0 - x_offset;
    handle = func_800A8A78(ot, handle, *(s32 *)(g_pad_ctx + 0x2C) - D_8011F420, 4, position, 1);
    func_800A88A0(handle, ot,
        text_base[0] + ((text_base[1] << 8) + text_base),
        4, 0xF0 - x_offset, position[1], 1);
}

/**
 * @brief Draw up to three active players in descending adjusted-score order.
 * @param arg0 Ordering table address passed to the drawing helpers.
 * @param arg1 Initial primitive-buffer address.
 * @param arg2 Horizontal drawing origin subtracted from each column position.
 * @param arg3 Vertical drawing origin subtracted from each row position.
 * @return Primitive-buffer address after the final emitted element.
 * @note Equal adjusted scores retain player order during insertion sorting.
 */
s32 func_800A7B54(s32 arg0, s32 arg1, s32 arg2, s32 arg3)
{
    extern PadContext *g_pad_ctx;
    extern FieldRankPlayer g_field_player_records[];
    extern s32 func_800A88A0(s32, s32, u8 *, s32, s32, s32, s32);
    extern s32 func_800A838C(s32, s32, s32, s32, s32);
    extern s32 func_800A8A78(s32, s32, s32, s32, Vec2s *, s32);

    FieldRankWork work;
    Vec2s *position;
    s32 count;
    s32 i;
    s32 result;
    s32 *score;
    s32 *end;
    s32 *work_base;
    FieldRankStats *stats;
    FieldRankStats *active_stats;
    FieldRankStats *base;
    s32 *scores;
    s32 pos;
    s32 byte_offset;
    s32 stats_addr;
    s32 j;
    s32 id;
    s32 *dst;
    s32 *src;
    s32 row;
    s16 y;
    u8 *text;
    u8 *text_base;

    result = func_800A88A0(arg1, arg0, (D_800EC3C6[1] << 8) + ((D_800EC3C6 - 2) + D_800EC3C6[0]), 4, 0x10 - arg2, -arg3, 0);
    count = 0;
    i = count;
    scores = D_801229A0;
    base = (FieldRankStats *)g_pad_ctx;
    work_base = (s32 *)((u8 *)&work + ((u8 *)scores - (u8 *)D_801229A0));
    score = scores;
    end = work_base;
    do
    {
        stats = (FieldRankStats *)((u8 *)base + i * 0x250);
        if (stats->active != 0)
        {
            pos = 0;
            while (pos < count && (s32)((((FieldRankStats *)(stats_addr = (s32)base + work_base[pos + 2] * 0x250, (u8 *)stats_addr))->score >> 8) - scores[work_base[pos + 2]]) >= (s32)((stats->score >> 8) - *score))
            {
                pos += 2;
                pos--;
            }
            if (pos == count)
            {
                end[2] = i;
            }
            else
            {
                j = count - 1;
                if (j >= pos)
                {
src = &work.indices[1];
                    dst = (s32 *)(j * 4 + (s32)src);
                    src = (s32 *)(j * 4 + (s32)work_base);
                    do
                    {
                        *dst = src[2];
                        src--;
                        j--;
                        dst--;
                    } while (j >= pos);
                }
                work_base[pos + 2] = i;
            }
            end++;
            count++;
        }
        score++;
        i++;

    } while (i < 3);
    do
        {
            i = 0;
        } while (0);
    if (count > 0)
    {
        text_base = D_800EC3DA;
        text = text_base - 0x16;
        row = i;
        position = &work.position;
        do
        {
            active_stats = (FieldRankStats *)((u8 *)g_pad_ctx + ((FieldRankWork *)((u8 *)position + i * 4))->indices[0] * 0x250);
            if (active_stats->active != 0)
            {
                work.position.x = 0x18 - arg2;
                y = arg3 - 0x10;
                y = row - y;
                work.position.y = y;
                result = field_emit_actor_portrait(result, arg0, ((FieldRankWork *)((u8 *)position + i * 4))->indices[0], position);
                y += 8;
                work.position.y = y;
                result = func_800A838C(arg0, result, 0x38 - arg2, y - 8, 1);
                work.position.x = 0x48 - arg2;
                result = func_800A8A78(arg0, result, g_field_player_records[((FieldRankWork *)((u8 *)position + i * 4))->indices[0]].first, 4, position, 0);
                result = func_800A88A0(result, arg0, D_800EC3DA[0] + ((text_base[1] << 8) + (s32)text), 4, 0x68 - arg2, work.position.y, 0);
                result = func_800A838C(arg0, result, 0x78 - arg2, work.position.y, 0);
                work.position.x = 0x88 - arg2;
                work.position.y = y;
                result = func_800A8A78(arg0, result, g_field_player_records[((FieldRankWork *)((u8 *)position + i * 4))->indices[0]].second, 4, position, 0);
                result = func_800A88A0(result, arg0, text[0x1A] + ((text[0x1B] << 8) + (s32)text), 4, 0xA8 - arg2, work.position.y, 0);
                work.position.x = 0xE0 - arg2;
                work.position.y = y;
                id = ((FieldRankWork *)((u8 *)position + i * 4))->indices[0];
                stats = (FieldRankStats *)((u8 *)g_pad_ctx + id * 0x250);
                result = func_800A8A78(arg0, result, (stats->score >> 8) - D_801229A0[id], 4, position, 1);
            }
            row += 0x1C;
            i++;

        } while (i < count);
    }
    return result;
}

/**
 * @brief Draw a FIELD text list and the nonzero amount beside each entry.
 * @param ot Ordering table receiving the generated primitives.
 * @param prim Current primitive-chain handle.
 * @param arg2 Horizontal origin adjustment, also used by the amount row position.
 * @param arg3 Vertical origin adjustment for the list text.
 * @return Updated primitive-chain handle after drawing the list.
 */
s32 func_800A7FB4(s32 *ot, s32 prim, s32 arg2, s32 arg3)
{
    extern s32 func_800A88A0(s32 prim, s32 *ot, void *tex, s32 arg3, s32 x, s32 y, s32 z);
    extern s32 func_800A8A78(s32 *ot, s32 prim, u32 val, s32 arg3, Vec2s *pos, s32 arg5);

    s32 i;
    Vec2s pos;
    u8 pad[0x84];
    s32 row;
    s32 low;
    s32 offset;
    u8 *tex;

    low = D_800EC3D8.unk0;
    offset = (D_800EC3D8.unk1 << 8) + (s32)((u8 *)&D_800EC3D8 - 0x14);
    tex = (u8 *)(low + offset);
    prim = func_800A88A0(prim, ot, tex, 4, 0x20 - arg2, -arg3, 0);
    i = 0;
    if (g_field_dialog_item_count > 0)
    {
        do
        {
            row = i * 0x10;
            prim = func_800A88A0(prim, ot, g_field_dialog_item_texts[i], 4, 0x10 - arg2, (row + 0x10) - arg3, 0);
            pos.x = 0xB0 - arg2;
            pos.y = (row + 0x10) - arg2;
            if (g_field_dialog_item_quantities[i] != 0)
            {
                prim = func_800A8A78(ot, prim, g_field_dialog_item_quantities[i], 4, &pos, 1);
            }
            i += 1;
        } while (i < g_field_dialog_item_count);
    }
    return prim;
}

/**
 * @brief Draw the visible rows of the two-group item list, inserting each group header once.
 *
 * @param ordering_table Ordering-table address used by the text renderer.
 * @param cursor Current primitive buffer cursor.
 * @param scroll_x Horizontal scroll offset.
 * @param scroll_y Vertical scroll offset.
 * @param viewport_height Bottom clipping boundary.
 * @return Primitive buffer cursor after drawing the visible text.
 */
s32 func_800A8128(s32 ordering_table, s32 cursor, s32 scroll_x, s32 scroll_y, s32 viewport_height)
{
    s32 func_800A88A0(s32, s32, void *, s32, s32, s32, s32); /* extern */
    extern TextResource *D_8010D038;

    unsigned char pad[8];
    unsigned char *normal_names;
    unsigned char *special_names;
    s32 header_x;
    s32 item_x;
    s32 normal_header_y;
    s32 special_header_y;
    s32 next_cursor;
    s32 special_header_drawn;
    s32 row;
    s32 index;
    s32 normal_header_drawn;
    s32 special_row_y;
    s32 normal_row_y;
    s32 item_y;
    u16 *entry;
    u16 name_index;
    s32 entry_count;

    next_cursor = cursor;
    special_header_drawn = 0;
    row = 0;
    normal_header_drawn = 0;
    index = 0;
    normal_names = (unsigned char *)D_8010D038 + D_8010D038->normal;
    special_names = (unsigned char *)D_8010D038 + D_8010D038->special;
    if (g_field_progression_unlock_count != 0)
    {
        header_x = 0x20 - scroll_x;
        item_x = 0x80 - scroll_x;
        entry = g_field_progression_unlocks;
        do
        {
            if (*entry & 0x8000)
            {
                special_row_y = row * 0x10;
                if (special_header_drawn == 0)
                {
                    special_header_y = special_row_y - scroll_y;
                    if ((special_header_y >= -0xF) && (special_header_y < viewport_height))
                    {
                        s32 low;
                        s32 offset;
                        unsigned char *base;

                        low = D_800EC3CE.low;
                        base = D_800EC3C4;
                        offset = (D_800EC3CE.high << 8) + (s32)base;
                        next_cursor = func_800A88A0(next_cursor, ordering_table, (void *)(low + offset), 4, header_x, special_header_y, 0);
                    }
                    special_header_drawn = 1;
                    row += 1;
                    special_row_y = row * 0x10;
                }
                item_y = special_row_y - scroll_y;
                if (item_y >= -0xF)
                {
                    if (item_y < viewport_height)
                    {
                        name_index = *entry & 0x7FFF;
                        next_cursor = func_800A88A0(next_cursor, ordering_table,
                                                    special_names +
                                                        ((u16 *)special_names)[name_index],
                                                    4, item_x, item_y, 2);
                    }
                }
            }
            else
            {
                normal_row_y = row * 0x10;
                if (normal_header_drawn == 0)
                {
                    normal_header_y = normal_row_y - scroll_y;
                    if ((normal_header_y >= -0xF) && (normal_header_y < viewport_height))
                    {
                        next_cursor =
                            func_800A88A0(next_cursor, ordering_table,
                                          D_800EC3C4 + D_800EC3CC.low + (D_800EC3CC.high << 8),
                                          4, header_x, normal_header_y, 0);
                    }
                    normal_header_drawn = 1;
                    row += 1;
                    normal_row_y = row * 0x10;
                }
                item_y = normal_row_y - scroll_y;
                if (item_y >= -0xF)
                {
                    if (item_y < viewport_height)
                    {
                        name_index = *entry;
                        next_cursor = func_800A88A0(next_cursor, ordering_table,
                                                    normal_names +
                                                        ((u16 *)normal_names)[name_index],
                                                    4, item_x, item_y, 2);
                    }
                }
            }
            row += 1;
            entry += 1;
            do
            {
                entry_count = g_field_progression_unlock_count;
                index += 1;
            } while (0);
        } while (index < entry_count);
    }
    return next_cursor;
}

/**
 * @brief Build and link an animated textured quad primitive.
 * @param ordering_table Ordering-table tag to link the primitive into.
 * @param prim Primitive buffer slot to populate.
 * @param x Left screen coordinate.
 * @param y Top screen coordinate.
 * @param wide Selects the larger geometry and texture region when nonzero.
 * @return Pointer just past the emitted primitive.
 */
POLY_FT4* func_800A838C(u32* ordering_table, POLY_FT4* prim, s16 x, s16 y, s32 wide)
{
    s32 phase_table[8] = {0, 1, 2, 3, 2, 1, 0, 0};
    s32 phase;

    phase = phase_table[((g_frame_counter >> 2) + 1) & 7];

    *(u32*)&prim->r0 = 0x808080;
    setPolyFT4(prim);
    prim->x2 = x;
    prim->x0 = x;
    prim->y1 = y;
    prim->y0 = y;

    if (wide != 0)
    {
        prim->x3 = x + 12;
        prim->x1 = x + 12;
        prim->y3 = y + 24;
        prim->y2 = y + 24;
        prim->u2 = phase * 12 + 0x20;
        prim->u0 = phase * 12 + 0x20;
        prim->u3 = phase * 12 + 0x2C;
        prim->u1 = phase * 12 + 0x2C;
        prim->v1 = 0x60;
        prim->v0 = 0x60;
        prim->v3 = 0x78;
        prim->v2 = 0x78;
    }
    else
    {
        prim->x3 = x + 8;
        prim->x1 = x + 8;
        prim->y3 = y + 16;
        prim->y2 = y + 16;
        prim->u2 = phase * 8 - 0x48;
        prim->u0 = phase * 8 - 0x48;
        prim->u3 = phase * 8 - 0x40;
        prim->u1 = phase * 8 - 0x40;
        prim->v1 = 0x40;
        prim->v0 = 0x40;
        prim->v3 = 0x50;
        prim->v2 = 0x50;
    }

    prim->clut = 0x7A87;
    prim->tpage = 0x26;
    addPrim(ordering_table, prim);
    return prim + 1;
}

/**
 * @brief Build and link an animated eight-pixel textured quad.
 * @param ordering_table Ordering-table tag receiving the primitive.
 * @param prim Primitive buffer to populate.
 * @param x Left screen coordinate.
 * @param y Top screen coordinate.
 * @return Buffer address immediately after the emitted primitive.
 */
void *func_800A8524(s32 *ordering_table, POLY_FT4 *prim, s32 x, s32 y)
{
    FieldQuadAnimationTable table;
    s32 phase_index;
    s32 frame;
    u32 color;
    s32 phase;
    u16 left_u;
    u16 right_u;

    table = D_800513E8;
    color = 0x808080;
    frame = g_frame_counter;
    phase_index = ((frame >> 2) + 2) & 7;
    phase = table.words[phase_index];

    *(u32 *)&prim->r0 = color;
    setPolyFT4(prim);
    prim->x2 = x;
    prim->x0 = x;
    prim->x3 = x + 8;
    prim->x1 = x + 8;
    prim->y1 = y;
    prim->y0 = y;
    prim->y3 = y + 8;
    prim->y2 = y + 8;
    prim->v1 = 0x78;
    prim->v0 = 0x78;
    prim->v3 = 0x80;
    prim->v2 = 0x80;
    prim->clut = 0x7A87;
    prim->tpage = 0x26;

    phase *= 8;
    left_u = phase + 0x20;
    right_u = phase + 0x28;
    prim->u3 = right_u;
    prim->u1 = right_u;
    prim->u2 = left_u;
    prim->u0 = left_u;

    addPrim(ordering_table, prim);
    return (u8 *)prim + 0x28;
}

/**
 * @brief Append an animated textured quad to an ordering table.
 * @param ordering_table Ordering-table entry receiving the primitive.
 * @param prim Writable primitive buffer.
 * @param x Horizontal origin.
 * @param y Vertical origin.
 * @return Buffer address immediately after the emitted primitive.
 */
void *func_800A8660(s32 *ordering_table, POLY_FT4 *prim, s32 x, s32 y)
{
    s32 frame;
    u32 color;
    FieldQuadAnimationTable tables[4];
    u16 left_x;
    u16 right_x;
    u16 width;
    u8 texture_u;
    s32 phase_index;

    tables[0] = D_800513E8;
    tables[1] = D_80051408;
    tables[2] = D_80051428;
    tables[3] = D_80051448;
    color = 0x808080;
    frame = g_frame_counter;
    *(u32 *)&prim->r0 = color;
    setPolyFT4(prim);
    phase_index = ((frame >> 2) + 3) & 7;
    left_x = *(u16 *)&tables[1].words[phase_index] + x;
    prim->x2 = left_x;
    prim->x0 = left_x;
    width = *(u16 *)&tables[3].words[phase_index];
    prim->y1 = y;
    prim->y0 = y;
    y += 0x10;
    prim->y3 = y;
    prim->y2 = y;
    right_x = left_x + width;
    prim->x3 = right_x;
    prim->x1 = right_x;
    texture_u = *(u8 *)&tables[2].words[phase_index];
    prim->u2 = texture_u;
    prim->u0 = texture_u;
    texture_u += *(u8 *)&tables[3].words[phase_index];
    prim->u3 = texture_u;
    prim->u1 = texture_u;
    prim->v1 = 0x80;
    prim->v0 = 0x80;
    prim->v3 = 0x90;
    prim->v2 = 0x90;
    prim->clut = 0x7A87;
    prim->tpage = 0x26;
    addPrim(ordering_table, prim);
    return prim + 1;
}

/**
 * @brief Thin wrapper delegating to the field menu-window handler.
 */
void func_800A8880(void)
{
    func_800AE008();
}
