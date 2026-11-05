#include "common.h"
#include <inline_c.h>
#include <libgpu.h>
#include <libgte.h>

#include "../internal/wmap_cells.h"
#include "../internal/wmap_frame_render.h"
#include "../internal/wmap_land_preview.h"
#include "../internal/wmap_land_preview_lines.h"
#include "../internal/wmap_land_transition.h"
#include "../internal/wmap_main.h"
#include "../internal/wmap_map_labels.h"
#include "../internal/wmap_resource_support.h"
#include "../internal/wmap_sequence_runtime.h"
#include "../internal/wmap_view_effects.h"
#include "common/gpu_packet.h"
#include "sdk/gte_dmpsx_compat.h"

#define WMAP_VIEW_ROWS 3
#define WMAP_PREVIEW_BOB_MASK 0xFF
#define WMAP_PREVIEW_BOB_PHASE 32
#define WMAP_PREVIEW_BOB_SCALE 32
#define WMAP_CAROUSEL_FRAME_SHIFT 2
#define WMAP_PREVIEW_OT_SHIFT 7
#define WMAP_PREVIEW_FRONT_OT 5
#define WMAP_PREVIEW_BACK_OT 6
#define WMAP_PREVIEW_BASE_OT 4
#define WMAP_EMPTY_MARKER_OT 173
#define WMAP_EMPTY_MARKER_SIZE 32
#define WMAP_EMPTY_MARKER_CLUT 0x7F2D
#define WMAP_EMPTY_MARKER_TPAGE 0x5B
#define WMAP_EMPTY_MARKER_V 192
#define WMAP_PREVIEW_SECOND_LAYER_V 32
#define WMAP_ARTIFACT_PICKUP_VOLUME 143
#define WMAP_ARTIFACT_RETURN_VOLUME 152
/* Seven slots ahead, with whole turns retained for signed remainder arithmetic. */
#define WMAP_PREVIEW_SLOT_OFFSET 43

/** @brief Screen position on the path between the map and artifact carousel. */
typedef struct
{
    s16 x;
    s16 y;
    u8 pad_04[4];
} WmapPreviewPathPoint;

/** @brief Artifact transfer position, marker appearance, sound, and draw priority. */
typedef struct
{
    s16 x;
    s16 y;
    s16 shape;
    s16 texture;
    s16 sound_id;
    s16 foreground;
} WmapArtifactTransferFrame;

/** @brief Component indices in each packed preview shape. */
enum WmapPreviewQuadByte
{
    WMAP_PREVIEW_X0,
    WMAP_PREVIEW_Y0,
    WMAP_PREVIEW_X1,
    WMAP_PREVIEW_Y1,
    WMAP_PREVIEW_X2,
    WMAP_PREVIEW_Y2,
    WMAP_PREVIEW_X3,
    WMAP_PREVIEW_Y3,
    WMAP_PREVIEW_QUAD_BYTES
};

/** @brief Two-byte cell indices in a preview texture record containing two layers. */
enum WmapPreviewTextureWord
{
    WMAP_PREVIEW_TEXTURE_EXTENT = 1,
    WMAP_PREVIEW_TEXTURE_WORDS = 4
};

/** @brief Width and height bytes following a packed U/V pair. */
typedef struct
{
    u8 width;
    u8 height;
} WmapPreviewExtent;

/** @brief Halfword texture cell, viewed as either packed UVs or byte extents. */
typedef union
{
    u16 uv;
    WmapPreviewExtent extent;
} WmapPreviewTextureCell;

extern const POLY_FT4 g_wmap_preview_quad_template;
extern const WmapPreviewTextureCell g_wmap_preview_textures[];
extern const DVECTOR g_wmap_preview_map_positions[WMAP_VIEW_ROWS * WMAP_VIEW_COLUMNS];
extern const DVECTOR g_wmap_empty_marker_positions[WMAP_VIEW_ROWS][WMAP_VIEW_COLUMNS];
extern u8 g_wmap_preview_quad_shapes[];
extern WmapPreviewPathPoint g_wmap_preview_travel_frames[];
extern WmapArtifactTransferFrame g_wmap_artifact_pickup_frames[];
extern s32 g_wmap_artifact_return_offsets[];
extern WmapArtifactTransferFrame g_wmap_artifact_return_frames[];
extern s32 g_wmap_preview_foreground;

extern s32 g_wmap_preview_bob_y;
extern s32 g_wmap_placement_preview_x;
extern s32 g_wmap_placement_preview_y;

extern s32 g_wmap_preview_projection_x;
extern s32 g_wmap_preview_projection_y;
extern s32 g_wmap_focus_origin_x;
extern s32 g_wmap_focus_origin_y;

/**
 * @brief Advance the map selection preview and draw its marker and artifact.
 * @note Artifact transfer frames run before the cursor returns to the map or carousel.
 */
void wmap_update_land_preview(void)
{
    SVECTOR position;
    MATRIX matrix;
    DVECTOR screen;
    DVECTOR* screen_ptr;
    WmapPreviewPathPoint* travel_point;
    WmapArtifactTransferFrame* return_frame;
    WmapArtifactTransferFrame* return_frames;
    WmapArtifactTransferFrame* pickup_frame;
    POLY_FT4* second_layer;
    s32 next_frame;
    s32 ot_shift;
    s32 cell_x;
    s32 cell_y;
    s32 land_id;
    s32 screen_y;
    s32 screen_x;
    s32 carousel_frame;
    s32 sound_id;
    s32 bob_sample;
    s32 travel_frame;

    func_8005B540();
    ot_shift = 0;

    if ((g_wmap_event_active == 0) || (g_wmap_placement_overlay_hidden == 0))
    {
        next_frame = g_wmap_preview_bob_frame + 1;
        g_wmap_preview_bob_frame = next_frame;

        if (g_wmap_artifact_placement_frame == 0)
        {
            bob_sample = g_wmap_cosine_wave[(next_frame & WMAP_PREVIEW_BOB_MASK) + WMAP_PREVIEW_BOB_PHASE];
            g_wmap_preview_bob_y = bob_sample / WMAP_PREVIEW_BOB_SCALE;
        }
        else
        {
            if (g_wmap_preview_bob_y > 0)
            {
                g_wmap_preview_bob_y--;
            }

            if (g_wmap_preview_bob_y < 0)
            {
                g_wmap_preview_bob_y++;
            }
        }

        if (g_wmap_selection_phase >= WMAP_SELECTION_ARTIFACTS)
        {
            g_wmap_preview_bob_y = 0;
        }

        if (g_wmap_view_scroll_mode == WMAP_SCROLL_SCRIPTED)
        {
            RotMatrix(&g_wmap_camera_rotation, &matrix);
            TransMatrix(&matrix, &g_wmap_camera_translation);
            SetRotMatrix(&matrix);
            SetTransMatrix(&matrix);

            position.vz = 0;
            position.vx = ((g_wmap_view.x * WMAP_MAP_PROJECTION_SCALE) / g_wmap_view.projection_scale * WMAP_VIEW_SCALE) / g_wmap_view.projection_scale;
            position.vy = ((g_wmap_view.y * WMAP_MAP_PROJECTION_SCALE) / g_wmap_view.projection_scale * WMAP_VIEW_SCALE) / g_wmap_view.projection_scale;

            gte_ldv0(&position);
            gte_rtps();
            screen_ptr = &screen;
            gte_stsxy(screen_ptr);

            g_wmap_preview_projection_x = g_wmap_focus_origin_x - screen.vx;
            g_wmap_preview_projection_y = g_wmap_focus_origin_y - screen_ptr->vy;
        }
        else
        {
            g_wmap_preview_projection_y = 0;
            g_wmap_preview_projection_x = 0;
        }

        /* Travel follows a shared path; selected artifacts have their own transfer frames. */
        switch (g_wmap_selection_phase)
        {
            case WMAP_SELECTION_MAP:
                g_wmap_preview_x = g_wmap_preview_map_positions[g_wmap_cursor_column + g_wmap_cursor_row * WMAP_VIEW_COLUMNS].vx;
                g_wmap_preview_y = g_wmap_preview_map_positions[g_wmap_cursor_column + g_wmap_cursor_row * WMAP_VIEW_COLUMNS].vy;
                break;

            case WMAP_SELECTION_OPENING:
                break;

            case WMAP_SELECTION_ARTIFACTS:
                g_wmap_preview_x = g_wmap_preview_travel_frames[g_wmap_preview_travel_frame].x;
                g_wmap_preview_y = g_wmap_preview_travel_frames[g_wmap_preview_travel_frame].y;
                ot_shift = WMAP_PREVIEW_OT_SHIFT;
                break;

            case WMAP_SELECTION_SHOW_ARTIFACTS:
                if (g_wmap_preview_travel_end == g_wmap_preview_travel_frame)
                {
                    travel_frame = g_wmap_preview_travel_frame;
                    g_wmap_preview_x = g_wmap_preview_travel_frames[travel_frame].x;
                    g_wmap_preview_y = g_wmap_preview_travel_frames[travel_frame].y;

                    if (g_wmap_selected_artifact == WMAP_NO_ARTIFACT)
                    {
                        g_wmap_selection_phase = WMAP_SELECTION_ARTIFACTS;
                        g_wmap_input_locked = 0;
                        g_wmap_artifact_transfer_frame = g_wmap_selected_artifact;
                        g_wmap_preview_texture = 0;
                        g_wmap_preview_shape = 0;
                    }
                    else if (g_wmap_artifact_transfer_frame == -1)
                    {
                        g_wmap_artifact_transfer_frame = g_wmap_artifact_return_offsets[g_wmap_selected_artifact];
                        g_wmap_artifact_transfer_end = g_wmap_artifact_return_offsets[g_wmap_selected_artifact + 1] - 1;
                    }
                    else if (g_wmap_artifact_transfer_frame == g_wmap_artifact_transfer_end)
                    {
                        wmap_play_sound(g_wmap_artifact_return_frames[g_wmap_artifact_transfer_end].sound_id, WMAP_ARTIFACT_RETURN_VOLUME);
                        g_wmap_selection_phase = WMAP_SELECTION_ARTIFACTS;
                        g_wmap_input_locked = 0;
                        g_wmap_preview_texture = 0;
                        g_wmap_artifact_transfer_frame = -1;
                        g_wmap_preview_shape = 0;
                        g_wmap_selected_artifact = -1;
                    }
                    else
                    {
                        return_frames = g_wmap_artifact_return_frames;
                        return_frame = &return_frames[g_wmap_artifact_transfer_frame];
                        g_wmap_preview_x = return_frame->x;
                        g_wmap_preview_y = return_frame->y;
                        next_frame = g_wmap_artifact_transfer_frame + 1;
                        g_wmap_artifact_transfer_frame = next_frame;
                        g_wmap_preview_shape = return_frame->shape;
                        g_wmap_preview_texture = return_frame->texture;
                        g_wmap_preview_foreground = return_frame->foreground;

                        if (next_frame == g_wmap_artifact_transfer_end)
                        {
                            s32* artifact_slots;
                            carousel_frame = g_wmap_carousel_frame;
                            artifact_slots = g_wmap_carousel_artifacts;

                            if (carousel_frame < 0)
                            {
                                carousel_frame += WMAP_CAROUSEL_STEP_FRAMES - 1;
                            }

                            g_wmap_preview_artifact_visible = 0;
                            artifact_slots[((carousel_frame >> WMAP_CAROUSEL_FRAME_SHIFT) + WMAP_PREVIEW_SLOT_OFFSET) % WMAP_ARTIFACT_SLOTS] =
                                g_wmap_selected_artifact;
                        }
                    }
                }
                else
                {
                    travel_frame = g_wmap_preview_travel_frame;
                    g_wmap_preview_travel_frame = travel_frame + 1;
                    g_wmap_preview_x = g_wmap_preview_travel_frames[travel_frame].x;
                    g_wmap_preview_y = g_wmap_preview_travel_frames[travel_frame].y;
                }

                break;

            case WMAP_SELECTION_SHOW_MAP:
            {
                WmapPreviewPathPoint* travel_frames;
                WmapArtifactTransferFrame* pickup_frames;
                s32 travel_index;
                s32 next_transfer_frame;

                travel_frames = g_wmap_preview_travel_frames;
                travel_index = g_wmap_preview_travel_frame;
                travel_point = &travel_frames[travel_index];
                g_wmap_preview_x = travel_point->x;
                g_wmap_preview_y = travel_point->y;

                if (g_wmap_selected_artifact == WMAP_NO_ARTIFACT)
                {
                    g_wmap_preview_texture = 0;
                    g_wmap_preview_artifact_visible = 0;
                }

                if (g_wmap_preview_travel_end == travel_index)
                {
                    g_wmap_selection_phase = WMAP_SELECTION_MAP;
                    g_wmap_input_locked = 0;
                }
                else if (g_wmap_artifact_transfer_frame == -1)
                {
                    g_wmap_preview_artifact_visible = 1;
                    g_wmap_preview_travel_frame = travel_index - 1;
                    g_wmap_preview_x = travel_point->x;
                    g_wmap_preview_y = travel_point->y;

                    if (g_wmap_selected_artifact != WMAP_NO_ARTIFACT)
                    {
                        s32* artifact_slots;
                        carousel_frame = g_wmap_carousel_frame;
                        artifact_slots = g_wmap_carousel_artifacts;
                        artifact_slots[((carousel_frame / WMAP_CAROUSEL_STEP_FRAMES) + WMAP_PREVIEW_SLOT_OFFSET) % WMAP_ARTIFACT_SLOTS] = -1;
                    }
                }
                else
                {
                    pickup_frames = g_wmap_artifact_pickup_frames;
                    pickup_frame = &pickup_frames[g_wmap_artifact_transfer_frame];
                    g_wmap_preview_x = pickup_frame->x;
                    g_wmap_preview_y = pickup_frame->y;
                    g_wmap_preview_shape = pickup_frame->shape;
                    g_wmap_preview_texture = pickup_frame->texture;
                    g_wmap_preview_foreground = pickup_frame->foreground;
                    sound_id = pickup_frame->sound_id;

                    if (sound_id != -1)
                    {
                        wmap_play_sound(sound_id, WMAP_ARTIFACT_PICKUP_VOLUME);
                    }

                    ot_shift = WMAP_PREVIEW_OT_SHIFT;
                    next_transfer_frame = g_wmap_artifact_transfer_frame + 1;
                    g_wmap_artifact_transfer_frame = next_transfer_frame;

                    if (g_wmap_artifact_transfer_end == next_transfer_frame)
                    {
                        g_wmap_artifact_transfer_frame = -1;
                    }
                }

                break;
            }
        }

        g_wmap_preview_draw_x = g_wmap_preview_x;
        g_wmap_preview_draw_y = g_wmap_preview_y;

        if ((g_wmap_event_active == 0) && (g_wmap_view_mode == 0))
        {
            POLY_FT4* packet;
            s16 x0_offset;
            s16 y0_offset;
            s16 x1_offset;
            s16 y1_offset;
            s16 x2_offset;
            s16 y2_offset;
            s16 x3_offset;
            s16 y3_offset;

            packet = g_wmap_current_frame->packet_cursor;
            *packet = g_wmap_preview_quad_template;

            if (g_wmap_selected_artifact == WMAP_NO_ARTIFACT)
            {
                screen_x = g_wmap_preview_draw_x;
                screen_y = g_wmap_preview_draw_y + g_wmap_artifact_placement_frame + g_wmap_preview_bob_y;
            }
            else
            {
                screen_x = g_wmap_preview_draw_x + g_wmap_artifact_images[g_wmap_selected_artifact].offset_x;
                screen_y =
                    g_wmap_preview_draw_y + g_wmap_artifact_images[g_wmap_selected_artifact].offset_y + g_wmap_artifact_placement_frame + g_wmap_preview_bob_y;
            }

            screen_x += g_wmap_preview_projection_x;
            screen_y += g_wmap_preview_projection_y;

            /* Shape bytes encode signed corner displacements. */
            x0_offset = (s8)g_wmap_preview_quad_shapes[g_wmap_preview_shape * WMAP_PREVIEW_QUAD_BYTES + WMAP_PREVIEW_X0];
            y0_offset = (s8)g_wmap_preview_quad_shapes[g_wmap_preview_shape * WMAP_PREVIEW_QUAD_BYTES + WMAP_PREVIEW_Y0];
            x1_offset = (s8)g_wmap_preview_quad_shapes[g_wmap_preview_shape * WMAP_PREVIEW_QUAD_BYTES + WMAP_PREVIEW_X1];
            y1_offset = (s8)g_wmap_preview_quad_shapes[g_wmap_preview_shape * WMAP_PREVIEW_QUAD_BYTES + WMAP_PREVIEW_Y1];
            x2_offset = (s8)g_wmap_preview_quad_shapes[g_wmap_preview_shape * WMAP_PREVIEW_QUAD_BYTES + WMAP_PREVIEW_X2];
            y2_offset = (s8)g_wmap_preview_quad_shapes[g_wmap_preview_shape * WMAP_PREVIEW_QUAD_BYTES + WMAP_PREVIEW_Y2];
            x3_offset = (s8)g_wmap_preview_quad_shapes[g_wmap_preview_shape * WMAP_PREVIEW_QUAD_BYTES + WMAP_PREVIEW_X3];
            y3_offset = (s8)g_wmap_preview_quad_shapes[g_wmap_preview_shape * WMAP_PREVIEW_QUAD_BYTES + WMAP_PREVIEW_Y3];

            packet->x0 = x0_offset + screen_x;
            packet->y0 = y0_offset + screen_y;
            packet->x1 = x1_offset + screen_x;
            packet->y1 = y1_offset + screen_y;
            packet->x2 = x2_offset + screen_x;
            packet->y2 = y2_offset + screen_y;
            packet->x3 = x3_offset + screen_x;
            packet->y3 = y3_offset + screen_y;

            /* Copy the adjacent U/V bytes together from the texture record. */
            *(u16*)&packet->u0 = g_wmap_preview_textures[g_wmap_preview_texture * WMAP_PREVIEW_TEXTURE_WORDS].uv;
            packet->u1 = packet->u0 + g_wmap_preview_textures[(g_wmap_preview_texture * WMAP_PREVIEW_TEXTURE_WORDS) | WMAP_PREVIEW_TEXTURE_EXTENT].extent.width;
            packet->v1 = packet->v0;
            packet->u2 = packet->u0;
            packet->v2 =
                packet->v0 + g_wmap_preview_textures[(g_wmap_preview_texture * WMAP_PREVIEW_TEXTURE_WORDS) | WMAP_PREVIEW_TEXTURE_EXTENT].extent.height;
            packet->u3 = packet->u0 + g_wmap_preview_textures[(g_wmap_preview_texture * WMAP_PREVIEW_TEXTURE_WORDS) | WMAP_PREVIEW_TEXTURE_EXTENT].extent.width;
            packet->v3 =
                packet->v0 + g_wmap_preview_textures[(g_wmap_preview_texture * WMAP_PREVIEW_TEXTURE_WORDS) | WMAP_PREVIEW_TEXTURE_EXTENT].extent.height;

            if (g_wmap_preview_texture != 0)
            {
                if (g_wmap_preview_foreground != 0)
                {
                    addPrim(&g_wmap_current_frame->ordering_table[WMAP_PREVIEW_FRONT_OT], packet);
                }
                else
                {
                    addPrim(&g_wmap_current_frame->ordering_table[WMAP_PREVIEW_BACK_OT + ot_shift], packet);
                }

                g_wmap_preview_saved_quads[(g_wmap_frame_count & 1) * WMAP_PREVIEW_QUADS_PER_BUFFER] = *packet;

                if (g_wmap_packet_bytes < WMAP_PACKET_LIMIT)
                {
                    g_wmap_packet_bytes += sizeof(POLY_FT4);
                    g_wmap_current_frame->packet_cursor += sizeof(POLY_FT4);
                }

                /* The second marker layer uses the next texture row. */
                second_layer = g_wmap_current_frame->packet_cursor;
                *second_layer = *packet;
                second_layer->v0 += WMAP_PREVIEW_SECOND_LAYER_V;
                second_layer->v1 += WMAP_PREVIEW_SECOND_LAYER_V;
                second_layer->v2 += WMAP_PREVIEW_SECOND_LAYER_V;
                second_layer->v3 += WMAP_PREVIEW_SECOND_LAYER_V;
                addPrim(&g_wmap_current_frame->ordering_table[WMAP_PREVIEW_BASE_OT + ot_shift], second_layer);
                g_wmap_preview_saved_quads[(g_wmap_frame_count & 1) * WMAP_PREVIEW_QUADS_PER_BUFFER + 1] = *second_layer;

                if (g_wmap_packet_bytes < WMAP_PACKET_LIMIT)
                {
                    g_wmap_packet_bytes += sizeof(POLY_FT4);
                    g_wmap_current_frame->packet_cursor += sizeof(POLY_FT4);
                }
            }
            else
            {
                addPrim(&g_wmap_current_frame->ordering_table[WMAP_PREVIEW_BASE_OT + ot_shift], packet);
                g_wmap_preview_saved_quads[(g_wmap_frame_count & 1) * WMAP_PREVIEW_QUADS_PER_BUFFER] = *packet;
                g_wmap_preview_saved_quads[(g_wmap_frame_count & 1) * WMAP_PREVIEW_QUADS_PER_BUFFER + 1] = *packet;

                if (g_wmap_packet_bytes < WMAP_PACKET_LIMIT)
                {
                    g_wmap_packet_bytes += sizeof(POLY_FT4);
                    g_wmap_current_frame->packet_cursor += sizeof(POLY_FT4);
                }
            }
        }

        if ((g_wmap_placement_overlay_hidden == 0) && (g_wmap_view_mode == 0))
        {
            if (((g_wmap_selected_artifact != WMAP_NO_ARTIFACT) && (g_wmap_preview_artifact_visible != 0)) || (g_wmap_placement_preview_locked != 0))
            {
                SPRT* sprite;

                sprite = g_wmap_current_frame->packet_cursor;

                if ((g_wmap_placement_preview_locked != 0) && (g_wmap_view_scroll_mode != WMAP_SCROLL_SCRIPTED))
                {
                    sprite->x0 = g_wmap_placement_preview_x;
                    sprite->y0 = g_wmap_placement_preview_y;
                }
                else
                {
                    sprite->x0 = (g_wmap_preview_draw_x + g_wmap_artifact_images[g_wmap_selected_artifact].anchor_x) + g_wmap_preview_projection_x;
                    g_wmap_placement_preview_x = sprite->x0;
                    sprite->y0 = ((g_wmap_preview_draw_y + g_wmap_artifact_images[g_wmap_selected_artifact].anchor_y) + g_wmap_artifact_placement_frame) +
                                 g_wmap_preview_bob_y + g_wmap_preview_projection_y;
                    g_wmap_placement_preview_y = sprite->y0;
                }

                sprite->w = g_wmap_artifact_images[g_wmap_selected_artifact].width;
                sprite->h = g_wmap_artifact_images[g_wmap_selected_artifact].height;
                sprite->u0 = g_wmap_artifact_images[g_wmap_selected_artifact].u;
                sprite->v0 = g_wmap_artifact_images[g_wmap_selected_artifact].v;
                SET_BGR0_PACKED(sprite, 0x80808080);
                sprite->clut = WMAP_ARTIFACT_CLUT;
                setSprt(sprite);
                setSemiTrans(sprite, 1);
                addPrim(&g_wmap_current_frame->ordering_table[WMAP_PREVIEW_FRONT_OT], sprite);

                if (g_wmap_packet_bytes < WMAP_PACKET_LIMIT)
                {
                    g_wmap_packet_bytes += sizeof(SPRT);
                    g_wmap_current_frame->packet_cursor += sizeof(SPRT);
                }

                wmap_queue_texture_page(WMAP_ARTIFACT_TPAGE, WMAP_PREVIEW_FRONT_OT);
            }

            cell_x = (g_wmap_view.x / WMAP_CELL_SPACING) + g_wmap_cursor_column;
            cell_y = (g_wmap_view.y / WMAP_CELL_SPACING) + g_wmap_cursor_row;

            if ((g_wmap_selection_phase == WMAP_SELECTION_MAP) && (g_wmap_land_label_updates_enabled != 0))
            {
                land_id = g_wmap_cells[cell_x][cell_y].land_id;

                if (land_id == WMAP_NO_LAND)
                {
                    land_id = -1;
                }

                wmap_set_map_label(land_id);
            }

            if ((g_wmap_cells[cell_x][cell_y].land_id == WMAP_NO_LAND) && (g_wmap_selection_phase == WMAP_SELECTION_MAP) &&
                (g_wmap_view_scroll_mode != WMAP_SCROLL_SCRIPTED) && (g_wmap_event_active == 0))
            {
                POLY_FT4* packet;
                s32 row;
                s32 column;
                packet = g_wmap_current_frame->packet_cursor;
                SET_BGR0_PACKED(packet, GPU_TINT_NEUTRAL);
                row = g_wmap_cursor_row;
                column = g_wmap_cursor_column;
                *(u16*)&packet->clut = WMAP_EMPTY_MARKER_CLUT;
                packet->u3 = WMAP_EMPTY_MARKER_SIZE;
                packet->u1 = WMAP_EMPTY_MARKER_SIZE;
                packet->v1 = WMAP_EMPTY_MARKER_V;
                packet->v0 = WMAP_EMPTY_MARKER_V;
                packet->v3 = WMAP_EMPTY_MARKER_V + WMAP_EMPTY_MARKER_SIZE;
                packet->v2 = WMAP_EMPTY_MARKER_V + WMAP_EMPTY_MARKER_SIZE;
                packet->u2 = 0;
                packet->u0 = 0;
                packet->tpage = WMAP_EMPTY_MARKER_TPAGE;
                /* The table and packet both store adjacent signed X/Y halfwords. */
                *(u32*)&packet->x0 = *(u32*)&g_wmap_empty_marker_positions[row][column].vx;
                setPolyFT4(packet);
                setSemiTrans(packet, 1);
                packet->x2 = packet->x0;
                packet->x3 = packet->x2 + WMAP_EMPTY_MARKER_SIZE;
                packet->x1 = packet->x3;
                packet->y1 = packet->y0;
                packet->y3 = packet->y1 + WMAP_EMPTY_MARKER_SIZE;
                packet->y2 = packet->y3;
                addPrim(&g_wmap_current_frame->ordering_table[WMAP_EMPTY_MARKER_OT], packet);

                if (g_wmap_packet_bytes < WMAP_PACKET_LIMIT)
                {
                    g_wmap_packet_bytes += sizeof(POLY_FT4);
                    g_wmap_current_frame->packet_cursor += sizeof(POLY_FT4);
                }
            }
        }
    }
}
