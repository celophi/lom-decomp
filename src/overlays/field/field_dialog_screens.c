/** @file field_dialog_screens.c
 * @brief Timed panels, actor texts, ability progression and the battle result screens.
 *
 * One translation unit covering 0x800A5638 .. 0x800A88A0: the timed image
 * panel started by a script, the short texts shown over actors (technique
 * names), weapon/ability proficiency and unlocks after a battle, the result
 * windows (party summary and experience ranking, items, unlocks) and the
 * return-to-title prompt with its battle retry.
 */

#include "common.h"
#include "field_actor_runtime.h"
#include "field_calls.h"
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
#include "field_records.h"
#include "controller_internal.h"
#include "field_menu_element.h"
#include "field_actor_tables.h"
#include "field_runtime.h"
#include "pad.h"
#include "gpu_packet.h"
#include "game_state.h"
#include "sdk/memory.h"

/* main.h declares this as PadContext *; FIELD reads the same saved game through FieldGameState. */
extern FieldGameState* g_pad_ctx;
extern u8* g_field_cd_buffer;

/* ---- Timed panels (0x800A5638 .. 0x800A6204) ---- */

/** @brief Image resources of the timed panels (index 0 first). */
#define FIELD_TIMED_PANEL_RESOURCE_BASE 0x1060
/** @brief VRAM position of the panel image and its palette row. */
#define FIELD_TIMED_PANEL_VRAM_X 320
#define FIELD_TIMED_PANEL_CLUT_Y 498
/** @brief Frames a panel stays up, and the lengths of its fades. */
#define FIELD_TIMED_PANEL_FRAMES 150
#define FIELD_TIMED_PANEL_FADE_IN_FRAMES 32
#define FIELD_TIMED_PANEL_FADE_OUT_START 32
/** @brief Screen fade length when the panel opens and closes. */
#define FIELD_TIMED_PANEL_FADE_FRAMES 20
/** @brief Confirm skips to the fade-out while the timer is below this. */
#define FIELD_TIMED_PANEL_SKIP_END 103
/** @brief Full brightness, and the level below which the fade primitive is added. */
#define FIELD_TIMED_PANEL_FULL_BRIGHTNESS 0x80
#define FIELD_TIMED_PANEL_FADE_LIMIT 0x7C
/** @brief Mode byte: bit 7 selects a quad effect group (low bits), else the plain image. */
#define FIELD_TIMED_PANEL_EFFECT 0x80
#define FIELD_TIMED_PANEL_EFFECT_MASK 0x7F

/** @brief Opening and closing sounds of the panel variants. */
#define FIELD_SOUND_PANEL_IMAGE_START 0x118
#define FIELD_SOUND_PANEL_IMAGE_END 0x119
#define FIELD_SOUND_PANEL_EFFECT1_START 0x11A
#define FIELD_SOUND_PANEL_EFFECT1_END 0x11B
#define FIELD_SOUND_PANEL_EFFECT0_START 0x11C
#define FIELD_SOUND_PANEL_EFFECT0_END 0x11D
#define FIELD_SOUND_PANEL_EFFECT2_START 0x11E
#define FIELD_SOUND_PANEL_EFFECT2_END 0x11F
/** @brief Centered pan for field_play_sound(). */
#define FIELD_SOUND_CENTER 0x80

/** @brief Confirm buttons of the result and panel screens. */
#define FIELD_PAD_CONFIRM (PAD_BTN_CROSS | PAD_BTN_L3)

/** @brief A 16-color palette strip (one row of a portrait image). */
typedef struct
{
    u16 colors[16];
} FieldPortraitPalette;

/** @brief FieldTransitionQuad::x of an unused descriptor. */
#define FIELD_TRANSITION_QUAD_UNUSED 0xFFFF
/** @brief FieldTransitionQuad::flags: bits 0-1 texture mode, 4-7 palette column, 9-10 blend, 11-14 effect. */
#define FIELD_TRANSITION_QUAD_HALF_WIDTH 0xF
#define FIELD_TRANSITION_QUAD_SEMI_TRANSPARENT 0x100
#define FIELD_TRANSITION_QUAD_EFFECT(flags) (((flags) >> 11) & 0xF)

/** @brief Position, texture coordinates, dimensions and effect flags of one quad. */
typedef struct
{
    u16 x, y;
    u8 u, v;
    u16 width, height, flags;
} FieldTransitionQuad;

extern FieldPortraitPalette g_field_portrait_palettes[];
extern FieldTransitionQuad g_field_timed_panel_quads[][4];
extern u8 g_field_timed_panel_modes[];
/** @brief Nonzero while a timed panel is shown. */
extern s32 D_800F2298;
extern Vec2s g_field_screen_scroll;
extern u16 g_field_timed_panel_width;
extern u16 g_field_timed_panel_height;
extern s32 g_field_timed_panel_index;
extern u16 g_field_timed_panel_brightness;
extern u16 g_field_timed_panel_timer;
extern s32 g_pad_input;

s32 field_load_vram_resource(s32 id, RECT* rect, s32 mode);
s32 rcos(s32);
s32 rsin(s32);
static POLY_FT4* field_draw_timed_panel_quads(POLY_FT4* prim, u_long* ordering_table, s32 index);
static POLY_FT4* field_draw_timed_panel_image(POLY_FT4* prim, u_long* ordering_table);

/**
 * @brief Copy one of the portrait palettes into a portrait image.
 * @param dest Palette strip at the start of the portrait image.
 * @param index Palette index.
 */
void field_copy_portrait_palette(void* dest, s32 index)
{
    bcopy((u8*)&g_field_portrait_palettes[index], dest, sizeof(FieldPortraitPalette));
}

/**
 * @brief Load a timed panel image, fade the screen and play the opening sound.
 * @param index Panel index (image resource and mode byte).
 */
void field_start_timed_panel(s32 index)
{
    RECT rect;
    u8 mode;
    s32 sound_id;

    if (D_800F2298 != 0)
    {
        return;
    }

    /* x/y: image destination, w/h: palette destination; the loader returns the image size in x/y. */
    setRECT(&rect, FIELD_TIMED_PANEL_VRAM_X, 0, 0, FIELD_TIMED_PANEL_CLUT_Y);
    field_load_vram_resource(index + FIELD_TIMED_PANEL_RESOURCE_BASE, &rect, 1);

    g_field_timed_panel_index = index;
    g_field_timed_panel_width = rect.x;
    g_field_timed_panel_height = rect.y;
    field_set_fade_target_only(0x80, 0x80, 0x80, FIELD_TIMED_PANEL_FADE_FRAMES);
    g_field_timed_panel_timer = FIELD_TIMED_PANEL_FRAMES;
    g_field_timed_panel_brightness = 0;
    D_800F2298 = 1;

    mode = g_field_timed_panel_modes[g_field_timed_panel_index];
    if (mode & FIELD_TIMED_PANEL_EFFECT)
    {
        switch (mode & FIELD_TIMED_PANEL_EFFECT_MASK)
        {
        case 0:
            sound_id = FIELD_SOUND_PANEL_EFFECT0_START;
            break;
        case 1:
            sound_id = FIELD_SOUND_PANEL_EFFECT1_START;
            break;
        case 2:
            sound_id = FIELD_SOUND_PANEL_EFFECT2_START;
            break;
        default:
            return;
        }
    }
    else
    {
        sound_id = FIELD_SOUND_PANEL_IMAGE_START;
    }

    field_play_sound(sound_id, FIELD_SOUND_CENTER);
}

/**
 * @brief Count down the timed panel, fade it in and out and draw it.
 * @param render Render half receiving the panel primitives.
 * @note Confirm skips ahead to the fade-out while the panel is fully shown.
 * @note timer is a u16 (a halfword pseudo), so CSE does not match it against
 *       the zero-extended timer read in the fade-in arm, which reloads it.
 */
void field_update_timed_panel(FieldRenderHalf* render)
{
    POLY_FT4* prim;
    u16 timer;
    u8 event_selector;
    u8 render_selector;

    if (D_800F2298 != 0)
    {
        if (--g_field_timed_panel_timer == FIELD_TIMED_PANEL_FADE_FRAMES)
        {
            field_restore_fade_target_with_duration(FIELD_TIMED_PANEL_FADE_FRAMES);
        }
        timer = g_field_timed_panel_timer;
        if (timer == 0)
        {
            D_800F2298 = 0;
            return;
        }
        if (timer < FIELD_TIMED_PANEL_FADE_OUT_START)
        {
            if (timer == FIELD_TIMED_PANEL_FADE_OUT_START - 1)
            {
                event_selector = g_field_timed_panel_modes[g_field_timed_panel_index];
                if (event_selector & FIELD_TIMED_PANEL_EFFECT)
                {
                    switch (event_selector & FIELD_TIMED_PANEL_EFFECT_MASK)
                    {
                    case 0:
                        field_play_sound(FIELD_SOUND_PANEL_EFFECT0_END, FIELD_SOUND_CENTER);
                        break;
                    case 1:
                        field_play_sound(FIELD_SOUND_PANEL_EFFECT1_END, FIELD_SOUND_CENTER);
                        break;
                    case 2:
                        field_play_sound(FIELD_SOUND_PANEL_EFFECT2_END, FIELD_SOUND_CENTER);
                        break;
                    }
                }
                else
                {
                    field_play_sound(FIELD_SOUND_PANEL_IMAGE_END, FIELD_SOUND_CENTER);
                }
            }
            g_field_timed_panel_brightness = g_field_timed_panel_timer * 4;
        }
        else if (timer >= FIELD_TIMED_PANEL_FRAMES - FIELD_TIMED_PANEL_FADE_IN_FRAMES)
        {
            g_field_timed_panel_brightness = (FIELD_TIMED_PANEL_FRAMES - g_field_timed_panel_timer) * 4;
        }
        else if (timer >= FIELD_TIMED_PANEL_FADE_OUT_START + 1 && timer < FIELD_TIMED_PANEL_SKIP_END && (g_pad_input & FIELD_PAD_CONFIRM))
        {
            g_field_timed_panel_timer = FIELD_TIMED_PANEL_FADE_OUT_START;
        }
        g_field_screen_scroll.y = 0;
        g_field_screen_scroll.x = 0;
        render_selector = g_field_timed_panel_modes[g_field_timed_panel_index];
        prim = (POLY_FT4*)render->primitive_cursor;
        if (render_selector != 0)
        {
            prim = field_draw_timed_panel_quads(prim, render->ordering_table, render_selector & FIELD_TIMED_PANEL_EFFECT_MASK);
        }
        else
        {
            prim = field_draw_timed_panel_image(prim, render->ordering_table);
        }
        render->primitive_cursor = (u8*)prim;
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
 * @brief Draw the up to four quads of a timed panel effect group.
 * @param prim Next free primitive.
 * @param ordering_table Ordering table receiving the quads.
 * @param index Effect group (four FieldTransitionQuad descriptors).
 * @return First unused primitive.
 * @note The effect bits select a zoom (0, 4), a slide (1), a spin (2) or a flash (3),
 *       all driven by the panel brightness.
 * @note The do/while(0) blocks around the two width products in the spin case
 *       give width the loop weight that keeps it in a saved register (UNSOLVED lever).
 * @note bottom_1 is a u16 screen coordinate: its sum is done on halfword reads,
 *       which CSE does not share with the zero-extended height read of the
 *       following product, so height is read twice as in the original.
 */
static POLY_FT4* field_draw_timed_panel_quads(POLY_FT4 *prim, u_long *ordering_table, s32 index)
{
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
    u16 bottom_1;
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

    quad = g_field_timed_panel_quads[index];
    flags_base = (u8 *)&quad->flags;
    for (quad_index = 0; quad_index < 4; quad_index++, quad++)
    {
        quad_flags = flags_base + quad_index * 12;
        if (quad->x != FIELD_TRANSITION_QUAD_UNUSED)
        {
            raw_width = QUAD_OF_FLAGS(quad_flags)->width;
            if (QUAD_OF_FLAGS(quad_flags)->flags & FIELD_TRANSITION_QUAD_HALF_WIDTH)
            {
                raw_width = (u16)(raw_width >> 1);
            }
            setPolyFT4(prim);
            width = raw_width & 0xFFFF;
            prim->r0 = prim->g0 = prim->b0 = g_field_timed_panel_brightness;
            if ((g_field_timed_panel_brightness != FIELD_TIMED_PANEL_FULL_BRIGHTNESS) || (QUAD_OF_FLAGS(quad_flags)->flags & FIELD_TRANSITION_QUAD_SEMI_TRANSPARENT))
            {
                setSemiTrans(prim, 1);
            }
            expansion = FIELD_TIMED_PANEL_FULL_BRIGHTNESS - g_field_timed_panel_brightness;
            switch (FIELD_TRANSITION_QUAD_EFFECT(QUAD_OF_FLAGS(quad_flags)->flags))
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
                bottom_1 = QUAD_OF_FLAGS(quad_flags)->y + QUAD_OF_FLAGS(quad_flags)->height;
                scaled = QUAD_OF_FLAGS(quad_flags)->height * expansion;
                if (scaled < 0)
                {
                    scaled += 0x7F;
                }
                prim->y2 = prim->y3 = bottom_1 + (scaled >> 7) - 1;
                break;
            case 1:
                if (g_field_timed_panel_timer < 0x25)
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
                if (g_field_timed_panel_brightness < 0x40)
                {
                    brightness = g_field_timed_panel_brightness * 4;
                }
                else
                {
                    brightness = ~((g_field_timed_panel_brightness - 0x40) * 2);
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
                bottom_1 = QUAD_OF_FLAGS(quad_flags)->y + QUAD_OF_FLAGS(quad_flags)->height;
                scaled = QUAD_OF_FLAGS(quad_flags)->height * expansion;
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
            prim->clut = getClut(QUAD_OF_FLAGS(quad_flags)->flags & palette_mask, FIELD_TIMED_PANEL_CLUT_Y);
            prim->tpage = getTPage(QUAD_OF_FLAGS(quad_flags)->flags & 3, (QUAD_OF_FLAGS(quad_flags)->flags >> 9) & 3, FIELD_TIMED_PANEL_VRAM_X, 0);
            addPrim(ordering_table, prim);
            if (g_field_timed_panel_brightness < FIELD_TIMED_PANEL_FADE_LIMIT)
            {
                term = FIELD_TRANSITION_QUAD_EFFECT(QUAD_OF_FLAGS(quad_flags)->flags);
                switch (term)
                {
                case 0:
                    break;
                case 1:
                    field_add_fade_prim(prim, 0);
                    break;
                case 2:
                    field_add_fade_prim(prim, 0);
                    break;
                }
            }
            prim++;
        }
    }
    return prim;
}

/**
 * @brief Draw the plain timed panel image, zoomed in from the screen center by the fade.
 * @param prim Primitive to fill.
 * @param ordering_table Ordering table receiving the quad.
 * @return Next free primitive.
 * @note The value0..value5 locals are reused on purpose: their reassignments
 *       order the scheduler the way the target is laid out.
 */
static POLY_FT4* field_draw_timed_panel_image(POLY_FT4 *prim, u_long *ordering_table)
{
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
    value0 = (u8)g_field_timed_panel_brightness;
    prim->b0 = value0;
    prim->g0 = value0;
    prim->r0 = value0;
    value1 = getcode(prim);
    value1 |= 2;
    setcode(prim, value1);
    value1 = FIELD_TIMED_PANEL_FULL_BRIGHTNESS;
    value0 = (u16)g_field_timed_panel_brightness;
    value4 = g_field_timed_panel_width;
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
    /* The loop notes keep sched1 from moving this subtract into the load delay slot (UNSOLVED lever). */
    do
    {
        value0 -= value1;
    } while (0);
    value3 = g_field_timed_panel_height;
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
    value0 = getClut(0, FIELD_TIMED_PANEL_CLUT_Y);
    prim->u2 = 0;
    prim->u0 = 0;
    value1 = g_field_timed_panel_width;
    prim->v1 = 0;
    prim->v0 = 0;
    prim->clut = value0;
    value1 <<= 2;
    value1--;
    prim->u3 = value1;
    prim->u1 = value1;
    value0 = (u8)g_field_timed_panel_height;
    value1 = getTPage(0, 1, FIELD_TIMED_PANEL_VRAM_X, 0);
    prim->tpage = value1;
    value0--;
    prim->v3 = value0;
    prim->v2 = value0;
    setaddr(prim, *ordering_table & address_mask);
    setaddr(ordering_table, prim);
    if ((u16)g_field_timed_panel_brightness < FIELD_TIMED_PANEL_FADE_LIMIT)
    {
        field_add_fade_prim(prim, 0);
    }
    return prim + 1;
}

/* ---- Actor text queue (0x800A6204 .. 0x800A68B4) ---- */

/** @brief Number of actor text slots (the first two follow the players). */
#define FIELD_ACTOR_TEXT_COUNT 3
/** @brief Frames an actor text stays up; it fades out over the last 32. */
#define FIELD_ACTOR_TEXT_FRAMES 63
#define FIELD_ACTOR_TEXT_FADE_FRAMES 32
/** @brief field_draw_tinted_text() color that selects the neutral tint. */
#define FIELD_TEXT_TINT_NEUTRAL 0x100
/** @brief Primitive-buffer depth of the text layer in the render half's ordering table. */
#define FIELD_TEXT_OT_DEPTH 16

/** @brief Packed screen position and countdown of an actor text. */
typedef union
{
    u32 word;
    struct
    {
        u32 x : 9;         /**< Screen X of the text. */
        u32 y : 8;         /**< Screen Y of the text. */
        u32 countdown : 6; /**< Frames left to show the text; 0 when the slot is free. */
        u32 unused : 9;
    } bits;
} FieldActorTextState;

/** @brief One actor text slot: a string shown over an actor for a few frames. */
typedef struct FieldActorText
{
    u8* text;
    FieldActorTextState state;
} FieldActorText;

extern FieldActorText g_field_actor_texts[FIELD_ACTOR_TEXT_COUNT];
/** @brief Technique name bank: u16 offsets from the bank start, then the strings. */
extern u8 g_field_technique_names[];

extern s32 g_field_text_session_active;
extern s32 g_field_actor_text_count;

static void field_draw_actor_text(FieldRenderHalf* render, FieldActorText* text);
static void* field_draw_tinted_text(SPRT* sprite_cursor, s32* ordering_table, u8* text, s32 text_color, s32 x, s32 y, s32 alignment, s32 color);
SPRT* func_800AD658(s32* ordering_table, SPRT* sprite_cursor, s32 count);

/**
 * @brief Clear the countdown of every actor text.
 */
void field_clear_actor_texts(void)
{
    g_field_actor_texts[2].state.bits.countdown = 0;
    g_field_actor_texts[1].state.bits.countdown = 0;
    g_field_actor_texts[0].state.bits.countdown = 0;
}

/**
 * @brief Show a text over a player's actor unless one is already showing.
 * @param index Player (actor and text slot) index; other actors are ignored.
 * @param text_id Technique name index, or negative: bits 16-23 party character and
 *        bits 0-7 record index of a name stored in the saved character.
 * @note The actor terms are (s16) screen coordinates; the casts also keep fold
 *       from moving the 160/112 screen-centre constants onto the view offsets.
 */
void field_start_actor_text(s32 index, s32 text_id)
{
    s16 point[2];
    s32 width;

    if (index < FIELD_PLAYER_COUNT)
    {
        if (g_field_actor_texts[index].state.bits.countdown == 0)
        {
            if (text_id < 0)
            {
                /* Bits 16-23 pick a party character, bits 0-7 one of its 64-byte records. */
                g_field_actor_texts[index].text = (u8 *)&g_pad_ctx->characters[((u32)text_id >> 16) & 0xFF].unk150[text_id & 0xFF];
            }
            else
            {
                g_field_actor_texts[index].text = (u8 *)(((u16 *)g_field_technique_names)[text_id] + (s32)g_field_technique_names);
            }
            g_field_actor_texts[index].state.bits.countdown = FIELD_ACTOR_TEXT_FRAMES;
            point[0] = g_field_view_offset_x / 256 + (s16)(g_field_actors[index].x / 256 + 160);
            point[1] = g_field_view_offset_y / 256 + (s16)(g_field_actors[index].y / 256 + 112) - g_field_actors[index].z / 512 - g_field_view_offset_z / 512;
            width = func_800AE864(g_field_actor_texts[index].text) * 6;
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
            g_field_actor_texts[index].state.bits.x = point[0];
            g_field_actor_texts[index].state.bits.y = point[1] + 4;
        }
    }
}

/**
 * @brief Report whether any actor text is still showing.
 * @return 1 if a slot's countdown is nonzero, otherwise 0.
 */
s32 field_actor_text_pending(void)
{
    s32 i;

    for (i = 0; i < FIELD_ACTOR_TEXT_COUNT; i++)
    {
        if (g_field_actor_texts[i].state.bits.countdown != 0)
        {
            return 1;
        }
    }
    return 0;
}

/**
 * @brief Draw and count down the actor texts, or clear them while a text session runs.
 * @param render Render half receiving the text primitives.
 * @note The previous active count controls window cleanup on the next frame.
 */
void field_update_actor_texts(FieldRenderHalf* render)
{
    s32 i;
    s32 active_count;

    active_count = 0;
    if (g_field_text_session_active != 0)
    {
        i = 0;
        for (; i < FIELD_ACTOR_TEXT_COUNT; i++)
        {
            g_field_actor_texts[i].state.bits.countdown = 0;
        }
        g_field_actor_text_count = 0;
        return;
    }

    for (i = 0; i < FIELD_ACTOR_TEXT_COUNT; i++)
    {
        if (g_field_actor_texts[i].state.bits.countdown)
        {
            active_count += 1;
        }
    }

    if (active_count != 0)
    {
        field_text_reset_scratch();
        for (i = 0; i < FIELD_ACTOR_TEXT_COUNT; i++)
        {
            if (g_field_actor_texts[i].state.bits.countdown)
            {
                field_draw_actor_text(render, &g_field_actor_texts[i]);
                g_field_actor_texts[i].state.bits.countdown--;
            }
        }
        field_text_upload_immediate_cache();
    }
    else if (g_field_actor_text_count != 0)
    {
        field_text_reset_windows();
    }
    g_field_actor_text_count = active_count;
}

/**
 * @brief Draw one actor text, fading it out over its last frames.
 * @param render Render half holding the primitive cursor and the text layer.
 * @param text Actor text slot to draw.
 */
static void field_draw_actor_text(FieldRenderHalf* render, FieldActorText* text)
{
    s32 tint;
    s32 countdown;
    FieldActorTextState state;
    u8* cursor;
    u_long* ot;

    tint = FIELD_TEXT_TINT_NEUTRAL;
    cursor = render->primitive_cursor;
    state = text->state;
    /* Shift and mask by hand: the bitfield read reuses one register for both steps. */
    countdown = (state.word >> 17) & 0x3F;
    ot = &render->ordering_table[FIELD_TEXT_OT_DEPTH];
    if (countdown < FIELD_ACTOR_TEXT_FADE_FRAMES)
    {
        tint = countdown * 4;
    }
    render->primitive_cursor = field_draw_tinted_text((SPRT*)cursor, (s32*)ot, text->text, 4, state.bits.x, text->state.bits.y, 2, tint);
}

/**
 * @brief Draw a text line like func_800A88A0, with a gray tint for fading.
 * @param sprite_cursor Primitive-buffer cursor used for generated sprites.
 * @param ordering_table Ordering-table entry that receives the generated primitives.
 * @param text Null-terminated text to render.
 * @param text_color Text style passed to the glyph builder.
 * @param x Horizontal origin used to position the rendered text.
 * @param y Vertical origin used to position the rendered text.
 * @param alignment Horizontal alignment mode for the generated glyphs.
 * @param color Tint level, or FIELD_TEXT_TINT_NEUTRAL for the neutral tint.
 * @return Primitive-buffer cursor immediately after the generated draw commands.
 */
static void* field_draw_tinted_text(SPRT* sprite_cursor, s32* ordering_table, u8* text, s32 text_color, s32 x, s32 y, s32 alignment, s32 color)
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
#define FIELD_PROFICIENCY_MAX 100U
#define FIELD_PROFICIENCY_BONUS 4
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

extern FieldTechniqueUnlockRule g_field_technique_unlock_rules[];
/* Scene image setup also uses this flag; its broader purpose is unresolved. */
extern s32 D_80115890;

/**
 * @brief Advance active player proficiency and queue newly learned abilities and techniques.
 * @note Proficiency saturates at 100, advancing by four when D_80115890 is set, or one otherwise.
 * @note A technique rule can unlock silently, without adding a dialog entry.
 */
void field_advance_ability_progression(void)
{
    s32 ability_index;
    FieldAbilityUnlockRule* ability_base;
    s32 technique_index;
    FieldTechniqueUnlockRule* technique_base;
    FieldTechniqueUnlockRule* active_rule;
    FieldGameState* context;
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
        if (g_field_player_records[party_index].flags & FIELD_PLAYER_ACTIVE)
        {
            if ((u32)(g_pad_ctx->characters[party_index].info.actions.type & FIELD_CHARACTER_TYPE_MASK) < FIELD_CHARACTER_GUEST)
            {
                weapon_category = (g_pad_ctx->characters[party_index].equipment[FIELD_WEAPON_SLOT].info.word >> FIELD_WEAPON_CATEGORY_SHIFT) & FIELD_WEAPON_CATEGORY_MASK;
                if (weapon_category < FIELD_WEAPON_CATEGORY_COUNT)
                {
                    weapon_proficiency = g_pad_ctx->weapon_proficiency[weapon_category];
                    if (weapon_proficiency < FIELD_PROFICIENCY_MAX)
                    {
                        if (D_80115890 != 0)
                        {
                            g_pad_ctx->weapon_proficiency[weapon_category] = weapon_proficiency + FIELD_PROFICIENCY_BONUS;

                            if (g_pad_ctx->weapon_proficiency[((g_pad_ctx->characters[party_index].equipment[FIELD_WEAPON_SLOT].info.word >> FIELD_WEAPON_CATEGORY_SHIFT) &
                                                               FIELD_WEAPON_CATEGORY_MASK)] > FIELD_PROFICIENCY_MAX)
                            {
                                g_pad_ctx->weapon_proficiency[((g_pad_ctx->characters[party_index].equipment[FIELD_WEAPON_SLOT].info.word >> FIELD_WEAPON_CATEGORY_SHIFT) &
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
                    ability_proficiency = g_pad_ctx->ability_proficiency[g_pad_ctx->characters[party_index].info.actions.commands[ability]];
                    if (ability_proficiency < FIELD_PROFICIENCY_MAX)
                    {
                        if (D_80115890 != 0)
                        {
                            g_pad_ctx->ability_proficiency[g_pad_ctx->characters[party_index].info.actions.commands[ability]] =
                                ability_proficiency + FIELD_PROFICIENCY_BONUS;

                            if (g_pad_ctx->ability_proficiency[g_pad_ctx->characters[party_index].info.actions.commands[ability]] > FIELD_PROFICIENCY_MAX)
                            {
                                g_pad_ctx->ability_proficiency[g_pad_ctx->characters[party_index].info.actions.commands[ability]] = FIELD_PROFICIENCY_MAX;
                            }
                        }
                        else
                        {
                            g_pad_ctx->ability_proficiency[g_pad_ctx->characters[party_index].info.actions.commands[ability]] = ability_proficiency + 1;
                        }
                    }
                }
            }
        }
        party_index++;
    } while (party_index < FIELD_PARTY_SIZE);
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
        if (!(context->ability_bits[ability_word] & (1 << (ability % FIELD_UNLOCK_BITS_PER_WORD))))
        {
            ability = ability_rule->prerequisites[0].ability;
            if (ability == FIELD_ABILITY_PREREQUISITE_NONE ||
                ((context->ability_bits[ability / FIELD_UNLOCK_BITS_PER_WORD] & (1 << (ability % FIELD_UNLOCK_BITS_PER_WORD))) &&
                 (context->ability_proficiency[ability] >= ability_rule->prerequisites[0].proficiency)))
            {
                ability = ability_rule->prerequisites[1].ability;
                if (ability == FIELD_ABILITY_PREREQUISITE_NONE ||
                    ((context->ability_bits[ability / FIELD_UNLOCK_BITS_PER_WORD] & (1 << (ability % FIELD_UNLOCK_BITS_PER_WORD))) &&
                     (context->ability_proficiency[ability] >= ability_rule->prerequisites[1].proficiency)))
                {
                    ability = ability_rule->result;
                    unlocked_ability_word = ability / FIELD_UNLOCK_BITS_PER_WORD;
                    context->ability_bits[unlocked_ability_word] =
                        (s32)(context->ability_bits[unlocked_ability_word] | (1 << (ability % FIELD_UNLOCK_BITS_PER_WORD)));
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
        if (!(context->technique_bits[technique_group] & (1 << (ability % FIELD_UNLOCK_BITS_PER_WORD))))
        {
            ability = technique_rule->prerequisites[0].ability;
            if (ability == FIELD_ABILITY_PREREQUISITE_NONE ||
                ((context->ability_bits[ability / FIELD_UNLOCK_BITS_PER_WORD] & (1 << (ability % FIELD_UNLOCK_BITS_PER_WORD))) &&
                 (context->ability_proficiency[ability] >= technique_rule->prerequisites[0].proficiency)))
            {
                ability = technique_rule->prerequisites[1].ability;
                if (ability == FIELD_ABILITY_PREREQUISITE_NONE ||
                    ((context->ability_bits[ability / FIELD_UNLOCK_BITS_PER_WORD] & (1 << (ability % FIELD_UNLOCK_BITS_PER_WORD))) &&
                     (context->ability_proficiency[ability] >= technique_rule->prerequisites[1].proficiency)))
                {
                    ability = technique_rule->prerequisites[2].ability;
                    if (ability == FIELD_ABILITY_PREREQUISITE_NONE ||
                        ((context->ability_bits[ability / FIELD_UNLOCK_BITS_PER_WORD] & (1 << (ability % FIELD_UNLOCK_BITS_PER_WORD))) &&
                         (context->ability_proficiency[ability] >= technique_rule->prerequisites[2].proficiency)))
                    {
                        ability = technique_rule->prerequisites[3].ability;
                        if (ability == FIELD_ABILITY_PREREQUISITE_NONE ||
                            ((context->ability_bits[ability / FIELD_UNLOCK_BITS_PER_WORD] & (1 << (ability % FIELD_UNLOCK_BITS_PER_WORD))) &&
                             (context->ability_proficiency[ability] >= technique_rule->prerequisites[3].proficiency)))
                        {
                            technique_party_index = 0;
                            active_rule = technique_rule;
                            ability = technique_rule->result & FIELD_TECHNIQUE_INDEX_MASK;
                            do
                            {
                                if ((g_field_player_records[technique_party_index].flags & FIELD_PLAYER_ACTIVE) &&
                                    ((u32)(context->characters[technique_party_index].info.actions.type & FIELD_CHARACTER_TYPE_MASK) <
                                     FIELD_CHARACTER_GUEST))
                                {
                                    weapon_requirement = active_rule->weapon;
                                    equipped_weapon =
                                        (context->characters[technique_party_index].equipment[FIELD_WEAPON_SLOT].info.word >> FIELD_WEAPON_CATEGORY_SHIFT) & FIELD_WEAPON_CATEGORY_MASK;
                                    if (equipped_weapon == (weapon_requirement & FIELD_TECHNIQUE_WEAPON_MASK))
                                    {
                                        if (context->weapon_proficiency[equipped_weapon] >= active_rule->weapon_proficiency)
                                        {
                                            if (!(context->technique_bits[technique_group] & (technique_mask = 1 << (ability % FIELD_UNLOCK_BITS_PER_WORD))))
                                            {
                                                technique_group = weapon_requirement >> FIELD_TECHNIQUE_GROUP_SHIFT;
                                                context->technique_bits[technique_group] = (s32)(context->technique_bits[technique_group] | technique_mask);
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
                            } while (technique_party_index < FIELD_PARTY_SIZE);
                        }
                    }
                }
            }
        }
        technique_index++;
    } while ((s32)&technique_base[technique_index] < (s32)&technique_base[FIELD_TECHNIQUE_UNLOCK_RULE_COUNT]);
}

/* ---- Dialog screens (0x800A6EEC .. 0x800A88A0) ---- */

/** @brief g_field_dialog_screen_mode values of the battle result screens. */
#define FIELD_RESULTS_PARTY_SUMMARY 1
#define FIELD_RESULTS_ITEM_LIST 2
#define FIELD_RESULTS_UNLOCK_LIST 3

/** @brief Result window layout: left edge, row height, vertical center and largest visible height. */
#define FIELD_RESULTS_WINDOW_X 0x20
#define FIELD_RESULTS_ROW_HEIGHT 16
#define FIELD_RESULTS_CENTER_Y 112
#define FIELD_RESULTS_MAX_HEIGHT 160
/** @brief Unlock list columns: group header and centered name. */
#define FIELD_RESULTS_HEADER_X 0x20
#define FIELD_RESULTS_NAME_X 0x80
/** @brief Palette and texture page of the animated result icons. */
#define FIELD_RESULTS_ICON_CLUT getClut(112, 490)
#define FIELD_RESULTS_ICON_TPAGE getTPage(0, 1, 384, 0)

/** @brief Resource with the ability and technique name tables. */
#define FIELD_RES_UNLOCK_NAMES 0x5DF
#define FIELD_SOUND_UNLOCK_LIST 0xA1
#define FIELD_SOUND_WINDOW_OPEN 0xB9
#define FIELD_SOUND_CURSOR 0x7D
#define FIELD_SOUND_SELECT 0x7E

/** @brief Frames the party summary waits before it accepts confirm. */
#define FIELD_RESULTS_WAIT_FRAMES 4

/** @brief g_field_progression_unlocks entry: technique (low bits: group * 24 + index) rather than ability. */
#define FIELD_UNLOCK_TECHNIQUE FIELD_TECHNIQUE_UNLOCK_FLAG

/** @brief Drawing position and three sorted actor indices sharing the local work area. */
typedef struct
{
    Vec2s position;
    s32 unused;
    s32 order[3];
} FieldRankWork;

/**
 * @brief Address of a string in a text bank that starts with little-endian u16 offsets.
 * @note The integer sum keeps the low byte as the first addu operand.
 */
#define FIELD_TEXT_AT(bank, low, high) ((u8 *)((low) + (((high) << 8) + (s32)(bank))))

/** @brief Experience a party member gained since g_field_experience_snapshot was sampled. */
#define FIELD_EXPERIENCE_GAIN(index) ((s32)((g_pad_ctx->characters[index].progress.word >> 8) - g_field_experience_snapshot[index]))

/** @brief One entry of the dialog text bank's offset table (little-endian, read bytewise). */
typedef struct PackedOffset
{
    u8 low;
    u8 high;
} PackedOffset;

/** @brief Header of the unlock name resource: offsets of its two name banks. */
typedef struct TextResource
{
    s32 unused;
    s32 ability_names;
    s32 technique_names;
} TextResource;

/** @brief Eight-frame animation table; the icon drawers copy it into a local array. */
typedef struct
{
    s32 words[8];
} FieldQuadAnimationTable;

extern u8 g_field_retry_snapshot[];
/*
 * The dialog text bank D_800EC3C4 starts with PackedOffset entries (offsets
 * from the bank start); the entries used here are referenced by their own
 * address symbols, which the code needs for its address arithmetic.
 */
extern u8 D_800EC3D6[];
extern u8 D_800EC3C6[], D_800EC3DA[];
extern u8 g_field_dialog_item_quantities[];
extern u8 D_800EC3C4[];
extern u8* g_field_dialog_item_texts[];
extern s32 g_field_experience_snapshot[];
extern PackedOffset D_800EC3D8;
extern PackedOffset D_800EC3CC;
extern PackedOffset D_800EC3CE;
extern FieldQuadAnimationTable g_field_small_icon_frames, g_field_marker_x_offsets, g_field_marker_u_offsets, g_field_marker_widths;
extern s32 g_field_results_wait_frames;
extern s32 g_field_text_session_active;
extern s32 g_field_dialog_item_count;
extern s32 g_field_dialog_screen_mode;
extern s32 g_field_money_snapshot;
extern s32 g_pad_input;
extern s32 g_pending_game_state;
extern s32 g_field_return_to_title_prompt_state;
extern s32 g_field_return_to_title_prompt_delay;
extern s32 g_frame_counter;
extern s32 g_field_pending_spawn_id, g_field_pending_music_id, g_field_pending_secondary_music_id, g_field_pending_scene_id, g_field_pending_object_id, g_field_pending_sound_bank_id;

extern void field_reset_input_repeat(void);
extern void akao_cmd_f1(void);
/* Defined as (void) in field_resource_load.c; the original call still passes the cursor in $a0. */
extern s32 func_800B0888(void *arg0);
/* Defined in field_modal_runtime.c and field_actor_hud_effects.c. */
void* func_800A88A0(SPRT* cursor, s32* ot, u8* text, s32 color, s32 x, s32 y, s32 flags);
void* func_800A8A78(void* ot, void* cursor, s32 value, s32 color, Vec2s* position, s32 flags);
void* field_emit_actor_portrait(SPRT* sprt, u_long* ot, s32 index, Vec2s* position);

void field_close_battle_results(void);
static void field_open_unlock_list(void);
static void field_open_item_list(void);
static void field_open_party_summary(void);
static void* field_draw_party_totals(void* ot, void* cursor, s32 x_offset, s32 y_offset);
static void* field_draw_experience_ranking(void* ot, void* cursor, s32 x_offset, s32 y_offset);
static void* field_draw_item_list(void* ot, void* cursor, s32 x_offset, s32 y_offset);
static void* field_draw_unlock_list(void* ot, void* cursor, s32 scroll_x, s32 scroll_y, s32 viewport_height);
static POLY_FT4* field_draw_animated_icon(u32* ordering_table, POLY_FT4* prim, s32 x, s32 y, s32 wide);
static void* field_draw_animated_small_icon(s32* ordering_table, POLY_FT4* prim, s32 x, s32 y);
static void* field_draw_animated_marker(s32* ordering_table, POLY_FT4* prim, s32 x, s32 y);

/**
 * @brief Save the game state before a battle so that it can be retried.
 */
void field_save_retry_snapshot(void)
{
    bcopy((u8*)g_pad_ctx, g_field_retry_snapshot, SAVED_GAME_DATA_SIZE);
}

/**
 * @brief Run the return-to-title prompt: take its choice, then quit or retry the battle.
 * @note Retrying restores g_field_retry_snapshot, refills the party's HP and
 *       technique gauges and restarts the scene.
 */
void field_handle_return_to_title_prompt(void)
{
    FieldObjectState* actor;
    s32 actor_index;
    u32 hp_mask, status_mask;
    s16 full_gauge;
    FieldPlayerRecord* player;

    ControllerState* controller = CONTROLLER_STATE;

    if (D_80122828[0].attr.bits.state == 0)
    {
        /* The prompt window has closed: act on the choice. */
        if (--g_field_return_to_title_prompt_state == 0)
        {
            if (g_field_return_to_title_choice != 0)
            {
                g_field_return_to_title_prompt_state = 1;
                g_pending_game_state = GAME_STATE_TITLE;
                return;
            }
            bcopy(g_field_retry_snapshot, (u8*)g_pad_ctx, SAVED_GAME_DATA_SIZE);
            if (g_pad_ctx->retry_count != -1)
            {
                g_pad_ctx->retry_count++;
            }
            controller->ports[1].small_motor_command = 0;
            controller->ports[0].small_motor_command = 0;
            controller->ports[1].actuator_control.fields.large_motor_command = 0;
            controller->ports[0].actuator_control.fields.large_motor_command = 0;
            field_rebuild_party_actions(0);
            field_set_scene_parameters(g_field_pending_scene_id, g_field_pending_object_id, g_field_pending_spawn_id, g_field_pending_music_id, g_field_pending_sound_bank_id,
                                       g_field_pending_secondary_music_id);
            actor_index = 0;
            /* The mask locals set the order of the hoisted loop constants. */
            hp_mask = 0xFFFFFF;
            status_mask = 0xFF000000;
            full_gauge = 0xFF;
            for (; actor_index < FIELD_PARTY_COUNT; actor_index++)
            {
                actor = &g_field_object_states[actor_index];
                player = &g_field_player_records[actor_index];
                actor->unk8.word = (actor->unk8.word & status_mask) | (actor->unk0 & hp_mask);
                actor->unk4.word = actor->unk0 & hp_mask;
                if (player->flags & FIELD_PLAYER_ACTIVE)
                {
                    actor->technique_gauge = full_gauge;
                }
            }
        }
    }
    else if (func_800ADEEC() == 0)
    {
        if (g_pad_input & (FIELD_PAD_CONFIRM | PAD_BTN_START))
        {
            field_play_sound(FIELD_SOUND_SELECT, FIELD_SOUND_CENTER);
            func_800ADF34();
            field_begin_return_to_title_prompt_close();
            return;
        }
        if (g_pad_input & (PAD_BTN_UP | PAD_BTN_RIGHT | PAD_BTN_DOWN | PAD_BTN_LEFT | PAD_BTN_SELECT))
        {
            field_play_sound(FIELD_SOUND_CURSOR, FIELD_SOUND_CENTER);
            g_field_return_to_title_choice ^= 1;
        }
    }
}

/**
 * @brief Advance the battle result screens on confirm: unlocks, then items, then the party summary, then close.
 */
void field_update_battle_results_input(void)
{
    if ((func_800ADEEC() == 0) && (g_pad_input & FIELD_PAD_CONFIRM))
    {
        switch (g_field_dialog_screen_mode)
        {
        case FIELD_RESULTS_PARTY_SUMMARY:
            if (g_field_results_wait_frames == 0)
            {
                field_close_battle_results();
                return;
            }
            break;
        case FIELD_RESULTS_UNLOCK_LIST:
            if (g_field_dialog_item_count != 0)
            {
                field_open_item_list();
                return;
            }
            /* fallthrough */
        case FIELD_RESULTS_ITEM_LIST:
            field_open_party_summary();
            break;
        }
    }
}

/**
 * @brief Open the list of abilities and techniques learned in the battle.
 */
static void field_open_unlock_list(void)
{
    s16 height;
    s32 padding;
    s32 index;
    s32 saw_technique;
    s32 saw_ability;
    FieldMenuElement *record;
    u32 size;

    cdrom_queue_read(FIELD_RES_UNLOCK_NAMES, g_field_cd_buffer);
    cdrom_wait_queue_empty();
    field_play_sound(FIELD_SOUND_UNLOCK_LIST, FIELD_SOUND_CENTER);
    g_field_dialog_screen_mode = FIELD_RESULTS_UNLOCK_LIST;
    func_800ADF34();
    record = func_800ADF84();
    record->attr.bits.step = 1;
    record->attr.bits.x = FIELD_RESULTS_WINDOW_X;
    /* Each group (abilities, techniques) gets a header row. */
    saw_ability = 0;
    saw_technique = 0;
    padding = 0;
    for (index = 0; index < g_field_progression_unlock_count; index++)
    {
        if (g_field_progression_unlocks[index] & FIELD_UNLOCK_TECHNIQUE)
        {
            if (saw_technique == 0)
            {
                saw_technique = 1;
                padding += FIELD_RESULTS_ROW_HEIGHT;
            }
        }
        else if (saw_ability == 0)
        {
            saw_ability = 1;
            padding += FIELD_RESULTS_ROW_HEIGHT;
        }
    }

    /* The callback has its own prototype; cast to the element draw type. */
    record->draw = (FieldMenuDrawFn)field_draw_unlock_list;
    record->attr.word &= ~FIELD_MENU_ATTR_WIDTH_LOW;
    size = record->size.word;
    size |= FIELD_MENU_SIZE_WIDTH_HIGH;
    record->size.word = size;
    height = g_field_progression_unlock_count * FIELD_RESULTS_ROW_HEIGHT + padding;
    record->size.word = size | FIELD_MENU_SIZE_BLINK;
    record->size.fields.content_height = height;
    if (height <= FIELD_RESULTS_MAX_HEIGHT)
    {
        record->size.bits.height = height;
        record->attr.bytes.y = FIELD_RESULTS_CENTER_Y - record->size.bits.height / 2;
        return;
    }

    /* Too tall for the screen: scroll it with the pad. */
    record->scroll = 0;
    record->scroll_target = 0;
    record->scroll_ticks = 0;
    record->size.bits.scroll_mode = 1;
    record->size.bits.height = FIELD_RESULTS_MAX_HEIGHT;
    record->attr.bytes.y = FIELD_RESULTS_CENTER_Y - record->size.bits.height / 2;
}

/**
 * @brief Close the result screens: get knocked-down players up and clear the party's battle flags.
 */
void field_close_battle_results(void)
{
    s32 i;

    g_field_text_session_active = 0;
    for (i = 0; i < FIELD_PARTY_COUNT; i++)
    {
        if ((g_field_player_records[i].flags & FIELD_PLAYER_ACTIVE) && g_field_actors[i].command == FIELD_ACTOR_COMMAND_KNOCKED_DOWN)
        {
            g_field_actors[i].command = 0;
        }
    }

    field_close_dialog_screen();

    for (i = 0; i < FIELD_PARTY_COUNT; i++)
    {
        g_field_object_states[i].flags = 0;
        g_field_object_states[i].contact.word &= ~FIELD_CONTACT_NO_HIT_TEST;
    }
    g_field_dialog_item_count = 0;
}

/**
 * @brief Open the battle result screens with the first one that has content.
 */
void field_open_battle_results(void)
{
    func_800ADEB0();
    g_field_results_wait_frames = FIELD_RESULTS_WAIT_FRAMES;
    field_reset_input_repeat();
    if (g_field_progression_unlock_count != 0)
    {
        field_open_unlock_list();
    }
    else if (g_field_dialog_item_count != 0)
    {
        field_open_item_list();
    }
    else
    {
        field_open_party_summary();
    }
    func_800B0A08(0);
}

/**
 * @brief End a duel: show its result panel instead of the result screens.
 */
void field_open_duel_results(void)
{
    field_reset_input_repeat();
    func_800B0A08(0);
    field_begin_duel_result();
}

/**
 * @brief Open the return-to-title prompt windows (coordinate panel and yes/no choice).
 * @note The mixed word and bitfield updates preserve the original store widths.
 */
void field_setup_return_to_title_prompt(void)
{
    FieldMenuElement *rec;
    s32 i;

    func_800ADEB0();
    field_play_sound(FIELD_SOUND_WINDOW_OPEN, FIELD_SOUND_CENTER);

    rec = func_800ADF84();
    /* The callback has its own prototype; cast to the element draw type. */
    rec->draw = (FieldMenuDrawFn)field_draw_coordinate_panel;
    rec->attr.bits.step = 1;
    rec->attr.bits.x = 0x20;
    rec->size.bits.width_high = 1;
    rec->size.bits.height = 48;
    rec->attr.bits.y = 0x30;
    rec->attr.word &= ~FIELD_MENU_ATTR_WIDTH_LOW;

    rec = func_800ADF84();
    rec->draw = (FieldMenuDrawFn)field_draw_return_to_title_choices;
    rec->attr.bits.step = 1;
    rec->attr.bits.x = 0x50;
    rec->size.bits.width_high = 0;
    rec->size.bits.height = 0x22;
    rec->attr.bits.y = 0x80;

    rec->attr.word = (rec->attr.word & ~FIELD_MENU_ATTR_WIDTH_LOW) | (160 << FIELD_MENU_ATTR_WIDTH_LOW_SHIFT);

    field_select_coordinate_labels();
    akao_cmd_f1();

    g_field_return_to_title_prompt_delay = 0x3C;
    g_field_return_to_title_prompt_state = 3;
    g_field_return_to_title_choice = 0;
    field_upload_player_icons();

    for (i = 2; i >= 0; i--)
    {
        g_field_player_records[i].revive_delay = 0;
    }
}

/**
 * @brief Open the list of items found in the battle.
 */
static void field_open_item_list(void)
{
    FieldMenuElement *rec;
    u32 state;

    g_field_dialog_screen_mode = FIELD_RESULTS_ITEM_LIST;
    field_play_sound(FIELD_SOUND_WINDOW_OPEN, FIELD_SOUND_CENTER);
    func_800ADF34();

    rec = func_800ADF84();
    rec->attr.bits.step = 1;
    rec->attr.bits.x = 0x40;

    state = rec->size.word & ~FIELD_MENU_SIZE_HEIGHT_MASK;
    state |= (((g_field_dialog_item_count << 4) + FIELD_RESULTS_ROW_HEIGHT) & 0xFF) << FIELD_MENU_SIZE_HEIGHT_SHIFT;
    rec->attr.bits.y = FIELD_RESULTS_CENTER_Y - FIELD_MENU_SIZE_HEIGHT(state) / 2;

    /* The callback has its own prototype; cast to the element draw type. */
    rec->draw = (FieldMenuDrawFn)field_draw_item_list;
    rec->size.word = state;
    rec->attr.word = (rec->attr.word & ~FIELD_MENU_ATTR_WIDTH_LOW) | (192 << FIELD_MENU_ATTR_WIDTH_LOW_SHIFT);
    rec->size.bits.width_high = 0;
}

/**
 * @brief Open the party summary: the totals row and the experience ranking below it.
 */
static void field_open_party_summary(void)
{
    FieldMenuElement *record;
    s32 actor_index;
    s32 active_count;

    func_800ADF34();
    g_field_dialog_screen_mode = FIELD_RESULTS_PARTY_SUMMARY;
    field_play_sound(FIELD_SOUND_WINDOW_OPEN, FIELD_SOUND_CENTER);

    active_count = 0;
    record = func_800ADF84();
    actor_index = active_count;
    /* The callback has its own prototype; cast to the element draw type. */
    record->draw = (FieldMenuDrawFn)field_draw_party_totals;
    record->attr.bits.step = 1;
    record->attr.bits.x = 0x20;
    record->attr.bits.y = 0x30;
    record->size.bits.width_high = 1;
    record->size.bits.height = 0x20;
    record->attr.word &= ~FIELD_MENU_ATTR_WIDTH_LOW;

    for (actor_index = 0; actor_index < 3; actor_index++)
    {
        if (g_field_player_records[actor_index].flags & FIELD_PLAYER_ACTIVE)
        {
            active_count++;
        }
    }

    record = func_800ADF84();
    record->draw = (FieldMenuDrawFn)field_draw_experience_ranking;
    record->attr.bits.step = 1;
    record->attr.bits.x = 0x20;
    record->attr.bits.y = 0x58;
    record->size.bits.width_high = 1;
    record->size.bits.height = ((active_count * 28) + 16) & 0xFF;
    record->attr.word &= ~FIELD_MENU_ATTR_WIDTH_LOW;
}

/**
 * @brief Draw the party totals row (two per-player counters summed, money gained).
 * @param ot Ordering table.
 * @param cursor Primitive cursor.
 * @param x_offset Horizontal window offset subtracted from each column.
 * @param y_offset Vertical window offset subtracted from each row.
 * @return Primitive cursor after the row.
 * @note While g_field_results_wait_frames runs it also waits for queued resources.
 */
static void* field_draw_party_totals(void* ot, void* cursor, s32 x_offset, s32 y_offset)
{
    s32 i;
    s32 total_x;
    s32 total_y;
    u8 *text_base;
    void *handle;
    void *first_cursor;
    Vec2s position;
    s16 y;

    if (g_field_results_wait_frames != 0)
    {
        first_cursor = cursor;
        if ((x_offset | y_offset) != 0)
        {
            goto loop_setup;
        }
        g_field_results_wait_frames--;
        if (g_field_results_wait_frames != 0)
        {
            goto loop_setup;
        }
        if (func_800B0888(first_cursor) != 0)
        {
            g_field_results_wait_frames = 1;
        }
    }

    /* The gotos skip this reassignment (UNSOLVED lever, see the hu34 report). */
    first_cursor = cursor;
loop_setup:
    i = 0;
    total_x = i;
    total_y = i;
    for (; i < 3; i++)
    {
        if (g_pad_ctx->characters[i].name[0] != 0)
        {
            total_x += g_field_player_records[i].unk25C;
            total_y += g_field_player_records[i].unk25D;
        }
    }

    handle = func_800A88A0(first_cursor, ot,
        D_800EC3D6[0] + (D_800EC3D6 - 0x12) + (D_800EC3D6[1] << 8),
        4, 0x10 - x_offset, -y_offset, 0);
    text_base = D_800EC3D6 - 0x12;

    y = 0x10 - y_offset;
    position.y = y;
    handle = field_draw_animated_marker(ot, handle, 0x18 - x_offset, y - 3);

    position.x = 0x28 - x_offset;
    handle = func_800A8A78(ot, handle, total_x, 4, &position, 0);
    handle = func_800A88A0(handle, ot,
        FIELD_TEXT_AT(text_base, text_base[0x16], text_base[0x17]),
        4, 0x40 - x_offset, position.y, 0);
    handle = field_draw_animated_small_icon(ot, handle, 0x50 - x_offset, position.y + 2);

    position.x = 0x60 - x_offset;
    handle = func_800A8A78(ot, handle, total_y, 4, &position, 0);
    handle = func_800A88A0(handle, ot,
        FIELD_TEXT_AT(text_base, text_base[0x1A], text_base[0x1B]),
        4, 0x80 - x_offset, position.y, 0);

    position.x = 0xD0 - x_offset;
    handle = func_800A8A78(ot, handle, g_pad_ctx->money - g_field_money_snapshot, 4, &position, 1);
    return func_800A88A0(handle, ot,
        FIELD_TEXT_AT(text_base, text_base[0], text_base[1]),
        4, 0xF0 - x_offset, position.y, 1);
}

/**
 * @brief Draw up to three party members sorted by experience gained, highest first.
 * @param ordering_table Ordering table address passed to the drawing helpers.
 * @param cursor Initial primitive-buffer address.
 * @param x_offset Horizontal drawing origin subtracted from each column position.
 * @param y_offset Vertical drawing origin subtracted from each row position.
 * @return Primitive-buffer address after the final emitted element.
 * @note Equal gains keep party order during the insertion sort.
 * @note The draw guard repeats the loop test `i < count` so CSE folds the
 *       rotated loop entry test away.
 */
static void* field_draw_experience_ranking(void* ordering_table, void* cursor, s32 x_offset, s32 y_offset)
{
    FieldRankWork work;
    s32 count;
    s32 i;
    s32 pos;
    s32 j;
    s32 row;
    void* result;
    s16 y;
    u8 *text;
    u8 *text_base;

    result = func_800A88A0(cursor, ordering_table, (D_800EC3C6[1] << 8) + ((D_800EC3C6 - 2) + D_800EC3C6[0]), 4, 0x10 - x_offset, -y_offset, 0);
    count = 0;
    for (i = 0; i < 3; i++)
    {
        if (g_pad_ctx->characters[i].name[0] != 0)
        {
            pos = 0;
            for (; pos < count; pos++)
            {
                if (FIELD_EXPERIENCE_GAIN(i) > FIELD_EXPERIENCE_GAIN(work.order[pos]))
                {
                    break;
                }
            }
            if (pos == count)
            {
                work.order[count] = i;
            }
            else
            {
                for (j = count - 1; j >= pos; j--)
                {
                    work.order[j + 1] = work.order[j];
                }
                work.order[pos] = i;
            }
            count++;
        }
    }
    i = 0;
    if (i < count)
    {
        text_base = D_800EC3DA;
        text = text_base - 0x16;
        for (row = 0; i < count; i++)
        {
            if (g_pad_ctx->characters[work.order[i]].name[0] != 0)
            {
                work.position.x = 0x18 - x_offset;
                y = y_offset - 0x10;
                y = row - y;
                work.position.y = y;
                result = field_emit_actor_portrait(result, ordering_table, work.order[i], &work.position);
                y += 8;
                work.position.y = y;
                result = field_draw_animated_icon(ordering_table, result, 0x38 - x_offset, y - 8, 1);
                work.position.x = 0x48 - x_offset;
                result = func_800A8A78(ordering_table, result, g_field_player_records[work.order[i]].unk25A, 4, &work.position, 0);
                result = func_800A88A0(result, ordering_table, FIELD_TEXT_AT(text, D_800EC3DA[0], text_base[1]), 4, 0x68 - x_offset, work.position.y, 0);
                result = field_draw_animated_icon(ordering_table, result, 0x78 - x_offset, work.position.y, 0);
                work.position.x = 0x88 - x_offset;
                work.position.y = y;
                result = func_800A8A78(ordering_table, result, g_field_player_records[work.order[i]].unk25B, 4, &work.position, 0);
                result = func_800A88A0(result, ordering_table, FIELD_TEXT_AT(text, text[0x1A], text[0x1B]), 4, 0xA8 - x_offset, work.position.y, 0);
                work.position.x = 0xE0 - x_offset;
                work.position.y = y;
                result = func_800A8A78(ordering_table, result, FIELD_EXPERIENCE_GAIN(work.order[i]), 4, &work.position, 1);
            }
            row += 0x1C;
        }
    }
    return result;
}

/**
 * @brief Draw the battle item list: a header, then each item name and its quantity.
 * @param ot Ordering table.
 * @param prim Primitive cursor.
 * @param x_offset Horizontal window offset (the quantity row also subtracts it from y).
 * @param y_offset Vertical window offset.
 * @return Primitive cursor after the list.
 */
static void* field_draw_item_list(void* ot, void* prim, s32 x_offset, s32 y_offset)
{
    s32 i;
    Vec2s pos;
    u8 pad[0x84]; /* never used; the original frame reserves it */
    s32 row;
    s32 low;
    s32 offset;
    u8* tex;

    low = D_800EC3D8.low;
    offset = (D_800EC3D8.high << 8) + (s32)((u8*)&D_800EC3D8 - 0x14);
    tex = (u8*)(low + offset);
    prim = func_800A88A0(prim, ot, tex, 4, 0x20 - x_offset, -y_offset, 0);
    for (i = 0; i < g_field_dialog_item_count; i++)
    {
        row = i * FIELD_RESULTS_ROW_HEIGHT;
        prim = func_800A88A0(prim, ot, g_field_dialog_item_texts[i], 4, 0x10 - x_offset, (row + FIELD_RESULTS_ROW_HEIGHT) - y_offset, 0);
        pos.x = 0xB0 - x_offset;
        /* The original subtracts the horizontal offset here too. */
        pos.y = (row + FIELD_RESULTS_ROW_HEIGHT) - x_offset;
        if (g_field_dialog_item_quantities[i] != 0)
        {
            prim = func_800A8A78(ot, prim, g_field_dialog_item_quantities[i], 4, &pos, 1);
        }
    }
    return prim;
}

/**
 * @brief Draw the visible rows of the unlock list, with one header before the abilities and one before the techniques.
 * @param ordering_table Ordering table.
 * @param cursor Primitive cursor.
 * @param scroll_x Horizontal scroll offset.
 * @param scroll_y Vertical scroll offset.
 * @param viewport_height Bottom clipping boundary.
 * @return Primitive buffer cursor after drawing the visible text.
 * @note The guard repeats the loop test so CSE folds the rotated entry test;
 *       the count is re-read on every pass because the loop calls out.
 */
static void* field_draw_unlock_list(void* ordering_table, void* cursor, s32 scroll_x, s32 scroll_y, s32 viewport_height)
{
    u8 pad[8]; /* never used; the original frame reserves it */
    u8* ability_names;
    u8* technique_names;
    s32 header_x;
    s32 item_x;
    s32 ability_header_y;
    s32 technique_header_y;
    void* next_cursor;
    s32 technique_header_drawn;
    s32 row;
    s32 index;
    s32 ability_header_drawn;
    s32 technique_row_y;
    s32 ability_row_y;
    s32 item_y;
    u16 *entry;
    u16 name_index;

    next_cursor = cursor;
    technique_header_drawn = 0;
    row = 0;
    ability_header_drawn = 0;
    index = 0;
    ability_names = g_field_cd_buffer + ((TextResource*)g_field_cd_buffer)->ability_names;
    technique_names = g_field_cd_buffer + ((TextResource*)g_field_cd_buffer)->technique_names;
    if (index < g_field_progression_unlock_count)
    {
        header_x = FIELD_RESULTS_HEADER_X - scroll_x;
        item_x = FIELD_RESULTS_NAME_X - scroll_x;
        entry = g_field_progression_unlocks;
        for (; index < g_field_progression_unlock_count; index++)
        {
            if (*entry & FIELD_UNLOCK_TECHNIQUE)
            {
                technique_row_y = row * FIELD_RESULTS_ROW_HEIGHT;
                if (technique_header_drawn == 0)
                {
                    technique_header_y = technique_row_y - scroll_y;
                    if ((technique_header_y > -FIELD_RESULTS_ROW_HEIGHT) && (technique_header_y < viewport_height))
                    {
                        s32 low;
                        s32 offset;
                        u8* base;

                        low = D_800EC3CE.low;
                        base = D_800EC3C4;
                        offset = (D_800EC3CE.high << 8) + (s32)base;
                        next_cursor = func_800A88A0(next_cursor, ordering_table, (void *)(low + offset), 4, header_x, technique_header_y, 0);
                    }
                    technique_header_drawn = 1;
                    row += 1;
                    technique_row_y = row * FIELD_RESULTS_ROW_HEIGHT;
                }
                item_y = technique_row_y - scroll_y;
                if (item_y > -FIELD_RESULTS_ROW_HEIGHT)
                {
                    if (item_y < viewport_height)
                    {
                        name_index = *entry & ~FIELD_UNLOCK_TECHNIQUE;
                        next_cursor = func_800A88A0(next_cursor, ordering_table,
                                                    technique_names +
                                                        ((u16 *)technique_names)[name_index],
                                                    4, item_x, item_y, 2);
                    }
                }
            }
            else
            {
                ability_row_y = row * FIELD_RESULTS_ROW_HEIGHT;
                if (ability_header_drawn == 0)
                {
                    ability_header_y = ability_row_y - scroll_y;
                    if ((ability_header_y > -FIELD_RESULTS_ROW_HEIGHT) && (ability_header_y < viewport_height))
                    {
                        next_cursor =
                            func_800A88A0(next_cursor, ordering_table,
                                          D_800EC3C4 + D_800EC3CC.low + (D_800EC3CC.high << 8),
                                          4, header_x, ability_header_y, 0);
                    }
                    ability_header_drawn = 1;
                    row += 1;
                    ability_row_y = row * FIELD_RESULTS_ROW_HEIGHT;
                }
                item_y = ability_row_y - scroll_y;
                if (item_y > -FIELD_RESULTS_ROW_HEIGHT)
                {
                    if (item_y < viewport_height)
                    {
                        name_index = *entry;
                        next_cursor = func_800A88A0(next_cursor, ordering_table,
                                                    ability_names +
                                                        ((u16 *)ability_names)[name_index],
                                                    4, item_x, item_y, 2);
                    }
                }
            }
            row += 1;
            entry += 1;
        }
    }
    return next_cursor;
}

/**
 * @brief Draw an animated icon (four-frame ping-pong animation from the frame counter).
 * @param ordering_table Ordering-table tag to link the primitive into.
 * @param prim Primitive buffer slot to populate.
 * @param x Left screen coordinate.
 * @param y Top screen coordinate.
 * @param wide Selects the larger geometry and texture region when nonzero.
 * @return Pointer just past the emitted primitive.
 */
static POLY_FT4* field_draw_animated_icon(u32* ordering_table, POLY_FT4* prim, s32 x, s32 y, s32 wide)
{
    s32 phase_table[8] = {0, 1, 2, 3, 2, 1, 0, 0};
    s32 phase;

    phase = phase_table[((g_frame_counter >> 2) + 1) & 7];

    SET_BGR0_PACKED(prim, GPU_TINT_NEUTRAL);
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

    prim->clut = FIELD_RESULTS_ICON_CLUT;
    prim->tpage = FIELD_RESULTS_ICON_TPAGE;
    addPrim(ordering_table, prim);
    return prim + 1;
}

/**
 * @brief Draw an animated 8x8 icon.
 * @param ordering_table Ordering-table tag receiving the primitive.
 * @param prim Primitive buffer to populate.
 * @param x Left screen coordinate.
 * @param y Top screen coordinate.
 * @return Buffer address immediately after the emitted primitive.
 */
static void* field_draw_animated_small_icon(s32* ordering_table, POLY_FT4* prim, s32 x, s32 y)
{
    FieldQuadAnimationTable table;
    s32 phase_index;
    s32 frame;
    u32 color;
    s32 phase;
    u16 left_u;
    u16 right_u;

    table = g_field_small_icon_frames;
    color = GPU_TINT_NEUTRAL;
    frame = g_frame_counter;
    phase_index = ((frame >> 2) + 2) & 7;
    phase = table.words[phase_index];

    SET_BGR0_PACKED(prim, color);
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
    prim->clut = FIELD_RESULTS_ICON_CLUT;
    prim->tpage = FIELD_RESULTS_ICON_TPAGE;

    phase *= 8;
    left_u = phase + 0x20;
    right_u = phase + 0x28;
    prim->u3 = right_u;
    prim->u1 = right_u;
    prim->u2 = left_u;
    prim->u0 = left_u;

    addPrim(ordering_table, prim);
    return prim + 1;
}

/**
 * @brief Draw an animated marker whose position, texture and width follow the frame counter.
 * @param ordering_table Ordering-table entry receiving the primitive.
 * @param prim Writable primitive buffer.
 * @param x Horizontal origin.
 * @param y Vertical origin.
 * @return Buffer address immediately after the emitted primitive.
 */
static void* field_draw_animated_marker(s32* ordering_table, POLY_FT4* prim, s32 x, s32 y)
{
    s32 frame;
    u32 color;
    FieldQuadAnimationTable tables[4];
    u16 left_x;
    u16 right_x;
    u16 width;
    u8 texture_u;
    s32 phase_index;

    tables[0] = g_field_small_icon_frames;
    tables[1] = g_field_marker_x_offsets;
    tables[2] = g_field_marker_u_offsets;
    tables[3] = g_field_marker_widths;
    color = GPU_TINT_NEUTRAL;
    frame = g_frame_counter;
    SET_BGR0_PACKED(prim, color);
    setPolyFT4(prim);
    phase_index = ((frame >> 2) + 3) & 7;
    left_x = (u16)tables[1].words[phase_index] + x;
    prim->x2 = left_x;
    prim->x0 = left_x;
    width = tables[3].words[phase_index];
    prim->y1 = y;
    prim->y0 = y;
    y += 0x10;
    prim->y3 = y;
    prim->y2 = y;
    right_x = left_x + width;
    prim->x3 = right_x;
    prim->x1 = right_x;
    texture_u = tables[2].words[phase_index];
    prim->u2 = texture_u;
    prim->u0 = texture_u;
    texture_u += tables[3].words[phase_index];
    prim->u3 = texture_u;
    prim->u1 = texture_u;
    prim->v1 = 0x80;
    prim->v0 = 0x80;
    prim->v3 = 0x90;
    prim->v2 = 0x90;
    prim->clut = FIELD_RESULTS_ICON_CLUT;
    prim->tpage = FIELD_RESULTS_ICON_TPAGE;
    addPrim(ordering_table, prim);
    return prim + 1;
}

/**
 * @brief Draw the dialog windows (the result screens or the return-to-title prompt).
 */
void field_draw_dialog_windows(void)
{
    func_800AE008();
}
