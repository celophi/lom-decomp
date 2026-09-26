#include "wmap_model_render.h"
#include "wmap_map_display.h"
#include "wmap_resource_support.h"
#include "wmap_main.h"
#include "wmap_effect_primitives.h"
#include "wmap_sequence_runtime.h"
#include "wmap_sprite_render.h"
#include "sdk/inline_c.h"
#include "sdk/gte_dmpsx_compat.h"

/** @brief Sequence and callback slot counts, and the number of sprite actors. */
#define WMAP_SEQUENCE_SLOTS 14
#define WMAP_CALLBACK_SLOTS 8
#define WMAP_ACTOR_COUNT 256

/** @brief An animation resource starts with this many sequence offsets, followed by the frame offsets. */
#define WMAP_ANIMATION_SEQUENCES 32
/** @brief Animation duration that never counts down. */
#define WMAP_ANIMATION_HOLD 255
/** @brief End-of-sequence marker in a sequence's frame list. */
#define WMAP_ANIMATION_LOOP 255

/** @brief Map-to-model and model scales used to project a map position (see wmap_view_effects.c). */
#define WMAP_MAP_PROJECTION_SCALE 0x14000
#define WMAP_VIEW_SCALE 0x6000
/** @brief Map units between two land cells, and map distance of one cell. */
#define WMAP_CELL_SIZE 160
#define WMAP_CELL_SPACING 48

/** @brief Step counts of the land focus, land entry and land return sequences. */
#define WMAP_LAND_FOCUS_STEPS 10
#define WMAP_LAND_ENTRY_STEPS 4
#define WMAP_LAND_RETURN_STEPS 6

/** @brief Default screen position of the land focus. */
#define WMAP_FOCUS_DEFAULT_X 164
#define WMAP_FOCUS_DEFAULT_Y 105

/** @brief Land entered without a screen fade. */
#define WMAP_NO_FADE_LAND 31

/** @brief Resource slot containing the animation data block. */
typedef struct
{
    s32 unknown_00;
    u8* data;
} WmapAnimationResource;

/** @brief Map scroll position and projection scale (see wmap_view_effects.c). */
typedef struct
{
    s32 x;
    s32 y;
    s32 projection_scale;
    s32 unknown_0c;
} WmapView;

/** @brief First word of a map cell: the land placed there. */
typedef struct
{
    s32 land_id;
    u8 unknown_04[36];
} WmapLandCell;

/** @brief Two signed coordinates stored consecutively. */
typedef struct
{
    s16 x;
    s16 y;
} WmapCoordinatePair;

extern s32 g_wmap_sequence_count;
extern s32 g_wmap_sequence_active[];
extern WmapSequenceCallback g_wmap_sequences[];
extern s32 D_8013B20C;
extern u32 g_wmap_land_entry_step;
extern u32 g_wmap_land_focus_step;
extern s32 g_wmap_land_focus_timer;
extern void (*g_wmap_land_focus_steps[WMAP_LAND_FOCUS_STEPS])(void);
extern s32 D_8011D4FC;
extern s32 D_80182E34;
extern s32 D_800DBE70;
extern s32 D_800DBE78;
extern s32 D_8013B208;
extern s32 g_wmap_callback_active[];
extern WmapSequenceCallback g_wmap_callbacks[];
extern WmapSpriteActor D_800D9268[];
extern WmapAnimationResource D_80139988[];

extern s32 D_8011D510;
extern s32 D_8011D530;
extern WmapView g_wmap_view;
extern s32 D_800D923C;
extern s32* D_80139280;
extern s32 D_80139234;
extern s32 D_8013923C;
extern s32 D_80139240;
extern s32 D_8013924C;
extern s32 D_80139250;
extern s32 D_80139260;
extern s32 D_80139264;
extern s32 D_80139268;
extern s32 D_8013926C;
extern s32 D_80139284;
extern WmapLandCell D_80139290[][6];
extern VECTOR D_8011CF60;
extern WmapCoordinatePair g_wmap_focus_screen_position;
extern SVECTOR g_wmap_camera_rotation;
extern s32 g_wmap_land_entry_timer;
extern void (*g_wmap_land_entry_steps[WMAP_LAND_ENTRY_STEPS])(void);
extern u32 g_wmap_land_return_step;
extern s32 g_wmap_land_return_timer;
extern void (*g_wmap_land_return_steps[WMAP_LAND_RETURN_STEPS])(void);
extern s32 g_wmap_vehicle_cell_x;
extern s32 g_wmap_vehicle_cell_y;
extern s32 g_wmap_view_scroll_mode;
extern s32 g_wmap_scroll_remaining_x;
extern s32 g_wmap_scroll_remaining_y;

/* Sequence steps: reached through the step tables and from the step before them. */
void wmap_land_entry_start_focus(void);
void wmap_land_focus_reset(void);
void wmap_land_focus_wait_2(void);
void wmap_land_focus_fade_out(void);
void wmap_land_focus_wait_4(void);
void wmap_land_focus_hold(void);
void wmap_land_focus_wait_6(void);
void wmap_land_focus_lock(void);
void wmap_land_focus_wait_8(void);
void wmap_land_entry_reset(void);
void wmap_land_entry_wait(void);
void wmap_land_entry_finish(void);
void wmap_land_return_reset(void);
void wmap_land_return_start(void);
void wmap_land_return_wait_sequences(void);
void wmap_land_return_scroll_back(void);
void wmap_land_return_wait_scroll(void);
void wmap_land_return_finish(void);
static void wmap_set_camera_model_transform(VECTOR* translation, SVECTOR* rotation);
static s32 wmap_run_land_return(s32 reset);

/**
 * @brief Install and initialize a counted sequence in the first free slot.
 * @param callback Sequence callback, called with 1 to initialize and 0 to update.
 */
static inline void wmap_start_sequence_slot(WmapSequenceCallback callback)
{
    s32 i;

    for (i = 0; i < WMAP_SEQUENCE_SLOTS; i++)
    {
        if (g_wmap_sequence_active[i] == 0)
        {
            g_wmap_sequences[i] = callback;
            g_wmap_sequence_active[i] = 1;
            (g_wmap_sequences[i])(1);
            g_wmap_sequence_count++;
            break;
        }
    }
    if (i == WMAP_SEQUENCE_SLOTS)
    {
        func_80064F14();
    }
}

/**
 * @brief Land entry step 1: start the land focus sequence and mark the land effect as running.
 */
void wmap_land_entry_start_focus(void)
{
    wmap_start_sequence_slot(wmap_run_land_focus);
    D_8013B20C = 1;
    g_wmap_land_entry_step++;
    wmap_land_entry_wait();
}

/**
 * @brief Run the current step of the land focus sequence (scroll to the land, fade, project it).
 * @param reset Nonzero restarts the sequence instead of running a step.
 * @return 1 while the sequence runs, 0 once it has finished.
 */
s32 wmap_run_land_focus(s32 reset)
{
    s32 result;

    if (reset != 0)
    {
        g_wmap_land_focus_step = 1;
        g_wmap_land_focus_timer = 1;
        return 1;
    }

    if (g_wmap_land_focus_step < WMAP_LAND_FOCUS_STEPS)
    {
        g_wmap_land_focus_steps[g_wmap_land_focus_step]();
        result = 1;
    }
    else
    {
        result = 0;
    }
    return result;
}

/**
 * @brief Land focus step 0: restart the sequence.
 */
void wmap_land_focus_reset(void)
{
    g_wmap_land_focus_step = 1;
    g_wmap_land_focus_timer = 1;
}

/**
 * @brief Land focus step 2: wait for the step timer.
 */
void wmap_land_focus_wait_2(void)
{
    if (--g_wmap_land_focus_timer == 0)
    {
        g_wmap_land_focus_step += 1;
    }
}

/**
 * @brief Land focus step 3: start the screen fade (not for WMAP_NO_FADE_LAND) and dim the spirit panel.
 */
void wmap_land_focus_fade_out(void)
{
    if (D_8011D4FC != WMAP_NO_FADE_LAND)
    {
        g_wmap_screen_fade_mode = 2;
    }
    D_80182E34 = 3;
    g_wmap_spirit_target_brightness = 0;
    g_wmap_land_focus_timer = 2;
    g_wmap_land_focus_step += 1;
}

/**
 * @brief Land focus step 4: wait for the step timer.
 */
void wmap_land_focus_wait_4(void)
{
    if (--g_wmap_land_focus_timer == 0)
    {
        g_wmap_land_focus_step += 1;
    }
}

/**
 * @brief Land focus step 5: wait four frames.
 */
void wmap_land_focus_hold(void)
{
    g_wmap_land_focus_timer = 4;
    g_wmap_land_focus_step += 1;
}

/**
 * @brief Land focus step 6: wait for the step timer.
 */
void wmap_land_focus_wait_6(void)
{
    if (--g_wmap_land_focus_timer == 0)
    {
        g_wmap_land_focus_step += 1;
    }
}

/**
 * @brief Land focus step 7: set the transition flags and wait eight frames.
 */
void wmap_land_focus_lock(void)
{
    D_800DBE70 = 0;
    D_8013B208 = 1;
    D_800DBE78 = 1;
    g_wmap_land_focus_timer = 8;
    g_wmap_land_focus_step += 1;
}

/**
 * @brief Land focus step 8: wait for the step timer.
 */
void wmap_land_focus_wait_8(void)
{
    if (--g_wmap_land_focus_timer == 0)
    {
        g_wmap_land_focus_step += 1;
    }
}

/**
 * @brief Update every running counted sequence; a sequence that returns 0 is freed and uncounted.
 */
void wmap_update_sequences(void)
{
    s32 i;
    s32 result;

    for (i = 0; i < WMAP_SEQUENCE_SLOTS; i++)
    {
        if (g_wmap_sequence_active[i] != 0)
        {
            result = g_wmap_sequences[i](0);
            g_wmap_sequence_active[i] = result;
            if (result == 0)
            {
                g_wmap_sequence_count--;
            }
        }
    }
}

/**
 * @brief Start a counted sequence in the first free slot (g_wmap_sequence_count blocks map input).
 * @param callback Callback, called with 1 to initialize and 0 to update.
 */
void wmap_start_sequence(WmapSequenceCallback callback)
{
    wmap_start_sequence_slot(callback);
}

/**
 * @brief Update every installed callback and keep the ones that return nonzero.
 */
void wmap_update_callbacks(void)
{
    s32 i;

    for (i = 0; i < WMAP_CALLBACK_SLOTS; i++)
    {
        if (g_wmap_callback_active[i] != 0)
        {
            g_wmap_callback_active[i] = g_wmap_callbacks[i](0);
        }
    }
}

/**
 * @brief Install an uncounted callback in the first free slot and run its initialization.
 * @param callback Callback, called with 1 to initialize and 0 to update.
 */
void wmap_install_callback(WmapSequenceCallback callback)
{
    s32 i;

    for (i = 0; i < WMAP_CALLBACK_SLOTS; i++)
    {
        if (g_wmap_callback_active[i] == 0)
        {
            g_wmap_callbacks[i] = callback;
            g_wmap_callback_active[i] = 1;
            g_wmap_callback_active[i] = g_wmap_callbacks[i](1);
            return;
        }
    }
}

/**
 * @brief Advance the selected animation, wrapping when its end marker is reached.
 * @param actor_data Actor animation state and frame pointers.
 * @param resource_data Animation block containing sequence and frame offsets.
 * @return New frame index, or -1 when the current frame is still active.
 */
s32 wmap_step_actor_animation(void* actor_data, void* resource_data)
{
    WmapSpriteActor* actor = actor_data;
    WmapAnimationResource* resource = resource_data;
    u8* data;
    s16* offsets;
    u8* cursor;
    s32 result;
    s32 frame;

    result = -1;
    offsets = (s16*)resource->data;
    data = (u8*)offsets;
    if (actor->previous_sequence != actor->sequence)
    {
        actor->previous_sequence = actor->sequence;
        actor->sequence_start = (u8*)offsets + offsets[actor->sequence];
        actor->cursor = actor->sequence_start;
        actor->remaining = 1;
    }
    if (actor->remaining != WMAP_ANIMATION_HOLD)
    {
        actor->remaining--;
    }
    if (actor->remaining == 0)
    {
        cursor = actor->cursor;
        frame = cursor[0];
        actor->remaining = cursor[1];
        if (frame == WMAP_ANIMATION_LOOP)
        {
            cursor = actor->sequence_start;
            actor->cursor = cursor;
            frame = cursor[0];
            actor->remaining = cursor[1];
        }
        actor->cursor += 4;
        actor->frame_data = data + offsets[WMAP_ANIMATION_SEQUENCES + frame];
        result = frame;
    }
    return result;
}

/**
 * @brief Give every sprite actor and animation slot its index and clear all sequence and callback slots.
 */
void wmap_init_sequences(void)
{
    s32 i;

    for (i = 0; i < WMAP_ACTOR_COUNT; i++)
    {
        D_800D9268[i].unknown_00 = i;
        D_80139988[i].unknown_00 = i;
        D_800D9268[i].unknown_02 = -1;
    }
    for (i = WMAP_SEQUENCE_SLOTS - 1; i >= 0; i--)
    {
        g_wmap_sequence_active[i] = 0;
    }
    for (i = WMAP_CALLBACK_SLOTS - 1; i >= 0; i--)
    {
        g_wmap_callback_active[i] = 0;
    }
}

/**
 * @brief Draw a model with no screen offset and the default depth divisor.
 */
void wmap_draw_model_default(u8* resource_table, s32 resource_index, s32 ot_index, s32 tpage, s32 clut, s32 blend_mode, s32 color_scale)
{
    wmap_draw_model(resource_table, resource_index, ot_index, tpage, clut, blend_mode, color_scale, 0, 0, -1);
}

/**
 * @brief Project the focused land cell, relative to the map view, through the current GTE matrix.
 */
void wmap_project_focus_cell(void)
{
    SVECTOR position;

    position.vz = 0;
    position.vx = (((D_8011D510 - 1) * WMAP_CELL_SIZE - g_wmap_view.x * WMAP_MAP_PROJECTION_SCALE / g_wmap_view.projection_scale) * WMAP_VIEW_SCALE) / g_wmap_view.projection_scale;
    position.vy = (((D_8011D530 - 1) * WMAP_CELL_SIZE - g_wmap_view.y * WMAP_MAP_PROJECTION_SCALE / g_wmap_view.projection_scale) * WMAP_VIEW_SCALE) / g_wmap_view.projection_scale;
    gte_ldv0(&position);
    gte_rtps();
}

/**
 * @brief Scale packed RGB channels, preserving the high byte.
 * @param color Packed RGB value and high byte.
 * @param scale Scale in units of 1/128, or -1 to preserve the color.
 * @return Packed scaled color; channel results wrap to eight bits.
 */
s32 wmap_scale_color(CVECTOR color, s32 scale)
{
    /* The packet color word is passed by value and returned in packed form. */
    CVECTOR* channels = &color;
    s32 red;
    s32 green;
    s32 blue;
    if (scale == -1)
    {
        return *(s32*)&color;
    }
    else
    {
        red = channels->r * scale;
        green = channels->g * scale;
        blue = channels->b * scale;
        channels->r = red >> 7;
        channels->g = green >> 7;
        channels->b = blue >> 7;
        return *(s32*)&color;
    }
}

/**
 * @brief Install the model transform through the helper selected by D_800D923C.
 */
void wmap_set_model_transform(VECTOR* translation, SVECTOR* rotation)
{
    if (D_800D923C != 0)
    {
        func_8006ADD0(translation, rotation);
        return;
    }
    wmap_set_camera_model_transform(translation, rotation);
}

/**
 * @brief Call func_8006D014 with its last drawing parameter cleared.
 * @param actor Actor configuration passed to the drawing helper.
 * @param resource Resource slot passed to the drawing helper.
 * @param arg2 TODO: drawing parameter meaning unknown.
 * @param arg3 TODO: drawing parameter meaning unknown.
 * @param arg4 TODO: drawing parameter meaning unknown.
 * @param arg5 TODO: drawing parameter meaning unknown.
 */
void func_8006CFE4(void* actor, void* resource, s32 arg2, s32 arg3, s32 arg4, s32 arg5)
{
    func_8006D014(actor, resource, arg2, arg3, arg4, arg5, 0);
}

/**
 * @brief Copy the global effect parameters into the descriptor and draw it.
 * @param actor Actor configuration passed to the drawing helper.
 * @param resource Resource slot passed to the drawing helper.
 * @param arg2 TODO: drawing parameter meaning unknown.
 * @param arg3 TODO: drawing parameter meaning unknown.
 * @param arg4 TODO: drawing parameter meaning unknown.
 * @param arg5 TODO: drawing parameter meaning unknown.
 * @param arg6 TODO: drawing parameter meaning unknown.
 */
void func_8006D014(void* actor, void* resource, s32 arg2, s32 arg3, s32 arg4, s32 arg5, s32 arg6)
{
    D_80139280[1] = D_80139234;
    D_80139280[2] = D_8013923C;
    D_80139280[3] = D_80139240;
    D_80139280[4] = D_8013924C;
    D_80139280[5] = D_80139250;
    D_80139280[6] = D_80139260;
    D_80139280[7] = D_80139264;
    D_80139280[8] = D_80139268;
    D_80139280[9] = D_8013926C;
    D_80139280[10] = D_80139284;
    func_8006A2FC(actor, resource, arg2, arg3, arg4, arg5, arg6, D_80139280);
}

/**
 * @brief Find the map cell that holds a land.
 * @param value Land id to find.
 * @param row_out Receives the cell x (first index of D_80139290).
 * @param column_out Receives the cell y.
 * @return One if found, otherwise zero; outputs are unchanged on failure.
 */
s32 wmap_find_land_cell(s32 value, s32* row_out, s32* column_out)
{
    s32 row;
    s32 column;
    for (column = 0; column < 6; column++)
    {
        for (row = 0; row < 6; row++)
        {
            if (D_80139290[row][column].land_id == value)
            {
                *row_out = row;
                *column_out = column;
                return 1;
            }
        }
    }
    return 0;
}

/**
 * @brief Install a rotation matrix with the map translation.
 */
void wmap_set_map_rotation(SVECTOR* rotation)
{
    MATRIX matrix;
    RotMatrix(rotation, &matrix);
    TransMatrix(&matrix, &D_8011CF60);
    SetRotMatrix(&matrix);
    SetTransMatrix(&matrix);
}

/**
 * @brief Put the land focus back at its default screen position.
 */
void wmap_reset_focus_screen_position(void)
{
    g_wmap_focus_screen_position.x = WMAP_FOCUS_DEFAULT_X;
    g_wmap_focus_screen_position.y = WMAP_FOCUS_DEFAULT_Y;
}

/**
 * @brief Compose the camera rotation with a model rotation and install the result.
 */
static void wmap_set_camera_model_transform(VECTOR* translation, SVECTOR* rotation)
{
    MATRIX matrices[2];

    RotMatrix(&g_wmap_camera_rotation, &matrices[0]);
    TransMatrix(&matrices[0], translation);
    SetRotMatrix(&matrices[0]);
    SetTransMatrix(&matrices[0]);
    RotMatrix(rotation, &matrices[1]);
    TransMatrix(&matrices[1], &D_8011CF60);
    CompMatrix(&matrices[0], &matrices[1], &matrices[1]);
    SetRotMatrix(&matrices[1]);
    SetTransMatrix(&matrices[1]);
}

/**
 * @brief Run the current step of the land entry sequence (land focus, then wait for the land effect).
 * @param reset Nonzero restarts the sequence instead of running a step.
 * @return 1 while the sequence runs, 0 once it has finished.
 */
s32 wmap_run_land_entry(s32 reset)
{
    s32 result;

    if (reset != 0)
    {
        g_wmap_land_entry_step = 1;
        g_wmap_land_entry_timer = 1;
        return 1;
    }

    if (g_wmap_land_entry_step < WMAP_LAND_ENTRY_STEPS)
    {
        g_wmap_land_entry_steps[g_wmap_land_entry_step]();
        result = 1;
    }
    else
    {
        result = 0;
    }
    return result;
}

/**
 * @brief Land entry step 0: restart the sequence.
 */
void wmap_land_entry_reset(void)
{
    g_wmap_land_entry_step = 1;
    g_wmap_land_entry_timer = 1;
}

/**
 * @brief Land entry step 2: wait until the land effect clears D_8013B20C.
 */
void wmap_land_entry_wait(void)
{
    if (D_8013B20C == 0)
    {
        g_wmap_land_entry_step += 1;
        wmap_land_entry_finish();
    }
}

/**
 * @brief Land entry step 3: finish the sequence.
 */
void wmap_land_entry_finish(void)
{
    g_wmap_land_entry_step += 1;
}

/**
 * @brief Run the current step of the land return sequence (scroll back to the vehicle cell).
 * @param reset Nonzero restarts the sequence instead of running a step.
 * @return 1 while the sequence runs, 0 once it has finished.
 */
static s32 wmap_run_land_return(s32 reset)
{
    s32 result;

    if (reset != 0)
    {
        g_wmap_land_return_step = 1;
        g_wmap_land_return_timer = 1;
        return 1;
    }

    if (g_wmap_land_return_step < WMAP_LAND_RETURN_STEPS)
    {
        g_wmap_land_return_steps[g_wmap_land_return_step]();
        result = 1;
    }
    else
    {
        result = 0;
    }
    return result;
}

/**
 * @brief Land return step 0: restart the sequence.
 */
void wmap_land_return_reset(void)
{
    g_wmap_land_return_step = 1;
    g_wmap_land_return_timer = 1;
}

/**
 * @brief Land return step 1: start the return.
 */
void wmap_land_return_start(void)
{
    g_wmap_land_return_step += 1;
    wmap_land_return_wait_sequences();
}

/**
 * @brief Land return step 2: wait until no counted sequence is running.
 */
void wmap_land_return_wait_sequences(void)
{
    if (g_wmap_sequence_count == 0)
    {
        g_wmap_land_return_step += 1;
        wmap_land_return_scroll_back();
    }
}

/**
 * @brief Land return step 3: scroll the map back to the vehicle cell.
 */
void wmap_land_return_scroll_back(void)
{
    g_wmap_view_scroll_mode = 2;
    g_wmap_scroll_remaining_x = g_wmap_view.x - ((g_wmap_vehicle_cell_x - 1) * WMAP_CELL_SPACING);
    g_wmap_scroll_remaining_y = g_wmap_view.y - ((g_wmap_vehicle_cell_y - 1) * WMAP_CELL_SPACING);
    g_wmap_land_return_step += 1;
    wmap_land_return_wait_scroll();
}

/**
 * @brief Land return step 4: wait for the map scroll.
 */
void wmap_land_return_wait_scroll(void)
{
    if (g_wmap_view_scroll_mode != 2)
    {
        g_wmap_land_return_step += 1;
        wmap_land_return_finish();
    }
}

/**
 * @brief Land return step 5: clear the transition flag and restore the map state.
 */
void wmap_land_return_finish(void)
{
    D_8013B208 = 0;
    wmap_reset_after_transition();
    g_wmap_land_return_step += 1;
}
