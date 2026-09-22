#include "wmap_land_transition.h"
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
#define WMAP_CELL_SPACING 48
#define WMAP_VIEW_COLUMNS 3
#define WMAP_PLACEMENT_DELAY 30
#define WMAP_PLACEMENT_DELAY_END 31
#define WMAP_CAROUSEL_TRIANGLES 168
#define WMAP_CAROUSEL_SLIDE_STEP 6
#define WMAP_CAROUSEL_OPEN_X 100
#define WMAP_CAROUSEL_CLOSED_X 148
#define WMAP_PACKET_LIMIT 32000
#define WMAP_CAROUSEL_OT 30
#define WMAP_ARTIFACT_OT 11
#define WMAP_ARTIFACT_SHADOW_OT 29
#define WMAP_CAROUSEL_TPAGE 0x1B
#define WMAP_ARTIFACT_TPAGE 0xAE
#define WMAP_ARTIFACT_CLUT 0x7FEC
#define WMAP_ARTIFACT_SHADOW_CLUT 0x7FC0
#define WMAP_ARTIFACT_SHADOW_SKEW 10
#define WMAP_PLACEMENT_RESOURCE_BASE 0x10CE
#define WMAP_NO_ARTIFACT (-1)

/** @brief Map selection panel animation phases. */
enum WmapSelectionPhase
{
    WMAP_SELECTION_MAP,
    WMAP_SELECTION_OPENING,
    WMAP_SELECTION_ARTIFACTS,
    WMAP_SELECTION_SHOW_ARTIFACTS,
    WMAP_SELECTION_SHOW_MAP
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
    s16 enabled;
    u8 pad_06[0x22];
} WmapCell;

/** @brief Route state reset when selecting an artifact. */
typedef struct
{
    u8 pad_00[2];
    s16 state;
    u8 pad_04[0x18];
} WmapRouteCell;

/** @brief Texture coordinates and palette for one carousel triangle. */
typedef struct
{
    u16 clut;
    u16 u0;
    u16 v0;
    u16 u1;
    u16 v1;
    u16 u2;
    u16 v2;
} WmapCarouselTexture;

/** @brief Ordering table and packet cursor in the active map drawing buffer. */
typedef struct
{
    u8 pad_00[0x70];
    u_long ordering_table[0xB3];
    u8* packet_cursor;
} WmapRenderContext;

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

extern const WmapPoint D_80054944[];
extern s16 D_800D928A;
extern s32 D_800DBE70;
extern s32 D_800DBE78;
extern s32 D_800DCEEC;
extern s32 D_800DCEF0;
extern s32 D_8011CF44;
extern s32 D_80139244;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern s32 D_8013B208;
extern s32 D_8013B254;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern s32 D_80182E34;
extern s32 D_801ADAF4;
extern const s32 D_8005135C[];
extern u16 D_800CC776;
extern s32 D_800CCBF4[];
extern s32 D_800D9168;
extern s32 D_800D9218;
extern s32 D_800D9220;
extern s32 D_800DCF0C;
extern s32 D_8011CF18;
extern s32 D_8011CF50;
extern WmapRouteCell D_8011D108[6][6];
extern s32 D_8011D4FC;
extern s32 D_8011D510;
extern s32 D_8011D52C;
extern s32 D_8011D530;
extern s32 D_80129550;
extern s32 D_8013922C;
extern WmapCell D_80139290[6][6];
extern s32 D_801398C0;
extern s32 D_801398F4;
extern WmapProjectionState D_80139950;
extern s32 D_8013B230;
extern s32 D_80182DDC;
extern s32 D_80182DE0;
extern s32 D_80182E24;
extern s32 D_800D921C;
extern s32 D_801ADAFC;
extern s32 D_80139838[WMAP_ARTIFACT_SLOTS];
extern s32 g_wmap_carousel_turn_frames;
extern s32 g_wmap_carousel_turn_step;
extern WmapRenderContext* D_801398EC;
extern const WmapCarouselTexture g_wmap_carousel_textures[];
extern const SVECTOR g_wmap_carousel_vertices[];
extern const s16 g_wmap_carousel_faces[][3];
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

    if (D_8011CF44 != 0)
    {
        return 1;
    }
    func_80064F64(func_8005D4A4() + WMAP_PLACEMENT_RESOURCE_BASE);
    func_8006683C(0x808080);
    D_800DBE70 = 2;
    D_8013B254 = 1;
    D_800DBE78 = WMAP_CAROUSEL_SLIDE_IN;
    D_80139244 = 0;
    D_80182E34 = 2;
    D_801ADAF4 = 0x10;
    func_8006D870(0);
    D_801398D0 = 2;
    D_8013986C = 0;
    D_8013B208 = 0;
    D_800D928A = 0x80;
    point = D_80054944;
    point += D_800DCEF0 * WMAP_VIEW_COLUMNS + D_800DCEEC;
    D_80182D68 = (s32)-point->x;
    D_80182D78 = (s32)-point->y;
    func_80064094();
    return 0;
}

/** @brief Handle artifact selection, carousel rotation, and placement confirmation. */
void wmap_update_artifact_selection(void)
{
    s32 map_x;
    s32 map_y;
    s32 table_index;
    map_x = D_80139950.x / WMAP_CELL_SPACING + D_800DCEEC;
    map_y = D_80139950.y / WMAP_CELL_SPACING + D_800DCEF0;

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
            D_8011CF50 = 0;
            D_8013922C = 0;
        }
    }
    else if (D_8011CF18 == WMAP_SELECTION_MAP)
    {
        s32 input_mask;
        s32 invalid;

        invalid = -1;
        input_mask = PADRleft;
        if (D_8011D4FC != invalid)
        {
            input_mask = PADRright;
        }

        if ((D_8013922C & input_mask) != 0)
        {
            if (D_80182DE0 == 0 && g_wmap_party_moving == 0)
            {
                s32 index;
                s32 data;

                D_8011CF18 = WMAP_SELECTION_SHOW_ARTIFACTS;
                D_8011CF50 = 1;
                D_801398C0 = 0;
                D_8013922C = 0;
                func_8005FF88(-1);

                index = D_800DCEEC + D_800DCEF0 * WMAP_VIEW_COLUMNS;
                data = D_8005135C[index];
                D_80182DDC = data;
                D_800D9218 = D_8005135C[index + 1] - 1;
                func_8006D8F0(0, data, D_8005135C);
                func_8006D870(1);
                D_80129550 = 0;
                D_800D9168 = invalid;
                func_800652A8(4, 0x80);
            }
        }

        if (D_80129550 == 1 && (D_8013922C & PADRdown) != 0 && D_8011D4FC != -1 && D_80139290[map_x][map_y].enabled != 0 && D_80182DE0 == 0)
        {
            D_80182DE0 = D_80129550;
            func_800652A8(0x16, 0x80);
            D_801398C0 = 0;
            D_8013922C = 0;
            D_800D9220 = 0x20;
            D_801398C0 = 0;
            D_8013922C = 0;
        }

        if ((D_8013922C & PADRright) != 0)
        {
            D_80182DE0 = 0;
            D_800D9220 = -1;
            cdrom_wait_queue_empty();
        }

        if (D_80139290[map_x][map_y].enabled != 0 && D_80182DE0 != 0)
        {
            if (D_80182DE0 < WMAP_PLACEMENT_DELAY_END)
            {
                D_80182DE0++;
            }

            if (D_80182DE0 == WMAP_PLACEMENT_DELAY)
            {
                s32 selection;

                D_8011D510 = map_x;
                D_8011D530 = map_y;
                D_8011CF50 = 1;
                D_801398C0 = 0;
                D_8011D52C = 1;
                D_8013922C = 0;
                D_8013B208 = 1;
                akao_cmd_c2(0, 60, 127, 1);
                func_800591A8(D_8011D4FC);
                selection = D_8011D4FC;
                if (selection == 22)
                {
                    func_8005BBC8(D_8011D510, D_8011D530, 16);
                }
                else
                {
                    func_8005BBC8(D_8011D510, D_8011D530, selection);
                }
                func_8006CBD8(wmap_begin_land_placement);
            }
        }
    }
    else
    {
        do
        {
            if ((D_8013922C & (PADRright | PADRleft)) != 0)
            {
                s32 index;

                func_8005FF88(-1);
                D_8011CF18 = WMAP_SELECTION_SHOW_MAP;
                D_8011CF50 = 1;
                index = D_800DCEEC + D_800DCEF0 * WMAP_VIEW_COLUMNS;
                D_80182DDC = D_8005135C[index + 1] - 1;
                D_800D9218 = D_8005135C[index];
                func_8006D870(0);
                D_8011D4FC = -1;
                D_80182E24 = 0;
                func_800652A8(0x18, 0x80);
                D_8011CF50 = 1;
                D_801398C0 = 0;
                D_8013922C = 0;
            }

            if ((D_8013922C & PADLright) != 0)
            {
                D_8011CF50 = 1;
                D_801398C0 = 0;
                D_8013922C = 0;
                func_800652A8(8, 0x8F);
                g_wmap_carousel_turn_step = 1;
                g_wmap_carousel_turn_frames = WMAP_CAROUSEL_STEP_FRAMES;
                func_8005CA3C(1, D_80139838);
            }

            if ((D_8013922C & PADLleft) != 0)
            {
                D_8011CF50 = 1;
                D_801398C0 = 0;
                D_8013922C = 0;
                func_800652A8(9, 0x8F);
                g_wmap_carousel_turn_step = -1;
                g_wmap_carousel_turn_frames = WMAP_CAROUSEL_STEP_FRAMES;
                func_8005CA3C(0, D_80139838);
            }

            if ((D_8013922C & PADRdown) != 0)
            {
                s32 selected;

                selected = func_8005D8FC();
                D_8011D4FC = selected;
                D_8013B230 = 0;
                if (selected != -1)
                {
                    D_80129550 = 1;
                    for (map_y = 0; map_y < WMAP_GRID_SIZE; map_y++)
                    {
                        for (map_x = 0; map_x < WMAP_GRID_SIZE; map_x++)
                        {
                            D_80139290[map_x][map_y].enabled = func_8005B8C8(map_x, map_y, D_8011D4FC);
                            D_8011D108[map_x][map_y].state = 0;
                        }
                    }

                    D_8011CF18 = WMAP_SELECTION_SHOW_MAP;
                    D_8011CF50 = 1;
                    D_801398C0 = 0;
                    D_8013922C = 0;
                    D_800D9168 = D_800CCBF4[D_8011D4FC];
                    D_800DCF0C = D_800CCBF4[D_8011D4FC + 1] - 1;
                    table_index = D_800DCEEC + D_800DCEF0 * WMAP_VIEW_COLUMNS;
                    D_80182DDC = D_8005135C[table_index + 1] - 1;
                    D_800D9218 = D_8005135C[table_index];
                    func_8006D870(0);
                    func_800A89DC(D_8011D4FC);
                }
            }

            if (D_8011CF18 == WMAP_SELECTION_ARTIFACTS)
            {
                s32* entries;
                s32* entry;
                s32 value;

                entries = D_80139838;
                entry = &entries[(D_801398F4 + 0x7FFF) % WMAP_ARTIFACT_SLOTS];
                if (*entry != 0xFF)
                {
                    value = func_8005D8FC(entry);
                    if (value != -1)
                    {
                        value += 0x40;
                    }
                    func_8005FF88(value);
                }
            }
        } while (0);
    }
}

/** @brief Slide and draw the artifact carousel, its icons, and their shadows. */
void wmap_draw_artifact_carousel(void)
{
    SVECTOR position;
    WmapScreenPosition screen;
    s32 facing;
    MATRIX transform;
    POLY_FT3* triangle;
    SPRT* sprite;
    POLY_FT4* shadow;
    WmapArtifactImage* image;
    s32 face;
    s32 slot;
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
    for (face = 0; face < WMAP_CAROUSEL_TRIANGLES; face++)
    {
        triangle = (POLY_FT3*)D_801398EC->packet_cursor;
        gte_ldv3(&g_wmap_carousel_vertices[g_wmap_carousel_faces[face][0]], &g_wmap_carousel_vertices[g_wmap_carousel_faces[face][1]],
                 &g_wmap_carousel_vertices[g_wmap_carousel_faces[face][2]]);
        gte_rtpt();
        triangle->clut = g_wmap_carousel_textures[face].clut;
        triangle->v0 = (u8)g_wmap_carousel_textures[face].v0;
        triangle->u0 = (u8)g_wmap_carousel_textures[face].u0;
        SET_BGR0_PACKED(triangle, 0x24808080);
        gte_stsxy3(&triangle->x0, &triangle->x1, &triangle->x2);
        gte_nclip();
        triangle->u1 = (u8)g_wmap_carousel_textures[face].u1;
        triangle->v1 = (u8)g_wmap_carousel_textures[face].v1;
        gte_stopz(&facing);
        if (facing > 0)
        {
            triangle->tag = 0x07000000;
            triangle->tpage = WMAP_CAROUSEL_TPAGE;
            triangle->u2 = (u8)g_wmap_carousel_textures[face].u2;
            triangle->v2 = (u8)g_wmap_carousel_textures[face].v2;
            addPrim(&D_801398EC->ordering_table[WMAP_CAROUSEL_OT], triangle);
            if (D_800D921C < WMAP_PACKET_LIMIT)
            {
                D_800D921C += sizeof(POLY_FT3);
                D_801398EC->packet_cursor += sizeof(POLY_FT3);
            }
        }
    }

    /* Draw from the far side toward the selected slot. */
    for (draw_index = g_wmap_carousel_frame / WMAP_CAROUSEL_STEP_FRAMES + WMAP_ARTIFACT_SLOTS - 1;
         draw_index > g_wmap_carousel_frame / WMAP_CAROUSEL_STEP_FRAMES - 1; draw_index--)
    {
        slot = (draw_index + WMAP_ARTIFACT_SLOTS) % WMAP_ARTIFACT_SLOTS;
        if (D_80139838[slot] != WMAP_NO_ARTIFACT)
        {
            sprite = (SPRT*)D_801398EC->packet_cursor;
            position.vx = g_wmap_artifact_positions[slot].x;
            position.vy = 0;
            position.vz = g_wmap_artifact_positions[slot].y;
            gte_ldv0(&position);
            gte_rtps();
            SET_BGR0_PACKED(sprite, 0x80808080);
            image = &g_wmap_artifact_images[D_80139838[slot]];
            gte_stsxy(&screen.packed);
            sprite->x0 = screen.packed + image->carousel_x;
            sprite->y0 = screen.point.y + image->carousel_y;
            sprite->w = image->width;
            sprite->h = image->height;
            sprite->u0 = image->u;
            sprite->clut = WMAP_ARTIFACT_CLUT;
            setlen(sprite, 4);
            setcode(sprite, 0x66);
            sprite->v0 = image->v;
            packet_bytes = D_800D921C;
            addPrim(&D_801398EC->ordering_table[WMAP_ARTIFACT_OT], sprite);
            if (packet_bytes < WMAP_PACKET_LIMIT)
            {
                D_800D921C = packet_bytes + sizeof(SPRT);
                D_801398EC->packet_cursor += sizeof(SPRT);
            }

            shadow = (POLY_FT4*)D_801398EC->packet_cursor;
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
            shadow->clut = WMAP_ARTIFACT_SHADOW_CLUT;
            setlen(shadow, 9);
            setcode(shadow, 0x2E);
            shadow->v3 = image->v + image->height;
            shadow->tpage = WMAP_ARTIFACT_TPAGE;
            if (D_801ADAFC != 0)
            {
                addPrim(&D_801398EC->ordering_table[WMAP_ARTIFACT_SHADOW_OT], shadow);
                if (D_800D921C < WMAP_PACKET_LIMIT)
                {
                    D_800D921C += sizeof(POLY_FT4);
                    D_801398EC->packet_cursor += sizeof(POLY_FT4);
                }
            }
        }
    }
    func_8006534C(WMAP_ARTIFACT_TPAGE, WMAP_ARTIFACT_SHADOW_OT);
    func_8006534C(WMAP_ARTIFACT_TPAGE, WMAP_ARTIFACT_OT);
}
