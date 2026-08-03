#ifndef _MENU_H
#define _MENU_H

#include "common.h"
#include "gpu_packet.h"
#include "main.h"
#include "pad.h"
#include "render_context.h"
#include "tim.h"
#include "psyq/libgte.h"
#include "psyq/libgpu.h"
#include "psyq/strings.h"

extern void func_800AA02C(void);
extern void menu_init_prim_rects(void);
extern void menu_upload_graphics(void);
extern void menu_state_init(void);
extern void menu_reset_slots(void);
extern void func_801423D8(void);

extern s32 g_menu_frame;
extern s32 g_menu_unk_e8;
extern s32 g_active_slot;
extern s32 g_script_cursor;
extern s32 g_pad_input_latched;
extern s32 g_script_repeat_last;
extern u8 g_menu_tim[];
extern u32 g_menu_initial_clut_pair;
extern u16 g_menu_glyph_src[];
extern u8* g_menu_state_ptr;
extern u8 D_80151EBC;

/** Unsigned screen rectangle used to initialize a menu window slot. */
typedef struct
{
    u16 x;
    u16 y;
    u16 w;
    u16 h;
} MenuSlotRect;

/**
 * @brief HUD/menu entry slot allocated from the @c g_menu_slots pool.
 *
 * @note Stride is 0x24 bytes; @c active == 0 marks a free slot.
 */
typedef struct MenuSlot_s
{
    u8 active;     /* 0x00 - non-zero when in use (1=opening, 2=open, 3=closing) */
    u8 index;      /* 0x01 - slot index within the pool */
    u8 anim_frame; /* 0x02 - animation frame counter: counts up to 6 on open, down to 0 on close */
    u8 has_title;  /* 0x03 - non-zero to draw the title/decoration bar above the window */
    u32 flags;     /* 0x04 - bits 31:25 select the ordering-table entry */
    u16 x;         /* 0x08 */
    u16 y;         /* 0x0A */
    u16 w;         /* 0x0C */
    u16 h;         /* 0x0E */
    u16 lerp_cur_a;    /* 0x10 - current interpolated value A (animated toward lerp_target_a) */
    u16 lerp_cur_b;    /* 0x12 - current interpolated value B (animated toward lerp_target_b) */
    u16 lerp_target_a; /* 0x14 - target value A for interpolation */
    u16 lerp_target_b; /* 0x16 - target value B for interpolation */
    u8 lerp_steps;     /* 0x18 - remaining interpolation steps (countdown divisor); 0 = snap to target */
    u8 _pad[3];
    /**
     * @brief Content callback - draws the window's interior primitives.
     *
     * Invoked from @c menu_draw_window; receives the slot's OT-link slot,
     * the slot itself, the current primitive write cursor, a forwarded arg,
     * and an active-highlight flag. Returns the new primitive cursor.
     * Empty parameter list (K&R) is intentional to preserve callsite codegen.
     */
    s32* (*content_cb)();                            /* 0x1C */
    void (*tick_cb)(struct MenuSlot_s* /* self */);  /* 0x20 - per-frame callback while slot is active */
} MenuSlot;

MenuSlot* menu_slot_alloc(s32 ot_index, const MenuSlotRect* rect);

extern MenuSlot g_menu_slots[];

/**
 * @brief One row of @c g_script_table - a canned sequence of pad-input masks.
 *
 * The script subsystem (see @ref menu_tick) replays scripted controller input:
 * @c g_active_script selects a row, @c g_script_cursor walks @c inputs, and
 * each value is fed into @c g_pad_input for that frame. A value of @c 0xFFFF
 * terminates the row. Row stride is 0x30 bytes (24 halfwords).
 */
typedef struct
{
    u16 inputs[24]; /* 0x00 - pad-input masks; MENU_SCRIPT_END terminates */
} MenuScript;

extern MenuScript g_script_table[];

/** @brief Terminator value in a @ref MenuScript @c inputs row. */
#define MENU_SCRIPT_END 0xFFFF

/**
 * @name Menu pad-input priority masks
 * @brief Button-group masks used by @ref menu_tick's three-step input filter.
 *
 * Applied in order: if any confirm/cancel bit is set this frame, all other
 * bits are dropped; otherwise if any other face button is set, drop the rest;
 * otherwise if any shoulder is set, drop the rest. The first mask is a subset
 * of the second by design - it gives Triangle/Cross priority over Circle/Square
 * when several face buttons fire on the same frame.
 * @{
 */
#define MENU_PAD_CONFIRM_CANCEL (PADLup | PADLdown)                  /* 0x5000: Triangle | Cross */
#define MENU_PAD_FACE_BUTTONS   (PADLup | PADLright | PADLdown | PADLleft) /* 0xF000: all four face buttons */
#define MENU_PAD_SHOULDERS      (PADL1  | PADL2    | PADR1  | PADR2)      /* 0x000F: L1 | L2 | R1 | R2 */
/** @} */

#endif
