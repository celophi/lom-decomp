#include "field_scene_internal.h"
#include "field_calls.h"

/** func_80062820 mode: stop with success when the stamped tile is goal_tile. */
#define FIELD_COLLISION_TRACE_STOP_AT_GOAL 2

/**
 * @brief Test one tile of a func_80062820 footprint and stamp it.
 * @param ptr Tile pointer.
 * @note Uses tile_value, stamp, mode and goal_tile of the caller and returns
 *       from it: 0 on a blocked tile, 1 when the goal tile is stamped.
 */
#define FIELD_COLLISION_TRACE_TILE(ptr)                                                                                                                        \
    {                                                                                                                                                          \
        tile_value = *(ptr);                                                                                                                                   \
        if (tile_value >= FIELD_COLLISION_TILE_DEFERRED || tile_value == FIELD_COLLISION_TILE_TOUCHED)                                                         \
        {                                                                                                                                                      \
            return 0;                                                                                                                                          \
        }                                                                                                                                                      \
        if (stamp != 0 && tile_value != FIELD_COLLISION_TILE_START)                                                                                            \
        {                                                                                                                                                      \
            *(ptr) = stamp;                                                                                                                                    \
            if (mode == FIELD_COLLISION_TRACE_STOP_AT_GOAL && (ptr) == goal_tile)                                                                              \
            {                                                                                                                                                  \
                return 1;                                                                                                                                      \
            }                                                                                                                                                  \
        }                                                                                                                                                      \
    }

/**
 * @brief Test and stamp the footprint tiles func_80062820 newly covers.
 * @note edge_state 1 tests the leading column, 2 the leading row and 3 the
 *       whole rectangle; then the stamp advances. Returns from the caller
 *       through FIELD_COLLISION_TRACE_TILE.
 */
#define FIELD_COLLISION_TRACE_EDGES()                                                                                                                          \
    {                                                                                                                                                          \
        if (edge_state != 0)                                                                                                                                   \
        {                                                                                                                                                      \
            switch (edge_state)                                                                                                                                \
            {                                                                                                                                                  \
            case 1:                                                                                                                                            \
                tile_ptr = tile_base;                                                                                                                          \
                if (step_x > 0)                                                                                                                                \
                {                                                                                                                                              \
                    tile_ptr = tile_base + x_span;                                                                                                             \
                }                                                                                                                                              \
                n = row_span;                                                                                                                                  \
                for (;;)                                                                                                                                       \
                {                                                                                                                                              \
                    FIELD_COLLISION_TRACE_TILE(tile_ptr);                                                                                                      \
                    if (n == 0)                                                                                                                                \
                    {                                                                                                                                          \
                        break;                                                                                                                                 \
                    }                                                                                                                                          \
                    tile_ptr += stride;                                                                                                                        \
                    n -= stride;                                                                                                                               \
                }                                                                                                                                              \
                break;                                                                                                                                         \
            case 2:                                                                                                                                            \
                tile_ptr = tile_base;                                                                                                                          \
                if (step_z > 0)                                                                                                                                \
                {                                                                                                                                              \
                    tile_ptr = tile_base + row_span;                                                                                                           \
                }                                                                                                                                              \
                n = x_span;                                                                                                                                    \
                do                                                                                                                                             \
                {                                                                                                                                              \
                    FIELD_COLLISION_TRACE_TILE(tile_ptr);                                                                                                      \
                    tile_ptr++;                                                                                                                                \
                } while (--n != -1);                                                                                                                           \
                break;                                                                                                                                         \
            case 3:                                                                                                                                            \
                tile_ptr = tile_base;                                                                                                                          \
                n = row_span;                                                                                                                                  \
                for (;;)                                                                                                                                       \
                {                                                                                                                                              \
                    scan_ptr = tile_ptr;                                                                                                                       \
                    m = x_span;                                                                                                                                \
                    do                                                                                                                                         \
                    {                                                                                                                                          \
                        FIELD_COLLISION_TRACE_TILE(scan_ptr);                                                                                                  \
                        scan_ptr++;                                                                                                                            \
                    } while (--m != -1);                                                                                                                       \
                    if (n == 0)                                                                                                                                \
                    {                                                                                                                                          \
                        break;                                                                                                                                 \
                    }                                                                                                                                          \
                    tile_ptr += stride;                                                                                                                        \
                    n -= stride;                                                                                                                               \
                }                                                                                                                                              \
                break;                                                                                                                                         \
            }                                                                                                                                                  \
            if (stamp != 0 && stamp != FIELD_COLLISION_TILE_GOAL)                                                                                              \
            {                                                                                                                                                  \
                stamp++;                                                                                                                                       \
            }                                                                                                                                                  \
        }                                                                                                                                                      \
    }

/** PSX scratchpad RAM, used as row work buffers by the rasterisers. */
#define FIELD_COLLISION_SCRATCH 0x1F800000

/** Fixed RAM list of the nodes that block a probe, filled by func_8005DA7C. */
#define FIELD_COLLISION_HIT_LIST ((FieldCollisionNode**)0x801E1000)

/** Fixed RAM list of the nodes a probe touches without being blocked. */
#define FIELD_COLLISION_TOUCH_LIST ((FieldCollisionNode**)0x801E1100)

/**
 * @brief Convert a 24.8 fixed-point coordinate to whole collision cells.
 * @note Rounds toward zero; the explicit branch is the original's spelling of @c v / 256.
 */
#define FIELD_COLLISION_CELL(v) (((v) < 0) ? (((v) + 0xFF) >> 8) : ((v) >> 8))

/** Mask of the surface-kind bits in FieldCollisionSurfaceDef::flags. */
#define FIELD_COLLISION_KIND_MASK 0x3
/** Surface kind: flat floor at FieldCollisionSurfaceDef::height0. */
#define FIELD_COLLISION_KIND_FLAT 0
/** Surface kind: slope sampled with func_8005DFAC. */
#define FIELD_COLLISION_KIND_SLOPE 1
/** Surface flag: the per-span edge bytes carry FIELD_COLLISION_EDGE_OPEN bits. */
#define FIELD_COLLISION_SURFACE_EDGE_FLAGS 0x8
/** Surface flag: movers standing on the surface move at half speed. */
#define FIELD_COLLISION_SURFACE_SLOW 0x10
/** Surface flag: standing on the surface reports FIELD_COLLISION_RESULT_CARRIED. */
#define FIELD_COLLISION_SURFACE_CARRY 0x40

/** Surface kind, read from the low byte of FieldCollisionSurfaceDef::flags. */
#define FIELD_COLLISION_SURFACE_KIND(s) (((u8*)&(s)->flags)[0] & FIELD_COLLISION_KIND_MASK)
/** Span pairs per row, stored in byte 2 of FieldCollisionSurfaceDef::flags. */
#define FIELD_COLLISION_SURFACE_SPANS(s) (((u8*)&(s)->flags)[2])

/** Span edge byte: the edge is open (no wall) on this side. */
#define FIELD_COLLISION_EDGE_OPEN 0x80
/** Span edge byte: mask of the node edge index passed to func_8005E1A8. */
#define FIELD_COLLISION_EDGE_INDEX 0x7F
/** func_8005E1A8 edge index for a wall running along the x axis. */
#define FIELD_COLLISION_EDGE_WALL_X 0x7F
/** func_8005E1A8 edge index for a wall running along the z axis. */
#define FIELD_COLLISION_EDGE_WALL_Z 0x7E

/** func_8005E1A8 slide angle: no wall seen yet. */
#define FIELD_COLLISION_SLIDE_NONE -2

/** FieldCollisionMover::collision_node value asking for a fresh floor search. */
#define FIELD_COLLISION_NODE_SEARCH ((FieldCollisionNode*)-1)
/** FieldCollisionMover::collision_node value for a mover not tied to any floor node. */
#define FIELD_COLLISION_NODE_DETACHED ((FieldCollisionNode*)-2)

/** FieldCollisionMover::flags bit: the mover stands on the node its footprint centre is over. */
#define FIELD_COLLISION_MOVER_SETTLED 0x1
/** FieldCollisionMover::mode_flags bits: the mover moves vertically on its own (jump or fall). */
#define FIELD_COLLISION_MOVER_AIRBORNE 0x30000

/** Floors this much higher than the current best win the floor search. */
#define FIELD_COLLISION_FLOOR_MARGIN 0x14

/** Push-out accumulator value meaning pushes in both directions cancelled out. */
#define FIELD_COLLISION_PUSH_BLOCKED 0x8000

/** func_8005B6AC result: the horizontal move was blocked. */
#define FIELD_COLLISION_RESULT_BLOCKED 0x1
/** func_8005B6AC result: the mover slid along a wall or was pushed out of it. */
#define FIELD_COLLISION_RESULT_SLID 0x2
/** func_8005B6AC result: the floor surface scaled the move (slope or slow surface). */
#define FIELD_COLLISION_RESULT_SLOWED 0x4
/** func_8005B6AC result: the floor surface moved or carried the mover. */
#define FIELD_COLLISION_RESULT_CARRIED 0x10
/** func_8005B6AC result: the mover's height was changed by its floor. */
#define FIELD_COLLISION_RESULT_HEIGHT 0x20
/** func_8005B6AC result: an airborne mover hit a ceiling. */
#define FIELD_COLLISION_RESULT_CEILING 0x40
/** func_8005B6AC result: an airborne mover landed on a floor. */
#define FIELD_COLLISION_RESULT_LANDED 0x80

/** func_8005DA7C class bit: the footprint overlaps the node and may stand on it. */
#define FIELD_COLLISION_CLASS_TOUCH 0x1
/** func_8005DA7C class bit: the node blocks the footprint (wall or too-high floor). */
#define FIELD_COLLISION_CLASS_BLOCK 0x2

/** Surface flag: the node blocks any mover whose step limit is below its top. */
#define FIELD_COLLISION_SURFACE_SOLID 0x4

/** A grounded mover can step onto floors up to this much above its feet. */
#define FIELD_COLLISION_STEP_UP 0x10

/** func_8005E1A8: a slide turning this far (about 100 degrees) from the move is blocked. */
#define FIELD_COLLISION_SLIDE_MAX_TURN 0x472

/** Surface edge-run count bit: the run's edges are open (see FIELD_COLLISION_EDGE_OPEN). */
#define FIELD_COLLISION_RUN_OPEN 0x8000
/** Padding raster span (x0 0x7F00 > x1 -0x8000, so it covers nothing). */
#define FIELD_COLLISION_RASTER_EMPTY_SPAN 0x80007F00
/** Padding value for both edge bytes of an unused raster span. */
#define FIELD_COLLISION_RASTER_EMPTY_FLAGS 0xFFFF

/**
 * @brief Address of span @p n - 1 of a raster row, as an integer sum.
 * @note The original adds the scaled index before the row pointer; pointer
 *       arithmetic (&row[n - 1]) always emits the pointer operand first.
 */
#define FIELD_COLLISION_RASTER_LAST_SPAN(row, n) ((FieldCollisionRasterSpan*)((n) * sizeof(*(row)) + (s32)(row) - sizeof(*(row))))

/** Most distinct group ids func_8005F158 collects while scanning. */
#define FIELD_COLLISION_GROUP_SCAN_MAX 20
/** Most groups a scene can keep (FieldScene::unk4A capacity). */
#define FIELD_COLLISION_GROUP_MAX 10
/** Estimated 4-pixel tile total above which 8-pixel tiles are used. */
#define FIELD_COLLISION_GROUP_TILE_BUDGET 0x4000

/* FieldScene::unk41 error codes, stored with unk28 cleared. */
/** Too many group ids (scan or pair filter). */
#define FIELD_COLLISION_GROUP_ERROR_GROUPS 1
/** More than FIELD_COLLISION_RASTER_NODE_MAX active nodes. */
#define FIELD_COLLISION_GROUP_ERROR_NODES 2
/** Span list overflow while collecting a sub-row. */
#define FIELD_COLLISION_GROUP_ERROR_COLLECT 3
/** Span list overflow while intersecting sub-rows. */
#define FIELD_COLLISION_GROUP_ERROR_INTERSECT 4
/** Span list overflow while building the union. */
#define FIELD_COLLISION_GROUP_ERROR_UNION 5

/** Most active nodes func_8005F5BC can rasterise. */
#define FIELD_COLLISION_RASTER_NODE_MAX 50
/** Capacity of each func_8005F5BC span list. */
#define FIELD_COLLISION_RASTER_SPAN_MAX 128
/** Bytes of one scratchpad span list; the two halves ping-pong between sub-rows. */
#define FIELD_COLLISION_RASTER_ROW_BYTES 0x200

/**
 * @brief Fill @p remaining bytes at @p out with 0xFF (blocked), word-aligned in the middle.
 * @param out Byte cursor; advanced past the filled run.
 * @param count Scratch counter.
 * @param remaining Byte count; consumed.
 * @note An open-coded memset: byte head up to word alignment, word body, byte tail.
 */
#define FIELD_COLLISION_FILL_BLOCKED(out, count, remaining) \
    {                                                       \
        count = (4 - (s32)(out)) & 3;                       \
        while (((count) != 0) && ((remaining) != 0))        \
        {                                                   \
            *(out) = -1;                                    \
            (out) += 1;                                     \
            (count) -= 1;                                   \
            (remaining) -= 1;                               \
        }                                                   \
        (count) = ((remaining) >> 2);                       \
        (count) -= 1;                                       \
        if ((count) != -1)                                  \
        {                                                   \
            do                                              \
            {                                               \
                *(s32*)(out) = -1;                          \
                (count) -= 1;                               \
                (out) += 4;                                 \
            } while ((count) != -1);                        \
        }                                                   \
        (count) = ((remaining) & 3);                        \
        (count) -= 1;                                       \
        if ((count) != -1)                                  \
        {                                                   \
            s32 end = -1;                                   \
            do                                              \
            {                                               \
                *(out) = -1;                                \
                (count) -= 1;                               \
                (out) += 1;                                 \
            } while ((count) != end);                       \
        }                                                   \
    }

/*
 * Group tile-map byte values seen by the path search (func_80060F58). 0 and 1
 * come from func_80060364 (free / near a wall), 0xFE from func_80060CB0 and
 * 0xFF from the dilation. The search writes wave stamps counting down from
 * FIELD_COLLISION_TILE_STAMP_FIRST, so a larger stamp is closer to the start.
 */
/** Tile next to a wall; entering it costs two extra waves. */
#define FIELD_COLLISION_TILE_TOUCHED 1
/** Lowest wave stamp; values below it are unvisited floor. */
#define FIELD_COLLISION_TILE_STAMP_MIN 4
/** Stamp of the first wave around the start tile. */
#define FIELD_COLLISION_TILE_STAMP_FIRST 0xFA
/** Start tile marker. */
#define FIELD_COLLISION_TILE_START 0xFB
/** Goal tile marker. */
#define FIELD_COLLISION_TILE_GOAL 0xFC
/** A wave stamp or the start marker: a tile the walk back may step onto. */
#define FIELD_COLLISION_TILE_IS_STEP(v) ((u8)((v) - FIELD_COLLISION_TILE_STAMP_MIN) < FIELD_COLLISION_TILE_GOAL - FIELD_COLLISION_TILE_STAMP_MIN)
/** Touched tile already queued in the deferred ring. */
#define FIELD_COLLISION_TILE_DEFERRED 0xFD
/** Footprint tile marked by func_80060CB0. */
#define FIELD_COLLISION_TILE_MARKED 0xFE

/*
 * Path queue entries: the tile address in the low 21 bits (plus KSEG0 bit 31)
 * and, from bit 21, the mask of neighbours still worth visiting. The 3x3
 * neighbourhood is numbered row by row, top row first.
 */
/** Mask that turns a queue entry back into its tile address. */
#define FIELD_COLLISION_PATH_TILE_MASK 0x801FFFFF
/** Bit position of the direction mask in a queue entry. */
#define FIELD_COLLISION_PATH_DIR_SHIFT 21
/** Direction mask bits placed in a queue entry. */
#define FIELD_COLLISION_PATH_DIRS(dirs) ((dirs) << FIELD_COLLISION_PATH_DIR_SHIFT)
/** Up-left neighbour (tile - columns - 1). */
#define FIELD_COLLISION_DIR_UL 0x01
/** Up neighbour (tile - columns). */
#define FIELD_COLLISION_DIR_UP 0x02
/** Up-right neighbour (tile - columns + 1). */
#define FIELD_COLLISION_DIR_UR 0x04
/** Left neighbour (tile - 1). */
#define FIELD_COLLISION_DIR_LEFT 0x08
/** Right neighbour (tile + 1). */
#define FIELD_COLLISION_DIR_RIGHT 0x10
/** Down-left neighbour (tile + columns - 1). */
#define FIELD_COLLISION_DIR_DL 0x20
/** Down neighbour (tile + columns). */
#define FIELD_COLLISION_DIR_DOWN 0x40
/** Down-right neighbour (tile + columns + 1). */
#define FIELD_COLLISION_DIR_DR 0x80
/** Top row of the neighbourhood. */
#define FIELD_COLLISION_DIRS_TOP (FIELD_COLLISION_DIR_UL | FIELD_COLLISION_DIR_UP | FIELD_COLLISION_DIR_UR)
/** Bottom row of the neighbourhood. */
#define FIELD_COLLISION_DIRS_BOTTOM (FIELD_COLLISION_DIR_DL | FIELD_COLLISION_DIR_DOWN | FIELD_COLLISION_DIR_DR)
/** Left column of the neighbourhood. */
#define FIELD_COLLISION_DIRS_LEFT (FIELD_COLLISION_DIR_UL | FIELD_COLLISION_DIR_LEFT | FIELD_COLLISION_DIR_DL)
/** Right column of the neighbourhood. */
#define FIELD_COLLISION_DIRS_RIGHT (FIELD_COLLISION_DIR_UR | FIELD_COLLISION_DIR_RIGHT | FIELD_COLLISION_DIR_DR)
/** All eight neighbours (the start entry). */
#define FIELD_COLLISION_DIRS_ALL 0xFF
/** Entry sits on a touched tile: it only expands into unvisited floor. */
#define FIELD_COLLISION_DIR_SLOW 0x100
/** Entry still waits in the deferred ring for one more wave. */
#define FIELD_COLLISION_DIR_DEFERRED 0x200
/** A direction taken into a touched tile, kept in the high byte of `taken`. */
#define FIELD_COLLISION_DIR_TOUCHED(dir) ((dir) << 8)
/** Entries per path bucket (and per path output column). */
#define FIELD_COLLISION_PATH_BUCKET_LEN 0x400
/**
 * Length slot `ring` of the bucket length array, seen through a u32 pointer.
 * @note An integer sum: `lens[ring]` and `lens + ring` put the index first in
 *       the addu and do not match.
 */
#define FIELD_COLLISION_PATH_LEN_SLOT(lens, ring) (*(u32*)((u32)(lens) + ((ring) << 2)))
/**
 * Path column entry `n` places after `p`.
 * @note An integer sum with the byte offset first: `p + n` puts the pointer
 *       first in the addu and does not match.
 */
#define FIELD_COLLISION_PATH_AHEAD(p, n) ((s32*)((n) * (s32)sizeof(s32) + (s32)(p)))
/**
 * A neighbour value the current wave may claim: below the wave limit, and
 * unvisited floor when the entry itself is slow.
 */
#define FIELD_COLLISION_PATH_CAN_ENTER(val, limit, dirs) \
    (((val) < (limit)) && (((val) < FIELD_COLLISION_TILE_STAMP_MIN) || !((dirs) & FIELD_COLLISION_DIR_SLOW)))

/**
 * Diagonal step test: both orthogonal neighbours between the tile and the
 * diagonal must be open and of the same kind (both touched or deferred, or
 * both plain floor). Sets `ok` to 1 when the corner can be cut, else 0.
 * `side` is scratch for the tile values.
 */
#define FIELD_COLLISION_CORNER_TEST(ok, side, first, second)                     \
    {                                                                            \
        (side) = (first);                                                        \
        (ok) = 0;                                                                \
        if ((side) < FIELD_COLLISION_TILE_MARKED)                                \
        {                                                                        \
            if (((side) == FIELD_COLLISION_TILE_TOUCHED) || ((side) == FIELD_COLLISION_TILE_DEFERRED)) \
            {                                                                    \
                (side) = (second);                                               \
                if (((side) == FIELD_COLLISION_TILE_TOUCHED) || ((side) == FIELD_COLLISION_TILE_DEFERRED)) \
                {                                                                \
                    (ok) = 1;                                                    \
                }                                                                \
            }                                                                    \
            else                                                                 \
            {                                                                    \
                (side) = (second);                                               \
                if (((side) < FIELD_COLLISION_TILE_DEFERRED) && ((side) != FIELD_COLLISION_TILE_TOUCHED)) \
                {                                                                \
                    (ok) = 1;                                                    \
                }                                                                \
            }                                                                    \
        }                                                                        \
    }
/** Push limit checked per pop: one pop pushes at most eight entries. */
#define FIELD_COLLISION_PATH_PUSH_LIMIT (FIELD_COLLISION_PATH_BUCKET_LEN - 7)
/** Most path points written to the caller. */
#define FIELD_COLLISION_PATH_OUT_MAX 16

/** func_80060F58 failure: the group tile maps failed to build. */
#define FIELD_COLLISION_PATH_ERROR_GROUPS -1
/** func_80060F58 failure: start or goal lies outside the tile map. */
#define FIELD_COLLISION_PATH_ERROR_BOUNDS -2
/** func_80060F58 failure: the search ran out of waves or queue entries. */
#define FIELD_COLLISION_PATH_ERROR_NO_ROUTE -3
/** func_80060F58 failure: a queue or the path buffer overflowed. */
#define FIELD_COLLISION_PATH_ERROR_OVERFLOW -4
/** func_80060F58 failure: the walk back from the goal found no higher stamp. */
#define FIELD_COLLISION_PATH_ERROR_DEAD_END -5

/**
 * @brief Probe position and footprint passed to func_8005B368.
 *
 * Carries the world-space probe position (x, y, z) plus the footprint
 * extents (width along x and depth along z) plus a vertical height
 * tolerance.
 */
typedef struct FieldCollisionQuery
{
    s32 x;
    s32 y;
    s32 z;
    u16 width;
    s16 height_tolerance;
    u16 depth;
} FieldCollisionQuery;

/**
 * @brief Collision view of the FieldMarkerDef a FieldMarker points at.
 * @note FieldMarkerDef leaves 0x12 as padding and declares 0x14 unsigned; this
 *       view needs both as s16.
 */
typedef struct
{
    u8 pad[0x10];
    /** 0x10 bottom of the vertical band the marker blocks (FieldMarkerDef::depth_bias). */
    s16 band_bottom;
    /** 0x12 height of the band; 1 means the band has no top. */
    s16 band_height;
    /** 0x14 value returned on a hit (FieldMarkerDef::label). */
    s16 value;
} FieldCollisionMarkerDef;

/** @brief One solid run of a collision row, in cells relative to the node origin. */
typedef struct FieldCollisionSpan
{
    s16 min_x;
    s16 max_x;
} FieldCollisionSpan;

/**
 * @brief One run of consecutive boundary points in a node's edge list.
 *
 * The list lives at FieldCollisionSurfaceDef+0x18 and is terminated by a record whose
 * count is zero. A run's points are consecutive entries of
 * g_field_node_angle_table starting at @c index, so only the first index is
 * stored.
 */
typedef struct FieldCollisionEdgeRun
{
    /** Number of points in this run; only the low 15 bits are the count. */
    u16 count;
    /** Angle-table index of the run's first point. */
    u16 index;
} FieldCollisionEdgeRun;

/**
 * @brief Collision view of a FieldNodeDef: surface kind, heights and outline.
 * @note Same layout as FieldNodeDef; here base_x/base_y are the heights height0/height1.
 */
typedef struct FieldCollisionSurfaceDef
{
    u8 pad0[4];
    /** Low byte: surface kind in bits 0-1 plus attribute bits; byte 2: spans per row. */
    s32 flags;
    u8 pad8[2];
    /** Index of the node's third boundary point (C) in g_field_node_angle_table. */
    u16 vertex_c;
    /** Index of the node's second boundary point (B). */
    u16 vertex_b;
    /** Index of the node's first boundary point (A). */
    u16 vertex_a;
    /** Surface height at C, before the node's height offset. */
    s16 height0;
    /** Surface height at B. */
    s16 height1;
    /** Height of the top of the node's solid volume. */
    s16 top;
    u8 pad16[2];
    /** Edge-run list; @c runs[0].index doubles as the closing point. */
    FieldCollisionEdgeRun runs[1];
} FieldCollisionSurfaceDef;

/**
 * @brief Collision view of a FieldNode: bounds, span tables, motion and placement.
 * @note Same layout as FieldNode; rows run along z and spans along x.
 */
typedef struct FieldCollisionNode
{
    struct FieldCollisionNode* next;
    FieldCollisionSurfaceDef* surface;
    u8 pad8[8];
    /** Per-row span table: (min_x, max_x) pairs, FIELD_COLLISION_SURFACE_SPANS per row. */
    void* spans;
    /** Per-row edge attribute bytes, two per span (left end, right end). */
    void* span_flags;
    /** Zero when the node takes no part in collision. */
    u8 active;
    u8 pad19[3];
    s16 min_x;
    s16 max_x;
    s16 max_z;
    s16 min_z;
    /** Per-frame motion carried onto a mover standing on the node (x, height, ?, z). */
    s32 motion_x;
    s32 motion_height;
    s32 unk2C;
    s32 motion_z;
    /** World placement in 24.8 fixed point: x, height of the near and far ends, z. */
    s32 offset_x;
    s32 height_offset;
    s32 far_height_offset;
    s32 offset_z;
} FieldCollisionNode;

/**
 * @brief Actor/mover state resolved by func_8005B6AC.
 * @note Positions are 24.8 fixed point; heights grow downwards (a floor at
 *       height h puts the mover at -(h << 8)). Callers see this record as
 *       FieldMoveRequest (field_contact_geometry.c).
 */
typedef struct FieldCollisionMover
{
    /** 0x00 world x. */
    s32 x;
    /** 0x04 world height, negated (see the note). */
    s32 height;
    /** 0x08 world z. */
    s32 z;
    /** 0x0C requested x movement this frame. */
    s32 move_x;
    /** 0x10 requested height movement this frame. */
    s32 move_height;
    /** 0x14 requested z movement this frame. */
    s32 move_z;
    /** 0x18 height of the highest floor under the footprint centre, negated. */
    s32 resolved_height;
    /** 0x1C floor node, or FIELD_COLLISION_NODE_SEARCH / FIELD_COLLISION_NODE_DETACHED. */
    FieldCollisionNode* collision_node;
    /** 0x20 FIELD_COLLISION_MOVER_SETTLED plus caller bits. */
    s32 flags;
    /** 0x24 footprint width along x, in cells. */
    u16 footprint_width;
    /** 0x26 extra height a floor may rise above the feet and still be stepped onto. */
    s16 height_bias;
    /** 0x28 low half: footprint depth along z in cells; FIELD_COLLISION_MOVER_AIRBORNE bits above. */
    s32 mode_flags;
} FieldCollisionMover;

/** @brief Footprint probe handed to func_8005DA7C (cell position plus height window). */
typedef struct FieldCollisionMoveProbe
{
    /** Mover whose footprint and height bias are tested. */
    FieldCollisionMover* mover;
    /** Footprint centre x, in cells. */
    s32 x;
    /** Footprint centre z, in cells. */
    s32 z;
    /** Feet height after the move (whole units). */
    s16 w;
    /** Step-up limit: feet height before the move plus the height bias. */
    s16 h;
} FieldCollisionMoveProbe;

/**
 * @brief One boundary point of a collision node, as stored in
 *        g_field_node_angle_table.
 * @note Both fields are read unsigned by func_8005E3B0, unlike the signed
 *       FieldCollisionSpan pairs at FieldCollisionNode::spans.
 */
typedef struct FieldCollisionEdgePoint
{
    /** X coordinate. */
    u16 x;
    /** Z coordinate (row before FieldCollisionNode::min_z is subtracted). */
    u16 z;
} FieldCollisionEdgePoint;

/**
 * @brief One horizontal span on a scanline: the inclusive x range it covers.
 * @note Declared as a union so the struct is 4-byte aligned. A plain
 *       two-s16 struct assignment compiles to lwl/lwr; the original uses
 *       lw/sw, so the whole-span copies in the bubble sort go through
 *       @c word.
 */
typedef union FieldCollisionRasterSpan
{
    struct
    {
        s16 x0;
        s16 x1;
    } x;
    s32 word;
} FieldCollisionRasterSpan;

/**
 * @brief Edge attributes for the two ends of the span at the same index.
 * @note Union for the same alignment reason as FieldCollisionRasterSpan; @c half carries
 *       the 0xFFFF padding value written to unused entries.
 */
typedef union FieldCollisionRasterSpanFlags
{
    struct
    {
        /** Attribute of the span's left (x0) end. */
        u8 f0;
        /** Attribute of the span's right (x1) end. */
        u8 f1;
    } f;
    u16 half;
} FieldCollisionRasterSpanFlags;

/**
 * @brief One entry of func_8005F158's group-collection scratch list.
 *
 * @note @c seen accumulates which surface kinds referenced the id: bit 0 from
 *       a flat node, bit 1 from a slope node. A scene with slopes keeps only
 *       the entries that ended up with both bits set.
 */
typedef struct
{
    /** Group id: a flat node's height0, or either end height of a slope. */
    s16 id;
    /** Bitmask of the kinds that referenced this id; 3 means both. */
    s16 seen;
} FieldGroupEntry;

/**
 * @brief One entry of func_8005F5BC's scratch list of active nodes.
 */
typedef struct
{
    FieldNode* node;
    /** Sort key: the node's @c row_start, ascending. */
    s16 key;
    s16 pad;
} FieldCollisionRasterNode;

/**
 * @brief A horizontal span (inclusive x range) in tile columns.
 */
typedef struct
{
    s16 x0;
    s16 x1;
} FieldCollisionTileSpan;

/**
 * @brief One node's in-progress walk over its span table.
 *
 * @note 12 bytes; @c skip counts sub-rows before the node starts contributing.
 */
typedef struct
{
    u16* src;
    s16 count;
    s16 step;
    s16 skip;
    s16 pad;
} FieldCollisionSpanRun;

/** One route point in world units, written by func_80060F58. */
typedef struct
{
    s32 x;
    s32 z;
} FieldCollisionPathPoint;

/** Outgoing parameter block for func_80062820, built at sp+0x4020. */
typedef struct
{
    s32 tile_base;
    s32 goal_tile;
    s32 start_x;
    s32 start_z;
    s32 end_x;
    s32 end_z;
    s32 footprint_width;
    s32 footprint_depth;
    s32 tile_size;
    /** Tile column shift (log2 of the tile size); func_80062820 does not read it. */
    s32 col_shift;
    s32 mode;
    u8 stamp;
} FieldCollisionTraceRequest;

/** Field allocator cursor at 0x801ED000, i.e. FieldMemState::top. */
extern s32 D_801ED000;

s32 func_8005E1A8(FieldCollisionSurfaceDef* surface, s32 edge_index, s32 move_angle, s32 best_angle);
s16 func_8005DFAC();
void func_80062F48(FieldCollisionSurfaceDef* surface, s32* movement);
void func_8005DA7C(FieldCollisionMoveProbe* probe, FieldCollisionNode* node, s32* out_hit, s32* out_touch);

/**
 * @brief Hit-test a probe footprint against the scene's marker volumes.
 *
 * Each FieldMarker is a quadrilateral (two pairs of parallel edges) with a
 * vertical band taken from its definition. The probe box must overlap the
 * band, the marker's bounding box, and the strip between each edge pair.
 *
 * @param query Probe position, footprint dimensions, and vertical tolerance.
 * @return The first overlapping marker's value, or -1 when none overlaps.
 */
s16 func_8005B368(FieldCollisionQuery* query)
{
    FieldScene* scene;
    s32 half_x;
    s32 half_z;
    s32 start_x;
    s32 end_x;
    s32 start_z;
    s32 end_z;
    s32 probe_y;
    FieldMarker* node;
    s32 hit;
    s32 edge_start;
    s32 edge_end;

    /*
     * Each edge is computed inside both arms: jump2 cross-jumps the tails
     * after scheduling, so the next axis's loads stay out of them.
     */
    scene = g_field_scene.scene;
    half_x = (s16)query->width / 2;
    if (query->x >= 0)
    {
        start_x = (query->x >> 8) - half_x;
    }
    else
    {
        start_x = ((query->x + 0xFF) >> 8) - half_x;
    }
    end_x = start_x + (s16)query->width;
    half_z = (s16)query->depth / 2;
    if (query->z >= 0)
    {
        start_z = (query->z >> 8) - half_z;
    }
    else
    {
        start_z = ((query->z + 0xFF) >> 8) - half_z;
    }
    end_z = start_z + (s16)query->depth;
    if (query->y >= 0)
    {
        probe_y = query->y >> 8;
    }
    else
    {
        probe_y = (query->y + 0xFF) >> 8;
    }

    for (node = scene->markers; node != NULL; node = node->next)
    {
        FieldCollisionMarkerDef* obj;
        s16 band_bottom;

        obj = (FieldCollisionMarkerDef*)node->def;
        band_bottom = obj->band_bottom;

        if ((probe_y - query->height_tolerance) >= band_bottom)
        {
            continue;
        }
        if ((obj->band_height != 1) && ((band_bottom + obj->band_height) >= probe_y))
        {
            continue;
        }
        if (node->y_min >= end_z)
        {
            continue;
        }
        if (start_z >= node->y_max)
        {
            continue;
        }
        if (node->x_min >= end_x)
        {
            continue;
        }
        if (start_x >= node->x_max)
        {
            continue;
        }

        hit = 0;
        if (node->side_dx != 0)
        {
            edge_start = (node->side_dy * start_x) / node->side_dx;
            edge_end = (node->side_dy * end_x) / node->side_dx;
            if (((((start_z - edge_start) >= node->side_lo) || ((end_z - edge_start) >= node->side_lo)) || ((start_z - edge_end) >= node->side_lo)) || ((end_z - edge_end) >= node->side_lo))
            {
                if ((((node->side_hi >= (start_z - edge_start)) || (node->side_hi >= (end_z - edge_start))) || (node->side_hi >= (start_z - edge_end))) || (node->side_hi >= (end_z - edge_end)))
                {
                    hit = 1;
                }
            }
        }
        else if (end_x >= node->side_lo)
        {
            if (node->side_hi >= start_x)
            {
                hit = 1;
            }
        }

        if (hit)
        {
            if (node->edge_dx != 0)
            {
                edge_start = (node->edge_dy * start_x) / node->edge_dx;
                edge_end = (node->edge_dy * end_x) / node->edge_dx;
                if (((((start_z - edge_start) >= node->edge_lo) || ((end_z - edge_start) >= node->edge_lo)) || ((start_z - edge_end) >= node->edge_lo)) || ((end_z - edge_end) >= node->edge_lo))
                {
                    if (!((((node->edge_hi < (start_z - edge_start)) && (node->edge_hi < (end_z - edge_start))) && (node->edge_hi < (start_z - edge_end))) && (node->edge_hi < (end_z - edge_end))))
                    {
                        return obj->value;
                    }
                }
            }
            else if (end_x >= node->edge_lo)
            {
                if (node->edge_hi >= start_x)
                {
                    return obj->value;
                }
            }
        }
    }

    return -1;
}

/**
 * @brief Move a mover one frame through the field collision nodes.
 *
 * Finds the floor node under the mover (when asked to), applies the floor's
 * slope, slow and carry effects to the requested move, then sweeps the
 * footprint along the move. A blocked move is turned into a slide along the
 * nearest wall and pushed back out of any node it still overlaps; a move that
 * stays blocked is cancelled. Finally the floor under the new position sets
 * the mover's height, or for an airborne mover the ceiling and landing checks.
 *
 * @param mover Mover state; position, height, floor node and flags are updated in place.
 * @return FIELD_COLLISION_RESULT_* bits, or 0 when the mover did not move.
 *
 * @see decomp.me (100%) https://decomp.me/scratch/N2GNJ
 */
s32 func_8005B6AC(FieldCollisionMover* mover)
{
    FieldCollisionMoveProbe probe;
    s32 hit_count;
    s32 touch_count;
    FieldScene* scene;
    FieldCollisionNode* nodes;
    u16 next_height;
    u16 step_height;
    u16 nodes_offset_x;
    s32 carry_x;
    s32 carry_z;
    s32 result;
    s32 step;
    s32 move_angle;
    s32 delta_x;
    s32 delta_z;
    FieldCollisionNode** hit_iter;
    FieldCollisionNode** touch_iter;
    s32 box_x_end;
    s32 push_x_end;
    s32 box_z_end;
    s32 box_z_start;
    s32 box_z_last;
    s32 box_x_last;
    s32 half_width;
    s32 push_offset_x;
    s32 push_offset_z;
    s32 push_z_last;
    s32 limit;
    s32 span_offset_x;
    s16 extent_z;
    s16 extent_x;
    s32 row_start;
    s16 bias;
    s16 span_max_x;
    s32 x_start;
    s16 x_end;
    s32 floor_candidate;
    s16 flagged_min_x;
    s16 plain_min_x;
    s32 z_end_raw;
    s16 push_min_x;
    s16 push_max_x;
    s32 floor_candidate_b;
    s32 first_floor;
    s32 first_floor_b;
    s32 footprint_w;
    s32 footprint_d;
    s16 z_end;
    s32 span_push_z;
    s32 blocked_marker;
    s32 push_z;
    s32 slope_height;
    s32 box_x_start;
    s32 push_x_start;
    s32 push_z_start;
    s32 push_z_end;
    s32 floor_height;
    s32 ground_cell_z;
    s32 row;
    FieldCollisionSpan* span;
    s32 node_offset_z;
    s32 value;
    s32 scaled;
    s32 touch_offset_z;
    s32 touch_kind;
    s32 hit_offset_z;
    s32 node_offset_x;
    s32 work;
    s32 touch_offset_x;
    s32 touch_height_raw;
    s32 hit_offset_x;
    s32 retry_x_start;
    s32 row_skip;
    s32 slide_z;
    s32 push_row_skip;
    s32 sample;
    s32 old_move_height;
    s16 node_height16;
    s32 touch_cell_x;
    s32 floor_fixed_candidate;
    s32 secondary_rise;
    s32 steps_taken;
    s32 offset_min_x;
    s32 move_z_copy;
    s32 nodes_offset_x_raw;
    s32 row_end;
    s32 shifted_min_x;
    s32 shifted_max_x;
    s32 row_from_start;
    s32 touch_row_start;
    s32 touch_height;
    s32 slope_floor_fixed;
    s32 first_row_start;
    s32 span_push_x;
    s32 span_hit;
    s32 on_slope;
    s32 cell_z;
    s32 scratch;
    s32 floor_fixed;
    s32 slide_angle;
    s32 push_x;
    s32 cell_x;
    s32 remaining;
    s32 hit;
    s32 step_count;
    s32 move_x_abs;
    s32 secondary_cell_x;
    s32 move_z_abs;
    s32 cell;
    s32 push_z_abs;
    s32 ground_x;
    s32 higher;
    s32 coord;
    s32 first_cell_x;
    s32 major;
    s32 push_x_abs;
    s32 node_top;
    s8* right_flags;
    FieldCollisionNode* secondary;
    u8* left_flags;
    s16 touch_spans;
    u8 first_spans;
    u8 hit_spans;
    FieldCollisionNode* standing;
    FieldCollisionNode* standing_node;
    FieldSceneHeader* header;
    FieldSceneHeader* retry_header;
    FieldCollisionSurfaceDef* surface;
    FieldCollisionNode* touch_node;
    FieldCollisionNode* head;
    FieldCollisionNode* node;

    s32 ground_z;
    s32 next_height16;
    s16 step_limit;
    u32 depth;
    s32 hit_row_end;
    s32 next_row;
    s32 rows_past_end;
    carry_x = 0;
    carry_z = 0;
    result = 0;
    scene = g_field_scene.scene;
    mover->resolved_height = 0;
    {
        s32 x = -mover->height - mover->move_height;
        next_height = (u16)((u32)(x + ((x < 0) ? 0xFF : 0)) >> 8);
    }
    {
        s32 x = -mover->height;
        s32 height_bias = mover->height_bias;
        if (x >= 0)
        {
            step_height = (x >> 8) + height_bias;
        }
        else
        {
            step_height = ((x + 0xFF) >> 8) + height_bias;
        }
    }
    head = (FieldCollisionNode*)scene->nodes;
    nodes = head;
    if (head != NULL)
    {
        if (mover->collision_node == FIELD_COLLISION_NODE_SEARCH)
        {
            floor_height = 0;
            coord = mover->x;
            on_slope = 0;
            mover->collision_node = NULL;
            if (coord < 0)
            {
                coord = (coord + 0xFF) >> 8;
            }
            else
            {
                coord >>= 8;
            }
            probe.x = coord;
            coord = mover->z;
            if (coord < 0)
            {
                coord += 0xFF;
            }
            node = nodes;
            cell = coord >> 8;
            probe.z = cell;
            /* span doubles as the cell x: the value must share span's register; an int local lands in v1. */
            span = (FieldCollisionSpan*)(u32)(u16)probe.x;
            first_cell_x = (s32)span;
            scratch = (u16)probe.z;
            cell_z = scratch;
            hit = 0;
            while (node != NULL)
            {
                s32 first_top;
                next_height16 = (s16)next_height;
                surface = node->surface;
                if ((node->active != 0) && ((node_height16 = nodes->height_offset >> 8, first_top = surface->top + (s16)node_height16, (first_top == 0)) ||
                                              (first_top < (next_height16 + mover->height_bias)) || (first_top < (s16)step_height)))
                {
                    node_offset_x = nodes->offset_x >> 8;
                    node_offset_z = (nodes->offset_z << 8) >> 16;
                    first_row_start = node->min_z + node_offset_z;
                    if (((s16)cell_z >= first_row_start) && ((node->max_z + node_offset_z) >= (s16)cell_z))
                    {
                        first_spans = FIELD_COLLISION_SURFACE_SPANS(surface);
                        span = (FieldCollisionSpan*)node->spans + (((s16)cell_z - first_row_start) * first_spans);
                        if ((s16)node_offset_x != 0)
                        {
                            for (scratch = first_spans - 1; scratch != -1; scratch--)
                            {
                                if (((s16)first_cell_x >= (span->min_x + (s16)node_offset_x)) && ((span->max_x + (s16)node_offset_x) >= (s16)first_cell_x))
                                {
                                    hit = 1;
                                    break;
                                }
                                span++;
                            }
                        }
                        else
                        {
                            for (scratch = first_spans - 1; scratch != -1; scratch--)
                            {
                                if (((s16)first_cell_x >= span->min_x) && (span->max_x >= (s16)first_cell_x))
                                {
                                    hit = 1;
                                    break;
                                }
                                span++;
                            }
                        }
                        if (hit != 0)
                        {
                            switch (FIELD_COLLISION_SURFACE_KIND(surface))
                            {
                            case FIELD_COLLISION_KIND_FLAT:
                                if ((surface->top + (s16)node_height16) < (s16)step_height)
                                {
                                    if (on_slope != 0)
                                    {
                                        s32 first_threshold = floor_height + FIELD_COLLISION_FLOOR_MARGIN;
                                        first_floor = surface->height0 + (s16)node_height16;
                                        if (first_threshold < first_floor)
                                        {
                                            floor_height = first_floor;
                                            mover->collision_node = node;
                                        }
                                    }
                                    else
                                    {
                                        first_floor_b = surface->height0 + (s16)node_height16;
                                        if (first_floor_b >= floor_height)
                                        {
                                            floor_height = first_floor_b;
                                            mover->collision_node = node;
                                        }
                                    }
                                }
                                break;
                            case FIELD_COLLISION_KIND_SLOPE:
                                if ((surface->top + (s16)node_height16) < (s16)step_height)
                                {
                                    scratch = func_8005DFAC(node, &probe.x);
                                    if (on_slope != 0)
                                    {
                                        if (floor_height < scratch)
                                        {
                                            floor_height = scratch;
                                            mover->collision_node = node;
                                        }
                                    }
                                    else
                                    {
                                        if (floor_height < (scratch + FIELD_COLLISION_FLOOR_MARGIN))
                                        {
                                            floor_height = scratch;
                                            mover->collision_node = node;
                                        }
                                    }
                                    on_slope = 1;
                                }
                                break;
                            }
                        }
                    }
                }
                node = node->next;
            }
        }
        standing = mover->collision_node;
        if ((standing != NULL) && (standing != FIELD_COLLISION_NODE_DETACHED))
        {
            node = standing;
            surface = node->surface;
            if (node->active != 0)
            {
                if (((surface->flags & FIELD_COLLISION_KIND_MASK) == FIELD_COLLISION_KIND_SLOPE) && (surface->height0 != surface->height1))
                {
                    probe.x = mover->move_x;
                    probe.z = mover->move_z;
                    func_80062F48(surface, &probe.x);
                    mover->move_x = probe.x;
                    result |= FIELD_COLLISION_RESULT_SLOWED;
                    mover->move_z = probe.z;
                }
                if (surface->flags & FIELD_COLLISION_SURFACE_SLOW)
                {
                    /* delta_x/delta_z carry the halving; x / 2 expands without the branch and does not match. */
                    delta_x = mover->move_x;
                    mover->move_x = (delta_x >= 0) ? (delta_x >> 1) : ((delta_x + 1) >> 1);
                    delta_z = mover->move_z;
                    mover->move_z = (delta_z >= 0) ? (delta_z >> 1) : ((delta_z + 1) >> 1);
                    result |= FIELD_COLLISION_RESULT_SLOWED;
                }
                if (surface->flags & FIELD_COLLISION_SURFACE_CARRY)
                {
                    result |= FIELD_COLLISION_RESULT_CARRIED;
                }
                if (!(mover->mode_flags & FIELD_COLLISION_MOVER_AIRBORNE) &&
                    ((node->motion_x != 0) || (node->motion_height != 0) || (node->unk2C != 0) || (node->motion_z != 0)))
                {
                    mover->move_x = mover->move_x + node->motion_x;
                    old_move_height = mover->move_height;
                    mover->move_z = mover->move_z + node->motion_z;
                    if ((surface->flags & FIELD_COLLISION_KIND_MASK) == FIELD_COLLISION_KIND_SLOPE)
                    {
                        probe.x = FIELD_COLLISION_CELL(mover->x);
                        probe.z = FIELD_COLLISION_CELL(mover->z);
                        mover->move_height = -((s32)(func_8005DFAC(node, &probe.x) << 0x10) >> 8) - mover->height;
                    }
                    else
                    {
                        mover->move_height = old_move_height + node->motion_height;
                    }
                    if (old_move_height != mover->move_height)
                    {
                        result |= FIELD_COLLISION_RESULT_CARRIED | FIELD_COLLISION_RESULT_HEIGHT;
                    }
                    else if ((node->motion_x != 0) || (node->motion_z != 0))
                    {
                        result |= FIELD_COLLISION_RESULT_CARRIED;
                    }
                }
                else
                {
                    secondary = (FieldCollisionNode*)scene->secondary_nodes;
                    if ((secondary != NULL) && (secondary != node) &&
                        ((secondary->motion_x != 0) || (secondary->motion_height != 0) || (secondary->unk2C != 0) || (secondary->motion_z != 0)))
                    {
                        old_move_height = mover->move_height;
                        mover->move_x = mover->move_x + secondary->motion_x;
                        value = mover->x;
                        mover->move_z = mover->move_z + secondary->motion_z;
                        if (value >= 0)
                        {
                            secondary_cell_x = value >> 8;
                        }
                        else
                        {
                            secondary_cell_x = (s32)(value + 0xFF) >> 8;
                        }
                        probe.x = secondary_cell_x;
                        probe.z = FIELD_COLLISION_CELL(mover->z);
                        sample = func_8005DFAC(secondary, &probe.x) - secondary->surface->height0;
                        if ((surface->flags & FIELD_COLLISION_KIND_MASK) == FIELD_COLLISION_KIND_SLOPE)
                        {
                            sample += func_8005DFAC(node, &probe.x);
                            mover->move_height = -(sample << 8) - mover->height;
                        }
                        else
                        {
                            mover->move_height = mover->move_height + (node->motion_height - (sample << 8));
                        }
                        if (old_move_height != mover->move_height)
                        {
                            result |= FIELD_COLLISION_RESULT_CARRIED | FIELD_COLLISION_RESULT_HEIGHT;
                        }
                        else if ((node->motion_x != 0) || (node->motion_z != 0))
                        {
                            result |= FIELD_COLLISION_RESULT_CARRIED;
                        }
                    }
                }
            }
        }
    }
    if ((mover->move_x == 0) && (mover->move_height == 0) && (mover->move_z == 0))
    {
        if ((nodes != NULL) && (mover->height != 0))
        {
            if (mover->flags & FIELD_COLLISION_MOVER_SETTLED)
            {
                standing_node = mover->collision_node;
                if (standing_node != FIELD_COLLISION_NODE_DETACHED)
                {
                    node = standing_node;
                    if (node != NULL)
                    {
                        surface = node->surface;
                        switch (FIELD_COLLISION_SURFACE_KIND(surface))
                        {
                        case FIELD_COLLISION_KIND_FLAT:
                            mover->resolved_height = -(node->height_offset + (surface->height0 << 8));
                            break;
                        case FIELD_COLLISION_KIND_SLOPE:
                            probe.x = FIELD_COLLISION_CELL(mover->x);
                            probe.z = FIELD_COLLISION_CELL(mover->z);
                            slope_height = func_8005DFAC(node, &probe.x);
                            secondary = (FieldCollisionNode*)scene->secondary_nodes;
                            if ((secondary != NULL) && (secondary != node))
                            {
                                slope_height += func_8005DFAC(secondary, &probe.x) - secondary->surface->height0;
                            }
                            mover->resolved_height = -(slope_height << 8);
                            break;
                        }
                    }
                    return 0;
                }
            }
        }
        else
        {
            return 0;
        }
    }
    probe.mover = mover;
    probe.w = next_height;
    probe.h = step_height;
    move_x_abs = mover->move_x;
    footprint_w = (s16)mover->footprint_width;
    if (move_x_abs < 0)
    {
        move_x_abs = -move_x_abs;
    }
    step = move_x_abs >> 8;
    if (step >= footprint_w)
    {
        step_count = (step / footprint_w) + 1;
    }
    else
    {
        step_count = 1;
    }
    move_z_abs = mover->move_z;
    footprint_d = (s16)mover->mode_flags;
    if (move_z_abs < 0)
    {
        move_z_abs = -move_z_abs;
    }
    step = move_z_abs >> 8;
    if (step >= footprint_d)
    {
        step = (step / footprint_d) + 1;
        if (step_count < step)
        {
            step_count = step;
        }
    }
    if (step_count == 1)
    {
        probe.x = FIELD_COLLISION_CELL(mover->x + mover->move_x);
        probe.z = (mover->z + mover->move_z) / 256;
        func_8005DA7C(&probe, nodes, &hit_count, &touch_count);
    }
    else
    {
        step = 0;
        do
        {
            step++;
            cell = mover->x + (mover->move_x * step / step_count);
            if (cell < 0)
            {
                cell = (cell + 0xFF) >> 8;
            }
            else
            {
                cell >>= 8;
            }
            probe.x = cell;
            probe.z = (mover->z + (mover->move_z * step / step_count)) / 256;
            func_8005DA7C(&probe, nodes, &hit_count, &touch_count);
        } while ((step_count != step) && (hit_count == 0));
    }
    hit = 0;
    if (hit_count == 0)
    {
        FieldSceneHeader* bound_header;
        s32 bound_width;
        s32 bound_x_start;
        s32 bound_z_start;
        s32 bound_z_end;
        bound_header = scene->header;
        if ((bound_header->flags & FIELD_SCENE_HEADER_BOUNDED) != 0)
        {
            bound_width = (u16)mover->footprint_width;
            span_push_z = (u16)mover->mode_flags;
            bound_x_start = (u16)probe.x - (s16)bound_width / 2;
            bound_z_start = (u16)probe.z - (s16)span_push_z / 2;
            bound_z_end = bound_z_start + span_push_z;
            value = bound_x_start + bound_width;
            if (((s16)bound_z_start < 0) || ((s16)bound_z_end >= bound_header->unk32) || ((s16)bound_x_start < 0) || ((s16)value >= bound_header->unk30))
            {
                hit = 1;
            }
        }
    }
    if ((hit_count != 0) || (hit != 0))
    {
        if (mover->move_x == 0)
        {
            move_angle = 0xC00;
            if (mover->move_z >= 0)
            {
                move_angle = 0x400;
            }
        }
        else
        {
            move_angle = ratan2(mover->move_z, mover->move_x) & 0xFFF;
        }
        step_count = abs(mover->move_x);
        major = abs(mover->move_z);
        step = major;
        if (step_count >= major)
        {
            step_count >>= 8;
        }
        else
        {
            step_count = step >> 8;
        }
        slide_angle = FIELD_COLLISION_SLIDE_NONE;
        if (step_count == 0)
        {
            step_count = 1;
        }
        step = step_count - 1;
        if (step_count != 0)
        {
            do
            {
                {
                    s32 width;
                    x_start = FIELD_COLLISION_CELL(mover->x + mover->move_x * (step_count - step) / step_count) - (s16)mover->footprint_width / 2;
                    width = (u16)mover->footprint_width;
                    x_end = x_start + width;
                    scratch = FIELD_COLLISION_CELL(mover->z + mover->move_z * (step_count - step) / step_count) - (s16)mover->mode_flags / 2;
                }
                header = scene->header;
                /* z_end_raw is shared with the push pass; per-site temps reallocate. */
                z_end_raw = scratch + (u16)mover->mode_flags;
                z_end = z_end_raw;
                if (header->flags & FIELD_SCENE_HEADER_BOUNDED)
                {
                    if (((s16)scratch < 0) || (z_end >= header->unk32))
                    {
                        slide_angle = func_8005E1A8(NULL, FIELD_COLLISION_EDGE_WALL_X, move_angle, slide_angle);
                    }
                    if (((s16)x_start < 0) || (x_end >= scene->header->unk30))
                    {
                        slide_angle = func_8005E1A8(NULL, FIELD_COLLISION_EDGE_WALL_Z, move_angle, slide_angle);
                    }
                }
                hit_iter = FIELD_COLLISION_HIT_LIST;
                touch_iter = FIELD_COLLISION_TOUCH_LIST;
                remaining = hit_count;
                remaining -= 1;
                touch_count = 0;
                if (remaining != -1)
                {
                    box_x_start = (s16)x_start;
                    box_z_start = (s16)scratch;
                    box_z_end = z_end;
                    box_z_last = z_end - 1;
                    box_x_end = x_end;
                    box_x_last = x_end - 1;
                    do
                    {
                        node = *hit_iter;
                        hit_iter += 1;
                        hit_offset_z = nodes->offset_z >> 8;
                        hit_offset_x = (nodes->offset_x << 8) >> 16;
                        surface = node->surface;
                        if (((node->min_x + hit_offset_x) < box_x_end) && (((node->max_x + hit_offset_x) >= box_x_start)))
                        {
                            work = node->min_z + (s16)hit_offset_z;
                            if (work < box_z_end)
                            {
                                hit_row_end = node->max_z + (s16)hit_offset_z;
                                if (hit_row_end >= box_z_start)
                                {
                                    row = work;
                                    if (box_z_last < hit_row_end)
                                    {
                                        scratch = box_z_last;
                                    }
                                    else
                                    {
                                        scratch = hit_row_end;
                                    }
                                    if (row < box_z_start)
                                    {
                                        row = box_z_start;
                                    }
                                    hit_spans = FIELD_COLLISION_SURFACE_SPANS(surface);
                                    row_skip = (row - work) * hit_spans;
                                    scratch = ((scratch - row) + 1) * hit_spans;
                                    hit = 0;
                                    span = (FieldCollisionSpan*)node->spans + (row_skip);
                                    left_flags = (u8*)node->span_flags + (row_skip * 2);
                                    if (surface->flags & FIELD_COLLISION_SURFACE_EDGE_FLAGS)
                                    {
                                        scratch = scratch - 1;
                                        right_flags = left_flags + 1;
                                        if (scratch != -1)
                                        {
                                            do
                                            {
                                                s32 span_flag;
                                                span_flag = span->min_x;
                                                flagged_min_x = span_flag;
                                                if ((flagged_min_x < box_x_end) && (span->max_x >= box_x_start))
                                                {
                                                    if ((box_x_start < flagged_min_x) && (*(s8*)left_flags >= 0))
                                                    {
                                                        slide_angle = func_8005E1A8(surface, (u8)*left_flags & FIELD_COLLISION_EDGE_INDEX, move_angle, slide_angle);
                                                        hit = 2;
                                                    }
                                                    if ((span->max_x < box_x_last) && (*right_flags >= 0))
                                                    {
                                                        slide_angle = func_8005E1A8(surface, (u8)*right_flags & FIELD_COLLISION_EDGE_INDEX, move_angle, slide_angle);
                                                        hit = 2;
                                                    }
                                                    {
                                                        u8 left_flag = *left_flags;
                                                        if ((s8)left_flag >= 0)
                                                        {
                                                            u8 right_flag = *right_flags;
                                                            if ((s8)right_flag >= 0)
                                                            {
                                                                span_flag = FIELD_COLLISION_EDGE_WALL_X;
                                                                if (((left_flag == span_flag) || (right_flag == span_flag)) && (span->min_x < box_x_start) &&
                                                                    (span->max_x >= box_x_end))
                                                                {
                                                                    slide_angle = func_8005E1A8(surface, FIELD_COLLISION_EDGE_WALL_X, move_angle, slide_angle);
                                                                    hit = 2;
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                                span++;
                                                right_flags += 2;
                                                scratch -= 1;
                                                left_flags += 2;
                                            } while (scratch != -1);
                                        }
                                    }
                                    else if (hit_offset_x != 0)
                                    {
                                        s32 scan_bias;
                                        scan_bias = hit_offset_x;
                                        scratch = scratch - 1;
                                        if (scratch != -1)
                                        {
                                            do
                                            {
                                                offset_min_x = span->min_x + scan_bias;
                                                if ((offset_min_x < box_x_end) && ((span->max_x + scan_bias) >= box_x_start))
                                                {
                                                    hit = 1;
                                                    if (box_x_start < offset_min_x)
                                                    {
                                                        slide_angle = func_8005E1A8(surface, left_flags[0] & FIELD_COLLISION_EDGE_INDEX, move_angle, slide_angle);
                                                    }
                                                    if ((span->max_x + scan_bias) < box_x_last)
                                                    {
                                                        slide_angle = func_8005E1A8(surface, left_flags[1] & FIELD_COLLISION_EDGE_INDEX, move_angle, slide_angle);
                                                    }
                                                    if (((span->min_x + scan_bias) < box_x_start) && ((span->max_x + scan_bias) >= box_x_end))
                                                    {
                                                        slide_angle = func_8005E1A8(surface, FIELD_COLLISION_EDGE_WALL_X, move_angle, slide_angle);
                                                    }
                                                }
                                                span++;
                                                scratch -= 1;
                                                left_flags += 2;
                                            } while (scratch != -1);
                                        }
                                    }
                                    else
                                    {
                                        scratch = scratch - 1;
                                        if (scratch != -1)
                                        {
                                            do
                                            {
                                                plain_min_x = span->min_x;
                                                if ((plain_min_x < box_x_end) && (span->max_x >= box_x_start))
                                                {
                                                    hit = 1;
                                                    if (box_x_start < plain_min_x)
                                                    {
                                                        slide_angle = func_8005E1A8(surface, left_flags[0] & FIELD_COLLISION_EDGE_INDEX, move_angle, slide_angle);
                                                    }
                                                    if (span->max_x < box_x_last)
                                                    {
                                                        slide_angle = func_8005E1A8(surface, left_flags[1] & FIELD_COLLISION_EDGE_INDEX, move_angle, slide_angle);
                                                    }
                                                    if ((span->min_x < box_x_start) && (span->max_x >= box_x_end))
                                                    {
                                                        slide_angle = func_8005E1A8(surface, FIELD_COLLISION_EDGE_WALL_X, move_angle, slide_angle);
                                                    }
                                                }
                                                span++;
                                                scratch -= 1;
                                                left_flags += 2;
                                            } while (scratch != -1);
                                        }
                                    }
                                    if (hit != 0)
                                    {
                                        *touch_iter = node;
                                        touch_iter += 1;
                                        touch_count += 1;
                                    }
                                }
                            }
                        }
                        remaining -= 1;
                    } while (remaining != -1);
                }
                if (slide_angle != FIELD_COLLISION_SLIDE_NONE)
                {
                    break;
                }
            } while (--step != -1);
        }
        value = mover->move_x;
        steps_taken = step_count - (step + 1);
        delta_x = value * steps_taken / step_count;
        move_z_copy = mover->move_z;
        slide_z = move_z_copy * steps_taken / step_count;
        delta_z = slide_z;
        if ((slide_angle >= 0) && (delta_x == 0) && (slide_z == 0))
        {
            sample = SquareRoot0((value * value) + (move_z_copy * move_z_copy));
            if ((sample * rcos(slide_angle)) >= 0)
            {
                delta_x = (s32)(sample * rcos(slide_angle)) >> 0xC;
            }
            else
            {
                delta_x = (s32)((sample * rcos(slide_angle)) + 0xFFF) >> 0xC;
            }
            if ((sample * rsin(slide_angle)) >= 0)
            {
                delta_z = (s32)(sample * rsin(slide_angle)) >> 0xC;
            }
            else
            {
                delta_z = (s32)((sample * rsin(slide_angle)) + 0xFFF) >> 0xC;
            }
            hit_count = 1;
            half_width = (s16)(u16)mover->footprint_width / 2;
            do
            {
                {
                    s32 width;
                    s32 cx = mover->x + delta_x;
                    if (cx >= 0)
                    {
                        x_start = ((cx >> 8) - half_width);
                    }
                    else
                    {
                        x_start = (((cx + 0xFF) >> 8) - half_width);
                    }
                    width = (u16)mover->footprint_width;
                    x_end = (s16)x_start + width;
                    scratch = FIELD_COLLISION_CELL(mover->z + delta_z) - (s16)mover->mode_flags / 2;
                }
                touch_iter = FIELD_COLLISION_TOUCH_LIST;
                hit = 0;
                push_x = 0;
                step_count = touch_count;
                depth = (u16)mover->mode_flags;
                push_z = 0;
                blocked_marker = FIELD_COLLISION_PUSH_BLOCKED;
                header = scene->header;
                z_end_raw = scratch + (u16)depth;
                z_end = z_end_raw;
                if (header->flags & FIELD_SCENE_HEADER_BOUNDED)
                {
                    if ((s16)scratch < 0)
                    {
                        push_z = -(s16)scratch;
                        hit = 1;
                    }
                    else
                    {
                        extent_z = header->unk32;
                        if (z_end >= extent_z)
                        {
                            push_z = (extent_z - z_end) - 1;
                            hit = 1;
                        }
                    }
                    if ((s16)x_start < 0)
                    {
                        push_x = -(s16)x_start;
                        hit = 1;
                    }
                    else
                    {
                        extent_x = scene->header->unk30;
                        if (x_end >= extent_x)
                        {
                            push_x = extent_x - x_end - 1;
                            hit = 1;
                        }
                    }
                }
                step_count -= 1;
                if (step_count != -1)
                {
                    push_x_end = x_end;
                    push_x_start = (s16)x_start;
                    push_z_start = (s16)scratch;
                    push_z_end = z_end;
                    push_z_last = push_z_end - 1;
                    nodes_offset_x_raw = nodes->offset_x >> 8;
                    nodes_offset_x = (u16)nodes_offset_x_raw;
                    push_offset_x = (s16)nodes_offset_x_raw;
                    push_offset_z = (nodes->offset_z << 8) >> 16;
                    do
                    {
                        node = *touch_iter;
                        touch_iter += 1;
                        surface = node->surface;
                        if (((node->min_x + push_offset_x) < push_x_end) && (((node->max_x + push_offset_x) >= push_x_start)))
                        {
                            row_start = node->min_z + push_offset_z;
                            if (row_start < push_z_end)
                            {
                                row_end = node->max_z + push_offset_z;
                                if (row_end >= push_z_start)
                                {
                                    scratch = row_end;
                                    if (push_z_last < row_end)
                                    {
                                        scratch = push_z_last;
                                    }
                                    row = row_start;
                                    if (row < push_z_start)
                                    {
                                        row = push_z_start;
                                    }
                                    push_row_skip = (row - row_start) * FIELD_COLLISION_SURFACE_SPANS(surface);
                                    scratch = scratch - row;
                                    span = (FieldCollisionSpan*)node->spans + (push_row_skip);
                                    left_flags = (u8*)node->span_flags + (push_row_skip * 2);
                                    if (scratch != -1)
                                    {
                                        limit = push_x_end - 1;
                                        do
                                        {
                                            remaining = FIELD_COLLISION_SURFACE_SPANS(surface);
                                            remaining -= 1;
                                            if (remaining != -1)
                                            {
                                                span_offset_x = (s16)nodes_offset_x;
                                                next_row = row + 1;
                                                rows_past_end = next_row - push_z_end;
                                                do
                                                {
                                                    push_min_x = span->min_x;
                                                    if (push_min_x < push_x_end)
                                                    {
                                                        span_max_x = span->max_x;
                                                        if (span_max_x >= push_x_start)
                                                        {
                                                            span_hit = 0;
                                                            if (surface->flags & FIELD_COLLISION_SURFACE_EDGE_FLAGS)
                                                            {
                                                                span_push_x = 0;
                                                                if ((push_x_start < push_min_x) && !(*left_flags & FIELD_COLLISION_EDGE_OPEN))
                                                                {
                                                                    span_hit = 1;
                                                                    if (span_max_x >= push_x_end)
                                                                    {
                                                                        span_push_x = push_min_x - push_x_end;
                                                                    }
                                                                }
                                                                else
                                                                {
                                                                    push_max_x = span->max_x;
                                                                    if ((push_max_x < limit) && !(left_flags[1] & FIELD_COLLISION_EDGE_OPEN))
                                                                    {
                                                                        if (span->min_x < push_x_start)
                                                                        {
                                                                            span_push_x = (push_max_x - push_x_start) + 1;
                                                                        }
                                                                        span_hit = 1;
                                                                    }
                                                                }
                                                                if (!(*left_flags & FIELD_COLLISION_EDGE_OPEN) && !(left_flags[1] & FIELD_COLLISION_EDGE_OPEN))
                                                                {
                                                                    span_hit = 1;
                                                                }
                                                            }
                                                            else
                                                            {
                                                                span_hit = 1;
                                                                shifted_min_x = push_min_x + span_offset_x;
                                                                span_push_x = 0;
                                                                if ((push_x_start < shifted_min_x) && ((span_max_x + span_offset_x) >= push_x_end))
                                                                {
                                                                    span_push_x = shifted_min_x - push_x_end;
                                                                }
                                                                else if ((span->min_x + span_offset_x) < push_x_start)
                                                                {
                                                                    shifted_max_x = span->max_x + span_offset_x;
                                                                    if (shifted_max_x < limit)
                                                                    {
                                                                        span_push_x = (shifted_max_x - push_x_start) + 1;
                                                                    }
                                                                }
                                                            }
                                                            row_from_start = row - push_z_start;
                                                            if (span_hit != 0)
                                                            {
                                                                hit = 1;
                                                                if ((push_z_end - next_row) >= row_from_start)
                                                                {
                                                                    span_push_z = row_from_start + 1;
                                                                }
                                                                else
                                                                {
                                                                    span_push_z = rows_past_end - 1;
                                                                }
                                                                if ((push_x != 0) || (push_z != 0))
                                                                {
                                                                    if ((push_x != blocked_marker) && (span_push_x != 0))
                                                                    {
                                                                        if (span_push_x > 0)
                                                                        {
                                                                            if (push_x >= 0)
                                                                            {
                                                                                if (push_x < span_push_x)
                                                                                {
                                                                                    push_x = span_push_x;
                                                                                }
                                                                            }
                                                                            else
                                                                            {
                                                                                push_x = FIELD_COLLISION_PUSH_BLOCKED;
                                                                            }
                                                                        }
                                                                        else if (push_x <= 0)
                                                                        {
                                                                            if (span_push_x < push_x)
                                                                            {
                                                                                push_x = span_push_x;
                                                                            }
                                                                        }
                                                                        else
                                                                        {
                                                                            push_x = FIELD_COLLISION_PUSH_BLOCKED;
                                                                        }
                                                                    }
                                                                    if (push_z != blocked_marker)
                                                                    {
                                                                        if (span_push_z > 0)
                                                                        {
                                                                            if (push_z >= 0)
                                                                            {
                                                                                if (push_z < span_push_z)
                                                                                {
                                                                                    push_z = span_push_z;
                                                                                }
                                                                            }
                                                                            else
                                                                            {
                                                                                push_z = blocked_marker;
                                                                            }
                                                                        }
                                                                        else if (push_z <= 0)
                                                                        {
                                                                            if (span_push_z < push_z)
                                                                            {
                                                                                push_z = span_push_z;
                                                                            }
                                                                        }
                                                                        else
                                                                        {
                                                                            push_z = FIELD_COLLISION_PUSH_BLOCKED;
                                                                        }
                                                                    }
                                                                }
                                                                else
                                                                {
                                                                    push_x = span_push_x;
                                                                    push_z = span_push_z;
                                                                }
                                                            }
                                                        }
                                                    }
                                                    span++;
                                                    left_flags += 2;
                                                    remaining -= 1;
                                                } while (remaining != -1);
                                            }
                                            scratch -= 1;
                                            row += 1;
                                        } while (scratch != -1);
                                    }
                                }
                            }
                        }
                        step_count -= 1;
                    } while (step_count != -1);
                }
                if (hit == 0)
                {
                    break;
                }
                scaled = push_x << 8;
                if (hit_count == 1)
                {
                    span_push_z = push_z << 8;
                    if ((push_x == 0) || (push_x == blocked_marker))
                    {
                        if (push_z != blocked_marker)
                        {
                            delta_z += span_push_z;
                        }
                        break;
                    }
                    else if ((push_z == 0) || (push_z == blocked_marker))
                    {
                        delta_x += scaled;
                        break;
                    }
                    else
                    {
                        push_x_abs = abs(scaled);
                        push_z_abs = abs(span_push_z);
                        if (push_z_abs < push_x_abs)
                        {
                            carry_x = scaled;
                            carry_z = 0;
                            delta_z += span_push_z;
                        }
                        else
                        {
                            carry_z = span_push_z;
                            carry_x = 0;
                            delta_x += scaled;
                        }
                    }
                }
                else
                {
                    delta_x += carry_x;
                    delta_z += carry_z;
                    break;
                }
            } while (--hit_count != -1);
            result |= FIELD_COLLISION_RESULT_SLID;
        }
        else
        {
            result |= FIELD_COLLISION_RESULT_BLOCKED;
        }
        if ((delta_x != 0) || (delta_z != 0))
        {
            probe.x = FIELD_COLLISION_CELL(mover->x + delta_x);
            probe.z = FIELD_COLLISION_CELL(mover->z + delta_z);
            func_8005DA7C(&probe, nodes, &hit_count, &touch_count);
            hit = 0;
            if (hit_count == 0)
            {
                s32 retry_z_start;
                s32 footprint_width;
                s32 retry_z_end;
                retry_header = scene->header;
                if (retry_header->flags & FIELD_SCENE_HEADER_BOUNDED)
                {
                    footprint_width = (u16)mover->footprint_width;
                    span_push_z = (u16)mover->mode_flags;
                    retry_x_start = (u16)probe.x - (s16)footprint_width / 2;
                    retry_z_start = (u16)probe.z - (s16)span_push_z / 2;
                    retry_z_end = retry_z_start + span_push_z;
                    scaled = retry_x_start + footprint_width;
                    if (((s16)retry_z_start < 0) || ((s16)retry_z_end >= retry_header->unk32) || ((s16)retry_x_start < 0) || ((s16)scaled >= retry_header->unk30))
                    {
                        hit = 1;
                    }
                }
            }
            if ((hit_count != 0) || (hit != 0))
            {
                probe.x = FIELD_COLLISION_CELL(mover->x);
                probe.z = FIELD_COLLISION_CELL(mover->z);
                func_8005DA7C(&probe, nodes, &hit_count, &touch_count);
                result |= FIELD_COLLISION_RESULT_BLOCKED | FIELD_COLLISION_RESULT_SLID;
            }
            else
            {
                mover->x = mover->x + delta_x;
                mover->z = mover->z + delta_z;
            }
        }
        else
        {
            probe.x = FIELD_COLLISION_CELL(mover->x);
            probe.z = FIELD_COLLISION_CELL(mover->z);
            func_8005DA7C(&probe, nodes, &hit_count, &touch_count);
            result |= FIELD_COLLISION_RESULT_BLOCKED | FIELD_COLLISION_RESULT_SLID;
        }
    }
    else
    {
        mover->x = mover->x + mover->move_x;
        mover->z = mover->z + mover->move_z;
    }
    limit = 0xFFFFFF;
    floor_height = 0;
    {
        floor_fixed = 0;
        on_slope = 0;
        ground_x = mover->x;
        node = NULL;
        mover->collision_node = NULL;
    }
    if (ground_x < 0)
    {
        ground_x = (ground_x + 0xFF) >> 8;
    }
    else
    {
        ground_x >>= 8;
    }
    probe.x = ground_x;
    probe.z = FIELD_COLLISION_CELL(mover->z);
    /* span doubles as the cell x: the value must share span's register; an int local lands in v1. */
    span = (FieldCollisionSpan*)(u32)(u16)probe.x;
    cell_x = (s32)span;
    scratch = (u16)probe.z;
    ground_z = scratch;
    touch_iter = FIELD_COLLISION_TOUCH_LIST;
    secondary = (FieldCollisionNode*)scene->secondary_nodes;
    touch_count -= 1;
    if (touch_count != -1)
    {
        ground_cell_z = (s16)ground_z;
        step_limit = (s16)step_height;
        do
        {
            touch_node = *touch_iter;
            nodes = touch_node;
            touch_cell_x = (s16)cell_x;
            touch_iter += 1;
            surface = touch_node->surface;
            touch_offset_x = touch_node->offset_x >> 8;
            touch_offset_z = (touch_node->offset_z << 8) >> 16;
            touch_row_start = touch_node->min_z + touch_offset_z;
            hit = 0;
            if ((ground_cell_z >= touch_row_start) && (((touch_node->max_z + touch_offset_z) >= ground_cell_z)))
            {
                touch_spans = FIELD_COLLISION_SURFACE_SPANS(surface);
                span = (FieldCollisionSpan*)touch_node->spans + ((ground_cell_z - touch_row_start) * touch_spans);
                if ((s16)touch_offset_x != 0)
                {
                    for (scratch = touch_spans - 1; scratch != -1; scratch--)
                    {
                        if ((touch_cell_x >= (span->min_x + (s16)touch_offset_x)) && ((span->max_x + (s16)touch_offset_x) >= touch_cell_x))
                        {
                            hit = 1;
                            break;
                        }
                        span++;
                    }
                }
                else
                {
                    for (scratch = touch_spans - 1; scratch != -1; scratch--)
                    {
                        if ((touch_cell_x >= span->min_x) && (span->max_x >= touch_cell_x))
                        {
                            hit = 1;
                            break;
                        }
                        span++;
                    }
                }
            }
            touch_height_raw = nodes->height_offset;
            {
                s32 node_height = touch_height_raw >> 8;
                touch_kind = FIELD_COLLISION_SURFACE_KIND(surface);
                touch_height = node_height;
            }
            switch (touch_kind)
            {
            case FIELD_COLLISION_KIND_FLAT:
                node_top = surface->top + (s16)touch_height;
                if (node_top < step_limit)
                {
                    if (hit != 0)
                    {
                        floor_fixed_candidate = -(touch_height_raw + (surface->height0 << 8));
                        if (floor_fixed_candidate < mover->resolved_height)
                        {
                            mover->resolved_height = floor_fixed_candidate;
                            node = nodes;
                        }
                    }
                    if (on_slope != 0)
                    {
                        s32 threshold = floor_height + FIELD_COLLISION_FLOOR_MARGIN;
                        floor_candidate = surface->height0 + (s16)touch_height;
                        if (threshold < floor_candidate)
                        {
                            floor_height = floor_candidate;
                            {
                                floor_fixed = nodes->height_offset + (surface->height0 << 8);
                                if (hit != 0)
                                {
                                    mover->collision_node = nodes;
                                }
                            }
                        }
                    }
                    else
                    {
                        floor_candidate_b = surface->height0 + (s16)touch_height;
                        if (floor_candidate_b >= floor_height)
                        {
                            floor_height = floor_candidate_b;
                            {
                                floor_fixed = nodes->height_offset + (surface->height0 << 8);
                                if (hit != 0)
                                {
                                    mover->collision_node = nodes;
                                }
                            }
                        }
                    }
                }
                else
                {
                    if (node_top < limit)
                    {
                        limit = node_top;
                    }
                }
                break;
            case FIELD_COLLISION_KIND_SLOPE:
                node_top = surface->top + (s16)touch_height;
                if (node_top < step_limit)
                {
                    scratch = func_8005DFAC(nodes, &probe.x);
                    if (hit != 0)
                    {
                        slope_floor_fixed = -(scratch << 8);
                        if (slope_floor_fixed < mover->resolved_height)
                        {
                            mover->resolved_height = slope_floor_fixed;
                            node = nodes;
                        }
                    }
                    if (on_slope != 0)
                    {
                        higher = floor_height < scratch;
                        if (secondary != NULL)
                        {
                            if (secondary == nodes)
                            {
                                secondary_rise = scratch - nodes->surface->height0;
                                floor_height += secondary_rise;
                                                            {
                                    floor_fixed += secondary_rise << 8;
                                }
                            }
                            else
                            {
                                floor_height = (scratch + floor_height) - secondary->surface->height0;
                                floor_fixed = floor_height << 8;
                                if (hit != 0)
                                {
                                    mover->collision_node = nodes;
                                }
                            }
                            mover->resolved_height = -(floor_height << 8);
                            node = nodes;
                        }
                        else
                        {
                            if (higher != 0)
                            {
                                floor_height = scratch;
                                floor_fixed = floor_height << 8;
                                if (hit != 0)
                                {
                                    mover->collision_node = nodes;
                                }
                            }
                        }
                    }
                    else
                    {
                        higher = floor_height < (scratch + FIELD_COLLISION_FLOOR_MARGIN);
                        if (higher != 0)
                        {
                            floor_height = scratch;
                            floor_fixed = floor_height << 8;
                            if (hit != 0)
                            {
                                mover->collision_node = nodes;
                            }
                        }
                    }
                    on_slope = 1;
                }
                else
                {
                    if (node_top < limit)
                    {
                        limit = node_top;
                    }
                }
                break;
            }
        } while (--touch_count != -1);
    }
    if (mover->mode_flags & FIELD_COLLISION_MOVER_AIRBORNE)
    {
        bias = mover->height_bias;
        if (limit < ((s16)next_height + bias))
        {
            mover->height = -((limit - bias) << 8);
            result |= FIELD_COLLISION_RESULT_CEILING;
        }
        else if (floor_height >= (s16)next_height)
        {
            mover->height = -floor_fixed;
            result |= FIELD_COLLISION_RESULT_LANDED;
        }
        else
        {
            mover->height = mover->height + mover->move_height;
        }
    }
    else if (((s16)next_height != floor_height) || (floor_fixed != 0))
    {
        mover->height = -floor_fixed;
        result |= FIELD_COLLISION_RESULT_HEIGHT;
    }
    if (node == mover->collision_node)
    {
        mover->flags = mover->flags | FIELD_COLLISION_MOVER_SETTLED;
    }
    else
    {
        mover->flags = mover->flags & (u16)~FIELD_COLLISION_MOVER_SETTLED;
    }
    return result;
}

/**
 * @brief Classify collision nodes against a mover probe and collect blocking and touching nodes.
 *
 * A node takes part when it is active, its top lies above the probe's height
 * window, and its bounds overlap the footprint. Its span rows under the
 * footprint are then scanned: any overlap touches the node, and with
 * per-span edge bytes a closed edge inside the footprint blocks it. A touched
 * node whose floor rises more than the mover can step (nothing for an
 * airborne mover, FIELD_COLLISION_STEP_UP otherwise) blocks instead.
 *
 * @param probe Probe position, vertical extents, and owning mover.
 * @param node Head of the collision node chain.
 * @param out_hit Receives the number of blocking nodes written to FIELD_COLLISION_HIT_LIST.
 * @param out_touch Receives the number of touching nodes written to FIELD_COLLISION_TOUCH_LIST.
 */
void func_8005DA7C(FieldCollisionMoveProbe* probe, FieldCollisionNode* node, s32* out_hit, s32* out_touch)
{
    FieldCollisionNode** hit_list;
    FieldCollisionNode** touch_list;
    FieldCollisionSurfaceDef* surface;
    FieldCollisionMover* mover;
    FieldCollisionSpan* span;
    u8* edge_flags;
    s16 feet;
    s16 step_limit;
    s16 z_start;
    s16 z_end;
    s16 x_start;
    s16 x_end;
    s32 row_first;
    s32 row_from;
    s32 row_last;
    s32 z_limit;
    s32 shift_x;
    s32 shift_z;
    s16 shift_height;
    s32 value;
    s32 offset;
    s32 count;
    s32 result;
    s32 ground;
    s8 left_edge;
    s32 z_from;
    u8 spans_per_row;
    u16 depth;
    u16 width;

    *out_hit = 0;
    *out_touch = 0;
    if (node != NULL)
    {
        hit_list = FIELD_COLLISION_HIT_LIST;
        touch_list = FIELD_COLLISION_TOUCH_LIST;
        step_limit = probe->h;
        mover = probe->mover;
        feet = probe->w;
        depth = mover->mode_flags;
        z_start = probe->z - (s16)depth / 2;
        z_end = z_start + depth;
        width = mover->footprint_width;
        x_start = probe->x - (s16)width / 2;
        x_end = x_start + width;
        do
        {
            surface = node->surface;
            if (node->active != 0)
            {
                shift_height = node->height_offset >> 8;
                if (surface->top + shift_height == 0 || (value = surface->top + shift_height) < feet + mover->height_bias || value < step_limit)
                {
                    shift_z = node->offset_z >> 8;
                    shift_x = (s32)(node->offset_x << 8) >> 16;
                    if (((node->min_x + shift_x) < x_end) && ((node->max_x + shift_x) >= x_start))
                    {
                        /* Widened copies of the s16 bounds; using z_end/z_start directly reallocates (97.8%/99.2%). */
                        z_limit = z_end;
                        row_first = node->min_z + (s16)shift_z;
                        if (row_first < z_limit)
                        {
                            z_from = z_start;
                            row_last = node->max_z + (s16)shift_z;
                            if (row_last >= z_from)
                            {
                                count = row_last;
                                if ((z_limit - 1) < count)
                                {
                                    count = z_limit - 1;
                                }
                                if (row_first < z_from)
                                {
                                    row_from = z_from;
                                }
                                else
                                {
                                    row_from = row_first;
                                }
                                spans_per_row = FIELD_COLLISION_SURFACE_SPANS(surface);
                                offset = (row_from - row_first) * spans_per_row;
                                count = ((count - row_from) + 1) * spans_per_row;
                                result = 0;
                                span = (FieldCollisionSpan*)node->spans + offset;
                                if (surface->flags & FIELD_COLLISION_SURFACE_EDGE_FLAGS)
                                {
                                    edge_flags = (u8*)node->span_flags + offset * 2;
                                    while (--count != -1)
                                    {
                                        if ((span->min_x < x_end) && (span->max_x >= x_start))
                                        {
                                            left_edge = (s8)edge_flags[0];
                                            if ((left_edge >= 0) && (x_start < span->min_x))
                                            {
                                                result = FIELD_COLLISION_CLASS_BLOCK;
                                                break;
                                            }
                                            if (((s8)edge_flags[1] >= 0) && (span->max_x < x_end - 1))
                                            {
                                                result = FIELD_COLLISION_CLASS_BLOCK;
                                                break;
                                            }
                                            if ((left_edge >= 0) && ((s8)edge_flags[1] >= 0))
                                            {
                                                result = FIELD_COLLISION_CLASS_BLOCK;
                                                if (edge_flags[0] == FIELD_COLLISION_EDGE_WALL_X)
                                                {
                                                    break;
                                                }
                                                if (edge_flags[1] == FIELD_COLLISION_EDGE_WALL_X)
                                                {
                                                    break;
                                                }
                                            }
                                            result = FIELD_COLLISION_CLASS_TOUCH;
                                        }
                                        span++;
                                        edge_flags += 2;
                                    }
                                }
                                else if (shift_x != 0)
                                {
                                    while (--count != -1)
                                    {
                                        /* value doubles as the shift copy; shift_x directly is 99.76%. */
                                        value = shift_x;
                                        if (((span->min_x + value) < x_end) && ((span->max_x + value) >= x_start))
                                        {
                                            result = FIELD_COLLISION_CLASS_TOUCH;
                                            break;
                                        }
                                        span++;
                                    }
                                }
                                else
                                {
                                    while (--count != -1)
                                    {
                                        if ((span->min_x < x_end) && (span->max_x >= x_start))
                                        {
                                            result = FIELD_COLLISION_CLASS_TOUCH;
                                            break;
                                        }
                                        span++;
                                    }
                                }
                                if (result != 0)
                                {
                                    switch (FIELD_COLLISION_SURFACE_KIND(surface))
                                    {
                                    case FIELD_COLLISION_KIND_FLAT:
                                        if ((surface->top + shift_height) < step_limit)
                                        {
                                            if (surface->flags & FIELD_COLLISION_SURFACE_SOLID)
                                            {
                                                result = FIELD_COLLISION_CLASS_BLOCK;
                                            }
                                            else
                                            {
                                                if (mover->mode_flags & FIELD_COLLISION_MOVER_AIRBORNE)
                                                {
                                                    if (feet < (surface->height0 + shift_height))
                                                    {
                                                        result = FIELD_COLLISION_CLASS_BLOCK;
                                                    }
                                                    else
                                                    {
                                                        result |= FIELD_COLLISION_CLASS_TOUCH;
                                                    }
                                                }
                                                else
                                                {
                                                    if ((feet + FIELD_COLLISION_STEP_UP) < (surface->height0 + shift_height))
                                                    {
                                                        result = FIELD_COLLISION_CLASS_BLOCK;
                                                    }
                                                    else
                                                    {
                                                        result |= FIELD_COLLISION_CLASS_TOUCH;
                                                    }
                                                }
                                            }
                                        }
                                        else
                                        {
                                            result = FIELD_COLLISION_CLASS_TOUCH;
                                        }
                                        break;
                                    case FIELD_COLLISION_KIND_SLOPE:
                                        if ((surface->top + shift_height) < step_limit)
                                        {
                                            ground = func_8005DFAC(node, &probe->x);
                                            if (mover->mode_flags & FIELD_COLLISION_MOVER_AIRBORNE)
                                            {
                                                if (feet < ground)
                                                {
                                                    result = FIELD_COLLISION_CLASS_BLOCK;
                                                }
                                                else
                                                {
                                                    result |= FIELD_COLLISION_CLASS_TOUCH;
                                                }
                                            }
                                            else
                                            {
                                                if ((feet + FIELD_COLLISION_STEP_UP) < ground)
                                                {
                                                    result = FIELD_COLLISION_CLASS_BLOCK;
                                                }
                                                else
                                                {
                                                    result |= FIELD_COLLISION_CLASS_TOUCH;
                                                }
                                            }
                                        }
                                        else
                                        {
                                            result = FIELD_COLLISION_CLASS_TOUCH;
                                        }
                                        break;
                                    }
                                    if (result & FIELD_COLLISION_CLASS_BLOCK)
                                    {
                                        *hit_list = node;
                                        hit_list++;
                                        *out_hit += 1;
                                    }
                                    if (result & FIELD_COLLISION_CLASS_TOUCH)
                                    {
                                        *touch_list = node;
                                        touch_list++;
                                        *out_touch += 1;
                                    }
                                }
                            }
                        }
                    }
                }
            }
            node = node->next;
        } while (node != NULL);
    }
}

/**
 * @brief Sample a collision node's surface height at a probe position.
 *
 * The node's definition names three boundary points by index into
 * g_field_node_angle_table (vertex_a, vertex_b and vertex_c, called A, B and C
 * here); each table entry is an x/y pair. The node's offset_x and offset_z,
 * both divided by 256, translate B and C into world space.
 *
 * The height is found by intersecting the C->B edge with the line that runs
 * through @p position parallel to A->B: hit_x is that intersection's x
 * coordinate, edge_offset its distance from C along the edge, and
 * edge_offset / dx_bc the fraction used to blend height0 (at C) and height1
 * (at B).
 *
 * The fraction is normally taken from the x axis. When the intersection lands
 * exactly on B's x coordinate that axis carries no usable span, so the z axis
 * is used instead; if the intersection is B itself the answer is height1 directly.
 * The blend is finally clamped back between the two endpoint heights, which is
 * spelled twice because which endpoint is the upper bound depends on their
 * order.
 *
 * @param node Collision node whose surface is being sampled.
 * @param position Probe position in grid cells: position[0] is x and position[1] is z.
 * @return Interpolated surface height, clamped between the node's endpoints.
 *
 * @note Both endpoints are clamped up to zero before use, so a node whose
 *       endpoint resolves below the origin behaves as if it sat on it.
 */
s16 func_8005DFAC(FieldCollisionNode* node, s32* position)
{
    FieldCollisionSurfaceDef* surface;
    s16* vertices;
    s16* vertex;
    s16* vertex_b;
    s32 height0;
    s32 height1;
    s32 offset_x;
    s32 offset_z;
    s32 ax;
    s32 ay;
    s32 bx;
    s32 by;
    s32 cx;
    s32 cy;
    s32 dx_ab;
    s32 dy_bc;
    s32 dx_bc;
    s32 cross_xy;
    s32 cross_yx;
    s32 cx_world;
    s32 cy_world;
    s32 hit_x;
    s32 edge_offset;
    s32 hit_y;
    s16 sampled_height;

    surface = node->surface;
    vertices = g_field_node_angle_table;
    vertex = &vertices[surface->vertex_a * 2];
    height0 = (node->height_offset >> 8) + surface->height0;
    height1 = (node->far_height_offset >> 8) + surface->height1;
    vertex_b = &vertices[surface->vertex_b * 2];
    offset_x = node->offset_x >> 8;
    if (height0 < 0)
    {
        height0 = 0;
    }
    if (height1 < 0)
    {
        height1 = 0;
    }
    ax = vertex[0];
    ay = vertex[1];
    bx = vertex_b[0];
    by = vertex_b[1];
    vertex = &vertices[surface->vertex_c * 2];
    cy = vertex[1];
    dx_ab = ax - bx;
    dy_bc = by - cy;
    cross_xy = dx_ab * dy_bc;
    offset_z = node->offset_z >> 8;
    cy_world = cy + offset_z;
    cx = vertex[0];
    dx_bc = bx - cx;
    cross_yx = (ay - by) * dx_bc;
    cx_world = cx + offset_x;
    hit_x = ((((position[1] - cy_world) * dx_ab) * dx_bc) + (cross_xy * cx_world) - (position[0] * cross_yx)) / (cross_xy - cross_yx);
    edge_offset = hit_x - cx_world;
    hit_y = ((edge_offset * dy_bc) / dx_bc) + cy_world;
    if (hit_x == (bx + offset_x))
    {
        if (hit_y == (by + offset_z))
        {
            sampled_height = height1;
        }
        else
        {
            sampled_height = (((hit_y - cy_world) * (height1 - height0)) / dy_bc) + height0;
        }
    }
    else
    {
        sampled_height = ((edge_offset * (height1 - height0)) / dx_bc) + height0;
    }

    if (height0 < height1)
    {
        if (sampled_height > height1)
        {
            sampled_height = height1;
        }
        else if (sampled_height < height0)
        {
            sampled_height = height0;
        }
    }
    else
    {
        if (sampled_height > height0)
        {
            sampled_height = height0;
        }
        else if (sampled_height < height1)
        {
            sampled_height = height1;
        }
    }
    return sampled_height;
}

/**
 * @brief Resolve the slide direction along one collision edge.
 *
 * Locates boundary edge number @p edge_index in the node's edge-run list, takes the
 * two angle-table points bounding it, and turns them into a wall angle with
 * ratan2. Indices 0x7E and 0x7F skip the walk entirely and stand for the two
 * screen-aligned world edges, giving a fixed angle of 0x400 and 0 respectively.
 *
 * The wall can be slid along in either direction, so the side whose angle sits
 * nearer @p move_angle is chosen; a tie means the movement runs straight into the wall
 * and nothing can be resolved. The chosen angle is then merged with the running
 * best @p best_angle: when both deviate from @p move_angle the same way the larger deviation
 * wins, and a deviation of FIELD_COLLISION_SLIDE_MAX_TURN or more counts as
 * blocked.
 *
 * @param surface Surface definition owning the edge list, or NULL when @p edge_index is
 *            0x7E or 0x7F.
 * @param edge_index Edge index within the node, or 0x7E / 0x7F for a world edge.
 * @param move_angle Desired movement direction, in 0x1000 units per revolution.
 * @param best_angle Slide direction resolved so far: FIELD_COLLISION_SLIDE_NONE means none
 *             yet and -1 means an earlier edge already blocked the movement.
 * @return The resolved slide direction, @p best_angle when the earlier one still
 *         wins, or -1 when the movement is blocked.
 */
s32 func_8005E1A8(FieldCollisionSurfaceDef* surface, s32 edge_index, s32 move_angle, s32 best_angle)
{
    s16* vertices;
    s16* previous_point;
    s16* edge_point;
    FieldCollisionEdgeRun* edge_run;
    /* Point countdown during the walk, then the angle difference. */
    s32 delta;
    s32 slide_angle;
    s32 dx;
    s32 dy;
    s32 opposite_delta;
    /* "Have a previous point" flag in the walk, then the opposite wall angle; split, the walk is 97.08%. */
    s32 work;
    s32 wrapped_angle;

    if (best_angle == -1)
    {
        return -1;
    }
    previous_point = NULL;
    if (edge_index < FIELD_COLLISION_EDGE_WALL_Z)
    {
        vertices = g_field_node_angle_table;
        for (edge_run = surface->runs; (delta = edge_run->count & FIELD_NODE_RUN_COUNT_MASK) != 0; edge_run++)
        {
            edge_point = &vertices[edge_run->index * 2];
            while (--delta != -1)
            {
                work = previous_point != NULL;
                if (work)
                {
                    if (edge_index == 0)
                    {
                        /* Nested-loop exit past the closing-edge default; a flag and break is 93.45%. */
                        goto found;
                    }
                    edge_index--;
                }
                previous_point = edge_point;
                edge_point += 2;
            }
        }
        edge_point = &vertices[surface->runs[0].index * 2];
    found:
        dx = edge_point[0] - previous_point[0];
        dy = edge_point[1] - previous_point[1];
        if (dx == 0)
        {
            slide_angle = 0x400;
        }
        else
        {
            slide_angle = ratan2(dy, dx) & 0x7FF;
        }
    }
    else
    {
        slide_angle = (edge_index == FIELD_COLLISION_EDGE_WALL_Z) << 10;
    }

    if (move_angle < slide_angle)
    {
        delta = slide_angle - move_angle;
    }
    else
    {
        delta = move_angle - slide_angle;
        if (delta > 0x800)
        {
            wrapped_angle = move_angle - 0x1000;
            delta = slide_angle - wrapped_angle;
        }
    }
    work = slide_angle + 0x800;
    if (move_angle < work)
    {
        opposite_delta = work - move_angle;
        if (opposite_delta > 0x800)
        {
            wrapped_angle = slide_angle - 0x800;
            opposite_delta = move_angle - wrapped_angle;
        }
    }
    else
    {
        opposite_delta = move_angle - work;
    }
    if (delta == opposite_delta)
    {
        return -1;
    }
    if (opposite_delta < delta)
    {
        slide_angle += 0x800;
    }
    if (best_angle == FIELD_COLLISION_SLIDE_NONE)
    {
        return slide_angle;
    }

    delta = slide_angle - move_angle;
    if (delta > 0x800)
    {
        delta -= 0x1000;
    }
    else if (delta < -0x800)
    {
        delta += 0x1000;
    }
    opposite_delta = best_angle - move_angle;
    if (opposite_delta > 0x800)
    {
        opposite_delta -= 0x1000;
    }
    else if (opposite_delta < -0x800)
    {
        opposite_delta += 0x1000;
    }

    if ((delta >= 0) && (opposite_delta >= 0))
    {
        if (delta < opposite_delta)
        {
            slide_angle = best_angle;
        }
        else if (delta >= FIELD_COLLISION_SLIDE_MAX_TURN)
        {
            slide_angle = -1;
        }
    }
    else if ((delta <= 0) && (opposite_delta <= 0))
    {
        if (opposite_delta < delta)
        {
            slide_angle = best_angle;
        }
        else if (delta <= -FIELD_COLLISION_SLIDE_MAX_TURN)
        {
            slide_angle = -1;
        }
    }
    else
    {
        slide_angle = -1;
    }
    return slide_angle;
}

/**
 * @brief Rasterise a collision node's outline into per-row span lists.
 *
 * Phase 1 walks the surface's edge-run list and draws every polygon edge with
 * a Bresenham line, appending one FieldCollisionRasterSpan per row touched.
 * Consecutive edges that continue in the same vertical direction are merged
 * into the span the previous edge left on that row rather than starting a new
 * one. The edge body then runs once more for the closing edge back to
 * @c runs[0].index; that copy also merges against the first span of a row, so
 * the outline joins up. Each span end records its edge index, plus
 * FIELD_COLLISION_EDGE_OPEN when the run is flagged FIELD_COLLISION_RUN_OPEN;
 * horizontal edges record FIELD_COLLISION_EDGE_WALL_X.
 *
 * Phase 2 bubble-sorts each row's spans by x0, merges them in pairs into the
 * final interior runs, pads the row out with FIELD_COLLISION_RASTER_EMPTY_SPAN /
 * FIELD_COLLISION_RASTER_EMPTY_FLAGS, and word-copies the flag array to
 * @c node->span_flags.
 *
 * @param node Collision node being prepared; rows run from @c min_z to
 *             @c max_z, and @c spans / @c span_flags receive the tables.
 * @param alloc In/out field allocator cursor (a byte address, the same cursor
 *              func_8005F158 takes). On entry it points at the free block
 *              used for both tables; on exit it is advanced past them,
 *              rounded up to a multiple of 4.
 *
 * @note The scratchpad (FIELD_COLLISION_SCRATCH) holds one span-count byte per
 *       row. @c w is the raster capacity per row, twice
 *       FIELD_COLLISION_SURFACE_SPANS.
 * @note @c last_dir / @c first_dir hold 0 (none yet), 2 (horizontal edge) or
 *       2 + ystep * sgn for a sloped edge.
 */
void func_8005E3B0(FieldCollisionNode* node, s32* alloc)
{
    FieldCollisionSurfaceDef* def;
    s16* table;
    FieldCollisionRasterSpan* spans;
    FieldCollisionRasterSpanFlags* flags;
    u8* counts;
    s32 first_dir;
    s32 edge;
    FieldCollisionEdgeRun* run;
    FieldCollisionEdgePoint* pt;
    FieldCollisionEdgePoint* prev;
    FieldCollisionRasterSpan* sp_row;
    FieldCollisionRasterSpan* p;
    FieldCollisionRasterSpan* a;
    FieldCollisionRasterSpanFlags* fl_row;
    FieldCollisionRasterSpanFlags* save_flags;
    FieldCollisionRasterSpanFlags* q;
    FieldCollisionRasterSpanFlags* b;
    FieldCollisionRasterSpan tmp_span;
    FieldCollisionRasterSpanFlags tmp_flag;
    u8* cp;
    s32 rows;
    s16 capacity;
    s32 count;
    FieldCollisionRasterSpan* merge_span;
    s32 j;
    s32 n;
    s32 sort_n;

    s32 last_dir;
    s32 attr;

    s32 span_bytes;
    s32 open;
    s32 closing_open;
    u32 prev_open;
    u16 w;
    u16 y0;
    u16 ybase;
    s16 dx;

    s32 dy;
    s32 main_dy;
    s16 x;
    s16 sgn;
    s16 main_sgn;
    s16 ystep;
    s16 row;
    s16 err;
    s16 merge_row;
    s16 merge_row2;

    def = node->surface;
    rows = node->max_z;
    rows = rows - node->min_z;

    capacity = FIELD_COLLISION_SURFACE_SPANS(def);
    capacity *= 2;
    counts = (u8*)FIELD_COLLISION_SCRATCH;
    spans = (FieldCollisionRasterSpan*)*alloc;
    node->spans = (void*)*alloc;
    count = rows + 1;
    span_bytes = count * (capacity << 1);
    node->span_flags = (void*)(*alloc + span_bytes);
    flags = (FieldCollisionRasterSpanFlags*)(*alloc + (count * (capacity << 2)));
    *alloc = *alloc + (((count * ((capacity << 1) + capacity)) + 2) & ~3);

    w = capacity;
    cp = counts;
    for (count = rows; count != -1; count--)
    {
        *cp = 0;
        cp++;
    }

    last_dir = 0;
    prev = NULL;
    prev_open = 0;
    first_dir = 0;
    edge = 0;
    run = def->runs;
    table = g_field_node_angle_table;
    count = run->count & FIELD_NODE_RUN_COUNT_MASK;
    while (count != 0)
    {
        pt = (FieldCollisionEdgePoint*)&table[run->index * 2];
        for (count = count - 1; count != -1; count--)
        {
            if (prev != NULL)
            {
                open = 0;
                if (run->count & FIELD_COLLISION_RUN_OPEN)
                {
                    open = prev_open << 7;
                }
                attr = edge | open;
                dx = pt->x - prev->x;
                if (dx >= 0)
                {
                    x = prev->x;
                    y0 = prev->z;
                    main_dy = pt->z;
                    main_dy -= y0;
                    ybase = node->min_z;
                    row = y0 - ybase;
                    main_sgn = 1;
                }
                else
                {
                    dx = -dx;
                    x = pt->x;
                    main_sgn = -1;
                    main_dy = pt->z;
                    y0 = main_dy;
                    main_dy = prev->z;
                    main_dy -= y0;
                    ybase = node->min_z;
                    row = y0 - ybase;
                }

                cp = &counts[row];
                sp_row = &spans[row * w];
                sgn = main_sgn;
                fl_row = &flags[row * w];
                if ((s16)main_dy != 0)
                {
                    ystep = 1;
                    if ((s16)main_dy < 0)
                    {
                        main_dy = -main_dy;
                        ystep = -1;
                    }

                    if ((last_dir == 2) || (((ystep * sgn) + 2) == last_dir))
                    {
                        merge_row = prev->z - node->min_z;
                    }
                    else
                    {
                        merge_row = -1;
                    }
                    last_dir = (ystep * sgn) + 2;
                    if ((first_dir == 0) || (first_dir == 2))
                    {
                        first_dir = last_dir;
                    }
                    if ((s16)main_dy < dx)
                    {
                        j = dx + 1;
                        err = -dx;
                        while (j > 0)
                        {
                            n = *cp;
                            if (row == merge_row)
                            {
                                if (sp_row[n - 1].x.x0 > x)
                                {
                                    sp_row[n - 1].x.x0 = x;
                                    fl_row[n - 1].f.f0 = attr;
                                }
                            }
                            else
                            {
                                sp_row[n].x.x0 = x;
                                fl_row[n].f.f1 = attr;
                                fl_row[n].f.f0 = attr;
                            }
                            do
                            {
                                x++;
                                err += (s16)main_dy * 2;
                                j--;
                            } while ((err < 0) && (j > 0));
                            if (row == merge_row)
                            {
                                if (sp_row[n - 1].x.x1 < x)
                                {
                                    sp_row[n - 1].x.x1 = x;
                                    fl_row[n - 1].f.f1 = attr;
                                }
                            }
                            else
                            {
                                sp_row[n].x.x1 = x - 1;
                                *cp = n + 1;
                            }
                            cp += ystep;
                            row += ystep;
                            err -= dx * 2;
                            sp_row += ystep * w;
                            fl_row += ystep * w;
                        }
                    }
                    else
                    {
                        err = -(s16)main_dy;
                        for (j = (s16)main_dy; j != -1; j--)
                        {
                            n = *cp;

                            if (row == merge_row)
                            {
                                if (sp_row[n - 1].x.x0 > x)
                                {
                                    sp_row[n - 1].x.x0 = x;
                                    fl_row[n - 1].f.f0 = attr;
                                }
                                if (sp_row[n - 1].x.x1 < x)
                                {
                                    sp_row[n - 1].x.x1 = x;
                                    fl_row[n - 1].f.f1 = attr;
                                }
                            }
                            else
                            {
                                sp_row[n].x.x1 = x;
                                sp_row[n].x.x0 = x;
                                fl_row[n].f.f1 = attr;
                                fl_row[n].f.f0 = attr;
                                *cp = n + 1;
                            }
                            sp_row += ystep * w;
                            fl_row += ystep * w;
                            cp += ystep;
                            row += ystep;
                            err += dx * 2;
                            if (err >= 0)
                            {
                                x++;
                                err -= (s16)main_dy * 2;
                            }
                        }
                    }
                }
                else
                {
                    n = *cp;
                    if (n != 0)
                    {
                        if (sp_row[n - 1].x.x0 >= x)
                        {
                            sp_row[n - 1].x.x0 = x;
                            fl_row[n - 1].f.f0 = attr | FIELD_COLLISION_EDGE_WALL_X;
                        }
                        if (sp_row[n - 1].x.x1 <= (s16)(x + dx))
                        {
                            sp_row[n - 1].x.x1 = x + dx;
                            fl_row[n - 1].f.f1 = attr | FIELD_COLLISION_EDGE_WALL_X;
                        }
                    }
                    else
                    {
                        last_dir = 2;
                        sp_row[0].x.x1 = x + dx;
                        sp_row[0].x.x0 = x;
                        fl_row[0].f.f1 = attr | FIELD_COLLISION_EDGE_WALL_X;
                        fl_row[0].f.f0 = attr | FIELD_COLLISION_EDGE_WALL_X;
                        *cp = 1;
                    }
                    if (first_dir == 0)
                    {
                        first_dir = 2;
                    }
                }
                edge++;
            }
            prev = pt;
            pt++;
            prev_open = run->count >> 15;
        }
        run++;
        count = run->count & FIELD_NODE_RUN_COUNT_MASK;
    }

    /* Closing edge: back to the first run's first point. */
    closing_open = 0;
    run = def->runs;
    /* y0 briefly holds the open bit; testing run->count directly is 99.92%. */
    y0 = run->count & FIELD_COLLISION_RUN_OPEN;
    if (y0)
    {
        closing_open = prev_open << 7;
    }
    pt = (FieldCollisionEdgePoint*)&table[run->index * 2];
    attr = edge | closing_open;
    dx = pt->x - prev->x;
    if (dx >= 0)
    {
        x = prev->x;
        y0 = prev->z;
        dy = pt->z;
        dy -= y0;
        ybase = node->min_z;
        row = y0 - ybase;
        sgn = 1;
    }
    else
    {
        dx = -dx;
        x = pt->x;
        sgn = -1;
        y0 = pt->z;
        dy = prev->z;
        dy -= y0;
        ybase = node->min_z;
        row = y0 - ybase;
    }

    cp = &counts[row];
    sp_row = &spans[row * w];
    fl_row = &flags[row * w];
    if ((s16)dy != 0)
    {
        ystep = 1;
        if ((s16)dy < 0)
        {
            dy = -dy;
            ystep = -1;
        }
        if ((last_dir == 2) || (((ystep * sgn) + 2) == last_dir))
        {
            merge_row = prev->z - node->min_z;
        }
        else
        {
            merge_row = -1;
        }
        if (((ystep * sgn) + 2) == first_dir)
        {
            merge_row2 = pt->z - node->min_z;
        }
        else
        {
            merge_row2 = -1;
        }

        if ((s16)dy < dx)
        {
            j = dx + 1;
            err = -dx;
            while (j > 0)
            {
                n = *cp;
                if (row == merge_row)
                {
                    if (sp_row[n - 1].x.x0 > x)
                    {
                        sp_row[n - 1].x.x0 = x;
                        fl_row[n - 1].f.f0 = attr;
                    }
                }
                else if (row == merge_row2)
                {
                    if (sp_row[0].x.x0 > x)
                    {
                        sp_row[0].x.x0 = x;
                        fl_row[0].f.f0 = attr;
                    }
                }
                else
                {
                    sp_row[n].x.x0 = x;
                    fl_row[n].f.f1 = attr;
                    fl_row[n].f.f0 = attr;
                }
                do
                {
                    x++;
                    err += (s16)dy * 2;
                    j--;
                } while ((err < 0) && (j > 0));
                if (row == merge_row)
                {
                    if (sp_row[n - 1].x.x1 < x)
                    {
                        sp_row[n - 1].x.x1 = x;
                        fl_row[n - 1].f.f1 = attr;
                    }
                }
                else if (row == merge_row2)
                {
                    if (sp_row[0].x.x1 < x)
                    {
                        sp_row[0].x.x1 = x;
                        fl_row[0].f.f1 = attr;
                    }
                }
                else
                {
                    sp_row[n].x.x1 = x - 1;
                    *cp = n + 1;
                }
                cp += ystep;
                err -= dx * 2;
                sp_row += ystep * w;
                row += ystep;
                fl_row += ystep * w;
            }
        }
        else
        {
            err = -(s16)dy;
            for (j = (s16)dy; j != -1; j--)
            {
                /* Net-zero pairs: they weight dy onto $a0 (none 99.43%, one pair 99.52%, two 99.84%, s16 dy 99.84%). */
                dy++;
                dy--;
                dy++;
                dy--;
                dy++;
                dy--;
                dy++;
                dy--;
                dy++;
                dy--;
                dy++;
                dy--;
                dy++;
                dy--;

                n = *cp;
                if (row == merge_row)
                {
                    merge_span = FIELD_COLLISION_RASTER_LAST_SPAN(sp_row, n);
                    if (sp_row[n - 1].x.x0 > x)
                    {
                        sp_row[n - 1].x.x0 = x;
                        fl_row[n - 1].f.f0 = attr;
                    }
                    if (merge_span->x.x1 < x)
                    {
                        merge_span->x.x1 = x;
                        fl_row[n - 1].f.f1 = attr;
                    }
                }
                else if (row == merge_row2)
                {
                    if (sp_row[0].x.x0 > x)
                    {
                        sp_row[0].x.x0 = x;
                        fl_row[0].f.f0 = attr;
                    }
                    if (sp_row[0].x.x1 < x)
                    {
                        sp_row[0].x.x1 = x;
                        fl_row[0].f.f1 = attr;
                    }
                }
                else
                {
                    sp_row[n].x.x0 = x;
                    sp_row[n].x.x1 = x;
                    fl_row[n].f.f1 = attr;
                    fl_row[n].f.f0 = attr;
                    *cp = n + 1;
                }
                sp_row += ystep * w;
                fl_row += ystep * w;
                cp += ystep;
                row += ystep;
                err += dx * 2;
                if (err >= 0)
                {
                    x++;
                    err -= (s16)dy * 2;
                }
            }
        }
    }
    else
    {
        n = *cp;
        if (first_dir == 2)
        {
            sp_row[n].x.x0 = x;
            sp_row[n].x.x1 = x;
            fl_row[n].f.f1 = attr | FIELD_COLLISION_EDGE_WALL_X;
            fl_row[n].f.f0 = attr | FIELD_COLLISION_EDGE_WALL_X;
            *cp = n + 1;
        }
        else
        {
            if (sp_row[n - 1].x.x0 >= x)
            {
                sp_row[n - 1].x.x0 = x;
                fl_row[n - 1].f.f0 = attr | FIELD_COLLISION_EDGE_WALL_X;
            }
            if (sp_row[n - 1].x.x1 <= (s16)(x + dx))
            {
                sp_row[n - 1].x.x1 = x + dx;
                fl_row[n - 1].f.f1 = attr | FIELD_COLLISION_EDGE_WALL_X;
            }
        }
    }

    /* Phase 2: sp_row / fl_row become the output cursors. */
    capacity = w;
    sp_row = spans;
    fl_row = flags;
    save_flags = flags;
    for (count = node->max_z - node->min_z; count != -1; count--)
    {
        p = spans;
        q = flags;
        for (j = *counts - 2; j != -1; j--)
        {
            sort_n = j + 1;
            a = &p[sort_n];
            b = &q[sort_n];
            for (sort_n = j; sort_n != -1; sort_n--)
            {
                if (p->x.x0 > a->x.x0)
                {
                    tmp_span = *p;
                    *p = *a;
                    *a = tmp_span;
                    tmp_flag = *q;
                    *q = *b;
                    *b = tmp_flag;
                }
                a--;
                b--;
            }
            p++;
            q++;
        }

        p = spans;
        q = flags;
        j = (*counts >> 1);
        j--;
        for (; j != -1; j--)
        {
            if (p[0].x.x1 < p[1].x.x1)
            {
                sp_row->x.x0 = p[0].x.x0;
                sp_row->x.x1 = p[1].x.x1;
                fl_row->f.f0 = q[0].f.f0;
                fl_row->f.f1 = q[1].f.f1;
            }
            else
            {
                *sp_row = p[0];
                *fl_row = q[0];
            }
            sp_row++;
            p += 2;
            fl_row++;
            q += 2;
        }

        j = (capacity - *counts) / 2;
        j--;
        for (; j != -1; j--)
        {
            sp_row->word = FIELD_COLLISION_RASTER_EMPTY_SPAN;
            sp_row++;
        }
        j = (capacity - *counts) / 2;
        j--;
        for (; j != -1; j--)
        {
            fl_row->half = FIELD_COLLISION_RASTER_EMPTY_FLAGS;
            fl_row++;
        }

        spans += capacity;
        flags += capacity;
        counts++;
    }

    count = w * (((node->max_z - node->min_z) + 2) / 2);
    fl_row = save_flags;
    flags = (FieldCollisionRasterSpanFlags*)node->span_flags;
    for (count = count - 1; count != -1; count--)
    {
        *(s32*)flags = *(s32*)fl_row;
        fl_row += 2;
        flags += 2;
    }
}

/**
 * @brief Collect the scene's distinct floor groups and size their tile budget.
 *
 * Walks the attached-node list and builds a list of the distinct group ids
 * (floor heights) carried by each node's definition, skipping inactive nodes
 * (@c unk18 == 0) and solid ones (FIELD_COLLISION_SURFACE_SOLID). A flat node
 * contributes @c base_x, a slope contributes @c base_x and @c base_y; an id
 * already present just gets its @c seen mask widened. A zero id from a slope
 * is recorded with @c seen = 3 so it survives the slope filter.
 *
 * If any slope was seen, the list is compacted down to the entries whose
 * @c seen is 3. The surviving ids are sorted into @c scene->unk4A and the
 * matching @c scene->unk5E counters are cleared.
 *
 * Finally the per-group tile budget is computed from the scene's pixel extent:
 * 4-pixel tiles normally, 8-pixel tiles once the estimated tile count exceeds
 * FIELD_COLLISION_GROUP_TILE_BUDGET. Two blocks are carved off @p alloc - the
 * tile area (@c unk2C) and the work area (@c unk28 .. @c unk30) - and
 * func_8005F5BC rasterises the groups into the work area.
 *
 * @param alloc In/out bump allocator; advanced past both blocks on success.
 *
 * @note Fails with @c unk28 = 0 and @c unk41 = FIELD_COLLISION_GROUP_ERROR_GROUPS
 *       when the scan overflows or more than FIELD_COLLISION_GROUP_MAX ids
 *       survive. The empty-scene path leaves @c unk41 = 0 instead, which is
 *       how callers tell "no nodes" from "too many groups".
 * @note func_8005F5BC is called through its unprototyped declaration with the
 *       three arguments the original passes; it reads only the second.
 */
void func_8005F158(s32* alloc)
{
    FieldGroupEntry list[FIELD_COLLISION_GROUP_SCAN_MAX];
    FieldScene* scene;
    FieldNode* node;
    FieldNodeDef* def;
    FieldSceneHeader* header;
    s32 i;
    s32 j;
    s32 has_pair;
    s32 seen;
    s32 seen2;
    s32 k;
    /* "Id not listed yet" flag during the scan, then the scene width; split, 99.61%. */
    s32 width;
    s32 height;
    s32 shift;
    s32 tw;
    s32 th;
    s32 rows;
    s32 count4;
    u16 key;
    u16 tmp;
    u32 count;

    scene = g_field_scene.scene;
    count = 0;
    node = scene->nodes;
    if (node == NULL)
    {
        scene->unk28 = 0;
        scene->unk41 = 0;
        return;
    }

    has_pair = 0;
    do
    {
        def = node->def;
        if ((node->unk18 != 0) && !(def->flags & FIELD_COLLISION_SURFACE_SOLID))
        {
            i = count - 1;
            switch ((u8)def->flags & 3)
            {
            case FIELD_COLLISION_KIND_FLAT:
                width = 1;
                for (; i != -1; i--)
                {
                    if (list[i].id == def->base_x)
                    {
                        width = 0;
                        list[i].seen |= 1;
                        break;
                    }
                }
                if (width != 0)
                {
                    list[count].id = def->base_x;
                    list[count].seen = 1;
                    if (count >= FIELD_COLLISION_GROUP_SCAN_MAX)
                    {
                        goto overflow;
                    }
                    count++;
                }
                break;

            case FIELD_COLLISION_KIND_SLOPE:
                has_pair = 1;
                width = 1;
                for (; i != -1; i--)
                {
                    if (list[i].id == def->base_x)
                    {
                        width = 0;
                        list[i].seen |= 2;
                        break;
                    }
                }
                if (width != 0)
                {
                    if (def->base_x != 0)
                    {
                        seen = 2;
                        list[count].id = def->base_x;
                    }
                    else
                    {
                        seen = 3;
                        list[count].id = 0;
                    }
                    list[count].seen = seen;
                    if (count >= FIELD_COLLISION_GROUP_SCAN_MAX)
                    {
                        goto overflow;
                    }
                    count++;
                }

                width = 1;
                for (i = count - 1; i != -1; i--)
                {
                    if (list[i].id == def->base_y)
                    {
                        width = 0;
                        list[i].seen |= 2;
                        break;
                    }
                }
                if (width != 0)
                {
                    if (def->base_y != 0)
                    {
                        seen2 = 2;
                        list[count].id = def->base_y;
                    }
                    else
                    {
                        seen2 = 3;
                        list[count].id = 0;
                    }
                    list[count].seen = seen2;
                    if (count >= FIELD_COLLISION_GROUP_SCAN_MAX)
                    {
                        goto overflow;
                    }
                    count++;
                }
                break;
            }
        }
        node = node->next;
    } while (node != NULL);

    if (has_pair != 0)
    {
        i = count;
        j = 0;
        count = 0;
        i--;
        if (i != -1)
        {
            do
            {
                if (list[j].seen == 3)
                {
                    if (j != count)
                    {
                        list[count].id = list[j].id;
                    }
                    count++;
                }
                j++;
                i--;
            } while (i != -1);
        }
    }

    if (count == 0)
    {
        list[0].id = 0;
        count = 1;
    }
    else if (count > FIELD_COLLISION_GROUP_MAX)
    {
        goto overflow;
    }

    k = count - 1;
    if (k != 0)
    {
        do
        {
            key = list[k].id;
            for (j = k; --j != -1;)
            {
                if ((s16)key < list[j].id)
                {
                    tmp = list[j].id;
                    list[j].id = key;
                    key = tmp;
                    list[k].id = tmp;
                }
            }
            scene->unk4A[k] = key;
            k--;
        } while (k != 0);
    }

    i = count - 1;
    scene->unk4A[0] = list[0].id;
    if (count != 0)
    {
        do
        {
            scene->unk5E[i] = 0;
            i--;
        } while (i != -1);
    }

    header = scene->header;
    width = header->unk30;
    height = header->unk32;
    j = 4;
    shift = 2;
    i = ((width + j - 1) >> shift) * ((height + j * 2 - 1) >> (shift + 1)) * (s32)count;
    if (i > FIELD_COLLISION_GROUP_TILE_BUDGET)
    {
        j = 8;
        shift = 3;
    }
    tw = ((width + j) - 1) >> shift;
    width = tw + 4;
    th = ((height + (j * 2)) - 1) >> (shift + 1);
    rows = th + 4;
    i = width;
    i *= rows;
    scene->unk44 = i;
    i = i * count;
    scene->unk40 = j;
    scene->unk41 = count;
    scene->unk46 = width;
    scene->unk48 = rows;
    scene->unk2C = *alloc;
    i = (i + 3) & ~3;
    *alloc += i;
    i = (((u32)(tw + 0x23) >> 5) * th) * 2;
    scene->unk42 = i;
    scene->unk28 = *alloc;
    count4 = count * 4;
    j = i * count4;
    *alloc += j;
    scene->unk30 = *alloc;
    /* func_8005F5BC takes (s32, FieldNode *); the original passes the allocator, a null clip and the size. */
    ((void (*)(s32 *, s32, s32))func_8005F5BC)(alloc, 0, i);
    return;

overflow:
    /* Shared failure exit; each site storing and returning is 91.7%. */
    scene->unk28 = 0;
    scene->unk41 = FIELD_COLLISION_GROUP_ERROR_GROUPS;
}

/**
 * @brief Rasterise the active nodes of every group into the scene work area.
 *
 * For each group id in @c scene->unk4A, walks the scene's attached-node list
 * (pre-sorted by @c row_start) and converts each node's span table into a
 * bitmask of covered tile columns, written as two interleaved planes of
 * @c words 32-bit words per tile row.
 *
 * Per tile row the active nodes are collected into @c runs, then each of the
 * @c tile2 sub-rows accumulates its spans into @c cur, merges overlapping
 * spans, and folds the result two ways: an intersection against the previous
 * sub-row (ping-ponged between the two FIELD_COLLISION_RASTER_ROW_BYTES
 * halves of FIELD_COLLISION_SCRATCH) and a union in @c acc. The intersection
 * drives plane 0 and the union drives plane 1 of the output words.
 *
 * A node takes part in group @c id when it is solid and @c id >= id_min, or
 * when @c id lies below its floor (@c base_x, or the lower of @c base_x /
 * @c base_y for a slope) and at or above @c id_min.
 *
 * @param unused Unused; the target never reads the first argument.
 * @param clip Optional clipping node. When NULL every tile row of the group is
 *             emitted; otherwise only the rows the node covers are, and its
 *             definition is tested against the group id first.
 *
 * @note Fails with @c unk28 = 0 and a FIELD_COLLISION_GROUP_ERROR_* code in
 *       @c unk41 when a node or span list overflows.
 * @note The many @c -1 locals (countdown_end, collect_end, ...) keep loop.c
 *       from hoisting the end mark of each countdown loop; every one was
 *       measured (literal -1: 94.9% all, 99.2-99.3% each; one shared local:
 *       95.3%).
 * @note The three remaining do/while(0) wrappers (run fetch, compaction x0
 *       load, intersection work/dst step) each add exactly one weighted
 *       flow ref (one extra loop level) to one pseudo. Unwrapped, the
 *       global-alloc priority ties flip: src 34 refs/57 live vs x1 86/173
 *       (src loses t6), x0 80/173 vs the collect loop's hoisted x1 + 1
 *       13/14 (x0 loses t8), dst 37/90 vs the union loop's hoisted x1 + 1
 *       11/16 (dst loses t6). The competitors' live lengths are fixed by the
 *       target layout and every statement reorder only swaps two loads, so
 *       each wrapper stands for one phantom reference; no natural source
 *       shape supplying it has been found (see the h5bc lever report).
 */
void func_8005F5BC(s32 unused, FieldNode* clip)
{
    FieldCollisionTileSpan cur[FIELD_COLLISION_RASTER_SPAN_MAX];
    FieldCollisionTileSpan acc[FIELD_COLLISION_RASTER_SPAN_MAX];
    FieldCollisionRasterNode list[FIELD_COLLISION_RASTER_NODE_MAX];
    FieldCollisionSpanRun runs[FIELD_COLLISION_RASTER_NODE_MAX];
    FieldScene* scene;
    u32* saved;
    s32 words;
    s32 rows;
    s32 tile;
    s32 tile2;
    s32 shift0;
    s32 shift1;
    s32 group;
    u32 node_count;
    u32 j;
    s32 prev_count;
    FieldCollisionSpanRun* runs_base;
    u32* out;
    u32 cur_count;
    u32 acc_count;
    s32 clip_tail;
    s32 clip_rows;
    s32 run_count;
    FieldNode* node;
    FieldNodeDef* def;
    FieldCollisionRasterNode* p;
    u32 list_offset;
    s32 countdown_end;
    s32 union_end;
    s32 threshold;
    s32 node_start;
    FieldCollisionTileSpan* span;
    FieldCollisionTileSpan* prev_span;
    FieldCollisionTileSpan* other;
    FieldCollisionTileSpan* prev;
    FieldCollisionTileSpan* dst;
    u32* wp;
    u32* wq;
    u16* src;
    s32 i;
    s32 k;
    s32 n;
    s32 fresh;
    s32 lo;
    s32 hi;
    s32 lo2;
    s32 hi2;
    s32 scratch;
    s32 word;
    s32 work;
    s32 tail_mask;
    s32 lead;
    s32 tail;

    s16 base;
    u16 id;
    s32 signed_id;
    u16 swap_key;
    s16 x0;
    s16 x1;

    saved = NULL;
    prev_count = 0;
    acc_count = 0;
    scene = g_field_scene.scene;
    out = (u32*)scene->unk28;
    if (out == NULL)
    {
        return;
    }

    node = scene->nodes;
    node_count = 0;
    if (node != NULL)
    {
        p = list;
        do
        {
            if (node->unk18 != 0)
            {
                if (node_count >= FIELD_COLLISION_RASTER_NODE_MAX)
                {
                    scene->unk28 = 0;
                    scene->unk41 = FIELD_COLLISION_GROUP_ERROR_NODES;
                    return;
                }
                p->node = node;
                node_count++;
                p->key = node->row_start;
                p++;
            }
            node = node->next;
        } while (node != NULL);
    }

    if (node_count != 0)
    {
        i = node_count - 1;
        if (i != 0)
        {
            do
            {
                base = list[i].key;
                for (k = i - 1; k != -1; k--)
                {
                    if ((s16)base < list[k].key)
                    {
                        swap_key = list[k].key;
                        list[k].key = base;
                        base = swap_key;
                        list[i].key = swap_key;
                        node = list[k].node;
                        list[k].node = list[i].node;
                        list[i].node = node;
                    }
                }
                i--;
            } while (i != 0);
        }
    }

    tile = scene->unk40;
    if (tile == 4)
    {
        shift0 = 2;
        shift1 = 3;
    }
    else
    {
        shift0 = 3;
        shift1 = 4;
    }
    group = 0;
    tile2 = tile * 2;
    words = ((u16)scene->unk46 + 0x1F) >> 5;
    if (scene->unk41 == 0)
    {
        return;
    }

    /* -1 end mark of the countdown loops and all-ones mask; a literal -1 is 99.75%. */
    countdown_end = -1;
    /* Indexing runs directly is 99.99% (s7 spill slots swap). */
    runs_base = runs;
    do
    {
        id = scene->unk4A[group];
        if (clip == NULL)
        {
            base = 0;
            rows = (u16)scene->unk48 - 4;
        }
        else
        {
            def = clip->def;
            saved = out + ((words * 2) * ((u16)scene->unk48 - 4));
            fresh = 0;
            if (def->flags & FIELD_COLLISION_SURFACE_SOLID)
            {
                fresh = def->id_min <= (s16)id;
            }
            else if ((def->flags & FIELD_COLLISION_KIND_MASK) == FIELD_COLLISION_KIND_SLOPE)
            {
                lo = def->base_x;
                hi = def->base_y;
                if (lo < hi)
                {
                    if ((s16)id < lo)
                    {
                        if (((s16)id < def->id_min) == 0)
                        {
                            fresh = 1;
                        }
                    }
                }
                else if ((s16)id < hi)
                {
                    if (((s16)id < def->id_min) == 0)
                    {
                        fresh = 1;
                    }
                }
            }
            else
            {
                scratch = def->base_x;
                scratch = (s16)id < scratch;
                if (scratch != 0)
                {
                    if (((s16)id < def->id_min) == 0)
                    {
                        fresh = 1;
                    }
                }
            }

            if (fresh != 0)
            {
                base = 0;
                if (clip->row_start >= 0)
                {
                    base = (u16)clip->row_start & -tile2;
                    clip_rows = (u16)scene->unk48;
                    i = clip->row_end >> shift1;
                    if (i >= (clip_rows - 4))
                    {
                        clip_tail = ((s16)base >> shift1) + 4;
                        rows = clip_rows - clip_tail;
                    }
                    else
                    {
                        rows = (i - ((s16)base >> shift1)) + 1;
                    }
                }
                else
                {
                    i = clip->row_end >> shift1;
                    rows = (u16)scene->unk48 - 4;
                    if (i < rows)
                    {
                        rows = i + 1;
                    }
                }
                out = out + ((words * 2) * ((s16)base >> shift1));
            }
            else
            {
                out = saved;
                group++;
                continue;
            }
        }

        run_count = 0;
        j = 0;
        rows = rows - 1;
        if (rows != countdown_end)
        {
            signed_id = (s16)id;
            /* Byte offset into list, read through def; list[j] is 97.55%, list + list_offset 97.44%. */
            list_offset = 0;
            do
            {
                if (j < node_count)
                {
                    def = (FieldNodeDef*)list;
                    if (((FieldCollisionRasterNode*)((u8*)def + list_offset))->key < ((s16)base + tile2))
                    {
                        work = (s16)base;
                        threshold = (s16)base + tile2;
                        do
                        {
                            node = ((FieldCollisionRasterNode*)((u8*)def + list_offset))->node;
                            def = node->def;
                            fresh = 0;
                            if (def->flags & FIELD_COLLISION_SURFACE_SOLID)
                            {
                                fresh = signed_id >= def->id_min;
                            }
                            else if ((def->flags & FIELD_COLLISION_KIND_MASK) == FIELD_COLLISION_KIND_SLOPE)
                            {
                                lo2 = def->base_x;
                                hi2 = def->base_y;
                                if (lo2 < hi2)
                                {
                                    if (signed_id < lo2)
                                    {
                                        if (signed_id >= def->id_min)
                                        {
                                            fresh = 1;
                                        }
                                    }
                                }
                                else if (signed_id < hi2)
                                {
                                    if (signed_id >= def->id_min)
                                    {
                                        fresh = 1;
                                    }
                                }
                            }
                            else if (signed_id < def->base_x)
                            {
                                if (signed_id >= def->id_min)
                                {
                                    fresh = 1;
                                }
                            }
                            if ((fresh != 0) && (node->row_end >= work))
                            {
                                node_start = node->row_start;
                                if (work >= node_start)
                                {
                                    runs_base[run_count].src = node->spans + ((work - node_start) * FIELD_NODE_DEF_ROWS(node->def) * 2);
                                    runs_base[run_count].count = ((u16)node->row_end - (s16)base) + 1;
                                    runs_base[run_count].skip = 0;
                                }
                                else
                                {
                                    runs_base[run_count].src = node->spans;
                                    runs_base[run_count].count = ((u16)node->row_end - (u16)node->row_start) + 1;
                                    runs_base[run_count].skip = (u16)node->row_start - (s16)base;
                                }
                                runs_base[run_count].step = FIELD_NODE_DEF_ROWS(node->def);
                                run_count++;
                            }
                            list_offset += 8;
                            j++;
                            if (j < node_count)
                            {
                                def = (FieldNodeDef*)list;
                            }
                        } while ((j < node_count) && (list[j].key < threshold));
                    }
                }

                wp = out;
                wp[1] = 3;
                i = words - 2;
                out[0] = 3;
                if (i != countdown_end)
                {
                    do
                    {
                        wp += 2;
                        i--;
                        wp[1] = 0;
                        wp[0] = 0;
                    } while (i != -1);
                }

                node_start = ((u16)scene->unk46 - 1) & 0x1F;
                if (node_start == 0)
                {
                    scratch = wp[-2] | 0x80000000;
                    node_start = wp[0] | 1;
                    wp[-1] = scratch;
                    wp[-2] = scratch;
                    wp[1] = node_start;
                    wp[0] = node_start;
                }
                else
                {
                    work = node_start;
                    work = 1 << work;
                    scratch = wp[0] | work | ((u32)work >> 1);
                    wp[1] = scratch;
                    wp[0] = scratch;
                }

                if (run_count != 0)
                {
                    i = tile2 - 1;
                    if (i != countdown_end)
                    {
                        do
                        {
                            k = run_count;
                            k--;
                            cur_count = 0;
                            if (k != countdown_end)
                            {
                                do
                                {
                                    if (runs_base[k].skip == 0)
                                    {
                                        do
                                        {
                                            src = runs_base[k].src;
                                            n = runs_base[k].step;
                                            runs_base[k].count = runs_base[k].count - 1;
                                            n--;
                                        } while (0);
                                        if (n != countdown_end)
                                        {
                                            do
                                            {
                                                x0 = src[0];
                                                x1 = src[1];
                                                src += 2;
                                                if (x0 <= x1)
                                                {
                                                    span = cur;
                                                    work = cur_count;
                                                    work--;
                                                    fresh = 1;
                                                    if (work != countdown_end)
                                                    {
                                                        s32 collect_end = -1;
                                                        do
                                                        {
                                                            if (((x1 + 1) >= span->x0) && ((span->x1 + 1) >= x0))
                                                            {
                                                                if (x0 < span->x0)
                                                                {
                                                                    span->x0 = x0;
                                                                }
                                                                if (span->x1 < x1)
                                                                {
                                                                    span->x1 = x1;
                                                                }
                                                                fresh = 0;
                                                                break;
                                                            }
                                                            span++;
                                                            work--;
                                                        } while (work != collect_end);
                                                    }
                                                    if (fresh != 0)
                                                    {
                                                        if (cur_count >= FIELD_COLLISION_RASTER_SPAN_MAX)
                                                        {
                                                            scene->unk28 = 0;
                                                            scene->unk41 = FIELD_COLLISION_GROUP_ERROR_COLLECT;
                                                            return;
                                                        }
                                                        cur_count++;
                                                        span->x0 = x0;
                                                        span->x1 = x1;
                                                    }
                                                }
                                                n--;
                                            } while (n != countdown_end);
                                        }
                                        if (runs_base[k].count == 0)
                                        {
                                            run_count--;
                                            if (k != run_count)
                                            {
                                                runs_base[k].src = runs_base[run_count].src;
                                                runs_base[k].count = runs_base[run_count].count;
                                                runs_base[k].step = runs_base[run_count].step;
                                            }
                                        }
                                        else
                                        {
                                            runs_base[k].src = src;
                                        }
                                    }
                                    else
                                    {
                                        runs_base[k].skip = runs_base[k].skip - 1;
                                    }
                                    k--;
                                } while (k != countdown_end);
                            }

                            k = cur_count - 1;
                            span = cur;
                            if (k != countdown_end)
                            {
                                do
                                {
                                    n = k - 1;
                                    do
                                    {
                                        x0 = span->x0;
                                    } while (0);
                                    x1 = span->x1;
                                    other = span + 1;
                                    if (n != countdown_end)
                                    {
                                        s32 compact_end = -1;
                                        do
                                        {
                                            if (((x1 + 1) >= other->x0) && (x0 <= (other->x1 + 1)))
                                            {
                                                cur_count--;
                                                k--;
                                                if (other->x0 < x0 || x1 < other->x1)
                                                {
                                                    if (other->x0 < x0)
                                                    {
                                                        x0 = other->x0;
                                                    }
                                                    if (x1 < other->x1)
                                                    {
                                                        x1 = other->x1;
                                                    }
                                                    span->x0 = x0;
                                                    span->x1 = x1;
                                                    if (n != 0)
                                                    {
                                                        *(s32*)other = *(s32*)&other[n];
                                                    }
                                                    n = k;
                                                    other = span + 1;
                                                }
                                                else if (n != 0)
                                                {
                                                    *(s32*)other = *(s32*)&other[n];
                                                }
                                            }
                                            else
                                            {
                                                other++;
                                            }
                                            n--;
                                        } while (n != compact_end);
                                    }
                                    span++;
                                    k--;
                                } while (k != countdown_end);
                            }

                            if (i == (tile2 - 1))
                            {
                                span = cur;
                                other = acc;
                                prev = (FieldCollisionTileSpan*)FIELD_COLLISION_SCRATCH;
                                acc_count = cur_count;
                                prev_count = acc_count;
                                k = acc_count - 1;
                                if (k != countdown_end)
                                {
                                    s32 copy_end = -1;
                                    do
                                    {
                                        word = *(s32*)span;
                                        span++;
                                        k--;
                                        *(s32*)other = word;
                                        other++;
                                        *(s32*)prev = word;
                                        prev++;
                                    } while (k != copy_end);
                                }
                            }
                            else
                            {
                                span = cur;
                                if (!(i & 1))
                                {
                                    prev = (FieldCollisionTileSpan*)FIELD_COLLISION_SCRATCH;
                                    dst = (FieldCollisionTileSpan*)(FIELD_COLLISION_SCRATCH + FIELD_COLLISION_RASTER_ROW_BYTES);
                                }
                                else
                                {
                                    prev = (FieldCollisionTileSpan*)(FIELD_COLLISION_SCRATCH + FIELD_COLLISION_RASTER_ROW_BYTES);
                                    dst = (FieldCollisionTileSpan*)FIELD_COLLISION_SCRATCH;
                                }
                                k = cur_count - 1;
                                work = 0;
                                if (k != countdown_end)
                                {
                                    do
                                    {
                                        x0 = span->x0;
                                        x1 = span->x1;
                                        n = prev_count;
                                        n = n - 1;
                                        prev_span = prev;
                                        if (n != countdown_end)
                                        {
                                            s32 intersect_end;
                                            do
                                            {
                                                if ((x1 >= prev_span->x0) && (prev_span->x1 >= x0))
                                                {
                                                    if (work >= FIELD_COLLISION_RASTER_SPAN_MAX)
                                                    {
                                                        scene->unk28 = 0;
                                                        scene->unk41 = FIELD_COLLISION_GROUP_ERROR_INTERSECT;
                                                        return;
                                                    }
                                                    if (x0 < prev_span->x0)
                                                    {
                                                        dst->x0 = prev_span->x0;
                                                    }
                                                    else
                                                    {
                                                        dst->x0 = x0;
                                                    }
                                                    if (prev_span->x1 < x1)
                                                    {
                                                        dst->x1 = prev_span->x1;
                                                    }
                                                    else
                                                    {
                                                        dst->x1 = x1;
                                                    }
                                                    do
                                                    {
                                                        work++;
                                                        dst++;
                                                    } while (0);
                                                }
                                                intersect_end = -1;
                                                prev_span++;
                                                n--;
                                            } while (n != intersect_end);
                                        }

                                        other = acc;
                                        n = acc_count - 1;
                                        fresh = 1;
                                        if (n != countdown_end)
                                        {
                                            do
                                            {
                                                if (((x1 + 1) >= other->x0) && ((other->x1 + 1) >= x0))
                                                {
                                                    if (x0 < other->x0)
                                                    {
                                                        other->x0 = x0;
                                                    }
                                                    if (other->x1 < x1)
                                                    {
                                                        other->x1 = x1;
                                                    }
                                                    fresh = 0;
                                                    break;
                                                }
                                                union_end = -1;
                                                other++;
                                                n--;
                                            } while (n != union_end);
                                        }
                                        if (fresh != 0)
                                        {
                                            if (acc_count >= FIELD_COLLISION_RASTER_SPAN_MAX)
                                            {
                                                scene->unk28 = 0;
                                                scene->unk41 = FIELD_COLLISION_GROUP_ERROR_UNION;
                                                return;
                                            }
                                            acc_count++;
                                            other->x0 = x0;
                                            other->x1 = x1;
                                        }
                                        k--;
                                        span++;
                                    } while (k != countdown_end);
                                }
                                prev_count = work;
                            }
                            i--;
                        } while (i != countdown_end);
                    }

                    prev = (FieldCollisionTileSpan*)(FIELD_COLLISION_SCRATCH + FIELD_COLLISION_RASTER_ROW_BYTES);
                    i = prev_count;
                    i--;
                    if (i != countdown_end)
                    {
                        do
                        {
                            lead = (((prev->x0 + tile) - 1) >> shift0) + 2;
                            tail = ((prev->x1 + 1) >> shift0) + 1;
                            n = lead >> 5;
                            if (tail >= lead)
                            {
                                k = tail >> 5;
                                wp = out + (n * 2);
                                work = lead;
                                work = countdown_end << (work & 0x1F);
                                tail_mask = (u32)countdown_end >> (0x1F - (tail & 0x1F));
                                if (n != k)
                                {
                                    k = (k - n) - 2;
                                    wp[0] |= work;
                                    wp += 2;
                                    if (k != countdown_end)
                                    {
                                        do
                                        {
                                            wp[0] = countdown_end;
                                            k--;
                                            wp += 2;
                                        } while (k != countdown_end);
                                    }
                                    wp[0] |= tail_mask;
                                }
                                else
                                {
                                    wp[0] |= work & tail_mask;
                                }
                            }
                            i--;
                            prev++;
                        } while (i != countdown_end);
                    }

                    i = acc_count - 1;
                    other = acc;
                    if (i != countdown_end)
                    {
                        do
                        {
                            lead = (other->x0 >> shift0) + 2;
                            tail = other->x1 >> shift0;
                            tail = tail + 2;
                            n = lead >> 5;
                            k = tail >> 5;
                            work = lead;
                            work = countdown_end << (work & 0x1F);
                            tail_mask = (u32)countdown_end >> (0x1F - (tail & 0x1F));
                            wq = out + (n * 2);
                            if (n != k)
                            {
                                wp = wq + 3;
                                k = (k - n) - 2;
                                wq[1] |= work;
                                if (k != countdown_end)
                                {
                                    do
                                    {
                                        wp[0] = countdown_end;
                                        k--;
                                        wp += 2;
                                    } while (k != countdown_end);
                                }
                                wp[0] |= tail_mask;
                            }
                            else
                            {
                                wq[1] |= work & tail_mask;
                            }
                            i--;
                            other++;
                        } while (i != countdown_end);
                    }
                }

                out += words * 2;
                base += tile2;
                rows--;
            } while (rows != countdown_end);
        }

        if (clip != NULL)
        {
            out = saved;
        }
        group++;
    } while (group != scene->unk41);
}

/**
 * @brief Dilate every group's tile bitmasks by a footprint into the byte tile maps.
 *
 * Reads the per-group bitmask rows written by func_8005F5BC (scene->unk28,
 * two words per 32 columns: solid bits, then touch bits) and writes one byte
 * per tile to scene->unk2C: 0 = free, 1 = touched, 0xFF = blocked. Each row is
 * spread right by the footprint width (the edge columns keep touch, the
 * interior columns become inner touch) and down by the footprint depth through
 * a ring of {solid, touch, inner} word triples in the scratchpad; inner touch
 * counts as blocked in the middle rows. The map is framed by blocked bytes:
 * two rows on top, footprint_depth + 1 rows at the bottom and a
 * footprint_width + 1 run after each emitted row.
 *
 * @param footprint_width Footprint width in collision-map columns.
 * @param footprint_depth Footprint depth in collision-map rows.
 */
void func_80060364(s32 footprint_width, s32 footprint_depth)
{
    FieldScene* scene;
    u32* src_row;
    s32 groups_left;
    s32 row_words;
    s32 ring_row_words;
    s32 rows_left;
    s32 spill_bits;
    s32 ring_bytes;
    s32 ring_row_bytes;
    s32 ring_words;
    u16 cols;
    u8 group_count;
    s32 last_group;
    s32 reach;
    u32* src_word;
    s32 word_bits;
    u8* out;
    u32 ring_write;
    u32 ring_read;
    u32 ring_end;
    s32 count;
    s32 remaining;
    s32 row;
    s32 value;
    u32 in_solid;
    u32 in_touch;
    u32 inner_touch;
    u32 acc_touch;
    u32 left_bits;
    u32 acc_solid;
    u32 spread;
    u32 shift_solid;
    u32 shift_touch;
    u32 carry_touch;
    u32 carry_solid;
    s32 bits_left;
    u32 ring_inner;
    u32 entry_touch;
    u32 entry_solid;
    u32 entry_inner;
    u8 cell;
    u32 prev_touch;
    u32 prev_solid;
    u8 cell2;
    u8 cell1;
    u32 spread_solid;
    u32 spread4;

    scene = g_field_scene.scene;
    cols = (u16)scene->unk46;
    row_words = ((s32)(cols + 0x1F) >> 5) * 2;
    ring_row_words = (((s32)(cols - 3) >> 5) + 1) * 3;
    reach = footprint_width - 1;
    /*
     * Unsolved levers (see ul01-report.md). loop_38 is not a C loop: any real loop
     * (do/while, while, for) makes loop.c hoist the second switch's
     * jump-table base out of it (threshold 55 * savings 2 * life 3 = 330
     * >= the loop's 291 insns). The do/while(0) blocks lift footprint_depth
     * (+3 refs, to 33) and ring_bytes/ring_row_bytes (+1 each, to 16) over
     * their floor_log2 priority boundaries, giving s5/s6/s7/s8 in target order.
     */
    do
    {
        if (footprint_depth >= 3)
        {
            ring_write = FIELD_COLLISION_SCRATCH;
            ring_read = FIELD_COLLISION_SCRATCH;
            ring_end = (ring_row_words * footprint_depth * 4) + FIELD_COLLISION_SCRATCH;
        }
        else
        {
            ring_write = 0;
            ring_read = 0;
            ring_end = 0;
        }
    } while (0);
    src_row = (u32*)scene->unk28;
    group_count = scene->unk41;
    groups_left = (s32)group_count; /* dead, but the spilled counter's store is real */
    out = (u8*)scene->unk2C;
    last_group = group_count - 1;
    groups_left = last_group;
    if (last_group != -1)
    {
        word_bits = 0x20;
        do
        {
            remaining = (u16)scene->unk46 * 2;
            FIELD_COLLISION_FILL_BLOCKED(out, count, remaining);
            do
            {
                row = 0;
                value = ((u16)scene->unk48 - footprint_depth) - 4;
            } while (0);
            rows_left = value;
            if (value != -1)
            {
                ring_words = ring_row_words * footprint_depth;
                ring_row_bytes = ring_row_words * 4;
                ring_bytes = ring_words * 4;
                do
                {
                    src_word = src_row;
                    remaining = (u16)scene->unk46;
                    src_row = src_word + row_words;
                    in_solid = src_word[0];
                    in_touch = src_word[1];
                    value = remaining - 1;
                    remaining = value - footprint_width;
                    src_word += 2;
                    switch (footprint_width - 1)
                    {
                    default:
                        acc_touch = in_touch | (in_touch >> reach);
                        inner_touch = (in_touch >> 1) | (in_touch >> 2);
                        shift_touch = inner_touch;
                        shift_solid = in_solid | (in_solid >> 1);
                        spread = shift_solid >> 2;
                        acc_solid = shift_solid | spread;
                        shift_solid = spread;
                        count = (footprint_width - 6) >> 1;
                        do
                        {
                            shift_touch = shift_touch >> 2;
                            inner_touch |= shift_touch;
                            shift_solid = shift_solid >> 2;
                            count -= 1;
                            acc_solid |= shift_solid;
                        } while (count != -1);
                        if (footprint_width & 1)
                        {
                            inner_touch |= in_touch >> (footprint_width - 2);
                            acc_solid |= in_solid >> reach;
                        }
                        carry_touch = in_touch;
                        carry_solid = in_solid;
                        break;
                    case 4:
                        inner_touch = (in_touch >> 1) | (in_touch >> 2) | (in_touch >> 3);
                        acc_touch = in_touch | (in_touch >> 4);
                        shift_solid = in_solid | (in_solid >> 1);
                        acc_solid = shift_solid | (shift_solid >> 2) | (in_solid >> 4);
                        carry_touch = in_touch;
                        carry_solid = in_solid;
                        break;
                    case 3:
                        inner_touch = (in_touch >> 1) | (in_touch >> 2);
                        acc_touch = in_touch | (in_touch >> 3);
                        shift_solid = in_solid | (in_solid >> 1);
                        acc_solid = shift_solid | (shift_solid >> 2);
                        carry_touch = in_touch;
                        carry_solid = in_solid;
                        break;
                    case 2:
                        inner_touch = in_touch >> 1;
                        acc_touch = in_touch | (in_touch >> 2);
                        acc_solid = in_solid | (in_solid >> 1) | (in_solid >> 2);
                        carry_touch = in_touch;
                        carry_solid = in_solid;
                        break;
                    case 1:
                        inner_touch = 0;
                        acc_touch = in_touch | (in_touch >> 1);
                        acc_solid = in_solid | (in_solid >> 1);
                        carry_touch = in_touch;
                        carry_solid = in_solid;
                        break;
                    case 0:
                        inner_touch = 0;
                        acc_touch = in_touch;
                        acc_solid = in_solid;
                        carry_touch = 0;
                        carry_solid = 0;
                        break;
                    }
                    bits_left = word_bits;
                    bits_left -= reach;
                    if (footprint_depth != 1)
                    {
                        if (footprint_depth == 2)
                        {
                            if (!(row & 1))
                            {
                                ring_write = FIELD_COLLISION_SCRATCH;
                                ring_read = ring_row_bytes + FIELD_COLLISION_SCRATCH;
                            }
                            else
                            {
                                ring_write = FIELD_COLLISION_SCRATCH + ring_row_bytes;
                                ring_read = FIELD_COLLISION_SCRATCH;
                            }
                        }
                        else
                        {
                            if (ring_write >= ring_end)
                            {
                                ring_write -= ring_bytes;
                            }
                            if (ring_read >= ring_end)
                            {
                                ring_read -= ring_bytes;
                            }
                        }
                    }
                    if (remaining != 0)
                    {
                        ring_inner = ring_write + 8;
                    loop_38:
                        if (bits_left < remaining)
                        {
                            remaining -= bits_left;
                        }
                        else
                        {
                            bits_left = remaining;
                            remaining = 0;
                        }
                        acc_touch = acc_touch | inner_touch;
                        switch (footprint_depth)
                        {
                        case 1:
                            do
                            {
                                if (acc_touch & 1)
                                {
                                    cell1 = 1;
                                    if (acc_solid & 1)
                                    {
                                        cell1 = -1;
                                    }
                                }
                                else
                                {
                                    cell1 = 0;
                                }
                                *out = cell1;
                                out += 1;
                                acc_touch = acc_touch >> 1;
                                bits_left -= 1;
                                acc_solid = acc_solid >> 1;
                            } while (bits_left != 0);
                            break;
                        case 2:
                            *(s32*)ring_write = acc_solid;
                            *(s32*)(ring_inner - 4) = acc_touch;
                            ring_inner += 0xC;
                            ring_write += 0xC;
                            if (row != 0)
                            {
                                prev_touch = *(u32*)(ring_read + 4);
                                prev_solid = *(u32*)(ring_read + 0);
                                ring_read += 0xC;
                                acc_touch = acc_touch | prev_touch;
                                acc_solid = acc_solid | prev_solid;
                                do
                                {
                                    if (acc_touch & 1)
                                    {
                                        cell2 = 1;
                                        if (acc_solid & 1)
                                        {
                                            cell2 = -1;
                                        }
                                    }
                                    else
                                    {
                                        cell2 = 0;
                                    }
                                    *out = cell2;
                                    out += 1;
                                    acc_touch = acc_touch >> 1;
                                    bits_left -= 1;
                                    acc_solid = acc_solid >> 1;
                                } while (bits_left != 0);
                            }
                            break;
                        default:
                            *(s32*)ring_write = acc_solid;
                            *(s32*)(ring_inner - 4) = acc_touch;
                            *(s32*)(ring_inner + 0) = inner_touch;
                            ring_inner += 0xC;
                            ring_write += 0xC;
                            if (row >= (footprint_depth - 1))
                            {
                                do
                                {
                                    shift_touch = ring_read + ring_row_bytes;
                                    acc_touch = acc_touch | *(u32*)(ring_read + 4);
                                    acc_solid |= *(u32*)(ring_read + 0);
                                    if (shift_touch >= ring_end)
                                    {
                                        shift_touch -= ring_bytes;
                                    }
                                } while (0);
                                count = footprint_depth - 2;
                                do
                                {
                                    entry_touch = *(u32*)(shift_touch + 4);
                                    entry_solid = *(u32*)(shift_touch + 0);
                                    entry_inner = *(u32*)(shift_touch + 8);
                                    shift_touch = shift_touch + ring_row_bytes;
                                    acc_solid |= entry_solid | entry_inner;
                                    acc_touch |= entry_touch;
                                    if (shift_touch >= ring_end)
                                    {
                                        shift_touch -= ring_bytes;
                                    }
                                    count -= 1;
                                } while (count != 0);
                                ring_read += 0xC;
                                do
                                {
                                    if (acc_touch & 1)
                                    {
                                        cell = 1;
                                        if (acc_solid & 1)
                                        {
                                            cell = -1;
                                        }
                                    }
                                    else
                                    {
                                        cell = 0;
                                    }
                                    *out = cell;
                                    out += 1;
                                    acc_touch = acc_touch >> 1;
                                    bits_left -= 1;
                                    acc_solid = acc_solid >> 1;
                                } while (bits_left != 0);
                            }
                            break;
                        }
                        if (remaining != 0)
                        {
                            in_solid = src_word[0];
                            in_touch = src_word[1];
                            src_word += 2;
                            switch (footprint_width - 1)
                            {
                            default:
                                {
                                    s32 edge_shift = 0x20 - reach;
                                    s32 inner_shift = 0x21 - reach;
                                    acc_touch = carry_touch >> edge_shift;
                                    inner_touch = (carry_touch >> inner_shift) | (carry_touch >> (0x22 - reach));
                                    shift_touch = inner_touch;
                                    count = (footprint_width - 6) >> 1;
                                }
                                do
                                {
                                    shift_touch = shift_touch >> 2;
                                    count -= 1;
                                    inner_touch |= shift_touch;
                                } while (count != -1);
                                {
                                    s32 edge_shift = 0x20 - reach;
                                    s32 inner_shift = 0x21 - reach;
                                    acc_solid = (carry_solid >> edge_shift) | (carry_solid >> inner_shift);
                                }
                                shift_solid = acc_solid;
                                count = (s32)(reach - 4) >> 1;
                                do
                                {
                                    shift_solid = shift_solid >> 2;
                                    count -= 1;
                                    acc_solid |= shift_solid;
                                } while (count != -1);
                                if (footprint_width & 1)
                                {
                                    inner_touch |= carry_touch >> 0x1F;
                                }
                                else
                                {
                                    acc_solid |= carry_solid >> 0x1F;
                                }
                                if (in_touch != 0)
                                {
                                    shift_touch = (in_touch * 2) | (in_touch * 4);
                                    inner_touch |= shift_touch;
                                    shift_solid = in_solid | (in_solid * 2);
                                    spread4 = shift_solid * 4;
                                    acc_solid |= shift_solid | spread4;
                                    shift_solid = spread4;
                                    count = (footprint_width - 6) >> 1;
                                    do
                                    {
                                        shift_touch *= 4;
                                        inner_touch |= shift_touch;
                                        shift_solid *= 4;
                                        count -= 1;
                                        acc_solid |= shift_solid;
                                    } while (count != -1);
                                    if (footprint_width & 1)
                                    {
                                        inner_touch |= in_touch << (footprint_width - 2);
                                        acc_solid |= in_solid << reach;
                                    }
                                    acc_touch |= in_touch | (in_touch << reach);
                                    carry_touch = in_touch;
                                    carry_solid = in_solid;
                                }
                                else
                                {
                                    carry_touch = 0;
                                    carry_solid = 0;
                                }
                                break;
                            case 4:
                                inner_touch = (carry_touch >> 0x1D) | (carry_touch >> 0x1E) | (carry_touch >> 0x1F);
                                acc_touch = carry_touch;
                                acc_touch >>= 0x1C;
                                shift_solid = (carry_solid >> 0x1C) | (carry_solid >> 0x1D);
                                acc_solid = shift_solid | (shift_solid >> 2);
                                if (in_touch != 0)
                                {
                                    left_bits = (in_touch * 2) | (in_touch * 4) | (in_touch * 8);
                                    inner_touch |= left_bits;
                                    acc_touch |= in_touch | (in_touch * 0x10);
                                    shift_solid = in_solid | (in_solid * 2);
                                    spill_bits = shift_solid * 4;
                                    spread_solid = shift_solid | spill_bits;
                                    left_bits = in_solid * 0x10;
                                    spread_solid |= left_bits;
                                    acc_solid |= spread_solid;
                                    carry_touch = in_touch;
                                    carry_solid = in_solid;
                                }
                                else
                                {
                                    carry_touch = 0;
                                    carry_solid = 0;
                                }
                                break;
                            case 3:
                                inner_touch = (carry_touch >> 0x1E) | (carry_touch >> 0x1F);
                                acc_touch = carry_touch >> 0x1D;
                                left_bits = (carry_solid >> 0x1D) | (carry_solid >> 0x1E);
                                carry_solid = (s32)carry_solid >> 0x1F;
                                acc_solid = left_bits | (carry_solid & 1);
                                if (in_touch != 0)
                                {
                                    left_bits = (in_touch * 2) | (in_touch * 4);
                                    inner_touch |= left_bits;
                                    acc_touch |= in_touch | (in_touch * 8);
                                    shift_solid = in_solid | (in_solid * 2);
                                    spread_solid = shift_solid | (shift_solid * 4);
                                    acc_solid |= spread_solid;
                                    carry_touch = in_touch;
                                    carry_solid = in_solid;
                                }
                                else
                                {
                                    carry_touch = 0;
                                    carry_solid = 0;
                                }
                                break;
                            case 2:
                                inner_touch = carry_touch >> 0x1F;
                                acc_touch = carry_touch >> 0x1E;
                                acc_solid = (carry_solid >> 0x1E) | (carry_solid >> 0x1F);
                                if (in_touch != 0)
                                {
                                    inner_touch |= in_touch * 2;
                                    acc_touch |= in_touch | (in_touch * 4);
                                    spill_bits = in_solid * 2;
                                    spread_solid = in_solid | spill_bits;
                                    left_bits = in_solid * 4;
                                    spread_solid |= left_bits;
                                    acc_solid |= spread_solid;
                                    carry_touch = in_touch;
                                    carry_solid = in_solid;
                                }
                                else
                                {
                                    carry_touch = 0;
                                    carry_solid = 0;
                                }
                                break;
                            case 1:
                                acc_touch = carry_touch >> 0x1F;
                                acc_solid = carry_solid >> 0x1F;
                                if (in_touch != 0)
                                {
                                    acc_touch |= in_touch | (in_touch * 2);
                                    spread_solid = in_solid | (in_solid * 2);
                                    acc_solid |= spread_solid;
                                    carry_touch = in_touch;
                                    carry_solid = in_solid;
                                }
                                else
                                {
                                    carry_touch = 0;
                                    carry_solid = 0;
                                }
                                break;
                            case 0:
                                acc_solid = in_solid;
                                acc_touch = in_touch;
                                break;
                            }
                            bits_left = word_bits;
                            if (remaining != 0)
                            {
                                goto loop_38;
                            }
                        }
                    }
                    if (row >= (footprint_depth - 1))
                    {
                        remaining = footprint_width;
                        if (remaining != -1)
                        {
                            s32 end = -1;
                            do
                            {
                                *out = -1;
                                remaining -= 1;
                                out += 1;
                            } while (remaining != end);
                        }
                    }
                    row += 1;
                    rows_left--;
                } while (rows_left != -1);
            }
            remaining = (u16)scene->unk46 * (footprint_depth + 1);
            FIELD_COLLISION_FILL_BLOCKED(out, count, remaining);
            groups_left--;
        } while (groups_left != -1);
    }
}

/**
 * @brief Mark the collision-map tiles covered by a query footprint and margins.
 * @param margins Query whose width/depth give the extra X/Z margin in tiles.
 * @param query World-space query position and footprint.
 * @return 0 on success, -1 when group data exists without a work map, or -2 when the footprint lies outside the usable map.
 * @note Tiles are marked 0xFE in the tile map of the group whose floor height
 *       matches the query; blocked tiles (0xFF) are left alone.
 * @note When the row start is clipped the original adjusts the column run
 *       (ncol/col_start), not the row run; kept as in the binary.
 */
s32 func_80060CB0(FieldCollisionQuery* margins, FieldCollisionQuery* query)
{
    FieldScene* scene;
    s32 count;
    s32 group;
    s32 i;
    s32 height;
    s32 cell;
    s32 floor_cell;
    s32 half;
    s32 margin;
    s32 tile;
    s32 col_shift;
    s32 row_shift;
    s32 col_mask;
    s32 row_mask;
    s32 left;
    s32 top;
    s32 col_start;
    s32 row_start;
    s32 ncol;
    s32 nrow;
    s32 cols;
    s32 rows;
    s32 col;
    s32 n;
    s32 clipped;
    u8* row_base;
    u8* p;

    scene = g_field_scene.scene;
    if (scene->unk28 == 0)
    {
        if (scene->unk41 == 0)
        {
            return 0;
        }
        return -1;
    }

    height = query->y;
    floor_cell = height >> 8;
    if (height < 0)
    {
        floor_cell = (height + 0xFF) >> 8;
    }

    count = scene->unk41;
    group = count - 1;
    i = 0;
    if (count != 0)
    {
        s32 group_count;
        group_count = count;
        for (; i != group_count; i++)
        {
            s32 group_floor = scene->unk4A[i];

            if (floor_cell < group_floor)
            {
                if (i != 0)
                {
                    i -= 1;
                }
                group = i;
                break;
            }
            else if (group_floor == floor_cell)
            {
                group = i;
                break;
            }
        }
    }

    tile = scene->unk40;
    col_shift = 3;
    if (tile == 4)
    {
        col_shift = 2;
    }

    /*
     * The x edge is computed in each arm and the z edge after the join: jump2
     * cross-jumps the two x tails after scheduling, so the z loads cannot be
     * scheduled into them. query->x/z are read directly (a shared coord local
     * is set twice and sched1 then hoists its load).
     */
    half = ((s16)query->width) >> 1;
    margin = (s16)margins->width - 1;
    if (query->x >= 0)
    {
        left = ((query->x >> 8) - half) - margin;
    }
    else
    {
        left = (((query->x + 0xFF) >> 8) - half) - margin;
    }

    half = ((s16)query->depth) >> 1;
    margin = (s16)margins->depth - 1;
    if (query->z >= 0)
    {
        cell = query->z >> 8;
    }
    else
    {
        cell = (query->z + 0xFF) >> 8;
    }
    top = (cell - half) - margin;

    col_start = (left >> col_shift) + 2;
    row_shift = col_shift + 1;
    row_start = (top >> row_shift) + 2;
    col_mask = tile - 1;
    ncol = (((left & col_mask) + (s16)margins->width + ((s16)query->width) + col_mask) - 1) >> col_shift;
    row_mask = (tile * 2) - 1;
    nrow = (((top & row_mask) + (s16)margins->depth + ((s16)query->depth) + row_mask) - 1) >> row_shift;
    /* Loop-depth weight on cols: it must take $v1 before the cols - 1 / rows - 1 temps do. */
    do
    {
        cols = (u16)scene->unk46;
    } while (0);
    rows = (u16)scene->unk48;

    if (col_start <= 0)
    {
        clipped = ncol - 1;
        clipped = clipped + col_start;
        if (clipped <= 0)
        {
            return -2;
        }
        ncol = clipped;
        col_start = 1;
    }
    if (row_start <= 0)
    {
        if ((nrow + (row_start - 1)) <= 0)
        {
            return -2;
        }
        ncol += col_start - 1;
        col_start = 1;
    }
    if ((col_start >= cols - 1) || (row_start >= rows - 1))
    {
        return -2;
    }

    /* top doubles as the group's tile-map offset (a separate local reallocates). */
    top = (u16)scene->unk44 * group;
    row_base = (u8*)(scene->unk2C + top + (cols * row_start) + col_start);
    for (nrow -= 1; nrow != -1; nrow--)
    {
        p = row_base;
        if (row_start >= rows - 1)
        {
            break;
        }
        col = col_start;
        for (n = ncol - 1; n != -1; n--)
        {
            if (col >= cols - 1)
            {
                break;
            }
            if (*p != 0xFF)
            {
                *p = 0xFE;
            }
            p++;
            col++;
        }
        row_start++;
    }
    return 0;
}

/**
 * @brief Find a route for a footprint across the group tile maps.
 *
 * Works in footprint cells (world >> 8, minus the half extents) on the byte
 * tile map of the floor group that matches each query's height. The goal tile
 * is marked FIELD_COLLISION_TILE_GOAL and the start tile
 * FIELD_COLLISION_TILE_START. When the scene has no group maps, or the straight
 * footprint walk from the goal to the start is clear (func_80062820), the goal
 * position itself is written as the single point.
 *
 * Otherwise a bucketed breadth-first search runs from the start tile. path[0..3]
 * are four rings of queue entries (tile address | open directions << 21, see
 * FIELD_COLLISION_PATH_DIRS). Each wave pops the current ring, claims the open
 * neighbours with the wave stamp (counting down from
 * FIELD_COLLISION_TILE_STAMP_FIRST) and pushes them to the next ring. Touched
 * tiles go to the deferred ring instead and cost two extra waves. Diagonal
 * moves need both orthogonal neighbours open (FIELD_COLLISION_CORNER_TEST).
 * The search stops when a neighbour is the goal tile (`route_value`).
 *
 * The goal is then linked to the route tile by a traced line, and the route is
 * walked back from there to the start by always stepping onto the highest
 * stamp; only the tiles where the step direction changes are kept. Those tiles
 * become cell points, a greedy string pull drops every point that a clear
 * trace can skip, and the second point is nudged towards the midpoints and
 * quarter points of its neighbours while the traces stay clear. Up to
 * FIELD_COLLISION_PATH_OUT_MAX points are written, start end first, in world
 * units (cell + half extent) << 8.
 *
 * @param start_query Mover position and footprint (width and depth are used
 *                    for both ends).
 * @param goal_query Destination position; only x, y and z are read.
 * @param output_path Receives the route points.
 * @param mode Trace mode passed to func_80062820 for the direct walk.
 * @return Number of points written (1 for a direct route), or one of the
 *         FIELD_COLLISION_PATH_ERROR_* codes.
 */
s32 func_80060F58(FieldCollisionQuery* start_query, FieldCollisionQuery* goal_query, FieldCollisionPathPoint* output_path, s32 mode)
{
    /* Separate pseudo for &path[0][0]; `read = &path[0][0]` directly: 99.90%. */
    s32* path_base;
    s32 via_x;
    s32 raw_depth;
    s32 last_queue;
    /* sp+0x0010: the four BFS rings, later the route tile / x / z columns. */
    s32 path[4][FIELD_COLLISION_PATH_BUCKET_LEN];
    /* sp+0x4010: entries queued in each ring. */
    s32 bucket_len[4];
    /* sp+0x4020: request block for func_80062820. */
    FieldCollisionTraceRequest rec;
    s32 start_group;
    s32 goal_group;
    s32 col_shift;
    /*
     * Start/goal cells. The smoothing reuses these four stack slots for its
     * first and third point; separate locals grow the frame (99.70%).
     */
    s32 start_col;
    s32 start_row;
    s32 goal_col;
    s32 goal_row;
    /* Start cell origin; also point scratch in the string pull. */
    s32 start_x;
    s32 start_z;
    s32 goal_x;
    s32 goal_z;
    u8 try_quarter;
    u32 quarter_x;
    /* quarter_x - dx + dx keeps dx live; `via_x = quarter_x`: 99.80%. */
    u32 restored_x_offset;
    s32* buckets;
    u32* bucket_lens;
    FieldScene* scene;
    u8 goal_mark;
    s32 goal_half_w;
    s32 goal_x_raw;
    s32 goal_half_d;
    s32 goal_z_raw;
    s32 start_half_w;
    s32 start_x_raw;
    s32 start_half_d;
    s32 start_z_raw;
    s32 row_shift;
    u32 columns;
    s32 rows;
    s32 goal_height;
    u8 goal_groups;
    s16 goal_group_id;
    /*
     * Goal tile, BFS neighbour, walk step and the smoothing's third tile: one
     * local (a split of any role reallocates, 94.5-99.5%).
     */
    u8* near_tile;
    s32 start_height;
    u8 start_groups;
    s16 start_group_id;
    u8* tile;
    s32 ring;
    u32 queue_len;
    /* Tile address of the goal neighbour that ended the search (0 = none). */
    s32 route_value;
    /* Wave stamp; in the walk back, the best neighbour stamp so far. */
    u8 stamp;
    /* NO_ROUTE code, then the next ring index (int-return delay-slot shape). */
    s32 result;
    u32 deferred_len;
    s32* read;
    s32 next_ring;
    /* Ring cursors; the smoothing also parks quarter-point ints in them. */
    s32* deferred_out;
    s32* next_out;
    u32 entry;
    u32 tile_size;
    u32 dirs;
    /* Directions claimed from the entry (BFS) or ruled out (walk back). */
    s32 taken;
    u32 up_raw;
    u32 up_val;
    u32 down_raw;
    u32 down_val;
    u32 left_raw;
    u32 left_val;
    u32 right_raw;
    u32 right_val;
    u8* ul_row;
    u32 ul_raw;
    u32 ul_val;
    u32 side_val;
    /* Corner test result, then "route needs / got a better second point". */
    u8 ok;
    s32 ul_entry;
    u8* ur_row;
    u32 ur_raw;
    u32 ur_val;
    s32 ur_entry;
    u8* dl_row;
    u32 dl_raw;
    u32 dl_val;
    s32 dl_entry;
    u8* dr_row;
    u32 dr_raw;
    u32 dr_val;
    s32 dr_entry;
    s32 prev_ring;
    /* Loop counter; also the quarter-point dz scratch. */
    s32 step;
    s32 mark_entry;
    u32 plane_size;
    s32 tile_map;
    u32 route_offset;
    /* Walk-back step code, 3x3 neighbourhood numbered 1..8 row by row (no centre). */
    s32 dir;
    s32 last_dir;
    u8 cur_stamp;
    u32 nb_raw;
    u32 walk_left;
    u32 walk_right;
    u8* up_tile;
    u32 walk_up;
    u8* down_tile;
    u32 walk_down;
    u32 walk_ul;
    u32 walk_ur;
    u32 walk_dl;
    u32 walk_dr;
    u32 plane_len;
    u32 cell_offset;
    s32* out_point;
    s32* path_end;
    s32* far_x;
    s32* far_z;
    s32 mid_x;
    s32 mid_z;
    s32 mid2_x;
    s32 mid2_z;
    s32 quarter_dx;
    s32 quarter_dz;
    s32 back_mid_x;
    s32 back_mid_z;
    s32 back_mid2_x;
    s32 back_mid2_z;
    s32 back_quarter_dx;
    s32 back_quarter_dz;
    FieldCollisionPathPoint* out;
    s32 point_x;
    s32 point_z;
    /* Pop countdown; in the smoothing, the second point's z. */
    s32 count;

    scene = g_field_scene.scene;
    if (scene->unk28 == 0)
    {
        if (scene->unk41 != 0)
        {
            return FIELD_COLLISION_PATH_ERROR_GROUPS;
        }
    }
    else
    {
        col_shift = 3;
        tile_size = scene->unk40;
        if (tile_size == 4)
        {
            col_shift = 2;
        }
        goal_half_w = (s16)start_query->width >> 1;
        goal_x_raw = goal_query->x;
        goal_x = (goal_x_raw >= 0 ? goal_x_raw >> 8 : (goal_x_raw + 0xFF) >> 8) - goal_half_w;
        goal_half_d = (s16)start_query->depth >> 1;
        goal_z_raw = goal_query->z;
        goal_z = (goal_z_raw >= 0 ? goal_z_raw >> 8 : (goal_z_raw + 0xFF) >> 8) - goal_half_d;
        start_half_w = (s16)start_query->width >> 1;
        start_x_raw = start_query->x;
        start_x = (start_x_raw >= 0 ? start_x_raw >> 8 : (start_x_raw + 0xFF) >> 8) - start_half_w;
        start_half_d = (s16)start_query->depth >> 1;
        start_z_raw = start_query->z;
        start_z = (start_z_raw >= 0 ? start_z_raw >> 8 : (start_z_raw + 0xFF) >> 8) - start_half_d;
        row_shift = col_shift + 1;
        goal_col = (goal_x >> col_shift) + 2;
        goal_row = (goal_z >> row_shift) + 2;
        start_col = (start_x >> col_shift) + 2;
        start_row = (start_z >> row_shift) + 2;
        columns = (u16)scene->unk46;
        rows = (u16)scene->unk48;
        if ((start_col <= 0) || (start_row <= 0) || (goal_col <= 0) || (goal_row <= 0) || (start_col >= (s32)columns - 1) ||
            (start_row >= rows - 1) || (goal_col >= (s32)columns - 1) || (goal_row >= rows - 1))
        {
            return FIELD_COLLISION_PATH_ERROR_BOUNDS;
        }
        goal_height = FIELD_COLLISION_CELL(goal_query->y);
        goal_groups = scene->unk41;
        goal_group = goal_groups - 1;
        for (count = 0; count != goal_groups; count++)
        {
            goal_group_id = scene->unk4A[count];
            if (goal_height < goal_group_id)
            {
                if (count != 0)
                {
                    count -= 1;
                }
                goal_group = count;
                break;
            }
            if (goal_group_id == goal_height)
            {
                goal_group = count;
                break;
            }
        }
        near_tile = (u8*)(scene->unk2C + ((u16)scene->unk44 * goal_group) + (columns * goal_row) + goal_col);
        *near_tile = FIELD_COLLISION_TILE_GOAL;
        start_height = FIELD_COLLISION_CELL(start_query->y);
        start_groups = scene->unk41;
        start_group = start_groups - 1;
        for (count = 0; count != start_groups; count++)
        {
            start_group_id = scene->unk4A[count];
            if (start_height < start_group_id)
            {
                if (count != 0)
                {
                    count -= 1;
                }
                start_group = count;
                break;
            }
            if (start_group_id == start_height)
            {
                start_group = count;
                break;
            }
        }
        tile = (u8*)(scene->unk2C + ((u16)scene->unk44 * start_group) + (columns * start_row) + start_col);
        if (*tile != FIELD_COLLISION_TILE_GOAL)
        {
            *tile = FIELD_COLLISION_TILE_START;
            rec.start_x = goal_x;
            rec.start_z = goal_z;
            rec.end_x = start_x;
            path[0][0] = (s32)tile | FIELD_COLLISION_PATH_DIRS(FIELD_COLLISION_DIRS_ALL);
            rec.tile_base = (s32)near_tile;
            rec.end_z = start_z;
            rec.footprint_width = (s16)start_query->width;
            rec.footprint_depth = (s16)start_query->depth;
            rec.tile_size = (s32)tile_size;
            rec.col_shift = col_shift;
            rec.stamp = FIELD_COLLISION_TILE_GOAL;
            rec.mode = mode;
            /* Straight walk blocked: search. */
            if (func_80062820(&rec) == 0)
            {
                /* Bucketed BFS from the start tile, one ring per wave. */
                ring = 0;
                goal_mark = FIELD_COLLISION_TILE_GOAL;
                queue_len = bucket_len[0] = 1;
                route_value = 0;
                stamp = FIELD_COLLISION_TILE_STAMP_FIRST;
                buckets = &path[0][0];
                bucket_lens = (u32*)bucket_len;
                bucket_len[3] = 0;
                bucket_len[2] = 0;
                bucket_len[1] = 0;
                while (1)
                {
                    count = queue_len;
                    if ((count == 0) && (bucket_len[0] == 0) && (bucket_len[1] == 0) && (bucket_len[2] == 0) &&
                        (bucket_len[3] == 0))
                    {
                        return FIELD_COLLISION_PATH_ERROR_NO_ROUTE;
                    }
                    read = buckets + ring * FIELD_COLLISION_PATH_BUCKET_LEN + (queue_len - 1);
                    deferred_len = 0;
                    count -= 1;
                    next_ring = (ring + 1) & 3;
                    queue_len = FIELD_COLLISION_PATH_LEN_SLOT(bucket_lens, next_ring);
                    deferred_out = buckets + ((ring + 3) & 3) * FIELD_COLLISION_PATH_BUCKET_LEN;
                    next_out = buckets + next_ring * FIELD_COLLISION_PATH_BUCKET_LEN + queue_len;
                    if (count != -1)
                    {
                        do
                        {
                            if ((queue_len >= FIELD_COLLISION_PATH_PUSH_LIMIT) || (deferred_len >= FIELD_COLLISION_PATH_PUSH_LIMIT))
                            {
                                return FIELD_COLLISION_PATH_ERROR_OVERFLOW;
                            }
                            entry = *read--;
                            tile = (u8*)(entry & FIELD_COLLISION_PATH_TILE_MASK);
                            dirs = entry >> FIELD_COLLISION_PATH_DIR_SHIFT;
                            taken = 0;
                            if ((dirs & (FIELD_COLLISION_DIR_DEFERRED | FIELD_COLLISION_DIR_SLOW)) == (FIELD_COLLISION_DIR_DEFERRED | FIELD_COLLISION_DIR_SLOW))
                            {
                                deferred_len += count + 1;
                                entry = ~FIELD_COLLISION_PATH_DIRS(FIELD_COLLISION_DIR_DEFERRED);
                                do
                                {
                                    *deferred_out = read[1] & entry;
                                    read -= 1;
                                    count -= 1;
                                    deferred_out += 1;
                                } while (count != -1);
                                break;
                            }
                            if (dirs & FIELD_COLLISION_DIR_UP)
                            {
                                near_tile = tile - columns;
                                up_raw = *near_tile;
                                up_val = up_raw & 0xFF;
                                if (FIELD_COLLISION_PATH_CAN_ENTER(up_val, stamp, dirs))
                                {
                                    if (up_val != FIELD_COLLISION_TILE_TOUCHED)
                                    {
                                        *next_out = (s32)near_tile | FIELD_COLLISION_PATH_DIRS(FIELD_COLLISION_DIRS_TOP | FIELD_COLLISION_DIR_LEFT | FIELD_COLLISION_DIR_RIGHT);
                                        next_out += 1;
                                        queue_len += 1;
                                        taken = FIELD_COLLISION_DIR_UP;
                                        *near_tile = stamp;
                                    }
                                    else
                                    {
                                        *deferred_out = (s32)near_tile | FIELD_COLLISION_PATH_DIRS(FIELD_COLLISION_DIR_DEFERRED | FIELD_COLLISION_DIR_SLOW | FIELD_COLLISION_DIRS_TOP | FIELD_COLLISION_DIR_LEFT | FIELD_COLLISION_DIR_RIGHT);
                                        deferred_out += 1;
                                        deferred_len += 1;
                                        taken = FIELD_COLLISION_DIR_UP | FIELD_COLLISION_DIR_TOUCHED(FIELD_COLLISION_DIR_UP);
                                        *near_tile = FIELD_COLLISION_TILE_DEFERRED;
                                    }
                                }
                                else if (up_raw == goal_mark)
                                {
                                    route_value = (s32)near_tile;
                                    if (dirs & FIELD_COLLISION_DIR_SLOW)
                                    {
                                        taken = FIELD_COLLISION_DIR_UP;
                                    }
                                }
                            }
                            if (dirs & FIELD_COLLISION_DIR_DOWN)
                            {
                                near_tile = tile + columns;
                                down_raw = *near_tile;
                                down_val = down_raw & 0xFF;
                                if (FIELD_COLLISION_PATH_CAN_ENTER(down_val, stamp, dirs))
                                {
                                    if (down_val != FIELD_COLLISION_TILE_TOUCHED)
                                    {
                                        *next_out = (s32)near_tile | FIELD_COLLISION_PATH_DIRS(FIELD_COLLISION_DIRS_BOTTOM | FIELD_COLLISION_DIR_LEFT | FIELD_COLLISION_DIR_RIGHT);
                                        next_out += 1;
                                        queue_len += 1;
                                        taken |= FIELD_COLLISION_DIR_DOWN;
                                        *near_tile = stamp;
                                    }
                                    else
                                    {
                                        *deferred_out = (s32)near_tile | FIELD_COLLISION_PATH_DIRS(FIELD_COLLISION_DIR_DEFERRED | FIELD_COLLISION_DIR_SLOW | FIELD_COLLISION_DIRS_BOTTOM | FIELD_COLLISION_DIR_LEFT | FIELD_COLLISION_DIR_RIGHT);
                                        deferred_out += 1;
                                        deferred_len += 1;
                                        taken |= FIELD_COLLISION_DIR_DOWN | FIELD_COLLISION_DIR_TOUCHED(FIELD_COLLISION_DIR_DOWN);
                                        *near_tile = FIELD_COLLISION_TILE_DEFERRED;
                                    }
                                }
                                else if (down_raw == goal_mark)
                                {
                                    route_value = (s32)near_tile;
                                    if (dirs & FIELD_COLLISION_DIR_SLOW)
                                    {
                                        taken |= FIELD_COLLISION_DIR_DOWN;
                                    }
                                }
                            }
                            if (dirs & FIELD_COLLISION_DIR_LEFT)
                            {
                                left_raw = tile[-1];
                                left_val = left_raw & 0xFF;
                                near_tile = tile - 1;
                                if (FIELD_COLLISION_PATH_CAN_ENTER(left_val, stamp, dirs))
                                {
                                    if (left_val != FIELD_COLLISION_TILE_TOUCHED)
                                    {
                                        *next_out = (s32)near_tile | FIELD_COLLISION_PATH_DIRS(FIELD_COLLISION_DIRS_LEFT | FIELD_COLLISION_DIR_UP | FIELD_COLLISION_DIR_DOWN);
                                        next_out += 1;
                                        queue_len += 1;
                                        taken |= FIELD_COLLISION_DIR_LEFT;
                                        tile[-1] = stamp;
                                    }
                                    else
                                    {
                                        *deferred_out = (s32)near_tile | FIELD_COLLISION_PATH_DIRS(FIELD_COLLISION_DIR_DEFERRED | FIELD_COLLISION_DIR_SLOW | FIELD_COLLISION_DIRS_LEFT | FIELD_COLLISION_DIR_UP | FIELD_COLLISION_DIR_DOWN);
                                        deferred_out += 1;
                                        deferred_len += 1;
                                        taken |= FIELD_COLLISION_DIR_LEFT | FIELD_COLLISION_DIR_TOUCHED(FIELD_COLLISION_DIR_LEFT);
                                        tile[-1] = FIELD_COLLISION_TILE_DEFERRED;
                                    }
                                }
                                else if (left_raw == goal_mark)
                                {
                                    route_value = (s32)near_tile;
                                    if (dirs & FIELD_COLLISION_DIR_SLOW)
                                    {
                                        taken |= FIELD_COLLISION_DIR_LEFT;
                                    }
                                }
                            }
                            if (dirs & FIELD_COLLISION_DIR_RIGHT)
                            {
                                right_raw = tile[1];
                                right_val = right_raw & 0xFF;
                                near_tile = tile + 1;
                                if (FIELD_COLLISION_PATH_CAN_ENTER(right_val, stamp, dirs))
                                {
                                    if (right_val != FIELD_COLLISION_TILE_TOUCHED)
                                    {
                                        *next_out = (s32)near_tile | FIELD_COLLISION_PATH_DIRS(FIELD_COLLISION_DIRS_RIGHT | FIELD_COLLISION_DIR_UP | FIELD_COLLISION_DIR_DOWN);
                                        next_out += 1;
                                        queue_len += 1;
                                        taken |= FIELD_COLLISION_DIR_RIGHT;
                                        tile[1] = stamp;
                                    }
                                    else
                                    {
                                        *deferred_out = (s32)near_tile | FIELD_COLLISION_PATH_DIRS(FIELD_COLLISION_DIR_DEFERRED | FIELD_COLLISION_DIR_SLOW | FIELD_COLLISION_DIRS_RIGHT | FIELD_COLLISION_DIR_UP | FIELD_COLLISION_DIR_DOWN);
                                        deferred_out += 1;
                                        deferred_len += 1;
                                        taken |= FIELD_COLLISION_DIR_RIGHT | FIELD_COLLISION_DIR_TOUCHED(FIELD_COLLISION_DIR_RIGHT);
                                        tile[1] = FIELD_COLLISION_TILE_DEFERRED;
                                    }
                                }
                                else if (right_raw == goal_mark)
                                {
                                    route_value = (s32)near_tile;
                                    if (dirs & FIELD_COLLISION_DIR_SLOW)
                                    {
                                        taken |= FIELD_COLLISION_DIR_RIGHT;
                                    }
                                }
                            }

                            if (dirs & FIELD_COLLISION_DIR_UL)
                            {
                                ul_row = tile - columns;
                                ul_raw = ul_row[-1];
                                near_tile = ul_row - 1;
                                ul_val = ul_raw & 0xFF;
                                if (FIELD_COLLISION_PATH_CAN_ENTER(ul_val, stamp, dirs))
                                {
                                    FIELD_COLLISION_CORNER_TEST(ok, side_val, near_tile[1], tile[-1]);
                                    if (ok != 0)
                                    {
                                        ul_entry = (s32)near_tile | FIELD_COLLISION_PATH_DIRS(FIELD_COLLISION_DIR_UL | FIELD_COLLISION_DIR_UP | FIELD_COLLISION_DIR_LEFT);
                                        if (!(taken & FIELD_COLLISION_DIR_UP))
                                        {
                                            ul_entry |= FIELD_COLLISION_PATH_DIRS(FIELD_COLLISION_DIR_UP | FIELD_COLLISION_DIR_UR);
                                        }
                                        if ((taken & FIELD_COLLISION_DIR_LEFT) == 0)
                                        {
                                            ul_entry |= FIELD_COLLISION_PATH_DIRS(FIELD_COLLISION_DIR_LEFT | FIELD_COLLISION_DIR_DL);
                                        }
                                        taken |= FIELD_COLLISION_DIR_UL;
                                        if (ul_raw != FIELD_COLLISION_TILE_TOUCHED)
                                        {
                                            *next_out = ul_entry;
                                            next_out += 1;
                                            queue_len += 1;
                                            *near_tile = stamp;
                                        }
                                        else
                                        {
                                            *deferred_out = ul_entry | FIELD_COLLISION_PATH_DIRS(FIELD_COLLISION_DIR_DEFERRED | FIELD_COLLISION_DIR_SLOW);
                                            deferred_out += 1;
                                            deferred_len += 1;
                                            *near_tile = FIELD_COLLISION_TILE_DEFERRED;
                                        }
                                    }
                                }
                                else if (ul_raw == goal_mark)
                                {
                                    route_value = (s32)near_tile;
                                    if (dirs & FIELD_COLLISION_DIR_SLOW)
                                    {
                                        taken |= FIELD_COLLISION_DIR_UL;
                                    }
                                }
                            }

                            if (dirs & FIELD_COLLISION_DIR_UR)
                            {
                                ur_row = tile - columns;
                                ur_raw = ur_row[1];
                                near_tile = ur_row + 1;
                                ur_val = ur_raw & 0xFF;
                                if (FIELD_COLLISION_PATH_CAN_ENTER(ur_val, stamp, dirs))
                                {
                                    FIELD_COLLISION_CORNER_TEST(ok, side_val, near_tile[-1], tile[1]);
                                    if (ok != 0)
                                    {
                                        ur_entry = (s32)near_tile | FIELD_COLLISION_PATH_DIRS(FIELD_COLLISION_DIR_UP | FIELD_COLLISION_DIR_UR | FIELD_COLLISION_DIR_RIGHT);
                                        if (!(taken & FIELD_COLLISION_DIR_UP))
                                        {
                                            ur_entry |= FIELD_COLLISION_PATH_DIRS(FIELD_COLLISION_DIR_UL | FIELD_COLLISION_DIR_UP);
                                        }
                                        if ((taken & FIELD_COLLISION_DIR_RIGHT) == 0)
                                        {
                                            ur_entry |= FIELD_COLLISION_PATH_DIRS(FIELD_COLLISION_DIR_RIGHT | FIELD_COLLISION_DIR_DR);
                                        }
                                        taken |= FIELD_COLLISION_DIR_UR;
                                        if (ur_raw != FIELD_COLLISION_TILE_TOUCHED)
                                        {
                                            *next_out = ur_entry;
                                            next_out += 1;
                                            queue_len += 1;
                                            *near_tile = stamp;
                                        }
                                        else
                                        {
                                            *deferred_out = ur_entry | FIELD_COLLISION_PATH_DIRS(FIELD_COLLISION_DIR_DEFERRED | FIELD_COLLISION_DIR_SLOW);
                                            deferred_out += 1;
                                            deferred_len += 1;
                                            *near_tile = FIELD_COLLISION_TILE_DEFERRED;
                                        }
                                    }
                                }
                                else if (ur_raw == goal_mark)
                                {
                                    route_value = (s32)near_tile;
                                    if (dirs & FIELD_COLLISION_DIR_SLOW)
                                    {
                                        taken |= FIELD_COLLISION_DIR_UR;
                                    }
                                }
                            }

                            if (dirs & FIELD_COLLISION_DIR_DL)
                            {
                                dl_row = tile + columns;
                                dl_raw = dl_row[-1];
                                near_tile = dl_row - 1;
                                dl_val = dl_raw & 0xFF;
                                if (FIELD_COLLISION_PATH_CAN_ENTER(dl_val, stamp, dirs))
                                {
                                    FIELD_COLLISION_CORNER_TEST(ok, side_val, near_tile[1], tile[-1]);
                                    if (ok != 0)
                                    {
                                        dl_entry = (s32)near_tile | FIELD_COLLISION_PATH_DIRS(FIELD_COLLISION_DIR_LEFT | FIELD_COLLISION_DIR_DL | FIELD_COLLISION_DIR_DOWN);
                                        if (!(taken & FIELD_COLLISION_DIR_DOWN))
                                        {
                                            dl_entry |= FIELD_COLLISION_PATH_DIRS(FIELD_COLLISION_DIR_DOWN | FIELD_COLLISION_DIR_DR);
                                        }
                                        if ((taken & FIELD_COLLISION_DIR_LEFT) == 0)
                                        {
                                            dl_entry |= FIELD_COLLISION_PATH_DIRS(FIELD_COLLISION_DIR_UL | FIELD_COLLISION_DIR_LEFT);
                                        }
                                        taken |= FIELD_COLLISION_DIR_DL;
                                        if (dl_raw != FIELD_COLLISION_TILE_TOUCHED)
                                        {
                                            *next_out = dl_entry;
                                            next_out += 1;
                                            queue_len += 1;
                                            *near_tile = stamp;
                                        }
                                        else
                                        {
                                            *deferred_out = dl_entry | FIELD_COLLISION_PATH_DIRS(FIELD_COLLISION_DIR_DEFERRED | FIELD_COLLISION_DIR_SLOW);
                                            deferred_out += 1;
                                            deferred_len += 1;
                                            *near_tile = FIELD_COLLISION_TILE_DEFERRED;
                                        }
                                    }
                                }
                                else if (dl_raw == goal_mark)
                                {
                                    route_value = (s32)near_tile;
                                    if (dirs & FIELD_COLLISION_DIR_SLOW)
                                    {
                                        taken |= FIELD_COLLISION_DIR_DL;
                                    }
                                }
                            }

                            if (dirs & FIELD_COLLISION_DIR_DR)
                            {
                                dr_row = tile + columns;
                                dr_raw = dr_row[1];
                                near_tile = dr_row + 1;
                                dr_val = dr_raw & 0xFF;
                                if (FIELD_COLLISION_PATH_CAN_ENTER(dr_val, stamp, dirs))
                                {
                                    FIELD_COLLISION_CORNER_TEST(ok, side_val, near_tile[-1], tile[1]);
                                    if (ok != 0)
                                    {
                                        dr_entry = (s32)near_tile | FIELD_COLLISION_PATH_DIRS(FIELD_COLLISION_DIR_RIGHT | FIELD_COLLISION_DIR_DOWN | FIELD_COLLISION_DIR_DR);
                                        if (!(taken & FIELD_COLLISION_DIR_DOWN))
                                        {
                                            dr_entry |= FIELD_COLLISION_PATH_DIRS(FIELD_COLLISION_DIR_DL | FIELD_COLLISION_DIR_DOWN);
                                        }
                                        if ((taken & FIELD_COLLISION_DIR_RIGHT) == 0)
                                        {
                                            dr_entry |= FIELD_COLLISION_PATH_DIRS(FIELD_COLLISION_DIR_UR | FIELD_COLLISION_DIR_RIGHT);
                                        }
                                        taken |= FIELD_COLLISION_DIR_DR;
                                        if (dr_raw != FIELD_COLLISION_TILE_TOUCHED)
                                        {
                                            *next_out = dr_entry;
                                            next_out += 1;
                                            queue_len += 1;
                                            *near_tile = stamp;
                                        }
                                        else
                                        {
                                            *deferred_out = dr_entry | FIELD_COLLISION_PATH_DIRS(FIELD_COLLISION_DIR_DEFERRED | FIELD_COLLISION_DIR_SLOW);
                                            deferred_out += 1;
                                            deferred_len += 1;
                                            *near_tile = FIELD_COLLISION_TILE_DEFERRED;
                                        }
                                    }
                                }
                                else if (dr_raw == goal_mark)
                                {
                                    route_value = (s32)near_tile;
                                    if (dirs & FIELD_COLLISION_DIR_SLOW)
                                    {
                                        taken |= FIELD_COLLISION_DIR_DR;
                                    }
                                }
                            }
                            if ((dirs & FIELD_COLLISION_DIR_SLOW) && (taken != 0))
                            {
                                tile[0] = stamp + 1;
                            }
                            count -= 1;
                        } while (count != -1);
                    }
                    stamp -= 1;
                    if ((stamp < FIELD_COLLISION_TILE_STAMP_MIN) && (route_value == 0))
                    {
                        return FIELD_COLLISION_PATH_ERROR_NO_ROUTE;
                    }
                    FIELD_COLLISION_PATH_LEN_SLOT(bucket_lens, ring) = 0;
                    result = (ring + 1) & 3;
                    prev_ring = ring + 3;
                    ring = result;
                    FIELD_COLLISION_PATH_LEN_SLOT(bucket_lens, result) = queue_len;
                    FIELD_COLLISION_PATH_LEN_SLOT(bucket_lens, prev_ring & 3) = deferred_len;
                    if (route_value != 0)
                    {
                        break;
                    }
                }

                /* Touched tiles still queued in the next three rings become deferred again. */
                step = 2;
                do
                {
                    read = &path[ring][queue_len - 1];
                    count = queue_len - 1;
                    if (count != -1)
                    {
                        last_queue = -1;
                        do
                        {
                            mark_entry = *read;
                            read -= 1;
                            if (mark_entry & FIELD_COLLISION_PATH_DIRS(FIELD_COLLISION_DIR_SLOW))
                            {
                                tile = (u8*)(mark_entry & FIELD_COLLISION_PATH_TILE_MASK);
                                *tile = FIELD_COLLISION_TILE_DEFERRED;
                            }
                            count -= 1;
                        } while (count != last_queue);
                    }
                    ring = (ring + 1) & 3;
                    queue_len = bucket_len[ring];
                    step -= 1;
                } while (step != -1);
                /* Link the goal to the route tile, then walk the stamps back to the start. */
                path_base = &path[0][0];
                plane_size = (u16)scene->unk44;
                tile_map = scene->unk2C;
                tile = (u8*)(tile_map + (plane_size * goal_group) + (columns * goal_row) + goal_col);
                read = path_base;
                if (tile != (u8*)route_value)
                {
                    route_offset = route_value - tile_map;
                    while (route_offset >= plane_size)
                    {
                        route_offset -= plane_size;
                    }
                    rec.start_x = goal_x;
                    rec.tile_base = (s32)tile;
                    rec.stamp = 4;
                    rec.start_z = goal_z;
                    rec.end_x = ((route_offset % columns) - 2) << col_shift;
                    rec.end_z = ((route_offset / columns) - 2) << (col_shift + 1);
                    if (func_80062820(&rec) != 0)
                    {
                        queue_len = 1;
                        *read = (s32)tile;
                        read += 1;
                        *tile = FIELD_COLLISION_TILE_GOAL;
                        tile = (u8*)route_value;
                    }
                    else
                    {
                        rec.goal_tile = route_value;
                        rec.stamp = 4;
                        rec.mode = FIELD_COLLISION_TRACE_STOP_AT_GOAL;
                        rec.end_x = start_x;
                        rec.end_z = start_z;
                        if (func_80062820(&rec) != 0)
                        {
                            queue_len = 1;
                            *read = (s32)tile;
                            read += 1;
                            *tile = FIELD_COLLISION_TILE_GOAL;
                            tile = (u8*)route_value;
                        }
                        else
                        {
                            queue_len = 0;
                            *tile = FIELD_COLLISION_TILE_GOAL;
                        }
                        rec.mode = mode;
                    }
                    ok = 1;
                }
                else
                {
                    queue_len = 0;
                    ok = 0;
                }
                dir = 0;
                last_dir = 0;
                cur_stamp = 0;
                do
                {
                    stamp = 0;
                    nb_raw = tile[-1];
                    taken = 0;
                    if (FIELD_COLLISION_TILE_IS_STEP(nb_raw) && (walk_left = nb_raw & 0xFF, cur_stamp < walk_left) &&
                        (stamp < walk_left))
                    {
                        near_tile = tile - 1;
                        dir = 4;
                        stamp = nb_raw;
                    }
                    else
                    {
                        taken |= FIELD_COLLISION_DIR_LEFT;
                    }
                    nb_raw = tile[1];
                    walk_right = nb_raw & 0xFF;
                    if (FIELD_COLLISION_TILE_IS_STEP(nb_raw) && (cur_stamp < walk_right) && (stamp < walk_right))
                    {
                        near_tile = tile + 1;
                        dir = 5;
                        stamp = nb_raw;
                    }
                    else
                    {
                        taken |= FIELD_COLLISION_DIR_RIGHT;
                    }
                    up_tile = tile - columns;
                    nb_raw = *up_tile;
                    walk_up = nb_raw & 0xFF;
                    if (FIELD_COLLISION_TILE_IS_STEP(nb_raw) && (cur_stamp < walk_up) && (stamp < walk_up))
                    {
                        near_tile = up_tile;
                        dir = 2;
                        stamp = nb_raw;
                    }
                    else
                    {
                        taken |= FIELD_COLLISION_DIR_UP;
                    }
                    down_tile = tile + columns;
                    nb_raw = *down_tile;
                    walk_down = nb_raw & 0xFF;
                    if (FIELD_COLLISION_TILE_IS_STEP(nb_raw) && (cur_stamp < walk_down) && (stamp < walk_down))
                    {
                        near_tile = down_tile;
                        dir = 7;
                        stamp = nb_raw;
                    }
                    else
                    {
                        taken |= FIELD_COLLISION_DIR_DOWN;
                    }
                    nb_raw = (tile - columns)[-1];
                    if (!(taken & (FIELD_COLLISION_DIR_UP | FIELD_COLLISION_DIR_LEFT)) || (stamp == 0))
                    {
                        walk_ul = nb_raw & 0xFF;
                        if (walk_ul < FIELD_COLLISION_TILE_GOAL)
                        {
                            if ((walk_ul >= FIELD_COLLISION_TILE_STAMP_MIN) && (cur_stamp < walk_ul) && (stamp < walk_ul))
                            {
                                near_tile = (tile - columns) - 1;
                                dir = 1;
                                stamp = nb_raw;
                            }
                        }
                    }
                    nb_raw = (tile - columns)[1];
                    if (!(taken & (FIELD_COLLISION_DIR_UP | FIELD_COLLISION_DIR_RIGHT)) || (stamp == 0))
                    {
                        walk_ur = nb_raw & 0xFF;
                        if (walk_ur < FIELD_COLLISION_TILE_GOAL)
                        {
                            if ((walk_ur >= FIELD_COLLISION_TILE_STAMP_MIN) && (cur_stamp < walk_ur) && (stamp < walk_ur))
                            {
                                near_tile = (tile - columns) + 1;
                                dir = 3;
                                stamp = nb_raw;
                            }
                        }
                    }
                    nb_raw = (tile + columns)[-1];
                    if (!(taken & (FIELD_COLLISION_DIR_DOWN | FIELD_COLLISION_DIR_LEFT)) || (stamp == 0))
                    {
                        walk_dl = nb_raw & 0xFF;
                        if (walk_dl < FIELD_COLLISION_TILE_GOAL)
                        {
                            if ((walk_dl >= FIELD_COLLISION_TILE_STAMP_MIN) && (cur_stamp < walk_dl) && (stamp < walk_dl))
                            {
                                near_tile = (tile + columns) - 1;
                                dir = 6;
                                stamp = nb_raw;
                            }
                        }
                    }
                    nb_raw = (tile + columns)[1];
                    if (!(taken & (FIELD_COLLISION_DIR_DOWN | FIELD_COLLISION_DIR_RIGHT)) || (stamp == 0))
                    {
                        walk_dr = nb_raw & 0xFF;
                        if (walk_dr < FIELD_COLLISION_TILE_GOAL)
                        {
                            if ((walk_dr >= FIELD_COLLISION_TILE_STAMP_MIN) && (cur_stamp < walk_dr) && (stamp < walk_dr))
                            {
                                near_tile = (tile + columns) + 1;
                                dir = 8;
                                stamp = nb_raw;
                            }
                        }
                        result = FIELD_COLLISION_PATH_ERROR_DEAD_END;
                        if (stamp == 0)
                        {
                            return result;
                        }
                    }
                    if (dir != last_dir)
                    {
                        last_dir = dir;
                        *read = (s32)tile;
                        queue_len += 1;
                        read += 1;
                        if (queue_len >= FIELD_COLLISION_PATH_BUCKET_LEN)
                        {
                            return FIELD_COLLISION_PATH_ERROR_OVERFLOW;
                        }
                        cur_stamp = stamp;
                    }
                    else
                    {
                        cur_stamp = stamp;
                    }
                    tile = near_tile;
                } while (cur_stamp != FIELD_COLLISION_TILE_START);
                /* Turn the kept tiles into cell points (goal first, start last). */
                count = queue_len;
                next_out = &path[1][1];
                deferred_out = &path[2][1];
                step = count - 2;
                tile = (u8*)(scene->unk2C + ((u16)scene->unk44 * start_group) + (columns * start_row) + start_col);
                *read = (s32)tile;
                read = &path[0][0];
                rec.stamp = 0;
                path[1][0] = goal_x;
                path[2][0] = goal_z;
                if (step != -1)
                {
                    do
                    {
                        read += 1;
                        plane_len = (u16)scene->unk44;
                        tile = (u8*)*read;
                        cell_offset = (s32)tile - scene->unk2C;
                        while (cell_offset >= plane_len)
                        {
                            cell_offset -= plane_len;
                        }
                        step -= 1;
                        *next_out = ((cell_offset % columns) - 2) << col_shift;
                        next_out += 1;
                        *deferred_out = ((cell_offset / columns) - 2) << (col_shift + 1);
                        deferred_out += 1;
                    } while (step != -1);
                    read = &path[0][0];
                }
                /* Greedy string pull: jump to the farthest point a clear trace reaches. */
                queue_len = 0;
                count -= 1;
                *next_out = start_x;
                next_out = &path[1][0];
                *deferred_out = start_z;
                deferred_out = &path[2][0];
                if (count != -1)
                {
                    do
                    {
                        tile = (u8*)*read;
                        read += 1;
                        start_x = *next_out;
                        next_out += 1;
                        out_point = &path[0][queue_len++];
                        start_z = *deferred_out;
                        deferred_out += 1;
                        out_point[0] = (s32)tile;
                        out_point[FIELD_COLLISION_PATH_BUCKET_LEN] = start_x;
                        out_point[2 * FIELD_COLLISION_PATH_BUCKET_LEN] = start_z;
                        if (count != 0)
                        {
                            step = count;
                            rec.tile_base = (s32)tile;
                            rec.start_x = start_x;
                            rec.start_z = start_z;
                            do
                            {
                                far_x = FIELD_COLLISION_PATH_AHEAD(next_out, step);
                                far_z = FIELD_COLLISION_PATH_AHEAD(deferred_out, step);
                                rec.end_x = *far_x;
                                rec.end_z = *far_z;
                                if (func_80062820(&rec) != 0)
                                {
                                    read += step;
                                    next_out = far_x;
                                    deferred_out = far_z;
                                    count -= step;
                                    break;
                                }
                                step -= 1;
                            } while (step != 0);
                        }
                        count -= 1;
                    } while (count != -1);
                }
                /* Nudge the second point towards midpoints and quarter points. */
                if ((ok != 0) && (queue_len >= 2U))
                {
                    ok = 0;
                    try_quarter = 1;
                    tile = (u8*)path[0][0];
                    start_col = path[1][0];
                    start_row = path[2][0];
                    path_end = &path[0][queue_len];
                    path_end[0] = *read;
                    /*
                     * Reused as the tile column base, like next_out and
                     * deferred_out below; a separate pointer drops read's
                     * weight below ok's and moves via_x off $s8.
                     */
                    read = &path[0][0];
                    path_end[FIELD_COLLISION_PATH_BUCKET_LEN] = *next_out;
                    next_out = &path[1][0];
                    path_end[2 * FIELD_COLLISION_PATH_BUCKET_LEN] = *deferred_out;
                    deferred_out = &path[2][0];
                    near_tile = (u8*)read[2];
                    via_x = next_out[1];
                    count = deferred_out[1];
                    goal_col = next_out[2];
                    goal_row = deferred_out[2];
                    rec.start_x = start_col;
                    rec.tile_base = (s32)tile;
                    rec.start_z = start_row;
                    mid_x = (via_x + goal_col) / 2;
                    rec.end_x = mid_x;
                    mid_z = (count + goal_row) / 2;
                    rec.end_z = mid_z;
                    if (func_80062820(&rec) != 0)
                    {
                        rec.tile_base = (s32)near_tile;
                        rec.start_x = goal_col;
                        rec.start_z = goal_row;
                        if (func_80062820(&rec) != 0)
                        {
                            ok = 1;
                            via_x = mid_x;
                            count = mid_z;
                            rec.start_x = start_col;
                            try_quarter = 0;
                            rec.tile_base = (s32)tile;
                            rec.start_z = start_row;
                            mid2_x = (via_x + goal_col) / 2;
                            mid2_z = (count + goal_row) / 2;
                            rec.end_x = mid2_x;
                            rec.end_z = mid2_z;
                            if (func_80062820(&rec) != 0)
                            {
                                rec.tile_base = (s32)near_tile;
                                rec.start_x = goal_col;
                                rec.start_z = goal_row;
                                if (func_80062820(&rec) != 0)
                                {
                                    via_x = mid2_x;
                                    count = mid2_z;
                                }
                            }
                        }
                    }
                    if (try_quarter != 0)
                    {
                        deferred_out = (s32*)(goal_col - via_x);
                        quarter_dx = ((s32)deferred_out);
                        rec.tile_base = (s32)tile;
                        rec.start_x = start_col;
                        rec.start_z = start_row;
                        if (((s32)deferred_out) < 0)
                        {
                            quarter_dx = ((s32)deferred_out) + 3;
                        }
                        quarter_x = via_x + (quarter_dx >> 2);
                        rec.end_x = quarter_x;
                        step = goal_row - count;
                        quarter_dz = step;
                        if (step < 0)
                        {
                            quarter_dz = step + 3;
                        }
                        next_out = (s32*)(count + (quarter_dz >> 2));
                        rec.end_z = ((s32)next_out);
                        if (func_80062820(&rec) != 0)
                        {
                            rec.tile_base = (s32)near_tile;
                            rec.start_x = goal_col;
                            rec.start_z = goal_row;
                            if (func_80062820(&rec) != 0)
                            {
                                ok = 1;
                                restored_x_offset = quarter_x - (u32)deferred_out;
                                via_x = (s32)(restored_x_offset + (u32)deferred_out);
                                count = ((s32)next_out);
                            }
                        }
                    }
                    try_quarter = 1;
                    rec.start_x = goal_col;
                    rec.tile_base = (s32)near_tile;
                    rec.start_z = goal_row;
                    back_mid_x = (via_x + start_col) / 2;
                    back_mid_z = (count + start_row) / 2;
                    rec.end_x = back_mid_x;
                    rec.end_z = back_mid_z;
                    if (func_80062820(&rec) != 0)
                    {
                        rec.tile_base = (s32)tile;
                        rec.start_x = start_col;
                        rec.start_z = start_row;
                        if (func_80062820(&rec) != 0)
                        {
                            ok = 1;
                            via_x = back_mid_x;
                            count = back_mid_z;
                            rec.start_x = goal_col;
                            try_quarter = 0;
                            rec.tile_base = (s32)near_tile;
                            rec.start_z = goal_row;
                            back_mid2_x = (via_x + start_col) / 2;
                            back_mid2_z = (count + start_row) / 2;
                            rec.end_x = back_mid2_x;
                            rec.end_z = back_mid2_z;
                            if (func_80062820(&rec) != 0)
                            {
                                rec.tile_base = (s32)tile;
                                rec.start_x = start_col;
                                rec.start_z = start_row;
                                if (func_80062820(&rec) != 0)
                                {
                                    via_x = back_mid2_x;
                                    count = back_mid2_z;
                                }
                            }
                        }
                    }
                    if (try_quarter != 0)
                    {
                        step = start_col - via_x;
                        back_quarter_dx = step;
                        rec.tile_base = (s32)near_tile;
                        rec.start_x = goal_col;
                        rec.start_z = goal_row;
                        if (step < 0)
                        {
                            back_quarter_dx = step + 3;
                        }
                        next_out = (s32*)(via_x + (back_quarter_dx >> 2));
                        rec.end_x = ((s32)next_out);
                        route_value = start_row - count;
                        back_quarter_dz = route_value;
                        if (route_value < 0)
                        {
                            back_quarter_dz = route_value + 3;
                        }
                        deferred_out = (s32*)(count + (back_quarter_dz >> 2));
                        rec.end_z = ((s32)deferred_out);
                        if (func_80062820(&rec) != 0)
                        {
                            rec.tile_base = (s32)tile;
                            rec.start_x = start_col;
                            rec.start_z = start_row;
                            if (func_80062820(&rec) != 0)
                            {
                                ok = 1;
                                via_x = ((s32)next_out);
                                count = ((s32)deferred_out);
                            }
                        }
                    }
                    if (ok != 0)
                    {
                        path[1][1] = via_x;
                        path[2][1] = count;
                    }
                }
                /* Write the points start end first, in world units. */
                count = queue_len;
                if ((u32)count > FIELD_COLLISION_PATH_OUT_MAX)
                {
                    count = FIELD_COLLISION_PATH_OUT_MAX;
                }
                next_out = &path[1][queue_len - 1];
                deferred_out = &path[2][queue_len - 1];
                count = count - 1;
                step = 0;
                if (count != -1)
                {
                    last_queue = -1;
                    out = output_path;
                    do
                    {
                        point_x = *next_out;
                        next_out -= 1;
                        step += 1;
                        count -= 1;
                        out->x = (point_x + ((s16)start_query->width >> 1)) << 8;
                        /* Shift through route_value: a literal << 8 gives 99.76-99.89%. */
                        route_value = 8;
                        raw_depth = start_query->depth;
                        point_z = *deferred_out;
                        deferred_out -= 1;
                        out->z = (point_z + ((s16)raw_depth >> 1)) << route_value;
                        out += 1;
                    } while (count != last_queue);
                }
                return step;
            }
        }
    }
    output_path->x = goal_query->x;
    output_path->z = goal_query->z;
    return 1;
}

/**
 * @brief Walk a rectangular tile footprint along a straight line and stamp it
 *        into the group tile map.
 *
 * Bresenham DDA over the field group tile grid. The footprint uses the request
 * width and depth, starts at (start_x, start_z), and is stepped one tile at a
 * time towards (end_x, end_z). The z axis uses double-height cells (mask
 * tile_size * 2 - 1). edge_state records which edge of the footprint moved on
 * the last step so only the newly covered tiles are re-tested: 1 = a new
 * column, 2 = a new row, 3 = the whole rectangle (the first iteration and both
 * edges crossing at once). A tile that is FIELD_COLLISION_TILE_TOUCHED or
 * FIELD_COLLISION_TILE_DEFERRED and above (deferred, marked, blocked) aborts
 * the walk.
 *
 * With a non-zero request->stamp, every tested tile except the start marker is
 * overwritten with the stamp, and the stamp is bumped after each step unless
 * it has reached FIELD_COLLISION_TILE_GOAL. Only in mode
 * FIELD_COLLISION_TRACE_STOP_AT_GOAL is the stamped tile compared against
 * goal_tile. A zero stamp only tests the line.
 *
 * @param request Walk request. tile_base is the starting tile, goal_tile the
 *                optional goal, tile_size sets the cell masks; col_shift
 *                is not read here.
 * @return 1 when the walk ran to completion or reached goal_tile, 0 when a
 *         blocked tile stopped it.
 */
s32 func_80062820(FieldCollisionTraceRequest* request)
{
    FieldScene* scene;
    u8* tile_base;
    u8* goal_tile;
    u8* tile_ptr;
    u8* scan_ptr;
    s32 footprint_width;
    s32 footprint_depth;
    s32 delta_x;
    s32 delta_z;
    s32 step_x;
    s32 step_z;
    s32 tile_size;
    s32 mask_x;
    s32 mask_z;
    s32 x_cell;
    s32 z_cell;
    s32 x_end;
    s32 z_end;
    s32 width_minus_one;
    s32 depth_minus_one;
    s32 col_hi;
    s32 row_hi;
    s32 x_span;
    s32 row_span;
    s32 stride;
    s32 mode;
    s32 edge_state;
    s32 error_term;
    s32 i;
    s32 n;
    s32 m;
    u32 tile_value;
    u8 stamp;

    tile_base = (u8*)request->tile_base;
    x_cell = request->start_x;
    delta_x = request->end_x - x_cell;
    z_cell = request->start_z;
    delta_z = request->end_z - z_cell;
    tile_size = request->tile_size;
    mask_x = tile_size - 1;
    x_cell &= mask_x;
    footprint_width = request->footprint_width;
    goal_tile = (u8*)request->goal_tile;
    width_minus_one = footprint_width - 1;
    x_end = x_cell + width_minus_one;
    col_hi = x_end & mask_x;
    x_span = x_end >= ((footprint_width + mask_x) & ~mask_x);
    footprint_depth = request->footprint_depth;
    depth_minus_one = footprint_depth - 1;
    mask_z = (tile_size * 2) - 1;
    z_cell &= mask_z;
    z_end = z_cell + depth_minus_one;
    row_hi = z_end & mask_z;
    scene = g_field_scene.scene;
    stride = (u16)scene->unk46;
    row_span = 0;
    if (z_end >= ((footprint_depth + mask_z) & ~mask_z))
    {
        row_span = stride;
    }
    stamp = request->stamp;
    mode = request->mode;

    if (delta_x >= 0)
    {
        step_x = 1;
    }
    else
    {
        delta_x = -delta_x;
        step_x = -1;
    }
    if (delta_z >= 0)
    {
        step_z = stride;
    }
    else
    {
        delta_z = -delta_z;
        step_z = -stride;
    }

    edge_state = 3;
    if (delta_x >= delta_z)
    {
        error_term = -delta_x;
        for (i = delta_x; i != -1; i--)
        {
            FIELD_COLLISION_TRACE_EDGES();
            if (i == 0)
            {
                return 1;
            }
            edge_state = 0;
            if (step_x > 0)
            {
                if (x_cell == mask_x)
                {
                    x_cell = 0;
                    x_span--;
                    tile_base++;
                }
                else
                {
                    x_cell++;
                }
                if (col_hi == mask_x)
                {
                    col_hi = 0;
                    x_span++;
                    edge_state = 1;
                }
                else
                {
                    col_hi++;
                }
            }
            else
            {
                if (x_cell == 0)
                {
                    x_cell = mask_x;
                    x_span++;
                    tile_base--;
                    edge_state = 1;
                }
                else
                {
                    x_cell--;
                }
                if (col_hi == 0)
                {
                    col_hi = mask_x;
                    x_span--;
                }
                else
                {
                    col_hi--;
                }
            }
            error_term += delta_z * 2;
            if (error_term >= 0)
            {
                if (step_z > 0)
                {
                    if (z_cell == mask_z)
                    {
                        z_cell = 0;
                        row_span -= stride;
                        tile_base += stride;
                    }
                    else
                    {
                        z_cell++;
                    }
                    if (row_hi == mask_z)
                    {
                        row_hi = 0;
                        row_span += stride;
                        edge_state |= 2;
                    }
                    else
                    {
                        row_hi++;
                    }
                }
                else
                {
                    if (z_cell == 0)
                    {
                        z_cell = mask_z;
                        row_span += stride;
                        tile_base -= stride;
                        edge_state |= 2;
                    }
                    else
                    {
                        z_cell--;
                    }
                    if (row_hi == 0)
                    {
                        row_hi = mask_z;
                        row_span -= stride;
                    }
                    else
                    {
                        row_hi--;
                    }
                }
                error_term -= delta_x * 2;
            }
        }
    }
    else
    {
        error_term = -delta_z;
        for (i = delta_z; i != -1; i--)
        {
            FIELD_COLLISION_TRACE_EDGES();
            if (i == 0)
            {
                return 1;
            }
            edge_state = 0;
            if (step_z > 0)
            {
                if (z_cell == mask_z)
                {
                    z_cell = 0;
                    row_span -= stride;
                    tile_base += stride;
                }
                else
                {
                    z_cell++;
                }
                if (row_hi == mask_z)
                {
                    row_hi = 0;
                    row_span += stride;
                    edge_state = 2;
                }
                else
                {
                    row_hi++;
                }
            }
            else
            {
                if (z_cell == 0)
                {
                    z_cell = mask_z;
                    row_span += stride;
                    tile_base -= stride;
                    edge_state = 2;
                }
                else
                {
                    z_cell--;
                }
                if (row_hi == 0)
                {
                    row_hi = mask_z;
                    row_span -= stride;
                }
                else
                {
                    row_hi--;
                }
            }
            error_term += delta_x * 2;
            if (error_term >= 0)
            {
                if (step_x > 0)
                {
                    if (x_cell == mask_x)
                    {
                        x_cell = 0;
                        x_span--;
                        tile_base++;
                    }
                    else
                    {
                        x_cell++;
                    }
                    if (col_hi == mask_x)
                    {
                        col_hi = 0;
                        x_span++;
                        edge_state |= 1;
                    }
                    else
                    {
                        col_hi++;
                    }
                }
                else
                {
                    if (x_cell == 0)
                    {
                        x_cell = mask_x;
                        x_span++;
                        tile_base--;
                        edge_state |= 1;
                    }
                    else
                    {
                        x_cell--;
                    }
                    if (col_hi == 0)
                    {
                        col_hi = mask_x;
                        x_span--;
                    }
                    else
                    {
                        col_hi--;
                    }
                }
                error_term -= delta_z * 2;
            }
        }
    }
    return 1;
}

/**
 * @brief Scale a movement vector down to account for a node edge's slope.
 *
 * The node's C->B edge runs between the boundary points vertex_c and vertex_b
 * in g_field_node_angle_table and rises by height1 - height0 over that span.
 * Moving along the edge covers its full 3D length while only advancing by the
 * horizontal run, so @p movement is multiplied by run / slope to hold the ground
 * speed constant. The run is first shortened by a further 1/16, a flat penalty
 * for travelling on a slope at all.
 *
 * @param surface Collision surface definition supplying the edge and its heights.
 * @param movement Movement vector rescaled in place; movement[0] is x and movement[1] is z.
 *
 * @note dx and dz are reused for their squares (separate locals 96.66%).
 */
void func_80062F48(FieldCollisionSurfaceDef* surface, s32* movement)
{
    s16* vertices;
    s16* vertex_c;
    s16* vertex_b;
    s32 dx;
    s32 dy;
    s32 dz;
    s32 horizontal_length;
    s32 surface_length;

    vertices = g_field_node_angle_table;
    vertex_c = &vertices[surface->vertex_c * 2];
    vertex_b = &vertices[surface->vertex_b * 2];
    dx = vertex_b[0] - vertex_c[0];
    dy = vertex_b[1] - vertex_c[1];
    dz = surface->height1 - surface->height0;
    dx = dx * dx + dy * dy;
    dz = dz * dz;
    horizontal_length = SquareRoot0(dx);
    surface_length = SquareRoot0(dx + dz);
    horizontal_length -= horizontal_length >> 4;
    movement[0] = movement[0] * horizontal_length / surface_length;
    movement[1] = movement[1] * horizontal_length / surface_length;
}

/**
 * @brief Convert a probe's footprint to whole tiles and stencil it.
 *
 * scene->unk40 is the tile edge in pixels, either 4 or 8, and tile_shift is its
 * base-2 log. The footprint width is rounded up to a whole number of
 * tiles and the depth to a whole number of double-height tiles, since
 * the depth axis is stored at half the horizontal resolution. The rounding is
 * the usual `(v + n - 1) >> log2(n)` ceiling divide.
 *
 * Does nothing when no per-group work area is allocated.
 *
 * @param query Probe query supplying the footprint extents.
 *
 * @note The extents are read as signed even though FieldCollisionQuery declares them u16.
 */
void func_8006304C(FieldCollisionQuery* query)
{
    FieldScene* scene;
    s32 tile_shift;
    s32 tile_size;

    scene = g_field_scene.scene;
    if (scene->unk28 != 0)
    {
        tile_size = scene->unk40;
        tile_shift = 3;
        if (tile_size == 4)
        {
            tile_shift = 2;
        }
        func_80060364(((s16)query->width + tile_size - 1) >> tile_shift, ((s16)query->depth + tile_size * 2 - 1) >> (tile_shift + 1));
    }
}

/**
 * @brief Fetch a record body from the scene header's linked list.
 *
 * Walks @p index links from the head, but stops advancing once it reaches the
 * tail, so an index past the end clamps to the last record rather than running
 * off the list. The counter is a u16, so an index of zero walks nothing and
 * returns the head's body.
 *
 * @param index Number of links to walk from the head of the list.
 * @return Pointer to the selected record's body, or NULL if the scene carries
 *         no records at all.
 */
void* func_800630BC(s32 index)
{
    FieldHeaderRec* record;
    u16 remaining;

    record = g_field_scene.scene->header->records;
    if (record != NULL)
    {
        remaining = index;
        while (remaining-- != 0)
        {
            if (record->next != NULL)
            {
                record = record->next;
            }
        }
        return &record->body;
    }
    return NULL;
}

/**
 * @brief Rebuild every attached node's span table, then regroup the scene.
 *
 * The field allocator cursor is pulled into a local, handed to each node's
 * span builder in turn so the successive tables pack contiguously, and finally
 * to the group scan. Whatever the two callees leave in the local is written
 * back to the global cursor, so the scratch they allocated stays reserved.
 */
void func_8006312C(void)
{
    FieldNode* node;
    s32 allocator_cursor;

    node = g_field_scene.scene->nodes;
    allocator_cursor = D_801ED000;
    while (node != NULL)
    {
        func_8005E3B0((FieldCollisionNode*)node, &allocator_cursor);
        node = node->next;
    }
    func_8005F158(&allocator_cursor);
    D_801ED000 = allocator_cursor;
}
