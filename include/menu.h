#ifndef _MENU_H
#define _MENU_H

#include "common.h"
#include "main.h"
#include "render_context.h"
#include "psyq/libgte.h"
#include "psyq/libgpu.h"

extern void func_800AA02C(void);
extern void menu_init_prim_rects(void);
extern void menu_clear_vram(void);
extern void menu_state_init(void);
extern void func_80141324(void);
extern void func_801423D8(void);

extern s32 g_menu_frame;
extern s32 g_menu_unk_e8;
extern s32 g_active_slot;
extern s32 g_script_cursor;
extern s32 g_pad_input_latched;
extern s32 g_script_repeat_last;
extern u8  g_menu_tim[];
extern s32 g_menu_tim_dy;
extern u16 g_menu_glyph_src[];
extern u8* g_menu_state_ptr;
extern u8  D_80151EBC;

/**
 * @brief Rectangle parameter block passed to the VRAM upload primitive.
 *
 * Same shape as PSX libgpu @c RECT but distinct here because some call sites
 * use it as a generic 4-halfword parameter rather than a true VRAM rect.
 */
typedef struct
{
    u16 x;
    u16 y;
    u16 w;
    u16 h;
} Rect16;

/**
 * @brief HUD/menu entry slot allocated from the @c g_menu_slots pool.
 *
 * @note Stride is 0x24 bytes; @c active == 0 marks a free slot.
 */
typedef struct
{
    u8  active;   /* 0x00 — non-zero when in use */
    u8  index;    /* 0x01 — slot index within the pool */
    u8  unk2;     /* 0x02 */
    u8  unk3;     /* 0x03 */
    u32 flags;    /* 0x04 — top 7 bits sourced from the caller (arg0 << 25) */
    u16 x;        /* 0x08 */
    u16 y;        /* 0x0A */
    u16 w;        /* 0x0C */
    u16 h;        /* 0x0E */
    u16 unk10;    /* 0x10 */
    u16 unk12;    /* 0x12 */
    u16 unk14;    /* 0x14 */
    u16 unk16;    /* 0x16 */
    u8  unk18;    /* 0x18 */
    u8  _pad[3];
    u32 unk1C;
    u32 unk20;
} MenuSlot;

extern MenuSlot g_menu_slots[];

/**
 * @brief One row of @c g_script_table — a canned sequence of pad-input masks.
 *
 * The script subsystem (see @ref menu_tick) replays scripted controller input:
 * @c g_active_script selects a row, @c g_script_cursor walks @c inputs, and
 * each value is fed into @c g_pad_input for that frame. A value of @c 0xFFFF
 * terminates the row; on termination the row may repeat @c g_script_repeat_count
 * times. Row stride is 0x30 bytes (24 halfwords).
 *
 * @note @c menu_tick indexes this table with hand-written shift arithmetic
 *       (@c off*3 then @c <<4) to reproduce the original codegen, so it casts
 *       @c g_script_table back to @c u8* rather than using @c inputs directly.
 */
typedef struct
{
    u16 inputs[24];   /* 0x00 — pad-input masks; 0xFFFF terminates */
} MenuScript;

extern MenuScript g_script_table[];

#endif
