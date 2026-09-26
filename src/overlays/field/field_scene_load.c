/** @file field_scene_load.c
 * @brief Field map loading, render-context setup and the per-frame draw entry.
 */

#include "field_text.h"
#include "movie.h"
#include "cdrom.h"
#include "common.h"
#include "field_calls.h"
#include "field_animation.h"
#include "cd_resources.h"
#include "scene_state.h"
#include "overlay_memory.h"
#include "field_scene_internal.h"
#include "field_actor_tables.h"

/** @brief Maps below this id are read with a blocking queued read instead of a stream. */
#define FIELD_MAP_QUEUED_READ_LIMIT 15
/** @brief Where MOVIE.BIN is streamed to. */
#define FIELD_MOVIE_LOAD_ADDRESS ((u8*)0x80140000)
/** @brief Where a field map resource is read to. */
#define FIELD_MAP_LOAD_ADDRESS ((u8*)0x80180000)
/** @brief First ordering-table entry the scene draws into; the entries in front of it belong to the text windows. */
#define FIELD_SCENE_OT_OFFSET 16
/** @brief Bytes reserved for the text configuration save area. */
#define FIELD_TEXT_SAVED_CONFIG_SIZE 96
/** @brief VRAM position of the map texture area. */
#define FIELD_MAP_TEXTURE_VRAM_X 320
#define FIELD_MAP_TEXTURE_VRAM_Y 256
/** @brief Height in rows of the map texture. */
#define FIELD_MAP_TEXTURE_HEIGHT 256
/** @brief Bytes of one VRAM column (one halfword wide) of the map texture. */
#define FIELD_MAP_TEXTURE_COLUMN_BYTES (FIELD_MAP_TEXTURE_HEIGHT * sizeof(u16))
/** @brief Most VRAM columns one LoadImage call uploads. */
#define FIELD_MAP_TEXTURE_UPLOAD_COLUMNS 16
/** @brief Capacity of the distinct-image list in field_load_map. */
#define FIELD_MAP_MAX_IMAGES 10
/** @brief VRAM row of the object CLUTs. */
#define FIELD_OBJECT_CLUT_VRAM_Y 472
/** @brief Width in VRAM halfwords of one object texture row. */
#define FIELD_OBJECT_IMAGE_WIDTH 256
/** @brief FieldMapColor flag: the object sets the frame buffers' background colour. */
#define FIELD_MAP_BACKGROUND_ENABLED 0x1

/**
 * @brief Map resource header at the start of the load buffer.
 *
 * The scene globals (g_field_scene and its neighbours) live in the same
 * block once the map has been built.
 */
typedef struct
{
    u8 _pad0[0xC];
    /** Size in bytes of the texture image that follows the header. */
    u32 texture_bytes;
    u8 _pad1[4];
    /** Texture image uploaded to VRAM column by column. */
    u8* texture;
} FieldMapHeader;

#define FIELD_MAP_HEADER ((FieldMapHeader*)FIELD_MAP_LOAD_ADDRESS)

/** @brief Start of the main executable's CD system block (CdSystem in cdrom.c). */
typedef struct
{
    u32 status_flags;
    /** Also reachable as the global g_cd_audio_enabled. */
    u8 audio_enabled;
} FieldCdSystem;

#define FIELD_CD_SYSTEM ((FieldCdSystem*)0x801ED800)

extern u8 g_cd_audio_enabled;
/** @brief SCENE_STATE->map_id as a plain global. */
extern u16 g_field_map_id;
/** @brief SCENE_STATE->object_index as a plain global. */
extern u16 g_field_object_index;

static void field_select_object(u16 object_index, FieldRenderHalf* buffers);

/**
 * @brief Initialize a field render context for a scene (no-FMV variant).
 *
 * Clears both ordering tables of the double-buffered render context, selects
 * field object @p object_index, reserves the text configuration save area and
 * seeds the two primitive-buffer cursors.
 *
 * @param buffers Field render context (two frame buffers).
 * @param object_index Field object index, passed to field_select_object.
 * @see decomp.me (100%) https://decomp.me/scratch/m1WWc
 */
void field_init_ctx(FieldRenderHalf* buffers, u16 object_index)
{
    FieldMemState* mem;

    DrawSync(0);
    ClearOTagR(buffers[0].ordering_table, FIELD_ORDERING_TABLE_SIZE);
    ClearOTagR(buffers[1].ordering_table, FIELD_ORDERING_TABLE_SIZE);
    field_select_object(object_index, buffers);
    mem = FIELD_MEM_STATE;
    mem->text_configs = mem->top;
    mem->top = mem->top + FIELD_TEXT_SAVED_CONFIG_SIZE;
    field_size_work_buffer();
    buffers[0].primitive_cursor = (u8*)mem->base;
    buffers[1].primitive_cursor = (u8*)mem->midpoint;
}

/**
 * @brief Reset the scene selection in the shared scene state and the fade state.
 * @see decomp.me (100%) https://decomp.me/scratch/S4vVP
 */
void field_scene_reset(void)
{
    SceneState* state = SCENE_STATE;

    state->map_id = 0;
    state->object_index = 0;
    state->pixel_lookup_selector = 0;
    g_field_scene_fade_mode = 0;
    field_text_init();
}

/**
 * @brief Build and draw one field frame, servicing streamed video when active.
 *
 * Runs the field draw helpers against the render context and pumps
 * movie_service_video_ops whenever CD audio is playing.
 *
 * @param alternate_half Unused; non-zero when @p buffer is the second frame buffer.
 * @param buffer Frame buffer being drawn.
 * @param update_mode Update mode for the scene objects and text windows (1 while a text session pauses the field).
 * @param force_unscaled Non-zero draws the scene objects with the unscaled camera offsets (update mode 2).
 * @see decomp.me (100%) https://decomp.me/scratch/lg9gw
 */
void field_draw_frame(s32 alternate_half, FieldRenderHalf* buffer, s32 update_mode, s32 force_unscaled)
{
    FieldCdSystem* cd_system;

    if (force_unscaled != 0)
    {
        field_draw_scene_objects(&buffer->primitive_cursor, &buffer->ordering_table[FIELD_SCENE_OT_OFFSET], 2);
    }
    else
    {
        field_draw_scene_objects(&buffer->primitive_cursor, &buffer->ordering_table[FIELD_SCENE_OT_OFFSET], update_mode);
    }
    cd_system = FIELD_CD_SYSTEM;
    field_update_scene_fade();
    if (g_cd_audio_enabled != 0)
    {
        movie_service_video_ops();
    }
    field_text_update(&buffer->primitive_cursor, (FieldOrderingTags*)buffer->ordering_table, update_mode);
    if (cd_system->audio_enabled != 0)
    {
        movie_service_video_ops();
    }
}

/**
 * @brief Zero the four per-node accumulators across the scene's node list.
 *
 * When both arguments are zero, also advances the scene animations.
 *
 * @param update_mode Field update mode; non-zero skips the animation update.
 * @param force_unscaled Non-zero skips the animation update.
 * @see decomp.me (100%) https://decomp.me/scratch/KyLZb
 */
void field_clear_node_accumulators(s32 update_mode, s32 force_unscaled)
{
    FieldNode* node;

    for (node = g_field_scene.scene->nodes; node != NULL; node = node->next)
    {
        node->unk24 = 0;
        node->delta_x = 0;
        node->delta_y = 0;
        node->unk30 = 0;
    }
    if ((update_mode == 0) && (force_unscaled == 0))
    {
        field_update_scene_animations();
    }
}

/**
 * @brief Initialize a field scene and its FMV using a caller-supplied context.
 *
 * Streams MOVIE.BIN and starts it, loads the map from the shared scene state,
 * then initializes render context @p buffers for the selected object.
 *
 * @param unused Unused first parameter.
 * @param buffers Field render context to initialize.
 * @see decomp.me (100%) https://decomp.me/scratch/EXpXm
 */
void field_init_with_fmv(void* unused, FieldRenderHalf* buffers)
{
    u16 object_index;
    u8** front_cursor;
    SceneState* state = SCENE_STATE;

    DrawSync(0);
    cdrom_stream(CD_RES_MOVIE_BIN, FIELD_MOVIE_LOAD_ADDRESS);
    movie_play(0);
    field_load_map(state->map_id);
    object_index = state->object_index;
    DrawSync(0);
    ClearOTagR(buffers[0].ordering_table, FIELD_ORDERING_TABLE_SIZE);
    ClearOTagR(buffers[1].ordering_table, FIELD_ORDERING_TABLE_SIZE);
    field_select_object(object_index, buffers);
    g_field_text_saved_configs = (FieldTextConfig*)g_field_mem_top;
    g_field_mem_top += FIELD_TEXT_SAVED_CONFIG_SIZE;
    field_size_work_buffer();
    /* Through a plain pointer: a member store would let the midpoint load move above it. */
    front_cursor = &buffers[0].primitive_cursor;
    *front_cursor = (u8*)g_field_mem_base;
    buffers[1].primitive_cursor = (u8*)g_field_mem_midpoint;
    field_text_reset_windows();
}

/**
 * @brief Initialize a field scene and its FMV in the field render buffers.
 *
 * Same flow as field_init_with_fmv, but obtains the render context from
 * get_field_render_buffers instead of receiving it as a parameter.
 * @see decomp.me (100%) https://decomp.me/scratch/KMYoZ
 */
void field_init_with_fmv_alloc(void)
{
    u16 object_index;
    u8** front_cursor;
    FieldRenderHalf* buffers;

    buffers = get_field_render_buffers();
    DrawSync(0);
    cdrom_stream(CD_RES_MOVIE_BIN, FIELD_MOVIE_LOAD_ADDRESS);
    movie_play(0);
    field_load_map(g_field_map_id);
    object_index = g_field_object_index;
    DrawSync(0);
    ClearOTagR(buffers[0].ordering_table, FIELD_ORDERING_TABLE_SIZE);
    ClearOTagR(buffers[1].ordering_table, FIELD_ORDERING_TABLE_SIZE);
    field_select_object(object_index, buffers);
    g_field_text_saved_configs = (FieldTextConfig*)g_field_mem_top;
    g_field_mem_top += FIELD_TEXT_SAVED_CONFIG_SIZE;
    field_size_work_buffer();
    /* Through a plain pointer: a member store would let the midpoint load move above it. */
    front_cursor = &buffers[0].primitive_cursor;
    *front_cursor = (u8*)g_field_mem_base;
    buffers[1].primitive_cursor = (u8*)g_field_mem_midpoint;
    field_text_reset_windows();
}

/**
 * @brief Load a field map's graphics and register its objects.
 *
 * Reads map resource CD_RES_FIELD_MAP_BASE + @p map_id into the map load
 * buffer, uploads its texture to VRAM, clears every object's built flag, and,
 * when a pixel lookup is selected, runs field_apply_pixel_lookup once per
 * distinct object image.
 *
 * @param map_id Map id; ids below 15 use a blocking queued read, others stream.
 * @see decomp.me (100%) https://decomp.me/scratch/Pvb0P
 */
void field_load_map(u16 map_id)
{
    RECT rect;
    u_long* seen_images[FIELD_MAP_MAX_IMAGES];
    s32 seen_count;
    s32 remaining;
    u_long** seen;
    s32 columns;
    u8* texture;
    FieldMapObject* object;
    u_long* image;
    FieldMapObject** objects;

    DrawSync(0);
    if (map_id < FIELD_MAP_QUEUED_READ_LIMIT)
    {
        cdrom_queue_read((u16)(map_id + CD_RES_FIELD_MAP_BASE), FIELD_MAP_LOAD_ADDRESS);
        cdrom_wait_queue_empty();
    }
    else
    {
        cdrom_stream((u16)(map_id + CD_RES_FIELD_MAP_BASE), FIELD_MAP_LOAD_ADDRESS);
    }
    texture = FIELD_MAP_HEADER->texture;
    rect.x = FIELD_MAP_TEXTURE_VRAM_X;
    rect.y = FIELD_MAP_TEXTURE_VRAM_Y;
    columns = FIELD_MAP_HEADER->texture_bytes / FIELD_MAP_TEXTURE_COLUMN_BYTES;
    rect.h = FIELD_MAP_TEXTURE_HEIGHT;
    while (columns != 0)
    {
        rect.w = (columns <= FIELD_MAP_TEXTURE_UPLOAD_COLUMNS) ? columns : FIELD_MAP_TEXTURE_UPLOAD_COLUMNS;
        LoadImage(&rect, (u_long*)texture);
        texture += FIELD_MAP_TEXTURE_UPLOAD_COLUMNS * FIELD_MAP_TEXTURE_COLUMN_BYTES;
        columns -= rect.w;
        rect.x += FIELD_MAP_TEXTURE_UPLOAD_COLUMNS;
    }

    DrawSync(0);
    for (objects = g_field_objects; *objects != NULL; objects++)
    {
        (*objects)->built = 0;
    }
    if (g_field_pixel_lookup_selector != 0)
    {
        seen_count = 0;
        for (objects = g_field_objects; *objects != NULL; objects++)
        {
            object = *objects;
            image = object->image;
            seen = seen_images;
            for (remaining = seen_count; remaining != 0; remaining--)
            {
                if (*seen == image)
                {
                    break;
                }
                seen++;
            }
            if (remaining == 0)
            {
                seen_count++;
                *seen = image;
                field_apply_pixel_lookup((u16*)image, object->pixel_count, g_field_pixel_lookup_selector - 1, object);
            }
        }
    }
}

/**
 * @brief Select a field map object and apply its image and background colour.
 *
 * Walks g_field_objects to object @p object_index, copies its map size to
 * FIELD_MAP_BOUNDS, sets the background colour of both frame buffers of
 * @p buffers, uploads the object's texture rows and CLUT to VRAM and builds
 * its render records.
 *
 * @param object_index Index into g_field_objects; stops early at the last object.
 * @param buffers Field render context; when NULL the DRAWENV update is skipped.
 * @see decomp.me (100%) https://decomp.me/scratch/vjiqR
 */
static void field_select_object(u16 object_index, FieldRenderHalf* buffers)
{
    FieldMapObject** objects = g_field_objects;
    FieldMapBounds* bounds = FIELD_MAP_BOUNDS;
    u16 remaining = object_index;
    FieldMapObject* object;
    u_long* image;
    u16 rows;
    u8 clut_width;
    RECT rect;

    while (remaining-- != 0)
    {
        if (objects[1] == NULL)
        {
            break;
        }
        objects++;
    }

    object = *objects;
    bounds->width = object->width;
    bounds->depth = object->depth;
    bounds->unk4 = (object->background.word >> 1) & 1;
    if (buffers != NULL)
    {
        buffers[0].draw_env.isbg = buffers[1].draw_env.isbg = 1;
        if (object->background.word & FIELD_MAP_BACKGROUND_ENABLED)
        {
            setRGB0(&buffers[0].draw_env, object->background.b.r, object->background.b.g, object->background.b.b);
            setRGB0(&buffers[1].draw_env, object->background.b.r, object->background.b.g, object->background.b.b);
        }
        else
        {
            setRGB0(&buffers[0].draw_env, 0, 0, 0);
            setRGB0(&buffers[1].draw_env, 0, 0, 0);
        }
    }
    image = object->image;
    rect.x = 0;
    rect.y = FIELD_OBJECT_CLUT_VRAM_Y;
    rows = object->image_size >> 8;
    if (rows != 0)
    {
        s32 row_count = rows;

        rect.w = FIELD_OBJECT_IMAGE_WIDTH;
        rect.h = rows;
        LoadImage(&rect, image);
        image += row_count * (FIELD_OBJECT_IMAGE_WIDTH / 2);
        rect.y += rows;
    }
    clut_width = object->image_size;
    if (clut_width != 0)
    {
        rect.w = clut_width;
        rect.h = 1;
        LoadImage(&rect, image);
    }
    field_build_render_records(object, object_index);
    field_collision_rebuild_spans();
}
