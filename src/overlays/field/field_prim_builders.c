#include "common.h"
#include "sdk/libgte.h"
#include "sdk/libgpu.h"

/** @brief 0x14-byte sprite primitive as written by func_80085FAC. */
typedef struct
{
    s32 unk0;
    s32 unk4;
    s32 unk8;
    s16 unkC;
    s16 unkE;
    s32 unk10;
} Prim;

/** @brief Object holding an ordering-table tag at 0xC. */
typedef struct
{
    u8 pad[0xC];
    s32 unkC;
} OtLike;

typedef struct
{
    u8 pad0[0x1C];
    u32 unk1C;
    u8 pad20[0x54 - 0x20];
} FieldObjRec86184;

void *func_80085FAC(void *arg0, void *arg1, s32 arg2, s32 *arg3);
void cdrom_queue_read(s32 resource_index, void *dst_buffer);
void cdrom_wait_queue_empty(void);
s32 func_80086374(RECT *rect, u8 *data, s32 mode);

extern s32 D_8010A004;
extern s32 D_8010A010;
extern FieldObjRec86184 D_800FDF58[];
extern u8 D_800FDCEA;
extern u16 D_800FE01E;
extern u8 *D_8010D038;

/**
 * @brief Emit a right-aligned 3-digit decimal number followed by a trailing
 *        glyph, advancing the horizontal cursor 7 units per column.
 * @param arg0 Primitive-buffer cursor passed through the digit emitter.
 * @param arg1 Opaque draw context forwarded to func_80085FAC.
 * @param arg2 Value to render (hundreds/tens/units).
 * @param arg3 Pointer to the current X cursor, bumped by 7 after each column.
 * @note Leading zeros in the hundreds/tens columns are suppressed until the
 *       first non-zero digit is emitted.
 */
void func_80085E84(void *arg0, void *arg1, s32 arg2, u16 *arg3)
{
    s32 emitted;

    emitted = 0;
    if (arg2 / 100 != 0)
    {
        emitted = 1;
        arg0 = func_80085FAC(arg0, arg1, arg2 / 100, (s32 *)arg3);
        arg2 -= 100;
    }
    *arg3 += 7;
    if (emitted || arg2 / 10 != 0)
    {
        s32 digit;
        digit = arg2 / 10;
        arg0 = func_80085FAC(arg0, arg1, digit, (s32 *)arg3);
        arg2 -= digit * 10;
    }
    *arg3 += 7;
    arg0 = func_80085FAC(arg0, arg1, arg2, (s32 *)arg3);
    *arg3 += 7;
    func_80085FAC(arg0, arg1, 10, (s32 *)arg3);
}

/**
 * @brief Build a textured sprite primitive and link it into an ordering table.
 *
 * Fills the primitive at @p arg0 with fixed colour, code, texture, clut, and
 * size fields (the UV taken from @p arg2), copies the packed xy from @p arg3,
 * then splices the primitive ahead of the tag stored at @c arg1->unkC.
 *
 * @param arg0 Primitive packet to populate.
 * @param arg1 Object holding the ordering-table tag at @c unkC.
 * @param arg2 UV selector; scaled by 8 and biased by 0x1558.
 * @param arg3 Source of the packed xy word written to the primitive.
 * @return Pointer just past the emitted primitive (@p arg0 + 0x14).
 * @see decomp.me (100%) TODO
 */
void *func_80085FAC(void *arg0, void *arg1, s32 arg2, s32 *arg3)
{
    Prim *p;
    OtLike *ot;
    s32 temp;

    p = (Prim *)arg0;
    ot = (OtLike *)arg1;

    p->unk4 = 0x808080;
    ((u8 *)p)[3] = 4;
    ((u8 *)p)[7] = 0x64;
    temp = *arg3;
    p->unkC = (s16)((arg2 * 8) + 0x1558);
    p->unk10 = 0xB0008;
    p->unkE = 0x7810;
    p->unk8 = temp;
    p->unk0 = (p->unk0 & 0xFF000000) | (ot->unkC & 0xFFFFFF);
    ot->unkC = (ot->unkC & 0xFF000000) | ((s32)p & 0xFFFFFF);
    return (u8 *)arg0 + 0x14;
}

/**
 * @brief Build a black flat quad from the caller's x0/x1, bordered 3 pixels inward at the bottom, and link it into the OT.
 * @param prim Quad whose x0 and x1 are already set.
 * @param y Top edge, added to D_8010A010.
 * @param ot Ordering-table tag to link into.
 * @return Pointer just past the quad.
 */
POLY_F4 *func_80086030(POLY_F4 *prim, s32 y, u32 *ot)
{
    *((u32 *)&prim->r0) = 0xFF;
    ((P_TAG *)prim)->len = 5, ((P_TAG *)prim)->code = 0x28;
    prim->x3 = prim->x1 - 3;
    prim->y1 = ((u16)D_8010A010) + y;
    prim->x2 = prim->x0 - 3;
    prim->y0 = ((u16)D_8010A010) + y;
    prim->y3 = prim->y1 + ((u16)D_8010A004);
    prim->y2 = prim->y3;
    ((P_TAG *)prim)->addr = (u32)((P_TAG *)ot)->addr,
        ((P_TAG *)ot)->addr = (u32)prim;
    return prim + 1;
}

/**
 * @brief Build a white flat quad like func_80086030, widening a zero-width x0/x1 pair to one pixel first.
 * @param prim Quad whose x0 and x1 are already set.
 * @param y Top edge, added to D_8010A010.
 * @param ot Ordering-table tag to link into.
 * @return Pointer just past the quad.
 */
POLY_F4 *func_800860CC(POLY_F4 *prim, s32 y, u32 *ot)
{
    if (prim->x1 == prim->x0) {
        prim->x1 = prim->x0 + 1;
    }
    *((u32 *)&prim->r0) = 0xFFFFFF;
    ((P_TAG *)prim)->len = 5, ((P_TAG *)prim)->code = 0x28;
    prim->x3 = prim->x1 - 3;
    prim->y1 = ((u16)D_8010A010) + y;
    prim->x2 = prim->x0 - 3;
    prim->y0 = ((u16)D_8010A010) + y;
    prim->y3 = prim->y1 + ((u16)D_8010A004);
    prim->y2 = prim->y3;
    ((P_TAG *)prim)->addr = (u32)((P_TAG *)ot)->addr,
        ((P_TAG *)ot)->addr = (u32)prim;
    return prim + 1;
}

/**
 * @brief Build the textured sprite primitive for a field object icon.
 * @param sprt Sprite primitive to populate.
 * @param ot Ordering table the primitive (and its tpage) are linked into.
 * @param index Object slot index; selects clut/tpage and a special case at 2.
 * @param xy Packed screen position copied into the sprite's x0/y0.
 * @return Pointer just past the appended DR_TPAGE primitive.
 */
void *func_80086184(SPRT *sprt, u32 *ot, s32 index, u32 *xy)
{
    DR_TPAGE *mode;

    *(u32 *)&sprt->r0 = 0x808080;
    setSprt(sprt);
    *(u32 *)&sprt->x0 = *xy;
    *(u16 *)&sprt->u0 = 0xE800;
    *(u32 *)&sprt->w = 0x180018;

    if (index == 2 && D_800FDCEA >= 0x41)
    {
        sprt->clut = (((D_800FE01E & 3) + 0x1EF) << 6) | 0x10;
    }
    else
    {
        sprt->clut = ((index + 0x1F4) << 6) | ((D_800FDF58[index].unk1C >> 19) & 0xF);
    }

    addPrim(ot, sprt);

    mode = (DR_TPAGE *)(sprt + 1);
    if (index >= 2)
    {
        setDrawTPage(mode, 0, 0, getTPage(0, 1, 0x340 - (index << 6), 0));
    }
    else
    {
        setDrawTPage(mode, 0, 0, getTPage(0, 1, 0x380 - (index << 7), 0));
    }
    addPrim(ot, mode);
    return mode + 1;
}

/**
 * @brief Loads a VRAM resource from disc and uploads it via func_80086374.
 *
 * Queues a CD read of resource @p id (masked to 16 bits) into the shared field
 * CD buffer @c D_8010D038, waits for the queue to drain, then hands the loaded
 * blob to func_80086374 to upload into VRAM using @p rect and @p arg2.
 *
 * @param id Resource index; masked to 16 bits for the CD queue.
 * @param rect Destination rectangle forwarded to func_80086374.
 * @param arg2 Upload mode forwarded to func_80086374.
 */
void field_load_vram_resource(s32 id, s16 *rect, s32 arg2)
{
    u8 *buf = D_8010D038;

    cdrom_queue_read(id & 0xFFFF, buf);
    cdrom_wait_queue_empty();
    func_80086374((RECT *)rect, buf, arg2);
}

/**
 * @brief Upload a two-part image resource (palette/CLUT block followed by the
 *        pixel block) into VRAM and report the payload's trailing status word.
 * @param rect Destination framebuffer rectangle; supplies the upload x/y
 *             (from @c rect->x / @c rect->y) and the CLUT width/height (from
 *             @c rect->w / @c rect->h), and is updated on return to the pixel
 *             block's dimensions.
 * @param data Resource blob. Offset 0x8 holds the pixel-block byte offset,
 *             0x10 the CLUT dimensions, 0x14 the CLUT pixels, and 0x1F4 the
 *             status word returned to the caller.
 * @param mode When non-zero, upload the CLUT as a single 1-tall run of
 *             width*height entries; when zero, upload it with its natural
 *             width and height.
 * @return The status word stored at @c data+0x1F4.
 * @note The nested @c do{}while(0) wrappers reproduce the original codegen and
 *       are required to match; do not remove them.
 * @see decomp.me (100.00%)
 */
s32 func_80086374(RECT *rect, u8 *data, s32 mode)
{
    RECT load_rect;
    s32 offset;
    s32 ret;
    u8 *image;
    u8 *dims;
    u16 x;

    dims = data + 0x10;
    do { do { do { do { offset = *(s32 *)(data + 8); } while (0); } while (0); } while (0); } while (0);
    if (mode != 0)
    {
        load_rect.x = rect->w;
        load_rect.y = rect->h;
        load_rect.w = *(u16 *)dims * *(u16 *)(dims + 2);
        load_rect.h = 1;
    }
    else
    {
        load_rect.x = rect->w;
        load_rect.y = rect->h;
        load_rect.w = *(u16 *)dims;
        do { do { load_rect.h = *(u16 *)(dims + 2); } while (0); } while (0);
    }
    LoadImage(&load_rect, (u_long *)(data + 0x14));

    offset += 8;
    image = data + offset;
    x = rect->x;
    ret = *(s32 *)(data + 0x1F4);
    load_rect.x = x;
    load_rect.y = rect->y;
    dims = image + 8;
    load_rect.w = *(u16 *)dims;
    load_rect.h = *(u16 *)(dims + 2);
    LoadImage(&load_rect, (u_long *)(image + 0xC));
    rect->x = *(u16 *)dims;
    rect->y = *(u16 *)(dims + 2);
    return ret;
}
