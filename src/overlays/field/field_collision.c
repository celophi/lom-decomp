#include "field_scene_internal.h"

/**
 * @brief Probe position and footprint passed to func_8005B368.
 *
 * Carries the world-space probe position (x, y, z) plus the footprint
 * extents (width along x and depth along z) plus a vertical height
 * tolerance.
 */
typedef struct
{
    s32 x;
    s32 y;
    s32 z;
    u16 width;
    s16 height_tolerance;
    u16 depth;
} FieldCollisionQuery;

/**
 * @brief Collision object referenced by a FieldCollisionBoundsNode.
 * @note unk10/unk12 are the vertical band the object occupies; unk14 is the
 *       value returned to the caller on a successful hit.
 */
typedef struct
{
    u8 pad[0x10];
    s16 unk10;
    s16 unk12;
    s16 unk14;
} FieldCollisionObject;

/**
 * @brief One entry in the field scene's collision-node list (scene+0x10).
 *
 * @note right/left/bottom/top form the node's axis-aligned bounding box.
 *       Each of the two (denom, mult, max, min) groups describes a sloped
 *       edge: when denom != 0 the edge is diagonal and the probe is tested
 *       against mult*coord/denom; when denom == 0 the edge is axis-aligned
 *       and only max/min are used.
 */
typedef struct FieldCollisionBoundsNode
{
    struct FieldCollisionBoundsNode* next;
    FieldCollisionObject* obj;
    u8 pad[8];
    s16 right;
    s16 left;
    s16 bottom;
    s16 top;
    s32 denom1;
    s32 mult1;
    s32 denom2;
    s32 mult2;
    s32 max1;
    s32 min1;
    s32 max2;
    s32 min2;
} FieldCollisionBoundsNode;

/**
 * @brief Hit-test a probe against the field scene's collision-node list.
 * @param query Probe position, footprint dimensions, and vertical tolerance.
 * @return Collision object's value on the first hit, or -1 when no node intersects the probe.
 */
s16 func_8005B368(FieldCollisionQuery* query)
{
    s32 sx;
    s32 ex;
    s32 work0;
    s32 work1;
    s32 ez;
    s32 sy;
    s32 start_z;
    s32 half_z;
    FieldCollisionBoundsNode* node;
    s32 hit;
    s32 raw_y;
    s32 tx;
    s32 ty;

    work0 = query->width;
    node = (FieldCollisionBoundsNode*)g_field_scene.scene;
    work0 <<= 16;
    work1 = work0 >> 16;
    work0 = (u32)work0 >> 31;
    work1 += work0;
    work0 = query->x;
    work1 >>= 1;
    if (work0 < 0)
    {
        work0 += 0xFF;
    }
    work0 >>= 8;
    sx = work0 - work1;

    work0 = (s16)query->width;
    work1 = query->depth;
    do
    {
        ex = sx + work0;
    } while (0);
    work1 <<= 16;
    work0 = work1 >> 16;
    work1 = (u32)work1 >> 31;
    work0 += work1;
    work1 = query->z;
    half_z = work0 >> 1;
    if (work1 >= 0)
    {
        work0 = work1 >> 8;
    }
    else
    {
        work0 = (work1 + 0xFF) >> 8;
    }
    start_z = work0 - half_z;
    work0 = (s16)query->depth;
    raw_y = query->y;
    ez = start_z + work0;
    if (raw_y >= 0)
    {
        sy = raw_y >> 8;
    }
    else
    {
        sy = (raw_y + 0xFF) >> 8;
    }

    for (node = *(FieldCollisionBoundsNode**)((u8*)node + 0x10); node != 0; node = node->next)
    {
        FieldCollisionObject* obj;
        s16 val;

        do
        {
            obj = node->obj;
        } while (0);
        val = obj->unk10;

        if ((sy - query->height_tolerance) >= val)
        {
            continue;
        }
        if ((obj->unk12 != 1) && ((val + obj->unk12) >= sy))
        {
            continue;
        }
        if (node->top >= ez)
        {
            continue;
        }
        if (start_z >= node->bottom)
        {
            continue;
        }
        if (node->left >= ex)
        {
            continue;
        }
        if (sx >= node->right)
        {
            continue;
        }

        hit = 0;
        if (node->denom1 != 0)
        {
            tx = (node->mult1 * sx) / node->denom1;
            ty = (node->mult1 * ex) / node->denom1;
            if (((((start_z - tx) >= node->max1) || ((ez - tx) >= node->max1)) || ((start_z - ty) >= node->max1)) || ((ez - ty) >= node->max1))
            {
                if ((((node->min1 >= (start_z - tx)) || (node->min1 >= (ez - tx))) || (node->min1 >= (start_z - ty))) || (node->min1 >= (ez - ty)))
                {
                    hit = 1;
                }
            }
        }
        else if (ex >= node->max1)
        {
            if (node->min1 >= sx)
            {
                hit = 1;
            }
        }

        if (hit)
        {
            if (node->denom2 != 0)
            {
                tx = (node->mult2 * sx) / node->denom2;
                ty = (node->mult2 * ex) / node->denom2;
                if (((((start_z - tx) >= node->max2) || ((ez - tx) >= node->max2)) || ((start_z - ty) >= node->max2)) || ((ez - ty) >= node->max2))
                {
                    if (!((((node->min2 < (start_z - tx)) && (node->min2 < (ez - tx))) && (node->min2 < (start_z - ty))) && (node->min2 < (ez - ty))))
                    {
                        return obj->unk14;
                    }
                }
            }
            else if (ex >= node->max2)
            {
                if (node->min2 >= sx)
                {
                    return obj->unk14;
                }
            }
        }
    }

    return -1;
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

extern long ratan2(long y, long x);
extern int rcos(int a);
extern int rsin(int a);

extern long SquareRoot0(long a);

/** Field allocator cursor at 0x801ED000, i.e. FieldMemState::top. */
extern s32 D_801ED000;

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

s32 func_8005E1A8(FieldCollisionSurfaceDef* surface, s32 edge_index, s32 move_angle, s32 best_angle);

typedef struct FieldCollisionNode
{
    struct FieldCollisionNode* next;
    FieldCollisionSurfaceDef* surface;
    u8 pad8[8];
    /** Per-row span table: (min_x, max_x) pairs, FIELD_COLLISION_ROW_SPANS per row. */
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

s16 func_8005DFAC();

/**
 * @brief Actor/mover state resolved by func_8005B6AC.
 * @note Distinct from the smaller `FieldCollisionQuery` probe struct above, which stores its
 *       footprint dimensions as u16 values. Prologue asm proves word loads at 0x4/0x10 and an s16 load
 *       at 0x26, so every field here is its asm-confirmed width.
 */
typedef struct FieldCollisionMover
{
    s32 x;                /* world X (fixed-point) */
    s32 height;           /* vertical position/height accumulator */
    s32 z;                /* world Z (fixed-point) */
    s32 move_x;           /* horizontal movement this step */
    s32 move_height;      /* vertical movement this step */
    s32 move_z;           /* depth movement this step */
    s32 resolved_height;  /* resolved surface-height delta */
    void* collision_node; /* selected collision node (-1 = needs search) */
    s32 flags;            /* state flags (bit 0 tracks sticky contact) */
    u16 footprint_width;  /* collision footprint width */
    s16 height_bias;      /* vertical collision bias */
    s32 mode_flags;       /* mode flags (0x30000 gate, low s16 = step size) */
} FieldCollisionMover;

void func_80062F48(FieldCollisionSurfaceDef* surface, s32* movement);

typedef struct FieldCollisionMoveProbe
{
    FieldCollisionMover* mover;
    s32 x;
    s32 z;
    s16 w;
    s16 h;
} FieldCollisionMoveProbe;

void func_8005DA7C(FieldCollisionMoveProbe* probe, FieldCollisionNode* node, s32* out_hit, s32* out_touch);

/**
 * @brief Resolve a mover against the field collision surfaces.
 * @param mover Mover state updated with resolved position, height, contact node, and flags.
 * @return Collision-resolution status bitmask.
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
    s32 step_height_raw;
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
    s32 half_move;
    s32 next_step;
    s32 prev_step;
    s16 node_height16;
    s32 touch_cell_x;
    s32 pass_left;
    s32 touch_left;
    s32 floor_fixed_candidate;
    s32 secondary_rise;
    s32 cells_x;
    s32 cells_z;
    s32 steps_z;
    s32 move_x_copy;
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
    s32 first_kind;
    s32 standing_value;
    s32 span_push_x;
    s32 span_hit;
    s32 on_slope;
    s32 cell_z;
    s32 scratch;
    s32 floor_fixed;
    s32 slide_angle;
    s32 push_x;
    s32 cell_x;
    s32 new_result;
    s32 remaining;
    s32 hit;
    s32 step_count;
    s32 move_x_abs;
    s32 secondary_cell_x;
    s32 standing_fixed;
    s32 standing_cell_z;
    s32 move_z_abs;
    s32 cell;
    s32 push_z_abs;
    s32 ground_x;
    s32 higher;
    s32 coord;
    s32 first_cell_x;
    s32 counter;
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
    carry_x = 0;
    carry_z = 0;
    result = 0;
    scene = g_field_scene.scene;
    mover->resolved_height = 0;
    {
        s32 x = -mover->height - mover->move_height;
        next_height = (u16)((u32)(x + ((x < 0) ? 0xFF : 0)) >> 8);
    }
    do
    {
        s32 x = -mover->height;
        step_height_raw = ((x + ((x < 0) ? 0xFF : 0)) >> 8) + mover->height_bias;
        step_height = (u16)step_height_raw;
    } while (0);
    head = (FieldCollisionNode*)scene->nodes;
    nodes = head;
    if (head != NULL)
    {
        if (mover->collision_node == (void*)-1)
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
                        first_spans = ((u8*)surface)[6];
                        span = (FieldCollisionSpan*)node->spans + (((s16)cell_z - first_row_start) * first_spans);
                        if ((s16)node_offset_x != 0)
                        {
                            scratch = first_spans - 1;
                            if (first_spans != 0)
                            {
                                do
                                {
                                    if (((s16)first_cell_x < (span->min_x + (s16)node_offset_x)) ||
                                        ((span->max_x + (s16)node_offset_x) < (s16)first_cell_x))
                                    {
                                        span++;
                                    }
                                    else
                                    {
                                        goto a_hit;
                                    }
                                } while (--scratch != -1);
                            }
                            goto a_done;
                        a_hit:
                            hit = 1;
                            goto a_done;
                        }
                        else
                        {
                            scratch = first_spans - 1;
                            if (first_spans != 0)
                            {
                                do
                                {
                                    if (((s16)first_cell_x < span->min_x) || (span->max_x < (s16)first_cell_x))
                                    {
                                        span++;
                                    }
                                    else
                                    {
                                        goto a_hit;
                                    }
                                } while (--scratch != -1);
                            }
                        }
                    a_done:
                        if (hit != 0)
                        {
                            first_kind = ((u8*)surface)[4] & 3;
                            switch (first_kind)
                            {       /* switch 1; irregular */
                            case 0: /* switch 1 */
                                if ((surface->top + (s16)node_height16) < (s16)step_height)
                                {
                                    if (on_slope != 0)
                                    {
                                        s32 first_threshold = floor_height + 0x14;
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
                            case 1: /* switch 1 */
                                if ((surface->top + (s16)node_height16) < (s16)step_height)
                                {
                                    scratch = func_8005DFAC(node, &probe.x);
                                    if (on_slope != 0)
                                    {
                                        on_slope = 1;
                                        if (floor_height < scratch)
                                        {
                                            floor_height = scratch;
                                            mover->collision_node = node;
                                        }
                                    }
                                    else
                                    {
                                        if (floor_height < (scratch + 0x14))
                                        {
                                            floor_height = scratch;
                                            mover->collision_node = node;
                                        }
                                        on_slope = 1;
                                    }
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
        if ((standing != NULL) && (standing != (FieldCollisionNode*)-2))
        {
            node = standing;
            surface = node->surface;
            if (node->active != 0)
            {
                if (((surface->flags & 3) == 1) && (surface->height0 != surface->height1))
                {
                    probe.x = mover->move_x;
                    probe.z = mover->move_z;
                    func_80062F48(surface, &probe.x);
                    mover->move_x = probe.x;
                    result |= 4;
                    mover->move_z = probe.z;
                }
                if (surface->flags & 0x10)
                {
                    half_move = mover->move_x;
                    delta_x = half_move;
                    mover->move_x = (half_move >= 0) ? (half_move >> 1) : ((s32)(delta_x + 1) >> 1);
                    half_move = mover->move_z;
                    delta_z = half_move;
                    mover->move_z = (half_move >= 0) ? (half_move >> 1) : ((s32)(delta_z + 1) >> 1);
                    result |= 4;
                }
                if (surface->flags & 0x40)
                {
                    result |= 0x10;
                }
                if (!(mover->mode_flags & 0x30000) &&
                    ((node->motion_x != 0) || (node->motion_height != 0) || (node->unk2C != 0) || (node->motion_z != 0)))
                {
                    mover->move_x = mover->move_x + node->motion_x;
                    old_move_height = mover->move_height;
                    mover->move_z = mover->move_z + node->motion_z;
                    if ((surface->flags & 3) == 1)
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
                        result |= 0x30;
                    }
                    else if ((node->motion_x != 0) || (node->motion_z != 0))
                    {
                        result |= 0x10;
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
                        if ((surface->flags & 3) == 1)
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
                            result |= 0x30;
                        }
                        else if ((node->motion_x != 0) || (node->motion_z != 0))
                        {
                            result |= 0x10;
                        }
                    }
                }
            }
        }
    }
    if ((mover->move_x == 0) && (mover->move_height == 0) && (mover->move_z == 0))
    {
        if (nodes == NULL)
        {
            goto return_zero;
        }
        if (mover->height == 0)
        {
            goto return_zero;
        }
        if (mover->flags & 1)
        {
            standing_node = mover->collision_node;
            if (standing_node != (FieldCollisionNode*)-2)
            {
                node = standing_node;
                if (node == NULL)
                {
                    goto return_zero;
                }
                surface = node->surface;
                standing_value = ((u8*)surface)[4] & 3;
                switch (standing_value)
                {       /* switch 2; irregular */
                case 0: /* switch 2 */
                    standing_value = surface->height0;
                    standing_fixed = node->height_offset;
                    standing_value <<= 8;
                    standing_fixed += standing_value;
                    mover->resolved_height = -standing_fixed;
                    goto return_zero;
                case 1: /* switch 2 */
                    probe.x = FIELD_COLLISION_CELL(mover->x);
                    standing_cell_z = mover->z;
                    if (standing_cell_z < 0)
                    {
                        standing_cell_z = (standing_cell_z + 0xFF) >> 8;
                    }
                    else
                    {
                        standing_cell_z >>= 8;
                    }
                    cell_x = (probe.z = standing_cell_z);
                    slope_height = func_8005DFAC(node, &probe.x);
                    secondary = (FieldCollisionNode*)scene->secondary_nodes;
                    if ((secondary != NULL) && (secondary != node))
                    {
                        slope_height += func_8005DFAC(secondary, &probe.x) - secondary->surface->height0;
                    }
                    standing_fixed = slope_height << 8;
                    mover->resolved_height = -standing_fixed;
                    goto return_zero;
                default: /* switch 2 */
                    goto return_zero;
                }
            }
        }
    }
    if (0)
    {
    return_zero:
        return 0;
    }
    {
        probe.mover = mover;
        probe.w = next_height;
        probe.h = step_height;
        move_x_abs = mover->move_x;
        footprint_w = (s16)mover->footprint_width;
        if (move_x_abs < 0)
        {
            move_x_abs = -move_x_abs;
        }
        cells_x = move_x_abs >> 8;
        step = cells_x;
        if (cells_x >= footprint_w)
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
        cells_z = move_z_abs >> 8;
        step = cells_z;
        if (cells_z >= footprint_d)
        {
            steps_z = (step / footprint_d) + 1;
            step = steps_z;
            if (step_count < steps_z)
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
                next_step = step + 1;
                step = next_step;
                cell = mover->x + (mover->move_x * next_step / step_count);
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
            work = mover->move_x;
            if (work == 0)
            {
                move_angle = 0xC00;
                if (mover->move_z >= 0)
                {
                    move_angle = 0x400;
                }
            }
            else
            {
                move_angle = ratan2(mover->move_z, work) & 0xFFF;
            }
            move_x_copy = mover->move_x;
            major = mover->move_z;
            step_count = abs(move_x_copy);
            major = abs(major);
            step = major;
            if (step_count >= major)
            {
                step_count = step_count >> 8;
            }
            else
            {
                step_count = step >> 8;
            }
            slide_angle = -2;
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
                        FieldCollisionMover* qt6;
                        s32 qt5;
                        s32 qv0;
                        s32 qv1;
                        s32 qa0;
                        qt6 = mover;
                        qt5 = step;
                        qv1 = qt6->move_x;
                        qv0 = step_count - qt5;
                        qv1 *= qv0;
                        qa0 = qv1 / step_count;
                        qv0 = (u16)qt6->footprint_width;
                        qv0 <<= 16;
                        qv1 = qv0 >> 16;
                        qv0 = (s32)((u32)qv0 >> 31);
                        qv1 += qv0;
                        qv0 = qt6->x;
                        qv0 += qa0;
                        qv1 >>= 1;
                        if (qv0 < 0)
                        {
                            qv0 += 0xFF;
                        }
                        qv0 >>= 8;
                        x_start = qv0 - qv1;
                        qt6 = mover;
                        qt5 = step;
                        qv1 = qt6->move_z;
                        qv0 = step_count - qt5;
                        qv1 *= qv0;
                        qa0 = qv1 / step_count;
                        do
                        {
                            qv1 = (u16)qt6->footprint_width;
                            qv0 = (u16)qt6->mode_flags;
                        } while (0);
                        x_end = x_start + qv1;
                        qv0 <<= 16;
                        qv1 = qv0 >> 16;
                        qv0 = (s32)((u32)qv0 >> 31);
                        qv1 += qv0;
                        qv0 = qt6->z;
                        qv0 += qa0;
                        qv1 >>= 1;
                        if (qv0 < 0)
                        {
                            qv0 += 0xFF;
                        }
                        qv0 >>= 8;
                        scratch = qv1;
                        scratch = qv0 - scratch;
                    }
                    header = scene->header;
                    z_end_raw = scratch + (u16)mover->mode_flags;
                    z_end = z_end_raw;
                    if (header->flags & FIELD_SCENE_HEADER_BOUNDED)
                    {
                        if (((s16)scratch < 0) || (z_end >= header->unk32))
                        {
                            slide_angle = func_8005E1A8(NULL, 0x7F, move_angle, slide_angle);
                        }
                        if (((s16)x_start < 0) || (x_end >= scene->header->unk30))
                        {
                            slide_angle = func_8005E1A8(NULL, 0x7E, move_angle, slide_angle);
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
                                        hit_spans = ((u8*)surface)[6];
                                        row_skip = (row - work) * hit_spans;
                                        scratch = ((scratch - row) + 1) * hit_spans;
                                        hit = 0;
                                        span = (FieldCollisionSpan*)node->spans + (row_skip);
                                        left_flags = (u8*)node->span_flags + (row_skip * 2);
                                        if (surface->flags & 8)
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
                                                            slide_angle = func_8005E1A8(surface, (u8)*left_flags & 0x7F, move_angle, slide_angle);
                                                            hit = 2;
                                                        }
                                                        if ((span->max_x < box_x_last) && (*right_flags >= 0))
                                                        {
                                                            slide_angle = func_8005E1A8(surface, (u8)*right_flags & 0x7F, move_angle, slide_angle);
                                                            hit = 2;
                                                        }
                                                        {
                                                            u8 left_flag = *left_flags;
                                                            if ((s8)left_flag >= 0)
                                                            {
                                                                u8 right_flag = *right_flags;
                                                                if ((s8)right_flag >= 0)
                                                                {
                                                                    span_flag = 0x7F;
                                                                    if (((left_flag == span_flag) || (right_flag == span_flag)) && (span->min_x < box_x_start) &&
                                                                        (span->max_x >= box_x_end))
                                                                    {
                                                                        slide_angle = func_8005E1A8(surface, 0x7F, move_angle, slide_angle);
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
                                                            slide_angle = func_8005E1A8(surface, left_flags[0] & 0x7F, move_angle, slide_angle);
                                                            hit = 1;
                                                        }
                                                        if ((span->max_x + scan_bias) < box_x_last)
                                                        {
                                                            slide_angle = func_8005E1A8(surface, left_flags[1] & 0x7F, move_angle, slide_angle);
                                                            hit = 1;
                                                        }
                                                        if (((span->min_x + scan_bias) < box_x_start) && ((span->max_x + scan_bias) >= box_x_end))
                                                        {
                                                            slide_angle = func_8005E1A8(surface, 0x7F, move_angle, slide_angle);
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
                                                            slide_angle = func_8005E1A8(surface, left_flags[0] & 0x7F, move_angle, slide_angle);
                                                            hit = 1;
                                                        }
                                                        if (span->max_x < box_x_last)
                                                        {
                                                            slide_angle = func_8005E1A8(surface, left_flags[1] & 0x7F, move_angle, slide_angle);
                                                            hit = 1;
                                                        }
                                                        if ((span->min_x < box_x_start) && (span->max_x >= box_x_end))
                                                        {
                                                            slide_angle = func_8005E1A8(surface, 0x7F, move_angle, slide_angle);
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
                            do
                            {
                                remaining -= 1;
                            } while (0);
                        } while (remaining != -1);
                    }
                    if (slide_angle != -2)
                    {
                        break;
                    }
                    prev_step = step - 1;
                    step = prev_step;
                } while (prev_step != -1);
            }
            value = mover->move_x;
            {
                s32 remaining_steps = step_count - 1;
                steps_taken = remaining_steps - step;
            }
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
                        FieldCollisionMover* qt5;
                        s32 qt6;
                        s32 qv0;
                        s32 qv1;
                        qt5 = mover;
                        qt6 = delta_x;
                        qv0 = qt5->x;
                        qv0 += qt6;
                        if (qv0 >= 0)
                        {
                            qv0 >>= 8;
                            span = (FieldCollisionSpan*)(qv0 - half_width);
                        }
                        else
                        {
                            qv0 += 0xFF;
                            qv0 >>= 8;
                            span = (FieldCollisionSpan*)(qv0 - half_width);
                        }
                        qt5 = mover;
                        qt6 = delta_z;
                        qv1 = (u16)qt5->footprint_width;
                        qv0 = (u16)qt5->mode_flags;
                        x_end = (s16)(s32)span + qv1;
                        qv0 <<= 16;
                        qv1 = qv0 >> 16;
                        qv0 = (s32)((u32)qv0 >> 31);
                        qv1 += qv0;
                        qv0 = qt5->z;
                        qv0 += qt6;
                        qv1 >>= 1;
                        if (qv0 < 0)
                        {
                            qv0 += 0xFF;
                        }
                        qv0 >>= 8;
                        scratch = qv0 - qv1;
                    }
                    touch_iter = FIELD_COLLISION_TOUCH_LIST;
                    hit = 0;
                    push_x = 0;
                    step_count = touch_count;
                    depth = (u16)mover->mode_flags;
                    push_z = 0;
                    blocked_marker = 0x8000;
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
                        if ((s16)(s32)span < 0)
                        {
                            push_x = -(s16)(s32)span;
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
                        push_x_start = (s16)(s32)span;
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
                                        push_row_skip = (row - row_start) * ((u8*)surface)[6];
                                        scratch = scratch - row;
                                        span = (FieldCollisionSpan*)node->spans + (push_row_skip);
                                        left_flags = (u8*)node->span_flags + (push_row_skip * 2);
                                        if (scratch != -1)
                                        {
                                            limit = push_x_end - 1;
                                            do
                                            {
                                                do
                                                {
                                                    remaining = ((u8*)surface)[6];
                                                    remaining -= 1;
                                                    if (remaining == -1)
                                                    {
                                                        goto f_next_row;
                                                    }
                                                } while (0);
                                                {
                                                    span_offset_x = (s16)nodes_offset_x;
                                                    slide_angle = row + 1;
                                                    node = (FieldCollisionNode*)(slide_angle - push_z_end);
                                                    do
                                                    {
                                                        push_min_x = span->min_x;
                                                        if (push_min_x < push_x_end)
                                                        {
                                                            span_max_x = span->max_x;
                                                            if (span_max_x >= push_x_start)
                                                            {
                                                                span_hit = 0;
                                                                if (surface->flags & 8)
                                                                {
                                                                    span_push_x = 0;
                                                                    if ((push_x_start < push_min_x) && !(*left_flags & 0x80))
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
                                                                        if ((push_max_x < limit) && !(left_flags[1] & 0x80))
                                                                        {
                                                                            if (span->min_x < push_x_start)
                                                                            {
                                                                                span_push_x = (push_max_x - push_x_start) + 1;
                                                                            }
                                                                            span_hit = 1;
                                                                        }
                                                                    }
                                                                    if (!(*left_flags & 0x80) && !(left_flags[1] & 0x80))
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
                                                                    if ((push_z_end - slide_angle) >= row_from_start)
                                                                    {
                                                                        span_push_z = row_from_start + 1;
                                                                    }
                                                                    else
                                                                    {
                                                                        span_push_z = (s32)node - 1;
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
                                                                                    push_x = 0x8000;
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
                                                                                push_x = 0x8000;
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
                                                                                push_z = 0x8000;
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
                                            f_next_row:
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
                    pass_left = hit_count - 1;
                    hit_count = pass_left;
                } while (pass_left != -1);
                result |= 2;
            }
            else
            {
                result |= 1;
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
                    result |= 3;
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
                result |= 3;
            }
        }
        else
        {
            mover->x = mover->x + mover->move_x;
            mover->z = mover->z + mover->move_z;
        }
        limit = 0xFFFFFF;
        floor_height = 0;
        do
        {
            floor_fixed = 0;
            on_slope = 0;
            ground_x = mover->x;
            node = NULL;
            mover->collision_node = NULL;
        } while (0);
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
        span = (FieldCollisionSpan*)(u32)(u16)probe.x;
        cell_x = (s32)span;
        scratch = (u16)probe.z;
        ground_z = scratch;
        touch_iter = FIELD_COLLISION_TOUCH_LIST;
        secondary = (FieldCollisionNode*)scene->secondary_nodes;
        touch_left = *(s32*)&touch_count - 1;
        touch_count = touch_left;
        if (touch_left != -1)
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
                do
                {
                    if ((ground_cell_z >= touch_row_start) && (((touch_node->max_z + touch_offset_z) >= ground_cell_z)))
                    {
                        touch_spans = ((u8*)surface)[6];
                        span = (FieldCollisionSpan*)touch_node->spans + ((ground_cell_z - touch_row_start) * touch_spans);
                        if ((s16)touch_offset_x != 0)
                        {
                            scratch = touch_spans - 1;
                            if (touch_spans != 0)
                            {
                                touch_spans = -1;
                                do
                                {
                                    if ((touch_cell_x < (span->min_x + (s16)touch_offset_x)) ||
                                        ((span->max_x + (s16)touch_offset_x) < (s16)cell_x))
                                    {
                                        span++;
                                    }
                                    else
                                    {
                                        goto g_hit;
                                    }
                                } while (--scratch != touch_spans);
                            }
                            goto g_done;
                        g_hit:
                            hit = 1;
                            goto g_done;
                        }
                        else
                        {
                            scratch = touch_spans - 1;
                            if (touch_spans != 0)
                            {
                                counter = -1;
                                do
                                {
                                    if (((s16)cell_x < span->min_x) || (span->max_x < (s16)cell_x))
                                    {
                                        span++;
                                    }
                                    else
                                    {
                                        goto g_hit;
                                    }
                                } while (--scratch != counter);
                            }
                        }
                    }
                } while (0);
            g_done:;
                touch_height_raw = nodes->height_offset;
                {
                    s32 node_height = touch_height_raw >> 8;
                    touch_kind = ((u8*)surface)[4] & 3;
                    touch_height = node_height;
                }
                switch (touch_kind)
                {       /* switch 3; irregular */
                case 0: /* switch 3 */
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
                            s32 threshold = floor_height + 0x14;
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
                case 1: /* switch 3 */
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
                                    do
                                    {
                                        floor_fixed += secondary_rise << 8;
                                    } while (0);
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
                                on_slope = 1;
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
                                on_slope = 1;
                            }
                        }
                        else
                        {
                            higher = floor_height < (scratch + 0x14);
                            if (higher != 0)
                            {
                                floor_height = scratch;
                                floor_fixed = floor_height << 8;
                                if (hit != 0)
                                {
                                    mover->collision_node = nodes;
                                }
                            }
                            on_slope = 1;
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
                }
                counter = touch_count;
                counter -= 1;
                touch_count = counter;
            } while (counter != -1);
        }
        if (mover->mode_flags & 0x30000)
        {
            bias = mover->height_bias;
            if (limit < ((s16)next_height + bias))
            {
                mover->height = -((limit - bias) << 8);
                new_result = result | 0x40;
                result = new_result;
            }
            else if (floor_height >= (s16)next_height)
            {
                mover->height = -floor_fixed;
                new_result = result | 0x80;
                result = new_result;
            }
            else
            {
                mover->height = mover->height + mover->move_height;
            }
        }
        else if (((s16)next_height != floor_height) || (floor_fixed != 0))
        {
            mover->height = -floor_fixed;
            new_result = result | 0x20;
            result = new_result;
        }
        if (node == mover->collision_node)
        {
            mover->flags = mover->flags | 1;
        }
        else
        {
            mover->flags = mover->flags & 0xFFFE;
        }
        move_x_abs = result;
        return move_x_abs;
    }
}

/**
 * @brief Classify collision nodes against a mover probe and collect blocking and touching nodes.
 * @param probe Probe position, vertical extents, and owning mover.
 * @param node Head of the collision node chain.
 * @param out_hit Receives the number of blocking nodes written to the hit list.
 * @param out_touch Receives the number of touching nodes written to the touch list.
 */
void func_8005DA7C(FieldCollisionMoveProbe* probe, FieldCollisionNode* node, s32* out_hit, s32* out_touch)
{
    FieldCollisionNode** hit_list;
    FieldCollisionNode** touch_list;
    FieldCollisionSurfaceDef* obj;
    FieldCollisionMover* m;
    FieldCollisionSpan* pt;
    u8* fl;
    s16 w;
    s16 h;
    s16 y0;
    s16 y1;
    s16 x0;
    s16 x1;
    s32 row_top;
    s32 row_clip;
    s32 row_bot;
    s32 row_lim;
    s32 dx;
    s32 dy;
    s16 dz;
    s32 value;
    s32 off;
    s32 count;
    s32 result;
    s8 over;
    s32 gnd;
    s8 f0;
    s32 y0v;
    u8 stride;
    u16 sy;
    u16 sx;

    *out_hit = 0;
    *out_touch = 0;
    if (node != NULL)
    {
        hit_list = FIELD_COLLISION_HIT_LIST;
        touch_list = FIELD_COLLISION_TOUCH_LIST;
        h = probe->h;
        m = probe->mover;
        w = probe->w;
        sy = m->mode_flags;
        y0 = probe->z - (s16)sy / 2;
        y1 = y0 + sy;
        sx = m->footprint_width;
        x0 = probe->x - (s16)sx / 2;
        x1 = x0 + sx;
        do
        {
            obj = node->surface;
            if (node->active != 0)
            {
                dz = (s32)node->height_offset >> 8;
                if (obj->top + dz == 0 || (value = obj->top + dz) < w + m->height_bias || value < h)
                {
                    dy = (s32)node->offset_z >> 8;
                    dx = (s32)(node->offset_x << 8) >> 16;
                    if (((node->min_x + dx) < x1) && ((node->max_x + dx) >= x0))
                    {
                        row_lim = y1;
                        row_top = node->min_z + (s16)dy;
                        if (row_top < row_lim)
                        {
                            y0v = y0;
                            row_bot = node->max_z + (s16)dy;
                            if (row_bot >= y0v)
                            {
                                count = row_bot;
                                if ((row_lim - 1) < count)
                                {
                                    count = row_lim - 1;
                                }
                                if (row_top < y0v)
                                {
                                    row_clip = y0v;
                                }
                                else
                                {
                                    row_clip = row_top;
                                }
                                stride = ((u8*)obj)[6];
                                off = (row_clip - row_top) * stride;
                                count = ((count - row_clip) + 1) * stride;
                                result = 0;
                                pt = (FieldCollisionSpan*)node->spans + off;
                                if (obj->flags & 8)
                                {
                                    fl = (u8*)node->span_flags + off * 2;
                                    while (--count != -1)
                                    {
                                        if ((pt->min_x < x1) && (pt->max_x >= x0))
                                        {
                                            f0 = (s8)fl[0];
                                            if ((f0 >= 0) && (x0 < pt->min_x))
                                            {
                                                result = 2;
                                                break;
                                            }
                                            if (((s8)fl[1] >= 0) && (pt->max_x < x1 - 1))
                                            {
                                                result = 2;
                                                break;
                                            }
                                            if ((f0 >= 0) && ((s8)fl[1] >= 0))
                                            {
                                                result = 2;
                                                if (fl[0] == 0x7F)
                                                {
                                                    break;
                                                }
                                                if (fl[1] == 0x7F)
                                                {
                                                    break;
                                                }
                                            }
                                            result = 1;
                                        }
                                        pt++;
                                        fl += 2;
                                    }
                                }
                                else if (dx != 0)
                                {
                                    while (--count != -1)
                                    {
                                        value = dx;
                                        if (((pt->min_x + value) < x1) && ((pt->max_x + value) >= x0))
                                        {
                                            result = 1;
                                            break;
                                        }
                                        pt++;
                                    }
                                }
                                else
                                {
                                    while (--count != -1)
                                    {
                                        if ((pt->min_x < x1) && (pt->max_x >= x0))
                                        {
                                            result = 1;
                                            break;
                                        }
                                        pt++;
                                    }
                                }
                                if (result != 0)
                                {
                                    switch (((u8*)obj)[4] & 3)
                                    {
                                    case 0:
                                        if ((obj->top + dz) < h)
                                        {
                                            if (obj->flags & 4)
                                            {
                                                result = 2;
                                            }
                                            else
                                            {
                                                if (m->mode_flags & 0x30000)
                                                {
                                                    over = w < (obj->height0 + dz);
                                                    if (over != 0)
                                                    {
                                                        result = 2;
                                                    }
                                                    else
                                                    {
                                                        result |= 1;
                                                    }
                                                }
                                                else
                                                {
                                                    over = (w + 0x10) < (obj->height0 + dz);
                                                    if (over != 0)
                                                    {
                                                        result = 2;
                                                    }
                                                    else
                                                    {
                                                        result |= 1;
                                                    }
                                                }
                                            }
                                        }
                                        else
                                        {
                                            result = 1;
                                        }
                                        break;
                                    case 1:
                                        if ((obj->top + dz) < h)
                                        {
                                            gnd = func_8005DFAC(node, &probe->x);
                                            if (m->mode_flags & 0x30000)
                                            {
                                                over = w < gnd;
                                                if (over != 0)
                                                {
                                                    result = 2;
                                                }
                                                else
                                                {
                                                    result |= 1;
                                                }
                                            }
                                            else
                                            {
                                                over = (w + 0x10) < gnd;
                                                if (over != 0)
                                                {
                                                    result = 2;
                                                }
                                                else
                                                {
                                                    result |= 1;
                                                }
                                            }
                                        }
                                        else
                                        {
                                            result = 1;
                                        }
                                        break;
                                    }
                                    if (result & 2)
                                    {
                                        *hit_list = node;
                                        hit_list++;
                                        *out_hit += 1;
                                    }
                                    if (result & 1)
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
 * g_field_node_angle_table (0x0E, 0x0C and 0x0A, called A, B and C here); each
 * table entry is an x/y pair. The node's swept offsets at 0x34 and 0x40, both
 * divided by 256, translate B and C into world space.
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
 * wins, and a deviation of 0x472 or more (about 100 degrees of the 0x1000-unit
 * circle) counts as blocked.
 *
 * @param surface Surface definition owning the edge list, or NULL when @p edge_index is
 *            0x7E or 0x7F.
 * @param edge_index Edge index within the node, or 0x7E / 0x7F for a world edge.
 * @param move_angle Desired movement direction, in 0x1000 units per revolution.
 * @param best_angle Slide direction resolved so far: -2 means none yet and -1 means
 *             an earlier edge already blocked the movement.
 * @return The resolved slide direction, @p best_angle when the earlier one still
 *         wins, or -1 when the movement is blocked.
 */
s32 func_8005E1A8(FieldCollisionSurfaceDef* surface, s32 edge_index, s32 move_angle, s32 best_angle)
{
    s16* vertices;
    s16* previous_point;
    s16* edge_point;
    FieldCollisionEdgeRun* edge_run;
    s32 scratch;
    s32 slide_angle;
    s32 dx;
    s32 dy;
    s32 opposite_delta;
    s32 tmp;
    s32 wrapped_angle;

    if (best_angle == -1)
    {
        return -1;
    }
    previous_point = NULL;
    if (edge_index < 0x7E)
    {
        vertices = g_field_node_angle_table;
        for (edge_run = surface->runs; (scratch = edge_run->count & 0x7FFF) != 0; edge_run++)
        {
            edge_point = &vertices[edge_run->index * 2];
            while (--scratch != -1)
            {
                tmp = previous_point != NULL;
                if (tmp)
                {
                    if (edge_index == 0)
                    {
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
        slide_angle = (edge_index == 0x7E) << 10;
    }

    if (move_angle < slide_angle)
    {
        scratch = slide_angle - move_angle;
    }
    else
    {
        scratch = move_angle - slide_angle;
        if (scratch > 0x800)
        {
            wrapped_angle = move_angle - 0x1000;
            scratch = slide_angle - wrapped_angle;
        }
    }
    tmp = slide_angle + 0x800;
    if (move_angle < tmp)
    {
        opposite_delta = tmp - move_angle;
        if (opposite_delta > 0x800)
        {
            wrapped_angle = slide_angle - 0x800;
            opposite_delta = move_angle - wrapped_angle;
        }
    }
    else
    {
        opposite_delta = move_angle - tmp;
    }
    if (scratch == opposite_delta)
    {
        return -1;
    }
    if (opposite_delta < scratch)
    {
        slide_angle += 0x800;
    }
    if (best_angle == -2)
    {
        return slide_angle;
    }

    scratch = slide_angle - move_angle;
    if (scratch > 0x800)
    {
        scratch -= 0x1000;
    }
    else if (scratch < -0x800)
    {
        scratch += 0x1000;
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

    if ((scratch >= 0) && (opposite_delta >= 0))
    {
        if (scratch < opposite_delta)
        {
            slide_angle = best_angle;
        }
        else if (scratch >= 0x472)
        {
            slide_angle = -1;
        }
    }
    else if ((scratch <= 0) && (opposite_delta <= 0))
    {
        if (opposite_delta < scratch)
        {
            slide_angle = best_angle;
        }
        else if (scratch < -0x471)
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
 * @brief One boundary point of a collision node, as stored in
 *        g_field_node_angle_table.
 * @note Both fields are loaded unsigned by func_8005E3B0 (the asm uses lhu),
 *       which is why this is not the signed FieldCollisionSpan pair used for the
 *       min/max edge records at FieldCollisionNode::unk10.
 */
typedef struct FieldCollisionEdgePoint
{
    /** X coordinate. */
    u16 x;
    /** Z coordinate (scanline before node->unk22 is subtracted). */
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
 * @brief Rasterise a collision node's outline into per-scanline span lists.
 *
 * Phase 1 walks the node's edge-run list at @c def->runs and draws every
 * polygon edge with a Bresenham line, appending one FieldCollisionRasterSpan per scanline
 * touched. Consecutive edges that continue in the same vertical direction are
 * merged into the span the previous edge left on that row rather than starting
 * a new one. The whole edge body then runs once more for the closing edge back
 * to @c runs[0].index; that copy additionally merges against the very first
 * span of a row, so the outline joins up cleanly.
 *
 * Phase 2 bubble-sorts each row's spans by x0, merges them in pairs into the
 * final interior runs, pads the row out to its capacity with 0x80007F00 /
 * 0xFFFF, and finally word-copies the flag array to @c node->unk14.
 *
 * @param node Collision node being prepared. @c unk20 / @c unk22 are the
 *             bottom and top scanlines; the span list is stored to @c unk10
 *             and the flag list to @c unk14.
 * @param alloc In/out bump allocator. On entry it points at the free block
 *              used for both lists; on exit it is advanced past them,
 *              rounded up to a multiple of 4.
 *
 * @note The scratchpad at 0x1F800000 holds one span-count byte per scanline.
 * @note @c w is the per-row span capacity, twice the byte at offset 6 of the
 *       node definition. That byte overlaps FieldCollisionSurfaceDef::unk4, which other
 *       functions read as a word, so it is taken by cast rather than by
 *       resplitting a field they depend on.
 *
 * @note MATCHED 100% with gcc280_g4_noexpanddiv (874/874 insns, -0x60 frame).
 *       Two constructs in the closing-edge loop are load-bearing and must not
 *       be "cleaned up": the @c dy++/dy-- sequence pins @c dy to the register
 *       the target uses (resolving the a0/t0 allocation tie), and the span
 *       address is formed by explicit byte arithmetic rather than @c sp_row +
 *       @c n - 1 to reproduce the target's addressing.
 */
void func_8005E3B0(FieldCollisionNode* node, u8** alloc)
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
    FieldCollisionRasterSpan* out_span;
    FieldCollisionRasterSpan* p;
    FieldCollisionRasterSpan* a;
    FieldCollisionRasterSpanFlags* fl_row;
    FieldCollisionRasterSpanFlags* out_flag;
    FieldCollisionRasterSpanFlags* save_flags;
    FieldCollisionRasterSpanFlags* q;
    FieldCollisionRasterSpanFlags* b;
    FieldCollisionRasterSpan tmp_span;
    FieldCollisionRasterSpanFlags tmp_flag;
    u8* cp;
    s32 rows;
    s16 capacity;
    union
    {
        s32 count;
        FieldCollisionRasterSpan* span;
    } cursor;
    s32 j;
    s32 n;
    s32 sort_n;

    s32 last_dir;
    s32 attr;

    s32 nbytes;
    s32 hi;
    s32 closing_hi;
    u32 prev_hi;
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

    capacity = ((u8*)def)[6];
    capacity *= 2;
    counts = (u8*)FIELD_COLLISION_SCRATCH;
    spans = (FieldCollisionRasterSpan*)*alloc;
    node->spans = *alloc;
    cursor.count = rows + 1;
    nbytes = cursor.count * (capacity << 1);
    node->span_flags = *alloc + nbytes;
    flags = (FieldCollisionRasterSpanFlags*)(*alloc + (cursor.count * (capacity << 2)));
    *alloc = *alloc + (((cursor.count * ((capacity << 1) + capacity)) + 2) & ~3);

    w = capacity;
    cp = counts;
    for (cursor.count = rows; cursor.count != -1; cursor.count--)
    {
        *cp = 0;
        cp++;
    }

    last_dir = 0;
    prev = NULL;
    prev_hi = 0;
    first_dir = 0;
    edge = 0;
    run = def->runs;
    table = g_field_node_angle_table;
    cursor.count = run->count & 0x7FFF;
    while (cursor.count != 0)
    {
        pt = (FieldCollisionEdgePoint*)&table[run->index * 2];
        for (cursor.count = cursor.count - 1; cursor.count != -1; cursor.count--)
        {
            if (prev != NULL)
            {
                hi = 0;
                if (run->count & 0x8000)
                {
                    hi = prev_hi << 7;
                }
                attr = edge | hi;
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
                            fl_row[n - 1].f.f0 = attr | 0x7F;
                        }
                        if (sp_row[n - 1].x.x1 <= (s16)(x + dx))
                        {
                            sp_row[n - 1].x.x1 = x + dx;
                            fl_row[n - 1].f.f1 = attr | 0x7F;
                        }
                    }
                    else
                    {
                        last_dir = 2;
                        sp_row[0].x.x1 = x + dx;
                        sp_row[0].x.x0 = x;
                        fl_row[0].f.f1 = attr | 0x7F;
                        fl_row[0].f.f0 = attr | 0x7F;
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
            prev_hi = run->count >> 15;
        }
        run++;
        cursor.count = run->count & 0x7FFF;
    }

    /* closing edge: back to the first run's first point */
    closing_hi = 0;
    run = def->runs;
    y0 = run->count & 0x8000;
    if (y0)
    {
        closing_hi = prev_hi << 7;
    }
    pt = (FieldCollisionEdgePoint*)&table[run->index * 2];
    attr = edge | closing_hi;
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
                /* ignore the man behind the curtain..... */
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
                    cursor.span = (FieldCollisionRasterSpan*)((n * sizeof(*sp_row)) + (s32)sp_row - sizeof(*sp_row));
                    if (sp_row[n - 1].x.x0 > x)
                    {
                        sp_row[n - 1].x.x0 = x;
                        fl_row[n - 1].f.f0 = attr;
                    }
                    if (cursor.span->x.x1 < x)
                    {
                        cursor.span->x.x1 = x;
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
            fl_row[n].f.f1 = attr | 0x7F;
            fl_row[n].f.f0 = attr | 0x7F;
            *cp = n + 1;
        }
        else
        {
            if (sp_row[n - 1].x.x0 >= x)
            {
                sp_row[n - 1].x.x0 = x;
                fl_row[n - 1].f.f0 = attr | 0x7F;
            }
            if (sp_row[n - 1].x.x1 <= (s16)(x + dx))
            {
                sp_row[n - 1].x.x1 = x + dx;
                fl_row[n - 1].f.f1 = attr | 0x7F;
            }
        }
    }

    capacity = w;
    out_span = spans;
    out_flag = flags;
    save_flags = flags;
    for (cursor.count = node->max_z - node->min_z; cursor.count != -1; cursor.count--)
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
                out_span->x.x0 = p[0].x.x0;
                out_span->x.x1 = p[1].x.x1;
                out_flag->f.f0 = q[0].f.f0;
                out_flag->f.f1 = q[1].f.f1;
            }
            else
            {
                *out_span = p[0];
                *out_flag = q[0];
            }
            out_span++;
            p += 2;
            out_flag++;
            q += 2;
        }

        j = (capacity - *counts) / 2;
        j--;
        for (; j != -1; j--)
        {
            do
            {
                out_span->word = 0x80007F00;
                out_span++;
            } while (0);
        }
        j = (capacity - *counts) / 2;
        j--;
        for (; j != -1; j--)
        {
            out_flag->half = 0xFFFF;
            out_flag++;
        }

        spans += capacity;
        flags += capacity;
        counts++;
    }

    cursor.count = w * (((node->max_z - node->min_z) + 2) / 2);
    fl_row = save_flags;
    flags = (FieldCollisionRasterSpanFlags*)node->span_flags;
    for (cursor.count = cursor.count - 1; cursor.count != -1; cursor.count--)
    {
        *(s32*)flags = *(s32*)fl_row;
        fl_row += 2;
        flags += 2;
    }
}

/**
 * @brief One entry of func_8005F158's group-collection scratch list.
 *
 * @note @c seen accumulates which node modes referenced the id: bit 0 from a
 *       mode-0 node, bit 1 from a mode-1 node. A pair-mode scene keeps only
 *       the entries that ended up with both bits set.
 */
typedef struct
{
    /** Group id, taken from FieldNodeDef::base_x or base_y. */
    s16 id;
    /** Bitmask of the modes that referenced this id; 3 means both. */
    s16 seen;
} FieldGroupEntry;

void func_8005F5BC();

/**
 * @brief Collect the scene's distinct node groups and size their tile budget.
 *
 * Walks the attached-node list and builds a list of the distinct group ids
 * carried by each node's definition, skipping nodes that are inactive
 * (@c unk18 == 0) or opted out (@c flags bit 2). A mode-0 node contributes one
 * id, a mode-1 node contributes two (@c base_x and @c base_y); an id already
 * present just gets its @c seen mask widened. Ids that are zero are recorded
 * with a @c seen value of 3 so they survive the pair filter unconditionally.
 *
 * If any mode-1 node was seen, the list is then compacted down to the entries
 * whose @c seen is 3. The surviving ids are sorted into ascending order in
 * @c scene->unk4A and the matching @c scene->unk5E counters are cleared.
 *
 * Finally the per-group tile budget is computed from the scene's pixel extent:
 * 4-pixel tiles normally, 8-pixel tiles once the estimated tile count would
 * exceed 0x4000. Two blocks are carved off @p alloc - the tile area
 * (@c unk2C) and the work area (@c unk28 .. @c unk30) - and func_8005F5BC is
 * handed the work-area geometry.
 *
 * @param alloc In/out bump allocator; advanced past both blocks on success.
 *
 * @note Bails out with @c unk28 = 0 and @c unk41 = 1 if more than 20 groups
 *       are found during the scan, or more than 10 survive the pair filter.
 *       The empty-scene path leaves @c unk41 = 0 instead, which is how callers
 *       tell "no nodes" from "too many groups".
 *
 */
void func_8005F158(s32* alloc)
{
    FieldGroupEntry list[20];
    FieldScene* scene;
    FieldNode* node;
    FieldNodeDef* def;
    FieldSceneHeader* header;
    s32 i;
    s32 j;
    s32 has_pair;
    s32 kind;
    s32 kind2;
    s32 k;
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
        if ((node->unk18 != 0) && !(def->flags & 4))
        {
            i = count - 1;
            switch ((u8)def->flags & 3)
            {
            case 0:
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
                    if (count >= 20)
                    {
                        goto overflow;
                    }
                    count++;
                }
                break;

            case 1:
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
                        kind = 2;
                        list[count].id = def->base_x;
                    }
                    else
                    {
                        kind = 3;
                        list[count].id = 0;
                    }
                    list[count].seen = kind;
                    if (count >= 20)
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
                        kind2 = 2;
                        list[count].id = def->base_y;
                    }
                    else
                    {
                        kind2 = 3;
                        list[count].id = 0;
                    }
                    list[count].seen = kind2;
                    if (count >= 20)
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
    else if (count >= 11)
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
    if (i >= 0x4001)
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
    func_8005F5BC(alloc, 0, i);
    return;

overflow:
    scene->unk28 = 0;
    scene->unk41 = 1;
}

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
 * sub-row (ping-ponged between the two halves of the PSX scratchpad at
 * 0x1F800000 / 0x1F800200) and a union in @c acc. The intersection drives
 * plane 0 and the union drives plane 1 of the output words.
 *
 * @param unused Unused; the target never reads the first argument.
 * @param clip Optional clipping node. When NULL every tile row of the group is
 *             emitted; otherwise only the rows the node covers are, and its
 *             definition is tested against the group id first.
 *
 */
void func_8005F5BC(s32 unused, FieldNode* clip)
{
    FieldCollisionTileSpan cur[128];
    FieldCollisionTileSpan acc[128];
    FieldCollisionRasterNode list[50];
    FieldCollisionSpanRun runs[50];
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
    s32 n_sp;
    FieldCollisionSpanRun* runs_base;
    u32* out;
    u32 ncur;
    u32 nacc;
    s32 clip_tail;
    s32 clip_rows;
    s32 nrun;
    FieldNode* nd;
    FieldNodeDef* def;
    FieldCollisionRasterNode* p;
    u32 list_offset;
    s32 group_end;
    s32 union_end;
    s32 eligible;
    s32 threshold;
    s32 node_start;
    FieldCollisionTileSpan* sp1;
    FieldCollisionTileSpan* sp2;
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
    s32 w0;
    s32 w1;
    s32 word;
    s32 zero_v;
    s32 m0;
    s32 m1;
    s32 lead;
    s32 tail;

    s32 wtail;
    s16 base;
    u16 id;
    s32 signed_id;
    u16 tmp;
    u16 x0;
    u16 x1;

    zero_v = 0;
    saved = NULL;
    nacc = 0;
    scene = g_field_scene.scene;
    out = (u32*)scene->unk28;
    n_sp = 0;
    if (out == NULL)
    {
        return;
    }

    nd = scene->nodes;
    node_count = 0;
    if (nd != NULL)
    {
        p = list;
        do
        {
            if (nd->unk18 != 0)
            {
                if (node_count >= 50)
                {
                    scene->unk28 = 0;
                    scene->unk41 = 2;
                    return;
                }
                p->node = nd;
                node_count++;
                p->key = nd->row_start;
                p++;
            }
            nd = nd->next;
        } while (nd != NULL);
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
                        tmp = list[k].key;
                        list[k].key = base;
                        base = tmp;
                        list[i].key = tmp;
                        nd = list[k].node;
                        list[k].node = list[i].node;
                        list[i].node = nd;
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

    group_end = -1;
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
            if (def->flags & 4)
            {
                fresh = def->id_min <= (s16)id;
            }
            else if ((def->flags & 3) == 1)
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
                w0 = def->base_x;
                w0 = (s16)id < w0;
                if (w0 != 0)
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

        nrun = zero_v;
        j = 0;
        rows = rows - 1;
        if (rows != group_end)
        {
            signed_id = (s16)id;
            list_offset = 0;
            do
            {
                if (j < node_count)
                {
                    def = (FieldNodeDef*)list;
                    if (((FieldCollisionRasterNode*)((u8*)def + list_offset))->key < ((s16)base + tile2))
                    {
                        m0 = (s16)base;
                        threshold = (s16)base + tile2;
                        do
                        {
                            nd = ((FieldCollisionRasterNode*)((u8*)def + list_offset))->node;
                            def = nd->def;
                            fresh = 0;
                            if (def->flags & 4)
                            {
                                fresh = signed_id >= def->id_min;
                            }
                            else
                            {
                                do
                                {
                                    if ((def->flags & 3) == 1)
                                    {
                                        lo2 = def->base_x;
                                        hi2 = def->base_y;
                                        if (lo2 < hi2)
                                        {
                                            eligible = signed_id < lo2;
                                        }
                                        else
                                        {
                                            eligible = signed_id < hi2;
                                        }
                                    }
                                    else
                                    {
                                        eligible = signed_id < def->base_x;
                                    }
                                    if ((eligible != 0) && (signed_id >= def->id_min))
                                    {
                                        fresh = 1;
                                    }
                                } while (0);
                            }
                            if ((fresh != 0) && (nd->row_end >= m0))
                            {
                                node_start = nd->row_start;
                                if (m0 >= node_start)
                                {
                                    runs_base[nrun].src = nd->spans + ((m0 - node_start) * FIELD_NODE_DEF_ROWS(nd->def) * 2);
                                    runs_base[nrun].count = ((u16)nd->row_end - (s16)base) + 1;
                                    runs_base[nrun].skip = 0;
                                }
                                else
                                {
                                    runs_base[nrun].src = nd->spans;
                                    runs_base[nrun].count = ((u16)nd->row_end - (u16)nd->row_start) + 1;
                                    runs_base[nrun].skip = (u16)nd->row_start - (s16)base;
                                }
                                runs_base[nrun].step = FIELD_NODE_DEF_ROWS(nd->def);
                                nrun++;
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
                if (i != group_end)
                {
                    s32 clear_end = -1;
                    do
                    {
                        wp += 2;
                        i--;
                        wp[1] = 0;
                        wp[0] = 0;
                    } while (i != clear_end);
                }

                node_start = ((u16)scene->unk46 - 1) & 0x1F;
                if (node_start == 0)
                {
                    w0 = wp[-2] | 0x80000000;
                    node_start = wp[0] | 1;
                    wp[-1] = w0;
                    wp[-2] = w0;
                    wp[1] = node_start;
                    wp[0] = node_start;
                }
                else
                {
                    m0 = node_start;
                    m0 = 1 << m0;
                    w0 = wp[0] | m0 | ((u32)m0 >> 1);
                    wp[1] = w0;
                    wp[0] = w0;
                }

                if (nrun != 0)
                {
                    i = tile2 - 1;
                    if (i != group_end)
                    {
                        do
                        {
                            k = nrun;
                            k--;
                            ncur = 0;
                            if (k != group_end)
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
                                        if (n != group_end)
                                        {
                                            do
                                            {
                                                x0 = src[0];
                                                x1 = src[1];
                                                src += 2;
                                                if ((s16)x0 <= (s16)x1)
                                                {
                                                    sp1 = cur;
                                                    m0 = ncur;
                                                    m0--;
                                                    fresh = 1;
                                                    if (m0 != group_end)
                                                    {
                                                        s32 collect_end = -1;
                                                        do
                                                        {
                                                            if ((((s16)x1 + 1) >= sp1->x0) && ((sp1->x1 + 1) >= (s16)x0))
                                                            {
                                                                if ((s16)x0 < sp1->x0)
                                                                {
                                                                    sp1->x0 = x0;
                                                                }
                                                                fresh = 0;
                                                                do
                                                                {
                                                                    if (sp1->x1 < (s16)x1)
                                                                    {
                                                                        sp1->x1 = x1;
                                                                    }
                                                                } while (0);
                                                                break;
                                                            }
                                                            sp1++;
                                                            m0--;
                                                        } while (m0 != collect_end);
                                                    }
                                                    if (fresh != 0)
                                                    {
                                                        if (ncur >= 0x80)
                                                        {
                                                            scene->unk28 = 0;
                                                            scene->unk41 = 3;
                                                            return;
                                                        }
                                                        ncur++;
                                                        sp1->x0 = x0;
                                                        sp1->x1 = x1;
                                                    }
                                                }
                                                n--;
                                            } while (n != group_end);
                                        }
                                        if (runs_base[k].count == 0)
                                        {
                                            nrun--;
                                            if (k != nrun)
                                            {
                                                runs_base[k].src = runs_base[nrun].src;
                                                runs_base[k].count = runs_base[nrun].count;
                                                runs_base[k].step = runs_base[nrun].step;
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
                                } while (k != group_end);
                            }

                            k = ncur - 1;
                            sp1 = cur;
                            if (k != group_end)
                            {
                                do
                                {
                                    n = k - 1;
                                    do
                                    {
                                        x0 = sp1->x0;
                                    } while (0);
                                    x1 = sp1->x1;
                                    other = sp1 + 1;
                                    if (n != group_end)
                                    {
                                        s32 compact_end = -1;
                                        do
                                        {
                                            if ((((s16)x1 + 1) >= other->x0) && ((s16)x0 <= (other->x1 + 1)))
                                            {
                                                ncur--;
                                                k--;
                                                if (other->x0 < (s16)x0 || (s16)x1 < other->x1)
                                                {
                                                    if (other->x0 < (s16)x0)
                                                    {
                                                        x0 = other->x0;
                                                    }
                                                    if ((s16)x1 < other->x1)
                                                    {
                                                        x1 = other->x1;
                                                    }
                                                    sp1->x0 = x0;
                                                    sp1->x1 = x1;
                                                    if (n != 0)
                                                    {
                                                        *(s32*)other = *(s32*)&other[n];
                                                    }
                                                    n = k;
                                                    other = sp1 + 1;
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
                                    sp1++;
                                    k--;
                                } while (k != group_end);
                            }

                            if (i == (tile2 - 1))
                            {
                                sp1 = cur;
                                other = acc;
                                prev = (FieldCollisionTileSpan*)FIELD_COLLISION_SCRATCH;
                                nacc = ncur;
                                n_sp = nacc;
                                k = nacc - 1;
                                if (k != group_end)
                                {
                                    s32 copy_end = -1;
                                    do
                                    {
                                        word = *(s32*)sp1;
                                        sp1++;
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
                                sp1 = cur;
                                if (!(i & 1))
                                {
                                    prev = (FieldCollisionTileSpan*)FIELD_COLLISION_SCRATCH;
                                    dst = (FieldCollisionTileSpan*)(FIELD_COLLISION_SCRATCH + 0x200);
                                }
                                else
                                {
                                    prev = (FieldCollisionTileSpan*)(FIELD_COLLISION_SCRATCH + 0x200);
                                    dst = (FieldCollisionTileSpan*)FIELD_COLLISION_SCRATCH;
                                }
                                k = ncur - 1;
                                m0 = 0;
                                if (k != group_end)
                                {
                                    do
                                    {
                                        x0 = sp1->x0;
                                        x1 = sp1->x1;
                                        n = n_sp;
                                        n = n - 1;
                                        sp2 = prev;
                                        if (n != group_end)
                                        {
                                            s32 intersect_end;
                                            do
                                            {
                                                if (((s16)x1 >= sp2->x0) && (sp2->x1 >= (s16)x0))
                                                {
                                                    if (m0 >= 0x80)
                                                    {
                                                        scene->unk28 = 0;
                                                        scene->unk41 = 4;
                                                        return;
                                                    }
                                                    if ((s16)x0 < sp2->x0)
                                                    {
                                                        dst->x0 = sp2->x0;
                                                    }
                                                    else
                                                    {
                                                        dst->x0 = x0;
                                                    }
                                                    w1 = sp2->x1 < (s16)x1;
                                                    if (w1)
                                                    {
                                                        dst->x1 = sp2->x1;
                                                    }
                                                    else
                                                    {
                                                        dst->x1 = x1;
                                                    }
                                                    do
                                                    {
                                                        m0++;
                                                        dst++;
                                                    } while (0);
                                                }
                                                intersect_end = -1;
                                                sp2++;
                                                n--;
                                            } while (n != intersect_end);
                                        }

                                        other = acc;
                                        n = nacc - 1;
                                        fresh = 1;
                                        if (n != group_end)
                                        {
                                            do
                                            {
                                                if ((((s16)x1 + 1) >= other->x0) && ((other->x1 + 1) >= (s16)x0))
                                                {
                                                    if ((s16)x0 < other->x0)
                                                    {
                                                        other->x0 = x0;
                                                    }
                                                    fresh = 0;
                                                    do
                                                    {
                                                        if (other->x1 < (s16)x1)
                                                        {
                                                            other->x1 = x1;
                                                        }
                                                    } while (0);
                                                    break;
                                                }
                                                union_end = -1;
                                                other++;
                                                n--;
                                            } while (n != union_end);
                                        }
                                        if (fresh != 0)
                                        {
                                            if (nacc >= 0x80)
                                            {
                                                scene->unk28 = 0;
                                                scene->unk41 = 5;
                                                return;
                                            }
                                            nacc++;
                                            other->x0 = x0;
                                            other->x1 = x1;
                                        }
                                        k--;
                                        sp1++;
                                    } while (k != group_end);
                                }
                                n_sp = m0;
                            }
                            i--;
                        } while (i != group_end);
                    }

                    prev = (FieldCollisionTileSpan*)(FIELD_COLLISION_SCRATCH + 0x200);
                    i = n_sp;
                    i--;
                    if (i != group_end)
                    {
                        do
                        {
                            lead = (((prev->x0 + tile) - 1) >> shift0) + 2;
                            tail = ((prev->x1 + 1) >> shift0) + 1;
                            n = lead >> 5;
                            if (tail >= lead)
                            {
                                wtail = tail >> 5;
                                wp = out + (n * 2);
                                m0 = lead;
                                m0 = group_end << (m0 & 0x1F);
                                m1 = (u32)group_end >> (0x1F - (tail & 0x1F));
                                if (n != wtail)
                                {
                                    k = (wtail - n) - 2;
                                    wp[0] |= m0;
                                    wp += 2;
                                    if (k != group_end)
                                    {
                                        do
                                        {
                                            wp[0] = group_end;
                                            k--;
                                            wp += 2;
                                        } while (k != group_end);
                                    }
                                    wp[0] |= m1;
                                }
                                else
                                {
                                    wp[0] |= m0 & m1;
                                }
                            }
                            i--;
                            prev++;
                        } while (i != group_end);
                    }

                    i = nacc - 1;
                    other = acc;
                    if (i != group_end)
                    {
                        do
                        {
                            lead = (other->x0 >> shift0) + 2;
                            tail = other->x1 >> shift0;
                            tail = tail + 2;
                            n = lead >> 5;
                            wtail = tail >> 5;
                            m0 = lead;
                            m0 = group_end << (m0 & 0x1F);
                            m1 = (u32)group_end >> (0x1F - (tail & 0x1F));
                            wq = out + (n * 2);
                            if (n != wtail)
                            {
                                wp = wq + 3;
                                k = (wtail - n) - 2;
                                wq[1] |= m0;
                                if (k != group_end)
                                {
                                    do
                                    {
                                        wp[0] = group_end;
                                        k--;
                                        wp += 2;
                                    } while (k != group_end);
                                }
                                wp[0] |= m1;
                            }
                            else
                            {
                                wq[1] |= m0 & m1;
                            }
                            i--;
                            other++;
                        } while (i != group_end);
                    }
                }

                out += words * 2;
                base += tile2;
                rows--;
            } while (rows != group_end);
        }

        if (clip != NULL)
        {
            out = saved;
        }
        group++;
    } while (group != scene->unk41);
}

/**
 * @brief Expand a group's rasterised tile-column bitmask into a per-pixel
 *        stencil buffer, dilated by the group's edge width.
 * @param footprint_width Footprint width in collision-map columns.
 * @param footprint_depth Footprint depth in collision-map rows.
 */
void func_80060364(s32 footprint_width, s32 footprint_depth)
{
    FieldScene* scene;
    u32 src_words;
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
    u32 next_input;
    s32 reach;
    s32 src;
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
    u32 near_touch;
    u32 acc_touch;
    u32 emit_mask;
    u32 left_bits;
    u32 acc_solid;
    u32 spread;
    u32 shift_solid;
    u32 shift_touch;
    u32 carry_touch;
    u32 carry_solid;
    s32 bits_left;
    u32 ring_touch;
    u32 ring_near;
    u32 ring_solid_a;
    u32 ring_solid_b;
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
    src_words = (u32)scene->unk28;
    group_count = scene->unk41;
    groups_left = (s32)group_count;
    out = (u8*)scene->unk2C;
    last_group = group_count - 1;
    groups_left = last_group;
    if (last_group != -1)
    {
        word_bits = 0x20;
        do
        {
            count = (4 - (s32)out) & 3;
            remaining = (u16)scene->unk46 * 2;
            while ((count != 0) && (remaining != 0))
            {
                *out = -1;
                out += 1;
                count -= 1;
                remaining -= 1;
            }
            count = (remaining >> 2);
            count -= 1;
            if (count != -1)
            {
                do
                {

                    *(s32*)out = -1;

                    count -= 1;
                    out += 4;
                } while (count != -1);
            }
            count = (remaining & 3);
            count -= 1;
            if (count != -1)
            {
                s32 end = -1;
                do
                {
                    *out = -1;
                    count -= 1;
                    out += 1;
                } while (count != end);
            }
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
                    src = src_words;
                    remaining = (u16)scene->unk46;
                    carry_solid = *(u32*)(src + 4);
                    src_words = src + (row_words * 4);
                    in_solid = *(u32*)(src + 0);
                    in_touch = carry_solid;
                    value = remaining - 1;
                    remaining = value - footprint_width;
                    src += 8;
                    switch (footprint_width - 1)
                    {
                    default:
                        acc_touch = in_touch | (in_touch >> reach);
                        near_touch = (in_touch >> 1) | (in_touch >> 2);
                        shift_touch = near_touch;
                        shift_solid = in_solid | (in_solid >> 1);
                        spread = shift_solid >> 2;
                        acc_solid = shift_solid | spread;
                        shift_solid = spread;
                        count = (footprint_width - 6) >> 1;
                        do
                        {
                            shift_touch = shift_touch >> 2;
                            near_touch |= shift_touch;
                            shift_solid = shift_solid >> 2;
                            count -= 1;
                            acc_solid |= shift_solid;
                        } while (count != -1);
                        if (footprint_width & 1)
                        {
                            near_touch |= in_touch >> (footprint_width - 2);
                            acc_solid |= in_solid >> reach;
                        }
                        goto block_25;
                    case 4:
                        near_touch = (in_touch >> 1) | (in_touch >> 2) | (in_touch >> 3);
                        acc_touch = in_touch | (in_touch >> 4);
                        shift_solid = in_solid | (in_solid >> 1);
                        acc_solid = shift_solid | (shift_solid >> 2) | (in_solid >> 4);
                        goto block_25;
                    case 3:
                        near_touch = (in_touch >> 1) | (in_touch >> 2);
                        acc_touch = in_touch | (in_touch >> 3);
                        shift_solid = in_solid | (in_solid >> 1);
                        acc_solid = shift_solid | (shift_solid >> 2);
                        goto block_25;
                    case 2:
                        near_touch = in_touch >> 1;
                        acc_touch = in_touch | (in_touch >> 2);
                        acc_solid = in_solid | (in_solid >> 1) | (in_solid >> 2);
                        goto block_25;
                    case 1:
                        near_touch = 0;
                        acc_touch = in_touch | (in_touch >> 1);
                        acc_solid = in_solid | (in_solid >> 1);
                    block_25:
                        carry_touch = in_touch;
                        carry_solid = in_solid;
                        break;
                    case 0:
                        near_touch = 0;
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
                            do
                            {
                                if (ring_write >= ring_end)
                                {
                                    ring_write -= ring_bytes;
                                }
                            } while (0);
                            if (ring_read >= ring_end)
                            {
                                ring_read -= ring_bytes;
                            }
                        }
                    }
                    if (remaining != 0)
                    {
                        ring_touch = ring_write + 8;
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
                        acc_touch = acc_touch | near_touch;
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
                            *(s32*)(ring_touch - 4) = acc_touch;
                            ring_touch += 0xC;
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
                            *(s32*)(ring_touch - 4) = acc_touch;
                            *(s32*)(ring_touch + 0) = near_touch;
                            ring_touch += 0xC;
                            ring_write += 0xC;
                            if (row >= (footprint_depth - 1))
                            {
                                shift_touch = ring_read + ring_row_bytes;
                                acc_touch = acc_touch | *(u32*)(ring_read + 4);
                                in_solid = *(u32*)(ring_read + 0) & 0xFFFF;
                                acc_solid |= in_solid | (*(u32*)(ring_read + 0) & 0xFFFF0000);
                                if (shift_touch >= ring_end)
                                {
                                    shift_touch -= ring_bytes;
                                }
                                count = footprint_depth - 2;
                                do
                                {
                                    ring_near = *(u32*)(shift_touch + 4);
                                    ring_solid_a = *(u32*)(shift_touch + 0);
                                    ring_solid_b = *(u32*)(shift_touch + 8);
                                    do
                                    {
                                        shift_touch = shift_touch + ring_row_bytes;
                                    } while (0);
                                    acc_solid |= ring_solid_a | ring_solid_b;
                                    acc_touch |= ring_near;
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
                            in_solid = *(u32*)(src + 0);
                            in_touch = *(u32*)(src + 4);

                            emit_mask = in_solid;

                            src += 8;
                            next_input = emit_mask;
                            switch (footprint_width - 1)
                            {
                            default:
                                do
                                {
                                    s32 edge_shift = 0x20 - reach;
                                    s32 word_bits_plus_one = 0x21;
                                    s32 inner_shift = word_bits_plus_one - reach;
                                    acc_touch = carry_touch >> edge_shift;
                                    near_touch = (carry_touch >> inner_shift) | (carry_touch >> (0x22 - reach));
                                    shift_touch = near_touch;
                                    count = (footprint_width - 6) >> 1;
                                } while (0);
                                do
                                {
                                    shift_touch = shift_touch >> 2;
                                    count -= 1;
                                    near_touch |= shift_touch;
                                } while (count != -1);
                                {
                                    s32 edge_shift = 0x20 - reach;
                                    s32 word_bits_plus_one = 0x21;
                                    s32 inner_shift = word_bits_plus_one - reach;
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
                                    near_touch |= carry_touch >> 0x1F;
                                }
                                else
                                {
                                    acc_solid |= carry_solid >> 0x1F;
                                }
                                if (in_touch != 0)
                                {
                                    shift_touch = (in_touch * 2) | (in_touch * 4);
                                    near_touch |= shift_touch;

                                    shift_solid = next_input | (next_input * 2);

                                    spread4 = shift_solid * 4;
                                    acc_solid |= shift_solid | spread4;
                                    shift_solid = spread4;
                                    count = (footprint_width - 6) >> 1;
                                    do
                                    {
                                        shift_touch *= 4;
                                        near_touch |= shift_touch;
                                        shift_solid *= 4;
                                        count -= 1;
                                        acc_solid |= shift_solid;
                                    } while (count != -1);
                                    if (footprint_width & 1)
                                    {
                                        near_touch |= in_touch << (footprint_width - 2);

                                        acc_solid |= next_input << reach;
                                    }
                                    acc_touch |= in_touch | (in_touch << reach);
                                    goto block_96;
                                }
                                goto block_97;
                            case 4:
                                do
                                {
                                    near_touch = (carry_touch >> 0x1D) | (carry_touch >> 0x1E) | (carry_touch >> 0x1F);
                                    acc_touch = carry_touch;
                                    acc_touch >>= 0x1C;
                                    shift_solid = (carry_solid >> 0x1C) | (carry_solid >> 0x1D);
                                    acc_solid = shift_solid | (shift_solid >> 2);
                                } while (0);
                                if (in_touch != 0)
                                {
                                    left_bits = (in_touch * 2) | (in_touch * 4) | (in_touch * 8);
                                    near_touch |= left_bits;
                                    acc_touch |= in_touch | (in_touch * 0x10);

                                    shift_solid = next_input | (next_input * 2);

                                    spill_bits = shift_solid * 4;
                                    spread_solid = shift_solid | spill_bits;
                                    left_bits = next_input * 0x10;
                                    spread_solid |= left_bits;

                                    goto block_95;
                                }
                                goto block_97;
                            case 3:
                                do
                                {
                                    near_touch = (carry_touch >> 0x1E) | (carry_touch >> 0x1F);
                                    acc_touch = carry_touch >> 0x1D;
                                    left_bits = (carry_solid >> 0x1D) | (carry_solid >> 0x1E);
                                    carry_solid = (s32)carry_solid >> 0x1F;
                                    acc_solid = left_bits | (carry_solid & 1);
                                } while (0);
                                if (in_touch != 0)
                                {
                                    left_bits = (in_touch * 2) | (in_touch * 4);
                                    near_touch |= left_bits;
                                    acc_touch |= in_touch | (in_touch * 8);

                                    shift_solid = next_input | (next_input * 2);

                                    spread_solid = shift_solid | (shift_solid * 4);
                                    goto block_95;
                                }
                                goto block_97;
                            case 2:

                                near_touch = carry_touch >> 0x1F;
                                acc_touch = carry_touch >> 0x1E;
                                acc_solid = (carry_solid >> 0x1E) | (carry_solid >> 0x1F);

                                if (in_touch != 0)
                                {
                                    near_touch |= in_touch * 2;
                                    acc_touch |= in_touch | (in_touch * 4);

                                    spill_bits = next_input * 2;
                                    spread_solid = next_input | spill_bits;
                                    left_bits = next_input * 4;
                                    spread_solid |= left_bits;

                                    goto block_95;
                                }
                                goto block_97;
                            case 1:
                                do
                                {
                                    acc_touch = carry_touch >> 0x1F;
                                    acc_solid = carry_solid >> 0x1F;
                                } while (0);
                                if (in_touch != 0)
                                {
                                    acc_touch |= in_touch | (in_touch * 2);

                                    spread_solid = next_input | (next_input * 2);

                                block_95:
                                    acc_solid |= spread_solid;
                                block_96:
                                    carry_touch = in_touch;
                                    do
                                    {
                                        carry_solid = next_input;
                                    } while (0);
                                    break;
                                }
                            block_97:
                                carry_touch = 0;
                                carry_solid = 0;
                                break;
                            case 0:

                                acc_solid = next_input;

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
            count = (4 - (s32)out) & 3;
            remaining = (u16)scene->unk46 * (footprint_depth + 1);
            while ((count != 0) && (remaining != 0))
            {
                *out = -1;
                out += 1;
                count -= 1;
                remaining -= 1;
            }
            count = (remaining >> 2);
            count -= 1;
            if (count != -1)
            {
                do
                {
                    *(s32*)out = -1;
                    count -= 1;
                    out += 4;
                } while (count != -1);
            }
            count = (remaining & 3);
            count -= 1;
            if (count != -1)
            {
                s32 end = -1;
                do
                {
                    *out = -1;
                    count -= 1;
                    out += 1;
                } while (count != end);
            }
            groups_left--;
        } while (groups_left != -1);
    }
}

typedef struct
{
    u8 pad0[0xC];
    s16 x_margin;
    u8 pad1[0x10 - 0xE];
    s16 z_margin;
} FieldCollisionMargin;

/**
 * @brief Mark the collision-map tiles covered by a query footprint and margins.
 * @param margins Extra X/Z footprint margins.
 * @param query World-space query position and footprint.
 * @return 0 on success, -1 when group data exists without a work map, or -2 when the footprint lies outside the usable map.
 */
s32 func_80060CB0(FieldCollisionMargin* margins, FieldCollisionQuery* query)
{
    FieldScene* scene;
    s32 count;
    s32 group;
    s32 i;
    s32 v;
    s32 y;
    s32 g;
    s32 gy;
    s32 half;
    s32 half2;
    s32 ext_x;
    s32 ext_z;
    s16 raw_margin_x;
    s16 raw_margin_z;
    s32 margin;
    s32 tile;
    s32 shift;
    s32 shift2;
    s32 mask;
    s32 mask2;
    s32 tx;
    s32 tz;
    s32 col_start;
    s32 row_start;
    s32 ncol;
    s32 nrow;
    s32 cols;
    s32 rows;
    s32 col;
    s32 n;
    u8* base;
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

    y = query->y;
    gy = y >> 8;
    if (y < 0)
    {
        gy = (y + 0xFF) >> 8;
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
            s32 gid = scene->unk4A[i];

            if (gy < gid)
            {
                if (i != 0)
                {
                    i -= 1;
                }
                group = i;
                break;
            }
            else if (gid == gy)
            {
                group = i;
                break;
            }
        }
    }

    tile = scene->unk40;
    shift = 3;
    if (tile == 4)
    {
        shift = 2;
    }

    ext_x = query->width;
    half = ((s16)ext_x) >> 1;
    do
    {
        raw_margin_x = margins->x_margin;
    } while (0);
    margin = raw_margin_x - 1;
    v = query->x;
    if (v >= 0)
    {
        g = v >> 8;
    }
    else
    {
        g = (v + 0xFF) >> 8;
    }
    do
    {
        tx = (g - half) - margin;
    } while (0);

    ext_z = query->depth;
    half2 = ((s16)ext_z) >> 1;
    do
    {
        raw_margin_z = margins->z_margin;
    } while (0);
    margin = raw_margin_z - 1;
    v = query->z;
    if (v >= 0)
    {
        g = v >> 8;
    }
    else
    {
        g = (v + 0xFF) >> 8;
    }
    tz = (g - half2) - margin;

    col_start = (tx >> shift) + 2;
    shift2 = shift + 1;
    row_start = (tz >> shift2) + 2;
    mask = tile - 1;
    ncol = (((tx & mask) + margins->x_margin + ((s16)query->width) + mask) - 1) >> shift;
    mask2 = (tile * 2) - 1;
    nrow = (((tz & mask2) + margins->z_margin + ((s16)query->depth) + mask2) - 1) >> shift2;
    do
    {
        cols = (u16)scene->unk46;
    } while (0);
    rows = (u16)scene->unk48;

    if (col_start <= 0)
    {
        g = ncol - 1;
        g = g + col_start;
        if (g <= 0)
        {
            goto bounds_fail;
        }
        ncol = g;
        col_start = 1;
    }
    if (row_start > 0)
    {
        goto check_bounds;
    }
    if ((nrow + (row_start - 1)) > 0)
    {
        goto row_clip_ok;
    }

bounds_fail:
    return -2;

row_clip_ok:
    ncol += col_start - 1;
    col_start = 1;

check_bounds:
    if (col_start >= cols - 1)
    {
        goto bounds_fail;
    }
    if (row_start >= rows - 1)
    {
        goto bounds_fail;
    }

    tz = (u16)scene->unk44 * group;
    base = (u8*)(scene->unk2C + tz + (cols * row_start) + col_start);
    for (nrow -= 1; nrow != -1; nrow--)
    {
        p = base;
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
            p += 1;
            col += 1;
        }
        row_start += 1;
    }
    return 0;
}

/** Two-word result written back through arg2. */
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
    s32 unk24;
    s32 mode;
    u8 stamp;
} FieldCollisionTraceRequest;

/**
 * @brief Find a collision-grid route between two footprint queries.
 * @param start_query Starting query and footprint dimensions.
 * @param goal_query Destination query.
 * @param output_path Output X/Z path points.
 * @param mode Trace mode forwarded to the footprint walker.
 * @return Number of path points, 1 for a direct result, or a negative failure code.
 *
 */
s32 func_80060F58(FieldCollisionQuery* start_query, FieldCollisionQuery* goal_query, FieldCollisionPathPoint* output_path, s32 mode)
{
    s32 closed_stamp;
    s32* trace_path;
    s32 raw_depth;
    s32 last_queue;
    s32 trace_result;
    FieldCollisionTraceRequest* request;
    s32 path[4][0x400];             /* sp+0x0010 */
    s32 flags[4];                   /* sp+0x4010 */
    FieldCollisionTraceRequest rec; /* sp+0x4020 */
    s32 sp4050;
    s32 sp4054;
    s32 sp4058;
    s32 sp405C;
    s32 sp4060;
    s32 sp4064;
    s32 sp4068;
    s32 sp406C;
    s32 sp4070;
    s32 sp4074;
    s32 sp4078;
    u8 sp407C;
    u32 quarter_x;
    u32 restored_x_offset;
    s32* sp4084;
    u32* sp4094;
    FieldScene* scene;
    FieldCollisionQuery* temp_s2;
    u8 var_open;
    s32 hx1;
    s32 temp_v1;
    s32 hz1;
    s32 temp_v1_2;
    s32 hx2;
    s32 temp_v1_3;
    s32 hz2;
    s32 temp_a0;
    s32 temp_v1_4;
    u32 columns;
    s32 temp_v1_5;
    s32 temp_v0_2;
    s32 var_a0;
    u8 temp_v0_3;
    s32 var_a1;
    s16 temp_v1_6;
    u8* var_s0;
    s32 temp_v0_4;
    s32 var_a0_2;
    u8 temp_v0_5;
    s16 temp_v1_7;
    u8* temp_s1;
    s32 var_t5;
    u32 var_s7;
    s32 route_value;
    u8 var_a3;
    s32 var_v0;
    u32 var_t0;
    s32* var_fp;
    s32 temp_v1_8;
    s32* var_s4;
    s32* var_s6;
    u32 temp_t3;
    u32 temp_v1_9;
    u32 temp_a2_2;
    s32 var_a1_2;
    u32 temp_a0_3;
    u32 temp_v1_10;
    u32 temp_a0_4;
    u32 temp_v1_11;
    u32 temp_a0_5;
    u32 temp_v1_12;
    u32 temp_a0_6;
    u32 temp_v1_13;
    u8* temp_v0_7;
    u32 temp_a0_7;
    u32 temp_v1_14;
    u32 adj_tile;
    u8 trace_flag;
    s32 var_v1;
    u8* temp_v0_8;
    u32 temp_a0_8;
    u32 temp_v1_18;
    s32 var_v1_2;
    u8* temp_v0_9;
    u32 temp_a0_9;
    u32 temp_v1_22;
    s32 var_v1_3;
    u8* temp_v0_10;
    u32 temp_a0_10;
    u32 temp_v1_26;
    s32 var_v1_4;
    s32 temp_v1_30;
    s32 var_s3_2;
    s32* sm0;
    s32 temp_v1_31;
    u32 temp_a0_12;
    s32 temp_v1_32;
    u32 var_v1_6;
    s32 var_t2;
    s32 var_t3;
    u8 var_t0_2;
    u32 temp_a0_13;
    u32 temp_v1_33;
    u32 temp_v1_34;
    u8* temp_a2_3;
    u32 temp_v1_35;
    u8* temp_a2_4;
    u32 temp_v1_36;
    u8* var_a2;
    u32 temp_v1_37;
    u8* var_v0_24;
    u32 temp_v1_38;
    u8* var_v0_25;
    u32 temp_v1_39;
    u32 temp_v1_40;
    u32 temp_a0_21;
    u32 var_v1_8;
    s32* temp_v0_12;
    s32* path_end;
    s32 var_s0_2;
    s32* temp_s2_2;
    s32* temp_s1_4;
    s32 temp_s6;
    s32 temp_v0_13;
    s32 temp_s4;
    s32 temp_s2_3;
    s32 temp_s4_2;
    s32 temp_s2_4;
    s32 var_v0_27;
    s32 var_v0_28;
    s32 temp_s4_4;
    s32 temp_s2_5;
    s32 temp_s4_5;
    s32 temp_s2_6;
    s32 var_v0_30;
    s32 var_v0_31;
    s32 final_x;
    FieldCollisionPathPoint* var_a2_2;
    s32 temp_v1_42;
    s32 temp_v1_43;

    s32 next_count;
    s32 count;

    scene = g_field_scene.scene;
    temp_s2 = goal_query;
    if (scene->unk28 == 0)
    {
        if (scene->unk41 != 0)
        {
            return -1;
        }
        goto write_pos;
    }
    sp4058 = 3;
    temp_a2_2 = scene->unk40;
    if (temp_a2_2 == 4)
    {
        sp4058 = 2;
    }
    hx1 = (s32)(start_query->width << 0x10) >> 0x11;
    temp_v1 = temp_s2->x;
    sp4074 = (temp_v1 >= 0 ? temp_v1 >> 8 : (temp_v1 + 0xFF) >> 8) - hx1;
    hz1 = (s32)(start_query->depth << 0x10) >> 0x11;
    temp_v1_2 = temp_s2->z;
    sp4078 = (temp_v1_2 >= 0 ? temp_v1_2 >> 8 : (temp_v1_2 + 0xFF) >> 8) - hz1;
    hx2 = (s32)(start_query->width << 0x10) >> 0x11;
    temp_v1_3 = start_query->x;
    sp406C = (temp_v1_3 >= 0 ? temp_v1_3 >> 8 : (temp_v1_3 + 0xFF) >> 8) - hx2;
    hz2 = (s32)(start_query->depth << 0x10) >> 0x11;
    temp_a0 = start_query->z;
    sp4070 = (temp_a0 >= 0 ? temp_a0 >> 8 : (temp_a0 + 0xFF) >> 8) - hz2;
    temp_v1_4 = sp4058 + 1;
    sp4064 = (sp4074 >> sp4058) + 2;
    sp4068 = (sp4078 >> temp_v1_4) + 2;
    sp405C = (sp406C >> sp4058) + 2;
    sp4060 = (sp4070 >> temp_v1_4) + 2;
    columns = (u16)scene->unk46;
    temp_v1_5 = (u16)scene->unk48;
    if ((sp405C > 0) && (sp4060 > 0) && (sp4064 > 0) && (sp4068 > 0))
    {
        if ((sp405C < (s32)columns - 1) && (sp4060 < temp_v1_5 - 1) && (sp4064 < (s32)columns - 1))
        {
            if (sp4068 >= temp_v1_5 - 1)
            {
                return -2;
            }
            temp_v0_2 = temp_s2->y;
            var_a0 = temp_v0_2 >> 8;
            if (temp_v0_2 < 0)
            {
                var_a0 = (s32)(temp_v0_2 + 0xFF) >> 8;
            }
            temp_v0_3 = scene->unk41;
            count = 0;
            sp4054 = temp_v0_3 - 1;
            if (temp_v0_3 != 0)
            {
                var_a1 = temp_v0_3;
                do
                {
                    temp_v1_6 = scene->unk4A[count];
                    if (var_a0 < temp_v1_6)
                    {
                        if (count != 0)
                        {
                            count -= 1;
                        }
                        sp4054 = count;
                        break;
                    }
                    if (temp_v1_6 == var_a0)
                    {
                        sp4054 = count;
                        break;
                    }
                    count += 1;
                } while (count != var_a1);
            }
            var_s0 = (u8*)(scene->unk2C + ((u16)scene->unk44 * sp4054) + (columns * sp4068) + sp4064);
            *var_s0 = 0xFC;
            temp_v0_4 = start_query->y;
            var_a0_2 = temp_v0_4 >> 8;
            if (temp_v0_4 < 0)
            {
                var_a0_2 = (s32)(temp_v0_4 + 0xFF) >> 8;
            }
            temp_v0_5 = scene->unk41;
            count = 0;
            sp4050 = temp_v0_5 - 1;
            if (temp_v0_5 != 0)
            {
                var_a1 = temp_v0_5;
                do
                {
                    temp_v1_7 = scene->unk4A[count];
                    if (var_a0_2 < temp_v1_7)
                    {
                        if (count != 0)
                        {
                            count -= 1;
                        }
                        sp4050 = count;
                        break;
                    }
                    if (temp_v1_7 == var_a0_2)
                    {
                        sp4050 = count;
                        break;
                    }
                    count += 1;
                } while (count != var_a1);
            }
            temp_s1 = (u8*)(scene->unk2C + ((u16)scene->unk44 * sp4050) + (columns * sp4060) + sp405C);
            if (*temp_s1 != 0xFC)
            {
                *temp_s1 = 0xFB;
                rec.start_x = sp4074;
                rec.start_z = sp4078;
                rec.end_x = sp406C;
                path[0][0] = (s32)temp_s1 | 0x1FE00000;
                rec.tile_base = (s32)var_s0;
                rec.end_z = sp4070;
                rec.footprint_width = (s32)(s16)start_query->width;
                rec.footprint_depth = (s32)(s16)start_query->depth;
                rec.tile_size = (s32)temp_a2_2;
                rec.unk24 = sp4058;
                rec.stamp = 0xFC;
                rec.mode = mode;
                if (func_80062820(&rec) == 0)
                {
                    var_t5 = 0;
                    var_open = 0xFC;
                    var_s7 = flags[0] = 1;
                    route_value = 0;
                    var_a3 = 0xFA;
                    sp4084 = &path[0][0];
                    sp4094 = (u32*)flags;
                    flags[3] = 0;
                    flags[2] = 0;
                    flags[1] = 0;
                    while (1)
                    {
                        count = var_s7;
                        if (count == 0)
                        {
                            if ((flags[0] == 0) && (flags[1] == 0) && (flags[2] == 0))
                            {
                                var_v0 = -3;
                                if (flags[3] == 0)
                                {
                                    return var_v0;
                                }
                            }
                        }
                        var_fp = sp4084 + (var_t5 << 10) + (var_s7 - 1);
                        var_t0 = 0;
                        count -= 1;
                        temp_v1_8 = (var_t5 + 1) & 3;
                        var_s7 = *(u32*)((u32)sp4094 + (temp_v1_8 << 2));
                        var_s4 = sp4084 + (((var_t5 + 3) & 3) << 10);
                        var_s6 = sp4084 + (temp_v1_8 << 10) + var_s7;
                        if (count != -1)
                        {
                            temp_t3 = var_a3 & 0xFF;
                            do
                            {
                                if ((var_s7 >= 0x3F9U) || (var_t0 >= 0x3F9U))
                                {
                                    return -4;
                                }
                                do
                                {
                                    temp_v1_9 = *var_fp--;
                                } while (0);
                                temp_s1 = (u8*)(temp_v1_9 & 0x801FFFFF);
                                temp_a2_2 = temp_v1_9 >> 0x15;
                                var_a1_2 = 0;
                                if ((temp_a2_2 & 0x300) == 0x300)
                                {
                                    next_count = var_t0 + 1;
                                    var_t0 = next_count + count;
                                    temp_v1_9 = 0xBFFFFFFF;
                                    do
                                    {
                                        *var_s4 = var_fp[1] & temp_v1_9;
                                        var_fp -= 1;
                                        count -= 1;
                                        var_s4 += 1;
                                    } while (count != -1);
                                    break;
                                }
                                if (temp_a2_2 & 2)
                                {
                                    var_s0 = temp_s1 - columns;
                                    temp_a0_3 = *var_s0;
                                    temp_v1_10 = temp_a0_3 & 0xFF;
                                    if ((temp_v1_10 < temp_t3) && ((temp_v1_10 < 4U) || !(temp_a2_2 & 0x100)))
                                    {
                                        if (temp_v1_10 != 1)
                                        {
                                            *var_s6 = (s32)var_s0 | 0x03E00000;
                                            var_s6 += 1;
                                            var_s7 += 1;
                                            var_a1_2 = 2;
                                            *var_s0 = var_a3;
                                        }
                                        else
                                        {
                                            *var_s4 = (s32)var_s0 | 0x63E00000;
                                            var_s4 += 1;
                                            var_t0 += 1;
                                            var_a1_2 = 0x202;
                                            *var_s0 = 0xFD;
                                        }
                                    }
                                    else if (temp_a0_3 == var_open)
                                    {
                                        route_value = (s32)var_s0;
                                        if (temp_a2_2 & 0x100)
                                        {
                                            var_a1_2 = 2;
                                        }
                                    }
                                }
                                if (temp_a2_2 & 0x40)
                                {
                                    var_s0 = temp_s1 + columns;
                                    temp_a0_4 = *var_s0;
                                    temp_v1_11 = temp_a0_4 & 0xFF;
                                    if ((temp_v1_11 < temp_t3) && ((temp_v1_11 < 4U) || !(temp_a2_2 & 0x100)))
                                    {
                                        if (temp_v1_11 != 1)
                                        {
                                            *var_s6 = (s32)var_s0 | 0x1F000000;
                                            var_s6 += 1;
                                            var_s7 += 1;
                                            var_a1_2 |= 0x40;
                                            *var_s0 = var_a3;
                                        }
                                        else
                                        {
                                            *var_s4 = (s32)var_s0 | 0x7F000000;
                                            var_s4 += 1;
                                            var_t0 += 1;
                                            var_a1_2 |= 0x4040;
                                            *var_s0 = 0xFD;
                                        }
                                    }
                                    else if (temp_a0_4 == var_open)
                                    {
                                        route_value = (s32)var_s0;
                                        if (temp_a2_2 & 0x100)
                                        {
                                            var_a1_2 |= 0x40;
                                        }
                                    }
                                }
                                if (temp_a2_2 & 8)
                                {
                                    temp_a0_5 = temp_s1[-1];
                                    temp_v1_12 = temp_a0_5 & 0xFF;
                                    var_s0 = temp_s1 - 1;
                                    if ((temp_v1_12 < temp_t3) && ((temp_v1_12 < 4U) || !(temp_a2_2 & 0x100)))
                                    {
                                        if (temp_v1_12 != 1)
                                        {
                                            *var_s6 = (s32)var_s0 | 0x0D600000;
                                            var_s6 += 1;
                                            var_s7 += 1;
                                            var_a1_2 |= 8;
                                            temp_s1[-1] = var_a3;
                                        }
                                        else
                                        {
                                            *var_s4 = (s32)var_s0 | 0x6D600000;
                                            var_s4 += 1;
                                            var_t0 += 1;
                                            var_a1_2 |= 0x808;
                                            temp_s1[-1] = 0xFD;
                                        }
                                    }
                                    else if (temp_a0_5 == var_open)
                                    {
                                        route_value = (s32)var_s0;
                                        if (temp_a2_2 & 0x100)
                                        {
                                            var_a1_2 |= 8;
                                        }
                                    }
                                }
                                if (temp_a2_2 & 0x10)
                                {
                                    temp_a0_6 = temp_s1[1];
                                    temp_v1_13 = temp_a0_6 & 0xFF;
                                    var_s0 = temp_s1 + 1;
                                    if ((temp_v1_13 < temp_t3) && ((temp_v1_13 < 4U) || !(temp_a2_2 & 0x100)))
                                    {
                                        if (temp_v1_13 != 1)
                                        {
                                            *var_s6 = (s32)var_s0 | 0x1AC00000;
                                            var_s6 += 1;
                                            var_s7 += 1;
                                            var_a1_2 |= 0x10;
                                            temp_s1[1] = var_a3;
                                        }
                                        else
                                        {
                                            *var_s4 = (s32)var_s0 | 0x7AC00000;
                                            var_s4 += 1;
                                            var_t0 += 1;
                                            var_a1_2 |= 0x1010;
                                            temp_s1[1] = 0xFD;
                                        }
                                    }
                                    else if (temp_a0_6 == var_open)
                                    {
                                        route_value = (s32)var_s0;
                                        if (temp_a2_2 & 0x100)
                                        {
                                            var_a1_2 |= 0x10;
                                        }
                                    }
                                }

                                if (temp_a2_2 & 1)
                                {
                                    temp_v0_7 = temp_s1 - columns;
                                    temp_a0_7 = temp_v0_7[-1];
                                    var_s0 = temp_v0_7 - 1;
                                    temp_v1_14 = temp_a0_7 & 0xFF;
                                    if ((temp_v1_14 < temp_t3) && ((temp_v1_14 < 4U) || !(temp_a2_2 & 0x100)))
                                    {
                                        adj_tile = var_s0[1];
                                        trace_flag = 0;
                                        if (adj_tile < 0xFEU)
                                        {
                                            if ((adj_tile == 1) || (adj_tile == 0xFD))
                                            {
                                                adj_tile = temp_s1[-1];
                                                if ((adj_tile == 1) || (adj_tile == 0xFD))
                                                {
                                                    trace_flag = 1;
                                                }
                                            }
                                            else
                                            {
                                                adj_tile = temp_s1[-1];
                                                if ((adj_tile < 0xFDU) && (adj_tile != 1))
                                                {
                                                    trace_flag = 1;
                                                }
                                            }
                                        }
                                        if ((trace_flag & 0xFF) != 0)
                                        {
                                            var_v1 = (s32)var_s0 | 0x01600000;
                                            if (!(var_a1_2 & 2))
                                            {
                                                var_v1 |= 0xC00000;
                                            }
                                            if ((var_a1_2 & 8) == 0)
                                            {
                                                var_v1 |= 0x05000000;
                                            }
                                            var_a1_2 |= 1;
                                            if (temp_a0_7 != 1)
                                            {
                                                *var_s6 = var_v1;
                                                var_s6 += 1;
                                                var_s7 += 1;
                                                *var_s0 = var_a3;
                                            }
                                            else
                                            {
                                                *var_s4 = var_v1 | 0x60000000;
                                                var_s4 += 1;
                                                var_t0 += 1;
                                                *var_s0 = 0xFD;
                                            }
                                        }
                                    }
                                    else if (temp_a0_7 == var_open)
                                    {
                                        route_value = (s32)var_s0;
                                        if (temp_a2_2 & 0x100)
                                        {
                                            var_a1_2 |= 1;
                                        }
                                    }
                                }

                                if (temp_a2_2 & 4)
                                {
                                    temp_v0_8 = temp_s1 - columns;
                                    temp_a0_8 = temp_v0_8[1];
                                    var_s0 = temp_v0_8 + 1;
                                    temp_v1_18 = temp_a0_8 & 0xFF;
                                    if ((temp_v1_18 < temp_t3) && ((temp_v1_18 < 4U) || !(temp_a2_2 & 0x100)))
                                    {
                                        adj_tile = var_s0[-1];

                                        trace_flag = 0;
                                        if (adj_tile < 0xFEU)
                                        {
                                            if ((adj_tile == 1) || (adj_tile == 0xFD))
                                            {
                                                adj_tile = temp_s1[1];
                                                if ((adj_tile == 1) || (adj_tile == 0xFD))
                                                {
                                                    trace_flag = 1;
                                                }
                                            }
                                            else
                                            {
                                                adj_tile = temp_s1[1];
                                                if ((adj_tile < 0xFDU) && (adj_tile != 1))
                                                {
                                                    trace_flag = 1;
                                                }
                                            }
                                        }
                                        if ((trace_flag & 0xFF) != 0)
                                        {
                                            var_v1_2 = (s32)var_s0 | 0x02C00000;
                                            if (!(var_a1_2 & 2))
                                            {
                                                var_v1_2 |= 0x600000;
                                            }
                                            if ((var_a1_2 & 0x10) == 0)
                                            {
                                                var_v1_2 |= 0x12000000;
                                            }
                                            var_a1_2 |= 4;
                                            if (temp_a0_8 != 1)
                                            {
                                                *var_s6 = var_v1_2;
                                                var_s6 += 1;
                                                var_s7 += 1;
                                                *var_s0 = var_a3;
                                            }
                                            else
                                            {
                                                *var_s4 = var_v1_2 | 0x60000000;
                                                var_s4 += 1;
                                                var_t0 += 1;
                                                *var_s0 = 0xFD;
                                            }
                                        }
                                    }
                                    else if (temp_a0_8 == var_open)
                                    {
                                        route_value = (s32)var_s0;
                                        if (temp_a2_2 & 0x100)
                                        {
                                            var_a1_2 |= 4;
                                        }
                                    }
                                }

                                if (temp_a2_2 & 0x20)
                                {
                                    temp_v0_9 = temp_s1 + columns;
                                    temp_a0_9 = temp_v0_9[-1];
                                    var_s0 = temp_v0_9 - 1;
                                    temp_v1_22 = temp_a0_9 & 0xFF;
                                    if ((temp_v1_22 < temp_t3) && ((temp_v1_22 < 4U) || !(temp_a2_2 & 0x100)))
                                    {
                                        adj_tile = var_s0[1];

                                        trace_flag = 0;
                                        if (adj_tile < 0xFEU)
                                        {
                                            if ((adj_tile == 1) || (adj_tile == 0xFD))
                                            {
                                                adj_tile = temp_s1[-1];
                                                if ((adj_tile == 1) || (adj_tile == 0xFD))
                                                {
                                                    trace_flag = 1;
                                                }
                                            }
                                            else
                                            {
                                                adj_tile = temp_s1[-1];
                                                if ((adj_tile < 0xFDU) && (adj_tile != 1))
                                                {
                                                    trace_flag = 1;
                                                }
                                            }
                                        }
                                        if ((trace_flag & 0xFF) != 0)
                                        {
                                            var_v1_3 = (s32)var_s0 | 0x0D000000;
                                            if (!(var_a1_2 & 0x40))
                                            {
                                                var_v1_3 |= 0x18000000;
                                            }
                                            if ((var_a1_2 & 8) == 0)
                                            {
                                                var_v1_3 |= 0x01200000;
                                            }
                                            var_a1_2 |= 0x20;
                                            if (temp_a0_9 != 1)
                                            {
                                                *var_s6 = var_v1_3;
                                                var_s6 += 1;
                                                var_s7 += 1;
                                                *var_s0 = var_a3;
                                            }
                                            else
                                            {
                                                *var_s4 = var_v1_3 | 0x60000000;
                                                var_s4 += 1;
                                                var_t0 += 1;
                                                *var_s0 = 0xFD;
                                            }
                                        }
                                    }
                                    else if (temp_a0_9 == var_open)
                                    {
                                        route_value = (s32)var_s0;
                                        if (temp_a2_2 & 0x100)
                                        {
                                            var_a1_2 |= 0x20;
                                        }
                                    }
                                }

                                if (temp_a2_2 & 0x80)
                                {
                                    temp_v0_10 = temp_s1 + columns;
                                    temp_a0_10 = temp_v0_10[1];
                                    var_s0 = temp_v0_10 + 1;
                                    temp_v1_26 = temp_a0_10 & 0xFF;
                                    if ((temp_v1_26 < temp_t3) && ((temp_v1_26 < 4U) || !(temp_a2_2 & 0x100)))
                                    {
                                        adj_tile = var_s0[-1];
                                        trace_flag = 0;
                                        if (adj_tile < 0xFEU)
                                        {
                                            if ((adj_tile == 1) || (adj_tile == 0xFD))
                                            {
                                                adj_tile = temp_s1[1];
                                                if ((adj_tile == 1) || (adj_tile == 0xFD))
                                                {
                                                    trace_flag = 1;
                                                }
                                            }
                                            else
                                            {
                                                adj_tile = temp_s1[1];
                                                if ((adj_tile < 0xFDU) && (adj_tile != 1))
                                                {
                                                    trace_flag = 1;
                                                }
                                            }
                                        }
                                        if ((trace_flag & 0xFF) != 0)
                                        {
                                            var_v1_4 = (s32)var_s0 | 0x1A000000;
                                            if (!(var_a1_2 & 0x40))
                                            {
                                                var_v1_4 |= 0x0C000000;
                                            }
                                            if ((var_a1_2 & 0x10) == 0)
                                            {
                                                var_v1_4 |= 0x02800000;
                                            }
                                            var_a1_2 |= 0x80;
                                            if (temp_a0_10 != 1)
                                            {
                                                *var_s6 = var_v1_4;
                                                var_s6 += 1;
                                                var_s7 += 1;
                                                *var_s0 = var_a3;
                                            }
                                            else
                                            {
                                                *var_s4 = var_v1_4 | 0x60000000;
                                                var_s4 += 1;
                                                var_t0 += 1;
                                                *var_s0 = 0xFD;
                                            }
                                        }
                                    }
                                    else if (temp_a0_10 == var_open)
                                    {
                                        route_value = (s32)var_s0;
                                        if (temp_a2_2 & 0x100)
                                        {
                                            var_a1_2 |= 0x80;
                                        }
                                    }
                                }
                                if ((temp_a2_2 & 0x100) && (var_a1_2 != 0))
                                {
                                    temp_s1[0] = (s8)(var_a3 + 1);
                                }
                                count -= 1;
                            } while (count != -1);
                        }
                        var_a3 -= 1;
                        if ((var_a3 < 4U) && (((u8*)route_value) == NULL))
                        {
                            return -3;
                        }
                        *(u32*)((u32)sp4094 + (var_t5 << 2)) = 0;
                        var_v0 = (var_t5 + 1) & 3;
                        temp_v1_30 = var_t5 + 3;
                        var_t5 = var_v0;
                        *(u32*)((u32)sp4094 + (var_v0 << 2)) = var_s7;
                        *(u32*)((u32)sp4094 + ((temp_v1_30 & 3) << 2)) = var_t0;
                        if (((u8*)route_value) != NULL)
                        {
                            break;
                        }
                    }

                    var_s3_2 = 2;
                    do
                    {
                        var_fp = &path[var_t5][var_s7 - 1];
                        count = var_s7 - 1;
                        if (count != -1)
                        {
                            last_queue = -1;
                            do
                            {
                                temp_v1_31 = *var_fp;
                                var_fp -= 1;
                                if (temp_v1_31 & 0x20000000)
                                {
                                    temp_s1 = (u8*)(temp_v1_31 & 0x801FFFFF);
                                    *temp_s1 = 0xFD;
                                }
                                count -= 1;
                            } while (count != last_queue);
                        }
                        var_t5 = (var_t5 + 1) & 3;
                        var_s7 = flags[var_t5];
                        var_s3_2 -= 1;
                    } while (var_s3_2 != (-1));
                    trace_path = &path[0][0];
                    temp_a0_12 = (u16)scene->unk44;
                    temp_v1_32 = scene->unk2C;
                    temp_s1 = (u8*)(temp_v1_32 + (temp_a0_12 * sp4054) + (columns * sp4068) + sp4064);
                    var_fp = trace_path;
                    if (temp_s1 != ((u8*)route_value))
                    {
                        var_v1_6 = (u32)((u8*)route_value) - (u32)temp_v1_32;
                        while (var_v1_6 >= temp_a0_12)
                        {
                            var_v1_6 -= temp_a0_12;
                        }
                        rec.start_x = sp4074;
                        rec.tile_base = (s32)temp_s1;
                        rec.stamp = 4;
                        rec.start_z = sp4078;
                        rec.end_x = ((var_v1_6 % columns) - 2) << sp4058;
                        rec.end_z = ((var_v1_6 / columns) - 2) << (sp4058 + 1);
                        request = &rec;
                        trace_result = func_80062820(request);
                        if (trace_result != 0)
                        {
                            var_s7 = 1;
                            *var_fp = (s32)temp_s1;
                            var_fp += 1;
                            *temp_s1 = 0xFC;
                            temp_s1 = ((u8*)route_value);
                        }
                        else
                        {
                            rec.goal_tile = (s32)((u8*)route_value);
                            rec.stamp = 4;
                            rec.mode = 2;
                            rec.end_x = sp406C;
                            rec.end_z = sp4070;
                            trace_result = func_80062820(request);
                            if (trace_result != 0)
                            {
                                var_s7 = 1;
                                *var_fp = (s32)temp_s1;
                                var_fp += 1;
                                *temp_s1 = 0xFC;
                                temp_s1 = ((u8*)route_value);
                            }
                            else
                            {
                                var_s7 = 0;
                                closed_stamp = 0xFC;
                                *temp_s1 = closed_stamp;
                            }
                            rec.mode = mode;
                        }
                        trace_flag = 1;
                    }
                    else
                    {
                        var_s7 = 0;
                        trace_flag = 0;
                    }
                    var_t2 = 0;
                    var_t3 = 0;
                    var_t0_2 = 0;
                    do
                    {
                        var_a3 = 0;
                        temp_a0_13 = temp_s1[-1];
                        var_a1_2 = 0;
                        if (((u8)(temp_a0_13 - 4) < 0xF8) && (temp_v1_33 = temp_a0_13 & 0xFF, var_t0_2 < temp_v1_33) &&
                            (var_a3 < temp_v1_33))
                        {
                            var_s0 = temp_s1 - 1;
                            var_t2 = 4;
                            var_a3 = temp_a0_13;
                        }
                        else
                        {
                            var_a1_2 |= 8;
                        }
                        temp_a0_13 = temp_s1[1];
                        temp_v1_34 = temp_a0_13 & 0xFF;
                        if (((u8)(temp_a0_13 - 4) < 0xF8) && (var_t0_2 < temp_v1_34) && (var_a3 < temp_v1_34))
                        {
                            var_s0 = temp_s1 + 1;
                            var_t2 = 5;
                            var_a3 = temp_a0_13;
                        }
                        else
                        {
                            var_a1_2 |= 0x10;
                        }
                        temp_a2_3 = temp_s1 - columns;
                        temp_a0_13 = *temp_a2_3;
                        temp_v1_35 = temp_a0_13 & 0xFF;
                        if (((u8)(temp_a0_13 - 4) < 0xF8) && (var_t0_2 < temp_v1_35) && (var_a3 < temp_v1_35))
                        {
                            var_s0 = temp_a2_3;
                            var_t2 = 2;
                            var_a3 = temp_a0_13;
                        }
                        else
                        {
                            var_a1_2 |= 2;
                        }
                        temp_a2_4 = temp_s1 + columns;
                        temp_a0_13 = *temp_a2_4;
                        temp_v1_36 = temp_a0_13 & 0xFF;
                        if (((u8)(temp_a0_13 - 4) < 0xF8) && (var_t0_2 < temp_v1_36) && (var_a3 < temp_v1_36))
                        {
                            var_s0 = temp_a2_4;
                            var_t2 = 7;
                            var_a3 = temp_a0_13;
                        }
                        else
                        {
                            var_a1_2 |= 0x40;
                        }
                        var_a2 = temp_s1 - columns;
                        temp_a0_13 = var_a2[-1];
                        if (!(var_a1_2 & 0xA) || !(var_a3 & 0xFF))
                        {
                            temp_v1_37 = temp_a0_13 & 0xFF;
                            if (temp_v1_37 < 0xFCU)
                            {
                                if ((temp_v1_37 >= 4U) && (var_t0_2 < temp_v1_37) && (var_a3 < temp_v1_37))
                                {
                                    var_s0 = (temp_s1 - columns) - 1;
                                    var_t2 = 1;
                                    var_a3 = temp_a0_13;
                                }
                            }
                        }
                        temp_a0_13 = (temp_s1 - columns)[1];
                        var_v0_24 = temp_s1 + columns;
                        if (!(var_a1_2 & 0x12) || !(var_a3 & 0xFF))
                        {
                            temp_v1_38 = temp_a0_13 & 0xFF;
                            if (temp_v1_38 < 0xFCU)
                            {
                                if ((temp_v1_38 >= 4U) && (var_t0_2 < temp_v1_38) && (var_a3 < temp_v1_38))
                                {
                                    var_s0 = (temp_s1 - columns) + 1;
                                    var_t2 = 3;
                                    var_a3 = temp_a0_13;
                                }
                            }
                        }
                        temp_a0_13 = (temp_s1 + columns)[-1];
                        var_v0_25 = temp_s1 + columns;
                        if (!(var_a1_2 & 0x48) || !(var_a3 & 0xFF))
                        {
                            temp_v1_39 = temp_a0_13 & 0xFF;
                            if (temp_v1_39 < 0xFCU)
                            {
                                if ((temp_v1_39 >= 4U) && (var_t0_2 < temp_v1_39) && (var_a3 < temp_v1_39))
                                {
                                    var_s0 = (temp_s1 + columns) - 1;
                                    var_t2 = 6;
                                    var_a3 = temp_a0_13;
                                }
                            }
                        }
                        temp_a0_13 = (temp_s1 + columns)[1];
                        if (!(var_a1_2 & 0x50) || !(var_a3 & 0xFF))
                        {
                            temp_v1_40 = temp_a0_13 & 0xFF;
                            if (temp_v1_40 < 0xFCU)
                            {
                                if ((temp_v1_40 >= 4U) && (var_t0_2 < temp_v1_40) && (var_a3 < temp_v1_40))
                                {
                                    var_s0 = (temp_s1 + columns) + 1;
                                    var_t2 = 8;
                                    var_a3 = temp_a0_13;
                                }
                            }
                            var_v0 = -5;
                            if ((var_a3 & 0xFF) == 0)
                            {
                                return var_v0;
                            }
                        }
                        if (var_t2 != var_t3)
                        {
                            var_t3 = var_t2;
                            *var_fp = (s32)temp_s1;
                            var_s7 += 1;
                            var_fp += 1;
                            if (var_s7 >= 0x400U)
                            {
                                return -4;
                            }
                            var_t0_2 = var_a3;
                        }
                        else
                        {
                            var_t0_2 = var_a3;
                        }
                        temp_s1 = var_s0;
                    } while ((var_t0_2 & 0xFF) != 0xFB);
                    count = var_s7;
                    var_s6 = &path[1][1];
                    var_s4 = &path[2][1];
                    var_s3_2 = count - 2;
                    temp_s1 = (u8*)(scene->unk2C + ((u16)scene->unk44 * sp4050) + (columns * sp4060) + sp405C);
                    *var_fp = (s32)temp_s1;
                    var_fp = &path[0][0];
                    rec.stamp = 0;
                    path[1][0] = sp4074;
                    path[2][0] = sp4078;
                    if (var_s3_2 != -1)
                    {
                        do
                        {
                            var_fp += 1;
                            temp_a0_21 = (u16)scene->unk44;
                            temp_s1 = (u8*)*var_fp;
                            var_v1_8 = (s32)temp_s1 - scene->unk2C;
                            while (var_v1_8 >= temp_a0_21)
                            {
                                var_v1_8 -= temp_a0_21;
                            }
                            var_s3_2 -= 1;
                            *var_s6 = ((var_v1_8 % columns) - 2) << sp4058;
                            var_s6 += 1;
                            *var_s4 = ((var_v1_8 / columns) - 2) << (sp4058 + 1);
                            var_s4 += 1;
                        } while (var_s3_2 != -1);
                        var_fp = &path[0][0];
                    }
                    var_s7 = 0;
                    count -= 1;
                    *var_s6 = sp406C;
                    var_s6 = &path[1][0];
                    *var_s4 = sp4070;
                    var_s4 = &path[2][0];
                    if (count != -1)
                    {
                        do
                        {
                            temp_s1 = (u8*)*var_fp;
                            var_fp += 1;
                            sp406C = *var_s6;
                            var_s6 += 1;
                            temp_v0_12 = &path[0][var_s7++];
                            sp4070 = *var_s4;
                            var_s4 += 1;
                            temp_v0_12[0] = (s32)temp_s1;
                            temp_v0_12[0x400] = sp406C;
                            temp_v0_12[0x800] = sp4070;
                            if (count != 0)
                            {
                                var_s3_2 = count;
                                rec.tile_base = (s32)temp_s1;
                                rec.start_x = sp406C;
                                rec.start_z = sp4070;
                                do
                                {
                                    var_s0_2 = var_s3_2 * 4;
                                    temp_s2_2 = (s32*)(var_s0_2 + (s32)var_s6);
                                    temp_s1_4 = (s32*)(var_s0_2 + (s32)var_s4);
                                    rec.end_x = *temp_s2_2;
                                    rec.end_z = *temp_s1_4;
                                    if (func_80062820(&rec) != 0)
                                    {
                                        var_fp = (s32*)((s32)var_fp + var_s0_2);
                                        var_s6 = temp_s2_2;
                                        var_s4 = temp_s1_4;
                                        count -= var_s3_2;
                                        break;
                                    }
                                    var_s3_2 -= 1;
                                    var_s0_2 = var_s3_2 * 4;
                                } while (var_s3_2 != 0);
                            }
                            count -= 1;
                        } while (count != -1);
                    }
                    if ((trace_flag & 0xFF) && (var_s7 >= 2U))
                    {
                        trace_flag = 0;
                        sp407C = 1;
                        temp_s1 = (u8*)path[0][0];
                        sp405C = path[1][0];
                        sp4060 = path[2][0];
                        path_end = &path[0][var_s7];
                        sm0 = &path[0][0];
                        path_end[0] = *var_fp;
                        path_end[0x400] = *var_s6;
                        var_s6 = &path[1][0];
                        path_end[0x800] = *var_s4;
                        var_s4 = &path[2][0];
                        var_s0 = (u8*)sm0[2];
                        var_fp = (s32*)(var_s6[1]);
                        count = var_s4[1];
                        temp_s6 = var_s6[2];
                        sp4064 = temp_s6;
                        sp4068 = var_s4[2];
                        rec.start_x = sp405C;
                        temp_v0_13 = ((s32)var_fp) + temp_s6;
                        rec.tile_base = (s32)temp_s1;
                        rec.start_z = sp4060;
                        temp_s4 = temp_v0_13 / 2;
                        rec.end_x = temp_s4;
                        temp_s2_3 = (count + sp4068) / 2;
                        rec.end_z = temp_s2_3;
                        if (func_80062820(&rec) != 0)
                        {
                            rec.tile_base = (s32)var_s0;
                            rec.start_x = sp4064;
                            rec.start_z = sp4068;
                            if (func_80062820(&rec) != 0)
                            {
                                trace_flag = 1;
                                var_fp = (s32*)(temp_s4);
                                count = temp_s2_3;
                                rec.start_x = sp405C;
                                sp407C = 0;
                                rec.tile_base = (s32)temp_s1;
                                rec.start_z = sp4060;
                                temp_s4_2 = (((s32)var_fp) + sp4064) / 2;
                                temp_s2_4 = (count + sp4068) / 2;
                                rec.end_x = temp_s4_2;
                                rec.end_z = temp_s2_4;
                                if (func_80062820(&rec) != 0)
                                {
                                    rec.tile_base = (s32)var_s0;
                                    rec.start_x = sp4064;
                                    rec.start_z = sp4068;
                                    if (func_80062820(&rec) != 0)
                                    {
                                        var_fp = (s32*)(temp_s4_2);
                                        count = temp_s2_4;
                                    }
                                }
                            }
                        }
                        if (sp407C != 0)
                        {
                            var_s4 = (s32*)(sp4064 - ((s32)var_fp));
                            var_v0_27 = ((s32)var_s4);
                            rec.tile_base = (s32)temp_s1;
                            rec.start_x = sp405C;
                            rec.start_z = sp4060;
                            if (((s32)var_s4) < 0)
                            {
                                var_v0_27 = ((s32)var_s4) + 3;
                            }
                            quarter_x = ((s32)var_fp) + (var_v0_27 >> 2);
                            rec.end_x = quarter_x;
                            var_s3_2 = sp4068 - count;
                            var_v0_28 = var_s3_2;
                            if (var_s3_2 < 0)
                            {
                                var_v0_28 = var_s3_2 + 3;
                            }
                            var_s6 = (s32*)(count + (var_v0_28 >> 2));
                            rec.end_z = ((s32)var_s6);
                            if (func_80062820(&rec) != 0)
                            {
                                rec.tile_base = (s32)var_s0;
                                rec.start_x = sp4064;
                                rec.start_z = sp4068;
                                if (func_80062820(&rec) != 0)
                                {
                                    trace_flag = 1;
                                    restored_x_offset = quarter_x - (u32)var_s4;
                                    var_fp = (s32*)(restored_x_offset + (u32)var_s4);
                                    count = ((s32)var_s6);
                                }
                            }
                        }
                        sp407C = 1;
                        rec.start_x = sp4064;
                        rec.tile_base = (s32)var_s0;
                        rec.start_z = sp4068;
                        temp_s4_4 = (((s32)var_fp) + sp405C) / 2;
                        temp_s2_5 = (count + sp4060) / 2;
                        rec.end_x = temp_s4_4;
                        rec.end_z = temp_s2_5;
                        if (func_80062820(&rec) != 0)
                        {
                            rec.tile_base = (s32)temp_s1;
                            rec.start_x = sp405C;
                            rec.start_z = sp4060;
                            if (func_80062820(&rec) != 0)
                            {
                                trace_flag = 1;
                                var_fp = (s32*)(temp_s4_4);
                                count = temp_s2_5;
                                rec.start_x = sp4064;
                                sp407C = 0;
                                rec.tile_base = (s32)var_s0;
                                rec.start_z = sp4068;
                                temp_s4_5 = (((s32)var_fp) + sp405C) / 2;
                                temp_s2_6 = (count + sp4060) / 2;
                                rec.end_x = temp_s4_5;
                                rec.end_z = temp_s2_6;
                                if (func_80062820(&rec) != 0)
                                {
                                    rec.tile_base = (s32)temp_s1;
                                    rec.start_x = sp405C;
                                    rec.start_z = sp4060;
                                    if (func_80062820(&rec) != 0)
                                    {
                                        var_fp = (s32*)(temp_s4_5);
                                        count = temp_s2_6;
                                    }
                                }
                            }
                        }
                        if (sp407C != 0)
                        {
                            var_s3_2 = sp405C - ((s32)var_fp);
                            var_v0_30 = var_s3_2;
                            rec.tile_base = (s32)var_s0;
                            rec.start_x = sp4064;
                            rec.start_z = sp4068;
                            if (var_s3_2 < 0)
                            {
                                var_v0_30 = var_s3_2 + 3;
                            }
                            var_s6 = (s32*)(((s32)var_fp) + (var_v0_30 >> 2));
                            rec.end_x = ((s32)var_s6);
                            route_value = sp4060 - count;
                            var_v0_31 = route_value;
                            if (route_value < 0)
                            {
                                var_v0_31 = route_value + 3;
                            }
                            var_s4 = (s32*)(count + (var_v0_31 >> 2));
                            rec.end_z = ((s32)var_s4);
                            if (func_80062820(&rec) != 0)
                            {
                                rec.tile_base = (s32)temp_s1;
                                final_x = sp405C;
                                rec.start_x = final_x;
                                rec.start_z = sp4060;
                                if (func_80062820(&rec) != 0)
                                {
                                    trace_flag = 1;
                                    var_fp = (s32*)(((s32)var_s6));
                                    count = ((s32)var_s4);
                                }
                            }
                        }
                        if ((trace_flag & 0xFF) != 0)
                        {
                            var_s4 = (s32*)(1);
                            path[1][1] = ((s32)var_fp);
                            path[2][((s32)var_s4)] = count;
                        }
                    }
                    count = var_s7;
                    if (count >= 0x11U)
                    {
                        count = 0x10;
                    }
                    var_s6 = &path[1][var_s7 - 1];
                    var_s4 = &path[2][var_s7 - 1];
                    count = count - 1;
                    var_s3_2 = 0;
                    if (count != -1)
                    {
                        last_queue = -1;
                        var_a2_2 = output_path;
                        do
                        {
                            temp_v1_42 = *var_s6;
                            var_s6 -= 1;
                            var_s3_2 += 1;
                            count -= 1;
                            var_a2_2->x = (temp_v1_42 + ((s32)(start_query->width << 0x10) >> 0x11)) << 8;
                            route_value = 8;
                            raw_depth = start_query->depth;
                            temp_v1_43 = *var_s4;
                            var_s4 -= 1;
                            var_a2_2->z = (temp_v1_43 + ((s32)(raw_depth << 0x10) >> 0x11)) << route_value;
                            var_a2_2 += 1;
                        } while (count != last_queue);
                    }
                    return var_s3_2;
                }
                goto write_pos;
            }
            goto write_pos;
        }
    }
    return -2;
write_pos:
    output_path->x = temp_s2->x;
    output_path->z = temp_s2->z;
    return 1;
}

/**
 * @brief Walk a rectangular tile footprint along a straight line and stamp it
 *        into the group tile map.
 *
 * Bresenham DDA over the field group tile grid. The footprint uses the request
 * width and depth, starts at (start_x, start_z), and is stepped one tile at a
 * time towards (end_x, end_z). edge_state records which edge of the
 * footprint moved on the last step so only the newly covered tiles are
 * re-tested: 1 = a new column, 2 = a new row, 3 = the whole rectangle (the
 * first iteration and both edges crossing at once). Every visited tile must be
 * below 0xFD and must not be 1; anything else aborts the walk.
 *
 * Tiles that pass the test are stamped with request->stamp, and the stamp value
 * is bumped after each step unless it is 0 or 0xFC. The store happens before
 * the mode check, so a tile is written even when mode is not 2 - only the goal
 * comparison is gated on it.
 *
 * @param request Walk request. tile_base is the starting tile, goal_tile is
 *                the optional goal, tile_size controls the cell masks, and
 *                unk24 is unused here.
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
    s32 step_y;
    s32 tile_size;
    s32 mask_x;
    s32 mask_y;
    s32 x_cell;
    s32 y_cell;
    s32 x_end;
    s32 y_end;
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
    y_cell = request->start_z;
    delta_z = request->end_z - y_cell;
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
    mask_y = (tile_size * 2) - 1;
    y_cell &= mask_y;
    y_end = y_cell + depth_minus_one;
    row_hi = y_end & mask_y;
    scene = g_field_scene.scene;
    stride = (u16)scene->unk46;
    row_span = 0;
    if (y_end >= ((footprint_depth + mask_y) & ~mask_y))
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
        step_y = stride;
    }
    else
    {
        delta_z = -delta_z;
        step_y = -stride;
    }

    edge_state = 3;
    if (delta_x >= delta_z)
    {
        error_term = -delta_x;
        for (i = delta_x; i != -1; i--)
        {
            if (edge_state != 0)
            {
                switch (edge_state)
                {
                case 1:
                    tile_ptr = tile_base;
                    if (step_x > 0)
                    {
                        tile_ptr = tile_base + x_span;
                    }
                    n = row_span;
                    for (;;)
                    {
                        tile_value = *tile_ptr;
                        if (tile_value >= 0xFD || tile_value == 1)
                        {
                            return 0;
                        }
                        if (stamp != 0 && tile_value != 0xFB)
                        {
                            *tile_ptr = stamp;
                            if (mode == 2 && tile_ptr == goal_tile)
                            {
                                return 1;
                            }
                        }
                        if (n == 0)
                        {
                            break;
                        }
                        tile_ptr += stride;
                        n -= stride;
                    }
                    break;
                case 2:
                    tile_ptr = tile_base;
                    if (step_y > 0)
                    {
                        tile_ptr = tile_base + row_span;
                    }
                    n = x_span;
                    do
                    {
                        tile_value = *tile_ptr;
                        if (tile_value >= 0xFD || tile_value == 1)
                        {
                            return 0;
                        }
                        if (stamp != 0 && tile_value != 0xFB)
                        {
                            *tile_ptr = stamp;
                            if (mode == 2 && tile_ptr == goal_tile)
                            {
                                return 1;
                            }
                        }
                        tile_ptr++;
                    } while (--n != -1);
                    break;
                case 3:
                    tile_ptr = tile_base;
                    n = row_span;
                    for (;;)
                    {
                        scan_ptr = tile_ptr;
                        m = x_span;
                        do
                        {
                            tile_value = *scan_ptr;
                            if (tile_value >= 0xFD || tile_value == 1)
                            {
                                return 0;
                            }
                            if (stamp != 0 && tile_value != 0xFB)
                            {
                                *scan_ptr = stamp;
                                if (mode == 2 && scan_ptr == goal_tile)
                                {
                                    return 1;
                                }
                            }
                            scan_ptr++;
                        } while (--m != -1);
                        if (n == 0)
                        {
                            break;
                        }
                        tile_ptr += stride;
                        n -= stride;
                    }
                    break;
                }
                if (stamp != 0 && stamp != 0xFC)
                {
                    stamp++;
                }
            }
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
                if (step_y > 0)
                {
                    if (y_cell == mask_y)
                    {
                        y_cell = 0;
                        row_span -= stride;
                        tile_base += stride;
                    }
                    else
                    {
                        y_cell++;
                    }
                    if (row_hi == mask_y)
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
                    if (y_cell == 0)
                    {
                        y_cell = mask_y;
                        row_span += stride;
                        tile_base -= stride;
                        edge_state |= 2;
                    }
                    else
                    {
                        y_cell--;
                    }
                    if (row_hi == 0)
                    {
                        row_hi = mask_y;
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
            if (edge_state != 0)
            {
                switch (edge_state)
                {
                case 1:
                    tile_ptr = tile_base;
                    if (step_x > 0)
                    {
                        tile_ptr = tile_base + x_span;
                    }
                    n = row_span;
                    for (;;)
                    {
                        tile_value = *tile_ptr;
                        if (tile_value >= 0xFD || tile_value == 1)
                        {
                            return 0;
                        }
                        if (stamp != 0 && tile_value != 0xFB)
                        {
                            *tile_ptr = stamp;
                            if (mode == 2 && tile_ptr == goal_tile)
                            {
                                return 1;
                            }
                        }
                        if (n == 0)
                        {
                            break;
                        }
                        tile_ptr += stride;
                        n -= stride;
                    }
                    break;
                case 2:
                    tile_ptr = tile_base;
                    if (step_y > 0)
                    {
                        tile_ptr = tile_base + row_span;
                    }
                    n = x_span;
                    do
                    {
                        tile_value = *tile_ptr;
                        if (tile_value >= 0xFD || tile_value == 1)
                        {
                            return 0;
                        }
                        if (stamp != 0 && tile_value != 0xFB)
                        {
                            *tile_ptr = stamp;
                            if (mode == 2 && tile_ptr == goal_tile)
                            {
                                return 1;
                            }
                        }
                        tile_ptr++;
                    } while (--n != -1);
                    break;
                case 3:
                    tile_ptr = tile_base;
                    n = row_span;
                    for (;;)
                    {
                        scan_ptr = tile_ptr;
                        m = x_span;
                        do
                        {
                            tile_value = *scan_ptr;
                            if (tile_value >= 0xFD || tile_value == 1)
                            {
                                return 0;
                            }
                            if (stamp != 0 && tile_value != 0xFB)
                            {
                                *scan_ptr = stamp;
                                if (mode == 2 && scan_ptr == goal_tile)
                                {
                                    return 1;
                                }
                            }
                            scan_ptr++;
                        } while (--m != -1);
                        if (n == 0)
                        {
                            break;
                        }
                        tile_ptr += stride;
                        n -= stride;
                    }
                    break;
                }
                if (stamp != 0 && stamp != 0xFC)
                {
                    stamp++;
                }
            }
            if (i == 0)
            {
                return 1;
            }
            edge_state = 0;
            if (step_y > 0)
            {
                if (y_cell == mask_y)
                {
                    y_cell = 0;
                    row_span -= stride;
                    tile_base += stride;
                }
                else
                {
                    y_cell++;
                }
                if (row_hi == mask_y)
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
                if (y_cell == 0)
                {
                    y_cell = mask_y;
                    row_span += stride;
                    tile_base -= stride;
                    edge_state = 2;
                }
                else
                {
                    y_cell--;
                }
                if (row_hi == 0)
                {
                    row_hi = mask_y;
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
 * The node's C->B edge runs between the boundary points named by unkA and
 * unkC in g_field_node_angle_table and rises by unk12 - unk10 over that span.
 * Moving along the edge covers its full 3D length while only advancing by the
 * horizontal run, so @p movement is multiplied by run / slope to hold the ground
 * speed constant. The run is first shortened by a further 1/16, a flat penalty
 * for travelling on a slope at all.
 *
 * @param surface Collision surface definition supplying the edge and its heights.
 * @param movement Movement vector rescaled in place; movement[0] is x and movement[1] is z.
 *
 * @note dx and dz are each reused to hold their own square once the raw delta
 *       is no longer needed.
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
        func_8005E3B0((FieldCollisionNode*)node, (u8**)&allocator_cursor);
        node = node->next;
    }
    func_8005F158(&allocator_cursor);
    D_801ED000 = allocator_cursor;
}
