#include "wmap_land_transition.h"
#include "wmap_land_preview.h"
#include "wmap_land_preview_lines.h"
#include "gpu_packet.h"
#include "wmap_resource_support.h"
#include "wmap_map_labels.h"
#include "sdk/libgte.h"
#include "sdk/libgpu.h"
#include "sdk/inline_c.h"
#include "sdk/gte_dmpsx_compat.h"

#define WMAP_VIEW_ROWS 3
#define WMAP_PREVIEW_BOB_MASK 0xFF
#define WMAP_PREVIEW_BOB_PHASE 32
#define WMAP_PREVIEW_BOB_SCALE 32
#define WMAP_PREVIEW_BOB_SHIFT 5
#define WMAP_CAROUSEL_FRAME_SHIFT 2
#define WMAP_PREVIEW_OT_SHIFT 7
#define WMAP_PREVIEW_FRONT_OT 5
#define WMAP_PREVIEW_BACK_OT 6
#define WMAP_PREVIEW_BASE_OT 4
#define WMAP_EMPTY_CELL 0xFF
#define WMAP_EMPTY_MARKER_OT 173
#define WMAP_EMPTY_MARKER_SIZE 32
#define WMAP_EMPTY_MARKER_CLUT 0x7F2D
#define WMAP_EMPTY_MARKER_TPAGE 0x5B
#define WMAP_EMPTY_MARKER_V 192
#define WMAP_PREVIEW_SECOND_LAYER_V 32
#define WMAP_ARTIFACT_PICKUP_VOLUME 143
#define WMAP_ARTIFACT_RETURN_VOLUME 152
#define WMAP_PREVIEW_MAP_SCALE 81920
#define WMAP_PREVIEW_EFFECT_SCALE 24576
#define WMAP_PREVIEW_PROJECTED_MODE 2
/* Seven slots ahead, with whole turns retained for signed remainder arithmetic. */
#define WMAP_PREVIEW_SLOT_OFFSET 43

/** @brief Ordering table and packet allocation cursor for the current map buffer. */
typedef struct
{
    u8 pad_00[0x70];
    u_long ordering_table[0xB3];
    u8* packet_cursor;
} WmapRenderContext;

/** @brief Map translation, projection scale, and unreconstructed trailing state. */
typedef struct
{
    s32 x;
    s32 y;
    s32 projection_scale;
    s32 unknown_0c;
    s32 unknown_10;
} WmapProjection;

/** @brief Land identifier in a world-map cell. */
typedef struct
{
    s32 land_id;
    u8 pad_04[0x24];
} WmapCell;

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

/** @brief Two packed U/V/width/height layers per preview texture record. */
enum WmapPreviewTextureByte
{
    WMAP_PREVIEW_TEXTURE_WIDTH = 2,
    WMAP_PREVIEW_TEXTURE_HEIGHT = 3,
    WMAP_PREVIEW_TEXTURE_BYTES = 8
};

extern const POLY_FT4 g_wmap_preview_quad_template;
extern u8 g_wmap_preview_textures[];
extern const DVECTOR g_wmap_preview_map_positions[WMAP_VIEW_ROWS][WMAP_VIEW_COLUMNS];
extern const DVECTOR g_wmap_empty_marker_positions[WMAP_VIEW_ROWS][WMAP_VIEW_COLUMNS];
extern u8 g_wmap_preview_quad_shapes[];
extern s8 D_80051B4C[];
extern WmapPreviewPathPoint g_wmap_preview_travel_frames[];
extern WmapArtifactTransferFrame g_wmap_artifact_pickup_frames[];
extern s32 g_wmap_artifact_return_offsets[];
extern WmapArtifactTransferFrame g_wmap_artifact_return_frames[];
extern s32 g_wmap_preview_foreground;
extern s32 D_800D921C;
extern s32 D_800DCEC0;
extern s32 D_800DCEEC;
extern s32 D_800DCEF0;
extern s32 D_8011CF18;
extern s32 g_wmap_preview_bob_y;
extern s32 D_8011CF50;
extern s32 D_8011CF74;
extern s32 D_8011D4FC;
extern s32 D_8011D504;
extern s32 D_8011D508;
extern s32 D_8011D52C;
extern SVECTOR D_80139278;
extern WmapCell D_80139290[][6];
extern s32 D_80139838[];
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapRenderContext* D_801398EC;
extern WmapProjection D_80139950;
extern s32 D_8013B208;
extern s32 g_wmap_preview_projection_x;
extern s32 g_wmap_preview_projection_y;
extern s32 D_80182234;
extern s32 D_8018223C;
extern VECTOR D_80182DC0;
extern s32 D_80182DE0;
extern s32 D_801ADAE0;

/**
 * @brief Advance the map selection preview and draw its marker and artifact.
 * @note Artifact transfer frames run before the cursor returns to the map or carousel.
 * @note Partial reconstruction: 95.887260% with gcc280_g0.
 */
void wmap_update_land_preview(void)
{
    SVECTOR position;
    MATRIX matrix;
    DVECTOR screen;
    DVECTOR* screen_ptr;
    WmapPreviewPathPoint* travel_point;
    WmapArtifactTransferFrame* return_frame;
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

    if ((D_8013B208 == 0) || (D_801ADAE0 == 0))
    {
        next_frame = g_wmap_preview_bob_frame + 1;
        g_wmap_preview_bob_frame = next_frame;

        if (D_80182DE0 == 0)
        {
            bob_sample = D_80051B4C[(next_frame & WMAP_PREVIEW_BOB_MASK) + WMAP_PREVIEW_BOB_PHASE];
            /* Bias negative samples so the signed shift rounds toward zero. */
            if (bob_sample < 0)
            {
                bob_sample += WMAP_PREVIEW_BOB_SCALE - 1;
            }
            bob_sample >>= WMAP_PREVIEW_BOB_SHIFT;
            g_wmap_preview_bob_y = bob_sample;
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

        if (D_8011CF18 >= WMAP_SELECTION_ARTIFACTS)
        {
            g_wmap_preview_bob_y = 0;
        }

        if (D_801398D0 == WMAP_PREVIEW_PROJECTED_MODE)
        {
            RotMatrix(&D_80139278, &matrix);
            TransMatrix(&matrix, &D_80182DC0);
            SetRotMatrix(&matrix);
            SetTransMatrix(&matrix);

            position.vz = 0;
            position.vx = ((D_80139950.x * WMAP_PREVIEW_MAP_SCALE) / D_80139950.projection_scale * WMAP_PREVIEW_EFFECT_SCALE) / D_80139950.projection_scale;
            position.vy = ((D_80139950.y * WMAP_PREVIEW_MAP_SCALE) / D_80139950.projection_scale * WMAP_PREVIEW_EFFECT_SCALE) / D_80139950.projection_scale;

            gte_ldv0(&position);
            gte_rtps();
            screen_ptr = &screen;
            gte_stsxy(screen_ptr);

            g_wmap_preview_projection_x = D_80182234 - screen.vx;
            g_wmap_preview_projection_y = D_8018223C - screen_ptr->vy;
        }
        else
        {
            g_wmap_preview_projection_y = 0;
            g_wmap_preview_projection_x = 0;
        }

        /* Travel follows a shared path; selected artifacts have their own transfer frames. */
        switch (D_8011CF18)
        {
        case WMAP_SELECTION_MAP:
            g_wmap_preview_x = g_wmap_preview_map_positions[D_800DCEF0][D_800DCEEC].vx;
            g_wmap_preview_y = g_wmap_preview_map_positions[D_800DCEF0][D_800DCEEC].vy;
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

                if (D_8011D4FC == WMAP_NO_ARTIFACT)
                {
                    D_8011CF18 = WMAP_SELECTION_ARTIFACTS;
                    D_8011CF50 = 0;
                    g_wmap_artifact_transfer_frame = D_8011D4FC;
                    g_wmap_preview_texture = 0;
                    g_wmap_preview_shape = 0;
                }
                else if (g_wmap_artifact_transfer_frame == -1)
                {
                    g_wmap_artifact_transfer_frame = g_wmap_artifact_return_offsets[D_8011D4FC];
                    g_wmap_artifact_transfer_end = g_wmap_artifact_return_offsets[D_8011D4FC + 1] - 1;
                }
                else if (g_wmap_artifact_transfer_frame == g_wmap_artifact_transfer_end)
                {
                    func_800652A8(g_wmap_artifact_return_frames[g_wmap_artifact_transfer_end].sound_id, WMAP_ARTIFACT_RETURN_VOLUME,
                                  g_wmap_artifact_transfer_end, D_8011D4FC);
                    D_8011CF18 = WMAP_SELECTION_ARTIFACTS;
                    D_8011CF50 = 0;
                    g_wmap_preview_texture = 0;
                    g_wmap_artifact_transfer_frame = -1;
                    g_wmap_preview_shape = 0;
                    D_8011D4FC = -1;
                }
                else
                {
                    return_frame = g_wmap_artifact_return_frames;
                    return_frame += g_wmap_artifact_transfer_frame;
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
                        artifact_slots = D_80139838;
                        if (carousel_frame < 0)
                        {
                            carousel_frame += WMAP_CAROUSEL_STEP_FRAMES - 1;
                        }
                        g_wmap_preview_artifact_visible = 0;
                        artifact_slots[((carousel_frame >> WMAP_CAROUSEL_FRAME_SHIFT) + WMAP_PREVIEW_SLOT_OFFSET) % WMAP_ARTIFACT_SLOTS] = D_8011D4FC;
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

            if (D_8011D4FC == WMAP_NO_ARTIFACT)
            {
                g_wmap_preview_texture = 0;
                g_wmap_preview_artifact_visible = 0;
            }

            if (g_wmap_preview_travel_end == travel_index)
            {
                D_8011CF18 = WMAP_SELECTION_MAP;
                D_8011CF50 = 0;
            }
            else if (g_wmap_artifact_transfer_frame == -1)
            {
                g_wmap_preview_artifact_visible = 1;
                g_wmap_preview_travel_frame = travel_index - 1;
                g_wmap_preview_x = travel_point->x;
                g_wmap_preview_y = travel_point->y;
                if (D_8011D4FC != WMAP_NO_ARTIFACT)
                {
                    s32* artifact_slots;
                    carousel_frame = g_wmap_carousel_frame;
                    artifact_slots = D_80139838;
                    if (carousel_frame < 0)
                    {
                        carousel_frame += WMAP_CAROUSEL_STEP_FRAMES - 1;
                    }
                    artifact_slots[((carousel_frame >> WMAP_CAROUSEL_FRAME_SHIFT) + WMAP_PREVIEW_SLOT_OFFSET) % WMAP_ARTIFACT_SLOTS] = -1;
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
                    func_800652A8(sound_id, WMAP_ARTIFACT_PICKUP_VOLUME, g_wmap_preview_travel_frame, D_8011D4FC);
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

        if ((D_8013B208 == 0) && (D_8013986C == 0))
        {
            POLY_FT4* packet;
            u8 x0_offset;
            u8 y0_offset;
            u8 x1_offset;
            u8 y1_offset;
            u8 x2_offset;
            u8 y2_offset;
            u8 x3_offset;
            u8 y3_offset;
            u16 u_base;
            u8 v_base;

            packet = (POLY_FT4*)D_801398EC->packet_cursor;
            *packet = g_wmap_preview_quad_template;

            if (D_8011D4FC == WMAP_NO_ARTIFACT)
            {
                screen_x = g_wmap_preview_draw_x;
                screen_y = g_wmap_preview_draw_y;
            }
            else
            {
                screen_x = g_wmap_preview_draw_x + g_wmap_artifact_images[D_8011D4FC].offset_x;
                screen_y = g_wmap_preview_draw_y + g_wmap_artifact_images[D_8011D4FC].offset_y;
            }

            screen_y += D_80182DE0 + g_wmap_preview_bob_y;
            screen_x += g_wmap_preview_projection_x;
            screen_y += g_wmap_preview_projection_y;

            /* Shape bytes encode signed corner displacements. */
            x0_offset = g_wmap_preview_quad_shapes[g_wmap_preview_shape * WMAP_PREVIEW_QUAD_BYTES + WMAP_PREVIEW_X0];
            y0_offset = g_wmap_preview_quad_shapes[g_wmap_preview_shape * WMAP_PREVIEW_QUAD_BYTES + WMAP_PREVIEW_Y0];
            x1_offset = g_wmap_preview_quad_shapes[g_wmap_preview_shape * WMAP_PREVIEW_QUAD_BYTES + WMAP_PREVIEW_X1];
            y1_offset = g_wmap_preview_quad_shapes[g_wmap_preview_shape * WMAP_PREVIEW_QUAD_BYTES + WMAP_PREVIEW_Y1];
            x2_offset = g_wmap_preview_quad_shapes[g_wmap_preview_shape * WMAP_PREVIEW_QUAD_BYTES + WMAP_PREVIEW_X2];
            y2_offset = g_wmap_preview_quad_shapes[g_wmap_preview_shape * WMAP_PREVIEW_QUAD_BYTES + WMAP_PREVIEW_Y2];
            x3_offset = g_wmap_preview_quad_shapes[g_wmap_preview_shape * WMAP_PREVIEW_QUAD_BYTES + WMAP_PREVIEW_X3];
            y3_offset = g_wmap_preview_quad_shapes[g_wmap_preview_shape * WMAP_PREVIEW_QUAD_BYTES + WMAP_PREVIEW_Y3];

            packet->x0 = (s8)x0_offset + screen_x;
            packet->y0 = (s8)y0_offset + screen_y;
            packet->x1 = (s8)x1_offset + screen_x;

            /* Copy the adjacent U/V bytes together from the texture record. */
            *(u16*)&packet->u0 = *(u16*)&g_wmap_preview_textures[g_wmap_preview_texture * WMAP_PREVIEW_TEXTURE_BYTES];
            u_base = packet->u0;
            v_base = packet->v0;
            packet->u1 = u_base + g_wmap_preview_textures[(g_wmap_preview_texture * WMAP_PREVIEW_TEXTURE_BYTES) | WMAP_PREVIEW_TEXTURE_WIDTH];
            packet->v1 = v_base;
            packet->u2 = u_base;
            packet->v2 = v_base + g_wmap_preview_textures[(g_wmap_preview_texture * WMAP_PREVIEW_TEXTURE_BYTES) | WMAP_PREVIEW_TEXTURE_HEIGHT];

            packet->y1 = (s8)y1_offset + screen_y;
            packet->x2 = (s8)x2_offset + screen_x;
            packet->y2 = (s8)y2_offset + screen_y;
            packet->x3 = (s8)x3_offset + screen_x;
            packet->y3 = (s8)y3_offset + screen_y;

            packet->u3 = u_base + g_wmap_preview_textures[(g_wmap_preview_texture * WMAP_PREVIEW_TEXTURE_BYTES) | WMAP_PREVIEW_TEXTURE_WIDTH];
            packet->v3 = v_base + g_wmap_preview_textures[(g_wmap_preview_texture * WMAP_PREVIEW_TEXTURE_BYTES) | WMAP_PREVIEW_TEXTURE_HEIGHT];

            if (g_wmap_preview_texture != 0)
            {
                if (g_wmap_preview_foreground != 0)
                {
                    addPrim(&D_801398EC->ordering_table[WMAP_PREVIEW_FRONT_OT], packet);
                }
                else
                {
                    addPrim(&D_801398EC->ordering_table[WMAP_PREVIEW_BACK_OT + ot_shift], packet);
                }

                g_wmap_preview_saved_quads[(D_8011CF74 & 1) * WMAP_PREVIEW_QUADS_PER_BUFFER] = *packet;

                if (D_800D921C < WMAP_PACKET_LIMIT)
                {
                    D_800D921C += sizeof(POLY_FT4);
                    D_801398EC->packet_cursor += sizeof(POLY_FT4);
                }

                /* The second marker layer uses the next texture row. */
                second_layer = (POLY_FT4*)D_801398EC->packet_cursor;
                *second_layer = *packet;
                second_layer->v0 += WMAP_PREVIEW_SECOND_LAYER_V;
                second_layer->v1 += WMAP_PREVIEW_SECOND_LAYER_V;
                second_layer->v2 += WMAP_PREVIEW_SECOND_LAYER_V;
                second_layer->v3 += WMAP_PREVIEW_SECOND_LAYER_V;
                addPrim(&D_801398EC->ordering_table[WMAP_PREVIEW_BASE_OT + ot_shift], second_layer);
                g_wmap_preview_saved_quads[(D_8011CF74 & 1) * WMAP_PREVIEW_QUADS_PER_BUFFER + 1] = *second_layer;

                if (D_800D921C < WMAP_PACKET_LIMIT)
                {
                    D_800D921C += sizeof(POLY_FT4);
                    D_801398EC->packet_cursor += sizeof(POLY_FT4);
                }
            }
            else
            {
                addPrim(&D_801398EC->ordering_table[WMAP_PREVIEW_BASE_OT + ot_shift], packet);
                g_wmap_preview_saved_quads[(D_8011CF74 & 1) * WMAP_PREVIEW_QUADS_PER_BUFFER] = *packet;
                g_wmap_preview_saved_quads[(D_8011CF74 & 1) * WMAP_PREVIEW_QUADS_PER_BUFFER + 1] = *packet;

                if (D_800D921C < WMAP_PACKET_LIMIT)
                {
                    D_800D921C += sizeof(POLY_FT4);
                    D_801398EC->packet_cursor += sizeof(POLY_FT4);
                }
            }
        }

        if ((D_801ADAE0 == 0) && (D_8013986C == 0))
        {
            if (((D_8011D4FC != WMAP_NO_ARTIFACT) && (g_wmap_preview_artifact_visible != 0)) || (D_8011D52C != 0))
            {
                SPRT* sprite;

                sprite = (SPRT*)D_801398EC->packet_cursor;
                if ((D_8011D52C != 0) && (D_801398D0 != WMAP_PREVIEW_PROJECTED_MODE))
                {
                    sprite->x0 = D_8011D504;
                    sprite->y0 = D_8011D508;
                }
                else
                {
                    sprite->x0 = (g_wmap_preview_draw_x + g_wmap_artifact_images[D_8011D4FC].anchor_x) + g_wmap_preview_projection_x;
                    D_8011D504 = sprite->x0;
                    sprite->y0 = ((g_wmap_preview_draw_y + g_wmap_artifact_images[D_8011D4FC].anchor_y) + D_80182DE0) + g_wmap_preview_bob_y +
                                 g_wmap_preview_projection_y;
                    D_8011D508 = sprite->y0;
                }

                sprite->w = g_wmap_artifact_images[D_8011D4FC].width;
                sprite->h = g_wmap_artifact_images[D_8011D4FC].height;
                sprite->u0 = g_wmap_artifact_images[D_8011D4FC].u;
                sprite->v0 = g_wmap_artifact_images[D_8011D4FC].v;
                sprite->clut = WMAP_ARTIFACT_CLUT;
                SET_BGR0_PACKED(sprite, 0x80808080);
                setSprt(sprite);
                setSemiTrans(sprite, 1);
                addPrim(&D_801398EC->ordering_table[WMAP_PREVIEW_FRONT_OT], sprite);

                if (D_800D921C < WMAP_PACKET_LIMIT)
                {
                    D_800D921C += sizeof(SPRT);
                    D_801398EC->packet_cursor += sizeof(SPRT);
                }
                func_8006534C(WMAP_ARTIFACT_TPAGE, WMAP_PREVIEW_FRONT_OT, D_801398EC, sprite);
            }

            cell_x = (D_80139950.x / WMAP_CELL_SPACING) + D_800DCEEC;
            cell_y = (D_80139950.y / WMAP_CELL_SPACING) + D_800DCEF0;
            if ((D_8011CF18 == WMAP_SELECTION_MAP) && (D_800DCEC0 != 0))
            {
                land_id = D_80139290[cell_x][cell_y].land_id;
                if (land_id == WMAP_EMPTY_CELL)
                {
                    land_id = -1;
                }
                func_8005FF88(land_id);
            }

            if ((D_80139290[cell_x][cell_y].land_id == WMAP_EMPTY_CELL) && (D_8011CF18 == WMAP_SELECTION_MAP) && (D_801398D0 != WMAP_PREVIEW_PROJECTED_MODE) &&
                (D_8013B208 == 0))
            {
                POLY_FT4* packet;
                s32 row;
                s32 column;
                packet = (POLY_FT4*)D_801398EC->packet_cursor;
                packet->clut = WMAP_EMPTY_MARKER_CLUT;
                SET_BGR0_PACKED(packet, GPU_TINT_NEUTRAL);
                row = D_800DCEF0;
                column = D_800DCEEC;
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
                addPrim(&D_801398EC->ordering_table[WMAP_EMPTY_MARKER_OT], packet);

                if (D_800D921C < WMAP_PACKET_LIMIT)
                {
                    D_800D921C += sizeof(POLY_FT4);
                    D_801398EC->packet_cursor += sizeof(POLY_FT4);
                }
            }
        }
    }
}
