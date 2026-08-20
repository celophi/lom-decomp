#include "field_scene_internal.h"

/**
 * @brief Draw every visible part of every active field object.
 *
 * Walks the scene's object list; for each active object it derives a scroll
 * offset from the camera state (scaled per-axis by the object's definition,
 * optionally negated and wrapped to a power-of-two boundary), applies the
 * object's per-frame drift, then walks the object's part list and submits each
 * visible part to field_draw_part. Parts near a wrap boundary are submitted more
 * than once so they appear on both sides of the seam.
 *
 * @param cursor_ptr Address of the primitive-buffer cursor, forwarded to field_draw_part and
 *             field_draw_marker_overlay.
 * @param ot_base Ordering-table base, forwarded unchanged as the 4th arg of
 *             field_draw_part and 2nd of field_draw_marker_overlay.
 * @param update_mode Mode selector: 0 advances the per-frame drift; 2 forces the
 *             unscaled camera offsets.
 *
 * @warning **THIS FUNCTION IS NOT A MATCH (92.92%) AND MAY NOT BE FUNCTIONALLY
 *          EQUIVALENT.** It is committed as work in progress. Do not rely on
 *          its exact behaviour, and re-verify before building a release image.
 *          The running analysis lives in working/func_80054CA8/status.md,
 *          including eleven measured-and-retired probe classes.
 *
 * @note The signed divides come in TWO forms and the choice is per-site.
 *       `x / 256` yields the compact `bgez / addiu / sra` sequence, which is
 *       what the target uses inside the part loop and for g_field_camera_y /
 *       g_field_camera_z. The head divide of g_field_camera_x and the scroll_x_px/scroll_y_px/scroll_z_px divides use
 *       a two-block form that ONLY appears if the rounding is written out as a
 *       real `if/else`. Collapsing those to `/ 256` costs ~11 exact rows.
 * @note `viewport` is one 5-word array, not five locals: viewport[2..4] are written
 *       and never read here, and survive only because the array's address is
 *       passed to field_draw_part.
 * @note `FieldObjDef.flags` must be `u32` (`srl`, not `sra`) - worth 6 rows.
 * @note The mask must be computed BEFORE loading obj->drift_x / obj->drift_y in the
 *       two wrap blocks - worth 3 rows.
 * @note Measured and rejected, despite the target visibly doing it: writing
 *       `scroll_x`/`scroll_z` as accumulate-in-place (`scroll_x = scroll_x + obj->drift_x`) scores 15-42
 *       exact rows WORSE. See status.md before retrying.
 *
 * @see decomp.me (92.92%) TODO
 */
void field_draw_scene_objects(s32 cursor_ptr, s32 ot_base, s32 update_mode)
{
    s32 viewport[5];
    FieldObj* obj;
    FieldObjDef* def;
    FieldPart* part;
    s32 scroll_x;
    s32 scroll_y;
    s32 scroll_z;
    s32 wrap_size;
    s32 wrap_height;
    s32 scroll_x_px;
    s32 scroll_y_px;
    s32 scroll_z_px;

    wrap_size = 0;
    wrap_height = 0;
    viewport[2] = g_field_scene.scene->header->unk30;
    {
        s32 t = g_field_camera_x;
        s32 q;

        if (t >= 0)
        {
            q = t >> 8;
        }
        else
        {
            q = (t + 0xFF) >> 8;
        }
        viewport[3] = q;
    }
    viewport[4] = (g_field_camera_y / 256 - g_field_camera_z / 512) + 0xE0;
    obj = g_field_scene.scene->objects;
    if (obj != 0)
    {
        do
        {
            if (obj->flags.word & 1)
            {
                def = obj->def;
                scroll_x = 0;
                if (def->flags & 2)
                {
                    scroll_y = 0;
                    scroll_z = 0;
                }
                else
                {
                    u8 sx = def->scroll_scale_x;

                    if ((sx == 0x10) || (update_mode == 2))
                    {
                        scroll_x = g_field_camera_x;
                    }
                    else
                    {
                        u8 mag = sx & 0x7F;
                        s32 v;

                        if (sx & 0x80)
                        {
                            v = -g_field_camera_x;
                        }
                        else
                        {
                            mag = def->scroll_scale_x;
                            v = g_field_camera_x;
                        }
                        scroll_x = (v * mag) / 16;
                    }
                    {
                        u8 sy = def->scroll_scale_y;

                        if ((sy == 0x10) || (update_mode == 2))
                        {
                            scroll_y = ((FieldCamera*)0x801ED480)->y;
                            scroll_z = ((FieldCamera*)0x801ED480)->z;
                        }
                        else
                        {
                            s32 a;
                            s32 b;

                            if (sy & 0x80)
                            {
                                scroll_y = (-g_field_camera_y * (sy & 0x7F)) / 16;
                                a = -g_field_camera_z;
                                b = def->scroll_scale_y & 0x7F;
                            }
                            else
                            {
                                a = def->scroll_scale_y;
                                scroll_y = (g_field_camera_y * a) / 16;
                                b = g_field_camera_z;
                            }
                            scroll_z = (b * a) / 16;
                        }
                    }
                }
                if (obj->flags.b.drift_speed != 0)
                {
                    if (update_mode == 0)
                    {
                        obj->drift_x += (rcos(obj->flags.b.drift_angle * 0x10) * obj->flags.b.drift_speed) / 256;
                        obj->drift_y -= (rsin(obj->flags.b.drift_angle * 0x10) * obj->flags.b.drift_speed) / 256;
                    }
                    if (def->flags & 4)
                    {
                        s32 t;

                        wrap_size = 0x10000 << ((def->flags >> 4) & 3);
                        t = obj->drift_x;
                        if (t >= 0)
                        {
                            obj->drift_x = t & (wrap_size - 1);
                        }
                        else
                        {
                            obj->drift_x = -(-t & (wrap_size - 1));
                        }
                    }
                    if (def->flags & 8)
                    {
                        s32 t;

                        wrap_size = 0x20000 << ((def->flags >> 6) & 3);
                        t = obj->drift_y;
                        if (t >= 0)
                        {
                            obj->drift_y = t & (wrap_size - 1);
                        }
                        else
                        {
                            obj->drift_y = -(-t & (wrap_size - 1));
                        }
                    }
                }
                {
                    s32 tx = scroll_x + obj->drift_x;
                    s32 tz = scroll_z + obj->drift_y;

                    if (tx >= 0)
                    {
                        scroll_x_px = tx >> 8;
                    }
                    else
                    {
                        scroll_x_px = (tx + 0xFF) >> 8;
                    }
                    if (scroll_y >= 0)
                    {
                        scroll_y_px = scroll_y >> 8;
                    }
                    else
                    {
                        scroll_y_px = (scroll_y + 0xFF) >> 8;
                    }
                    if (tz >= 0)
                    {
                        scroll_z_px = tz >> 9;
                    }
                    else
                    {
                        scroll_z_px = (tz + 0x1FF) >> 9;
                    }
                }
                part = obj->parts;
                if (part != 0)
                {
                    do
                    {
                        if ((part->visible != 0) && (part->instance_count != 0))
                        {
                            viewport[0] = scroll_x_px + (obj->x + part->x) / 256;
                            {
                                s32 a = (scroll_y_px - scroll_z_px) + (obj->y + part->y) / 256;
                                s32 d = part->def->u.b.rows * 0x10 - 0xE0;

                                a = a - (obj->z + part->z) / 512;
                                viewport[1] = a - d;
                            }
                            if (def->flags & 4)
                            {
                                wrap_size = 0x100 << ((def->flags >> 4) & 3);
                                if (viewport[0] >= 0)
                                {
                                    viewport[0] = viewport[0] & (wrap_size - 1);
                                }
                                else
                                {
                                    viewport[0] = wrap_size - (-viewport[0] & (wrap_size - 1));
                                }
                            }
                            if (def->flags & 8)
                            {
                                wrap_height = 0x100 << ((def->flags >> 6) & 3);
                                if (viewport[1] >= 0)
                                {
                                    viewport[1] = viewport[1] & (wrap_height - 1);
                                }
                                else
                                {
                                    viewport[1] = wrap_height - (-viewport[1] & (wrap_height - 1));
                                }
                            }
                            field_draw_part(part, cursor_ptr, viewport, ot_base);
                            if (def->flags & 4)
                            {
                                if (viewport[0] > 0)
                                {
                                    viewport[0] -= wrap_size;
                                    field_draw_part(part, cursor_ptr, viewport, ot_base);
                                    viewport[0] += wrap_size;
                                }
                                if (!(def->flags & 0x30))
                                {
                                    s32 t = viewport[0] + wrap_size;

                                    if (t < 0x140)
                                    {
                                        viewport[0] = t;
                                        field_draw_part(part, cursor_ptr, viewport, ot_base);
                                        viewport[0] -= wrap_size;
                                    }
                                }
                            }
                            if (def->flags & 8)
                            {
                                if (viewport[1] > 0)
                                {
                                    viewport[1] -= wrap_height;
                                    field_draw_part(part, cursor_ptr, viewport, ot_base);
                                }
                                if (def->flags & 4)
                                {
                                    if (viewport[0] > 0)
                                    {
                                        viewport[0] -= wrap_size;
                                        field_draw_part(part, cursor_ptr, viewport, ot_base);
                                        viewport[0] += wrap_size;
                                    }
                                    if (!(def->flags & 0x30))
                                    {
                                        s32 t = viewport[0] + wrap_size;

                                        if (t < 0x140)
                                        {
                                            viewport[0] = t;
                                            field_draw_part(part, cursor_ptr, viewport, ot_base);
                                        }
                                    }
                                }
                            }
                        }
                        part = part->next;
                    } while (part != 0);
                }
            }
            obj = obj->next;
        } while (obj != 0);
    }
    if (g_field_marker_overlay_enabled != 0)
    {
        field_draw_marker_overlay((u32*)cursor_ptr, (u32*)ot_base);
    }
}

/**
 * @brief Draw the field's marker overlay: an outline and a numeric label per
 *        marker.
 *
 * Walks the scene's marker list (FieldScene offset 0x10). Each marker emits a
 * red LINE_F4 quad through its def's two points and its own two points, then a
 * red LINE_F2 closing the first point back to the third, then a numeric label
 * (func_800AD208) drawn at the first point with one digit below 10 and two
 * otherwise. All points are shifted by the camera scroll, and every vertical
 * coordinate is halved toward zero. The whole run is chained onto the ordering
 * table entry at @p ot[-1], ahead of whatever the cursor already pointed at.
 *
 * @param cursor Packet-buffer cursor; read for the first primitive address and
 *               written back with the address one past the last primitive.
 * @param ot     Ordering table pointer; the run is linked into @p ot[-1].
 *
 * @note `prim` must be ONE pointer variable advanced in place, not a separate
 *       LINE_F4 and LINE_F2 local: two locals allocate two registers (a3/a1)
 *       where the target carries everything in t0. Splitting them costs 36
 *       exact rows.
 * @note The advance past the LINE_F2 must be its own statement before the
 *       label block, not an argument expression at the call (`prim + 1`):
 *       folding it into the call costs 3 rows.
 * @note `depth` must be assigned BEFORE setLineF4/setRGB0, not after. That one
 *       move is worth 8 exact rows (82.49% -> 92.51%): it lets sched1 hoist the
 *       `lh` above the primitive stores.
 * @note `depth` must exist at all. Written inline as `sy + (def->depth_bias + 0xE0)`
 *       GCC reassociates to `(sy + 0xE0) + def->depth_bias` and hoists the constant
 *       add out of the loop; that costs 18 rows.
 * @note `scene` must be split out of the marker-list read: `g_field_scene.scene`
 *       and `scene->markers` are two statements straddling the scroll divides,
 *       which is what puts the `lw 0x10(a0)` after them. Merging them costs 7
 *       rows.
 * @note `shadow = shadow->next` belongs AFTER the call, not before it (2 rows).
 * @note The camera-Y divide needs its own `cam_y` temp and BOTH statements need
 *       the do/while(0) wrapper. The wrappers are not decoration: the loop
 *       notes they leave in the RTL stop the next global load from being
 *       hoisted across the divide's compare, which is what keeps `sx` in v0 and
 *       duplicates the shift into the delay slot. Dropping the second wrapper
 *       costs 17 rows; dropping the `cam_y` temp costs 6.
 * @note The tail is `addPrims`, not two hand-written `setaddr` calls, even
 *       though they expand to the same stores - the macro form gets the two
 *       mask constants into a0/a1 the way the target has them (7 rows).
 * @note Measured NON-factors, all 100% either way: `sy + depth` vs
 *       `depth + sy`, `count`/`mode` statement order, a plain `{ }` block or a
 *       block-local temp in place of either do/while(0), and `addPrims` spelled
 *       out as `setaddr(prev, getaddr(&ot[-1]))`.
 *
 * @see decomp.me (100%) TODO
 */
void field_draw_marker_overlay(u32* cursor, u32* ot)
{
    s16 pos[2];
    FieldScene* scene;
    FieldMarker* marker;
    FieldMarkerDef* def;
    LINE_F4* prim;
    void* prev;
    s32 sx;
    s32 sy;
    s32 cam_y;
    s32 base_y;
    s32 depth;
    s32 value;
    s32 digits;
    u32* ot_entry;

    prim = (LINE_F4*)*cursor;
    prev = NULL;
    scene = g_field_scene.scene;
    sx = g_field_camera_x / 256;
    do
    {
        cam_y = g_field_camera_y / 256;
    } while (0);
    do
    {
        sy = cam_y - g_field_camera_z / 512;
    } while (0);
    marker = scene->markers;
    if (marker != NULL)
    {
        ot_entry = ot - 1;
        do
        {
            def = marker->def;
            depth = def->depth_bias + 0xE0;
            setLineF4(prim);
            setRGB0(prim, 0xFF, 0, 0);
            base_y = sy + depth;
            prim->x0 = def->x0 + sx;
            prim->y0 = base_y - HALF_TOWARD_ZERO((s16)def->y0);
            prim->x1 = def->x1 + sx;
            prim->y1 = base_y - HALF_TOWARD_ZERO((s16)def->y1);
            prim->x2 = marker->x3 + sx;
            prim->y2 = base_y - HALF_TOWARD_ZERO((s16)marker->y3);
            prim->x3 = marker->x2 + sx;
            prim->y3 = base_y - HALF_TOWARD_ZERO((s16)marker->y2);
            if (prev != NULL)
            {
                setaddr(prev, prim);
            }
            prev = prim;
            prim = prim + 1;
            setLineF2((LINE_F2*)prim);
            setRGB0(prim, 0xFF, 0, 0);
            prim->x0 = def->x0 + sx;
            prim->y0 = base_y - HALF_TOWARD_ZERO((s16)def->y0);
            prim->x1 = marker->x2 + sx;
            prim->y1 = base_y - HALF_TOWARD_ZERO((s16)marker->y2);
            setaddr(prev, prim);
            prev = prim;
            prim = (LINE_F4*)((LINE_F2*)prim + 1);
            pos[0] = def->x0 + sx;
            pos[1] = base_y - HALF_TOWARD_ZERO((s16)def->y0);
            value = def->label;
            digits = 2;
            if (def->label < 0xA)
            {
                digits = 1;
            }
            prim = (LINE_F4*)func_800AD208(ot_entry, prim, value, digits, pos);
            marker = marker->next;
        } while (marker != NULL);
    }
    if (prev != NULL)
    {
        addPrims(&ot[-1], (void*)*cursor, prev);
        *cursor = (u32)prim;
    }
}

/**
 * @brief Screen-space placement of the grid being drawn.
 *
 * field_emit_sprite_grid only needs the origin; field_emit_rotated_sprite_grid also reads unk8/unkC/unk10
 * to derive the rotation centre for its non-default placement modes.
 */
typedef struct
{
    s32 x;     /* 0x00 */
    s32 y;     /* 0x04 */
    s32 unk8;  /* 0x08 */
    s32 unkC;  /* 0x0C */
    s32 unk10; /* 0x10 */
} FieldViewport;

/**
 * @brief GPU primitive as field_emit_sprite_grid writes it: four raw words.
 *
 * Layout-compatible with SPRT_16 (tag / rgb+code / x0+y0 / u0+v0+clut) and,
 * for the 8-byte form, with DR_TPAGE. It is declared as plain words rather
 * than reusing those Psy-Q types because every field is written as one whole
 * 32-bit store; going through setaddr/setlen or the byte members turns each
 * tag write into a read-modify-write and costs the match.
 */
typedef struct
{
    u32 tag;  /* 0x00 */
    u32 code; /* 0x04 */
    u32 xy;   /* 0x08 packed (y << 16) | (x & 0xFFFF) */
    u32 uv;   /* 0x0C uv pair plus CLUT id */
} FieldPrim;

/**
 * @brief One entry of FieldPart::records, consumed per set bit plane bit.
 *
 * The stride is 0xC bytes, less 4 when the part carries a global code word and
 * another 4 when it carries a global texture page, so unk4/unk8 are only
 * present in the longer forms.
 */
typedef struct
{
    /** 0x00 uv pair plus CLUT id; -1 means the cell emits nothing. */
    s32 uv_clut;
    /** 0x04 rgb/code word used when the part has no global code word. */
    s32 rgb_code;
    /** 0x08 texture-page word tested against the running page code. */
    s32 tpage;
} FieldCellRec;

/**
 * @brief Colour view of a FieldCellRec, used by the tint pass.
 *
 * Names the two halves of FieldCellRec::rgb_code that field_tint_animation_cel writes on their
 * own: the rgb/code word's low halfword and its blue byte.
 */
/**
 * @brief Emit GPU primitives for one bit-plane driven 16x16 sprite grid.
 *
 * Walks @p part 's bit plane row-major. Each set bit consumes one record from
 * the part's record stream and emits a 16-byte len-3 primitive at the current
 * grid cell, preceded by an 8-byte len-1 texture-page primitive whenever the
 * page code changes. Emitted primitives accumulate into a local chain that is
 * spliced into @p ot_base 's ordering-table head for the current CLUT with
 * addPrims, both whenever the interpolated CLUT changes and once at the end.
 * Rows and columns outside the 320x224 viewport are skipped by consuming their
 * bits without emitting.
 *
 * Sibling of field_emit_rotated_sprite_grid; both are reached from the field_draw_part dispatch
 * on FieldPart::kind.
 *
 * @param part Field part supplying the grid, the bit plane, the record stream
 *             and the four corner CLUT ids.
 * @param cursor_ptr In/out primitive-buffer cursor; advanced past everything
 *                   emitted.
 * @param origin Screen-space origin of the grid, in pixels.
 * @param ot_base Base of the 8-byte-per-entry ordering-table head array,
 *                indexed by CLUT id.
 *
 * @note `step` is the record stride: 0xC, less 4 when a global code word makes
 *       the per-record copy unnecessary, less another 4 for a global page word.
 * @note The 0xFFFFFF masks and the `-1` loop sentinels are written as literals
 *       on purpose; loop.c hoists them into the loop preheaders, which is where
 *       the target's s2 and s7 come from. Naming them costs the match.
 * @note The two CLUT multiplies must be written out inline. The `col * clut`
 *       accumulators m2c reconstructs are loop.c strength reduction, not source.
 * @note `bit = 0;` must sit below the tpage test so that it shares a CSE block
 *       with the two NULL inits; that is what makes them copy from `bit`'s
 *       register instead of materialising zero again.
 * @note Both loop counters are seeded in two statements (`row = height;
 *       row = row - 1;` and the same for `col`/`width`) rather than one
 *       `height - 1`. The extra ref crosses the floor_log2 step in global.c's
 *       priority formula, which is what puts `row` in t9 and `col` in t0
 *       ([ALLOC-19]). Collapsing the row pair costs -29 exact rows; collapsing
 *       the col pair costs -21.
 * @note `idx = width; idx -= col;` must stay split for the same reason, and
 *       only pays off while `width` is a full-width `s32`: `u8 width` with the
 *       split is -52 exact, `s32 width` without it is -254. The two are one
 *       change, not two. SImode `width` needs no separate zero_extend, which
 *       shortens `clut`'s live range by exactly the insn that made it conflict
 *       with a1; the split then restores `col`'s ref count so it still outranks
 *       `clut` and keeps t0.
 * @note `width` may be `s32` or `u32` (both 100%); `s32` matches `height`.
 * @note `prim->code = last_code;` is duplicated into both arms, and the record
 *       loads are written as `last_code = (prim->code = ...)`, for the same
 *       allocation-priority reason.
 *
 * @see decomp.me (100%) TODO
 */
void field_emit_sprite_grid(FieldPart* part, s32** cursor_ptr, FieldViewport* origin, s32 ot_base)
{
    s32 uv_word;
    s32 tpage_word;
    s32 code_word;
    s32 height;
    s32 interp;
    u32 clut_right;
    s32 clut_cur;
    s32 clut_left;
    s32 last_code;
    s32 row;
    s32 col;
    s32 idx;
    s32 x;
    s32 y;
    s32 step;
    s32 bits;
    s32 bit;
    s32 count;
    u32 clut;
    u32 clut_b;
    s16 val_a;
    s16 val_b;
    FieldPrim* prim;
    u8* cursor;
    u8* chain;
    u8* recp;
    s32* bitp;
    s32 width;
    FieldPartDef* info;

    last_code = 0;
    bits = 0;
    clut_cur = part->clut_tl;
    clut_left = 0;
    clut_right = 0;
    if ((clut_cur == part->clut_tr) && (clut_cur == part->clut_bl) && (clut_cur == part->clut_br))
    {
        interp = 0;
    }
    else
    {
        clut_cur = 0xFFFF;
        interp = 1;
    }
    code_word = part->code_word;
    tpage_word = part->tpage_word;
    step = 0xC;
    if (code_word != 0)
    {
        step = 8;
    }
    if (tpage_word != 0)
    {
        step -= 4;
    }
    bit = 0;
    prim = NULL;
    bitp = part->bits;
    recp = part->records;
    info = part->def;
    cursor = (u8*)*cursor_ptr;
    y = origin->y;
    height = info->u.b.rows;
    row = height;
    row = row - 1;
    width = info->u.b.cols;
    chain = NULL;
    while (row != -1)
    {
        if (y >= 0xE0)
        {
            break;
        }
        if (y < -0xF)
        {
            count = 0;
            do
            {
                for (col = width - 1; col != -1; col--)
                {
                    if (bit == 0)
                    {
                        bits = *bitp++;
                        bit = 1;
                    }
                    if ((bits & bit) != 0)
                    {
                        count++;
                    }
                    bit <<= 1;
                }
                y += 0x10;
            } while ((y < -0xF) && (--row != -1));
            recp += step * count;
            if (row <= 0)
            {
                break;
            }
            row--;
        }
        if (interp != 0)
        {
            val_a = part->clut_tl;
            val_b = part->clut_bl;
            if (val_a != val_b)
            {
                clut_left = ((val_a * (row + 1)) + (val_b * ((height - row) - 1))) / height;
            }
            else
            {
                clut_left = val_a;
            }
            val_a = part->clut_tr;
            val_b = part->clut_br;
            if (val_a != val_b)
            {
                clut_right = ((val_a * (row + 1)) + (val_b * ((height - row) - 1))) / height;
            }
            else
            {
                clut_right = val_a;
            }
        }
        x = origin->x;
        col = width;
        col = col - 1;
        while (col != -1)
        {
            if (x >= 0x140)
            {
                count = 0;
                do
                {
                    if (bit == 0)
                    {
                        bits = *bitp++;
                        bit = 1;
                    }
                    if ((bits & bit) != 0)
                    {
                        count++;
                    }
                    col--;
                    bit <<= 1;
                } while (col != -1);
                recp += step * count;
                break;
            }
            if (x < -0xF)
            {
                count = 0;
                do
                {
                    if (bit == 0)
                    {
                        bits = *bitp++;
                        bit = 1;
                    }
                    if ((bits & bit) != 0)
                    {
                        count++;
                    }
                    x += 0x10;
                    bit <<= 1;
                } while ((x < -0xF) && (--col != -1));
                recp += step * count;
                if (col <= 0)
                {
                    break;
                }
                col--;
            }
            if (bit == 0)
            {
                bits = *bitp++;
                bit = 1;
            }
            if ((bits & bit) != 0)
            {
                uv_word = ((FieldCellRec*)recp)->uv_clut;
                if (uv_word != -1)
                {
                    if (interp != 0)
                    {
                        idx = width;
                        idx -= col;
                        if (clut_left != clut_right)
                        {
                            clut = ((clut_left * (col + 1)) + (clut_right * (idx - 1))) / width;
                            clut_b = ((clut_left * col) + (clut_right * idx)) / width;
                            if (clut < clut_b)
                            {
                                clut = clut_b;
                            }
                        }
                        else
                        {
                            clut = clut_left;
                        }
                        if (clut != clut_cur)
                        {
                            if (chain != NULL)
                            {
                                addPrims((FieldPrim*)((clut_cur * 8) + ot_base), chain, prim);
                                chain = NULL;
                            }
                            clut_cur = clut;
                        }
                    }
                    if (tpage_word != 0)
                    {
                        prim = (FieldPrim*)cursor;
                        if (chain == NULL)
                        {
                            chain = cursor;
                            cursor += 8;
                            prim->tag = ((u32)cursor & 0xFFFFFF) | 0x01000000;
                            prim->code = tpage_word;
                            prim = (FieldPrim*)cursor;
                        }
                    }
                    else
                    {
                        prim = (FieldPrim*)cursor;
                        if (chain == NULL)
                        {
                            chain = cursor;
                            cursor += 8;
                            prim->tag = ((u32)cursor & 0xFFFFFF) | 0x01000000;
                            if (code_word != 0)
                            {
                                last_code = (prim->code = ((FieldCellRec*)recp)->rgb_code);
                            }
                            else
                            {
                                last_code = (prim->code = ((FieldCellRec*)recp)->tpage);
                            }
                        }
                        else if (last_code != ((FieldCellRec*)recp)->tpage)
                        {
                            cursor += 8;
                            prim->tag = ((u32)cursor & 0xFFFFFF) | 0x01000000;
                            if (code_word != 0)
                            {
                                last_code = (prim->code = ((FieldCellRec*)recp)->rgb_code);
                            }
                            else
                            {
                                last_code = (prim->code = ((FieldCellRec*)recp)->tpage);
                            }
                        }
                        prim = (FieldPrim*)cursor;
                    }
                    cursor += 0x10;
                    prim->tag = ((u32)cursor & 0xFFFFFF) | 0x03000000;
                    if (code_word != 0)
                    {
                        prim->code = code_word;
                    }
                    else
                    {
                        prim->code = ((FieldCellRec*)recp)->rgb_code;
                    }
                    prim->xy = (x & 0xFFFF) | (y << 16);
                    prim->uv = uv_word;
                }
                recp += step;
            }
            bit <<= 1;
            x += 0x10;
            col--;
        }
        row--;
        y += 0x10;
    }
    if (chain != NULL)
    {
        addPrims((FieldPrim*)((clut_cur * 8) + ot_base), chain, prim);
    }
    *cursor_ptr = (s32*)cursor;
}

/**
 * @brief POLY_FT4 as field_emit_rotated_sprite_grid writes it: ten raw words.
 *
 * Layout-compatible with Psy-Q's POLY_FT4 (tag / rgb+code / four x,y pairs each
 * followed by its u,v pair). It is declared as plain words rather than reusing
 * POLY_FT4 because every field is written as one whole 32-bit store: the vertex
 * slots take a packed (x,y) pair straight out of the point buffer, and going
 * through the byte members or setXY0 would turn each into a read-modify-write.
 */
typedef struct
{
    u32 tag;  /* 0x00 */
    u32 code; /* 0x04 */
    u32 xy0;  /* 0x08 */
    u32 uv0;  /* 0x0C uv pair plus CLUT id, straight from the record */
    u32 xy1;  /* 0x10 */
    u32 uv1;  /* 0x14 uv pair plus texture page */
    u32 xy2;  /* 0x18 */
    u32 uv2;  /* 0x1C */
    u32 xy3;  /* 0x20 */
    u32 uv3;  /* 0x24 */
} FieldPolyPrim;

/**
 * @brief One column's rotated unit step, cached in the scratchpad at 0x1F800000.
 *
 * There are width + 1 of these, one per column edge. Each holds the column
 * offset already multiplied by the grid's sine and cosine, so the per-row pass
 * only has to add the row's contribution and shift.
 */
typedef struct
{
    s32 sin_term; /* 0x00 column offset * sin */
    s32 cos_term; /* 0x04 column offset * cos */
} FieldColStep;

/**
 * @brief A screen-space point in one of the two scratchpad row buffers.
 *
 * The pair is compared component-wise for the viewport reject but copied into
 * the primitive as a single word, so the two views have to share storage.
 */
typedef union
{
    /** Packed (y << 16) | (x & 0xFFFF), as stored into a POLY_FT4 vertex. */
    s32 word;
    struct
    {
        s16 x;
        s16 y;
    } p;
} FieldPoint;

/**
 * @brief Emit rotated, scaled POLY_FT4 primitives for one bit-plane sprite grid.
 *
 * Rotated sibling of field_emit_sprite_grid, reached from the same field_draw_part
 * dispatch on FieldPart::kind. Walks @p part 's bit plane row-major and emits
 * one 40-byte POLY_FT4 per set bit, taking the quad's four corners from two
 * ping-pong row buffers of pre-rotated points.
 *
 * The rotation is precomputed in the PSX scratchpad: 0x1F800000 holds width + 1
 * FieldColStep entries (one per column edge), and 0x1F800200 / 0x1F800300 hold
 * width + 1 points each for the previous and current row. Each row advances the
 * vertical offset by 16, rebuilds the current row's points, then walks the
 * columns emitting a quad per set bit. Cells whose four corners all fall off one
 * side of the 320x224 viewport are skipped. Primitives accumulate into a local
 * chain spliced into @p ot_base 's ordering-table head for the current CLUT with
 * addPrims, both when the interpolated CLUT changes and once at the end.
 *
 * @param part Field part: def gives the grid size and the placement mode, bits
 *             the bit plane, records the record stream, row_angle/column_angle/rotation_angle the rotation
 *             angles, scale_x/scale_y the scales, unk44..4A the four corner CLUT ids.
 * @param cursor_ptr In/out primitive-buffer cursor; advanced past everything
 *                   emitted.
 * @param origin Screen-space placement; the mode selects which of its words
 *               form the rotation centre.
 * @param ot_base Base of the 8-byte-per-entry ordering-table head array,
 *                indexed by CLUT id.
 *
 * @see decomp.me (100%) TODO
 */
void field_emit_rotated_sprite_grid(FieldPart *part, s32 **cursor_ptr, FieldViewport *origin, s32 ot_base)
{
    u8 *recp;
    s32 *bitp;
    s32 tpage_word;
    s32 code_word;
    s32 bits;
    s32 bit;
    s32 height;
    s32 interp;
    s32 step;
    s32 sin_c;
    s32 cos_c;
    s32 flip;
    s32 clut_cur;
    s32 clut_left;
    u32 clut_right;
    s32 clut_init;
    s32 row;
    s32 col;
    s32 idx;
    s32 width;
    s32 x_off;
    s32 y_off;
    s32 cx;
    s32 cy;
    s32 cos_a;
    s32 cos_b;
    s32 scaled;
    s32 dx;
    s32 dy;
    s32 visible;
    s32 uv_word;
    u32 clut;
    u32 clut_b;
    FieldColStep *steps;
    FieldPoint *pt;
    FieldPoint *prev_row;
    FieldPoint *this_row;
    FieldPolyPrim *prim;
    u8 *cursor;
    u8 *chain;

    bits = 0;
    clut_left = 0;
    clut_init = part->clut_tl;
    clut_right = 0;
    if ((clut_init == part->clut_tr) && (clut_init == part->clut_bl) && (clut_init == part->clut_br))
    {
        clut_cur = clut_init;
        interp = 0;
    }
    else
    {
        clut_cur = 0xFFFF;
        interp = 1;
    }
    step = 0xC;
    code_word = part->code_word;
    tpage_word = part->tpage_word;
    if (code_word != 0)
    {
        step = 8;
    }
    if (tpage_word != 0)
    {
        step -= 4;
    }
    bit = 0;
    bitp = part->bits;
    recp = part->records;
    cursor = (u8 *) *cursor_ptr;
    prim = NULL;
    width = part->def->u.b.cols;
    height = part->def->u.b.rows;
    cos_a = rcos(part->row_angle);
    cos_b = rcos(part->column_angle);
    sin_c = rsin(part->rotation_angle);
    chain = (u8 *) prim;
    cos_c = rcos(part->rotation_angle);
    switch ((part->def->u.word >> 12) & 0xF)
    {
    case 1:
    case 2:
        cy = origin->unk10;
        cx = origin->unkC + (origin->unk8 / 2);
        x_off = origin->x - cx;
        y_off = origin->y - cy;
        break;
    case 3:
        cx = origin->unkC;
        cy = origin->unk10;
        x_off = origin->x - cx;
        y_off = origin->y - cy;
        break;
    case 4:
        cx = origin->unk8 + origin->unkC;
        cy = origin->unk10;
        x_off = origin->x - cx;
        y_off = origin->y - cy;
        break;
    case 5:
    default:
        x_off = -width * 8;
        y_off = -height * 8;
        cx = origin->x + (width * 8);
        cy = origin->y + (height * 8);
        break;
    }
    scaled = SHIFT_TOWARD_ZERO(SHIFT_TOWARD_ZERO(y_off * part->scale_y, 8) * cos_a, 12);
    steps = (FieldColStep *) 0x1F800000;
    for (col = width; col != -1; col--)
    {
        scaled = SHIFT_TOWARD_ZERO(SHIFT_TOWARD_ZERO(x_off * part->scale_x, 8) * cos_b, 12);
        x_off += 0x10;
        steps->sin_term = scaled * sin_c;
        steps->cos_term = scaled * cos_c;
        steps++;
    }
    steps = (FieldColStep *) 0x1F800000;
    pt = (FieldPoint *) 0x1F800200;
    scaled = SHIFT_TOWARD_ZERO(SHIFT_TOWARD_ZERO(y_off * part->scale_y, 8) * cos_a, 12);
    dx = scaled * sin_c;
    dy = scaled * cos_c;
    for (col = width; col != -1; col--)
    {
        pt->p.x = SHIFT_TOWARD_ZERO(steps->cos_term - dx, 16) + cx;
        pt->p.y = SHIFT_TOWARD_ZERO(steps->sin_term + dy, 16) + cy;
        do {
            steps++;
        } while (0);
        pt++;
    }
    flip = 0;
    row = height;
    row = row - 1;
    if (height != 0)
    {
        do
        {
            if (interp != 0)
            {
                if (part->clut_tl != part->clut_bl)
                {
                    clut_left = ((part->clut_tl * (row + 1)) + (part->clut_bl * ((height - row) - 1))) / height;
                }
                else
                {
                    clut_left = part->clut_tl;
                }
                if (part->clut_tr != part->clut_br)
                {
                    clut_right = ((part->clut_tr * (row + 1)) + (part->clut_br * ((height - row) - 1))) / height;
                }
                else
                {
                    clut_right = part->clut_tr;
                }
            }
            if (flip == 0)
            {
                prev_row = (FieldPoint *) 0x1F800200;
                this_row = (FieldPoint *) 0x1F800300;
                flip = 1;
            }
            else
            {
                prev_row = (FieldPoint *) 0x1F800300;
                this_row = (FieldPoint *) 0x1F800200;
                flip = 0;
            }
            y_off += 0x10;
            steps = (FieldColStep *) 0x1F800000;
            pt = this_row;
            scaled = SHIFT_TOWARD_ZERO(SHIFT_TOWARD_ZERO(y_off * part->scale_y, 8) * cos_a, 12);
            dx = scaled * sin_c;
            dy = scaled * cos_c;
            for (col = width; col != -1; col--)
            {
                pt->p.x = SHIFT_TOWARD_ZERO(steps->cos_term - dx, 16) + cx;
                pt->p.y = SHIFT_TOWARD_ZERO(steps->sin_term + dy, 16) + cy;
                steps++;
                pt++;
            }
            for (col = width - 1; col != -1; col--)
            {
                if (bit == 0)
                {
                    bits = *bitp++;
                    bit = 1;
                }
                if ((bits & bit) != 0)
                {
                    if (!((prev_row[0].p.x >= 0) || (prev_row[1].p.x >= 0) || (this_row[0].p.x >= 0) || (this_row[1].p.x >= 0)))
                    {
                        visible = 0;
                        goto visible_done;
                    }
                    if (!((prev_row[0].p.y >= 0) || (prev_row[1].p.y >= 0) || (this_row[0].p.y >= 0) || (this_row[1].p.y >= 0)))
                    {
                        visible = 0;
                        goto visible_done;
                    }
                    if (!((prev_row[0].p.x < 0x140) || (prev_row[1].p.x < 0x140) || (this_row[0].p.x < 0x140) || (this_row[1].p.x < 0x140)))
                    {
                        visible = 0;
                        goto visible_done;
                    }
                    if ((prev_row[0].p.y < 0xE0) || (prev_row[1].p.y < 0xE0) || (this_row[0].p.y < 0xE0) || (this_row[1].p.y < 0xE0))
                        visible = 1;
                    else
                        visible = 0;
visible_done:;
                    if (visible != 0)
                    {
                        uv_word = ((FieldCellRec *) recp)->uv_clut;
                        if (uv_word != -1)
                        {
                            if (interp != 0)
                            {
                                idx = width - col;
                                if (clut_left != clut_right)
                                {
                                    clut = ((clut_left * (col + 1)) + (clut_right * (idx - 1))) / width;
                                    clut_b = ((clut_left * col) + (clut_right * idx)) / width;
                                    if (clut < clut_b)
                                    {
                                        clut = clut_b;
                                    }
                                }
                                else
                                {
                                    clut = clut_left;
                                }
                                if (clut != clut_cur)
                                {
                                    if (chain != NULL)
                                    {
                                        addPrims((FieldPolyPrim *) ((clut_cur * 8) + ot_base), chain, prim);
                                        chain = NULL;
                                    }
                                    clut_cur = clut;
                                }
                            }
                            if (chain == NULL)
                            {
                                prim = (FieldPolyPrim *) cursor;
                                chain = cursor;
                            }
                            else
                            {
                                prim = (FieldPolyPrim *) cursor;
                            }
                            cursor += 0x28;
                            prim->tag = ((u32) cursor & 0xFFFFFF) | 0x09000000;
                            if (code_word != 0)
                            {
                                prim->code = code_word;
                            }
                            else
                            {
                                prim->code = ((FieldCellRec *) recp)->rgb_code;
                            }
                            if (tpage_word != 0)
                            {
                                prim->uv1 = ((uv_word & 0xFFFF) + 0xF) | tpage_word;
                            }
                            else if (code_word != 0)
                            {
                                prim->uv1 = ((FieldCellRec *) recp)->rgb_code;
                            }
                            else
                            {
                                prim->uv1 = ((FieldCellRec *) recp)->tpage;
                            }
                            prim->uv0 = uv_word;
                            prim->uv2 = (uv_word & 0xFFFF) + 0xF00;
                            prim->uv3 = (uv_word & 0xFFFF) + 0xF0F;
                            prim->xy0 = prev_row[0].word;
                            prim->xy1 = prev_row[1].word;
                            prim->xy2 = this_row[0].word;
                            prim->xy3 = this_row[1].word;
                        }
                    }
                    recp += step;
                }
                prev_row++;
                this_row++;
                bit <<= 1;
            }
            do { row--; } while (0);
        } while (row != -1);
    }
    if (chain != NULL)
    {
        addPrims((FieldPolyPrim *) ((clut_cur * 8) + ot_base), chain, prim);
    }
    *cursor_ptr = (s32 *) cursor;
}

/**
 * @brief Find an already-built part in the scene that this one can share.
 *
 * Scans every part of every object in @p scene for one whose definition key
 * matches @p key and whose owning object is interchangeable with @p obj - either
 * literally the same definition, or one with the same shared-source handle and
 * the same 0x10/0x14 pair. The caller uses the result to reuse an existing
 * part's build instead of doing the work twice.
 *
 * @param scene Scene whose object list is searched.
 * @param obj   Object the candidate must be interchangeable with.
 * @param part  Part being built; excluded from its own search, and skipped
 *              entirely when its definition is marked unshareable (bit 7).
 * @param key   Definition key to match on (FieldPartDef::key).
 * @return The matching FieldPart, or NULL if none qualifies - including when
 *         the only candidate found is @p part itself on @p obj.
 *
 * @note The whole body must be wrapped in `if (!(part->def->u.word & 0x80))`
 *       with ONE trailing `return NULL;`. Spelling it as an early
 *       `if (...) { return NULL; }` guard makes gcc emit a second `jr ra` tail
 *       and merges the in-loop return into it - the exact opposite of the
 *       target, which shares the guard's exit with the final return and keeps
 *       the in-loop one separate (89.70%).
 * @note The success test must be one `||` expression. Splitting it into two
 *       consecutive `if`s costs the shared tail (92.12%).
 * @note `obj->unk14` must be `u16`; `s16` turns the `lhu` pair into `lh`
 *       (98.18%).
 * @note Operand order is required on both equality tests: `key == p->def->key`
 *       (99.85% reversed) and `(obj == o) && (part == p)` (99.70% reversed).
 * @note Measured non-factor: the `want`/`have` temporaries are cosmetic -
 *       repeating `obj->def` and `o->def` inline is also 100%.
 *
 * @see decomp.me (100%) TODO
 */
FieldPart* field_find_shareable_part(FieldScene* scene, FieldObj* obj, FieldPart* part, s32 key)
{
    FieldObj* o;
    FieldPart* p;
    FieldObjDef* want;
    FieldObjDef* have;

    if (!(part->def->u.word & 0x80))
    {
        for (o = scene->objects; o != NULL; o = o->next)
        {
            for (p = o->parts; p != NULL; p = p->next)
            {
                if (key == p->def->key)
                {
                    if ((obj == o) && (part == p))
                    {
                        return NULL;
                    }
                    if (!(p->def->u.word & 0x80))
                    {
                        want = obj->def;
                        have = o->def;
                        if ((want == have) || ((want->shared_source == have->shared_source) && (obj->unk10 == o->unk10) && (obj->unk14 == o->unk14)))
                        {
                            return p;
                        }
                    }
                }
            }
        }
    }
    return NULL;
}

/**
 * @brief Dispatch one field part to the emitter its kind selects.
 *
 * Kind 0 draws an axis-aligned grid, kinds 2 through 5 a rotated/scaled one.
 * Kind 1 and anything from 6 up draw nothing. All four arguments are forwarded
 * verbatim.
 *
 * @param part Field part to draw; its kind byte selects the emitter.
 * @param cursor_ptr Primitive-buffer cursor, forwarded as field_emit_sprite_grid's 2nd param.
 * @param origin Screen-space placement, forwarded as the 3rd param.
 * @param ot_base Ordering-table head array base, forwarded as the 4th param.
 *
 * @note **`case 1: break;` must be written out** even though it does nothing.
 *       gcc balances the switch's comparison tree around the median case node,
 *       so the presence of a do-nothing case 1 is what makes `beq v1, 1` the
 *       ROOT test; without it the tree re-balances around case 0 and the whole
 *       cascade changes (52.59%). The case set is readable straight off the
 *       tree: adding a case 6 also breaks it (47.41%).
 * @note It must be a `switch`. The equivalent
 *       `if (kind == 0) ... else if (kind >= 2 && kind < 6)` chain folds the
 *       range test into a single unsigned compare (47.22%) - see [EXPAND-09].
 * @note `kind` must stay `u8`; `s8` costs the zero-extend shape (97.78%).
 * @note Measured non-factor: adding `default:` alongside `case 1:` is also 100%.
 * @note The parameters keep the loose `(FieldPart *, s32, s32 *, s32)` shape of
 *       the forward declaration above, which field_draw_scene_objects's six call sites are
 *       matched against; the casts at the two calls are free.
 *
 * @see decomp.me (100%) TODO
 */
void field_draw_part(FieldPart* part, s32 cursor_ptr, s32* origin, s32 ot_base)
{
    switch (part->kind)
    {
    case 0:
        field_emit_sprite_grid(part, (s32**)cursor_ptr, (FieldViewport*)origin, ot_base);
        break;
    case 1:
        break;
    case 2:
    case 3:
    case 4:
    case 5:
        field_emit_rotated_sprite_grid(part, (s32**)cursor_ptr, (FieldViewport*)origin, ot_base);
        break;
    }
}

/**
 * @brief Flush the scene's pending VRAM uploads.
 *
 * Walks the scene's upload list, issues each node's LoadImage, then empties the
 * list. Nodes are not freed - the list head is simply cleared.
 *
 * @note The `scene` local is required to match: LoadImage is an ordinary call,
 *       so gcc's alias model treats it as clobbering memory. Writing
 *       `g_field_scene.scene->uploads = NULL;` inline after the loop forces a
 *       reload of the global that the target does not have - it keeps the scene
 *       pointer in s1 across every call (68.17%).
 * @note Measured non-factors, all still 100%: `while` and guarded `do/while`
 *       loop forms, declaring `data` as `void *` and casting at the call, and
 *       replacing the inline `RECT rect;` member with a raw `(RECT *)(p + 4)`
 *       cast. The inline RECT member is kept because it is what makes
 *       `&req->rect` read naturally.
 *
 * @see decomp.me (100%) TODO
 */
void field_flush_vram_uploads(void)
{
    FieldScene* scene;
    FieldImageReq* req;

    scene = g_field_scene.scene;
    for (req = scene->uploads; req != NULL; req = req->next)
    {
        LoadImage(&req->rect, req->data);
    }
    scene->uploads = NULL;
}

/**
 * @brief Does nothing.
 *
 * @note The original is `jr $ra; nop` with no frame and no body. Nothing in the
 *       decompiled tree references it, and no data table holds its address, so
 *       neither its purpose nor its parameter list can be recovered; a `void`
 *       signature is a placeholder that happens to be codegen-correct, since an
 *       empty body ignores its arguments either way. func_800569FC directly
 *       below it is a second, identical stub - the pair is most likely two
 *       unused slots in a per-part hook set whose siblings do real work.
 *
 * @see decomp.me (100%) TODO
 */
void func_800569F4(void)
{
}

/**
 * @brief Does nothing.
 *
 * @note Byte-identical twin of func_800569F4 above; the same caveats apply.
 *       Unreferenced and unrecoverable as to purpose or parameters.
 *
 * @see decomp.me (100%) TODO
 */
void func_800569FC(void)
{
}
