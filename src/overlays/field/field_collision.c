#include "field_scene_internal.h"

/**
 * @brief Probe position and footprint passed to func_8005B368.
 *
 * Carries the world-space probe position (x, y, z) plus the footprint
 * extents (unkC = width along x, unk10 = depth along z) and a height
 * tolerance (unkE).
 */
typedef struct
{
    s32 x;
    s32 y;
    s32 z;
    u16 unkC;
    s16 unkE;
    u16 unk10;
} Query;

/**
 * @brief Collision object referenced by a CollNode.
 * @note unk10/unk12 are the vertical band the object occupies; unk14 is the
 *       value returned to the caller on a successful hit.
 */
typedef struct
{
    u8 pad[0x10];
    s16 unk10;
    s16 unk12;
    s16 unk14;
} Object;

/**
 * @brief One entry in the field scene's collision-node list (scene+0x10).
 *
 * @note right/left/bottom/top form the node's axis-aligned bounding box.
 *       Each of the two (denom, mult, max, min) groups describes a sloped
 *       edge: when denom != 0 the edge is diagonal and the probe is tested
 *       against mult*coord/denom; when denom == 0 the edge is axis-aligned
 *       and only max/min are used.
 */
typedef struct CollNode
{
    struct CollNode *next;
    Object *obj;
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
} CollNode;

/**
 * @brief Hit-test a probe against the field scene's collision-node list.
 *
 * Converts the query position from world units to grid cells (signed divide
 * by 256, i.e. a >> 8 with a round-toward-zero bias for negatives) and
 * centres the footprint by subtracting half the extent. Each node is
 * rejected by its bounding box first; survivors are tested against the
 * node's two sloped/axis-aligned edges. The first node that passes returns
 * its object's unk14.
 *
 * @param q Probe query (position, footprint extents, height tolerance).
 * @return obj->unk14 of the first node hit, or -1 if nothing is hit.
 *
 * @note Matches 98.85% under gcc280_g4 (no expand-div) and is functionally
 *       faithful to the target. The residual diff is register-allocation
 *       noise in three tightly-coupled clusters (the start-x/start-z setup
 *       seeded by the uninitialised reads, the denom1 division ordering, and
 *       the denom2 ty/max2 coloring); every source rearrangement tried so
 *       far - including decomp-permuter - regresses it.
 * @see decomp.me (98.85%) https://decomp.me/scratch/ElbpA
 */
s16 func_8005B368(Query *q)
{
    s32 sx;
    s32 ex;
    unsigned long new_var;
    Query *q_dup;
    s32 sz;
    FieldScene *scene;
    s32 ez;
    s32 sy;
    s32 half_x;
    s32 start_z;
    s32 half_z;
    s32 temp;
    CollNode *node;
    s32 hit;

    /* half_x = (s16)unkC / 2: sign-extend, add the sign bit, then arithmetic
     * shift right by one. The two-step (ez then >>1) and the redundant q_dup
     * copy are kept verbatim because they are required to match. */
    temp = q->unkC;
    half_x = ((s16) temp) + (((u32) (temp << 16)) >> 31);
    q_dup = q;
    scene = g_field_scene.scene;
    ez = half_x;
    half_x = ((s32) ez) >> 1;

    /* sx reads q->x: the explicit "sx = q->x" load is omitted so the compiler
     * keeps q->x in sx's register (an m2c artifact, but required to match).
     * The +0xFF before >>8 biases negative values toward zero (signed /256). */
    if (q->x < 0)
    {
        sx += 0xFF;
    }
    sx = (sx >> 8) - half_x;
    temp = q_dup->unk10;
    ex = sx + ((s16) q->unkC);
    half_z = ((s32) (((s16) temp) + (((u32) (temp << 16)) >> 31))) >> 1;

    /* sz reads q->z (same omitted-load artifact as sx). The "^ 0" is a no-op
     * kept to match. */
    if (q->z >= 0)
    {
        sz = (sz ^ 0) >> 8;
    }
    else
    {
        sz = (sz + 0xFF) >> 8;
    }
    sz -= half_z;
    start_z = sz;
    ez = start_z + ((s16) q->unk10);
    if (q->y >= 0)
    {
        sy = sy >> 8;
    }
    else
    {
        sy = (sy + 0xFF) >> 8;
    }

    /*
     * The debug marker renderer and collision code are two interpretations of
     * the same spatial-node chain at FieldScene+0x10.
     */
    for (node = (CollNode*)scene->markers; node != 0; node = node->next)
    {
        Object *obj;
        s16 val;
        obj = node->obj;
        val = obj->unk10;

        /* Reject by vertical band and bounding box. */
        if ((sy - q->unkE) >= val)
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
            s32 tx;
            s32 ty;
            new_var = (node->mult1 * ex) / node->denom1;
            tx = (node->mult1 * sx) / node->denom1;
            ty = new_var;
            if (((((start_z - tx) >= node->max1) || ((ez - tx) >= node->max1)) || ((start_z - ty) >= node->max1)) || ((ez - ty) >= node->max1))
            {
                if ((((node->min1 >= (start_z - tx)) || (node->min1 >= (ez - tx))) || (node->min1 >= (start_z - ty))) || (node->min1 >= (ez - ty)))
                {
                    hit = 1;
                }
            }
        }
        else
            if (ex >= node->max1)
        {
            if (node->min1 >= (sx ^ 0))
            {
                hit = 1;
            }
        }

        if (hit)
        {
            if (node->denom2 != 0)
            {
                s32 tx;
                s32 ty;
                tx = (node->mult2 * sx) / node->denom2;
                ty = (node->mult2 * ex) / node->denom2;
                /* half_x is reused here as a scratch for ty; required to match. */
                half_x = ty;
                if (((((start_z - tx) >= node->max2) || ((ez - tx) >= node->max2)) || ((start_z - half_x) >= node->max2)) || ((ez - half_x) >= node->max2))
                {
                    if ((((node->min2 < (start_z - tx)) && (node->min2 < (ez - tx))) && (node->min2 < (start_z - half_x))) && (node->min2 < (ez - half_x)))
                    {
                    }
                    else
                    {
                        return obj->unk14;
                    }
                }
            }
            else
                if (ex >= node->max2)
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

extern long ratan2(long y, long x);
extern int rcos(int a);
extern int rsin(int a);

extern long SquareRoot0(long a);

/** Field allocator cursor at 0x801ED000, i.e. FieldMemState::top. */
extern s32 D_801ED000;

typedef struct Move_UnkS16 {
    s16 unk0;
    s16 unk2;
} Move_UnkS16;

/**
 * @brief One run of consecutive boundary points in a node's edge list.
 *
 * The list lives at Move_UnkNode2+0x18 and is terminated by a record whose
 * count is zero. A run's points are consecutive entries of
 * g_field_node_angle_table starting at @c index, so only the first index is
 * stored.
 */
typedef struct Move_EdgeRun {
    /** 0x00 number of points in this run; only the low 15 bits are the count. */
    u16 count;
    /** 0x02 angle-table index of the run's first point. */
    u16 index;
} Move_EdgeRun;

typedef struct Move_UnkNode2 {
    u8 pad0[4];
    s32 unk4;
    u8 pad8[2];
    /** 0x0A index of the node's third boundary point in g_field_node_angle_table. */
    u16 unkA;
    /** 0x0C index of the node's second boundary point. */
    u16 unkC;
    /** 0x0E index of the node's first boundary point. */
    u16 unkE;
    s16 unk10;
    s16 unk12;
    s16 unk14;
    u8 pad16[2];
    /** 0x18 edge-run list; @c runs[0].index doubles as the closing point. */
    Move_EdgeRun runs[1];
} Move_UnkNode2;

s32 func_8005E1A8(Move_UnkNode2*, s32, s32, s32);

typedef struct Move_UnkNode1 {
    struct Move_UnkNode1* unk0;
    Move_UnkNode2* unk4;
    u8 pad8[8];
    void* unk10;
    void* unk14;
    u8 unk18;
    u8 pad19[3];
    s16 unk1C;
    s16 unk1E;
    s16 unk20;
    s16 unk22;
    s32 unk24;
    s32 unk28;
    s32 unk2C;
    s32 unk30;
    s32 unk34;
    s32 unk38;
    s32 unk3C;
    s32 unk40;
} Move_UnkNode1;

s16 func_8005DFAC(Move_UnkNode1*, s32*);

typedef struct Move_UnkNode3 {
    u8 pad0[0x2C];
    s32 unk2C;
    s16 unk30;
    s16 unk32;
} Move_UnkNode3;

/**
 * @brief Actor/mover state resolved by func_8005B6AC.
 * @note Distinct from the smaller `Query` probe struct above (which has u16
 *       unkC/unk10). Prologue asm proves word loads at 0x4/0x10 and an s16 load
 *       at 0x26, so every field here is its asm-confirmed width.
 */
typedef struct Move_Mover {
    s32 unk0;       /* 0x00 world X (fixed-point, >>8 to screen) */
    s32 unk4;       /* 0x04 world Z/height accumulator */
    s32 unk8;       /* 0x08 world Y */
    s32 unkC;       /* 0x0C dX this frame */
    s32 unk10;      /* 0x10 dZ this frame */
    s32 unk14;      /* 0x14 dY this frame */
    s32 unk18;      /* 0x18 output height delta (zeroed in prologue) */
    void* unk1C;    /* 0x1C selected collision node (-1 = needs search) */
    s32 unk20;      /* 0x20 flags (bit0 sticky-contact) */
    u16 unk24;      /* 0x24 footprint width */
    s16 unk26;      /* 0x26 height bias */
    s32 unk28;      /* 0x28 mode flags (0x30000 gate, low s16 = step size) */
} Move_Mover;

void func_80062F48(Move_UnkNode2*, s32*);

typedef struct Move_Probe {
    Move_Mover* m;
    s32 x;
    s32 y;
    s16 w;
    s16 h;
} Move_Probe;

void func_8005DA7C(Move_Probe*, Move_UnkNode1*, s32*, s32*);

/**
 * @see decomp.me (57.91%) https://decomp.me/scratch/N2GNJ
 * @note local objdiff 84.66% (gcc280_g4_noexpanddiv), 2026-08-08 - active
 *       matching scratch is working/func_8005B6AC/code.c; see
 *       working/func_8005B6AC/STATUS.md for the open leads.
 */
s32 func_8005B6AC(Move_Mover* a0) {
    Move_Probe probe;
    s32 sp20;
    s32 sp24;
    FieldScene* sp28;
    void* sp2C;
    u16 sp30;
    u16 sp38;
    u16 sp40;
    s32 sp48;
    s32 sp4C;
    s32 sp50;
    s32 sp54;
    s32 sp58;
    s32 sp5C;
    s32 sp60;
    void** sp64;
    void** sp68;
    s32 sp6C;
    s32 sp70;
    s32 sp74;
    s32 sp78;
    s32 sp7C;
    s32 sp80;
    s32 sp84;
    s32 sp88;
    s32 sp8C;
    u8* spA8;
    s32 spAC;
    s16 temp_a0_12;
    s16 temp_a0_13;
    s32 temp_a0_14;
    s16 temp_a0_20;
    s16 temp_a1_5;
    s16 temp_s0;
    s16 temp_s0_3;
    s16 temp_s0_5;
    s16 temp_s0_6;
    s16 temp_s1;
    s16 temp_s2_3;
    s16 temp_s2_4;
    s32 temp_v0_17;
    s16 temp_v1_11;
    s16 temp_v1_13;
    s16 temp_v1_15;
    s16 temp_v1_18;
    s16 temp_v1_19;
    s32 temp_v1_26;
    s32 temp_v1_4;
    s32 temp_v1_5;
    s16 temp_v1_7;
    s16 temp_v1_8;
    s16 temp_v1_9;
    s32 var_a1;
    s32 k_blocked;
    s32 var_a3_2;
    s16 var_s0_3;
    s16 var_s1_3;
    s32 e_x_min;
    s32 f_x_min;
    s32 f_x_max;
    s32 f_x_max_2;
    s32 f_y_min;
    s32 f_y_max;
    s32 var_s2;
    s16 var_t0_2;
    s16 var_t1_2;
    s16 var_t1_3;
    u8* var_s1;
    u8* var_s1_5;
    u8* var_s1_2;
    u8* var_s1_4;
    u8* var_s2_2;
    u8* var_s2_3;
    u8* var_s2_4;
    u8* var_t3;
    s32 temp_a0;
    s32 temp_a0_10;
    s32 temp_a0_15;
    s32 temp_a0_17;
    s32 temp_a0_19;
    s32 temp_a0_4;
    s32 temp_a0_8;
    s32 temp_a1;
    s32 temp_a1_3;
    s32 temp_a1_4;
    s32 temp_a1_6;
    s32 temp_a1_8;
    s32 temp_a1_9;
    s32 temp_a2_2;
    s32 temp_a3;
    s32 temp_a3_2;
    s32 temp_lo;
    s32 temp_lo_2;
    s32 temp_lo_3;
    s32 temp_lo_4;
    s32 temp_s0_2;
    s32 temp_s0_4;
    s32 temp_s2;
    s32 temp_s2_2;
    s32 temp_s5;
    s32 temp_s7;
    s32 temp_t5;
    s32 temp_t5_2;
    s32 temp_t5_3;
    s32 temp_t6_2;
    s32 temp_v0;
    s32 temp_v0_10;
    s32 temp_v0_12;
    s32 temp_v0_13;
    s32 temp_v0_14;
    s32 temp_v0_15;
    s32 temp_v0_16;
    s32 temp_v0_18;
    s32 temp_v0_2;
    s32 temp_v0_3;
    s32 temp_v0_4;
    s32 temp_v0_5;
    s32 temp_v0_6;
    s32 temp_v0_9;
    s32 temp_v1;
    s32 temp_v1_10;
    s32 temp_v1_12;
    s32 temp_v1_14;
    s32 temp_v1_16;
    s32 temp_v1_17;
    s32 temp_v1_20;
    s32 temp_v1_21;
    s32 temp_v1_22;
    s32 temp_v1_23;
    s32 temp_v1_24;
    s32 temp_v1_25;
    s32 temp_v1_27;
    s32 temp_v1_2;
    s32 temp_v1_3;
    s32 temp_v1_6;
    s32 var_a0;
    s32 var_a2;
    s32 var_a3;
    s32 ux;
    s32 uy;
    s32 var_s0;
    s32 var_s0_10;
    s32 var_s0_11;
    s32 var_s0_2;
    s32 var_s0_4;
    s32 var_s0_5;
    s32 var_s0_6;
    s32 var_s0_7;
    s32 var_s0_8;
    s32 var_s0_9;
    s32 var_s4_3;
    s32 var_s5;
    s32 var_s7;
    s32 var_t0;
    s32 var_t1;
    s32 var_t5;
    s32 var_t7;
    s32 var_t8;
    s32 var_t9;
    s32 var_v0;
    s32 var_v0_10;
    s32 var_v0_11;
    s32 var_v0_12;
    s32 var_v0_13;
    s32 var_v0_14;
    s32 var_v0_15;
    s32 var_v0_16;
    s32 var_v0_17;
    s32 var_v0_18;
    s32 var_v0_19;
    s32 var_v0_20;
    s32 var_v0_21;
    s32 var_v0_22;
    s32 var_v0_23;
    s32 var_v0_24;
    s32 var_v0_25;
    s32 var_v0_26;
    s32 var_v0_27;
    s32 var_v0_28;
    s32 var_v0_29;
    s32 var_v0_2;
    s32 var_v0_30;
    s32 var_v0_31;
    s32 var_v0_32;
    s32 var_v0_33;
    s32 var_v0_34;
    s32 var_v0_35;
    s32 var_v0_36;
    s32 var_v0_37;
    s32 var_v0_3;
    s32 var_v0_4;
    s32 var_v0_5;
    s32 var_v0_6;
    s32 var_v0_7;
    s32 var_v0_8;
    s32 var_v0_9;
    s32 var_v1;
    s32 var_v1_2;
    s32 var_v1_3;
    s32 var_v1_4;
    s8* var_s3;
    s8* var_s4;
    s32 temp_a0_16;
    s32 temp_a0_6;
    s32 temp_a1_2;
    s32 temp_a1_7;
    s32 temp_v0_11;
    s32 temp_v0_7;
    s32 temp_v0_8;
    u8 temp_a0_18;
    u8 temp_a0_2;
    u8 temp_a0_9;
    u8* var_s4_2;
    void* temp_a0_11;
    void* temp_a0_3;
    void* temp_a0_5;
    void* temp_a0_7;
    void* temp_a2;
    void* temp_a2_3;
    void* temp_s3;
    void* temp_s3_2;
    void* temp_s3_3;
    void* temp_s6;
    void* temp_s6_2;
    void* temp_s6_3;
    void* temp_s6_4;
    void* temp_s6_5;
    void* temp_s6_6;
    void* temp_t5_4;
    void* temp_t6;
    void* var_fp;
    void* var_fp_2;

    sp48 = 0;
    sp4C = 0;
    sp50 = 0;
    sp28 = g_field_scene.scene;
    a0->unk18 = 0;
    var_v0_2 = -a0->unk4 - a0->unk10;
    if (var_v0_2 < 0) {
        var_v0_2 += 0xFF;
    }
    sp30 = (u16) ((u32) var_v0_2 >> 8);
    var_v0_3 = -a0->unk4;
    if (var_v0_3 < 0) {
        var_v0_3 += 0xFF;
    }
    sp38 = (var_v0_3 >> 8) + a0->unk26;
    temp_t6 = sp28->nodes;
    sp2C = temp_t6;
    if (temp_t6 != NULL) {
        if (a0->unk1C == (void* )-1) {
            var_s2 = 0;
            var_v0_4 = a0->unk0;
            var_a3 = 0;
            a0->unk1C = NULL;
            if (var_v0_4 < 0) {
                var_v0_4 += 0xFF;
            }
            probe.x = var_v0_4 >> 8;
            var_v0_5 = a0->unk8;
            if (var_v0_5 < 0) {
                var_v0_5 += 0xFF;
            }
            var_fp = sp28->nodes;
            probe.y = var_v0_5 >> 8;
            ux = (u16) probe.x;
            uy = (u16) probe.y;
            var_t8 = 0;
            while (var_fp != NULL) {
                    temp_s6 = ((Move_UnkNode1*)var_fp)->unk4;
                    if ((((Move_UnkNode1*)var_fp)->unk18 != 0) && ((temp_v0 = (s32) ((Move_UnkNode1*)sp2C)->unk38 >> 8, temp_v1 = ((Move_UnkNode2*)temp_s6)->unk14 + (s16) temp_v0, (temp_v1 == 0)) || (temp_v1 < ((s16) sp30 + a0->unk26)) || (temp_v1 < (s16) sp38))) {
                        temp_a1 = (s32) ((Move_UnkNode1*)sp2C)->unk34 >> 8;
                        temp_a0 = (s32) (((Move_UnkNode1*)sp2C)->unk40 << 8) >> 0x10;
                        temp_v1_2 = ((Move_UnkNode1*)var_fp)->unk22 + temp_a0;
                        if (((s16) uy >= temp_v1_2) && ((((Move_UnkNode1*)var_fp)->unk20 + temp_a0) >= (s16) uy)) {
                            temp_a0_2 = ((u8*)temp_s6)[6];
                            var_s1 = (u8*)((Move_UnkNode1*)var_fp)->unk10 + (((s16) uy - temp_v1_2) * temp_a0_2 * 4);
                            if ((s16) temp_a1 != 0) {
                                var_s0 = temp_a0_2 - 1;
                                if (temp_a0_2 != 0) {
                                    do {
                                        if (((s16) ux < (((Move_UnkS16*)var_s1)->unk0 + (s16) temp_a1)) || ((((Move_UnkS16*)var_s1)->unk2 + (s16) temp_a1) < (s16) ux)) {
                                            var_s1 += 4;
                                        } else {
                                            goto a_hit;
                                        }
                                    } while (--var_s0 != -1);
                                }
                            } else {
                                var_s0_2 = temp_a0_2 - 1;
                                if (temp_a0_2 != 0) {
                                    do {
                                        if (((s16) ux < ((Move_UnkS16*)var_s1)->unk0) || (((Move_UnkS16*)var_s1)->unk2 < (s16) ux)) {
                                            var_s1 += 4;
                                        } else {
                                            goto a_hit;
                                        }
                                    } while (--var_s0_2 != -1);
                                }
                            }
                                goto a_done;
                            a_hit:
                                var_t8 = 1;
                            a_done:
                            if (var_t8 != 0) {
                                temp_v1_3 = ((Move_UnkNode2*)temp_s6)->unk4 & 3;
                                switch (temp_v1_3) { /* switch 1; irregular */
                                case 0:             /* switch 1 */
                                    if ((((Move_UnkNode2*)temp_s6)->unk14 + (s16) temp_v0) < (s16) sp38) {
                                        if (var_a3 != 0) {
                                            temp_v1_4 = ((Move_UnkNode2*)temp_s6)->unk10 + (s16) temp_v0;
                                            if ((var_s2 + 0x14) < temp_v1_4) {
                                                var_s2 = temp_v1_4;
                                                a0->unk1C = var_fp;
                                            }
                                        } else {
                                            temp_v1_5 = ((Move_UnkNode2*)temp_s6)->unk10 + (s16) temp_v0;
                                            if (temp_v1_5 >= var_s2) {
                                                var_fp_2 = var_fp;
                                                var_s2 = temp_v1_5;
                                                a0->unk1C = var_fp_2;
                                            }
                                        }
                                    }
                                    break;
                                case 1:             /* switch 1 */
                                    if ((((Move_UnkNode2*)temp_s6)->unk14 + (s16) temp_v0) < (s16) sp38) {
                                        temp_s0 = func_8005DFAC(var_fp_2, &probe.x);
                                        if (var_a3 != 0) {
                                            var_a3 = 1;
                                            if (var_s2 < temp_s0) {
                                                var_s2 = temp_s0;
                                                a0->unk1C = var_fp_2;
                                            }
                                        } else {
                                            var_a3 = 1;
                                            if (var_s2 < (temp_s0 + 0x14)) {
                                                var_s2 = temp_s0;
                                                a0->unk1C = var_fp_2;
                                            }
                                        }
                                    }
                                    break;
                                }
                            }
                        }
                    }
                    var_fp = ((Move_UnkNode1*)var_fp)->unk0;
            }
        }
        temp_a0_3 = a0->unk1C;
        if ((temp_a0_3 != NULL) && (temp_a0_3 != (void* )-2)) {
            temp_s6_2 = ((Move_UnkNode1*)temp_a0_3)->unk4;
            if (((Move_UnkNode1*)temp_a0_3)->unk18 != 0) {
                if (((((Move_UnkNode2*)temp_s6_2)->unk4 & 3) == 1) && (((Move_UnkNode2*)temp_s6_2)->unk10 != ((Move_UnkNode2*)temp_s6_2)->unk12)) {
                    probe.x = a0->unkC;
                    probe.y = a0->unk14;
                    func_80062F48(temp_s6_2, &probe.x);
                    a0->unkC = probe.x;
                    sp50 |= 4;
                    a0->unk14 = probe.y;
                }
                if (((Move_UnkNode2*)temp_s6_2)->unk4 & 0x10) {
                    temp_t5 = a0->unkC;
                    sp5C = temp_t5;
                    if (temp_t5 >= 0) {
                        var_v0_6 = temp_t5 >> 1;
                    } else {
                        var_v0_6 = (s32) (sp5C + 1) >> 1;
                    }
                    a0->unkC = var_v0_6;
                    temp_t5_2 = a0->unk14;
                    sp60 = temp_t5_2;
                    if (temp_t5_2 >= 0) {
                        var_v0_7 = temp_t5_2 >> 1;
                    } else {
                        var_v0_7 = (s32) (sp60 + 1) >> 1;
                    }
                    a0->unk14 = var_v0_7;
                    sp50 |= 4;
                }
                if (((Move_UnkNode2*)temp_s6_2)->unk4 & 0x40) {
                    sp50 |= 0x10;
                }
                if (!(a0->unk28 & 0x30000) && ((((Move_UnkNode1*)temp_a0_3)->unk24 != 0) || (((Move_UnkNode1*)temp_a0_3)->unk28 != 0) || (((Move_UnkNode1*)temp_a0_3)->unk2C != 0) || (((Move_UnkNode1*)temp_a0_3)->unk30 != 0))) {
                    a0->unkC = (s32) (a0->unkC + ((Move_UnkNode1*)temp_a0_3)->unk24);
                    temp_s2 = a0->unk10;
                    a0->unk14 = (s32) (a0->unk14 + ((Move_UnkNode1*)temp_a0_3)->unk30);
                    if ((((Move_UnkNode2*)temp_s6_2)->unk4 & 3) == 1) {
                        var_v0_8 = a0->unk0;
                        if (var_v0_8 < 0) {
                            var_v0_8 += 0xFF;
                        }
                        probe.x = var_v0_8 >> 8;
                        var_v0_9 = a0->unk8;
                        if (var_v0_9 < 0) {
                            var_v0_9 += 0xFF;
                        }
                        probe.y = var_v0_9 >> 8;
                        a0->unk10 = (s32) (-((s32) (func_8005DFAC(temp_a0_3, &probe.x) << 0x10) >> 8) - a0->unk4);
                    } else {
                        a0->unk10 = (s32) (temp_s2 + ((Move_UnkNode1*)temp_a0_3)->unk28);
                    }
                    if (temp_s2 != a0->unk10) {
                        sp50 |= 0x30;
                    } else if ((((Move_UnkNode1*)temp_a0_3)->unk24 != 0) || (((Move_UnkNode1*)temp_a0_3)->unk30 != 0)) {
                        sp50 |= 0x10;
                    }
                } else {
                    temp_s3 = sp28->secondary_nodes;
                    if ((temp_s3 != NULL) && (temp_s3 != temp_a0_3) && ((((Move_UnkNode1*)temp_s3)->unk24 != 0) || (((Move_UnkNode1*)temp_s3)->unk28 != 0) || (((Move_UnkNode1*)temp_s3)->unk2C != 0) || (((Move_UnkNode1*)temp_s3)->unk30 != 0))) {
                        temp_s2_2 = a0->unk10;
                        a0->unkC = (s32) (a0->unkC + ((Move_UnkNode1*)temp_s3)->unk24);
                        temp_a0_4 = a0->unk0;
                        a0->unk14 = (s32) (a0->unk14 + ((Move_UnkNode1*)temp_s3)->unk30);
                        if (temp_a0_4 >= 0) {
                            var_v0_10 = temp_a0_4 >> 8;
                        } else {
                            var_v0_10 = (s32) (temp_a0_4 + 0xFF) >> 8;
                        }
                        probe.x = var_v0_10;
                        var_v0_11 = a0->unk8;
                        if (var_v0_11 < 0) {
                            var_v0_11 += 0xFF;
                        }
                        probe.y = var_v0_11 >> 8;
                        temp_s0_2 = func_8005DFAC(temp_s3, &probe.x) - ((Move_UnkNode2*)((Move_UnkNode1*)temp_s3)->unk4)->unk10;
                        if ((((Move_UnkNode2*)temp_s6_2)->unk4 & 3) == 1) {
                            a0->unk10 = (s32) (-((temp_s0_2 + func_8005DFAC(temp_a0_3, &probe.x)) << 8) - a0->unk4);
                        } else {
                            a0->unk10 = (s32) (a0->unk10 + (((Move_UnkNode1*)temp_a0_3)->unk28 - (temp_s0_2 << 8)));
                        }
                        if (temp_s2_2 != a0->unk10) {
                            sp50 |= 0x30;
                        } else if ((((Move_UnkNode1*)temp_a0_3)->unk24 != 0) || (((Move_UnkNode1*)temp_a0_3)->unk30 != 0)) {
                            sp50 |= 0x10;
                        }
                    }
                }
            }
        }
    }
    if ((a0->unkC == 0) && (a0->unk10 == 0) && (a0->unk14 == 0)) {
        var_v0 = 0;
        if ((sp2C != NULL) && (var_v0 = 0, (a0->unk4 != 0))) {
            if ((a0->unk20 & 1) && (temp_a0_5 = a0->unk1C, (temp_a0_5 != (void* )-2))) {
                var_v0 = 0;
                if (temp_a0_5 != NULL) {
                    temp_s6_3 = ((Move_UnkNode1*)temp_a0_5)->unk4;
                    temp_v1_6 = ((Move_UnkNode2*)temp_s6_3)->unk4 & 3;
                    switch (temp_v1_6) {            /* switch 2; irregular */
                    case 0:                         /* switch 2 */
                        var_v0_12 = ((Move_UnkNode1*)temp_a0_5)->unk38 + (((Move_UnkNode2*)temp_s6_3)->unk10 << 8);
                        a0->unk18 = (s32) -var_v0_12;
                        return 0;
                    case 1:                         /* switch 2 */
                        var_v0_13 = a0->unk0;
                        if (var_v0_13 < 0) {
                            var_v0_13 += 0xFF;
                        }
                        probe.x = var_v0_13 >> 8;
                        var_v0_14 = a0->unk8;
                        if (var_v0_14 < 0) {
                            var_v0_14 += 0xFF;
                        }
                        probe.y = var_v0_14 >> 8;
                        temp_s3_2 = sp28->secondary_nodes;
                        var_s0_3 = func_8005DFAC(temp_a0_5, &probe.x);
                        if ((temp_s3_2 != NULL) && (temp_s3_2 != temp_a0_5)) {
                            var_s0_3 += func_8005DFAC(temp_s3_2, &probe.x) - ((Move_UnkNode2*)((Move_UnkNode1*)temp_s3_2)->unk4)->unk10;
                        }
                        var_v0_12 = var_s0_3 << 8;
                        a0->unk18 = (s32) -var_v0_12;
                        return 0;
                    default:                        /* switch 2 */
                        return 0;
                    }
                } else {
                    /* Duplicate return node #441. Try simplifying control flow for better match */
                    return var_v0;
                }
            }
        } else {
            /* Duplicate return node #441. Try simplifying control flow for better match */
            return var_v0;
        }
    }
    {
        probe.m = a0;
        probe.w = sp30;
        probe.h = sp38;
        var_v0_15 = a0->unkC;
        temp_v1_7 = a0->unk24;
        if (var_v0_15 < 0) {
            var_v0_15 = -var_v0_15;
        }
        temp_v0_2 = var_v0_15 >> 8;
        sp54 = temp_v0_2;
        var_t9 = 1;
        if (temp_v0_2 >= temp_v1_7) {
            var_t9 = (sp54 / temp_v1_7) + 1;
        }
        var_v0_16 = a0->unk14;
        temp_v1_8 = (s16) a0->unk28;
        if (var_v0_16 < 0) {
            var_v0_16 = -var_v0_16;
        }
        temp_v0_3 = var_v0_16 >> 8;
        sp54 = temp_v0_3;
        if (temp_v0_3 >= temp_v1_8) {
            temp_v0_4 = (sp54 / temp_v1_8) + 1;
            sp54 = temp_v0_4;
            if (var_t9 < temp_v0_4) {
                var_t9 = sp54;
            }
        }
        if (var_t9 == 1) {
            var_v0_17 = a0->unk0 + a0->unkC;
            if (var_v0_17 < 0) {
                var_v0_17 += 0xFF;
            }
            probe.x = var_v0_17 >> 8;
            var_v0_18 = a0->unk8 + a0->unk14;
            if (var_v0_18 < 0) {
                var_v0_18 += 0xFF;
            }
            probe.y = var_v0_18 >> 8;
            func_8005DA7C(&probe, sp2C, &sp20, &sp24);
        } else {
            sp54 = 0;
            do {
                temp_t5_3 = sp54 + 1;
                sp54 = temp_t5_3;
                var_v0_19 = a0->unk0 + ((s32) (a0->unkC * temp_t5_3) / var_t9);
                if (var_v0_19 < 0) {
                    var_v0_19 += 0xFF;
                }
                probe.x = var_v0_19 >> 8;
                var_v0_20 = a0->unk8 + ((s32) (a0->unk14 * sp54) / var_t9);
                if (var_v0_20 < 0) {
                    var_v0_20 += 0xFF;
                }
                probe.y = var_v0_20 >> 8;
                func_8005DA7C(&probe, sp2C, &sp20, &sp24);
            } while ((var_t9 != sp54) && (sp20 == 0));
        }
        var_t8 = 0;
        if (sp20 == 0) {
            temp_a2 = sp28->header;
            if ((((Move_UnkNode3*)temp_a2)->unk2C & 2) && ((temp_a0_6 = (u16) a0->unk24, temp_a1_2 = (u16) a0->unk28, temp_a3 = (u16) probe.x - ((s32) ((s16) temp_a0_6 + ((u32) (temp_a0_6 << 0x10) >> 0x1F)) >> 1), temp_v0_5 = (u16) probe.y - ((s32) ((s16) temp_a1_2 + ((u32) (temp_a1_2 << 0x10) >> 0x1F)) >> 1), (temp_v0_5 & 0x8000)) || ((s16) (temp_v0_5 + temp_a1_2) >= ((Move_UnkNode3*)temp_a2)->unk32) || (temp_a3 & 0x8000) || ((s16) (temp_a3 + temp_a0_6) >= ((Move_UnkNode3*)temp_a2)->unk30))) {
                var_t8 = 1;
            }
        }
        if ((sp20 != 0) || (var_t8 != 0)) {
        temp_a1_3 = a0->unkC;
        if (temp_a1_3 == 0) {
            sp58 = 0xC00;
            if (a0->unk14 >= 0) {
                sp58 = 0x400;
            }
        } else {
            sp58 = ratan2(a0->unk14, temp_a1_3) & 0xFFF;
        }
        temp_v0_6 = a0->unkC;
        var_v1_2 = a0->unk14;
        var_t9 = temp_v0_6;
        if (temp_v0_6 < 0) {
            var_t9 = -var_t9;
        }
        if (var_v1_2 < 0) {
            var_v1_2 = -var_v1_2;
        }
        sp54 = var_v1_2;
        if (var_t9 >= var_v1_2) {
            var_t9 = var_t9 >> 8;
        } else {
            var_t9 = sp54 >> 8;
        }
        var_s5 = -2;
        if (var_t9 == 0) {
            var_t9 = 1;
        }
        sp54 = var_t9 - 1;
        if (var_t9 != 0) {
            do {
            temp_v0_7 = (u16) a0->unk24;
            var_v0_21 = a0->unk0 + ((s32) (a0->unkC * (var_t9 - sp54)) / var_t9);
            if (var_v0_21 < 0) {
                var_v0_21 += 0xFF;
            }
            temp_s1 = (var_v0_21 >> 8) - ((s32) ((s16) temp_v0_7 + ((u32) (temp_v0_7 << 0x10) >> 0x1F)) >> 1);
            temp_v0_8 = (u16) a0->unk28;
            temp_s2_3 = temp_s1 + (u16) a0->unk24;
            var_v0_22 = a0->unk8 + ((s32) (a0->unk14 * (var_t9 - sp54)) / var_t9);
            if (var_v0_22 < 0) {
                var_v0_22 += 0xFF;
            }
            temp_s0_3 = (var_v0_22 >> 8) - ((s32) ((s16) temp_v0_8 + ((u32) (temp_v0_8 << 0x10) >> 0x1F)) >> 1);
            temp_a0_7 = sp28->header;
            temp_v1_9 = temp_s0_3 + (u16) a0->unk28;
            if (((Move_UnkNode3*)temp_a0_7)->unk2C & 2) {
                if ((temp_s0_3 < 0) || (temp_v1_9 >= ((Move_UnkNode3*)temp_a0_7)->unk32)) {
                    var_s5 = func_8005E1A8(NULL, 0x7F, sp58, var_s5);
                }
                if ((temp_s1 < 0) || (temp_s2_3 >= ((Move_UnkNode3*)sp28->header)->unk30)) {
                    var_s5 = func_8005E1A8(NULL, 0x7E, sp58, var_s5);
                }
            }
            sp64 = (void** )0x801E1000;
            sp68 = (void** )0x801E1100;
            var_t7 = sp20 - 1;
            sp24 = 0;
            if (var_t7 != -1) {
                e_x_min = temp_s1;
                sp74 = temp_s0_3;
                sp70 = temp_v1_9;
                sp78 = temp_v1_9 - 1;
                sp6C = temp_s2_3;
                sp7C = temp_s2_3 - 1;
                do {
                    var_fp = *sp64;
                    sp64 += 1;
                    temp_a0_8 = (s32) ((Move_UnkNode1*)sp2C)->unk40 >> 8;
                    temp_a2_2 = (s32) (((Move_UnkNode1*)sp2C)->unk34 << 8) >> 0x10;
                    temp_s6_4 = ((Move_UnkNode1*)var_fp)->unk4;
                    if (((((Move_UnkNode1*)var_fp)->unk1C + temp_a2_2) < sp6C) && (((((Move_UnkNode1*)var_fp)->unk1E + temp_a2_2) >= e_x_min))) {
                        temp_a1_4 = ((Move_UnkNode1*)var_fp)->unk22 + (s16) temp_a0_8;
                        if (temp_a1_4 < sp70) {
                            temp_v1_10 = ((Move_UnkNode1*)var_fp)->unk20 + (s16) temp_a0_8;
                            if (temp_v1_10 >= sp74) {
                                var_s0_4 = temp_v1_10;
                                var_t1 = temp_a1_4;
                                if (sp78 < temp_v1_10) {
                                    var_s0_4 = sp78;
                                }
                                if (var_t1 < sp74) {
                                    var_t1 = sp74;
                                }
                                temp_a0_9 = ((u8*)temp_s6_4)[6];
                                temp_lo = (var_t1 - temp_a1_4) * temp_a0_9;
                                temp_lo_2 = ((var_s0_4 - var_t1) + 1) * temp_a0_9;
                                var_t8 = 0;
                                var_s1_2 = (u8*)((Move_UnkNode1*)var_fp)->unk10 + (temp_lo * 4);
                                var_s4 = (s8*)((Move_UnkNode1*)var_fp)->unk14 + (temp_lo * 2);
                                if (((Move_UnkNode2*)temp_s6_4)->unk4 & 8) {
                                    var_s0_5 = temp_lo_2 - 1;
                                    var_s3 = var_s4 + 1;
                                    if (var_s0_5 != -1) {
                                        var_s2_2 = var_s1_2 + 2;
                                        do {
                                            temp_v1_11 = *(s16*)var_s1_2;
                                            if ((temp_v1_11 < sp6C) && (*(s16*)var_s2_2 >= e_x_min)) {
                                                if ((e_x_min < temp_v1_11) && (*var_s4 >= 0)) {
                                                    var_s5 = func_8005E1A8(temp_s6_4, (u8) *var_s4 & 0x7F, sp58, var_s5);
                                                    var_t8 = 2;
                                                }
                                                if ((*(s16*)var_s2_2 < sp7C) && (*var_s3 >= 0)) {
                                                    var_s5 = func_8005E1A8(temp_s6_4, (u8) *var_s3 & 0x7F, sp58, var_s5);
                                                    var_t8 = 2;
                                                }
                                                if ((*var_s4 >= 0) && (*var_s3 >= 0) && (((u8) *var_s4 == 0x7F) || ((u8) *var_s3 == 0x7F)) && (*(s16*)var_s1_2 < e_x_min) && (*(s16*)var_s2_2 >= sp6C)) {
                                                    var_s5 = func_8005E1A8(temp_s6_4, 0x7F, sp58, var_s5);
                                                    var_t8 = 2;
                                                }
                                            }
                                            var_s2_2 += 4;
                                            var_s1_2 += 4;
                                            var_s3 += 2;
                                            var_s0_5 -= 1;
                                            var_s4 += 2;
                                        } while (var_s0_5 != -1);
                                    }
                                } else if (temp_a2_2 != 0) {
                                    var_s0_6 = temp_lo_2 - 1;
                                    if (var_s0_6 != -1) {
                                        var_s2_3 = var_s1_2 + 2;
                                        do {
                                            temp_v1_12 = *(s16*)var_s1_2 + temp_a2_2;
                                            if ((temp_v1_12 < sp6C) && ((*(s16*)var_s2_3 + temp_a2_2) >= e_x_min)) {
                                                var_t8 = 1;
                                                if (e_x_min < temp_v1_12) {
                                                    var_s5 = func_8005E1A8(temp_s6_4, var_s4[0] & 0x7F, sp58, var_s5);
                                                    var_t8 = 1;
                                                }
                                                if ((*(s16*)var_s2_3 + temp_a2_2) < sp7C) {
                                                    var_s5 = func_8005E1A8(temp_s6_4, var_s4[1] & 0x7F, sp58, var_s5);
                                                    var_t8 = 1;
                                                }
                                                if (((*(s16*)var_s1_2 + temp_a2_2) < e_x_min) && ((*(s16*)var_s2_3 + temp_a2_2) >= sp6C)) {
                                                    var_s5 = func_8005E1A8(temp_s6_4, 0x7F, sp58, var_s5);
                                                }
                                            }
                                            var_s2_3 += 4;
                                            var_s1_2 += 4;
                                            var_s0_6 -= 1;
                                            var_s4 += 2;
                                        } while (var_s0_6 != -1);
                                    }
                                } else {
                                    var_s0_7 = temp_lo_2 - 1;
                                    if (var_s0_7 != -1) {
                                        var_s2_4 = var_s1_2 + 2;
                                        do {
                                            temp_v1_13 = *(s16*)var_s1_2;
                                            if ((temp_v1_13 < sp6C) && (*(s16*)var_s2_4 >= e_x_min)) {
                                                var_t8 = 1;
                                                if (e_x_min < temp_v1_13) {
                                                    var_s5 = func_8005E1A8(temp_s6_4, var_s4[0] & 0x7F, sp58, var_s5);
                                                    var_t8 = 1;
                                                }
                                                if (*(s16*)var_s2_4 < sp7C) {
                                                    var_s5 = func_8005E1A8(temp_s6_4, var_s4[1] & 0x7F, sp58, var_s5);
                                                    var_t8 = 1;
                                                }
                                                if ((*(s16*)var_s1_2 < e_x_min) && (*(s16*)var_s2_4 >= sp6C)) {
                                                    var_s5 = func_8005E1A8(temp_s6_4, 0x7F, sp58, var_s5);
                                                }
                                            }
                                            var_s2_4 += 4;
                                            var_s1_2 += 4;
                                            var_s0_7 -= 1;
                                            var_s4 += 2;
                                        } while (var_s0_7 != -1);
                                    }
                                }
                                if (var_t8 != 0) {
                                    *sp68 = var_fp;
                                    sp68 += 1;
                                    sp24 += 1;
                                }
                            }
                        }
                    }
                    var_t7 -= 1;
                } while (var_t7 != -1);
            }
            if (var_s5 != -2) {
                break;
            }
            temp_t6_2 = sp54 - 1;
            sp54 = temp_t6_2;
            } while (temp_t6_2 != -1);
        }
        temp_a0_10 = a0->unkC;
        temp_v0_9 = (var_t9 - 1) - sp54;
        sp5C = (s32) (temp_a0_10 * temp_v0_9) / var_t9;
        temp_v1_14 = a0->unk14;
        temp_lo_3 = (s32) (temp_v1_14 * temp_v0_9) / var_t9;
        sp60 = temp_lo_3;
        if ((var_s5 >= 0) && (sp5C == 0) && (temp_lo_3 == 0)) {
            temp_s0_4 = SquareRoot0((temp_a0_10 * temp_a0_10) + (temp_v1_14 * temp_v1_14));
            if ((temp_s0_4 * rcos(var_s5)) >= 0) {
                sp5C = (s32) (temp_s0_4 * rcos(var_s5)) >> 0xC;
            } else {
                sp5C = (s32) ((temp_s0_4 * rcos(var_s5)) + 0xFFF) >> 0xC;
            }
            if ((temp_s0_4 * rsin(var_s5)) >= 0) {
                sp60 = (s32) (temp_s0_4 * rsin(var_s5)) >> 0xC;
            } else {
                sp60 = (s32) ((temp_s0_4 * rsin(var_s5)) + 0xFFF) >> 0xC;
            }
            sp20 = 1;
            sp80 = (s16) (u16) a0->unk24 / 2;
            do {
            temp_v0_10 = a0->unk0 + sp5C;
            if (temp_v0_10 >= 0) {
                var_s1_3 = (temp_v0_10 >> 8) - sp80;
            } else {
                var_s1_3 = ((s32) (temp_v0_10 + 0xFF) >> 8) - sp80;
            }
            temp_v0_11 = (u16) a0->unk28;
            temp_s2_4 = var_s1_3 + (u16) a0->unk24;
            var_v0_24 = a0->unk8 + sp60;
            if (var_v0_24 < 0) {
                var_v0_24 += 0xFF;
            }
            temp_s0_5 = (var_v0_24 >> 8) - ((s32) ((s16) temp_v0_11 + ((u32) (temp_v0_11 << 0x10) >> 0x1F)) >> 1);
            var_t8 = 0;
            var_t0 = 0;
            var_a3_2 = 0;
            k_blocked = 0x8000;
            sp68 = (void** )0x801E1100;
            temp_a0_11 = sp28->header;
            temp_v1_15 = temp_s0_5 + (u16) a0->unk28;
            if (((Move_UnkNode3*)temp_a0_11)->unk2C & 2) {
                if (temp_s0_5 < 0) {
                    var_a3_2 = -temp_s0_5;
                    var_t8 = 1;
                } else {
                    temp_a0_12 = ((Move_UnkNode3*)temp_a0_11)->unk32;
                    if (temp_v1_15 >= temp_a0_12) {
                        var_a3_2 = (temp_a0_12 - temp_v1_15) - 1;
                        var_t8 = 1;
                    }
                }
                if (var_s1_3 < 0) {
                    var_t0 = -var_s1_3;
                    var_t8 = 1;
                } else {
                    temp_a0_13 = ((Move_UnkNode3*)sp28->header)->unk30;
                    if (temp_s2_4 >= temp_a0_13) {
                        var_t0 = (temp_a0_13 - temp_s2_4) - 1;
                        var_t8 = 1;
                    }
                }
            }
            var_t9 = sp24 - 1;
            if (var_t9 != -1) {
                f_x_max = temp_s2_4;
                f_x_min = var_s1_3;
                f_y_min = temp_s0_5;
                f_y_max = temp_v1_15;
                sp8C = f_y_max - 1;
                temp_v1_16 = (s32) ((Move_UnkNode1*)sp2C)->unk34 >> 8;
                sp40 = (u16) temp_v1_16;
                sp84 = (s32) (s16) temp_v1_16;
                sp88 = (s32) (((Move_UnkNode1*)sp2C)->unk40 << 8) >> 0x10;
                do {
                    var_fp = *sp68;
                    sp68 += 1;
                    temp_s6_5 = ((Move_UnkNode1*)var_fp)->unk4;
                    if (((((Move_UnkNode1*)var_fp)->unk1C + sp84) < f_x_max) && (((((Move_UnkNode1*)var_fp)->unk1E + sp84) >= f_x_min))) {
                        temp_a0_14 = ((Move_UnkNode1*)var_fp)->unk22 + sp88;
                        if (temp_a0_14 < f_y_max) {
                            temp_v1_17 = ((Move_UnkNode1*)var_fp)->unk20 + sp88;
                            if (temp_v1_17 >= f_y_min) {
                                var_s0_8 = temp_v1_17;
                                var_t1_2 = temp_a0_14;
                                if (sp8C < temp_v1_17) {
                                    var_s0_8 = sp8C;
                                }
                                if (var_t1_2 < f_y_min) {
                                    var_t1_2 = f_y_min;
                                }
                                temp_lo_4 = (var_t1_2 - temp_a0_14) * ((u8*)temp_s6_5)[6];
                                var_s0_9 = var_s0_8 - var_t1_2;
                                var_s1_4 = (u8*)((Move_UnkNode1*)var_fp)->unk10 + (temp_lo_4 * 4);
                                f_x_max_2 = f_x_max;
                                var_s4_2 = (u8*)((Move_UnkNode1*)var_fp)->unk14 + (temp_lo_4 * 2);
                                if (var_s0_9 != -1) {
                                    temp_s7 = f_x_max_2 - 1;
                                    do {
                                        var_t7 = ((u8*)temp_s6_5)[6] - 1;
                                        if (var_t7 != -1) {
                                            temp_s5 = var_t1_2 + 1;
                                            var_t3 = var_s1_4 + 2;
                                            spAC = (s32) (s16) sp40;
                                            spA8 = var_s4_2 + 1;
                                            do {
                                                temp_v1_18 = *(s16*)var_s1_4;
                                                if (temp_v1_18 < f_x_max) {
                                                    temp_a1_5 = *(s16*)var_t3;
                                                    if (temp_a1_5 >= f_x_min) {
                                                        var_a2 = 0;
                                                        if (((Move_UnkNode2*)temp_s6_5)->unk4 & 8) {
                                                            var_a0 = 0;
                                                            if ((f_x_min < temp_v1_18) && !(*var_s4_2 & 0x80)) {
                                                                var_a2 = 1;
                                                                if (temp_a1_5 >= f_x_max_2) {
                                                                    var_a0 = temp_v1_18 - f_x_max_2;
                                                                }
                                                            } else {
                                                                temp_v1_19 = *(s16*)var_t3;
                                                                if ((temp_v1_19 < temp_s7) && !(*spA8 & 0x80)) {
                                                                    if (*(s16*)var_s1_4 < f_x_min) {
                                                                        var_a0 = (temp_v1_19 - f_x_min) + 1;
                                                                    }
                                                                    var_a2 = 1;
                                                                }
                                                            }
                                                            if (!(*var_s4_2 & 0x80) && !(*spA8 & 0x80)) {
                                                                var_a2 = 1;
                                                            }
                                                        } else {
                                                            var_a2 = 1;
                                                            temp_v1_20 = temp_v1_18 + spAC;
                                                            var_a0 = 0;
                                                            if ((f_x_min < temp_v1_20) && ((temp_a1_5 + spAC) >= f_x_max_2)) {
                                                                var_a0 = temp_v1_20 - f_x_max_2;
                                                            } else if ((*(s16*)var_s1_4 + spAC) < f_x_min) {
                                                                temp_v1_21 = *(s16*)var_t3 + spAC;
                                                                if (temp_v1_21 < temp_s7) {
                                                                    var_a0 = (temp_v1_21 - f_x_min) + 1;
                                                                }
                                                            }
                                                        }
                                                        temp_v1_22 = var_t1_2 - f_y_min;
                                                        if (var_a2 != 0) {
                                                            var_t8 = 1;
                                                            if ((f_y_max - temp_s5) >= temp_v1_22) {
                                                                var_a1 = temp_v1_22 + 1;
                                                            } else {
                                                                var_a1 = (temp_s5 - f_y_max) - 1;
                                                            }
                                                            if ((var_t0 != 0) || (var_a3_2 != 0)) {
                                                                if ((var_t0 != 0x8000) && (var_a0 != 0)) {
                                                                    if (var_a0 > 0) {
                                                                        var_v0_26 = var_t0 < var_a0;
                                                                        if (var_t0 >= 0) {
                                                                            if (var_v0_26 != 0) {
                                                                                var_t0 = var_a0;
                                                                            }
                                                                        } else {
                                                                            var_t0 = 0x8000;
                                                                        }
                                                                    } else {
                                                                        var_v0_26 = var_a0 < var_t0;
                                                                        if (var_t0 <= 0) {
                                                                            if (var_v0_26 != 0) {
                                                                                var_t0 = var_a0;
                                                                            }
                                                                        } else {
                                                                            var_t0 = 0x8000;
                                                                        }
                                                                    }
                                                                }
                                                                if (var_a3_2 != k_blocked) {
                                                                    if (var_a1 > 0) {
                                                                        if (var_a3_2 >= 0) {
                                                                            if (var_a3_2 < var_a1) {
                                                                                var_a3_2 = var_a1;
                                                                            }
                                                                        } else {
                                                                            var_a3_2 = k_blocked;
                                                                        }
                                                                    } else if (var_a3_2 <= 0) {
                                                                        if (var_a1 < var_a3_2) {
                                                                            var_a3_2 = var_a1;
                                                                        }
                                                                    } else {
                                                                        var_a3_2 = 0x8000;
                                                                    }
                                                                }
                                                            } else {
                                                                var_t0 = var_a0;
                                                                var_a3_2 = var_a1;
                                                            }
                                                        }
                                                    }
                                                }
                                                var_t3 += 4;
                                                var_s1_4 += 4;
                                                var_s4_2 += 2;
                                                var_t7 -= 1;
                                                spA8 += 2;
                                            } while (var_t7 != -1);
                                        }
                                        var_s0_9 -= 1;
                                        var_t1_2 += 1;
                                    } while (var_s0_9 != -1);
                                }
                            }
                        }
                    }
                    var_t9 -= 1;
                } while (var_t9 != -1);
            }
            if (var_t8 == 0) {
                break;
            }
            temp_a0_15 = var_t0 << 8;
            if (sp20 == 1) {
                temp_a1_6 = var_a3_2 << 8;
                if ((var_t0 == 0) || (var_t0 == k_blocked)) {
                    if (var_a3_2 != k_blocked) {
                        sp60 += temp_a1_6;
                    }
                    break;
                } else if ((var_a3_2 == 0) || (var_a3_2 == k_blocked)) {
                    sp5C += temp_a0_15;
                    break;
                } else {
                    var_v1_3 = temp_a0_15;
                    if (var_v1_3 < 0) {
                        var_v1_3 = -var_v1_3;
                    }
                    var_v0_27 = temp_a1_6;
                    if (var_v0_27 < 0) {
                        var_v0_27 = -var_v0_27;
                    }
                    if (var_v0_27 < var_v1_3) {
                        sp48 = temp_a0_15;
                        sp4C = 0;
                        sp60 += temp_a1_6;
                    } else {
                        sp4C = temp_a1_6;
                        sp48 = 0;
                        sp5C += temp_a0_15;
                    }
                }
            } else {
                sp5C += sp48;
                sp60 += sp4C;
                break;
            }
            temp_v0_13 = sp20 - 1;
            sp20 = temp_v0_13;
            } while (temp_v0_13 != -1);
            sp50 |= 2;
        } else {
            sp50 |= 1;
        }
        if ((sp5C != 0) || (sp60 != 0)) {
            var_v0_28 = a0->unk0 + sp5C;
            if (var_v0_28 < 0) {
                var_v0_28 += 0xFF;
            }
            probe.x = var_v0_28 >> 8;
            var_v0_29 = a0->unk8 + sp60;
            if (var_v0_29 < 0) {
                var_v0_29 += 0xFF;
            }
            probe.y = var_v0_29 >> 8;
            func_8005DA7C(&probe, sp2C, &sp20, &sp24);
            var_t8 = 0;
            if (sp20 == 0) {
                temp_a2_3 = sp28->header;
                if ((((Move_UnkNode3*)temp_a2_3)->unk2C & 2) && ((temp_a0_16 = (u16) a0->unk24, temp_a1_7 = (u16) a0->unk28, temp_a3_2 = (u16) probe.x - ((s32) ((s16) temp_a0_16 + ((u32) (temp_a0_16 << 0x10) >> 0x1F)) >> 1), temp_v0_14 = (u16) probe.y - ((s32) ((s16) temp_a1_7 + ((u32) (temp_a1_7 << 0x10) >> 0x1F)) >> 1), (temp_v0_14 & k_blocked)) || ((s16) (temp_v0_14 + temp_a1_7) >= ((Move_UnkNode3*)temp_a2_3)->unk32) || (temp_a3_2 & k_blocked) || ((s16) (temp_a3_2 + temp_a0_16) >= ((Move_UnkNode3*)temp_a2_3)->unk30))) {
                    var_t8 = 1;
                }
            }
            if ((sp20 != 0) || (var_t8 != 0)) {
                var_v0_30 = a0->unk0;
                if (var_v0_30 < 0) {
                    var_v0_30 += 0xFF;
                }
                probe.x = var_v0_30 >> 8;
                var_v0_31 = a0->unk8;
                if (var_v0_31 < 0) {
                    var_v0_31 += 0xFF;
                }
                probe.y = var_v0_31 >> 8;
                func_8005DA7C(&probe, sp2C, &sp20, &sp24);
                sp50 |= 3;
            } else {
                a0->unk0 = (s32) (a0->unk0 + sp5C);
                a0->unk8 = (s32) (a0->unk8 + sp60);
            }
        } else {
            var_v0_32 = a0->unk0;
            if (var_v0_32 < 0) {
                var_v0_32 += 0xFF;
            }
            probe.x = var_v0_32 >> 8;
            var_v0_33 = a0->unk8;
            if (var_v0_33 < 0) {
                var_v0_33 += 0xFF;
            }
            probe.y = var_v0_33 >> 8;
            func_8005DA7C(&probe, sp2C, &sp20, &sp24);
            sp50 |= 3;
        }
        } else {
            a0->unk0 = (s32) (a0->unk0 + a0->unkC);
            a0->unk8 = (s32) (a0->unk8 + a0->unk14);
        }
        var_s7 = 0xFFFFFF;
        var_s2 = 0;
        var_s4_3 = 0;
        var_a3 = 0;
        var_v0_34 = a0->unk0;
        var_fp = NULL;
        a0->unk1C = NULL;
        if (var_v0_34 < 0) {
            var_v0_34 += 0xFF;
        }
        probe.x = var_v0_34 >> 8;
        var_v0_35 = a0->unk8;
        if (var_v0_35 < 0) {
            var_v0_35 += 0xFF;
        }
        probe.y = var_v0_35 >> 8;
        ux = (u16) probe.x;
        uy = (u16) probe.y;
        sp68 = (void** )0x801E1100;
        temp_s3_3 = sp28->secondary_nodes;
        temp_v0_15 = sp24 - 1;
        sp24 = temp_v0_15;
        if (temp_v0_15 != -1) {
            var_t0_2 = (s16) uy;
            var_t1_3 = (s16) sp38;
            do {
                temp_t5_4 = *sp68;
                sp68 += 1;
                sp2C = temp_t5_4;
                temp_s6_6 = ((Move_UnkNode1*)temp_t5_4)->unk4;
                temp_a1_8 = (s32) ((Move_UnkNode1*)temp_t5_4)->unk34 >> 8;
                temp_a0_17 = (s32) (((Move_UnkNode1*)temp_t5_4)->unk40 << 8) >> 0x10;
                temp_v1_24 = ((Move_UnkNode1*)temp_t5_4)->unk22 + temp_a0_17;
                var_t8 = 0;
                if ((var_t0_2 >= temp_v1_24) && (((((Move_UnkNode1*)temp_t5_4)->unk20 + temp_a0_17) >= var_t0_2))) {
                    temp_a0_18 = ((u8*)temp_s6_6)[6];
                    var_s1_5 = (u8*)((Move_UnkNode1*)temp_t5_4)->unk10 + ((var_t0_2 - temp_v1_24) * temp_a0_18 * 4);
                    if ((s16) temp_a1_8 != 0) {
                        var_s0_10 = temp_a0_18 - 1;
                        if (temp_a0_18 != 0) {
                            do {
                                if (((s16) ux < (((Move_UnkS16*)var_s1_5)->unk0 + (s16) temp_a1_8)) || ((((Move_UnkS16*)var_s1_5)->unk2 + (s16) temp_a1_8) < (s16) ux)) {
                                    var_s1_5 += 4;
                                } else {
                                    var_t8 = 1;
                                    break;
                                }
                            } while (--var_s0_10 != -1);
                        }
                    } else {
                        var_s0_11 = temp_a0_18 - 1;
                        if (temp_a0_18 != 0) {
                            do {
                                if (((s16) ux < ((Move_UnkS16*)var_s1_5)->unk0) || (((Move_UnkS16*)var_s1_5)->unk2 < (s16) ux)) {
                                    var_s1_5 += 4;
                                } else {
                                    var_t8 = 1;
                                    break;
                                }
                            } while (--var_s0_11 != -1);
                        }
                    }
                }
                temp_a1_9 = ((Move_UnkNode1*)sp2C)->unk38;
                temp_a0_19 = ((Move_UnkNode2*)temp_s6_6)->unk4 & 3;
                temp_v1_25 = temp_a1_9 >> 8;
                switch (temp_a0_19) {               /* switch 3; irregular */
                case 0:                             /* switch 3 */
                    var_v1_4 = ((Move_UnkNode2*)temp_s6_6)->unk14 + (s16) temp_v1_25;
                    var_v0_36 = var_v1_4 < var_s7;
                    if (var_v1_4 < var_t1_3) {
                        if (var_t8 != 0) {
                            temp_v0_16 = -(temp_a1_9 + (((Move_UnkNode2*)temp_s6_6)->unk10 << 8));
                            if (temp_v0_16 < a0->unk18) {
                                a0->unk18 = temp_v0_16;
                                var_fp = sp2C;
                            }
                        }
                        if (var_a3 != 0) {
                            temp_v0_17 = ((Move_UnkNode2*)temp_s6_6)->unk10 + (s16) temp_v1_25;
                            if ((var_s2 + 0x14) < temp_v0_17) {
                                var_s2 = temp_v0_17;
                                var_s4_3 = ((Move_UnkNode1*)sp2C)->unk38 + (((Move_UnkNode2*)temp_s6_6)->unk10 << 8);
                                if (var_t8 != 0) {
                                    a0->unk1C = sp2C;
                                }
                            }
                        } else {
                            temp_v1_26 = ((Move_UnkNode2*)temp_s6_6)->unk10 + (s16) temp_v1_25;
                            if (temp_v1_26 >= var_s2) {
                                var_s2 = temp_v1_26;
                                var_s4_3 = ((Move_UnkNode1*)sp2C)->unk38 + (((Move_UnkNode2*)temp_s6_6)->unk10 << 8);
                                if (var_t8 != 0) {
                                    a0->unk1C = sp2C;
                                }
                            }
                        }
                    } else {
                        if (var_v0_36 != 0) {
                            var_s7 = var_v1_4;
                        }
                    }
                    break;
                case 1:                             /* switch 3 */
                    var_v1_4 = ((Move_UnkNode2*)temp_s6_6)->unk14 + (s16) temp_v1_25;
                    if (var_v1_4 < var_t1_3) {
                        temp_s0_6 = func_8005DFAC(sp2C, &probe.x);
                        if (var_t8 != 0) {
                            temp_v1_27 = -(temp_s0_6 << 8);
                            if (temp_v1_27 < a0->unk18) {
                                a0->unk18 = temp_v1_27;
                                var_fp = sp2C;
                            }
                        }
                        if (var_a3 != 0) {
                            var_v0_37 = var_s2 < temp_s0_6;
                            if (temp_s3_3 != NULL) {
                                if (temp_s3_3 == sp2C) {
                                    temp_v0_18 = temp_s0_6 - ((Move_UnkNode2*)((Move_UnkNode1*)sp2C)->unk4)->unk10;
                                    var_s2 += temp_v0_18;
                                    var_s4_3 += temp_v0_18 << 8;
                                } else {
                                    var_s2 = (temp_s0_6 + var_s2) - ((Move_UnkNode2*)((Move_UnkNode1*)temp_s3_3)->unk4)->unk10;
                                    var_s4_3 = var_s2 << 8;
                                    if (var_t8 != 0) {
                                        a0->unk1C = sp2C;
                                    }
                                }
                                a0->unk18 = (s32) -(var_s2 << 8);
                                var_fp = sp2C;
                                var_a3 = 1;
                            } else {
                                var_a3 = 1;
                                if (var_v0_37 != 0) {
                                    var_s2 = temp_s0_6;
                                    var_s4_3 = var_s2 << 8;
                                    if (var_t8 != 0) {
                                        a0->unk1C = sp2C;
                                    }
                                }
                            }
                        } else {
                            var_v0_37 = var_s2 < (temp_s0_6 + 0x14);
                            var_a3 = 1;
                            if (var_v0_37 != 0) {
                                var_s2 = temp_s0_6;
                                var_s4_3 = var_s2 << 8;
                                if (var_t8 != 0) {
                                    a0->unk1C = sp2C;
                                }
                            }
                        }
                    } else {
                        var_v0_36 = var_v1_4 < var_s7;
                        if (var_v0_36 != 0) {
                            var_s7 = var_v1_4;
                        }
                    }
                    break;
                }
                temp_v1_23 = sp24 - 1;
                sp24 = temp_v1_23;
            } while (temp_v1_23 != -1);
        }
        if (a0->unk28 & 0x30000) {
            temp_a0_20 = a0->unk26;
            if (var_s7 < ((s16) sp30 + temp_a0_20)) {
                a0->unk4 = (s32) -((var_s7 - temp_a0_20) << 8);
                var_t5 = sp50 | 0x40;
                sp50 = var_t5;
            } else if (var_s2 >= (s16) sp30) {
                a0->unk4 = (s32) -var_s4_3;
                var_t5 = sp50 | 0x80;
                sp50 = var_t5;
            } else {
                a0->unk4 = (s32) (a0->unk4 + a0->unk10);
            }
        } else if (((s16) sp30 != var_s2) || (var_s4_3 != 0)) {
            a0->unk4 = (s32) -var_s4_3;
            var_t5 = sp50 | 0x20;
            sp50 = var_t5;
        }
        if (var_fp == a0->unk1C) {
            a0->unk20 = (s32) (a0->unk20 | 1);
        } else {
            a0->unk20 = (s32) (a0->unk20 & 0xFFFE);
        }
        var_v0 = sp50;
        return var_v0;
    }
}

/**
 * @brief Classify a linked list of collision nodes against a probe box.
 *
 * Walks @p node's `unk0` chain and tests each node's tile span against the
 * probe rectangle. Nodes that block movement are appended to the array at
 * 0x801E1000 and counted in @p out_hit; nodes that are merely touched are
 * appended to the array at 0x801E1100 and counted in @p out_touch. A node can
 * be both.
 *
 * Per node the span [row_top, row_bot] is clipped to the probe's vertical
 * extent, then a run of `Move_UnkS16` edge pairs (`node->unk10`) is scanned for
 * horizontal overlap. `obj->unk4 & 8` selects a variant that also consults the
 * per-edge flag bytes at `node->unk14`; otherwise a cheaper scan runs, with a
 * separate path when the node has a horizontal offset (`dx`). `((u8*)obj)[4] &
 * 3` then selects the height test: case 0 compares against the node's own
 * height, case 1 asks func_8005DFAC for the interpolated ground height.
 *
 * @param node      Head of the collision node chain; walked directly (the
 *                  parameter is the loop variable).
 * @param probe     Move_Probe box: owning Move_Mover plus centre x/y and half extents.
 * @param out_hit   Receives the number of blocking nodes written to 0x801E1000.
 * @param out_touch Receives the number of touching nodes written to 0x801E1100.
 *
 * @note `result` bit 1 = touching, bit 2 = blocking.
 * @note `obj->unk4` is read as a word for the `& 8` and `& 4` tests but as a
 *       byte for the `& 3` dispatch, matching the original's access widths.
 * @note The flag bytes are read both signed and unsigned from the same address
 *       (`(s8)fl[n] >= 0` versus `fl[n] == 0x7F`); these are deliberately
 *       distinct reads and must not be collapsed into one.
 * @note NOT YET MATCHED - 99.67% (329/332 exact rows, frame and all sp slots
 *       match, instruction count is one short). The single remaining defect is
 *       at +0xE8: the original emits `sra v0,v0,8 / addu t8,v0,zero /
 *       sll v0,v0,16`, keeping `dz` in a separate register from the temporary
 *       the sign-extension reads, while this source fuses them into
 *       `sra t8,v0,8 / sll v0,t8,16`. Declaring `dz` as `s16` does reproduce
 *       the copy but costs rows elsewhere. See working/func_8005DA7C/status.md
 *       for the full probe log and the retired hypothesis classes.
 * @note No decomp.me scratch exists for this function yet.
 */
void func_8005DA7C(Move_Probe* probe, Move_UnkNode1* node, s32* out_hit, s32* out_touch)
{
    Move_UnkNode1** hit_list;
    Move_UnkNode1** touch_list;
    Move_UnkNode2* obj;
    Move_Mover* m;
    Move_UnkS16* pt;
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
    s32 dz;
    s32 zc;
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
        hit_list = (Move_UnkNode1**)0x801E1000;
        touch_list = (Move_UnkNode1**)0x801E1100;
        h = probe->h;
        m = probe->m;
        w = probe->w;
        sy = m->unk28;
        y0 = probe->y - (s16)sy / 2;
        y1 = y0 + sy;
        sx = m->unk24;
        x0 = probe->x - (s16)sx / 2;
        x1 = x0 + sx;
        do
        {
            obj = node->unk4;
            if (node->unk18 != 0)
            {
                dz = (s32)node->unk38 >> 8;
                zc = obj->unk14 + (s16)dz;
                if ((zc == 0) || (zc < w + m->unk26) || (zc < h))
                {
                    dy = (s32)node->unk40 >> 8;
                    dx = (s32)(node->unk34 << 8) >> 16;
                    if (((node->unk1C + dx) < x1) && ((node->unk1E + dx) >= x0))
                    {
                        row_lim = y1;
                        row_top = node->unk22 + (s16)dy;
                        if (row_top < row_lim)
                        {
                            y0v = y0;
                            row_bot = node->unk20 + (s16)dy;
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
                                pt = (Move_UnkS16*)node->unk10 + off;
                                if (obj->unk4 & 8)
                                {
                                    fl = (u8*)node->unk14 + off * 2;
                                    while (--count != -1)
                                    {
                                        if ((pt->unk0 < x1) && (pt->unk2 >= x0))
                                        {
                                            f0 = (s8)fl[0];
                                            if ((f0 >= 0) && (x0 < pt->unk0))
                                            {
                                                result = 2;
                                                break;
                                            }
                                            if (((s8)fl[1] >= 0) && (pt->unk2 < x1 - 1))
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
                                        zc = dx;
                                        if (((pt->unk0 + zc) < x1) && ((pt->unk2 + zc) >= x0))
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
                                        if ((pt->unk0 < x1) && (pt->unk2 >= x0))
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
                                        if ((obj->unk14 + (s16)dz) < h)
                                        {
                                            if (obj->unk4 & 4)
                                            {
                                                result = 2;
                                            }
                                            else
                                            {
                                                if (m->unk28 & 0x30000)
                                                {
                                                    over = w < (obj->unk10 + (s16)dz);
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
                                                    over = (w + 0x10) < (obj->unk10 + (s16)dz);
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
                                        if ((obj->unk14 + (s16)dz) < h)
                                        {
                                            gnd = func_8005DFAC(node, &probe->x);
                                            if (m->unk28 & 0x30000)
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
            node = node->unk0;
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
 * through @p pt parallel to A->B: `hit_x` is that intersection's x coordinate,
 * `along` its distance from C along the edge, and `along / dx_bc` the fraction
 * used to blend the node's two height endpoints h0 (at C) and h1 (at B).
 *
 * The fraction is normally taken from the x axis. When the intersection lands
 * exactly on B's x coordinate that axis carries no usable span, so the y axis
 * is used instead; if the intersection is B itself the answer is h1 directly.
 * The blend is finally clamped back into [min(h0, h1), max(h0, h1)], which is
 * spelled twice because which endpoint is the upper bound depends on their
 * order.
 *
 * @param node Collision node whose surface is being sampled.
 * @param pt Probe position in grid cells: pt[0] is x and pt[1] is y.
 * @return Interpolated surface height, clamped between the node's endpoints.
 *
 * @note Both endpoints are clamped up to zero before use, so a node whose
 *       endpoint resolves below the origin behaves as if it sat on it.
 * @see decomp.me (100%) TODO
 */
s16 func_8005DFAC(Move_UnkNode1* node, s32* pt)
{
    Move_UnkNode2* def;
    s16* tbl;
    s16* vert;
    s16* vert_b;
    s32 h0;
    s32 h1;
    s32 ox;
    s32 oy;
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
    s32 along;
    s32 hit_y;
    s16 height;

    def = node->unk4;
    tbl = g_field_node_angle_table;
    vert = &tbl[def->unkE * 2];
    h0 = (node->unk38 >> 8) + def->unk10;
    h1 = (node->unk3C >> 8) + def->unk12;
    vert_b = &tbl[def->unkC * 2];
    ox = node->unk34 >> 8;
    if (h0 < 0)
    {
        h0 = 0;
    }
    if (h1 < 0)
    {
        h1 = 0;
    }
    ax = vert[0];
    ay = vert[1];
    bx = vert_b[0];
    by = vert_b[1];
    vert = &tbl[def->unkA * 2];
    cy = vert[1];
    dx_ab = ax - bx;
    dy_bc = by - cy;
    cross_xy = dx_ab * dy_bc;
    oy = node->unk40 >> 8;
    cy_world = cy + oy;
    cx = vert[0];
    dx_bc = bx - cx;
    cross_yx = (ay - by) * dx_bc;
    cx_world = cx + ox;
    hit_x = ((((pt[1] - cy_world) * dx_ab) * dx_bc) + (cross_xy * cx_world) - (pt[0] * cross_yx)) / (cross_xy - cross_yx);
    along = hit_x - cx_world;
    hit_y = ((along * dy_bc) / dx_bc) + cy_world;
    if (hit_x == (bx + ox))
    {
        if (hit_y == (by + oy))
        {
            height = h1;
        }
        else
        {
            height = (((hit_y - cy_world) * (h1 - h0)) / dy_bc) + h0;
        }
    }
    else
    {
        height = ((along * (h1 - h0)) / dx_bc) + h0;
    }

    if (h0 < h1)
    {
        if (height > h1)
        {
            height = h1;
        }
        else if (height < h0)
        {
            height = h0;
        }
    }
    else
    {
        if (height > h0)
        {
            height = h0;
        }
        else if (height < h1)
        {
            height = h1;
        }
    }
    return height;
}

/**
 * @brief Resolve the slide direction along one collision edge.
 *
 * Locates boundary edge number @p edge in the node's edge-run list, takes the
 * two angle-table points bounding it, and turns them into a wall angle with
 * ratan2. Indices 0x7E and 0x7F skip the walk entirely and stand for the two
 * screen-aligned world edges, giving a fixed angle of 0x400 and 0 respectively.
 *
 * The wall can be slid along in either direction, so the side whose angle sits
 * nearer @p dir is chosen; a tie means the movement runs straight into the wall
 * and nothing can be resolved. The chosen angle is then merged with the running
 * best @p best: when both deviate from @p dir the same way the larger deviation
 * wins, and a deviation of 0x472 or more (about 100 degrees of the 0x1000-unit
 * circle) counts as blocked.
 *
 * @param def Node definition owning the edge list, or NULL when @p edge is
 *            0x7E or 0x7F.
 * @param edge Edge index within the node, or 0x7E / 0x7F for a world edge.
 * @param dir Desired movement direction, in 0x1000 units per revolution.
 * @param best Slide direction resolved so far: -2 means none yet and -1 means
 *             an earlier edge already blocked the movement.
 * @return The resolved slide direction, @p best when the earlier one still
 *         wins, or -1 when the movement is blocked.
 * @see decomp.me (97.50%, 126/130 exact) TODO
 */
s32 func_8005E1A8(Move_UnkNode2* def, s32 edge, s32 dir, s32 best)
{
    s16* tbl;
    s16* prev;
    s16* ep;
    Move_EdgeRun* run;
    s32 scratch;
    s32 angle;
    s32 dx;
    s32 dy;
    s32 d2;
    s32 mid;
    s32 wrap;
    s32 e1;
    s32 e2;

    if (best == -1)
    {
        return -1;
    }
    prev = NULL;
    if (edge < 0x7E)
    {
        tbl = g_field_node_angle_table;
        for (run = def->runs; (scratch = run->count & 0x7FFF) != 0; run++)
        {
            ep = &tbl[run->index * 2];
            while (--scratch != -1)
            {
                if (prev != NULL)
                {
                    if (edge == 0)
                    {
                        goto found;
                    }
                    edge--;
                }
                prev = ep;
                ep += 2;
            }
        }
        ep = &tbl[def->runs[0].index * 2];
    found:
        dx = ep[0] - prev[0];
        dy = ep[1] - prev[1];
        if (dx == 0)
        {
            angle = 0x400;
        }
        else
        {
            angle = ratan2(dy, dx) & 0x7FF;
        }
    }
    else
    {
        angle = (edge == 0x7E) << 10;
    }

    if (dir < angle)
    {
        scratch = angle - dir;
    }
    else
    {
        scratch = dir - angle;
        if (scratch > 0x800)
        {
            wrap = dir - 0x1000;
            scratch = angle - wrap;
        }
    }
    mid = angle + 0x800;
    if (dir < mid)
    {
        d2 = mid - dir;
        if (d2 > 0x800)
        {
            wrap = angle - 0x800;
            d2 = dir - wrap;
        }
    }
    else
    {
        d2 = dir - mid;
    }
    if (scratch == d2)
    {
        return -1;
    }
    if (d2 < scratch)
    {
        angle += 0x800;
    }
    if (best == -2)
    {
        return angle;
    }

    e2 = best - dir;
    e1 = angle - dir;
    if (e1 > 0x800)
    {
        e1 -= 0x1000;
    }
    else if (e1 < -0x800)
    {
        e1 += 0x1000;
    }
    if (e2 > 0x800)
    {
        e2 -= 0x1000;
    }
    else if (e2 < -0x800)
    {
        e2 += 0x1000;
    }

    if ((e1 >= 0) && (e2 >= 0))
    {
        if (e1 < e2)
        {
            angle = best;
        }
        else if (e1 >= 0x472)
        {
            angle = -1;
        }
    }
    else if ((e1 <= 0) && (e2 <= 0))
    {
        scratch = e1;
        if (e2 < e1)
        {
            angle = best;
        }
        else if (scratch < -0x471)
        {
            angle = -1;
        }
    }
    else
    {
        angle = -1;
    }
    return angle;
}


/**
 * @brief One boundary point of a collision node, as stored in
 *        g_field_node_angle_table.
 * @note Both fields are loaded unsigned by func_8005E3B0 (the asm uses lhu),
 *       which is why this is not the signed Move_UnkS16 pair used for the
 *       min/max edge records at Move_UnkNode1::unk10.
 */
typedef struct Move_EdgePoint {
    /** 0x00 x coordinate. */
    u16 x;
    /** 0x02 y coordinate (scanline before node->unk22 is subtracted). */
    u16 y;
} Move_EdgePoint;

/**
 * @brief One horizontal span on a scanline: the inclusive x range it covers.
 * @note Declared as a union so the struct is 4-byte aligned. A plain
 *       two-s16 struct assignment compiles to lwl/lwr; the original uses
 *       lw/sw, so the whole-span copies in the bubble sort go through
 *       @c word.
 */
typedef union Move_Span {
    struct {
        s16 x0;
        s16 x1;
    } x;
    s32 word;
} Move_Span;

/**
 * @brief Edge attributes for the two ends of the span at the same index.
 * @note Union for the same alignment reason as Move_Span; @c half carries
 *       the 0xFFFF padding value written to unused entries.
 */
typedef union Move_SpanFlags {
    struct {
        /** Attribute of the span's left (x0) end. */
        u8 f0;
        /** Attribute of the span's right (x1) end. */
        u8 f1;
    } f;
    u16 half;
} Move_SpanFlags;

/**
 * @brief Rasterise a collision node's outline into per-scanline span lists.
 *
 * Phase 1 walks the node's edge-run list at @c def->runs and draws every
 * polygon edge with a Bresenham line, appending one Move_Span per scanline
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
 *       node definition. That byte overlaps Move_UnkNode2::unk4, which other
 *       functions read as a word, so it is taken by cast rather than by
 *       resplitting a field they depend on.
 *
 * @note UNMATCHED. The residual is a single register rotation, not a
 *       structural difference: 650 of 874 rows are exact, 206 differ only in
 *       register names, and no structural run is longer than 3 rows. The
 *       target holds @c i in t8 and @c row in t9 where this source gets a3
 *       and a0; because @c i occupies a3, phase 2 cannot use a3 for the
 *       row-stride invariant, which flips the s4/s5 reload pairing in the
 *       epilogue and costs the two extra instructions (a reload of @c node
 *       plus its load-delay nop). Fixing @c i is expected to resolve the rest.
 * @note Measured dead ends, so they are not retried: widening any of the s16
 *       scalars; splitting the merged @c i / @c j / @c n counters; a separate
 *       capacity variable for the pointer arithmetic; removing @c attr2 (-59
 *       exact rows); reordering the epilogue statements; a row carrier in the
 *       closing edge; and swapping the @c dy / @c row statements (-193).
 * @see decomp.me (94.98%, 650/874 exact) TODO
 */
void func_8005E3B0(Move_UnkNode1* node, u8** alloc)
{
    Move_UnkNode2* def;
    s16* table;
    Move_Span* spans;
    Move_SpanFlags* flags;
    u8* counts;
    s32 first_dir;
    s32 edge;
    Move_EdgeRun* run;
    Move_EdgePoint* pt;
    Move_EdgePoint* prev;
    Move_Span* sp_row;
    Move_Span* out_span;
    Move_Span* p;
    Move_Span* a;
    Move_SpanFlags* fl_row;
    Move_SpanFlags* out_flag;
    Move_SpanFlags* save_flags;
    Move_SpanFlags* q;
    Move_SpanFlags* b;
    Move_Span tmp_span;
    Move_SpanFlags tmp_flag;
    s32* src;
    u8* cp;
    s32 rows;
    s32 i;
    s32 j;
    s32 n;
    s32 count;
    s32 last_dir;
    s32 attr;
    s32 attr2;
    u16 nbytes;
    s32 hi;
    u32 prev_hi;
    u16 w;
    u16 y0;
    u16 y1;
    u16 ybase;
    s16 dx;
    s16 dy;
    s16 x;
    s16 sgn;
    s16 ystep;
    s16 row;
    s16 err;
    s16 merge_row;
    s16 merge_row2;

    def = node->unk4;
    rows = node->unk20;
    rows = rows - node->unk22;
    w = ((u8*)def)[6] * 2;
    counts = (u8*)0x1F800000;
    spans = (Move_Span*)*alloc;
    node->unk10 = *alloc;
    nbytes = (rows + 1) * (w * 2);
    node->unk14 = *alloc + nbytes;
    flags = (Move_SpanFlags*)(*alloc + ((rows + 1) * (w * 4)));
    *alloc = *alloc + ((((rows + 1) * (w * 3)) + 2) & ~3);

    cp = counts;
    for (i = rows; i != -1; i--)
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
    i = run->count & 0x7FFF;
    while (i != 0)
    {
        pt = (Move_EdgePoint*)&table[run->index * 2];
        for (i = i - 1; i != -1; i--)
        {
            if (prev != NULL)
            {
                hi = 0;
                if (run->count & 0x8000)
                {
                    hi = prev_hi << 7;
                }
                attr = edge | hi;
                x = pt->x;
                dx = x - prev->x;
                if (dx >= 0)
                {
                    x = prev->x;
                    y0 = prev->y;
                    dy = pt->y;
                    ybase = node->unk22;
                    sgn = 1;
                }
                else
                {
                    dx = -dx;
                    x = pt->x;
                    sgn = -1;
                    y0 = pt->y;
                    dy = prev->y;
                    ybase = node->unk22;
                }
                dy = dy - y0;
                row = y0 - ybase;
                cp = &counts[row];
                sp_row = &spans[row * w];
                fl_row = &flags[row * w];
                if (dy != 0)
                {
                    ystep = 1;
                    if (dy < 0)
                    {
                        dy = -dy;
                        ystep = -1;
                    }
                    attr2 = attr;
                    if ((last_dir == 2) || (((ystep * sgn) + 2) == last_dir))
                    {
                        merge_row = prev->y - node->unk22;
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
                    count = dx + 1;
                    if (dy < dx)
                    {
                        err = -dx;
                        while (count > 0)
                        {
                            n = *cp;
                            if (row == merge_row)
                            {
                                if (sp_row[n - 1].x.x0 > x)
                                {
                                    sp_row[n - 1].x.x0 = x;
                                    fl_row[n - 1].f.f0 = attr2;
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
                                err += dy * 2;
                                count--;
                            } while ((err < 0) && (count > 0));
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
                        err = -dy;
                        for (j = dy; j != -1; j--)
                        {
                            n = *cp;
                            prev_hi = merge_row;
                            if (row == prev_hi)
                            {
                                if (x < sp_row[n - 1].x.x0)
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
                                fl_row[n].f.f1 = attr2;
                                fl_row[n].f.f0 = attr2;
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
                                err -= dy * 2;
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
        i = run->count & 0x7FFF;
    }

    /* closing edge: back to the first run's first point */
    hi = 0;
    run = def->runs;
    if (run->count & 0x8000)
    {
        hi = prev_hi << 7;
    }
    pt = (Move_EdgePoint*)&table[run->index * 2];
    attr = edge | hi;
    x = pt->x;
    dx = x - prev->x;
    if (dx >= 0)
    {
        x = prev->x;
        y0 = prev->y;
        y1 = pt->y;
        ybase = node->unk22;
        sgn = 1;
    }
    else
    {
        dx = -dx;
        x = pt->x;
        sgn = -1;
        y0 = pt->y;
        y1 = prev->y;
        ybase = node->unk22;
    }
    dy = y1 - y0;
    row = y0 - ybase;
    cp = &counts[row];
    sp_row = &spans[row * w];
    fl_row = &flags[row * w];
    if (dy != 0)
    {
        ystep = 1;
        if (dy < 0)
        {
            dy = -dy;
            ystep = -1;
        }
        if ((last_dir == 2) || (((ystep * sgn) + 2) == last_dir))
        {
            merge_row = prev->y - node->unk22;
        }
        else
        {
            merge_row = -1;
        }
        if (((ystep * sgn) + 2) == first_dir)
        {
            merge_row2 = pt->y - node->unk22;
        }
        else
        {
            merge_row2 = -1;
        }
        count = dx + 1;
        if (dy < dx)
        {
            err = -dx;
            while (count > 0)
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
                    err += dy * 2;
                    count--;
                } while ((err < 0) && (count > 0));
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
                row += ystep;
                cp += ystep;
                err -= dx * 2;
                sp_row += ystep * w;
                fl_row += ystep * w;
            }
        }
        else
        {
            err = -dy;
            for (j = dy; j != -1; j--)
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
                    err -= dy * 2;
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
            if ((s16)(x + dx) >= sp_row[n - 1].x.x1)
            {
                sp_row[n - 1].x.x1 = x + dx;
                fl_row[n - 1].f.f1 = attr | 0x7F;
            }
        }
    }

    out_flag = flags;
    out_span = spans;
    save_flags = flags;
    for (i = node->unk20 - node->unk22; i != -1; i--)
    {
        p = spans;
        q = flags;
        for (j = *counts - 2; j != -1; j--)
        {
            n = j + 1;
            a = &p[n];
            b = &q[n];
            for (n = j; n != -1; n--)
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
        for (j = (*counts >> 1) - 1; j != -1; j--)
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

        for (j = ((w - *counts) / 2) - 1; j != -1; j--)
        {
            out_span->word = 0x80007F00;
            out_span++;
        }
        for (j = ((w - *counts) / 2) - 1; j != -1; j--)
        {
            out_flag->half = 0xFFFF;
            out_flag++;
        }

        spans += w;
        flags += w;
        counts++;
    }

    i = w * (((node->unk20 - node->unk22) + 2) / 2);
    src = (s32*)save_flags;
    flags = (Move_SpanFlags*)node->unk14;
    for (i = i - 1; i != -1; i--)
    {
        *(s32*)flags = *src;
        src++;
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
 * whose @c seen is 3. The surviving ids are sorted into descending order in
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
 * @note UNMATCHED. Instruction count (281), frame (-0x68) and every stack slot
 *       match; the residual is one register rotation plus a scheduling gap in
 *       the arithmetic tail. The target holds the scan index in a2, the write
 *       cursor in a0 and the scene pointer in t5, where this source gets a0, a2
 *       and t4. The scan index is the highest-priority allocno here, so GCC 2.8
 *       global.c hands it a0 first; the target instead has a0 held by the write
 *       cursor. See working/func_8005F158/status.md for the measured dead ends
 *       - in particular do NOT reorder `*alloc += total` past
 *       `scene->unk30 = *alloc`, which scores well but is semantically wrong
 *       (the target stores the post-increment value).
 * @see decomp.me (90.87%, 173/281 exact) TODO
 */
void func_8005F158(s32* alloc)
{
    FieldGroupEntry list[20];
    FieldScene* scene;
    FieldNode* node;
    FieldNodeDef* def;
    FieldGroupEntry* out;
    FieldGroupEntry* w2;
    FieldGroupEntry* p;
    FieldSceneHeader* header;
    s32 i;
    s32 j;
    s32 fresh;
    s32 has_pair;
    s32 kind;
    s32 k;
    s32 width;
    s32 height;
    s32 unit;
    s32 shift;
    s32 tw;
    s32 th;
    s32 cols;
    s32 rows;
    s32 area;
    s32 stride;
    s32 total;
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
    out = list;
    do
    {
        def = node->def;
        if ((node->unk18 != 0) && !(def->flags & 4))
        {
            i = count - 1;
            switch ((u8)def->flags & 3)
            {
            case 0:
                fresh = 1;
                for (; i != -1; i--)
                {
                    if (list[i].id == def->base_x)
                    {
                        fresh = 0;
                        list[i].seen |= 1;
                        break;
                    }
                }
                if (fresh != 0)
                {
                    out->seen = 1;
                    out->id = def->base_x;
                    out++;
                    if (count >= 20)
                    {
                        goto overflow;
                    }
                    count++;
                }
                break;

            case 1:
                has_pair = 1;
                fresh = 1;
                for (; i != -1; i--)
                {
                    if (list[i].id == def->base_x)
                    {
                        fresh = 0;
                        list[i].seen |= 2;
                        break;
                    }
                }
                if (fresh != 0)
                {
                    kind = 2;
                    if (def->base_x != 0)
                    {
                        out->id = def->base_x;
                    }
                    else
                    {
                        kind = 3;
                        out->id = 0;
                    }
                    out->seen = kind;
                    out++;
                    if (count >= 20)
                    {
                        goto overflow;
                    }
                    count++;
                }

                fresh = 1;
                for (i = count - 1; i != -1; i--)
                {
                    if (list[i].id == def->base_y)
                    {
                        fresh = 0;
                        list[i].seen |= 2;
                        break;
                    }
                }
                if (fresh != 0)
                {
                    kind = 2;
                    if (def->base_y != 0)
                    {
                        out->id = def->base_y;
                    }
                    else
                    {
                        kind = 3;
                        out->id = 0;
                    }
                    out->seen = kind;
                    out++;
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
        j = 0;
        i = count - 1;
        count = 0;
        if (i != -1)
        {
            p = list;
            w2 = list;
            do
            {
                if (p->seen == 3)
                {
                    if (j != count)
                    {
                        w2->id = p->id;
                    }
                    w2++;
                    count++;
                }
                p++;
                i--;
                j++;
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
            for (j = k - 1; j != -1; j--)
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
    unit = 4;
    shift = 2;
    if ((((width + 3) >> 2) * ((height + 7) >> 3) * (s32)count) >= 0x4001)
    {
        unit = 8;
        shift = 3;
    }
    tw = ((width + unit) - 1) >> shift;
    cols = tw + 4;
    th = ((height + (unit * 2)) - 1) >> (shift + 1);
    rows = th + 4;
    area = cols * rows;
    scene->unk44 = area;
    scene->unk40 = unit;
    scene->unk41 = count;
    scene->unk46 = cols;
    scene->unk48 = rows;
    scene->unk2C = *alloc;
    stride = ((u32)(tw + 0x23) >> 5) * th;
    *alloc += ((area * count) + 3) & ~3;
    stride = stride * 2;
    scene->unk42 = stride;
    scene->unk28 = *alloc;
    total = stride * (count * 4);
    *alloc += total;
    scene->unk30 = *alloc;
    func_8005F5BC(alloc, 0, stride, total);
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
} NodeEnt;

/**
 * @brief A horizontal span (inclusive x range) in tile columns.
 */
typedef struct
{
    s16 x0;
    s16 x1;
} Span;

/**
 * @brief One node's in-progress walk over its span table.
 *
 * @note 12 bytes; @c skip counts sub-rows before the node starts contributing.
 */
typedef struct
{
    u16* src;   /* 0x00 cursor into FieldNode::spans */
    s16 count;  /* 0x04 sub-rows remaining */
    s16 step;   /* 0x06 span pairs per sub-row */
    s16 skip;   /* 0x08 sub-rows still to skip */
    s16 pad;
} Run;

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
 * @param arg0 Unused; the target never reads a0.
 * @param clip Optional clipping node. When NULL every tile row of the group is
 *             emitted; otherwise only the rows the node covers are, and its
 *             definition is tested against the group id first.
 *
 * @note UNMATCHED (85.92%, 414/874 exact). Structure and semantics are
 *       believed correct; the residual is a register-allocation cascade. The
 *       target keeps @c base in @c s5 and needs no reload temps, giving a
 *       -0x850 frame; this source spills @c base to 0x820 plus six reload
 *       temps, giving -0x868. Every sp offset is therefore shifted, which is
 *       what most of the 403 argdiff rows are. The 14 named stack slots
 *       already match the target's order exactly - do NOT reorder the
 *       declarations. See working/func_8005F5BC/status.md for the measured
 *       dead ends (volatile counters, shared-tail goto in the clip test, rp
 *       address-computation spellings, base as s32/u16 - all measured
 *       negative or inert).
 * @note @c zero_v is required to match: sourcing @c nrun's zero from a
 *       function-scope variable is worth +47 exact rows over a literal 0.
 * @see decomp.me (85.92%) TODO
 */
void func_8005F5BC(s32 arg0, FieldNode* clip)
{
    Span cur[128];
    Span acc[128];
    NodeEnt list[50];
    Run runs[50];
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
    Run* runs_base;
    u32* out;
    u32 ncur;
    u32 nacc;
    u32 nout;
    s32 nrun;
    FieldNode* nd;
    FieldNode* ent;
    FieldNodeDef* def;
    NodeEnt* p;
    NodeEnt* p2;
    Run* rp;
    Run* rq;
    Span* sp1;
    Span* sp2;
    Span* sc0;
    Span* sc1;
    Span* prev;
    Span* dst;
    u32* wp;
    u32* wq;
    u16* src;
    s32 i;
    s32 k;
    s32 n;
    s32 hit;
    s32 fresh;
    s32 lo;
    s32 hi;
    s32 lo2;
    s32 hi2;
    s32 w0;
    s32 w1;
    s32 word;
    s32 zero_v;
    s32 sbase;
    s32 m0;
    s32 m1;
    s32 lead;
    s32 tail;
    s32 wlead;
    s32 wtail;
    s16 base;
    s16 key;
    u16 id;
    u16 tmp;
    u16 x0;
    u16 x1;
    FieldNode* swap;

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
                key = list[i].key;
                for (k = i - 1; k != -1; k--)
                {
                    if ((s16)key < list[k].key)
                    {
                        tmp = list[k].key;
                        list[k].key = key;
                        key = tmp;
                        list[i].key = tmp;
                        swap = list[k].node;
                        list[k].node = list[i].node;
                        list[i].node = swap;
                    }
                }
                i--;
            } while (i != 0);
        }
    }

    tile = scene->unk40;
    if (tile == 4)
    {
        shift1 = 3;
        shift0 = 2;
    }
    else
    {
        shift0 = 3;
        shift1 = 4;
    }
    group = 0;
    tile2 = tile * 2;
    words = (scene->unk46 + 0x1F) >> 5;
    if (scene->unk41 == 0)
    {
        return;
    }

    runs_base = runs;
    do
    {
        id = scene->unk4A[group];
        base = 0;
        if (clip == NULL)
        {
            rows = scene->unk48 - 4;
        }
        else
        {
            def = clip->def;
            saved = out + (words * ((scene->unk48 - 4) * 2));
            hit = 0;
            if (def->flags & 4)
            {
                hit = (s16)id >= def->id_min;
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
                            hit = 1;
                        }
                    }
                }
                else if ((s16)id < hi)
                {
                    if (((s16)id < def->id_min) == 0)
                    {
                        hit = 1;
                    }
                }
            }
            else
            {
                lo2 = def->base_x;
                if ((s16)id < lo2)
                {
                    if (((s16)id < def->id_min) == 0)
                    {
                        hit = 1;
                    }
                }
            }

            if (hit != 0)
            {
                base = 0;
                if (clip->row_start >= 0)
                {
                    base = (u16)clip->row_start & -tile2;
                    n = scene->unk48;
                    k = clip->row_end >> shift1;
                    if (k >= (n - 4))
                    {
                        rows = n - (((s16)base >> shift1) + 4);
                    }
                    else
                    {
                        rows = (k - ((s16)base >> shift1)) + 1;
                    }
                }
                else
                {
                    k = clip->row_end >> shift1;
                    rows = scene->unk48 - 4;
                    if (k < rows)
                    {
                        rows = k + 1;
                    }
                }
                out = out + (words * (((s16)base >> shift1) * 2));
            }
            else
            {
                group++;
                out = saved;
                continue;
            }
        }

        nrun = zero_v;
        j = 0;
        rows = rows - 1;
        if (rows != -1)
        {
            p2 = list;
            do
            {
                if (j < node_count)
                {
                    sbase = (s16)base;
                    n = sbase + tile2;
                    if (p2->key < n)
                    {
                        rp = &runs_base[nrun];
                        do
                        {
                            ent = p2->node;
                            def = ent->def;
                            hit = 0;
                            if (def->flags & 4)
                            {
                                hit = (s16)id >= def->id_min;
                            }
                            else
                            {
                                if ((def->flags & 3) == 1)
                                {
                                    lo2 = def->base_x;
                                    hi2 = def->base_y;
                                    fresh = (s16)id < lo2;
                                    if (lo2 >= hi2)
                                    {
                                        fresh = (s16)id < hi2;
                                    }
                                }
                                else
                                {
                                    fresh = (s16)id < def->base_x;
                                }
                                if ((fresh != 0) && ((s16)id >= def->id_min))
                                {
                                    hit = 1;
                                }
                            }
                            if ((hit != 0) && (ent->row_end >= sbase))
                            {
                                k = ent->row_start;
                                if (sbase >= k)
                                {
                                    rp->src = ent->spans + ((sbase - k) * FIELD_NODE_DEF_ROWS(ent->def) * 2);
                                    rp->skip = 0;
                                    rp->count = ((u16)ent->row_end - (s16)base) + 1;
                                }
                                else
                                {
                                    rp->src = ent->spans;
                                    rp->count = ((u16)ent->row_end - (u16)ent->row_start) + 1;
                                    rp->skip = (u16)ent->row_start - (s16)base;
                                }
                                nrun++;
                                rp->step = FIELD_NODE_DEF_ROWS(ent->def);
                                rp++;
                            }
                            p2++;
                            j++;
                        } while ((j < node_count) && (list[j].key < n));
                    }
                }

                wp = out;
                wp[1] = 3;
                i = words - 2;
                out[0] = 3;
                if (i != -1)
                {
                    do
                    {
                        wp += 2;
                        i--;
                        wp[1] = 0;
                        wp[0] = 0;
                    } while (i != -1);
                }

                k = (scene->unk46 - 1) & 0x1F;
                if (k == 0)
                {
                    w0 = wp[-2] | 0x80000000;
                    w1 = wp[0] | 1;
                    wp[-1] = w0;
                    wp[-2] = w0;
                    wp[1] = w1;
                    wp[0] = w1;
                }
                else
                {
                    m0 = 1 << k;
                    w0 = wp[0] | m0 | (m0 >> 1);
                    wp[1] = w0;
                    wp[0] = w0;
                }

                if (nrun != 0)
                {
                    i = tile2 - 1;
                    if (i != -1)
                    {
                        do
                        {
                            k = nrun - 1;
                            ncur = 0;
                            if (k != -1)
                            {
                                rq = &runs_base[k];
                                rp = &runs_base[nrun];
                                do
                                {
                                    if (rq->skip == 0)
                                    {
                                        src = rq->src;
                                        n = rq->step - 1;
                                        rq->count = rq->count - 1;
                                        if (n != -1)
                                        {
                                            do
                                            {
                                                x0 = src[0];
                                                x1 = src[1];
                                                src += 2;
                                                if ((s16)x1 >= (s16)x0)
                                                {
                                                    sp1 = cur;
                                                    m0 = ncur - 1;
                                                    fresh = 1;
                                                    if (m0 != -1)
                                                    {
                                                        do
                                                        {
                                                            if ((((s16)x1 + 1) >= sp1->x0) && ((sp1->x1 + 1) >= (s16)x0))
                                                            {
                                                                if ((s16)x0 < sp1->x0)
                                                                {
                                                                    sp1->x0 = x0;
                                                                }
                                                                fresh = 0;
                                                                if (sp1->x1 < (s16)x1)
                                                                {
                                                                    sp1->x1 = x1;
                                                                }
                                                                break;
                                                            }
                                                            sp1++;
                                                            m0--;
                                                        } while (m0 != -1);
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
                                            } while (n != -1);
                                        }
                                        if (rq->count == 0)
                                        {
                                            nrun--;
                                            rp--;
                                            if (k != nrun)
                                            {
                                                rq->src = rp->src;
                                                rq->count = rp->count;
                                                rq->step = rp->step;
                                            }
                                        }
                                        else
                                        {
                                            rq->src = src;
                                        }
                                    }
                                    else
                                    {
                                        rq->skip = rq->skip - 1;
                                    }
                                    k--;
                                    rq--;
                                } while (k != -1);
                            }

                            k = ncur - 1;
                            sp1 = cur;
                            if (k != -1)
                            {
                                do
                                {
                                    n = k - 1;
                                    x0 = sp1->x0;
                                    x1 = sp1->x1;
                                    sp2 = sp1 + 1;
                                    if (n != -1)
                                    {
                                        do
                                        {
                                            if ((((s16)x1 + 1) >= sp2->x0) && ((sp2->x1 + 1) >= (s16)x0))
                                            {
                                                ncur--;
                                                k--;
                                                if (sp2->x0 >= (s16)x0)
                                                {
                                                    if ((s16)x1 < sp2->x1)
                                                    {
                                                        goto grow;
                                                    }
                                                    if (n != 0)
                                                    {
                                                        *(s32*)sp2 = *(s32*)&sp2[n];
                                                    }
                                                }
                                                else
                                                {
                                                    x0 = sp2->x0;
                                                grow:
                                                    n_sp = (s16)x1 < sp2->x1;
                                                    if (n_sp)
                                                    {
                                                        x1 = sp2->x1;
                                                    }
                                                    sp1->x0 = x0;
                                                    sp1->x1 = x1;
                                                    if (n != 0)
                                                    {
                                                        *(s32*)sp2 = *(s32*)&sp2[n];
                                                    }
                                                    n = k;
                                                    sp2 = sp1 + 1;
                                                }
                                            }
                                            else
                                            {
                                                sp2++;
                                            }
                                            n--;
                                        } while (n != -1);
                                    }
                                    k--;
                                    sp1++;
                                } while (k != -1);
                            }

                            if (i == (tile2 - 1))
                            {
                                sc0 = cur;
                                sc1 = acc;
                                dst = (Span*)0x1F800000;
                                nacc = ncur;
                                n_sp = nacc;
                                n = nacc - 1;
                                if (n != -1)
                                {
                                    do
                                    {
                                        word = *(s32*)sc0;
                                        sc0++;
                                        n--;
                                        *(s32*)sc1 = word;
                                        sc1++;
                                        *(s32*)dst = word;
                                        dst++;
                                    } while (n != -1);
                                }
                            }
                            else
                            {
                                sc0 = cur;
                                if (!(i & 1))
                                {
                                    prev = (Span*)0x1F800000;
                                    dst = (Span*)0x1F800200;
                                }
                                else
                                {
                                    prev = (Span*)0x1F800200;
                                    dst = (Span*)0x1F800000;
                                }
                                k = ncur - 1;
                                nout = 0;
                                if (k != -1)
                                {
                                    do
                                    {
                                        x0 = sc0->x0;
                                        x1 = sc0->x1;
                                        n = n_sp - 1;
                                        sp2 = prev;
                                        if (n != -1)
                                        {
                                            do
                                            {
                                                if (((s16)x1 >= sp2->x0) && (sp2->x1 >= (s16)x0))
                                                {
                                                    if (nout >= 0x80)
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
                                                    nout++;
                                                    dst++;
                                                }
                                                n--;
                                                sp2++;
                                            } while (n != -1);
                                        }

                                        wtail = 0;
                                        sp1 = acc;
                                        n = nacc - 1;
                                        fresh = 1;
                                        if (n != -1)
                                        {
                                            do
                                            {
                                                if ((((s16)x1 + 1) >= sp1->x0) && ((sp1->x1 + 1) >= (s16)x0))
                                                {
                                                    if ((s16)x0 < sp1->x0)
                                                    {
                                                        sp1->x0 = x0;
                                                    }
                                                    fresh = wtail;
                                                    if (sp1->x1 < (s16)x1)
                                                    {
                                                        sp1->x1 = x1;
                                                    }
                                                    break;
                                                }
                                                sp1++;
                                                n--;
                                            } while (n != -1);
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
                                            sp1->x0 = x0;
                                            sp1->x1 = x1;
                                        }
                                        k--;
                                        sc0++;
                                    } while (k != -1);
                                }
                                n_sp = nout;
                            }
                            i--;
                        } while (i != -1);
                    }

                    k = n_sp - 1;
                    sp2 = (Span*)0x1F800200;
                    if (k != -1)
                    {
                        do
                        {
                            lead = (((sp2->x0 + tile) - 1) >> shift0) + 2;
                            tail = ((sp2->x1 + 1) >> shift0) + 1;
                            wlead = lead >> 5;
                            if (tail >= lead)
                            {
                                wtail = tail >> 5;
                                wq = out + (wlead * 2);
                                m0 = -1 << (lead & 0x1F);
                                m1 = (u32)-1 >> (0x1F - (tail & 0x1F));
                                if (wlead != wtail)
                                {
                                    n = (wtail - wlead) - 2;
                                    wq[0] |= m0;
                                    wq += 2;
                                    if (n != -1)
                                    {
                                        do
                                        {
                                            wq[0] = -1;
                                            n--;
                                            wq += 2;
                                        } while (n != -1);
                                    }
                                    wq[0] |= m1;
                                }
                                else
                                {
                                    wq[0] |= m0 & m1;
                                }
                            }
                            k--;
                            sp2++;
                        } while (k != -1);
                    }

                    k = nacc - 1;
                    sp1 = acc;
                    if (k != -1)
                    {
                        do
                        {
                            lead = (sp1->x0 >> shift0) + 2;
                            tail = (sp1->x1 >> shift0) + 2;
                            wlead = lead >> 5;
                            wtail = tail >> 5;
                            m0 = -1 << (lead & 0x1F);
                            m1 = (u32)-1 >> (0x1F - (tail & 0x1F));
                            wq = out + (wlead * 2);
                            if (wlead != wtail)
                            {
                                wp = wq + 3;
                                n = (wtail - wlead) - 2;
                                wq[1] |= m0;
                                if (n != -1)
                                {
                                    do
                                    {
                                        wp[0] = -1;
                                        n--;
                                        wp += 2;
                                    } while (n != -1);
                                }
                                wp[0] |= m1;
                            }
                            else
                            {
                                wq[1] |= m0 & m1;
                            }
                            k--;
                            sp1++;
                        } while (k != -1);
                    }
                }

                out += words * 2;
                base += tile2;
                rows--;
            } while (rows != -1);
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
 * @see decomp.me (88%) TODO
 */
extern u32 D_1F800008;

void func_80060364(s32 arg0, s32 arg1)
{
    FieldScene *scene;
    u32 sp4;
    s32 sp8;
    s32 spC;
    s32 sp10;
    s32 sp14;
    s32 sp18;
    u32 sp1C;
    s32 sp20;
    s32 sp24;
    s32 sp28;
    s32 sp30;
    s32 stride4;
    u16 temp_v1;
    u8 temp_s6;
    s32 temp_s6_2;
    s32 temp_s6_3;
    s32 temp_s6_4;
    s32 temp_s6_5;
    s32 temp_s3;
    s32 temp_s1;
    s32 var_s1;
    u8 *var_t7;
    u32 var_a1;
    u32 var_t9;
    u32 var_fp;
    s32 var_a2;
    s32 var_t8;
    s32 var_a2_2;
    s32 var_a2_3;
    s32 var_s4;
    s32 temp_v0;
    u32 temp_t2;
    u32 temp_t0;
    s32 var_t8_2;
    u32 var_t6;
    u32 var_t3;
    u32 temp_a0;
    u32 var_t1;
    u32 temp_a0_2;
    u32 temp_a0_3;
    u32 temp_v0_2;
    u32 var_a0;
    s32 var_a2_4;
    u32 var_a3;
    u32 var_t4;
    u32 var_t5;
    s32 var_t0;
    u32 var_s0;
    u32 var_t3_2;
    u32 var_a3_2;
    u32 var_t3_3;
    u32 var_t1_2;
    s32 var_a2_5;
    u32 temp_a0_4;
    u32 temp_v0_3;
    u32 temp_v1_2;
    u8 var_v1;
    u32 temp_v0_4;
    u32 temp_v1_3;
    u32 var_t3_4;
    u32 var_t1_3;
    u8 var_v1_2;
    u8 var_v1_3;
    u32 temp_t2_2;
    u32 temp_t0_2;
    u32 temp_a0_5;
    u32 var_v0_2;
    u32 temp_a0_6;
    u32 temp_a0_7;
    u32 var_a3_3;
    s32 var_a2_6;
    u32 var_a0_2;
    s32 var_a2_7;
    u32 var_a3_4;
    u32 temp_a0_8;
    u32 temp_v1_4;
    u32 var_a0_3;
    s32 var_a2_8;
    s32 var_t8_3;
    s32 var_a2_9;
    s32 var_t8_4;
    s32 var_a2_10;
    s32 var_a2_11;

    scene = g_field_scene.scene;
    temp_v1 = (u16) scene->unk46;
    spC = ((s32) (temp_v1 + 0x1F) >> 5) * 2;
    sp10 = (((s32) (temp_v1 - 3) >> 5) + 1) * 3;
    temp_s3 = arg0 - 1;
    if (arg1 >= 3)
    {
        var_a1 = 0x1F800000;
        var_t9 = 0x1F800000;
        var_fp = (sp10 * arg1 * 4) + 0x1F800000;
    }
    else
    {
        var_a1 = 0;
        var_t9 = 0;
        var_fp = 0;
    }
    sp4 = (u32) scene->unk28;
    temp_s6 = scene->unk41;
    sp8 = (s32) temp_s6;
    var_t7 = (u8 *) scene->unk2C;
    temp_s6_2 = temp_s6 - 1;
    sp8 = temp_s6_2;
    if (temp_s6_2 != -1)
    {
        temp_s6_3 = arg0 - 1;
        sp1C = temp_s6_3;
        sp20 = temp_s6_3 * 4;
        sp18 = arg0 - 6;
        do
        {
            var_a2 = (4 - (s32) var_t7) & 3;
            var_t8 = (u16) scene->unk46 * 2;
            if (var_a2 != 0)
            {
loop_6:
                if (var_t8 != 0)
                {
                    *var_t7 = -1;
                    var_t7 += 1;
                    var_a2 -= 1;
                    var_t8 -= 1;
                    if (var_a2 != 0)
                    {
                        goto loop_6;
                    }
                }
            }
            var_a2_2 = (var_t8 >> 2) - 1;
            if (var_a2_2 != -1)
            {
                do
                {
                    *(s32 *) var_t7 = -1;
                    var_a2_2 -= 1;
                    var_t7 += 4;
                } while (var_a2_2 != -1);
            }
            var_a2_3 = (var_t8 & 3) - 1;
            if (var_a2_3 != -1)
            {
                do
                {
                    *var_t7 = -1;
                    var_a2_3 -= 1;
                    var_t7 += 1;
                } while (var_a2_3 != -1);
            }
            var_s4 = 0;
            temp_v0 = ((u16) scene->unk48 - arg1) - 4;
            sp14 = temp_v0;
            if (temp_v0 != -1)
            {
                stride4 = sp10 * 4;
                sp24 = 0x20 - temp_s3;
                sp28 = 0x21 - temp_s3;
                sp30 = stride4 * arg1;
                do
                {
                    temp_s1 = sp4;
                    sp4 = temp_s1 + (spC * 4);
                    temp_t2 = *(u32 *) (temp_s1 + 0);
                    temp_t0 = *(u32 *) (temp_s1 + 4);
                    var_t8_2 = ((u16) scene->unk46 - 1) - arg0;
                    temp_s1 += 8;
                    switch (sp1C)
                    {
                        default:
                            var_t3 = temp_t0 | (temp_t0 >> temp_s3);
                            var_t6 = (temp_t0 >> 1) | (temp_t0 >> 2);
                            var_a3 = var_t6;
                            temp_a0_3 = temp_t2 | (temp_t2 >> 1);
                            temp_v0_2 = temp_a0_3 >> 2;
                            var_t1 = temp_a0_3 | temp_v0_2;
                            var_a0 = temp_v0_2;
                            var_a2_4 = sp18 >> 1;
                            do
                            {
                                var_a3 = var_a3 >> 2;
                                var_t6 |= var_a3;
                                var_a0 = var_a0 >> 2;
                                var_a2_4 -= 1;
                                var_t1 |= var_a0;
                            } while (var_a2_4 != -1);
                            if (arg0 & 1)
                            {
                                var_t6 |= temp_t0 >> (arg0 - 2);
                                var_t1 |= temp_t2 >> temp_s3;
                            }
                            goto block_25;
                        case 4:
                            var_t6 = (temp_t0 >> 1) | (temp_t0 >> 2) | (temp_t0 >> 3);
                            var_t3 = temp_t0 | (temp_t0 >> 4);
                            temp_a0 = temp_t2 | (temp_t2 >> 1);
                            var_t1 = temp_a0 | (temp_a0 >> 2) | (temp_t2 >> 4);
                            goto block_25;
                        case 3:
                            var_t6 = (temp_t0 >> 1) | (temp_t0 >> 2);
                            var_t3 = temp_t0 | (temp_t0 >> 3);
                            temp_a0_2 = temp_t2 | (temp_t2 >> 1);
                            var_t1 = temp_a0_2 | (temp_a0_2 >> 2);
                            goto block_25;
                        case 2:
                            var_t6 = temp_t0 >> 1;
                            var_t3 = temp_t0 | (temp_t0 >> 2);
                            var_t1 = temp_t2 | (temp_t2 >> 1) | (temp_t2 >> 2);
                            goto block_25;
                        case 1:
                            var_t6 = 0;
                            var_t3 = temp_t0 | (temp_t0 >> 1);
                            var_t1 = temp_t2 | (temp_t2 >> 1);
block_25:
                            var_t4 = temp_t0;
                            var_t5 = temp_t2;
                            break;
                        case 0:
                            var_t6 = 0;
                            var_t3 = temp_t0;
                            var_t1 = temp_t2;
                            var_t4 = 0;
                            var_t5 = 0;
                            break;
                    }
                    var_t0 = 0x20 - temp_s3;
                    if (arg1 != 1)
                    {
                        if (arg1 == 2)
                        {
                            if (!(var_s4 & 1))
                            {
                                var_a1 = (u32) &D_1F800008;
                                var_t9 = stride4 + (u32) &D_1F800008;
                            }
                            else
                            {
                                var_a1 = (u32) &D_1F800008 + stride4;
                                var_t9 = (u32) &D_1F800008;
                            }
                        }
                        else
                        {
                            if (var_a1 >= var_fp)
                            {
                                var_a1 -= sp30;
                            }
                            if (var_t9 >= var_fp)
                            {
                                var_t9 -= sp30;
                            }
                        }
                    }
                    if (var_t8_2 != 0)
                    {
                        var_s0 = var_a1;
loop_38:
                        if (var_t0 < var_t8_2)
                        {
                            var_t8_2 -= var_t0;
                        }
                        else
                        {
                            var_t0 = var_t8_2;
                            var_t8_2 = 0;
                        }
                        var_t3_2 = var_t3 | var_t6;
                        switch (arg1)
                        {
                        case 1:
                            do
                            {
                                if (var_t3_2 & 1)
                                {
                                    var_v1_3 = 1;
                                    if (var_t1 & 1)
                                    {
                                        var_v1_3 = -1;
                                    }
                                }
                                else
                                {
                                    var_v1_3 = 0;
                                }
                                *var_t7 = var_v1_3;
                                var_t7 += 1;
                                var_t3_2 = var_t3_2 >> 1;
                                var_t0 -= 1;
                                var_t1 = var_t1 >> 1;
                            } while (var_t0 != 0);
                            break;
                        case 2:
                            *(s32 *) var_a1 = var_t1;
                            *(s32 *) (var_s0 - 4) = var_t3_2;
                            var_s0 += 0xC;
                            var_a1 += 0xC;
                            if (var_s4 != 0)
                            {
                                temp_v0_4 = *(u32 *) (var_t9 + 4);
                                temp_v1_3 = *(u32 *) (var_t9 + 0);
                                var_t9 += 0xC;
                                var_t3_4 = var_t3_2 | temp_v0_4;
                                var_t1_3 = var_t1 | temp_v1_3;
                                do
                                {
                                    if (var_t3_4 & 1)
                                    {
                                        var_v1_2 = 1;
                                        if (var_t1_3 & 1)
                                        {
                                            var_v1_2 = -1;
                                        }
                                    }
                                    else
                                    {
                                        var_v1_2 = 0;
                                    }
                                    *var_t7 = var_v1_2;
                                    var_t7 += 1;
                                    var_t3_4 = var_t3_4 >> 1;
                                    var_t0 -= 1;
                                    var_t1_3 = var_t1_3 >> 1;
                                } while (var_t0 != 0);
                            }
                            break;
                        default:
                            *(s32 *) var_a1 = var_t1;
                            *(s32 *) (var_s0 - 4) = var_t3_2;
                            *(s32 *) (var_s0 + 0) = var_t6;
                            var_s0 += 0xC;
                            var_a1 += 0xC;
                            if (var_s4 >= (arg1 - 1))
                            {
                                var_a3_2 = var_t9 + stride4;
                                var_t3_3 = var_t3_2 | *(u32 *) (var_t9 + 4);
                                var_t1_2 = var_t1 | *(u32 *) (var_t9 + 0);
                                if (var_a3_2 >= var_fp)
                                {
                                    var_a3_2 -= sp30;
                                }
                                var_a2_5 = arg1 - 2;
                                do
                                {
                                    temp_a0_4 = *(u32 *) (var_a3_2 + 4);
                                    temp_v0_3 = *(u32 *) (var_a3_2 + 0);
                                    temp_v1_2 = *(u32 *) (var_a3_2 + 8);
                                    var_a3_2 = var_a3_2 + stride4;
                                    var_t1_2 |= temp_v0_3 | temp_v1_2;
                                    var_t3_3 |= temp_a0_4;
                                    if (var_a3_2 >= var_fp)
                                    {
                                        var_a3_2 -= sp30;
                                    }
                                    var_a2_5 -= 1;
                                } while (var_a2_5 != 0);
                                var_t9 += 0xC;
                                do
                                {
                                    if (var_t3_3 & 1)
                                    {
                                        var_v1 = 1;
                                        if (var_t1_2 & 1)
                                        {
                                            var_v1 = -1;
                                        }
                                    }
                                    else
                                    {
                                        var_v1 = 0;
                                    }
                                    *var_t7 = var_v1;
                                    var_t7 += 1;
                                    var_t3_3 = var_t3_3 >> 1;
                                    var_t0 -= 1;
                                    var_t1_2 = var_t1_2 >> 1;
                                } while (var_t0 != 0);
                            }
                            break;
                        }
                        if (var_t8_2 != 0)
                        {
                            temp_t2_2 = *(u32 *) (temp_s1 + 0);
                            temp_t0_2 = *(u32 *) (temp_s1 + 4);
                            temp_s1 += 8;
                            switch (sp1C)
                            {
                                default:
                                    var_t3 = var_t4 >> sp24;
                                    var_t6 = (var_t4 >> sp28) | (var_t4 >> (0x22 - temp_s3));
                                    var_a3_3 = var_t6;
                                    var_a2_6 = sp18 >> 1;
                                    do
                                    {
                                        var_a3_3 = var_a3_3 >> 2;
                                        var_a2_6 -= 1;
                                        var_t6 |= var_a3_3;
                                    } while (var_a2_6 != -1);
                                    var_t1 = (var_t5 >> sp24) | (var_t5 >> sp28);
                                    var_a0_2 = var_t1;
                                    var_a2_7 = (s32) (temp_s3 - 4) >> 1;
                                    do
                                    {
                                        var_a0_2 = var_a0_2 >> 2;
                                        var_a2_7 -= 1;
                                        var_t1 |= var_a0_2;
                                    } while (var_a2_7 != -1);
                                    if (arg0 & 1)
                                    {
                                        var_t6 |= var_t4 >> 0x1F;
                                    }
                                    else
                                    {
                                        var_t1 |= var_t5 >> 0x1F;
                                    }
                                    if (temp_t0_2 != 0)
                                    {
                                        var_a3_4 = (temp_t0_2 * 2) | (temp_t0_2 * 4);
                                        var_t6 |= var_a3_4;
                                        temp_a0_8 = temp_t2_2 | (temp_t2_2 * 2);
                                        temp_v1_4 = temp_a0_8 * 4;
                                        var_t1 |= temp_a0_8 | temp_v1_4;
                                        var_a0_3 = temp_v1_4;
                                        var_a2_8 = sp18 >> 1;
                                        do
                                        {
                                            var_a3_4 *= 4;
                                            var_t6 |= var_a3_4;
                                            var_a0_3 *= 4;
                                            var_a2_8 -= 1;
                                            var_t1 |= var_a0_3;
                                        } while (var_a2_8 != -1);
                                        if (arg0 & 1)
                                        {
                                            var_t6 |= temp_t0_2 << (arg0 - 2);
                                            var_t1 |= temp_t2_2 << temp_s3;
                                        }
                                        var_t3 |= temp_t0_2 | (temp_t0_2 << temp_s3);
                                        goto block_96;
                                    }
                                    goto block_97;
                                case 4:
                                    var_t6 = (var_t4 >> 0x1D) | (var_t4 >> 0x1E) | (var_t4 >> 0x1F);
                                    var_t3 = var_t4 >> 0x1C;
                                    temp_a0_5 = (var_t5 >> 0x1C) | (var_t5 >> 0x1D);
                                    var_t1 = temp_a0_5 | (temp_a0_5 >> 2);
                                    if (temp_t0_2 != 0)
                                    {
                                        var_t6 |= (temp_t0_2 * 2) | (temp_t0_2 * 4) | (temp_t0_2 * 8);
                                        var_t3 |= temp_t0_2 | (temp_t0_2 * 0x10);
                                        temp_a0_6 = temp_t2_2 | (temp_t2_2 * 2);
                                        var_v0_2 = temp_a0_6 | (temp_a0_6 * 4) | (temp_t2_2 * 0x10);
                                        goto block_95;
                                    }
                                    goto block_97;
                                case 3:
                                    var_t6 = (var_t4 >> 0x1E) | (var_t4 >> 0x1F);
                                    var_t3 = var_t4 >> 0x1D;
                                    var_t1 = (var_t5 >> 0x1D) | (var_t5 >> 0x1E) | (var_t5 >> 0x1F);
                                    if (temp_t0_2 != 0)
                                    {
                                        var_t6 |= (temp_t0_2 * 2) | (temp_t0_2 * 4);
                                        var_t3 |= temp_t0_2 | (temp_t0_2 * 8);
                                        temp_a0_7 = temp_t2_2 | (temp_t2_2 * 2);
                                        var_v0_2 = temp_a0_7 | (temp_a0_7 * 4);
                                        goto block_95;
                                    }
                                    goto block_97;
                                case 2:
                                    var_t6 = var_t4 >> 0x1F;
                                    var_t3 = var_t4 >> 0x1E;
                                    var_t1 = (var_t5 >> 0x1E) | (var_t5 >> 0x1F);
                                    if (temp_t0_2 != 0)
                                    {
                                        var_t6 |= temp_t0_2 * 2;
                                        var_t3 |= temp_t0_2 | (temp_t0_2 * 4);
                                        var_v0_2 = temp_t2_2 | (temp_t2_2 * 2) | (temp_t2_2 * 4);
                                        goto block_95;
                                    }
                                    goto block_97;
                                case 1:
                                    var_t3 = var_t4 >> 0x1F;
                                    var_t1 = var_t5 >> 0x1F;
                                    if (temp_t0_2 != 0)
                                    {
                                        var_t3 |= temp_t0_2 | (temp_t0_2 * 2);
                                        var_v0_2 = temp_t2_2 | (temp_t2_2 * 2);
block_95:
                                        var_t1 |= var_v0_2;
block_96:
                                        var_t4 = temp_t0_2;
                                        var_t5 = temp_t2_2;
                                        break;
                                    }
block_97:
                                    var_t4 = 0;
                                    var_t5 = 0;
                                    break;
                                case 0:
                                    var_t1 = temp_t2_2;
                                    var_t3 = temp_t0_2;
                                    break;
                            }
                            var_t0 = 0x20;
                            if (var_t8_2 != 0)
                            {
                                goto loop_38;
                            }
                        }
                    }
                    if (var_s4 >= (arg1 - 1))
                    {
                        var_t8_3 = arg0;
                        if (var_t8_3 != -1)
                        {
                            do
                            {
                                *var_t7 = -1;
                                var_t8_3 -= 1;
                                var_t7 += 1;
                            } while (var_t8_3 != -1);
                        }
                    }
                    var_s4 += 1;
                    temp_s6_5 = sp14 - 1;
                    sp14 = temp_s6_5;
                } while (temp_s6_5 != -1);
            }
            var_a2_9 = (4 - (s32) var_t7) & 3;
            var_t8_4 = (u16) scene->unk46 * (arg1 + 1);
            if (var_a2_9 != 0)
            {
loop_107:
                if (var_t8_4 != 0)
                {
                    *var_t7 = -1;
                    var_t7 += 1;
                    var_a2_9 -= 1;
                    var_t8_4 -= 1;
                    if (var_a2_9 != 0)
                    {
                        goto loop_107;
                    }
                }
            }
            var_a2_10 = (var_t8_4 >> 2) - 1;
            if (var_a2_10 != -1)
            {
                do
                {
                    *(s32 *) var_t7 = -1;
                    var_a2_10 -= 1;
                    var_t7 += 4;
                } while (var_a2_10 != -1);
            }
            var_a2_11 = (var_t8_4 & 3) - 1;
            if (var_a2_11 != -1)
            {
                do
                {
                    *var_t7 = -1;
                    var_a2_11 -= 1;
                    var_t7 += 1;
                } while (var_a2_11 != -1);
            }
            temp_s6_4 = sp8 - 1;
            sp8 = temp_s6_4;
        } while (temp_s6_4 != -1);
    }
}

typedef struct
{
    u8 pad0[0xC];
    s16 unkC;
    u8 pad1[0x10 - 0xE];
    s16 unk10;
} Margin;

s32 func_80060CB0(Margin *m, Query *q)
{
    FieldScene *scene;
    s32 count;
    s32 group;
    s32 i;
    s32 v;
    s32 g;
    s32 gy;
    s32 half;
    s32 half2;
    s32 ext_z;
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
    u8 *base;
    u8 *p;

    scene = g_field_scene.scene;
    if (scene->unk28 == 0)
    {
        if (scene->unk41 == 0)
        {
            return 0;
        }
        return -1;
    }

    v = q->y;
    gy = v >> 8;
    if (v < 0)
    {
        gy = (v + 0xFF) >> 8;
    }

    count = scene->unk41;
    group = count - 1;
    i = 0;
    if (count != 0)
    {
        half = count;
        for (; i != half; i++)
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

    tx = q->unkC;
    ext_z = q->unk10;
    tile = scene->unk40;
    shift = 3;
    if (tile == 4)
    {
        shift = 2;
    }

    half = ((s16) tx) >> 1;
    margin = m->unkC;
    margin = margin - 1;
    v = q->x;
    if (v >= 0)
    {
        g = v >> 8;
    }
    else
    {
        g = (v + 0xFF) >> 8;
    }
    tx = (g - half) - margin;

    tz = ext_z;
    half2 = ((s16) tz) >> 1;
    margin = m->unk10;
    margin = margin - 1;
    v = q->z;
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
    ncol = (((tx & mask) + m->unkC + ((s16) q->unkC) + mask) - 1) >> shift;
    mask2 = (tile * 2) - 1;
    nrow = (((tz & mask2) + m->unk10 + ((s16) q->unk10) + mask2) - 1) >> shift2;
    cols = (u16) scene->unk46;
    rows = (u16) scene->unk48;

    if (col_start <= 0)
    {
        g = ncol - 1;
        g = g + col_start;
        if (g <= 0)
        {
            return -2;
        }
        ncol = g;
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
    if (col_start >= cols - 1)
    {
        return -2;
    }
    if (row_start >= rows - 1)
    {
        return -2;
    }

    tz = (u16) scene->unk44;
    base = (u8 *) (scene->unk2C + (tz * group) + (cols * row_start) + col_start);
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
    s32 unk0;
    s32 unk4;
} OutPair;

/** Outgoing parameter block for func_80062820, built at sp+0x4020. */
typedef struct
{
    s32 unk0;
    s32 unk4;
    s32 unk8;
    s32 unkC;
    s32 unk10;
    s32 unk14;
    s32 unk18;
    s32 unk1C;
    s32 unk20;
    s32 unk24;
    s32 unk28;
    u8 unk2C;
} Req;

/**
 * @see decomp.me (75.62%, 489/1586 exact) TODO
 */
s32 func_80060F58(Query *arg0, Query *arg1, OutPair *arg2, s32 arg3)
{
    s32 path[4][0x400]; /* sp+0x0010 */
    s32 flags[4];       /* sp+0x4010 */
    Req rec;            /* sp+0x4020 */
    FieldScene *scene;
    Query *temp_s2;

    u32 sp408C;
    s32 sp4080;
    u8 sp407C;
    s32 sp4078;
    s32 sp4074;
    s32 sp4070;
    s32 sp406C;
    s32 sp4068;
    s32 sp4064;
    s32 sp4060;
    s32 sp405C;
    s32 sp4058;
    s32 sp4054;
    s32 sp4050;
    s16 temp_v1_6;
    s16 temp_v1_7;
    s32 *temp_s1_4;
    s32 *temp_s2_2;
    s32 *temp_v0_12;
    s32 *var_fp_2;
    s32 *var_fp_3;
    s32 *var_fp_4;
    s32 *var_s4;
    s32 *var_s4_2;
    s32 *var_s4_3;
    s32 *var_s4_4;
    s32 *var_s6;
    s32 *var_s6_2;
    s32 *var_s6_3;
    s32 *var_s6_4;
    s32 temp_a0;
    s32 temp_s1_3;
    s32 temp_s2_3;
    s32 temp_s2_4;
    s32 temp_s2_5;
    s32 temp_s2_6;
    s32 temp_s2_7;
    s32 temp_s3;
    s32 temp_s3_2;
    s32 temp_s4;
    s32 temp_s4_2;
    s32 temp_s4_3;
    s32 temp_s4_4;
    s32 temp_s4_5;
    s32 temp_s4_6;
    s32 temp_s6;
    s32 temp_s6_2;
    s32 temp_s6_3;
    s32 temp_t8;
    s32 temp_t9;
    s32 temp_v0;
    s32 hx1;
    s32 hz1;
    s32 hx2;
    s32 hz2;
    s32 temp_v0_2;
    s32 temp_v0_4;
    s32 temp_v0_13;
    s32 temp_v0_14;
    s32 temp_v1;
    s32 temp_v1_2;
    s32 temp_v1_3;
    s32 temp_v1_4;
    s32 temp_v1_5;
    s32 temp_v1_8;
    s32 temp_v1_30;
    s32 temp_v1_31;
    s32 temp_v1_32;
    s32 temp_v1_41;
    s32 temp_v1_42;
    s32 temp_v1_43;
    s32 temp_s0;
    s32 var_a0;
    s32 var_a0_2;
    s32 var_a1;
    s32 var_a1_2;
    s32 var_a1_3;
    s32 var_fp_5;
    s32 var_s0_2;
    s32 var_s3;
    s32 var_s3_2;
    s32 var_s3_3;
    s32 var_s3_4;
    s32 var_s5;
    s32 var_s5_2;
    s32 var_s5_3;
    s32 var_s5_4;
    s32 var_s5_5;
    s32 var_s5_7;
    s32 var_s5_8;
    s32 var_t2;
    s32 var_t3;
    s32 var_t5;
    s32 var_v0;
    s32 var_v0_2;
    s32 var_v0_3;
    s32 var_v0_4;
    s32 var_v0_5;
    s32 var_v0_27;
    s32 var_v0_28;
    s32 var_v0_30;
    s32 var_v0_31;
    s32 var_v1;
    s32 var_v1_2;
    s32 var_v1_3;
    s32 var_v1_4;
    s32 var_v1_5;
    u16 temp_a0_12;
    u16 temp_a0_21;
    u16 temp_t4;
    u32 *var_fp;
    u32 temp_a2_2;
    u32 temp_t3;
    u32 temp_v1_9;
    u32 temp_v1_10;
    u32 temp_v1_11;
    u32 temp_v1_12;
    u32 temp_v1_13;
    u32 temp_v1_14;
    u32 temp_v1_18;
    u32 temp_v1_22;
    u32 temp_v1_26;
    u32 temp_v1_33;
    u32 temp_v1_34;
    u32 temp_v1_35;
    u32 temp_v1_36;
    u32 temp_v1_37;
    u32 temp_v1_38;
    u32 temp_v1_39;
    u32 temp_v1_40;
    u32 var_s5_6;
    u32 var_s7;
    u32 var_s7_2;
    u32 var_s7_3;
    u32 var_t0;
    u32 var_v1_6;
    u32 var_v1_7;
    u32 var_v1_8;
    u32 var_v1_9;
    u32 temp_lo;
    u8 *temp_a2_3;
    u8 *temp_a2_4;
    u8 *temp_s1;
    u8 *temp_s1_2;
    u8 *temp_v0_7;
    u8 *temp_v0_8;
    u8 *temp_v0_9;
    u8 *temp_v0_10;
    u8 *var_s0;
    u8 *var_s1;
    u8 *var_s2;
    u8 *var_a2;
    u8 *var_v0_24;
    u8 *var_v0_25;
    u8 temp_a0_3;
    u8 temp_a0_4;
    u8 temp_a0_5;
    u8 temp_a0_6;
    u8 temp_a0_7;
    u8 temp_a0_8;
    u8 temp_a0_9;
    u8 temp_a0_10;
    u8 temp_a0_13;
    u8 temp_a0_14;
    u8 temp_a0_15;
    u8 temp_a0_16;
    u8 temp_a0_17;
    u8 temp_a0_18;
    u8 temp_a0_19;
    u8 temp_a0_20;
    u8 temp_a2;
    u8 temp_v0_3;
    u8 temp_v0_5;
    u8 temp_v1_15;
    u8 temp_v1_16;
    u8 temp_v1_17;
    u8 temp_v1_19;
    u8 temp_v1_20;
    u8 temp_v1_21;
    u8 temp_v1_23;
    u8 temp_v1_24;
    u8 temp_v1_25;
    u8 temp_v1_27;
    u8 temp_v1_28;
    u8 temp_v1_29;
    u8 var_a3;
    u8 var_a3_2;
    u8 var_t0_2;
    u8 var_t1;
    u8 var_t1_2;
    u8 var_t1_3;
    u8 var_t1_4;
    u8 var_t1_5;
    OutPair *var_a2_2;

    scene = g_field_scene.scene;
    temp_s2 = arg1;
    if (scene->unk28 == 0)
    {
        if (scene->unk41 != 0)
        {
            return -1;
        }
        arg2->unk0 = temp_s2->x;
        arg2->unk4 = temp_s2->z;
        return 1;
    }
    sp4058 = 3;
    temp_a2 = scene->unk40;
    if (temp_a2 == 4)
    {
        sp4058 = 2;
    }
    hx1 = (s32) (arg0->unkC << 0x10) >> 0x11;
    temp_v1 = temp_s2->x;
    if (temp_v1 >= 0)
    {
        var_v0_2 = temp_v1 >> 8;
    }
    else
    {
        var_v0_2 = (s32) (temp_v1 + 0xFF) >> 8;
    }
    sp4074 = var_v0_2 - hx1;
    hz1 = (s32) (arg0->unk10 << 0x10) >> 0x11;
    temp_v1_2 = temp_s2->z;
    if (temp_v1_2 >= 0)
    {
        var_v0_3 = temp_v1_2 >> 8;
    }
    else
    {
        var_v0_3 = (s32) (temp_v1_2 + 0xFF) >> 8;
    }
    sp4078 = var_v0_3 - hz1;
    hx2 = (s32) (arg0->unkC << 0x10) >> 0x11;
    temp_v1_3 = arg0->x;
    if (temp_v1_3 >= 0)
    {
        var_v0_4 = temp_v1_3 >> 8;
    }
    else
    {
        var_v0_4 = (s32) (temp_v1_3 + 0xFF) >> 8;
    }
    sp406C = var_v0_4 - hx2;
    hz2 = (s32) (arg0->unk10 << 0x10) >> 0x11;
    temp_a0 = arg0->z;
    if (temp_a0 >= 0)
    {
        var_v0_5 = temp_a0 >> 8;
    }
    else
    {
        var_v0_5 = (s32) (temp_a0 + 0xFF) >> 8;
    }
    sp4070 = var_v0_5 - hz2;
    temp_v1_4 = sp4058 + 1;
    sp4064 = (sp4074 >> sp4058) + 2;
    sp4068 = (sp4078 >> temp_v1_4) + 2;
    temp_v0 = (sp406C >> sp4058) + 2;
    sp405C = temp_v0;
    sp4060 = (sp4070 >> temp_v1_4) + 2;
    temp_t4 = (u16) scene->unk46;
    if ((temp_v0 > 0) && (sp4060 > 0) && (sp4064 > 0) && (sp4068 > 0))
    {
        temp_v1_5 = (u16) scene->unk48 - 1;
        if ((temp_v0 < temp_t4 - 1) && (sp4060 < temp_v1_5) && (sp4064 < temp_t4 - 1))
        {
            if (sp4068 >= temp_v1_5)
            {
                return -2;
            }
            temp_v0_2 = temp_s2->y;
            var_a0 = temp_v0_2 >> 8;
            if (temp_v0_2 < 0)
            {
                var_a0 = (s32) (temp_v0_2 + 0xFF) >> 8;
            }
            temp_v0_3 = scene->unk41;
            var_s5 = 0;
            sp4054 = temp_v0_3 - 1;
            if (temp_v0_3 != 0)
            {
                var_a1 = temp_v0_3;
                do
                {
                    temp_v1_6 = scene->unk4A[var_s5];
                    if (var_a0 < temp_v1_6)
                    {
                        if (var_s5 != 0)
                        {
                            var_s5 -= 1;
                        }
                        sp4054 = var_s5;
                        break;
                    }
                    if (temp_v1_6 == var_a0)
                    {
                        sp4054 = var_s5;
                        break;
                    }
                    var_s5 += 1;
                } while (var_s5 != var_a1);
            }
            var_s0 = (u8 *) (scene->unk2C + ((u16) scene->unk44 * sp4054) + (temp_t4 * sp4068) + sp4064);
            *var_s0 = 0xFC;
            temp_v0_4 = arg0->y;
            var_a0_2 = temp_v0_4 >> 8;
            if (temp_v0_4 < 0)
            {
                var_a0_2 = (s32) (temp_v0_4 + 0xFF) >> 8;
            }
            temp_v0_5 = scene->unk41;
            var_s5_2 = 0;
            sp4050 = temp_v0_5 - 1;
            if (temp_v0_5 != 0)
            {
                var_a1 = temp_v0_5;
                do
                {
                    temp_v1_7 = scene->unk4A[var_s5_2];
                    if (var_a0_2 < temp_v1_7)
                    {
                        if (var_s5_2 != 0)
                        {
                            var_s5_2 -= 1;
                        }
                        sp4050 = var_s5_2;
                        break;
                    }
                    if (temp_v1_7 == var_a0_2)
                    {
                        sp4050 = var_s5_2;
                        break;
                    }
                    var_s5_2 += 1;
                } while (var_s5_2 != var_a1);
            }
            temp_s1 = (u8 *) (scene->unk2C + ((u16) scene->unk44 * sp4050) + (temp_t4 * sp4060) + sp405C);
            if (*temp_s1 != 0xFC)
            {
                *temp_s1 = 0xFB;
                rec.unk8 = sp4074;
                rec.unkC = sp4078;
                rec.unk10 = sp406C;
                path[0][0] = (s32) temp_s1 | 0x1FE00000;
                rec.unk0 = (s32) var_s0;
                rec.unk14 = sp4070;
                rec.unk18 = (s32) (s16) arg0->unkC;
                rec.unk20 = (s32) temp_a2;
                rec.unk24 = sp4058;
                rec.unk2C = 0xFC;
                sp408C = (u32) temp_t4;
                rec.unk28 = arg3;
                rec.unk1C = (s32) (s16) arg0->unk10;
                var_t5 = 0;
                if (func_80062820(&rec) == 0)
                {
                    var_s7 = 1;
                    var_s2 = NULL;
                    var_a3 = 0xFA;
                    flags[0] = 1;
                    flags[3] = 0;
                    flags[2] = 0;
                    flags[1] = 0;
                    while (1)
                    {
                        if (var_s7 == 0)
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
                        var_t0 = 0;
                        var_s5_3 = var_s7 - 1;
                        var_fp = (u32 *) &path[var_t5][var_s7 - 1];
                        temp_v1_8 = (var_t5 + 1) & 3;
                        var_s7 = flags[temp_v1_8];
                        var_s4 = &path[(var_t5 + 3) & 3][0];
                        var_s6 = &path[temp_v1_8][var_s7];
                        if (var_s5_3 != -1)
                        {
                            temp_t3 = var_a3 & 0xFF;
                            do
                            {
                                if ((var_s7 >= 0x3F9U) || (var_t0 >= 0x3F9U))
                                {
                                    return -4;
                                }
                                temp_v1_9 = *var_fp;
                                var_fp -= 1;
                                temp_s1_2 = (u8 *) (temp_v1_9 & 0x801FFFFF);
                                temp_a2_2 = temp_v1_9 >> 0x15;
                                var_a1_2 = 0;
                                if ((temp_a2_2 & 0x300) == 0x300)
                                {
                                    var_t0 = var_t0 + 1 + var_s5_3;
                                    do
                                    {
                                        *var_s4 = var_fp[1] & 0xBFFFFFFF;
                                        var_fp -= 1;
                                        var_s5_3 -= 1;
                                        var_s4 += 1;
                                    } while (var_s5_3 != -1);
                                    break;
                                }
                                if (temp_a2_2 & 2)
                                {
                                    var_s0 = temp_s1_2 - sp408C;
                                    temp_a0_3 = *var_s0;
                                    temp_v1_10 = temp_a0_3 & 0xFF;
                                    if ((temp_v1_10 < temp_t3) && ((temp_v1_10 < 4U) || !(temp_a2_2 & 0x100)))
                                    {
                                        if (temp_v1_10 != 1)
                                        {
                                            *var_s6 = (s32) var_s0 | 0x03E00000;
                                            var_s6 += 1;
                                            var_s7 += 1;
                                            var_a1_2 = 2;
                                            *var_s0 = var_a3;
                                        }
                                        else
                                        {
                                            *var_s4 = (s32) var_s0 | 0x63E00000;
                                            var_s4 += 1;
                                            var_t0 += 1;
                                            var_a1_2 = 0x202;
                                            *var_s0 = 0xFD;
                                        }
                                    }
                                    else if (temp_a0_3 == 0xFC)
                                    {
                                        var_s2 = var_s0;
                                        if (temp_a2_2 & 0x100)
                                        {
                                            var_a1_2 = 2;
                                        }
                                    }
                                }
                                if (temp_a2_2 & 0x40)
                                {
                                    var_s0 = temp_s1_2 + sp408C;
                                    temp_a0_4 = *var_s0;
                                    temp_v1_11 = temp_a0_4 & 0xFF;
                                    if ((temp_v1_11 < temp_t3) && ((temp_v1_11 < 4U) || !(temp_a2_2 & 0x100)))
                                    {
                                        if (temp_v1_11 != 1)
                                        {
                                            *var_s6 = (s32) var_s0 | 0x1F000000;
                                            var_s6 += 1;
                                            var_s7 += 1;
                                            var_a1_2 |= 0x40;
                                            *var_s0 = var_a3;
                                        }
                                        else
                                        {
                                            *var_s4 = (s32) var_s0 | 0x7F000000;
                                            var_s4 += 1;
                                            var_t0 += 1;
                                            var_a1_2 |= 0x4040;
                                            *var_s0 = 0xFD;
                                        }
                                    }
                                    else if (temp_a0_4 == 0xFC)
                                    {
                                        var_s2 = var_s0;
                                        if (temp_a2_2 & 0x100)
                                        {
                                            var_a1_2 |= 0x40;
                                        }
                                    }
                                }
                                if (temp_a2_2 & 8)
                                {
                                    temp_a0_5 = temp_s1_2[-1];
                                    temp_v1_12 = temp_a0_5 & 0xFF;
                                    var_s0 = temp_s1_2 - 1;
                                    if ((temp_v1_12 < temp_t3) && ((temp_v1_12 < 4U) || !(temp_a2_2 & 0x100)))
                                    {
                                        if (temp_v1_12 != 1)
                                        {
                                            *var_s6 = (s32) var_s0 | 0x0D600000;
                                            var_s6 += 1;
                                            var_s7 += 1;
                                            var_a1_2 |= 8;
                                            temp_s1_2[-1] = var_a3;
                                        }
                                        else
                                        {
                                            *var_s4 = (s32) var_s0 | 0x6D600000;
                                            var_s4 += 1;
                                            var_t0 += 1;
                                            var_a1_2 |= 0x808;
                                            temp_s1_2[-1] = 0xFD;
                                        }
                                    }
                                    else if (temp_a0_5 == 0xFC)
                                    {
                                        var_s2 = var_s0;
                                        if (temp_a2_2 & 0x100)
                                        {
                                            var_a1_2 |= 8;
                                        }
                                    }
                                }
                                if (temp_a2_2 & 0x10)
                                {
                                    temp_a0_6 = temp_s1_2[1];
                                    temp_v1_13 = temp_a0_6 & 0xFF;
                                    var_s0 = temp_s1_2 + 1;
                                    if ((temp_v1_13 < temp_t3) && ((temp_v1_13 < 4U) || !(temp_a2_2 & 0x100)))
                                    {
                                        if (temp_v1_13 != 1)
                                        {
                                            *var_s6 = (s32) var_s0 | 0x1AC00000;
                                            var_s6 += 1;
                                            var_s7 += 1;
                                            var_a1_2 |= 0x10;
                                            temp_s1_2[1] = var_a3;
                                        }
                                        else
                                        {
                                            *var_s4 = (s32) var_s0 | 0x7AC00000;
                                            var_s4 += 1;
                                            var_t0 += 1;
                                            var_a1_2 |= 0x1010;
                                            temp_s1_2[1] = 0xFD;
                                        }
                                    }
                                    else if (temp_a0_6 == 0xFC)
                                    {
                                        var_s2 = var_s0;
                                        if (temp_a2_2 & 0x100)
                                        {
                                            var_a1_2 |= 0x10;
                                        }
                                    }
                                }
                                temp_v0_7 = temp_s1_2 - sp408C;
                                if (temp_a2_2 & 1)
                                {
                                    temp_a0_7 = temp_v0_7[-1];
                                    var_s0 = temp_v0_7 - 1;
                                    temp_v1_14 = temp_a0_7 & 0xFF;
                                    if ((temp_v1_14 < temp_t3) && ((temp_v1_14 < 4U) || !(temp_a2_2 & 0x100)))
                                    {
                                        temp_v1_15 = temp_v0_7[0];
                                        var_t1 = 0;
                                        if (temp_v1_15 < 0xFEU)
                                        {
                                            if ((temp_v1_15 == 1) || (temp_v1_15 == 0xFD))
                                            {
                                                temp_v1_16 = temp_s1_2[-1];
                                                if ((temp_v1_16 == 1) || (temp_v1_16 == 0xFD))
                                                {
                                                    var_t1 = 1;
                                                }
                                            }
                                            else
                                            {
                                                temp_v1_17 = temp_s1_2[-1];
                                                if ((temp_v1_17 < 0xFDU) && (temp_v1_17 != 1))
                                                {
                                                    var_t1 = 1;
                                                }
                                            }
                                        }
                                        if (var_t1 != 0)
                                        {
                                            var_v1 = (s32) var_s0 | 0x01600000;
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
                                                temp_v0_7[-1] = var_a3;
                                            }
                                            else
                                            {
                                                *var_s4 = var_v1 | 0x60000000;
                                                var_s4 += 1;
                                                var_t0 += 1;
                                                temp_v0_7[-1] = 0xFD;
                                            }
                                        }
                                    }
                                    else if (temp_a0_7 == 0xFC)
                                    {
                                        var_s2 = var_s0;
                                        if (temp_a2_2 & 0x100)
                                        {
                                            var_a1_2 |= 1;
                                        }
                                    }
                                }
                                temp_v0_8 = temp_s1_2 - sp408C;
                                if (temp_a2_2 & 4)
                                {
                                    temp_a0_8 = temp_v0_8[1];
                                    var_s0 = temp_v0_8 + 1;
                                    temp_v1_18 = temp_a0_8 & 0xFF;
                                    if ((temp_v1_18 < temp_t3) && ((temp_v1_18 < 4U) || !(temp_a2_2 & 0x100)))
                                    {
                                        temp_v1_19 = var_s0[-1];
                                        var_t1_2 = 0;
                                        if (temp_v1_19 < 0xFEU)
                                        {
                                            if ((temp_v1_19 == 1) || (temp_v1_19 == 0xFD))
                                            {
                                                temp_v1_20 = temp_s1_2[1];
                                                if ((temp_v1_20 == 1) || (temp_v1_20 == 0xFD))
                                                {
                                                    var_t1_2 = 1;
                                                }
                                            }
                                            else
                                            {
                                                temp_v1_21 = temp_s1_2[1];
                                                if ((temp_v1_21 < 0xFDU) && (temp_v1_21 != 1))
                                                {
                                                    var_t1_2 = 1;
                                                }
                                            }
                                        }
                                        if (var_t1_2 != 0)
                                        {
                                            var_v1_2 = (s32) var_s0 | 0x02C00000;
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
                                                temp_v0_8[1] = var_a3;
                                            }
                                            else
                                            {
                                                *var_s4 = var_v1_2 | 0x60000000;
                                                var_s4 += 1;
                                                var_t0 += 1;
                                                temp_v0_8[1] = 0xFD;
                                            }
                                        }
                                    }
                                    else if (temp_a0_8 == 0xFC)
                                    {
                                        var_s2 = var_s0;
                                        if (temp_a2_2 & 0x100)
                                        {
                                            var_a1_2 |= 4;
                                        }
                                    }
                                }
                                temp_v0_9 = temp_s1_2 + sp408C;
                                if (temp_a2_2 & 0x20)
                                {
                                    temp_a0_9 = temp_v0_9[-1];
                                    var_s0 = temp_v0_9 - 1;
                                    temp_v1_22 = temp_a0_9 & 0xFF;
                                    if ((temp_v1_22 < temp_t3) && ((temp_v1_22 < 4U) || !(temp_a2_2 & 0x100)))
                                    {
                                        temp_v1_23 = temp_v0_9[0];
                                        var_t1_3 = 0;
                                        if (temp_v1_23 < 0xFEU)
                                        {
                                            if ((temp_v1_23 == 1) || (temp_v1_23 == 0xFD))
                                            {
                                                temp_v1_24 = temp_s1_2[-1];
                                                if ((temp_v1_24 == 1) || (temp_v1_24 == 0xFD))
                                                {
                                                    var_t1_3 = 1;
                                                }
                                            }
                                            else
                                            {
                                                temp_v1_25 = temp_s1_2[-1];
                                                if ((temp_v1_25 < 0xFDU) && (temp_v1_25 != 1))
                                                {
                                                    var_t1_3 = 1;
                                                }
                                            }
                                        }
                                        if (var_t1_3 != 0)
                                        {
                                            var_v1_3 = (s32) var_s0 | 0x0D000000;
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
                                                temp_v0_9[-1] = var_a3;
                                            }
                                            else
                                            {
                                                *var_s4 = var_v1_3 | 0x60000000;
                                                var_s4 += 1;
                                                var_t0 += 1;
                                                temp_v0_9[-1] = 0xFD;
                                            }
                                        }
                                    }
                                    else if (temp_a0_9 == 0xFC)
                                    {
                                        var_s2 = var_s0;
                                        if (temp_a2_2 & 0x100)
                                        {
                                            var_a1_2 |= 0x20;
                                        }
                                    }
                                }
                                temp_v0_10 = temp_s1_2 + sp408C;
                                if (temp_a2_2 & 0x80)
                                {
                                    temp_a0_10 = temp_v0_10[1];
                                    var_s0 = temp_v0_10 + 1;
                                    temp_v1_26 = temp_a0_10 & 0xFF;
                                    if ((temp_v1_26 < temp_t3) && ((temp_v1_26 < 4U) || !(temp_a2_2 & 0x100)))
                                    {
                                        temp_v1_27 = var_s0[-1];
                                        var_t1_4 = 0;
                                        if (temp_v1_27 < 0xFEU)
                                        {
                                            if ((temp_v1_27 == 1) || (temp_v1_27 == 0xFD))
                                            {
                                                temp_v1_28 = temp_s1_2[1];
                                                if ((temp_v1_28 == 1) || (temp_v1_28 == 0xFD))
                                                {
                                                    var_t1_4 = 1;
                                                }
                                            }
                                            else
                                            {
                                                temp_v1_29 = temp_s1_2[1];
                                                if ((temp_v1_29 < 0xFDU) && (temp_v1_29 != 1))
                                                {
                                                    var_t1_4 = 1;
                                                }
                                            }
                                        }
                                        if (var_t1_4 != 0)
                                        {
                                            var_v1_4 = (s32) var_s0 | 0x1A000000;
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
                                                temp_v0_10[1] = var_a3;
                                            }
                                            else
                                            {
                                                *var_s4 = var_v1_4 | 0x60000000;
                                                var_s4 += 1;
                                                var_t0 += 1;
                                                temp_v0_10[1] = 0xFD;
                                            }
                                        }
                                    }
                                    else if (temp_a0_10 == 0xFC)
                                    {
                                        var_s2 = var_s0;
                                        if (temp_a2_2 & 0x100)
                                        {
                                            var_a1_2 |= 0x80;
                                        }
                                    }
                                }
                                if ((temp_a2_2 & 0x100) && (var_a1_2 != 0))
                                {
                                    temp_s1_2[0] = (s8) (var_a3 + 1);
                                }
                                var_s5_3 -= 1;
                            } while (var_s5_3 != -1);
                        }
                        var_a3 -= 1;
                        if (((u32) (var_a3 & 0xFF) < 4U) && (var_s2 == NULL))
                        {
                            return -3;
                        }
                        temp_v1_30 = var_t5 + 3;
                        flags[var_t5] = 0;
                        var_t5 = (var_t5 + 1) & 3;
                        flags[var_t5] = var_s7;
                        flags[temp_v1_30 & 3] = var_t0;
                        if (var_s2 != NULL)
                        {
                            var_s3_2 = 2;
                            var_v1_5 = var_t5;
                            do
                            {
                                var_s5_4 = var_s7 - 1;
                                var_fp_2 = &path[var_v1_5][var_s7 - 1];
                                if (var_s5_4 != -1)
                                {
                                    do
                                    {
                                        temp_v1_31 = *var_fp_2;
                                        var_fp_2 -= 1;
                                        if (temp_v1_31 & 0x20000000)
                                        {
                                            *(u8 *) (temp_v1_31 & 0x801FFFFF) = 0xFD;
                                        }
                                        var_s5_4 -= 1;
                                    } while (var_s5_4 != -1);
                                }
                                var_t5 = (var_t5 + 1) & 3;
                                var_s7 = flags[var_t5];
                                var_s3_2 -= 1;
                                var_v1_5 = var_t5;
                            } while (var_s3_2 != -1);
                            temp_a0_12 = (u16) scene->unk44;
                            temp_lo = sp408C * sp4068;
                            temp_v1_32 = scene->unk2C;
                            var_s1 = (u8 *) (temp_v1_32 + (temp_a0_12 * sp4054) + temp_lo + sp4064);
                            var_fp_3 = &path[0][0];
                            if (var_s1 != var_s2)
                            {
                                var_v1_6 = (u32) var_s2 - (u32) temp_v1_32;
                                if (var_v1_6 >= temp_a0_12)
                                {
                                    var_v1_7 = var_v1_6 - temp_a0_12;
                                    do
                                    {
                                        var_v1_7 -= temp_a0_12;
                                    } while (var_v1_7 >= temp_a0_12);
                                    var_v1_6 = var_v1_7 + temp_a0_12;
                                }
                                rec.unk8 = sp4074;
                                rec.unk0 = (s32) var_s1;
                                rec.unk2C = 4;
                                rec.unkC = sp4078;
                                rec.unk10 = ((var_v1_6 % sp408C) - 2) << sp4058;
                                rec.unk14 = ((var_v1_6 / sp408C) - 2) << (sp4058 + 1);
                                var_s7_2 = 1;
                                if (func_80062820(&rec) != 0)
                                {
                                    *var_fp_3 = (s32) var_s1;
                                    var_fp_3 += 1;
                                    *var_s1 = 0xFC;
                                    var_s1 = var_s2;
                                }
                                else
                                {
                                    rec.unk4 = (s32) var_s2;
                                    rec.unk2C = 4;
                                    rec.unk28 = 2;
                                    rec.unk10 = sp406C;
                                    rec.unk14 = sp4070;
                                    var_s7_2 = 1;
                                    if (func_80062820(&rec) != 0)
                                    {
                                        *var_fp_3 = (s32) var_s1;
                                        var_fp_3 += 1;
                                        *var_s1 = 0xFC;
                                        var_s1 = var_s2;
                                    }
                                    else
                                    {
                                        var_s7_2 = 0;
                                        *var_s1 = 0xFC;
                                    }
                                    rec.unk28 = arg3;
                                }
                                var_t1_5 = 1;
                            }
                            else
                            {
                                var_s7_2 = 0;
                                var_t1_5 = 0;
                            }
                            var_t2 = 0;
                            var_t3 = 0;
                            var_t0_2 = 0;
                            do
                            {
                                var_a3_2 = 0;
                                temp_a0_13 = var_s1[-1];
                                var_a1_3 = 0;
                                temp_v1_33 = temp_a0_13 & 0xFF;
                                if (((u32) ((temp_a0_13 - 4) & 0xFF) < 0xF8U) && ((u32) (var_t0_2 & 0xFF) < temp_v1_33) && ((u32) (var_a3_2 & 0xFF) < temp_v1_33))
                                {
                                    var_s0 = var_s1 - 1;
                                    var_t2 = 4;
                                    var_a3_2 = temp_a0_13;
                                }
                                else
                                {
                                    var_a1_3 = 8;
                                }
                                temp_a0_14 = var_s1[1];
                                temp_v1_34 = temp_a0_14 & 0xFF;
                                if (((u32) ((temp_a0_14 - 4) & 0xFF) < 0xF8U) && ((u32) (var_t0_2 & 0xFF) < temp_v1_34) && ((u32) (var_a3_2 & 0xFF) < temp_v1_34))
                                {
                                    var_s0 = var_s1 + 1;
                                    var_t2 = 5;
                                    var_a3_2 = temp_a0_14;
                                }
                                else
                                {
                                    var_a1_3 |= 0x10;
                                }
                                temp_a2_3 = var_s1 - sp408C;
                                temp_a0_15 = *temp_a2_3;
                                temp_v1_35 = temp_a0_15 & 0xFF;
                                if (((u32) ((temp_a0_15 - 4) & 0xFF) < 0xF8U) && ((u32) (var_t0_2 & 0xFF) < temp_v1_35) && ((u32) (var_a3_2 & 0xFF) < temp_v1_35))
                                {
                                    var_s0 = temp_a2_3;
                                    var_t2 = 2;
                                    var_a3_2 = temp_a0_15;
                                }
                                else
                                {
                                    var_a1_3 |= 2;
                                }
                                temp_a2_4 = var_s1 + sp408C;
                                temp_a0_16 = *temp_a2_4;
                                temp_v1_36 = temp_a0_16 & 0xFF;
                                if (((u32) ((temp_a0_16 - 4) & 0xFF) < 0xF8U) && ((u32) (var_t0_2 & 0xFF) < temp_v1_36) && ((u32) (var_a3_2 & 0xFF) < temp_v1_36))
                                {
                                    var_s0 = temp_a2_4;
                                    var_t2 = 7;
                                    var_a3_2 = temp_a0_16;
                                }
                                else
                                {
                                    var_a1_3 |= 0x40;
                                }
                                var_a2 = var_s1 - sp408C;
                                temp_a0_17 = var_a2[-1];
                                if (!(var_a1_3 & 0xA) || !(var_a3_2 & 0xFF))
                                {
                                    temp_v1_37 = temp_a0_17 & 0xFF;
                                    if ((temp_v1_37 < 0xFCU) && (temp_v1_37 >= 4U) && ((u32) (var_t0_2 & 0xFF) < temp_v1_37) && ((u32) (var_a3_2 & 0xFF) < temp_v1_37))
                                    {
                                        var_s0 = var_a2 - 1;
                                        var_t2 = 1;
                                        var_a3_2 = temp_a0_17;
                                    }
                                }
                                temp_a0_18 = var_a2[1];
                                var_v0_24 = var_s1 + sp408C;
                                if (!(var_a1_3 & 0x12) || !(var_a3_2 & 0xFF))
                                {
                                    temp_v1_38 = temp_a0_18 & 0xFF;
                                    if ((temp_v1_38 < 0xFCU) && (temp_v1_38 >= 4U) && ((u32) (var_t0_2 & 0xFF) < temp_v1_38) && ((u32) (var_a3_2 & 0xFF) < temp_v1_38))
                                    {
                                        var_s0 = var_a2 + 1;
                                        var_t2 = 3;
                                        var_a3_2 = temp_a0_18;
                                    }
                                }
                                temp_a0_19 = var_v0_24[-1];
                                var_v0_25 = var_s1 + sp408C;
                                if (!(var_a1_3 & 0x48) || !(var_a3_2 & 0xFF))
                                {
                                    temp_v1_39 = temp_a0_19 & 0xFF;
                                    if ((temp_v1_39 < 0xFCU) && (temp_v1_39 >= 4U) && ((u32) (var_t0_2 & 0xFF) < temp_v1_39) && ((u32) (var_a3_2 & 0xFF) < temp_v1_39))
                                    {
                                        var_s0 = var_v0_24 - 1;
                                        var_t2 = 6;
                                        var_a3_2 = temp_a0_19;
                                    }
                                }
                                temp_a0_20 = var_v0_25[1];
                                if (!(var_a1_3 & 0x50) || !(var_a3_2 & 0xFF))
                                {
                                    temp_v1_40 = temp_a0_20 & 0xFF;
                                    if ((temp_v1_40 < 0xFCU) && (temp_v1_40 >= 4U) && ((u32) (var_t0_2 & 0xFF) < temp_v1_40) && ((u32) (var_a3_2 & 0xFF) < temp_v1_40))
                                    {
                                        var_s0 = var_v0_25 + 1;
                                        var_t2 = 8;
                                        var_a3_2 = temp_a0_20;
                                    }
                                    var_v0 = -5;
                                    if ((var_a3_2 & 0xFF) == 0)
                                    {
                                        return var_v0;
                                    }
                                }
                                var_t0_2 = var_a3_2;
                                if (var_t2 != var_t3)
                                {
                                    var_t3 = var_t2;
                                    *var_fp_3 = (s32) var_s1;
                                    var_s7_2 += 1;
                                    var_fp_3 += 1;
                                    if (var_s7_2 >= 0x400U)
                                    {
                                        return -4;
                                    }
                                    var_t0_2 = var_a3_2;
                                }
                                var_s1 = var_s0;
                            } while ((var_t0_2 & 0xFF) != 0xFB);
                            var_s6_2 = &path[1][1];
                            var_s4_2 = &path[2][1];
                            var_s3_3 = var_s7_2 - 2;
                            *var_fp_3 = scene->unk2C + ((u16) scene->unk44 * sp4050) + (sp408C * sp4060) + sp405C;
                            var_fp_4 = &path[0][0];
                            rec.unk2C = 0;
                            path[1][0] = sp4074;
                            path[2][0] = sp4078;
                            if (var_s3_3 != -1)
                            {
                                do
                                {
                                    var_fp_4 += 1;
                                    temp_a0_21 = (u16) scene->unk44;
                                    var_v1_8 = *var_fp_4 - scene->unk2C;
                                    if (var_v1_8 >= temp_a0_21)
                                    {
                                        var_v1_9 = var_v1_8 - temp_a0_21;
                                        do
                                        {
                                            var_v1_9 -= temp_a0_21;
                                        } while (var_v1_9 >= temp_a0_21);
                                        var_v1_8 = var_v1_9 + temp_a0_21;
                                    }
                                    var_s3_3 -= 1;
                                    *var_s6_2 = ((var_v1_8 % sp408C) - 2) << sp4058;
                                    var_s6_2 += 1;
                                    *var_s4_2 = ((var_v1_8 / sp408C) - 2) << (sp4058 + 1);
                                    var_s4_2 += 1;
                                } while (var_s3_3 != -1);
                                var_fp_4 = &path[0][0];
                            }
                            var_s7_3 = 0;
                            var_s5_5 = var_s7_2 - 1;
                            *var_s6_2 = sp406C;
                            var_s6_3 = &path[1][0];
                            *var_s4_2 = sp4070;
                            var_s4_3 = &path[2][0];
                            if (var_s5_5 != -1)
                            {
                                do
                                {
                                    temp_s1_3 = *var_fp_4;
                                    var_fp_4 += 1;
                                    temp_t9 = *var_s6_3;
                                    var_s6_3 += 1;
                                    temp_v1_41 = var_s7_3;
                                    var_s7_3 += 1;
                                    temp_v0_12 = &path[0][temp_v1_41];
                                    sp406C = temp_t9;
                                    temp_t8 = *var_s4_3;
                                    var_s4_3 += 1;
                                    sp4070 = temp_t8;
                                    temp_v0_12[0] = temp_s1_3;
                                    temp_v0_12[0x400] = temp_t9;
                                    temp_v0_12[0x800] = temp_t8;
                                    if (var_s5_5 != 0)
                                    {
                                        var_s3_4 = var_s5_5;
                                        rec.unk0 = temp_s1_3;
                                        rec.unk8 = temp_t9;
                                        rec.unkC = temp_t8;
                                        var_s0_2 = var_s3_4;
                                        do
                                        {
                                            temp_s2_2 = var_s0_2 + var_s6_3;
                                            temp_s1_4 = var_s0_2 + var_s4_3;
                                            rec.unk10 = *temp_s2_2;
                                            rec.unk14 = *temp_s1_4;
                                            if (func_80062820(&rec) != 0)
                                            {
                                                var_fp_4 += var_s0_2;
                                                var_s6_3 = temp_s2_2;
                                                var_s4_3 = temp_s1_4;
                                                var_s5_5 -= var_s3_4;
                                                break;
                                            }
                                            var_s3_4 -= 1;
                                            var_s0_2 = var_s3_4;
                                        } while (var_s3_4 != 0);
                                    }
                                    var_s5_5 -= 1;
                                } while (var_s5_5 != -1);
                            }
                            var_s5_6 = var_s7_3;
                            if ((var_t1_5 & 0xFF) && (var_s7_3 >= 2U))
                            {
                                sp407C = 1;
                                sp405C = path[1][0];
                                sp4060 = path[2][0];
                                temp_v0_12 = &path[0][var_s7_3];
                                temp_v0_12[0] = *var_fp_4;
                                temp_v0_12[0x400] = *var_s6_3;
                                temp_v0_12[0x800] = *var_s4_3;
                                temp_s0 = path[0][2];
                                var_fp_5 = path[1][1];
                                var_s5_7 = path[2][1];
                                temp_s6 = path[1][2];
                                sp4064 = temp_s6;
                                rec.unk8 = path[1][0];
                                temp_v0_13 = var_fp_5 + temp_s6;
                                rec.unk0 = path[0][0];
                                rec.unkC = path[2][0];
                                var_t1_5 = 0;
                                sp4068 = path[2][2];
                                temp_s4 = temp_v0_13 / 2;
                                rec.unk10 = temp_s4;
                                temp_s2_3 = (var_s5_7 + sp4068) / 2;
                                rec.unk14 = temp_s2_3;
                                if (func_80062820(&rec) != 0)
                                {
                                    rec.unk0 = temp_s0;
                                    rec.unk8 = sp4064;
                                    rec.unkC = sp4068;
                                    if (func_80062820(&rec) != 0)
                                    {
                                        var_fp_5 = temp_s4;
                                        var_s5_7 = temp_s2_3;
                                        rec.unk8 = sp405C;
                                        sp407C = 0;
                                        rec.unk0 = path[0][0];
                                        var_t1_5 = 1;
                                        rec.unkC = sp4060;
                                        temp_s4_2 = (var_fp_5 + sp4064) / 2;
                                        temp_s2_4 = (var_s5_7 + sp4068) / 2;
                                        rec.unk10 = temp_s4_2;
                                        rec.unk14 = temp_s2_4;
                                        if (func_80062820(&rec) != 0)
                                        {
                                            rec.unk0 = temp_s0;
                                            rec.unk8 = sp4064;
                                            rec.unkC = sp4068;
                                            if (func_80062820(&rec) != 0)
                                            {
                                                var_fp_5 = temp_s4_2;
                                                var_s5_7 = temp_s2_4;
                                            }
                                        }
                                    }
                                }
                                if (sp407C != 0)
                                {
                                    temp_s4_3 = sp4064 - var_fp_5;
                                    var_v0_27 = temp_s4_3;
                                    rec.unk0 = path[0][0];
                                    rec.unk8 = sp405C;
                                    rec.unkC = sp4060;
                                    if (temp_s4_3 < 0)
                                    {
                                        var_v0_27 = temp_s4_3 + 3;
                                    }
                                    temp_v0_14 = var_fp_5 + (var_v0_27 >> 2);
                                    sp4080 = temp_v0_14;
                                    rec.unk10 = temp_v0_14;
                                    temp_s3 = sp4068 - var_s5_7;
                                    var_v0_28 = temp_s3;
                                    if (temp_s3 < 0)
                                    {
                                        var_v0_28 = temp_s3 + 3;
                                    }
                                    temp_s6_2 = var_s5_7 + (var_v0_28 >> 2);
                                    rec.unk14 = temp_s6_2;
                                    if (func_80062820(&rec) != 0)
                                    {
                                        rec.unk0 = temp_s0;
                                        rec.unk8 = sp4064;
                                        rec.unkC = sp4068;
                                        if (func_80062820(&rec) != 0)
                                        {
                                            var_t1_5 = 1;
                                            var_fp_5 = sp4080;
                                            var_s5_7 = temp_s6_2;
                                        }
                                    }
                                }
                                sp407C = 1;
                                rec.unk8 = sp4064;
                                rec.unk0 = temp_s0;
                                rec.unkC = sp4068;
                                temp_s4_4 = (var_fp_5 + sp405C) / 2;
                                temp_s2_5 = (var_s5_7 + sp4060) / 2;
                                rec.unk10 = temp_s4_4;
                                rec.unk14 = temp_s2_5;
                                if (func_80062820(&rec) != 0)
                                {
                                    rec.unk0 = path[0][0];
                                    rec.unk8 = sp405C;
                                    rec.unkC = sp4060;
                                    if (func_80062820(&rec) != 0)
                                    {
                                        var_fp_5 = temp_s4_4;
                                        var_s5_7 = temp_s2_5;
                                        rec.unk8 = sp4064;
                                        sp407C = 0;
                                        rec.unk0 = temp_s0;
                                        var_t1_5 = 1;
                                        rec.unkC = sp4068;
                                        temp_s4_5 = (var_fp_5 + sp405C) / 2;
                                        temp_s2_6 = (var_s5_7 + sp4060) / 2;
                                        rec.unk10 = temp_s4_5;
                                        rec.unk14 = temp_s2_6;
                                        if (func_80062820(&rec) != 0)
                                        {
                                            rec.unk0 = path[0][0];
                                            rec.unk8 = sp405C;
                                            rec.unkC = sp4060;
                                            if (func_80062820(&rec) != 0)
                                            {
                                                var_fp_5 = temp_s4_5;
                                                var_s5_7 = temp_s2_6;
                                            }
                                        }
                                    }
                                }
                                if (sp407C != 0)
                                {
                                    temp_s3_2 = sp405C - var_fp_5;
                                    var_v0_30 = temp_s3_2;
                                    rec.unk0 = temp_s0;
                                    rec.unk8 = sp4064;
                                    rec.unkC = sp4068;
                                    if (temp_s3_2 < 0)
                                    {
                                        var_v0_30 = temp_s3_2 + 3;
                                    }
                                    temp_s6_3 = var_fp_5 + (var_v0_30 >> 2);
                                    rec.unk10 = temp_s6_3;
                                    temp_s2_7 = sp4060 - var_s5_7;
                                    var_v0_31 = temp_s2_7;
                                    if (temp_s2_7 < 0)
                                    {
                                        var_v0_31 = temp_s2_7 + 3;
                                    }
                                    temp_s4_6 = var_s5_7 + (var_v0_31 >> 2);
                                    rec.unk14 = temp_s4_6;
                                    if (func_80062820(&rec) != 0)
                                    {
                                        rec.unk0 = path[0][0];
                                        rec.unk8 = sp405C;
                                        rec.unkC = sp4060;
                                        if (func_80062820(&rec) != 0)
                                        {
                                            var_t1_5 = 1;
                                            var_fp_5 = temp_s6_3;
                                            var_s5_7 = temp_s4_6;
                                        }
                                    }
                                }
                                if ((var_t1_5 & 0xFF) != 0)
                                {
                                    path[1][1] = var_fp_5;
                                    path[2][1] = var_s5_7;
                                }
                                var_s5_6 = var_s7_3;
                            }
                            if (var_s5_6 >= 0x11U)
                            {
                                var_s5_6 = 0x10;
                            }
                            var_s6_4 = &path[1][var_s7_3 - 1];
                            var_s4_4 = &path[2][var_s7_3 - 1];
                            var_s5_8 = var_s5_6 - 1;
                            var_s3 = 0;
                            if (var_s5_8 != -1)
                            {
                                var_a2_2 = arg2;
                                do
                                {
                                    temp_v1_42 = *var_s6_4;
                                    var_s6_4 -= 1;
                                    var_s3 += 1;
                                    var_s5_8 -= 1;
                                    var_a2_2->unk0 = (temp_v1_42 + ((s32) (arg0->unkC << 0x10) >> 0x11)) << 8;
                                    temp_v1_43 = *var_s4_4;
                                    var_s4_4 -= 1;
                                    var_a2_2->unk4 = (temp_v1_43 + ((s32) (arg0->unk10 << 0x10) >> 0x11)) << 8;
                                    var_a2_2 += 1;
                                } while (var_s5_8 != -1);
                            }
                            return var_s3;
                        }
                    }
                }
                arg2->unk0 = temp_s2->x;
                arg2->unk4 = temp_s2->z;
                return 1;
            }
            return -2;
        }
    }
    return -2;
}

/**
 * @brief Walk a rectangular tile footprint along a straight line and stamp it
 *        into the group tile map.
 *
 * Bresenham DDA over the field group tile grid. The footprint is @c unk18 by
 * @c unk1C tiles anchored at (@c unk8, @c unkC) and is stepped one tile at a
 * time towards (@c unk10, @c unk14). @c state records which edge of the
 * footprint moved on the last step so only the newly covered tiles are
 * re-tested: 1 = a new column, 2 = a new row, 3 = the whole rectangle (the
 * first iteration and both edges crossing at once). Every visited tile must be
 * below 0xFD and must not be 1; anything else aborts the walk.
 *
 * Tiles that pass the test are stamped with @c unk2C, and the stamp value is
 * bumped after each step unless it is 0 or 0xFC. The store happens before the
 * @c unk28 check, so a tile is written even when @c unk28 is not 2 - only the
 * goal comparison is gated on it.
 *
 * @param req Walk request, laid out as @c Req above. @c unk0 is the tile the
 *            walk starts on, @c unk4 the goal tile, @c unk20 the group's tile
 *            pitch (the vertical cell masks use twice that value), and @c unk24
 *            is unused here.
 * @return 1 when the walk ran to completion or reached @c unk4, 0 when a
 *         blocked tile stopped it.
 * @see decomp.me (100%) TODO
 */
s32 func_80062820(Req *req)
{
    FieldScene *scene;
    u8 *base;
    u8 *goal;
    u8 *p;
    u8 *q;
    s32 w;
    s32 h;
    s32 dx;
    s32 dy;
    s32 step_x;
    s32 step_y;
    s32 cell;
    s32 mask_x;
    s32 mask_y;
    s32 x_cell;
    s32 y_cell;
    s32 x_end;
    s32 y_end;
    s32 wm1;
    s32 hm1;
    s32 col_hi;
    s32 row_hi;
    s32 x_span;
    s32 row_span;
    s32 stride;
    s32 kind;
    s32 state;
    s32 err;
    s32 i;
    s32 n;
    s32 m;
    u32 tile;
    u8 mark;

    base = (u8 *)req->unk0;
    x_cell = req->unk8;
    dx = req->unk10 - x_cell;
    y_cell = req->unkC;
    dy = req->unk14 - y_cell;
    cell = req->unk20;
    mask_x = cell - 1;
    x_cell &= mask_x;
    w = req->unk18;
    goal = (u8 *)req->unk4;
    wm1 = w - 1;
    x_end = x_cell + wm1;
    col_hi = x_end & mask_x;
    x_span = x_end >= ((w + mask_x) & ~mask_x);
    h = req->unk1C;
    hm1 = h - 1;
    mask_y = (cell * 2) - 1;
    y_cell &= mask_y;
    y_end = y_cell + hm1;
    row_hi = y_end & mask_y;
    scene = g_field_scene.scene;
    stride = (u16)scene->unk46;
    row_span = 0;
    if (y_end >= ((h + mask_y) & ~mask_y))
    {
        row_span = stride;
    }
    mark = req->unk2C;
    kind = req->unk28;

    if (dx >= 0)
    {
        step_x = 1;
    }
    else
    {
        dx = -dx;
        step_x = -1;
    }
    if (dy >= 0)
    {
        step_y = stride;
    }
    else
    {
        dy = -dy;
        step_y = -stride;
    }

    state = 3;
    if (dx >= dy)
    {
        err = -dx;
        for (i = dx; i != -1; i--)
        {
            if (state != 0)
            {
                switch (state)
                {
                case 1:
                    p = base;
                    if (step_x > 0)
                    {
                        p = base + x_span;
                    }
                    n = row_span;
                    for (;;)
                    {
                        tile = *p;
                        if (tile >= 0xFD || tile == 1)
                        {
                            return 0;
                        }
                        if (mark != 0 && tile != 0xFB)
                        {
                            *p = mark;
                            if (kind == 2 && p == goal)
                            {
                                return 1;
                            }
                        }
                        if (n == 0)
                        {
                            break;
                        }
                        p += stride;
                        n -= stride;
                    }
                    break;
                case 2:
                    p = base;
                    if (step_y > 0)
                    {
                        p = base + row_span;
                    }
                    n = x_span;
                    do
                    {
                        tile = *p;
                        if (tile >= 0xFD || tile == 1)
                        {
                            return 0;
                        }
                        if (mark != 0 && tile != 0xFB)
                        {
                            *p = mark;
                            if (kind == 2 && p == goal)
                            {
                                return 1;
                            }
                        }
                        p++;
                    } while (--n != -1);
                    break;
                case 3:
                    p = base;
                    n = row_span;
                    for (;;)
                    {
                        q = p;
                        m = x_span;
                        do
                        {
                            tile = *q;
                            if (tile >= 0xFD || tile == 1)
                            {
                                return 0;
                            }
                            if (mark != 0 && tile != 0xFB)
                            {
                                *q = mark;
                                if (kind == 2 && q == goal)
                                {
                                    return 1;
                                }
                            }
                            q++;
                        } while (--m != -1);
                        if (n == 0)
                        {
                            break;
                        }
                        p += stride;
                        n -= stride;
                    }
                    break;
                }
                if (mark != 0 && mark != 0xFC)
                {
                    mark++;
                }
            }
            if (i == 0)
            {
                return 1;
            }
            state = 0;
            if (step_x > 0)
            {
                if (x_cell == mask_x)
                {
                    x_cell = 0;
                    x_span--;
                    base++;
                }
                else
                {
                    x_cell++;
                }
                if (col_hi == mask_x)
                {
                    col_hi = 0;
                    x_span++;
                    state = 1;
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
                    base--;
                    state = 1;
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
            err += dy * 2;
            if (err >= 0)
            {
                if (step_y > 0)
                {
                    if (y_cell == mask_y)
                    {
                        y_cell = 0;
                        row_span -= stride;
                        base += stride;
                    }
                    else
                    {
                        y_cell++;
                    }
                    if (row_hi == mask_y)
                    {
                        row_hi = 0;
                        row_span += stride;
                        state |= 2;
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
                        base -= stride;
                        state |= 2;
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
                err -= dx * 2;
            }
        }
    }
    else
    {
        err = -dy;
        for (i = dy; i != -1; i--)
        {
            if (state != 0)
            {
                switch (state)
                {
                case 1:
                    p = base;
                    if (step_x > 0)
                    {
                        p = base + x_span;
                    }
                    n = row_span;
                    for (;;)
                    {
                        tile = *p;
                        if (tile >= 0xFD || tile == 1)
                        {
                            return 0;
                        }
                        if (mark != 0 && tile != 0xFB)
                        {
                            *p = mark;
                            if (kind == 2 && p == goal)
                            {
                                return 1;
                            }
                        }
                        if (n == 0)
                        {
                            break;
                        }
                        p += stride;
                        n -= stride;
                    }
                    break;
                case 2:
                    p = base;
                    if (step_y > 0)
                    {
                        p = base + row_span;
                    }
                    n = x_span;
                    do
                    {
                        tile = *p;
                        if (tile >= 0xFD || tile == 1)
                        {
                            return 0;
                        }
                        if (mark != 0 && tile != 0xFB)
                        {
                            *p = mark;
                            if (kind == 2 && p == goal)
                            {
                                return 1;
                            }
                        }
                        p++;
                    } while (--n != -1);
                    break;
                case 3:
                    p = base;
                    n = row_span;
                    for (;;)
                    {
                        q = p;
                        m = x_span;
                        do
                        {
                            tile = *q;
                            if (tile >= 0xFD || tile == 1)
                            {
                                return 0;
                            }
                            if (mark != 0 && tile != 0xFB)
                            {
                                *q = mark;
                                if (kind == 2 && q == goal)
                                {
                                    return 1;
                                }
                            }
                            q++;
                        } while (--m != -1);
                        if (n == 0)
                        {
                            break;
                        }
                        p += stride;
                        n -= stride;
                    }
                    break;
                }
                if (mark != 0 && mark != 0xFC)
                {
                    mark++;
                }
            }
            if (i == 0)
            {
                return 1;
            }
            state = 0;
            if (step_y > 0)
            {
                if (y_cell == mask_y)
                {
                    y_cell = 0;
                    row_span -= stride;
                    base += stride;
                }
                else
                {
                    y_cell++;
                }
                if (row_hi == mask_y)
                {
                    row_hi = 0;
                    row_span += stride;
                    state = 2;
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
                    base -= stride;
                    state = 2;
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
            err += dx * 2;
            if (err >= 0)
            {
                if (step_x > 0)
                {
                    if (x_cell == mask_x)
                    {
                        x_cell = 0;
                        x_span--;
                        base++;
                    }
                    else
                    {
                        x_cell++;
                    }
                    if (col_hi == mask_x)
                    {
                        col_hi = 0;
                        x_span++;
                        state |= 1;
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
                        base--;
                        state |= 1;
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
                err -= dy * 2;
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
 * horizontal run, so @p vec is multiplied by run / slope to hold the ground
 * speed constant. The run is first shortened by a further 1/16, a flat penalty
 * for travelling on a slope at all.
 *
 * @param def Collision node definition supplying the edge and its heights.
 * @param vec Movement vector rescaled in place; vec[0] is x, vec[1] is y.
 *
 * @note dx and dz are each reused to hold their own square once the raw delta
 *       is no longer needed.
 * @see decomp.me (100%) TODO
 */
void func_80062F48(Move_UnkNode2* def, s32* vec)
{
    s16* tbl;
    s16* pt_c;
    s16* pt_b;
    s32 dx;
    s32 dy;
    s32 dz;
    s32 run;
    s32 slope;

    tbl = g_field_node_angle_table;
    pt_c = &tbl[def->unkA * 2];
    pt_b = &tbl[def->unkC * 2];
    dx = pt_b[0] - pt_c[0];
    dy = pt_b[1] - pt_c[1];
    dz = def->unk12 - def->unk10;
    dx = dx * dx + dy * dy;
    dz = dz * dz;
    run = SquareRoot0(dx);
    slope = SquareRoot0(dx + dz);
    run -= run >> 4;
    vec[0] = vec[0] * run / slope;
    vec[1] = vec[1] * run / slope;
}

/**
 * @brief Convert a probe's footprint to whole tiles and stencil it.
 *
 * scene->unk40 is the tile edge in pixels, either 4 or 8, and @c shift is its
 * base-2 log. The footprint width (unkC) is rounded up to a whole number of
 * tiles and the depth (unk10) to a whole number of double-height tiles, since
 * the depth axis is stored at half the horizontal resolution. The rounding is
 * the usual `(v + n - 1) >> log2(n)` ceiling divide.
 *
 * Does nothing when no per-group work area is allocated.
 *
 * @param q Probe query supplying the footprint extents.
 *
 * @note The extents are read as signed even though Query declares them u16.
 * @see decomp.me (100%) TODO
 */
void func_8006304C(Query* q)
{
    FieldScene* scene;
    s32 shift;
    s32 edge;

    scene = g_field_scene.scene;
    if (scene->unk28 != 0)
    {
        edge = scene->unk40;
        shift = 3;
        if (edge == 4)
        {
            shift = 2;
        }
        func_80060364(((s16) q->unkC + edge - 1) >> shift,
                      ((s16) q->unk10 + edge * 2 - 1) >> (shift + 1));
    }
}

/**
 * @brief Fetch the body of the count'th record on the scene header's list.
 *
 * Walks @p count links from the head, but stops advancing once it reaches the
 * tail, so an index past the end clamps to the last record rather than running
 * off the list. The counter is a u16, so a count of zero walks nothing and
 * returns the head's body.
 *
 * @param count Number of links to walk from the head of the list.
 * @return Pointer to the selected record's body, or NULL if the scene carries
 *         no records at all.
 */
void* func_800630BC(s32 count)
{
    FieldHeaderRec* rec;
    u16 i;

    rec = g_field_scene.scene->header->records;
    if (rec != NULL)
    {
        i = count;
        while (i-- != 0)
        {
            if (rec->next != NULL)
            {
                rec = rec->next;
            }
        }
        return &rec->body;
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
    s32 alloc;

    node = g_field_scene.scene->nodes;
    alloc = D_801ED000;
    while (node != NULL)
    {
        func_8005E3B0((Move_UnkNode1*) node, (u8**) &alloc);
        node = node->next;
    }
    func_8005F158(&alloc);
    D_801ED000 = alloc;
}

/**
 * @brief One entry of the macro table at D_80122B80.
 *
 * A replacement string plus the number of characters it may contribute before
 * the expansion is dropped (see FieldTextState::unk4C).
 */
typedef struct
{
    /** 0x00 character budget for the expansion; -1 means unlimited. */
    u8 unk0;
    u8 _pad1[3];
    /** 0x04 the replacement string itself. */
    u8* unk4;
} FieldTextMacro;

/**
 * @brief Text-window state block, live at 0x801ED0CC.
 *
 * unk0/unk4/unk8 are a three-level cursor stack: unk0 is the script string,
 * unk4 a macro expansion pushed over it, and unk8 a glyph run pushed over
 * that. func_800632E0 always reads from the innermost non-null level.
 */
typedef struct
{
    /** 0x00 script cursor. */
    u8* unk0;
    /** 0x04 macro-expansion cursor, or NULL. */
    u8* unk4;
    /** 0x08 glyph-run cursor, or NULL. */
    u8* unk8;
    s32 unkC;
    /** 0x10 flags; bit 0x30 and 0x800 and 0x1000 are read here. */
    u32 unk10;
    u8 unk14;
    u8 unk15;
    u8 unk16;
    u8 unk17;
    /** 0x18 nesting depth of the 0x07 control code. */
    u8 unk18;
    /** 0x19 pending blank-space count; while non-zero each step emits a space
        instead of consuming a character. */
    u8 unk19;
    u8 unk1A;
    u8 unk1B;
    /** 0x1C set to 0x10 when the text has run out of room and the step ends. */
    u8 unk1C;
    u8 _pad1D;
    u8 unk1E;
    u8 unk1F;
    /** 0x20 inline string buffer, pushed as an expansion by control code 15. */
    u8 unk20[0x49 - 0x20];
    /** 0x49 set when the last emitted character was a break opportunity. */
    u8 unk49;
    u8 _pad4A[2];
    /** 0x4C remaining character budget of the active macro expansion; -1
        disables the countdown. */
    s16 unk4C;
    u8 _pad4E[4];
    u16 unk52;
    u8 _pad54[2];
    /** 0x56 horizontal advance applied when starting a new line. */
    u16 unk56;
    u16 unk58;
    /** 0x5A space left on the current line, in the same units as the glyph
        widths from D_801E26E0. */
    u16 unk5A;
    u16 unk5C;
    u16 unk5E;
    /** 0x60 left edge of the live text region, in quarter-pixel units. */
    u16 unk60;
    /** 0x62 top row of the live text region, in staging-buffer rows. */
    u16 unk62;
    /** 0x64 width of the last row of the live text region. */
    u16 unk64;
    /** 0x66 bottom row of the live text region. */
    u16 unk66;
    s16 unk68;
    s16 unk6A;
    s16 unk6C;
    s16 unk6E;
    /** 0x70 per-row carry buffer: the halfword of each glyph row that spilled
        past the right edge of the previous staging block, one entry per row. */
    u16 unk70[16];
} FieldTextState;

extern FieldTextMacro D_80122B80[];
/** Per-character advance widths, indexed by character code below 0x80. */
extern u8 D_801E26E0[];

void func_8006429C(FieldTextState*);
void func_8006700C(FieldTextState*, s32, u8);
void func_80063B6C(FieldTextState*, s32, u16);
s32 func_80064210(FieldTextState*);

/**
 * @brief Upload the staging buffer at 0x801DE000 to VRAM as a 12-row image.
 *
 * The image is @c count halfwords wide, derived from the two halfwords at
 * 0x801ED0CC, and is stored in the staging buffer as rows of stride 64.
 *
 * Whole 64-wide blocks go up first, stacked into a single 64 x (12 * blocks)
 * rectangle that can be read straight out of the linear buffer; the VRAM y and
 * the buffer cursor both advance past it, leaving only the remainder columns in
 * @c count. Those remaining 12 rows still sit at stride 64, so rows 1..11 are
 * packed down to stride @c count in place (row 0 is already where it belongs)
 * before the closing @c count x 12 upload.
 *
 * @note The shift by 6 is deliberately left inside the two expressions that use
 *       it rather than hoisted into a local, and the leading subtraction is a
 *       variable of its own; both are required to match.
 */
void func_80063194(void)
{
    RECT rect;
    u16* buf;
    u16* src;
    u16* dst;
    s32 count;
    s32 adj;
    s32 diff;
    s32 i;
    s32 j;

    rect.x = 0x3C0;
    rect.y = 0x180;
    diff = ((FieldTextState*) 0x801ED0CC)->unk52 - ((FieldTextState*) 0x801ED0CC)->unk5A;
    count = ((diff & 3) + diff + 5) >> 2;
    buf = (u16*) 0x801DE000;
    if (count >= 0x40)
    {
        rect.w = 0x40;
        adj = count;
        if (count < 0)
        {
            adj = count + 0x3F;
        }
        rect.h = (adj >> 6) * 12;
        LoadImage(&rect, (u_long*) 0x801DE000);
        buf += rect.w * rect.h;
        count -= (adj >> 6) * 64;
        rect.y = rect.y + rect.h;
    }
    if (count > 0)
    {
        dst = buf + count;
        src = buf + 0x40;
        j = 11;
        while (--j != -1)
        {
            i = count;
            while (--i != -1)
            {
                *dst++ = *src++;
            }
            src += 0x40 - count;
        }
        rect.w = count;
        rect.h = 0xC;
        LoadImage(&rect, (u_long*) buf);
    }
}

/**
 * @brief Typeset one step of the field text window.
 *
 * Walks the innermost active cursor interpreting control codes below 0x20 and
 * emitting each glyph through func_80063B6C. Codes 0x20 and above are literal
 * characters; 0x19 introduces a two-byte code. Control codes push and pop the
 * cursor stack (14 pushes a macro from D_80122B80, 15 pushes the inline buffer
 * at unk20, 0 and 6 pop), set pending delays, or end the step.
 *
 * Before emitting a character the routine word-wraps: if the character is a
 * break opportunity it runs a LOOKAHEAD that re-walks the same control-code
 * alphabet over a private copy of the cursor stack, accumulating the width of
 * the next word, and asks func_80064210 for a new line when that word will not
 * fit on the current one.
 *
 * @param st Text-window state; its cursor stack is advanced in place.
 * @param arg1 Budget of characters to emit before returning.
 *
 * @see decomp.me (97.19%) TODO
 */
void func_800632E0(FieldTextState* st, s32 arg1)
{
    u8* cur;
    u8* look;
    u8* look_str;
    u8* look_exp;
    u8* look_run;
    s32 remaining;
    s32 advance;
    s32 fresh;
    s32 first;
    s32 look_adv;
    s32 emit_w;
    u16 code;
    u16 width;
    u16 look_code;
    u16 look_width;
    u16 look_count;
    u32 v1;
    u16 y;
    u32 x;
    s16 tmp;
    u8 c;
    u8 look_c;
    u8 flag;
    s32 four;
    FieldTextMacro* rec;
    u16 nc;
    u16 nc2;

    remaining = arg1;
    width = 0;
    advance = 0;
    first = 1;
    y = st->unk5E;
    x = (st->unk5C + st->unk52) - st->unk5A;
    while (x >= 0x100)
    {
        x -= 0x100;
        y += st->unk58;
    }
    tmp = x & 0xFFFC;
    st->unk6C = tmp;
    st->unk68 = tmp;
    st->unk6E = y;
    st->unk6A = y;
    if (st->unk52 == st->unk5A)
    {
        st->unk49 = 0;
        fresh = 1;
    }
    else
    {
        fresh = 0;
    }
    four = 4;

    while (1)
    {
        cur = st->unk8;
        if (cur == NULL)
        {
            cur = st->unk4;
            if (cur == NULL)
            {
                cur = st->unk0;
            }
        }
        code = 0;

        while (1)
        {
            if (st->unk19 != 0)
            {
                code = 0x20;
                width = 5;
                advance = 0;
                st->unk19 = st->unk19 - 1;
            }
            else
            {
                c = *cur;
                cur++;
                if ((c < 0x20) && (c != 0x19))
                {
                    switch (c)
                    {
                    case 0:
                        if (st->unk8 != NULL)
                        {
                            goto pop_run;
                        }
                        if (st->unk4 != NULL)
                        {
                            goto pop_expand;
                        }
                        if (st->unk18 != 0)
                        {
                            goto set_wide;
                        }
                        st->unk14 = 1;
                        goto set_break;
                    case 6:
                        if (st->unk8 != NULL)
                        {
                        pop_run:
                            cur = st->unk4;
                            st->unk8 = NULL;
                            if (cur == NULL)
                            {
                                cur = st->unk0;
                                v1 = code;
                                goto have_v1;
                            }
                            break;
                        }
                        if (st->unk4 != NULL)
                        {
                        pop_expand:
                            cur = st->unk0;
                            st->unk4 = NULL;
                            break;
                        }
                        st->unk0 = NULL;
                        if (st->unk10 & 0x1000)
                        {
                            func_8006700C(st, 1, c);
                        }
                        return;
                    case 1:
                        if (func_80064210(st) == 1)
                        {
                            goto store_and_return;
                        }
                        fresh = 1;
                        st->unk49 = 0;
                        break;
                    case 2:
                        st->unk14 = 3;
                        goto set_break;
                    case 3:
                        st->unk14 = 2;
                        goto set_break;
                    case 4:
                        if (first == 0)
                        {
                            return;
                        }
                        func_8006429C(st);
                        goto store_and_return;
                    case 5:
                        st->unk14 = four;
                        goto set_break;
                    case 7:
                        if (st->unk18 == 0)
                        {
                            st->unk16 = st->unk15;
                        }
                        st->unk18 = st->unk18 + 1;
                        break;
                    case 8:
                        st->unk19 = 2;
                        break;
                    case 9:
                        st->unk19 = 3;
                        break;
                    case 10:
                        st->unk19 = four;
                        break;
                    case 11:
                        st->unk19 = *cur;
                        cur++;
                        break;
                    case 12:
                        st->unk1A = four;
                        goto store_and_return;
                    case 13:
                        st->unk1A = *cur;
                        cur++;
                        goto store_and_return;
                    case 14:
                        c = *cur;
                        cur++;
                        st->unk0 = cur;
                        rec = &D_80122B80[c];
                        cur = rec->unk4;
                        st->unk4 = cur;
                        st->unk4C = rec->unk0;
                        break;
                    case 15:
                        st->unk0 = cur;
                        st->unk4 = st->unk20;
                        cur = st->unk20;
                        st->unk4C = -1;
                        break;
                    case 16:
                        st->unk1B = *cur;
                        cur++;
                        break;
                    case 17:
                        st->unk1B = 0;
                        break;
                    case 19:
                        v1 = code;
                        if (fresh != 0)
                        {
                            code = 0xFFFF;
                            width = 0xC;
                            advance = 1;
                            break;
                        }
                        goto have_v1;
                    case 18:
                        c = *cur;
                        cur++;
                        if (c == 0)
                        {
                            code = 0x20;
                            width = 5;
                            advance = 2;
                        }
                        /* fallthrough */
                    case 31:
                        c = *cur + 0x1F;
                        cur++;
                        /* fallthrough */
                    default:
                        if (st->unk4 != NULL)
                        {
                            st->unk4 = cur;
                        }
                        else
                        {
                            st->unk0 = cur;
                        }
                        st->unk8 = (u8*) 0x801E2780 + ((u16*) 0x801E2758)[c];
                        cur = st->unk8;
                        break;
                    }
                }
                else
                {
                    if (c >= 0x20)
                    {
                        code = c;
                        advance = 1;
                    }
                    else
                    {
                        code = *cur | ((c + 0xFFE8) << 8);
                        cur++;
                        advance = 2;
                    }

                    v1 = code;
                    if (v1 == 0x80)
                    {
                        width = 0xC;
                    }
                    else if (v1 >= 0x80)
                    {
                        width = 9;
                    }
                    else
                    {
                        width = D_801E26E0[v1];
                    }
                }
            }

            v1 = code;

        have_v1:
            if ((v1 == 0x20) || (v1 == 0x80))
            {
                if (st->unk5A < width)
                {
                    st->unk49 = 1;
                    code = 0;
                }
            }
            if (code == 0)
            {
                continue;
            }
            if (st->unk5A < width)
            {
                if (func_80064210(st) == 1)
                {
                    return;
                }
                fresh = 1;
                st->unk49 = 0;
            }
            flag = st->unk49;
            if ((code == 0x20) || (code == 0x80) || (code == 0xFFFF))
            {
                st->unk49 = 1;
                break;
            }
            look_width = width;
            if (flag != 0)
            {
                look_adv = advance;
                look = cur;
                look_str = st->unk0;
                look_exp = st->unk4;
                look_run = st->unk8;
                look_count = st->unk4C;
                do
                {
                    if (look_run != NULL)
                    {
                        look_run = look;
                    }
                    else if (look_exp != NULL)
                    {
                        nc = look_count - look_adv;
                        if ((s16) look_count != -1)
                        {
                            look_count = nc;
                            if ((nc << 16) <= 0)
                            {
                                look = NULL;
                            }
                        }
                        look_exp = look;
                    }
                    else
                    {
                        look_str = look;
                    }
                    look = look_run;
                    look_code = 0;
                    if (look == NULL)
                    {
                        look = look_str;
                        if (look_exp != NULL)
                        {
                            look = look_exp;
                        }
                    }
                    while (1)
                    {
                        look_c = *look;
                        look++;
                        if ((look_c < 0x20) && (look_c != 0x19))
                        {
                            switch (look_c)
                            {
                            case 0:
                            case 6:
                                if (look_run != NULL)
                                {
                                    look_run = NULL;
                                    look = look_str;
                                    if (look_exp != NULL)
                                    {
                                        look = look_exp;
                                    }
                                }
                                else if (look_exp != NULL)
                                {
                                    look_exp = NULL;
                                    look = look_str;
                                }
                                else
                                {
                                    flag = 0;
                                }
                                break;
                            case 14:
                                look_str = look + 1;
                                look_c = *look;
                                rec = &D_80122B80[look_c];
                                look = rec->unk4;
                                look_count = rec->unk0;
                                look_exp = look;
                                break;
                            case 15:
                                look_str = look;
                                look = st->unk20;
                                look_exp = look;
                                look_count = -1;
                                break;
                            case 18:
                                look_c = *look;
                                look++;
                                if (look_c == 0)
                                {
                                    flag = 0;
                                }
                                /* fallthrough */
                            case 31:
                                look_c = *look + 0x1F;
                                look++;
                                /* fallthrough */
                            default:
                                if (look_exp != NULL)
                                {
                                    look_exp = look;
                                }
                                else
                                {
                                    look_str = look;
                                }
                                look = (u8*) 0x801E2780 + ((u16*) 0x801E2758)[look_c];
                                look_run = look;
                                break;
                            }
                            if (look_code != 0)
                            {
                                break;
                            }
                            if (flag != 0)
                            {
                                continue;
                            }
                            break;
                        }
                        if (look_c >= 0x20)
                        {
                            look_code = look_c;
                            look_adv = 1;
                        }
                        else
                        {
                            look_code = *look | ((look_c + 0xFFE8) << 8);
                            look++;
                            look_adv = 2;
                        }
                        if (look_code != 0)
                        {
                            if ((look_code == 0x20) || (look_code == 0x80) || (look_code == 0xFFFF))
                            {
                                flag = 0;
                            }
                            else if (look_code >= 0x80)
                            {
                                look_width += 9;
                            }
                            else
                            {
                                look_width += D_801E26E0[look_code];
                            }
                            if (look_code != 0)
                            {
                                break;
                            }
                        }
                        if (flag == 0)
                        {
                            break;
                        }
                    }
                } while (flag != 0);

                if (st->unk5A >= look_width)
                {
                    break;
                }
                if (func_80064210(st) != 1)
                {
                    fresh = 1;
                    st->unk49 = 0;
                    break;
                }
                return;
            }
            break;
        }

        if (st->unk8 != NULL)
        {
            st->unk8 = cur;
        }
        else if (st->unk4 != NULL)
        {
            if ((s16) st->unk4C != -1)
            {
                nc2 = st->unk4C - advance;
                st->unk4C = nc2;
                if ((nc2 << 16) <= 0)
                {
                    cur = NULL;
                }
            }
            st->unk4 = cur;
        }
        else
        {
            st->unk0 = cur;
        }
        emit_w = width;
        if (code == 0xFFFF)
        {
            code = 0x20;
            width = 0xC;
            if ((st->unkC == 0) || (st->unk10 & 0x30))
            {
                func_80063B6C(st, 0x20, 0xC);
            }
            emit_w = width;
        }
        if (emit_w != 0)
        {
            func_80063B6C(st, code, emit_w);
        }
        if ((remaining != 0) && !(st->unk10 & 0x800) && ((fresh == 0) || (code != 0x20)))
        {
            first = 0;
            remaining--;
            fresh = 0;
            if (remaining == 0)
            {
                return;
            }
        }
    }

set_wide:
    st->unk14 = 0x10;
    st->unk1E = 0;
    st->unk1F = 4;
    st->unk17 = 0;
    goto store_and_return;

set_break:
    st->unk1E = 0;
    st->unk1F = 8;

store_and_return:
    if (st->unk8 != NULL)
    {
        st->unk8 = cur;
        return;
    }
    if (st->unk4 != NULL)
    {
        st->unk4 = cur;
        return;
    }
    st->unk0 = cur;
}

/**
 * @brief Blit one glyph into the text window's 4bpp staging buffer.
 *
 * Expands the 1bpp font bitmap for @p code (0x18 bytes per character at
 * 0x801E1200, one halfword per row) into 4bpp pixels, applying a drop shadow,
 * and merges the result into the 64-halfword-wide staging image at 0x801DE000
 * that func_80063194 later uploads to VRAM.
 *
 * The expansion runs through a small staging area in the PSX scratchpad at
 * 0x1F800000, laid out as one 10-byte (5 halfword) row per glyph row. Each row
 * is primed from FieldTextState::unk70, the carry left over from the previous
 * glyph, then filled nibble by nibble; @c st->unk52 - @c st->unk5A gives the
 * sub-block pixel offset, so a glyph may straddle two 64-wide blocks. What
 * runs past the right edge is written back to unk70 for the next call.
 *
 * Two colour indices are used per glyph: an even "fill" index and the odd
 * index above it for the shadow, selected from @c st->unk1B (or forced to 6/7
 * when @c st->unk10 has the 0xC0 field equal to 0x40, which also widens the
 * glyph by one nibble and takes a heavier two-tap shadow).
 *
 * @param st    text-window state block (live at 0x801ED0CC).
 * @param code  character code to draw; the font table is indexed from 0x20.
 * @param width advance width of this glyph, in quarter-pixel units.
 *
 * @see decomp.me (95.46%) scratch not yet published
 */
void func_80063B6C(FieldTextState* st, s32 code, u16 width)
{
    u16* scratch;
    u16* carry;
    u16* glyph;
    u8* line;
    u16* dst;
    u16* row_src;
    u16* row_dst;
    u8* px;
    s32 rows;
    s32 y;
    s32 shift;
    s32 i;
    s32 j;
    s32 r;
    s32 f;
    s32 m;
    s32 count;
    s32 col;
    s32 x;
    s32 words;
    s32 lo_fill;
    s32 hi_fill;
    s32 lo_shadow;
    s32 hi_shadow;
    u32 nibbles;
    u32 left;
    u32 mask;
    u32 fill;
    u32 shade;
    u32 acc;
    u32 cur;
    u32 next;
    u32 avail;
    u32 span;
    s32 nib;

    scratch = (u16*) 0x1F800000;
    carry = st->unk70;
    rows = st->unk58;
    shift = st->unk52 - st->unk5A;
    f = rows - 1;
    for (m = f; m != -1; m--)
    {
        count = 4;
        if (shift != 0)
        {
            *scratch++ = *carry++;
        }
        else
        {
            count = 5;
        }
        for (j = count - 1; j != -1; j--)
        {
            *scratch++ = 0;
        }
    }

    lo_fill = 6;
    if ((st->unk10 & 0xC0) == 0x40)
    {
        hi_fill = 0x60;
        lo_shadow = 7;
        hi_shadow = 0x70;
        nibbles = (u16) width + 2;
    }
    else
    {
        switch (st->unk1B)
        {
        case 0:
            lo_fill = 2;
            hi_fill = 0x20;
            lo_shadow = 3;
            hi_shadow = 0x30;
            break;
        case 1:
            lo_fill = 4;
            hi_fill = 0x40;
            lo_shadow = 5;
            hi_shadow = 0x50;
            break;
        case 2:
            lo_fill = 6;
            hi_fill = 0x60;
            lo_shadow = 7;
            hi_shadow = 0x70;
            break;
        case 3:
            lo_fill = 8;
            hi_fill = 0x80;
            lo_shadow = 9;
            hi_shadow = 0x90;
            break;
        case 4:
            lo_fill = 0xA;
            hi_fill = 0xA0;
            lo_shadow = 0xB;
            hi_shadow = 0xB0;
            break;
        case 5:
            lo_fill = 0xC;
            hi_fill = 0xC0;
            lo_shadow = 0xD;
            hi_shadow = 0xD0;
            break;
        default:
            lo_fill = 0xE;
            hi_fill = 0xE0;
            lo_shadow = 0xF;
            hi_shadow = 0xF0;
            break;
        }
        nibbles = (u16) width + 1;
    }

    glyph = (u16*) (0x801E1200 + (((u16) code - 0x20) * 0x18));
    px = (u8*) (0x1F800000 + (((u32) shift & 3) >> 1));
    fill = 0;
    shade = fill;
    acc = fill;
    for (i = rows - 1; i != -1; i--)
    {
        u8* p = px;

        nib = shift & 1;
        mask = 0x8000;
        if ((st->unk10 & 0xC0) == 0x40)
        {
            cur = 0;
            if (i != 0)
            {
                next = *glyph;
                cur = (next & 0xFFFF) >> 1;
                next |= (next & 0xFFFF) >> 2;
                acc |= next;
                next |= cur;
                shade |= next;
            }
            else
            {
                next = cur;
            }
            for (j = nibbles - 1; j != -1; j--)
            {
                if (nib == 0)
                {
                    if ((shade & mask) != 0)
                    {
                        *p = lo_shadow | (*p & 0xF0);
                    }
                    nib = 1;
                    if ((fill & mask) != 0)
                    {
                        *p = lo_fill | (*p & 0xF0);
                    }
                }
                else
                {
                    if ((shade & mask) != 0)
                    {
                        *p = hi_shadow | (*p & 0xF);
                    }
                    nib = 0;
                    if ((fill & mask) != 0)
                    {
                        *p = hi_fill | (*p & 0xF);
                    }
                    p++;
                }
                mask >>= 1;
            }
            shade = acc;
            fill = cur;
        }
        else
        {
            cur = *glyph;
            next = cur >> 1;
            acc |= next;
            next |= cur;
            for (j = nibbles - 1; j != -1; j--)
            {
                if (nib == 0)
                {
                    if ((acc & mask) != 0)
                    {
                        *p = lo_shadow | (*p & 0xF0);
                    }
                    nib = 1;
                    if ((cur & mask) != 0)
                    {
                        *p = lo_fill | (*p & 0xF0);
                    }
                }
                else
                {
                    if ((acc & mask) != 0)
                    {
                        *p = hi_shadow | (*p & 0xF);
                    }
                    nib = 0;
                    if ((cur & mask) != 0)
                    {
                        *p = hi_fill | (*p & 0xF);
                    }
                    p++;
                }
                mask >>= 1;
            }
        }
        glyph++;
        acc = next;
        px += 10;
    }

    y = st->unk5E;
    x = st->unk5C + shift;
    while (x >= 0x100)
    {
        x -= 0x100;
        y += rows;
    }

    if ((st->unk10 & 0xC0) == 0x40)
    {
        avail = st->unk5A + 4;
    }
    else
    {
        avail = st->unk5A;
    }
    if (avail < nibbles)
    {
        span = avail + (shift & 3);
    }
    else
    {
        span = nibbles + (shift & 3);
    }
    left = (span + 3) >> 2;

    scratch = (u16*) 0x1F800000;
    if (left != 0)
    {
        line = (u8*) 0x801DE000 - (-(y << 7));
        do
        {
            col = x >> 2;
            dst = (u16*) (line + col * 2);
            if ((u32) (col + left) >= 0x41U)
            {
                words = 0x40 - col;
                left -= words;
                x = 0;
                m = rows << 7;
                line += m;
                y += rows;
            }
            else
            {
                x += left * 4;
                words = left;
                left = 0;
            }
            row_src = scratch;
            for (r = rows - 1; r != -1; r--)
            {
                u16* s = row_src;

                row_dst = dst;
                for (j = words - 1; j != -1; j--)
                {
                    *row_dst++ = *s++;
                }
                row_src += 5;
                dst += 0x40;
            }
            scratch += words;
        } while (left != 0);
    }

    st->unk6C = x;
    scratch = (u16*) 0x1F800000 + (((shift & 3) + (u16) width) >> 2);
    carry = st->unk70;
    st->unk6E = y;
    for (f = rows - 1; f != -1; f--)
    {
        *carry++ = *scratch;
        scratch += 5;
    }
    st->unk5A = st->unk5A - width;
}

/**
 * @brief Erase the region the previous text step occupied in the staging buffer.
 *
 * The live text region is described by unk60/unk62 (left edge and top row) and
 * unk64/unk66 (last-row width and bottom row); the staging image at 0x801DE000
 * is 64 halfwords wide, so one row is 0x40 halfwords. Three passes clear it:
 * the rows of the current block from the left edge across, then any whole rows
 * between the block and the bottom, then the partial last row at the bottom.
 * The current rectangle is then snapshotted into unk68..unk6E.
 *
 * @param st text-window state block (live at 0x801ED0CC).
 */
void func_800640B4(FieldTextState* st)
{
    u16* row;
    u16* p;
    s32 span;
    s32 half;
    s32 x;
    s32 y;
    s32 rows;
    s32 i;
    s32 j;

    y = st->unk62;
    x = st->unk60;
    if (y == st->unk66)
    {
        span = st->unk64 - x;
        half = span >> 1;
    }
    else
    {
        span = 0x100 - x;
        half = span >> 1;
    }
    rows = st->unk58;
    row = ((u16*) 0x801DE000 + (x >> 2)) + (y << 6);
    for (i = rows - 1; i != -1; i--)
    {
        p = row;
        j = half >> 1;
        while (--j != -1)
        {
            *p++ = 0;
        }
        row += 0x40;
    }

    if (y != st->unk66)
    {
        y += rows;
        if (y != st->unk66)
        {
            row = (u16*) 0x801DE000 + (y << 6);
            i = (st->unk66 - y) << 6;
            while (--i != -1)
            {
                *row++ = 0;
            }
        }
        if (st->unk64 != 0)
        {
            row = (u16*) 0x801DE000 + (st->unk66 << 6);
            for (i = rows - 1; i != -1; i--)
            {
                p = row;
                j = st->unk64 >> 2;
                while (--j != -1)
                {
                    *p++ = 0;
                }
                row += 0x40;
            }
        }
    }
    st->unk68 = st->unk60;
    st->unk6A = st->unk62;
    st->unk6C = st->unk64;
    st->unk6E = st->unk66;
}

/**
 * @brief Advance the text cursor to the next line, or report the window full.
 *
 * Adds the line advance (unk56) to the horizontal cursor (unk5C) and carries
 * every whole 0x100 into the vertical cursor (unk5E), one text row (unk58) per
 * carry. If the cursor lands exactly on the end of the live region
 * (unk64/unk66) there is no room left: unk1C is set to 0x10 and the caller is
 * told to stop. Otherwise the new cursor is committed, the remaining width on
 * the line is reset from unk52, and the line counter unk15 is bumped.
 *
 * @param st text-window state block (live at 0x801ED0CC).
 * @return 1 when the window is full and the step must end, 0 to keep going.
 */
s32 func_80064210(FieldTextState* st)
{
    u16 x;
    u16 y;

    y = st->unk5E;
    x = st->unk5C + st->unk56;
    while (x >= 0x100)
    {
        x -= 0x100;
        y += st->unk58;
    }
    if ((y == st->unk66) && (x == st->unk64))
    {
        st->unk1C = 0x10;
        return 1;
    }
    st->unk5C = x;
    st->unk5E = y;
    st->unk5A = st->unk52;
    st->unk15 = st->unk15 + 1;
    return 0;
}
