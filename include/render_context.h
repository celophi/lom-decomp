#ifndef _RENDER_CONTEXT_H
#define _RENDER_CONTEXT_H

#include "common.h"

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
    u_long ot[0x1010];   /* 0x0000 - ordering table (24-bit tag-link array) */
    void*  prim_cursor;  /* 0x4040 - next free byte in the primitive heap */
    u8     _pad4044[8];  /* 0x4044 - unknown */
    u32    frame_parity; /* 0x404C - active double-buffer index (0 or 1) */
    /* The buffer continues past 0x4050 (packet heap etc.); not yet mapped. */
} RenderContext;

#endif
