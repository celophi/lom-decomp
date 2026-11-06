#ifndef _MENU_H
#define _MENU_H

#include "common.h"
#include "common/gpu_packet.h"
#include "main/main.h"
#include "common/pad.h"
#include "common/render_context.h"
#include "common/tim.h"
#include <libgte.h>
#include <libgpu.h>
#include <strings.h>
#include "overlays/field/field_input.h"

extern void menu_upload_party_portraits(void);
extern void menu_upload_graphics(void);
extern void menu_state_init(void);
extern void menu_reset_slots(void);
extern void menu_node_tree_init(void);

extern s32 g_menu_frame;
/** Non-zero while the equipment comparison window is open. */
extern s32 g_menu_compare_window_active;
extern s32 g_active_slot;
extern s32 g_script_cursor;
extern s32 g_pad_input_latched;
extern s32 g_script_repeat_last;
extern u8 g_menu_tim[];
extern u32 g_menu_initial_clut_pair;
extern u16 g_menu_glyph_src[];
extern u8* g_menu_state_ptr;
/** Base of the menu state/string data blob referenced through g_menu_state_ptr. */
extern u8 g_menu_state_data;

/** Unsigned screen rectangle used to initialize a menu window slot. */
typedef struct
{
    u16 x;
    u16 y;
    u16 w;
    u16 h;
} MenuSlotRect;

/** @brief List navigation fields sharing a menu slot's flags word. */
typedef union
{
    u32 packed;
    struct
    {
        u16 selected_index;
        u16 count_and_ot; /**< Low nine bits: item count; upper seven: ordering-table index. */
    } fields;
} MenuListNavigation;

/** @brief One 0x24-byte menu window slot. */
typedef struct MenuSlot_s
{
    u8 active;                     /* non-zero when in use (1=opening, 2=open, 3=closing) */
    u8 index;                      /* slot index within the pool */
    u8 anim_frame;                 /* animation frame counter: counts up to 6 on open, down to 0 on close */
    u8 has_title;                  /* non-zero to draw the title/decoration bar above the window */
    MenuListNavigation navigation; /* selection, item count, and ordering-table index */
    u16 x;
    u16 y;
    s16 w;
    s16 h;
    u16 lerp_cur_a; /* current interpolated value A (animated toward lerp_target_a) */
    u16 lerp_cur_b; /* current interpolated value B (animated toward lerp_target_b) */
    u16 lerp_target_a;
    u16 lerp_target_b;
    u8 lerp_steps; /* remaining interpolation steps (countdown divisor); 0 = snap to target */
    u8 _pad[3];
    /* K&R callback signature is required by the original call sites. */
    void* (*content_cb)();
    void (*tick_cb)(struct MenuSlot_s* /* self */); /* per-frame callback while slot is active */
} MenuSlot;

MenuSlot* menu_slot_alloc(s32 ot_index, const MenuSlotRect* rect);

extern MenuSlot g_menu_slots[];

/** @brief One 24-entry scripted controller-input sequence. */
typedef struct
{
    u16 inputs[24]; /**< Pad-input masks terminated by MENU_SCRIPT_END. */
} MenuScript;

extern MenuScript g_script_table[];

/** @brief Terminator value in a @ref MenuScript @c inputs row. */
#define MENU_SCRIPT_END 0xFFFF

/** @name Menu pad-input priority masks
 * @{ */
/** @brief Vertical directional-pad input used for menu navigation. */
#define MENU_PAD_VERTICAL (PADLup | PADLdown)
#define MENU_PAD_SHOULDERS (PADL1 | PADL2 | PADR1 | PADR2)
/** @} */

/**
 * @brief Run one frame of the MENU overlay; FIELD calls it once the overlay is loaded.
 * @param render_buffers Render context of the frame.
 * @return Screen id of the request MENU hands back to FIELD, or 0 while it keeps running.
 */
s32 menu_run(RenderContext* render_buffers);

#endif
