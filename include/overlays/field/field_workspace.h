#ifndef FIELD_WORKSPACE_H
#define FIELD_WORKSPACE_H

#include "common.h"

#include "main/display.h"
#include "overlays/field/field_text.h"

/**
 * @file field_workspace.h
 * @brief FIELD's resident workspace at FIELD_WORKSPACE_ADDRESS, shared by the main executable's
 *        frame loop, the scene allocator, the text renderer and the scene fade.
 */

/**
 * @brief FIELD's resident workspace.
 * @note top, primitive_buffers and fade_mode are also reached through the globals g_field_mem_top,
 *       g_field_mem_base, g_field_mem_midpoint and g_field_scene_fade_mode; text_configs through
 *       g_field_text_saved_configs.
 * @note The scene fade (field_update_scene_fade) steps fade_level by 8 from fully lit (0x100) while
 *       fade_mode is 1 fading out, 2 held out or 3 fading in; 0 is idle.
 */
typedef struct
{
    /** Top of the scene allocator's region. */
    u8* top;
    /** Text window configurations saved while their slots reopen (FIELD_TEXT_WINDOW_SLOTS). */
    FieldTextConfig* text_configs;
    u8* timed_text;
    /** Primitive buffers of the two render halves, carved from the allocator region. */
    u8* primitive_buffers[DISPLAY_BUFFER_COUNT];
    u32 draw_mode0;
    u32 draw_mode1;
    u16 text_clut;
    u16 text_alt_clut;
    u16 window_clut;
    u16 prompt_clut;
    u16 portrait_clut[2];
    /** Portrait VRAM slots in use, one bit each. */
    u16 portrait_slots;
    u16 _pad2A;
    s32 fade_mode;
    s32 fade_level;
    FieldTextState windows[FIELD_TEXT_WINDOW_SLOTS];
} FieldWorkspace;

#define FIELD_WORKSPACE ((FieldWorkspace*)FIELD_WORKSPACE_ADDRESS)

#endif
