#include "common.h"
#include "sdk/libgpu.h"
#include "sdk/libgte.h"
#include "sdk/memory.h"

/** @brief Three fixed-point coordinates at the head of the field actor record. */
typedef struct
{
    s32 x, y, z;
} FieldSelectionPosition;
/** @brief Two halfword state fields cleared when selection begins. */
typedef struct
{
    s16 first, second;
} FieldSelectionState;

/** @brief Streamed chunk header: payload offset, end offset, and reserved word. */
typedef struct { s32 unk0; s32 unk4; s32 unk8; } BufHdr;

/** @brief Ordering-table head and primitive allocation cursor. */
typedef struct
{
    u32 tag;
    u8 pad4[0x40B8 - 4];
    u8 *cursor;
} RenderContext;

/** @brief Textured quad packet with word views for tag and color writes. */
typedef struct
{
    union
    {
        u32 word;
        struct
        {
            u8 link[3];
            u8 length;
        } bytes;
    } tag;
    union
    {
        u32 word;
        struct
        {
            u8 r, g, b, code;
        } bytes;
    } color;
    s16 x0, y0;
    u8 u0, v0;
    s16 clut;
    s16 x1, y1;
    u8 u1, v1;
    s16 tpage;
    s16 x2, y2;
    u8 u2, v2;
    s16 pad2;
    s16 x3, y3;
    u8 u3, v3;
    s16 pad3;
} TexturedQuad;

/** @brief Active flag and palette index in the runtime slot context. */
typedef struct
{
    u8 pad0[0x2B0C];
    u8 unk2B0C;
    u8 pad2B0D[0x2B54 - 0x2B0D];
    s32 unk2B54;
} PadCtxB800A54D0;

/*
 * Shared file-scope externs used with a consistent type across the TU.
 * D_8011F3AC (s32 vs s32[]) and D_8011F388 (s8[] vs u8[]) carry type
 * conflicts across functions and are declared at block scope instead.
 */
extern s32 D_800F22A0;
extern s32 D_800F22A4;
extern s32 D_800F22A8;
extern FieldSelectionPosition D_800FDF58;
extern FieldSelectionState D_801077FC;
extern u8 D_800EDED8[];
extern u8 D_800EE2D8;
extern u8 D_800EE4D8;
extern u8 D_8010A038[];
extern u8 *D_8010D038;
extern u8 D_801148B0[];
extern s16 D_8011F330;
extern s32 D_8011F334;
extern s32 D_8011F338;
extern s32 D_8011F33C;
extern s32 D_8011F340;
extern s32 D_8011F344;
extern s32 D_8011F348;
extern s32 D_8011F34C;
extern s32 D_8011F350;
extern s32 D_8011F354;
extern u8 D_8011F358[];
extern s32 D_8011F378;
extern s32 D_8011F37C;
extern s32 D_8011F380;
extern s32 D_8011F3A8;
extern s32 D_8011F3B0;
extern s32 D_8011F3B4;
extern s32 D_8011F3B8;
extern s32 D_8011F3BC;
extern s32 D_8011F3C0;
extern s32 D_8011F3C4;
extern s32 D_8011F3C8;
extern u8 *g_pad_ctx;
extern s32 g_pad_input;
extern s32 g_frame_counter;

/* External callees (not members of this TU). */
u32 field_load_vram_resource(s32, s16 *, s32);
void func_800AA02C(void);
void func_80086F48(POLY_FT4 *, s32);
void cdrom_queue_read(s32 id, void *dest);
void cdrom_wait_queue_empty(void);

/* Forward declarations for members called before their definition. */
void func_800A4838(void);
void func_800A496C(void);
void func_800A4A0C(void);
void func_800A4D1C(u8 *render_context);

/**
 * @brief Initialize a resource-backed selection display and its rotation state.
 * @param position_mode Zero centers the display; one follows the first actor.
 * @param resource_index Resource offset and saved-selection index.
 * @param excluded_mask Bit mask of resource entries omitted from selection.
 * @param cancel_index Selection used when cancelling, or minus one to disable it.
 * @note The six-halfword load buffer also holds the signed screen coordinates.
 * @note Subtracting the negated offset preserves the original addition operand order.
 * @note GCC 2.7.2 CDK matches all 215 instructions (860 bytes).
 */
void func_800A43E8(s32 position_mode, s32 resource_index, u16 excluded_mask, s32 cancel_index)
{
    extern s32 D_8011F3AC;
    extern s8 D_8011F388[];
    s16 load_params[6];
    s16 screen_x;
    s16 screen_y;
    s32 rotation;
    s32 unused_count;
    s32 unused_actor_x;
    s32 unused_actor_z;
    s32 bit;
    s32 unused_actor_y;
    s32 camera_x;
    s32 camera_y;
    s32 unused_camera_z;
    s32 index;
    u32 resource_info;
    u8 *saved_index;
    u8 selection;

    if (D_8011F3AC == 0)
    {
        func_800AA02C();
        D_8011F37C = cancel_index;
        D_8011F330 = excluded_mask;
        load_params[0] = 0x140;
        load_params[1] = 0;
        load_params[2] = 0;
        load_params[3] = 0x1F2;
        resource_info = field_load_vram_resource(resource_index + 0xBE8, &load_params[0], 1);
        bit = 1;
        index = 0;
        D_8011F348 = (resource_info & 0x1F) * 8;
        D_8011F34C = (resource_info >> 2) & 0xF8;
        D_8011F3C4 = (resource_info >> 0xA) & 0x1F;
        D_8011F3B8 = 0;
        if (D_8011F3C4 != 0)
        {
            do
            {
                if (!(excluded_mask & 0xFFFF & bit))
                {
                    D_8011F388[D_8011F3B8] = index;
                    D_8011F3B8 += 1;
                }
                index += 1;
                bit *= 2;
            } while (index < D_8011F3C4);
        }
        D_8011F350 = 0x100;
        D_8011F354 = (resource_info >> 0xD) & 0xF8;
        D_8011F338 = 0x20;
        saved_index = resource_index + D_8011F358;
        selection = *saved_index;
        if ((s32)selection >= D_8011F3B8)
        {
            *saved_index = 0;
            D_8011F378 = 0;
        }
        else
        {
            D_8011F378 = (s32)selection % (s32)D_8011F3B8;
        }
        rotation = -(0x1000 / (s32)D_8011F3B8) * D_8011F378;
        D_8011F334 = resource_index;
        D_8011F380 = 0;
        D_8011F3C0 = rotation;
        D_8011F33C = rotation;
        switch (position_mode)
        { /* irregular */
        case 0:
            D_8011F340 = 0xA0;
            D_8011F344 = 0x70;
            break;
        case 1:
            camera_x = D_800F22A0 / 256;
            screen_x = D_800FDF58.x / 256 + 160;
            screen_x = camera_x - (-screen_x);
            load_params[4] = screen_x;
            camera_y = D_800F22A4 / 256;
            screen_y = D_800FDF58.y / 256 + 112;
            screen_y = camera_y - (-screen_y);
            screen_y -= D_800FDF58.z / 512;
            screen_y -= D_800F22A8 / 512;
            load_params[5] = screen_y;
            D_8011F340 = load_params[4];
            D_8011F344 = load_params[5];
            break;
        }
        D_8011F3AC = 2;
        D_8011F3BC = 0;
        D_8011F3B4 = 1;
        D_8011F3A8 = 0;
        D_801077FC.second = 0;
        D_801077FC.first = 0;
    }
}

/**
 * @brief Return the resource entry currently selected by the ring, or -1 while busy.
 * @return The selected entry byte when idle (state 0), otherwise -1.
 */
s32 func_800A4744(void)
{
    extern s32 D_8011F3AC[];
    extern u8 D_8011F388[];
    s32 value;

    value = D_8011F3AC[0];
    if (value == 0)
    {
        s32 index = D_8011F378;
        value = D_8011F388[index];
    }
    else
    {
        value = -1;
    }
    return value;
}

/**
 * @brief Look up a byte from the D_8011F388 table using the D_8011F378
 *        index.
 * @return D_8011F388[D_8011F378].
 */
u8 func_800A4778(void)
{
    extern u8 D_8011F388[];
    return D_8011F388[D_8011F378];
}

/**
 * @brief Drive the ring selection state machine for one frame and redraw it.
 * @param arg0 Render context passed through to the draw routine.
 * @return 1 while the ring is active, 0 when it is idle.
 */
s32 func_800A4798(s32 arg0)
{
    extern s32 D_8011F3AC;
    s32 state;

    state = D_8011F3AC;
    if (state == 0)
    {
        return 0;
    }

    switch (state)
    {
        case 1:
            func_800A4A0C();
            break;

        case 2:
            func_800A4838();
            break;

        case 3:
            func_800A496C();
            break;
    }

    func_800A4D1C(arg0);
    return 1;
}

/** @brief Advance interpolation and the three-phase field animation counter. */
void func_800A4838(void)
{
    extern s32 D_8011F3AC;
    if (D_8011F338 != 0)
    {
        D_8011F350 += (D_8011F354 - D_8011F350) / D_8011F338;
        D_8011F338 -= 1;
    }
    else
    {
        D_8011F350 = D_8011F354;
    }

    if (D_8011F3A8 < 8)
    {
        D_8011F3C0 += 0x200;
    }
    else if (D_8011F3A8 < 0x10)
    {
        D_8011F3C0 += 0x100;
    }
    else if (D_8011F3A8 < 0x20)
    {
        D_8011F3C0 += 0x80;
    }
    else
    {
        D_8011F3B4 = 0;
        D_8011F3AC = 1;
        D_8011F3A8 = 0;
        D_8011F3BC = 0x80;
        return;
    }
    D_8011F3BC += 4;
    D_8011F3A8 += 1;
}

/**
 * @brief Advance the field fade-in ramp by one step, or finish it.
 *
 * Marks the ramp active, then eases @c D_8011F350 toward its target by the
 * remaining-steps fraction. Once @c D_8011F3A8 reaches 0x10 the ramp completes:
 * the step counter and phase are reset and the routine returns early; otherwise
 * it decrements @c D_8011F3BC and advances the step counter.
 *
 * @see decomp.me (100%) TODO
 */
void func_800A496C(void)
{
    extern s32 D_8011F3AC;
    D_8011F3B4 = 1;
    if (D_8011F3A8 < 0x10)
    {
        D_8011F350 -= D_8011F350 / (0x10 - D_8011F3A8);
    }
    else
    {
        D_8011F3A8 = 0;
        D_8011F3AC = 0;
        return;
    }
    D_8011F3BC -= 8;
    D_8011F3A8 += 1;
}

/**
 * @brief Advance the FIELD selection rotation and process selection input.
 * @note Active rotation interpolates over ten updates before snapping to its target.
 * @note Input branches remain independent so simultaneous button bits retain order.
 * @note Save the current rotation before decrementing the selected index.
 * @note GCC 2.7.2 CDK matches all 196 instructions (784 bytes), with no stack frame.
 */
void func_800A4A0C(void)
{
    extern s32 D_8011F3AC;
    s32 previous_index;

    if (D_8011F3A8 != 0)
    {
        D_8011F3C0 = D_8011F3B0 + (((D_8011F33C - D_8011F3B0) * D_8011F3A8) / 10);
        if (D_8011F3A8 == 0xA)
        {
            D_8011F3A8 = 0;
            D_8011F3C0 = (s32) (-D_8011F378 << 12) / (s32) D_8011F3B8;
            return;
        }
        D_8011F3A8 += 1;
        return;
    }
    if (g_pad_input & 0x220)
    {
        D_8011F3AC = 3;
        if (D_8011F37C != -1)
        {
            if (D_8011F378 == D_8011F37C)
            {
                D_8011F358[D_8011F334] = 0;
            }
            else
            {
                goto select_current;
            }
        }
        else
        {
select_current:
            D_8011F358[D_8011F334] = (u8) D_8011F378;
        }
    }
    if ((g_pad_input & 0x40) && (D_8011F37C != -1))
    {
        D_8011F378 = D_8011F37C;
        D_8011F3A8 = 1;
        D_8011F3B0 = D_8011F3C0;
        D_8011F33C = (s32) (-D_8011F37C << 12) / (s32) D_8011F3B8;
    }
    if (g_pad_input & 0x9008)
    {
        D_8011F3A8 = 1;
        D_8011F3B0 = D_8011F3C0;
        previous_index = D_8011F378 - 1;
        D_8011F378 = previous_index;
        D_8011F33C = D_8011F3C0 + (0x1000 / (s32) D_8011F3B8);
        if (previous_index < 0)
        {
            D_8011F378 = D_8011F3B8 - 1;
        }
    }
    if (g_pad_input & 0x6004)
    {
        D_8011F3A8 = 1;
        D_8011F3B0 = D_8011F3C0;
        D_8011F33C = D_8011F3C0 - (0x1000 / (s32) D_8011F3B8);
        D_8011F378 = (s32) (D_8011F378 + 1) % (s32) D_8011F3B8;
    }
}

/**
 * @brief Draw the selectable textured quads arranged around the rotating menu ring.
 * @param render_context Rendering context with an ordering table at 0x40 and packet cursor at
 * 0x40B8.
 * @note Ring angle controls position, brightness, size, and ordering depth. The selected
 * item grows by one eighth every eighth frame. Only submitted quads advance the packet cursor.
 */
void func_800A4D1C(u8 *render_context)
{
    extern u8 D_8011F388[];
    s32 top;
    s16 right;
    s32 bottom;
    s32 *bucket;
    POLY_FT4 *prim;
    u32 address_mask;
    u32 tag_mask;
    s32 columns;
    s32 vertical_product;
    s32 u_or_angle;
    s32 final_cosine;
    s32 initial_count;
    s32 depth_cosine;
    s32 depth;
    s32 final_depth;
    s32 v_or_width;
    s32 height;
    s32 index;
    s32 visible_depth;
    s32 depth_numerator;
    s32 right_u;
    s32 bottom_v;
    s32 brightness;
    s32 packet_code;
    u16 left;
    u8 *entry;
    u8 tile;
    u32 *ordering_table;

    initial_count = D_8011F3B8;
    index = 0;
    ordering_table = (u32 *)(render_context + 0x40);
    prim = *(POLY_FT4 **)(render_context + 0x40B8);
    if (initial_count > 0)
    {
        address_mask = 0xFFFFFF;
        entry = D_8011F388;
        do
        {
            do
            {
                setPolyFT4(prim);
                packet_code = D_8011F3B4;
                if (packet_code == 0)
                {
                    packet_code = 0x2C;
                }
                else
                {
                    packet_code = 0x2E;
                }
                prim->code = packet_code;
            } while (0);
            columns = 0x100 / (s32)D_8011F348;
            tile = *entry;
            u_or_angle = ((s32)tile % columns) * D_8011F348;
            v_or_width = ((s32)tile / columns) * D_8011F34C;
            prim->u2 = u_or_angle;
            prim->u0 = u_or_angle;
            right_u = ((u8)D_8011F348 + u_or_angle) - 1;
            prim->u3 = right_u;
            prim->u1 = right_u;
            prim->v1 = v_or_width;
            prim->v0 = v_or_width;
            bottom_v = ((u8)D_8011F34C + v_or_width) - 1;
            prim->v3 = bottom_v;
            prim->v2 = bottom_v;
            u_or_angle = ((s32)(index << 0xC) / (s32)D_8011F3B8) + D_8011F3C0;
            do
            {
                final_cosine = rcos(u_or_angle);
            } while (0);
            brightness = (u8)D_8011F3BC +
                         ((s32)(((s32)D_8011F3BC >> 1) * (final_cosine - 0x1000)) >> 0xD);
            prim->b0 = brightness;
            prim->g0 = brightness;
            prim->r0 = brightness;
            v_or_width =
                D_8011F348 + ((s32)(((s32)D_8011F348 >> 1) * (rcos(u_or_angle) - 0x1000)) >> 0xD);
            height =
                D_8011F34C + ((s32)(((s32)D_8011F34C >> 1) * (rcos(u_or_angle) - 0x1000)) >> 0xD);
            if ((index == D_8011F378) && !(g_frame_counter & 7))
            {
                v_or_width = v_or_width * 9 / 8;
                height = height * 9 / 8;
            }
            left = ((u16)D_8011F340 + ((s32)(D_8011F350 * rsin(u_or_angle)) >> 0xC)) -
                   (v_or_width >> 1);
            prim->x2 = left;
            prim->x0 = left;
            vertical_product = D_8011F350 * rcos(u_or_angle);
            prim->tpage = 0x25;
            do
            {
                top = (u16)D_8011F344;
            } while (0);
            top += vertical_product >> 0xE;
            top -= height >> 1;
            prim->y0 = top;
            do
            {
                bottom = top;
                top++;
                top--;
                do
                {
                    right = (u16)prim->x0;
                } while (0);
                bottom += height;
                prim->y1 = top;
                prim->y3 = bottom;
                prim->y2 = bottom;
            } while (0);
            do
            {
                do
                {
                    do
                    {
                        right += v_or_width;
                        prim->x3 = right;
                        prim->x1 = right;
                    } while (0);
                } while (0);
            } while (0);
            prim->clut = (s16)((*entry & 0x3F) | 0x7C80);
            visible_depth = (rcos(u_or_angle) - 0x1000) / 64;
            if (visible_depth <= 0)
            {
                do
                {
                    depth_cosine = rcos(u_or_angle);
                } while (0);
                depth_numerator = depth_cosine - 0x1000;
                if (depth_numerator < 0)
                {
                    depth_numerator = depth_cosine - 0xFC1;
                }
                {
                    s32 *link_bucket;

                    link_bucket = (s32 *)(ordering_table - (depth_numerator >> 6));
                    tag_mask = 0xFF000000;
                    *(u32 *)prim = (*(u32 *)prim & tag_mask) | (*(u32 *)link_bucket & address_mask);
                }
                depth = (rcos(u_or_angle) - 0x1000) / 64;
                bucket = (s32 *)(ordering_table - depth);
                *bucket = (*bucket & tag_mask) | ((s32)prim & address_mask);
                final_cosine = rcos(u_or_angle);
                final_depth = final_cosine - 0x1000;
                if (final_depth < 0)
                {
                    final_depth = final_cosine - 0xFC1;
                }
                func_80086F48(prim, -(final_depth >> 6) + 0x10);
                prim++;
            }
            entry += 1;
        } while (++index < D_8011F3B8);
    }
    *(POLY_FT4 **)(render_context + 0x40B8) = prim;
}

/**
 * @brief Stream a field data chunk from CD and copy its sections into RAM.
 * @param arg0 Destination bank selector; also scales the D_801148B0 copy offset.
 * @param arg1 CD queue id (low 16 bits) of the chunk to read.
 */
void func_800A5174(s32 arg0, s32 arg1)
{
    s32 *temp_v0;
    s32 temp_v0_2;
    BufHdr *temp_s0;
    u8 *temp_a0;
    s32 count;

    temp_s0 = (BufHdr *)D_8010D038;
    cdrom_queue_read(arg1 & 0xFFFF, temp_s0);
    cdrom_wait_queue_empty();
    temp_v0 = (s32 *)(D_8010D038 + temp_s0->unk0);
    temp_a0 = (u8 *)temp_v0 + 4;
    count = *temp_v0;
    if (arg0 == 2) {
        u8 *dst = D_8010A038;
        dst += 0x320;
        bcopy(temp_a0, dst, count * 8);
    }
    temp_s0 = (BufHdr *)((u8 *)temp_s0 + 4);
    {
        u8 *src = D_8010D038;
        s32 end;
        temp_v0_2 = temp_s0->unk0;
        end = temp_s0->unk4;
        src += temp_v0_2;
        bcopy(src, D_801148B0 + (arg0 << 12), end - temp_v0_2);
    }
}

/**
 * @brief Queue five textured strips using the selected field rendering layout.
 * @param context Render context containing the ordering table and packet cursor.
 * @param layout Zero selects two quads per strip; nonzero selects one quad.
 */
void func_800A5224(RenderContext *context, s32 layout)
{
    u32 *head;
    u32 split_address_mask;
    u32 split_tag_mask;
    u32 full_address_mask;
    u32 full_tag_mask;
    s32 full_color;
    s32 full_length;
    s32 full_height;
    s32 full_v_top;
    s32 full_u_right;
    s32 full_v_bottom;
    s16 split_left_x;
    s16 full_left_x;
    s16 full_right_x;
    s16 split_right_x;
    u8 *packet;
    s32 split_page;
    s32 full_page;
    s32 strip_index;
    s32 packet_code;
    TexturedQuad *full_quad;
    TexturedQuad *split_quad;

    if (D_8011F3C8 != 0)
    {
        packet = context->cursor;
        head = &context->tag;
        if (layout == 0)
        {
            strip_index = 0;
            split_quad = (TexturedQuad *)packet;
            do
            {
                split_left_x = strip_index * 0x10;
                split_page = strip_index & 0xF;
                split_quad->color.word = 0x808080;
                split_quad->tag.bytes.length = 9;
                split_quad->color.bytes.code = 0x2C;
                split_quad->x0 = split_left_x;
                split_quad->y0 = 0;
                split_quad->x1 = ((strip_index * 0x10) + 0x10);
                split_quad->y1 = 0;
                split_quad->x2 = split_left_x;
                split_quad->y2 = 4;
                split_quad->x3 = ((strip_index * 0x10) + 0x10);
                split_quad->y3 = 4;
                split_quad->u0 = 0;
                split_quad->v0 = 0xF0;
                split_quad->u1 = 0x40;
                split_quad->v1 = 0xF0;
                split_quad->u2 = 0;
                split_quad->v2 = 0xFF;
                split_quad->u3 = 0x40;
                split_quad->v3 = 0xFF;
                split_quad->clut = 0;
                split_quad->tpage = (s16) (split_page | 0x120);
                split_quad++;
                split_address_mask = 0xFFFFFF;
                split_tag_mask = 0xFF000000;
                ((TexturedQuad *)packet)->tag.word = (((TexturedQuad *)packet)->tag.word & split_tag_mask) | (*head & split_address_mask);
                *head = (s32) ((*head & split_tag_mask) | ((s32) packet & split_address_mask));
                packet += 0x28;
                split_quad->color.word = 0x808080;
                split_quad->tag.bytes.length = 9;
                split_quad->color.bytes.code = 0x2C;
                setXY4(split_quad, split_left_x, 4, ((strip_index * 0x10) + 0x10), 4, split_left_x, 0x38, ((strip_index * 0x10) + 0x10), 0x38);
                strip_index += 1;
                split_quad->u0 = 0;
                split_quad->v0 = 0;
                split_quad->u1 = 0x40;
                split_quad->v1 = 0;
                split_quad->u2 = 0;
                split_quad->v2 = 0xD0;
                split_quad->u3 = 0x40;
                split_quad->v3 = 0xD0;
                split_quad->clut = 0;
                split_quad->tpage = (s16) (split_page | 0x130);
                split_quad += 2;
                ((TexturedQuad *)packet)->tag.word = (s32) ((((TexturedQuad *)packet)->tag.word & split_tag_mask) | (*head & split_address_mask));
                *head = (s32) ((*head & split_tag_mask) | ((s32) packet & split_address_mask));
                packet += 0x50;
            } while (strip_index < 5);
            context->cursor = packet;
            return;
        }
        strip_index = 0;
        full_color = 0x808080;
        full_length = 9;
        full_height = 0x38;
        full_v_top = 8;
        full_u_right = 0x40;
        full_v_bottom = 0xE8;
        full_address_mask = 0xFFFFFF;
        full_tag_mask = 0xFF000000;
        full_right_x = 0x10;
        full_quad = (TexturedQuad *)packet;
        do
        {
            full_quad->x1 = full_right_x;
            full_quad->x3 = full_right_x;
            full_right_x += 0x10;
            full_left_x = strip_index * 0x10;
            full_page = strip_index & 0xF;
            strip_index += 1;
            full_quad->color.word = full_color;
            full_quad->tag.bytes.length = full_length;
            packet_code = 0x2C;
            full_quad->color.bytes.code = packet_code;
            full_quad->x0 = full_left_x;
            packet_code = 0;
            full_quad->y0 = packet_code;
            full_quad->y1 = 0;
            full_quad->x2 = full_left_x;
            full_quad->y2 = full_height;
            full_quad->y3 = full_height;
            full_quad->u0 = 0;
            full_quad->v0 = full_v_top;
            full_quad->u1 = full_u_right;
            full_quad->v1 = full_v_top;
            full_quad->u2 = 0;
            full_quad->v2 = full_v_bottom;
            full_quad->u3 = full_u_right;
            full_quad->v3 = full_v_bottom;
            full_quad->clut = 0;
            full_quad->tpage = (s16) (full_page | 0x120);
            full_quad += 2;
            ((TexturedQuad *)packet)->tag.word = (((TexturedQuad *)packet)->tag.word & full_tag_mask) | (*head & full_address_mask);
            *head = (s32) ((*head & full_tag_mask) | ((s32) packet & full_address_mask));
            packet += 0x50;
        } while (strip_index < 5);
        context->cursor = packet;
    }
}

/** @brief Upload palettes for the three active runtime slots.
 * @see decomp.me (100%)
 */
void func_800A54D0(void)
{
    RECT rect;
    PadCtxB800A54D0 *p;
    s32 i;
    s32 offset;

    i = 0;
    do
    {
        offset = i * 0x14C;
        p = (PadCtxB800A54D0 *) (g_pad_ctx + offset);
        if (p->unk2B0C != 0)
        {
            if ((u32) p->unk2B54 < 0x10)
            {
                rect.x = 0x100;
                rect.y = i + 0x1F0;
                rect.w = 0x10;
                rect.h = 1;
                LoadImage(&rect, (u_long *)(D_800EDED8 + (p->unk2B54 << 5)));
            }
            else
            {
                rect.x = 0x100;
                rect.y = i + 0x1F0;
                rect.w = 0x10;
                rect.h = 1;
                LoadImage(&rect, (u_long *)((D_800EDED8 + 0x200) + ((p->unk2B54 - 0x10) << 5)));
            }
        }
        i += 1;
    } while (i < 3);
}

/**
 * @brief Copies a 32-byte entry from one of two field-data tables.
 *
 * @param destination Buffer that receives the selected entry.
 * @param index Combined index across the two 16-entry tables.
 */
void func_800A55E4(unsigned char *destination, s32 index)
{
    if (index < 0x10)
    {
        bcopy(&D_800EE2D8 + (index << 5), destination, 0x20);
    }
    else
    {
        bcopy(&D_800EE4D8 + ((index - 0x10) << 5), destination, 0x20);
    }
}
