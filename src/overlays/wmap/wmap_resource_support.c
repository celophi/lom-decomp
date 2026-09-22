#include "wmap_resource_support.h"
#include "sdk/libgpu.h"
#include "cdrom.h"

/** @brief Unaligned eight-byte rectangle in a texture block. */
typedef struct
{
    u8 bytes[8];
} WmapTextureRect;

/** @brief GPU packet for a flat-shaded textured triangle. */
typedef struct
{
    u32 tag;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    u8 u0, v0;
    u16 clut;
    s16 x1, y1;
    u8 u1, v1;
    u16 tpage;
    s16 x2, y2;
    u8 u2, v2;
    u16 pad;
} WmapTexturedTriangle;

/** @brief World-map ordering table and current primitive allocation cursor. */
typedef struct
{
    u8 unknown_00[0x70];
    u32 ordering_table[(0x33C - 0x70) / 4];
    u8* primitive_cursor;
} WmapRenderContext;

/** @brief World-map resource record with its initialization field at offset 0x26. */
typedef struct
{
    u8 unknown_0[0x26];
    s16 initial_value;
    u8 unknown_28[4];
} WmapInitResource;

/** @brief World-map display record with a leading state field. */
typedef struct
{
    s16 state;
    u8 unknown_2[0x12];
} WmapInitDisplay;

/** @brief Header containing the world-map value read at offset six. */
typedef struct
{
    u8 unknown_0[6];
    u16 value;
} WmapValueHeader;

extern u8 D_800DCF18[];
extern s32 D_801ADAFC;
extern void akao_play_sfx_from_buffer(s32, s32, s32, s32);
extern s32 D_800CB1FC[];
extern RECT D_80051A88;
extern WmapRenderContext* D_801398EC;
extern s32 D_800D921C;
extern WmapInitResource D_800D9268[];
extern WmapInitDisplay D_801AFBD0[];
extern WmapValueHeader* D_800D0454;

/**
 * @brief Empty world-map handler (no operation).
 */
void func_80064F14(void)
{
}

/**
 * @brief Copy whole words from source to destination.
 * @param source Source words.
 * @param destination Destination words.
 * @param byte_count Nonnegative byte count; trailing partial words are ignored.
 */
void func_80064F1C(s32* source, s32* destination, s32 byte_count)
{
    byte_count /= 4;
    while (--byte_count != -1)
    {
        *destination++ = *source++;
    }
}

/**
 * @brief Empty world-map handler (no operation).
 */
void func_80064F5C(void)
{
}

/**
 * @brief Queue a TIM read from CD and upload the decoded image(s) to VRAM.
 * @param resource_index CD resource id to queue.
 * @note Best match ~73% (gcc280_g0); residual is a cross-jump/reg-alloc tie
 *       (target keeps both pointer regs live; the identical branch tails merge).
 */
void func_80064F64(s32 resource_index)
{
    RECT rect;
    u8* data;
    u8* header;

    data = D_800DCF18;
    cdrom_queue_read(resource_index & 0xFFFF, data);
    cdrom_wait_queue_empty();
    header = data;
    data += 8;
    if (header[4] & 8)
    {
        rect = *(RECT*)((u8*)data + 4);
        LoadImage(&rect, (u_long*)(data + 0xC));
        data += *(s32*)data;
        rect = *(RECT*)((u8*)data + 4);
        if (rect.x == -1)
        {
            return;
        }
        goto draw;
    }
    rect = *(RECT*)((u8*)data + 4);
    if (rect.x == -1)
    {
        return;
    }
draw:
    LoadImage(&rect, (u_long*)(data + 0xC));
    DrawSync(0);
    D_801ADAFC = 1;
}

/**
 * @brief Stream a TIM read from CD and upload the decoded image(s) to VRAM.
 * @param resource_index CD resource id to stream.
 * @note Best match ~73% (gcc280_g0); residual is a cross-jump/reg-alloc tie
 *       (target keeps both pointer regs live; the identical branch tails merge).
 */
void func_80065078(s32 resource_index)
{
    RECT rect;
    u8* data;
    u8* header;

    data = D_800DCF18;
    cdrom_stream(resource_index & 0xFFFF, data);
    cdrom_wait_queue_empty();
    header = data;
    data += 8;
    if (header[4] & 8)
    {
        rect = *(RECT*)((u8*)data + 4);
        LoadImage(&rect, (u_long*)(data + 0xC));
        data += *(s32*)data;
        rect = *(RECT*)((u8*)data + 4);
        if (rect.x == -1)
        {
            return;
        }
    }
    else
    {
        rect = *(RECT*)((u8*)data + 4);
        if (rect.x == -1)
        {
            return;
        }
    }
    LoadImage(&rect, (u_long*)(data + 0xC));
    DrawSync(0);
    D_801ADAFC = 1;
}

/**
 * @brief Queue a resource read into the world-map buffer.
 * @param resource_index Resource identifier; only the low 16 bits are used.
 */
void func_8006518C(s32 resource_index)
{
    cdrom_queue_read(resource_index & 0xFFFF, D_800DCF18);
}

/**
 * @brief Upload an optional palette and the enabled pixel data from a TIM resource.
 * @param data Texture resource header.
 */
void func_800651B4(u8* data)
{
    WmapTextureRect rectangle;

    if (data[4] & 8)
    {
        data += 8;
        rectangle = *(WmapTextureRect*)(data + 4);
        LoadImage((RECT*)&rectangle, (u_long*)(data + 12));
        data += *(s32*)data;
        rectangle = *(WmapTextureRect*)(data + 4);
        if (((RECT*)&rectangle)->x != -1)
        {
            LoadImage((RECT*)&rectangle, (u_long*)(data + 12));
            DrawSync(0);
        }
    }
    else
    {
        data += 8;
        rectangle = *(WmapTextureRect*)(data + 4);
        if (((RECT*)&rectangle)->x != -1)
        {
            LoadImage((RECT*)&rectangle, (u_long*)(data + 12));
            DrawSync(0);
        }
    }
}

/**
 * @brief Play a sound selected from the world-map sound table.
 * @param sound_index One-based sound index; invalid indices select sound one.
 * @param volume Volume passed to the audio player.
 */
void func_800652A8(s32 sound_index, s32 volume)
{
    if ((u32)(sound_index - 1) >= 0x41U)
    {
        sound_index = 1;
    }
    akao_play_sfx_from_buffer(D_800CB1FC[sound_index - 1], 0, volume, 0x7F);
}

/** @brief Clear the configured image rectangle to black. */
void func_800652F8(void)
{
    RECT rectangle = D_80051A88;
    ClearImage(&rectangle, 0, 0, 0);
}

/**
 * @brief Link an offscreen textured triangle that selects the texture page.
 * @param texture_page GPU texture-page bits.
 * @param depth Ordering-table index.
 */
void func_8006534C(s32 texture_page, s32 depth)
{
    WmapTexturedTriangle* primitive;
    WmapRenderContext* table_base;

    primitive = (WmapTexturedTriangle*)D_801398EC->primitive_cursor;
    ((u8*)&primitive->tag)[3] = 7;
    primitive->code = 0x24;
    primitive->tpage = texture_page;
    *(s32*)&primitive->x2 = 400;
    *(s32*)&primitive->x1 = 400;
    *(s32*)&primitive->x0 = 400;
    table_base = (WmapRenderContext*)(depth * 4 + (s32)D_801398EC);
    primitive->tag = (primitive->tag & 0xFF000000) | (table_base->ordering_table[0] & 0xFFFFFF);
    table_base->ordering_table[0] = (table_base->ordering_table[0] & 0xFF000000) | ((u32)primitive & 0xFFFFFF);
    if (D_800D921C < 0x7D00)
    {
        D_800D921C += 32;
        D_801398EC->primitive_cursor += 32;
    }
}

/** @brief Initialize the 256 resource and display records. */
void func_800653EC(void)
{
    s32 index;
    for (index = 0; index < 256; index++)
    {
        D_800D9268[index].initial_value = 16;
        D_801AFBD0[index].state = 0;
    }
}

/**
 * @brief Combine the header value shifted right and left by one byte.
 * @param selection Caller selection, unused by this helper.
 * @param state Caller state, unused by this helper.
 * @return The combined value, including the upper bits of the left shift.
 */
s32 func_80065428(s32 selection, s32* state)
{
    s32 value = D_800D0454->value;
    return ((s32)value >> 8) | (value << 8);
}
