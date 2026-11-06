#include "common.h"
#include <libgpu.h>

#include "../internal/wmap_actor_motion.h"
#include "../internal/wmap_frame_render.h"
#include "../internal/wmap_main.h"
#include "../internal/wmap_resource_support.h"
#include "../internal/wmap_sprite_render.h"
#include "common/gpu_packet.h"
#include "common/tim.h"
#include "main/audio/akao_cmd.h"
#include "main/cdrom.h"

#define WMAP_COPY_WORD_SHIFT 2
#define WMAP_COPY_END -1

/** @brief Unaligned eight-byte rectangle in a texture block. */
typedef struct
{
    u8 bytes[8];
} WmapTextureRect;

extern u8 g_wmap_load_buffer[];
extern RECT g_wmap_framebuffer_rect;

/**
 * @brief Report invalid data or a full table; empty in the retail build.
 * @note Called for a bad sprite part count, a bad model face, and a full sequence table; some calls pass the model's face attributes.
 */
void wmap_report_error(void)
{
}

/**
 * @brief Copy complete words forward between aligned buffers.
 * @param source Four-byte-aligned source words.
 * @param destination Four-byte-aligned destination words.
 * @param byte_count Nonnegative byte length; partial trailing words are ignored.
 */
void wmap_copy_words(s32* source, s32* destination, s32 byte_count)
{
    s32 copy_limit = byte_count;

    if (byte_count < 0)
    {
        copy_limit = byte_count + BYTES_PER_WORD - 1;
    }

    byte_count = copy_limit >> WMAP_COPY_WORD_SHIFT;

    if (--byte_count != WMAP_COPY_END)
    {
        for (copy_limit = WMAP_COPY_END; byte_count != copy_limit; byte_count--)
        {
            *destination++ = *source++;
        }
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
 */
void wmap_load_tim_file(s32 resource_index)
{
    WmapTextureRect rectangle;
    u8* data;
    u8* header;

    data = g_wmap_load_buffer;
    cdrom_queue_read(resource_index & 0xFFFF, data);
    cdrom_wait_queue_empty();
    data += TIM_HEADER_SIZE;
    header = g_wmap_load_buffer;

    if (header[OFFSETOF(TimPrefix, flags)] & TIM_FLAG_HAS_CLUT)
    {
        header = (u8*)&rectangle;
        rectangle = *(WmapTextureRect*)(data + OFFSETOF(TimBlock, dx));
        LoadImage((RECT*)header, (u_long*)(data + sizeof(TimBlock)));
        data += *(s32*)data;
        rectangle = *(WmapTextureRect*)(data + OFFSETOF(TimBlock, dx));

        if (((RECT*)header)->x != -1)
        {
            LoadImage((RECT*)header, (u_long*)(data + sizeof(TimBlock)));
            DrawSync(0);
            g_wmap_artifact_shadows_enabled = 1;
        }
    }
    else
    {
        rectangle = *(WmapTextureRect*)(data + OFFSETOF(TimBlock, dx));

        if (((RECT*)&rectangle)->x != -1)
        {
            LoadImage((RECT*)&rectangle, (u_long*)(data + sizeof(TimBlock)));
            DrawSync(0);
            g_wmap_artifact_shadows_enabled = 1;
        }
    }
}

/**
 * @brief Stream a TIM read from CD and upload the decoded image(s) to VRAM.
 * @param resource_index CD resource id to stream.
 */
void wmap_stream_tim_file(s32 resource_index)
{
    WmapTextureRect rectangle;
    u8* data;
    u8* header;

    data = g_wmap_load_buffer;
    cdrom_stream(resource_index & 0xFFFF, data);
    cdrom_wait_queue_empty();
    data += TIM_HEADER_SIZE;
    header = g_wmap_load_buffer;

    if (header[OFFSETOF(TimPrefix, flags)] & TIM_FLAG_HAS_CLUT)
    {
        header = (u8*)&rectangle;
        rectangle = *(WmapTextureRect*)(data + OFFSETOF(TimBlock, dx));
        LoadImage((RECT*)header, (u_long*)(data + sizeof(TimBlock)));
        data += *(s32*)data;
        rectangle = *(WmapTextureRect*)(data + OFFSETOF(TimBlock, dx));

        if (((RECT*)header)->x != -1)
        {
            LoadImage((RECT*)header, (u_long*)(data + sizeof(TimBlock)));
            DrawSync(0);
            g_wmap_artifact_shadows_enabled = 1;
        }
    }
    else
    {
        rectangle = *(WmapTextureRect*)(data + OFFSETOF(TimBlock, dx));

        if (((RECT*)&rectangle)->x != -1)
        {
            LoadImage((RECT*)&rectangle, (u_long*)(data + sizeof(TimBlock)));
            DrawSync(0);
            g_wmap_artifact_shadows_enabled = 1;
        }
    }
}

/**
 * @brief Queue a resource read into the world-map buffer.
 * @param resource_index Resource identifier; only the low 16 bits are used.
 */
void wmap_queue_load_buffer_read(s32 resource_index)
{
    cdrom_queue_read(resource_index & 0xFFFF, g_wmap_load_buffer);
}

/**
 * @brief Upload an optional palette and the enabled pixel data from a TIM resource.
 * @param data Texture resource header.
 */
void wmap_upload_tim(u8* data)
{
    WmapTextureRect rectangle;

    if (data[OFFSETOF(TimPrefix, flags)] & TIM_FLAG_HAS_CLUT)
    {
        data += TIM_HEADER_SIZE;
        rectangle = *(WmapTextureRect*)(data + OFFSETOF(TimBlock, dx));
        LoadImage((RECT*)&rectangle, (u_long*)(data + sizeof(TimBlock)));
        data += *(s32*)data;
        rectangle = *(WmapTextureRect*)(data + OFFSETOF(TimBlock, dx));

        if (((RECT*)&rectangle)->x != -1)
        {
            LoadImage((RECT*)&rectangle, (u_long*)(data + sizeof(TimBlock)));
            DrawSync(0);
        }
    }
    else
    {
        data += TIM_HEADER_SIZE;
        rectangle = *(WmapTextureRect*)(data + OFFSETOF(TimBlock, dx));

        if (((RECT*)&rectangle)->x != -1)
        {
            LoadImage((RECT*)&rectangle, (u_long*)(data + sizeof(TimBlock)));
            DrawSync(0);
        }
    }
}

/**
 * @brief Play a sound selected from the world-map sound table.
 * @param sound_index One-based sound index; invalid indices select sound one.
 * @param pan Stereo pan position (128 = center).
 */
void wmap_play_sound(s32 sound_index, s32 pan)
{
    if ((sound_index - 1) >= 0x41U)
    {
        sound_index = 1;
    }

    akao_play_sfx_from_buffer(g_wmap_sfx_buffers[sound_index - 1], 0, pan, 0x7F);
}

/** @brief Clear the configured image rectangle to black. */
void wmap_clear_framebuffers(void)
{
    RECT rectangle = g_wmap_framebuffer_rect;
    ClearImage(&rectangle, 0, 0, 0);
}

/**
 * @brief Link an offscreen textured triangle that selects the texture page.
 * @param texture_page GPU texture-page bits.
 * @param depth Ordering-table index.
 */
void wmap_queue_texture_page(s32 texture_page, s32 depth)
{
    POLY_FT3* primitive;

    primitive = g_wmap_current_frame->packet_cursor;
    setlen(primitive, 7);
    setcode(primitive, GPU_CODE_POLY_FT3);
    primitive->tpage = texture_page;
    SET_XY2_PACKED(primitive, WMAP_MODEL_TPAGE_XY);
    SET_XY1_PACKED(primitive, WMAP_MODEL_TPAGE_XY);
    SET_XY0_PACKED(primitive, WMAP_MODEL_TPAGE_XY);
    addPrim(&g_wmap_current_frame->ordering_table[depth], primitive);

    if (g_wmap_packet_bytes < WMAP_PACKET_LIMIT)
    {
        g_wmap_packet_bytes += sizeof(POLY_FT3);
        g_wmap_current_frame->packet_cursor += sizeof(POLY_FT3);
    }
}

/** @brief Initialize the 256 resource and display records. */
void wmap_init_actor_display_states(void)
{
    s32 index;

    for (index = 0; index < 256; index++)
    {
        g_wmap_sprite_actors[index].shade_step = 16;
        g_wmap_actor_motions[index].state = 0;
    }
}

/**
 * @brief Read the first controller's repeat buttons in hardware button order.
 * @return Byte-swapped buttons, including the upper bits of the left shift.
 */
s32 wmap_get_controller_repeat_buttons(void)
{
    s32 buttons = g_wmap_controller_ports->published_sample.repeat_buttons;
    return (buttons >> 8) | (buttons << 8);
}
