#include "wmap_frame_render.h"
#include "wmap_land_transition.h"
#include "wmap_land_preview.h"
#include "wmap_party_travel.h"
#include "wmap_land_layout.h"
#include "wmap_land_effect_loader.h"
#include "wmap_view_effects.h"
#include "wmap_resource_support.h"
#include "wmap_main.h"
#include "wmap_effect_backdrop.h"
#include "wmap_map_labels.h"
#include "wmap_sequence_runtime.h"
#include "wmap_effect_resources.h"
#include "cdrom.h"
#include "gpu_packet.h"
#include "sdk/libgte.h"
#include "sdk/libgpu.h"
#include "sdk/libetc.h"
#include "sdk/inline_c.h"
#include "sdk/gte_dmpsx_compat.h"

#define WMAP_GRID_SIZE 6
#define WMAP_PLACEMENT_DELAY 30
#define WMAP_PLACEMENT_DELAY_END 31
#define WMAP_CAROUSEL_TRIANGLES 168
#define WMAP_TRIANGLE_VERTICES 3
#define WMAP_CAROUSEL_SLIDE_STEP 6
#define WMAP_CAROUSEL_OPEN_X 100
#define WMAP_CAROUSEL_CLOSED_X 148
#define WMAP_CAROUSEL_OT 30
#define WMAP_ARTIFACT_OT 11
#define WMAP_ARTIFACT_SHADOW_OT 29
#define WMAP_CAROUSEL_TPAGE 0x1B
#define WMAP_ARTIFACT_SHADOW_CLUT 0x7FC0
#define WMAP_ARTIFACT_SHADOW_SKEW 10
#define WMAP_PLACEMENT_RESOURCE_BASE 0x10CE
#define WMAP_SELECTION_VOLUME 128
#define WMAP_CAROUSEL_VOLUME 143

/** @brief Sounds used by artifact selection and placement. */
enum WmapSelectionSound
{
    WMAP_SOUND_OPEN_ARTIFACTS = 4,
    WMAP_SOUND_TURN_RIGHT = 8,
    WMAP_SOUND_TURN_LEFT = 9,
    WMAP_SOUND_PLACE_ARTIFACT = 22,
    WMAP_SOUND_CLOSE_ARTIFACTS = 24
};

/** @brief Carousel slide and visibility modes. */
enum WmapCarouselMode
{
    WMAP_CAROUSEL_VISIBLE,
    WMAP_CAROUSEL_SLIDE_OUT,
    WMAP_CAROUSEL_SLIDE_IN,
    WMAP_CAROUSEL_HIDDEN
};

/** @brief Signed map-screen coordinate pair. */
typedef struct
{
    s16 x;
    s16 y;
} WmapPoint;

/** @brief Map translation and projection scale. */
typedef struct
{
    s32 x;
    s32 y;
    s32 scale;
} WmapProjectionState;

/** @brief Placement eligibility within a map cell. */
typedef struct
{
    u8 pad_00[4];
    s16 placement_allowed;
    u8 pad_06[0x22];
} WmapCell;

/** @brief Route state reset when selecting an artifact. */
typedef struct
{
    u8 pad_00[2];
    s16 state;
    u8 pad_04[0x18];
} WmapRouteCell;

/** @brief Word indices in each carousel triangle's texture record. */
enum WmapCarouselTextureWord
{
    WMAP_TEXTURE_CLUT,
    WMAP_TEXTURE_U0,
    WMAP_TEXTURE_V0,
    WMAP_TEXTURE_U1,
    WMAP_TEXTURE_V1,
    WMAP_TEXTURE_U2,
    WMAP_TEXTURE_V2,
    WMAP_TEXTURE_WORDS
};

/** @brief GTE screen position available as a packed word or coordinate pair. */
typedef union
{
    u32 packed;
    struct
    {
        u16 x;
        u16 y;
    } point;
} WmapScreenPosition;

extern const WmapPoint g_wmap_cell_focus_offsets[];
extern s16 D_800D928A;
extern s32 D_800DBE70;
extern s32 D_800DBE78;
extern s32 g_wmap_cursor_column;
extern s32 g_wmap_cursor_row;
extern s32 g_wmap_sequence_count;
extern s32 D_80139244;
extern s32 g_wmap_view_mode;
extern s32 g_wmap_view_scroll_mode;
extern s32 D_8013B208;
extern s32 g_wmap_scroll_remaining_x;
extern s32 g_wmap_scroll_remaining_y;
extern s32 D_80182E34;
extern const s32 D_8005135C[];
extern u16 D_800CC776;
extern s32 D_800CCBF4[];
extern s32 D_8011CF18;
extern WmapRouteCell D_8011D108[6][6];
extern s32 D_8011D4FC;
extern s32 D_8011D510;
extern s32 D_8011D52C;
extern s32 D_8011D530;
extern s32 D_80129550;
extern WmapCell D_80139290[6][6];
extern s32 D_801398F4;
extern WmapProjectionState g_wmap_view;
extern s32 D_80182DE0;

extern s32 D_801ADAFC;
extern s32 D_80139838[WMAP_ARTIFACT_SLOTS];
extern s32 g_wmap_carousel_turn_frames;
extern s32 g_wmap_carousel_turn_step;

extern const u16 g_wmap_carousel_textures[];
extern const SVECTOR g_wmap_carousel_vertices[];
extern const s16 g_wmap_carousel_faces[];
extern WmapPoint g_wmap_artifact_positions[WMAP_ARTIFACT_SLOTS];
extern VECTOR g_wmap_carousel_translation;

s32 akao_cmd_c2(s32 value0, s32 value1, s32 value2, s32 value3);
s32 wmap_begin_land_placement(s32 initialize);

/**
 * @brief Begin the land placement effect once map movement has stopped.
 * @param initialize Scheduler initialization flag; unused by this callback.
 * @return One to retry next frame, or zero after starting the effect.
 */
s32 wmap_begin_land_placement(s32 initialize)
{
    const WmapPoint* point;
    const WmapPoint* points;
    s32 index;
    s32 x;
    s32 y;

    if (g_wmap_sequence_count != 0)
    {
        return 1;
    }
    func_80064F64(wmap_get_land_count_tier() + WMAP_PLACEMENT_RESOURCE_BASE);
    wmap_start_map_tint(0x808080);
    D_800DBE70 = 2;
    g_wmap_screen_fade_mode = 1;
    D_800DBE78 = WMAP_CAROUSEL_SLIDE_IN;
    D_80139244 = 0;
    D_80182E34 = 2;
    g_wmap_backdrop_target_level = 0x10;
    func_8006D870(0);
    g_wmap_view_scroll_mode = 2;
    g_wmap_view_mode = 0;
    D_8013B208 = 0;
    D_800D928A = 0x80;
    points = g_wmap_cell_focus_offsets;
    y = g_wmap_cursor_row;
    x = g_wmap_cursor_column;
    index = y * WMAP_VIEW_COLUMNS + x;
    point = &points[index];
    g_wmap_scroll_remaining_x = -point->x;
    g_wmap_scroll_remaining_y = -point->y;
    wmap_reset_after_transition();
    return 0;
}

/** @brief Handle artifact selection, carousel rotation, and placement confirmation. */
void wmap_update_artifact_selection(void)
{
    s32 map_x;
    s32 map_y;

    map_x = g_wmap_view.x / WMAP_CELL_SPACING + g_wmap_cursor_column;
    map_y = g_wmap_view.y / WMAP_CELL_SPACING + g_wmap_cursor_row;

    if (g_wmap_carousel_turn_frames != 0)
    {
        s32 rotation_frame;

        rotation_frame = g_wmap_carousel_frame + g_wmap_carousel_turn_step;
        g_wmap_carousel_frame = rotation_frame;
        if (rotation_frame < 0)
        {
            g_wmap_carousel_frame = WMAP_CAROUSEL_FRAMES - 1;
        }
        else if (rotation_frame >= WMAP_CAROUSEL_FRAMES)
        {
            g_wmap_carousel_frame = 0;
        }

        g_wmap_carousel_turn_frames--;
        D_800CC776 = g_wmap_carousel_angles[g_wmap_carousel_frame].angle;
        if (g_wmap_carousel_turn_frames == 0)
        {
            g_wmap_input_locked = 0;
            g_wmap_buttons_repeat = 0;
        }
    }
    else if (D_8011CF18 == WMAP_SELECTION_MAP)
    {
        s32 input_mask;
        s32 no_artifact;

        no_artifact = -1;
        input_mask = PADRleft;
        if (D_8011D4FC != no_artifact)
        {
            input_mask = PADRright;
        }

        if ((g_wmap_buttons_repeat & input_mask) != 0)
        {
            if (D_80182DE0 == 0 && g_wmap_party_moving == 0)
            {
                s32 view_index;
                s32 first_frame;

                D_8011CF18 = WMAP_SELECTION_SHOW_ARTIFACTS;
                g_wmap_input_locked = 1;
                g_wmap_buttons_held = 0;
                g_wmap_buttons_repeat = 0;
                func_8005FF88(-1);

                view_index = g_wmap_cursor_column + g_wmap_cursor_row * WMAP_VIEW_COLUMNS;
                first_frame = D_8005135C[view_index];
                g_wmap_preview_travel_frame = first_frame;
                g_wmap_preview_travel_end = D_8005135C[view_index + 1] - 1;
                func_8006D8F0(0, first_frame, D_8005135C);
                func_8006D870(1);
                D_80129550 = 0;
                g_wmap_artifact_transfer_frame = no_artifact;
                wmap_play_sound(WMAP_SOUND_OPEN_ARTIFACTS, WMAP_SELECTION_VOLUME);
            }
        }

        if (D_80129550 == 1 && (g_wmap_buttons_repeat & PADRdown) != 0 && D_8011D4FC != -1 && D_80139290[map_x][map_y].placement_allowed != 0 && D_80182DE0 == 0)
        {
            D_80182DE0 = D_80129550;
            wmap_play_sound(WMAP_SOUND_PLACE_ARTIFACT, WMAP_SELECTION_VOLUME);
            g_wmap_buttons_held = 0;
            g_wmap_buttons_repeat = 0;
            g_wmap_map_button_mask = 0x20;
            g_wmap_buttons_held = 0;
            g_wmap_buttons_repeat = 0;
        }

        if ((g_wmap_buttons_repeat & PADRright) != 0)
        {
            D_80182DE0 = 0;
            g_wmap_map_button_mask = -1;
            cdrom_wait_queue_empty();
        }

        if (D_80139290[map_x][map_y].placement_allowed != 0 && D_80182DE0 != 0)
        {
            if (D_80182DE0 < WMAP_PLACEMENT_DELAY_END)
            {
                D_80182DE0++;
            }

            if (D_80182DE0 == WMAP_PLACEMENT_DELAY)
            {
                s32 artifact_id;

                D_8011D510 = map_x;
                D_8011D530 = map_y;
                g_wmap_input_locked = 1;
                g_wmap_buttons_held = 0;
                D_8011D52C = 1;
                g_wmap_buttons_repeat = 0;
                D_8013B208 = 1;
                akao_cmd_c2(0, 60, 127, 1);
                func_800591A8(D_8011D4FC);
                artifact_id = D_8011D4FC;
                if (artifact_id == 22)
                {
                    wmap_place_land(D_8011D510, D_8011D530, 16);
                }
                else
                {
                    wmap_place_land(D_8011D510, D_8011D530, artifact_id);
                }
                wmap_install_callback(wmap_begin_land_placement);
            }
        }
    }
    else
    {
        if ((g_wmap_buttons_repeat & (PADRright | PADRleft)) != 0)
        {
            func_8005FF88(-1);
            D_8011CF18 = WMAP_SELECTION_SHOW_MAP;
            g_wmap_input_locked = 1;
            g_wmap_preview_travel_frame = D_8005135C[g_wmap_cursor_column + g_wmap_cursor_row * WMAP_VIEW_COLUMNS + 1] - 1;
            g_wmap_preview_travel_end = D_8005135C[g_wmap_cursor_column + g_wmap_cursor_row * WMAP_VIEW_COLUMNS];
            func_8006D870(0);
            D_8011D4FC = -1;
            g_wmap_preview_artifact_visible = 0;
            wmap_play_sound(WMAP_SOUND_CLOSE_ARTIFACTS, WMAP_SELECTION_VOLUME);
            g_wmap_input_locked = 1;
            g_wmap_buttons_held = 0;
            g_wmap_buttons_repeat = 0;
        }

        if ((g_wmap_buttons_repeat & PADLright) != 0)
        {
            g_wmap_input_locked = 1;
            g_wmap_buttons_held = 0;
            g_wmap_buttons_repeat = 0;
            wmap_play_sound(WMAP_SOUND_TURN_RIGHT, WMAP_CAROUSEL_VOLUME);
            g_wmap_carousel_turn_step = 1;
            g_wmap_carousel_turn_frames = WMAP_CAROUSEL_STEP_FRAMES;
            wmap_scroll_artifact_list(1, D_80139838);
        }

        if ((g_wmap_buttons_repeat & PADLleft) != 0)
        {
            g_wmap_input_locked = 1;
            g_wmap_buttons_held = 0;
            g_wmap_buttons_repeat = 0;
            wmap_play_sound(WMAP_SOUND_TURN_LEFT, WMAP_CAROUSEL_VOLUME);
            g_wmap_carousel_turn_step = -1;
            g_wmap_carousel_turn_frames = WMAP_CAROUSEL_STEP_FRAMES;
            wmap_scroll_artifact_list(0, D_80139838);
        }

        if ((g_wmap_buttons_repeat & PADRdown) != 0)
        {
            s32 artifact_id;

            artifact_id = wmap_get_selected_artifact();
            D_8011D4FC = artifact_id;
            g_wmap_preview_bob_frame = 0;
            if (artifact_id != -1)
            {
                D_80129550 = 1;
                for (map_y = 0; map_y < WMAP_GRID_SIZE; map_y++)
                {
                    for (map_x = 0; map_x < WMAP_GRID_SIZE; map_x++)
                    {
                        D_80139290[map_x][map_y].placement_allowed = wmap_can_place_land(map_x, map_y, D_8011D4FC);
                        D_8011D108[map_x][map_y].state = 0;
                    }
                }

                D_8011CF18 = WMAP_SELECTION_SHOW_MAP;
                g_wmap_input_locked = 1;
                g_wmap_buttons_held = 0;
                g_wmap_buttons_repeat = 0;
                g_wmap_artifact_transfer_frame = D_800CCBF4[D_8011D4FC];
                g_wmap_artifact_transfer_end = D_800CCBF4[D_8011D4FC + 1] - 1;
                g_wmap_preview_travel_frame = D_8005135C[g_wmap_cursor_column + g_wmap_cursor_row * WMAP_VIEW_COLUMNS + 1] - 1;
                g_wmap_preview_travel_end = D_8005135C[g_wmap_cursor_column + g_wmap_cursor_row * WMAP_VIEW_COLUMNS];
                func_8006D870(0);
                func_800A89DC(D_8011D4FC);
            }
        }

        if (D_8011CF18 == WMAP_SELECTION_ARTIFACTS)
        {
            s32* artifacts;
            s32* artifact_slot;
            s32 label_id;

            artifacts = D_80139838;
            artifact_slot = &artifacts[(D_801398F4 + 0x7FFF) % WMAP_ARTIFACT_SLOTS];
            if (*artifact_slot != 0xFF)
            {
                label_id = wmap_get_selected_artifact();
                if (label_id != -1)
                {
                    label_id += 0x40;
                }
                func_8005FF88(label_id);
            }
        }
    }
}

/** @brief Slide and draw the artifact carousel, its icons, and their shadows. */
void wmap_draw_artifact_carousel(void)
{
    MATRIX transform;
    SVECTOR position;
    s32 facing;
    WmapScreenPosition screen;
    POLY_FT3* triangle;
    SPRT* sprite;
    POLY_FT4* shadow;
    WmapArtifactImage* image;
    s32 index;
    s32 draw_index;
    s32 packet_bytes;

    if (D_800DBE78 == WMAP_CAROUSEL_HIDDEN)
    {
        return;
    }
    if (D_800DBE78 == WMAP_CAROUSEL_SLIDE_IN)
    {
        if (g_wmap_carousel_translation.vx > WMAP_CAROUSEL_OPEN_X)
        {
            g_wmap_carousel_translation.vx -= WMAP_CAROUSEL_SLIDE_STEP;
            g_wmap_carousel_translation.vy -= WMAP_CAROUSEL_SLIDE_STEP;
        }
        else
        {
            D_800DBE78 = WMAP_CAROUSEL_VISIBLE;
        }
    }
    else if (D_800DBE78 == WMAP_CAROUSEL_SLIDE_OUT)
    {
        if (g_wmap_carousel_translation.vx < WMAP_CAROUSEL_CLOSED_X)
        {
            g_wmap_carousel_translation.vx += WMAP_CAROUSEL_SLIDE_STEP;
            g_wmap_carousel_translation.vy += WMAP_CAROUSEL_SLIDE_STEP;
        }
        else
        {
            return;
        }
    }

    TransMatrix(&transform, &g_wmap_carousel_translation);
    RotMatrix(&g_wmap_carousel_rotation, &transform);
    SetRotMatrix(&transform);
    SetTransMatrix(&transform);

    /* Cull the back of the carousel before committing each triangle packet. */
    for (index = 0; index < WMAP_CAROUSEL_TRIANGLES; index++)
    {
        triangle = (POLY_FT3*)g_wmap_current_frame->packet_cursor;
        gte_ldv3(&g_wmap_carousel_vertices[g_wmap_carousel_faces[index * WMAP_TRIANGLE_VERTICES]],
                 &g_wmap_carousel_vertices[g_wmap_carousel_faces[index * WMAP_TRIANGLE_VERTICES + 1]],
                 &g_wmap_carousel_vertices[g_wmap_carousel_faces[index * WMAP_TRIANGLE_VERTICES + 2]]);
        gte_rtpt();
        SET_BGR0_PACKED(triangle, 0x24808080);
        triangle->clut = g_wmap_carousel_textures[index * WMAP_TEXTURE_WORDS + WMAP_TEXTURE_CLUT];
        triangle->u0 = g_wmap_carousel_textures[index * WMAP_TEXTURE_WORDS + WMAP_TEXTURE_U0];
        triangle->v0 = g_wmap_carousel_textures[index * WMAP_TEXTURE_WORDS + WMAP_TEXTURE_V0];
        gte_stsxy3(&triangle->x0, &triangle->x1, &triangle->x2);
        gte_nclip();
        triangle->u1 = g_wmap_carousel_textures[index * WMAP_TEXTURE_WORDS + WMAP_TEXTURE_U1];
        triangle->v1 = g_wmap_carousel_textures[index * WMAP_TEXTURE_WORDS + WMAP_TEXTURE_V1];
        gte_stopz(&facing);
        if (facing > 0)
        {
            triangle->tag = 0x07000000;
            triangle->tpage = WMAP_CAROUSEL_TPAGE;
            triangle->u2 = g_wmap_carousel_textures[index * WMAP_TEXTURE_WORDS + WMAP_TEXTURE_U2];
            triangle->v2 = g_wmap_carousel_textures[index * WMAP_TEXTURE_WORDS + WMAP_TEXTURE_V2];
            addPrim(&g_wmap_current_frame->ordering_table[WMAP_CAROUSEL_OT], triangle);
            if (g_wmap_packet_bytes < WMAP_PACKET_LIMIT)
            {
                g_wmap_packet_bytes += sizeof(POLY_FT3);
                g_wmap_current_frame->packet_cursor += sizeof(POLY_FT3);
            }
        }
    }

    /* Draw from the far side toward the selected slot. */
    for (draw_index = g_wmap_carousel_frame / WMAP_CAROUSEL_STEP_FRAMES + WMAP_ARTIFACT_SLOTS - 1;
         draw_index > g_wmap_carousel_frame / WMAP_CAROUSEL_STEP_FRAMES - 1; draw_index--)
    {
        index = (draw_index + WMAP_ARTIFACT_SLOTS) % WMAP_ARTIFACT_SLOTS;
        if (D_80139838[index] != WMAP_NO_ARTIFACT)
        {
            sprite = (SPRT*)g_wmap_current_frame->packet_cursor;
            position.vx = g_wmap_artifact_positions[index].x;
            position.vy = 0;
            position.vz = g_wmap_artifact_positions[index].y;
            gte_ldv0(&position);
            gte_rtps();
            SET_BGR0_PACKED(sprite, 0x80808080);
            image = &g_wmap_artifact_images[D_80139838[index]];
            gte_stsxy(&screen.packed);
            sprite->x0 = screen.packed + image->carousel_x;
            sprite->y0 = screen.point.y + image->carousel_y;
            sprite->w = image->width;
            sprite->h = image->height;
            sprite->u0 = image->u;
            sprite->v0 = image->v;
            sprite->clut = WMAP_ARTIFACT_CLUT;
            setlen(sprite, 4);
            setcode(sprite, 0x66);
            addPrim(&g_wmap_current_frame->ordering_table[WMAP_ARTIFACT_OT], sprite);
            packet_bytes = g_wmap_packet_bytes;
            if (packet_bytes < WMAP_PACKET_LIMIT)
            {
                g_wmap_packet_bytes = packet_bytes + sizeof(SPRT);
                g_wmap_current_frame->packet_cursor += sizeof(SPRT);
            }

            shadow = (POLY_FT4*)g_wmap_current_frame->packet_cursor;
            SET_BGR0_PACKED(shadow, 0x00404040);
            shadow->x0 = screen.packed + image->shadow_x + WMAP_ARTIFACT_SHADOW_SKEW;
            shadow->y0 = image->height + (screen.point.y + image->shadow_y);
            shadow->x1 = image->width + (screen.packed + image->shadow_x) + WMAP_ARTIFACT_SHADOW_SKEW;
            shadow->y1 = image->height + (screen.point.y + image->shadow_y);
            shadow->x2 = screen.packed + image->shadow_x;
            shadow->y2 = screen.point.y + image->shadow_y;
            shadow->x3 = image->width + (screen.packed + image->shadow_x);
            shadow->y3 = screen.point.y + image->shadow_y;
            shadow->u0 = image->u;
            shadow->v0 = image->v;
            shadow->u1 = image->u + image->width;
            shadow->v1 = image->v;
            shadow->u2 = image->u;
            shadow->v2 = image->v + image->height;
            shadow->u3 = image->u + image->width;
            shadow->v3 = image->v + image->height;
            shadow->clut = WMAP_ARTIFACT_SHADOW_CLUT;
            setlen(shadow, 9);
            setcode(shadow, 0x2E);
            shadow->tpage = WMAP_ARTIFACT_TPAGE;
            if (D_801ADAFC != 0)
            {
                addPrim(&g_wmap_current_frame->ordering_table[WMAP_ARTIFACT_SHADOW_OT], shadow);
                if (g_wmap_packet_bytes < WMAP_PACKET_LIMIT)
                {
                    g_wmap_packet_bytes += sizeof(POLY_FT4);
                    g_wmap_current_frame->packet_cursor += sizeof(POLY_FT4);
                }
            }
        }
    }
    func_8006534C(WMAP_ARTIFACT_TPAGE, WMAP_ARTIFACT_SHADOW_OT);
    func_8006534C(WMAP_ARTIFACT_TPAGE, WMAP_ARTIFACT_OT);
}
