#ifndef _RENDER_CONTEXT_H
#define _RENDER_CONTEXT_H

#include "common.h"
#include "psyq_compat/libgte.h"
#include "psyq_compat/libgpu.h"

/**
 * @brief Per-frame render context handed to the in-game overlay modules
 *        (MENU, GNAME, ...).
 *
 * The leading 0x4040 bytes are an ordering table: an array of 24-bit
 * tag-links that overlay render passes splice primitives into via @c addPrim.
 * Different passes target different depth entries, e.g. @c ot[0x0F] (offset
 * 0x3C) for the menu grid and name-cursor passes, @c ot[0x0E] (0x38) and
 * @c ot[0x0A] (0x28) for other GNAME passes, and @c ot[0] for the fade
 * overlay. @c prim_cursor is the running write position in the shared
 * primitive packet heap; @c frame_parity selects the active double buffer.
 *
 * @note This is deliberately distinct from the display-buffer struct used by
 *       the full-screen overlays (CHECKPS / TITLE). Those place a @c DISPENV
 *       at offset 0x4040 (verified: CHECKPS passes @c base+0x4040 to
 *       @c PutDispEnv) because they own their display environment. The
 *       in-game overlays only append primitives and leave the DISPENV/DRAWENV
 *       to the resident engine - so 0x4040 is a heap cursor here, not a
 *       DISPENV. The two layouts are different structs, not a decomp error.
 */
typedef struct
{
    u_long  ot[0x1010];   /* 0x0000 - ordering table (24-bit tag-link array) */
    void*   prim_cursor;  /* 0x4040 - next free byte in the primitive heap */
    RECT    clear_rect;   /* 0x4044 - ClearImage rect for this buffer's draw area */
    u32     frame_parity; /* 0x404C - active double-buffer index (0 or 1) */
    DISPENV disp_env;     /* 0x4050 - display environment for this buffer */
    DRAWENV draw_env;     /* 0x4064 - drawing environment for this buffer */
} RenderContext;          /* 0x40C0 bytes == DRAW_BUF_STRIDE */

/**
 * @brief Stride in bytes between the two double-buffered draw buffers.
 *
 * Each frame's draw buffer (OT + heap + DRAWENV) occupies this many bytes, so
 * the inactive buffer is at @c base + ((parity ^ 1) * DRAW_BUF_STRIDE).
 */
#define DRAW_BUF_STRIDE 0x40C0

/**
 * @brief Offset of the DRAWENV reserve slot within a draw buffer.
 *
 * Used as the @c SetDrawEnv target (and the template-packet source) for the
 * inactive frame: @c base + ((parity ^ 1) * DRAW_BUF_STRIDE) + DRAW_BUF_DRAWENV_OFF.
 */
#define DRAW_BUF_DRAWENV_OFF 0x4064

#endif
