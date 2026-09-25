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

/** @brief First CD resource of the field map archives; the map id is added to it. */
#define FIELD_MAP_RESOURCE_BASE 0xB4
/** @brief Maps below this id are read with a blocking queued read instead of a stream. */
#define FIELD_MAP_QUEUED_READ_LIMIT 15
/** @brief Where MOVIE.BIN is streamed to. */
#define FIELD_MOVIE_LOAD_ADDRESS ((u8*)0x80140000)
/** @brief Where a field map resource is read to. */
#define FIELD_MAP_LOAD_ADDRESS ((u8*)0x80180000)
/** @brief Number of ordering-table entries cleared per frame buffer. */
#define FIELD_RENDER_OT_LENGTH 0x1010
/** @brief Bytes reserved for the text configuration save area. */
#define FIELD_TEXT_SAVED_CONFIG_SIZE 0x60
/** @brief VRAM row of the object CLUTs. */
#define FIELD_OBJECT_CLUT_VRAM_Y 0x1D8
/** @brief Width in halfwords of one map texture column. */
#define FIELD_MAP_TEXTURE_COLUMN_WIDTH 0x10
/** @brief Bytes of one map texture column (16 halfwords by 256 rows). */
#define FIELD_MAP_TEXTURE_COLUMN_BYTES 0x2000
/** @brief Most texture columns one LoadImage call uploads. */
#define FIELD_MAP_TEXTURE_UPLOAD_COLUMNS 0x10

/**
 * @brief One half of the field's double-buffered render state.
 *
 * The ordering table has sixteen extra entries in front of the 0x1000 the
 * scene draws into; ClearOTagR clears them together.
 */
typedef struct
{
    u_long ordering_table[FIELD_RENDER_OT_LENGTH];
    DISPENV disp_env;
    DRAWENV draw_env;
    RECT clear_rect;
    u8* prim_cursor;
    u8 prim_buffer[0x3C08];
} FieldRenderBuffer;

/**
 * @brief Map resource header at the start of the load buffer at 0x80180000.
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

/** @brief Byte offset of @p member within @p type. */
#define FIELD_OFFSETOF(type, member) ((u32) & ((type*)0)->member)

/**
 * @brief Primitive cursor of a FieldRenderBuffer, written through a plain pointer.
 * @note The FMV setup paths store the cursors this way; a member store lets the
 *       scheduler move it past the following load.
 */
#define FIELD_PRIM_CURSOR(buf) (*(u8**)((u8*)(buf) + FIELD_OFFSETOF(FieldRenderBuffer, prim_cursor)))

/** @brief Parameters of the selected map object, kept at 0x801ED400. */
typedef struct
{
    u16 unk0;
    u16 unk2;
    u8 unk4;
} FieldObjectParams;

#define FIELD_OBJECT_PARAMS ((FieldObjectParams*)0x801ED400)
#define FIELD_MEM_STATE ((FieldMemState*)0x801ED000)
#define FIELD_CD_SYSTEM ((u8*)0x801ED800)

extern u8 g_cd_audio_enabled;
extern s32 D_801ED000;
extern s32 D_801ED00C;
extern s32 D_801ED010;
extern s32 D_801ED02C;
extern u16 D_801ED480;
extern u16 D_801ED482;
extern s32 D_801ED490;

void func_80140018(s32 mode);
void field_collision_rebuild_spans(void);
void field_select_object(u16 object_index, FieldRenderBuffer* buffers);
void field_build_render_records(FieldMapObject* object, u16 object_index);
void field_load_map(s32 map_id);

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
void field_init_ctx(FieldRenderBuffer* buffers, u16 object_index)
{
    FieldMemState* mem;
    s32 mode = 0;

    DrawSync(mode);
    ClearOTagR(buffers[0].ordering_table, FIELD_RENDER_OT_LENGTH);
    ClearOTagR(buffers[1].ordering_table, FIELD_RENDER_OT_LENGTH);
    field_select_object(object_index & 0xFFFF, buffers);
    mem = FIELD_MEM_STATE;
    mem->text_configs = mem->top;
    mem->top = mem->top + FIELD_TEXT_SAVED_CONFIG_SIZE;
    field_size_work_buffer();
    buffers[0].prim_cursor = (u8*)mem->base;
    buffers[1].prim_cursor = (u8*)mem->midpoint;
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
    D_801ED02C = 0;
    field_text_init();
}

/**
 * @brief Build and draw one field frame, servicing streamed video when active.
 *
 * Runs the field draw helpers against the render context and pumps
 * movie_service_video_ops whenever CD audio is playing.
 *
 * @param unused Unused first parameter.
 * @param buffer Frame buffer being drawn.
 * @param draw_mode Draw mode for field_draw_scene_objects; forced to 2 when @p unscaled is non-zero.
 * @param unscaled Non-zero draws with the unscaled camera offsets.
 * @see decomp.me (100%) https://decomp.me/scratch/lg9gw
 */
void field_draw_frame(s32 unused, FieldRenderBuffer* buffer, s32 draw_mode, s32 unscaled)
{
    u8* cd_system;

    if (unscaled != 0)
    {
        field_draw_scene_objects(&buffer->prim_cursor, &buffer->ordering_table[0x10], 2);
    }
    else
    {
        field_draw_scene_objects(&buffer->prim_cursor, &buffer->ordering_table[0x10], draw_mode);
    }
    cd_system = FIELD_CD_SYSTEM;
    field_update_scene_fade();
    if (g_cd_audio_enabled != 0)
    {
        movie_service_video_ops();
    }
    field_text_update(&buffer->prim_cursor, (FieldOrderingTags*)buffer, draw_mode);
    if (cd_system[4] != 0)
    {
        movie_service_video_ops();
    }
}

/**
 * @brief Zero the four per-node accumulators across the scene's node list.
 *
 * When both arguments are zero, also advances the scene animations.
 *
 * @param skip_animation_x TODO: meaning unknown; both zero runs field_update_scene_animations.
 * @param skip_animation_y TODO: meaning unknown.
 * @see decomp.me (100%) https://decomp.me/scratch/KyLZb
 */
void field_clear_node_accumulators(s32 skip_animation_x, s32 skip_animation_y)
{
    FieldNode* node;

    for (node = g_field_scene.scene->nodes; node != NULL; node = node->next)
    {
        node->unk24 = 0;
        node->delta_x = 0;
        node->delta_y = 0;
        node->unk30 = 0;
    }
    if ((skip_animation_x == 0) && (skip_animation_y == 0))
    {
        field_update_scene_animations();
    }
}

/**
 * @brief Initialize a field scene and its FMV using a caller-supplied context.
 *
 * Streams MOVIE.BIN into 0x80140000 and starts it, loads the map from the
 * shared scene state, then initializes render context @p buffers for the
 * selected object.
 *
 * @param unused Unused first parameter.
 * @param buffers Field render context to initialize.
 * @see decomp.me (100%) https://decomp.me/scratch/EXpXm
 */
void field_init_with_fmv(void* unused, FieldRenderBuffer* buffers)
{
    u16 object_index;
    SceneState* state = SCENE_STATE;

    DrawSync(0);
    cdrom_stream(CD_RES_MOVIE_BIN, FIELD_MOVIE_LOAD_ADDRESS);
    func_80140018(0);
    field_load_map(state->map_id);
    object_index = state->object_index;
    DrawSync(0);
    ClearOTagR(buffers[0].ordering_table, FIELD_RENDER_OT_LENGTH);
    ClearOTagR(buffers[1].ordering_table, FIELD_RENDER_OT_LENGTH);
    field_select_object(object_index & 0xFFFF, buffers);
    g_field_text_saved_configs = (FieldTextConfig*)D_801ED000;
    D_801ED000 += FIELD_TEXT_SAVED_CONFIG_SIZE;
    field_size_work_buffer();
    FIELD_PRIM_CURSOR(&buffers[0]) = (u8*)D_801ED00C;
    FIELD_PRIM_CURSOR(&buffers[1]) = (u8*)D_801ED010;
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
    FieldRenderBuffer* buffers;

    buffers = get_field_render_buffers();
    DrawSync(0);
    cdrom_stream(CD_RES_MOVIE_BIN, FIELD_MOVIE_LOAD_ADDRESS);
    func_80140018(0);
    field_load_map(D_801ED480);
    object_index = D_801ED482;
    DrawSync(0);
    ClearOTagR(buffers[0].ordering_table, FIELD_RENDER_OT_LENGTH);
    ClearOTagR(buffers[1].ordering_table, FIELD_RENDER_OT_LENGTH);
    field_select_object(object_index & 0xFFFF, buffers);
    g_field_text_saved_configs = (FieldTextConfig*)D_801ED000;
    D_801ED000 += FIELD_TEXT_SAVED_CONFIG_SIZE;
    field_size_work_buffer();
    FIELD_PRIM_CURSOR(&buffers[0]) = (u8*)D_801ED00C;
    FIELD_PRIM_CURSOR(&buffers[1]) = (u8*)D_801ED010;
    field_text_reset_windows();
}

/**
 * @brief Load a field map's graphics and register its objects.
 *
 * Reads map resource FIELD_MAP_RESOURCE_BASE + @p map_id into 0x80180000,
 * uploads its texture to VRAM, clears every object's built flag, and, when a
 * pixel lookup is selected, runs field_apply_pixel_lookup once per distinct
 * object image.
 *
 * @param map_id Map id; ids below 15 use a blocking queued read, others stream.
 * @see decomp.me (100%) https://decomp.me/scratch/Pvb0P
 */
void field_load_map(s32 map_id)
{
    RECT rect;
    u_long* seen_images[10];
    s32 seen_count;
    s32 remaining;
    u_long** seen;
    s32 columns;
    u8* texture;
    FieldMapObject* object;
    u_long* image;
    FieldMapObject** objects;
    u16 queued_id;
    u32 id;

    DrawSync(0);
    if (map_id == 0)
    {
        id = 0;
    }
    else
    {
        id = map_id;
    }
    queued_id = id;
    if (queued_id < FIELD_MAP_QUEUED_READ_LIMIT)
    {
        cdrom_queue_read((map_id + FIELD_MAP_RESOURCE_BASE) & 0xFFFF, FIELD_MAP_LOAD_ADDRESS);
        cdrom_wait_queue_empty();
    }
    else
    {
        cdrom_stream((map_id + FIELD_MAP_RESOURCE_BASE) & 0xFFFF, FIELD_MAP_LOAD_ADDRESS);
    }
    texture = FIELD_MAP_HEADER->texture;
    rect.x = 0x140;
    rect.y = 0x100;
    columns = FIELD_MAP_HEADER->texture_bytes >> 9;
    rect.h = 0x100;
    while (columns != 0)
    {
        rect.w = (columns <= FIELD_MAP_TEXTURE_UPLOAD_COLUMNS) ? columns : FIELD_MAP_TEXTURE_UPLOAD_COLUMNS;
        LoadImage(&rect, (u_long*)texture);
        texture += FIELD_MAP_TEXTURE_COLUMN_BYTES;
        columns -= rect.w;
        rect.x += FIELD_MAP_TEXTURE_COLUMN_WIDTH;
    }

    DrawSync(0);
    for (objects = g_field_objects; *objects != NULL; objects++)
    {
        (*objects)->built = 0;
    }
    if (D_801ED490 != 0)
    {
        objects = g_field_objects;
        seen_count = 0;
        if (*objects != NULL)
        {
            seen = seen_images;
            do
            {
                object = *objects;
                image = object->image;
                remaining = seen_count;
                while (remaining != 0)
                {
                    if (*seen == image)
                    {
                        break;
                    }
                    remaining--;
                    seen++;
                }
                if (remaining == 0)
                {
                    seen_count++;
                    *seen = image;
                    field_apply_pixel_lookup((u16*)image, object->pixel_count, D_801ED490 - 1, object);
                }
                objects++;
                seen = seen_images;
            } while (*objects != NULL);
        }
    }
}

/**
 * @brief Select a field map object and apply its image and background colour.
 *
 * Walks g_field_objects to object @p object_index, copies its parameters to
 * the block at 0x801ED400, sets the background colour of both frame buffers
 * of @p buffers, uploads the object's texture rows and CLUT to VRAM and builds
 * its render records.
 *
 * @param object_index Index into g_field_objects; stops early at the last object.
 * @param buffers Field render context; when NULL the DRAWENV update is skipped.
 * @see decomp.me (100%) https://decomp.me/scratch/vjiqR
 */
void field_select_object(u16 object_index, FieldRenderBuffer* buffers)
{
    FieldMapObject** objects = g_field_objects;
    FieldObjectParams* params = FIELD_OBJECT_PARAMS;
    s16 remaining = object_index - 1;
    FieldMapObject* object;
    u_long* image;
    u16 rows;
    u8 clut_width;
    RECT rect;

    while ((remaining & 0xFFFF) != 0xFFFF)
    {
        if (objects[1] == NULL)
        {
            break;
        }
        remaining--;
        objects++;
    }

    object = *objects;
    params->unk0 = object->unk30;
    params->unk2 = object->unk32;
    params->unk4 = (object->background.word >> 1) & 1;
    if (buffers != NULL)
    {
        FieldRenderBuffer* back = &buffers[1];

        back->draw_env.isbg = 1;
        buffers[0].draw_env.isbg = 1;
        if (object->background.word & 1)
        {
            buffers[0].draw_env.r0 = object->background.b.r;
            buffers[0].draw_env.g0 = object->background.b.g;
            buffers[0].draw_env.b0 = object->background.b.b;
            buffers[1].draw_env.r0 = object->background.b.r;
            buffers[1].draw_env.g0 = object->background.b.g;
            buffers[1].draw_env.b0 = object->background.b.b;
        }
        else
        {
            buffers[0].draw_env.r0 = 0;
            buffers[0].draw_env.g0 = 0;
            buffers[0].draw_env.b0 = 0;
            buffers[1].draw_env.r0 = 0;
            buffers[1].draw_env.g0 = 0;
            buffers[1].draw_env.b0 = 0;
        }
    }
    image = object->image;
    rect.x = 0;
    rect.y = FIELD_OBJECT_CLUT_VRAM_Y;
    rows = object->image_size >> 8;
    if (rows != 0)
    {
        s32 row_count = rows;

        rect.w = 0x100;
        rect.h = rows;
        LoadImage(&rect, image);
        image += row_count << 7;
        rect.y += rows;
    }
    clut_width = object->image_size;
    if (clut_width != 0)
    {
        rect.w = clut_width;
        rect.h = 1;
        LoadImage(&rect, image);
    }
    field_build_render_records(object, object_index & 0xFFFF);
    field_collision_rebuild_spans();
}
