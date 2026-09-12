#ifndef _RENDER_CONTEXT_H
#define _RENDER_CONTEXT_H

#include "common.h"
#include "sdk/libgte.h"
#include "sdk/libgpu.h"

/**
 * @brief Per-frame render context handed to the in-game overlay modules
 *        (MENU, GNAME, ...).
 *
 * The first 16 words form the ordering table. The next 0x4000 bytes hold
 * primitive packets; the current @c ot array covers both regions. MENU and
 * GNAME clear only the 16 table entries and begin emitting packets at
 * @c &ot[16]. Render passes splice primitives into the table with @c addPrim:
 * @c ot[0x0F] (offset 0x3C) receives the menu grid and name cursor, while other
 * passes use @c ot[0x0E], @c ot[0x0A], or @c ot[0]. The @c prim_cursor member
 * tracks the next packet address, and @c frame_parity selects the active
 * double buffer.
 *
 * @note This is deliberately distinct from the display-buffer struct used by
 *       the full-screen overlays (CHECKPS / TITLE). Those place a @c DISPENV
 *       at offset 0x4040 (verified: CHECKPS passes @c base+0x4040 to
 *       @c PutDispEnv) because they own their display environment. The
 *       MENU entry routine initializes its display and draw environments at
 *       0x4050 and 0x4064. Offset 0x4040 is its packet cursor, not a DISPENV.
 *       These overlays use different buffer layouts.
 */
typedef struct
{
    u_long  ot[0x1010];   /* 0x0000 - 16 OT entries, then 0x4000 bytes of packets */
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
