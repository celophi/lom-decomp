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

void func_80062F48(void*, s32*);                        /* extern */

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
 * @note local objdiff 80.21% (gcc280_g4_noexpanddiv), 2026-07-02 - active
 *       matching scratch is working/func_8005B6AC.c.
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
    s32 var_a3_2;
    s16 var_s0_3;
    s16 var_s1_3;
    s32 e_x_min;
    s32 f_x_min;
    s32 f_x_max;
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
            var_fp = sp2C;
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
                                            var_t8 = 1;
                                            break;
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
                                            var_t8 = 1;
                                            break;
                                        }
                                    } while (--var_s0_2 != -1);
                                }
                            }
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
                                                var_s2 = temp_v1_5;
                                                a0->unk1C = var_fp;
                                            }
                                        }
                                    }
                                    break;
                                case 1:             /* switch 1 */
                                    if ((((Move_UnkNode2*)temp_s6)->unk14 + (s16) temp_v0) < (s16) sp38) {
                                        temp_s0 = func_8005DFAC(var_fp, &probe.x);
                                        if (var_a3 != 0) {
                                            var_a3 = 1;
                                            if (var_s2 < temp_s0) {
                                                var_s2 = temp_s0;
                                                a0->unk1C = var_fp;
                                            }
                                        } else {
                                            var_a3 = 1;
                                            if (var_s2 < (temp_s0 + 0x14)) {
                                                var_s2 = temp_s0;
                                                a0->unk1C = var_fp;
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
        if ((sp20 == 0) && (var_t8 == 0)) {
            var_v1 = a0->unk8 + a0->unk14;
            a0->unk0 = (s32) (a0->unk0 + a0->unkC);
            a0->unk8 = var_v1;
        } else {
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
                                var_s4_2 = (u8*)((Move_UnkNode1*)var_fp)->unk14 + (temp_lo_4 * 2);
                                if (var_s0_9 != -1) {
                                    temp_s7 = f_x_max - 1;
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
                                                                if (temp_a1_5 >= f_x_max) {
                                                                    var_a0 = temp_v1_18 - f_x_max;
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
                                                            if ((f_x_min < temp_v1_20) && ((temp_a1_5 + spAC) >= f_x_max)) {
                                                                var_a0 = temp_v1_20 - f_x_max;
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
                                                                if (var_a3_2 != 0x8000) {
                                                                    if (var_a1 > 0) {
                                                                        if (var_a3_2 >= 0) {
                                                                            if (var_a3_2 < var_a1) {
                                                                                var_a3_2 = var_a1;
                                                                            }
                                                                        } else {
                                                                            var_a3_2 = 0x8000;
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
                if ((var_t0 == 0) || (var_t0 == 0x8000)) {
                    if (var_a3_2 != 0x8000) {
                        sp60 += temp_a1_6;
                    }
                    break;
                } else if ((var_a3_2 == 0) || (var_a3_2 == 0x8000)) {
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
                if ((((Move_UnkNode3*)temp_a2_3)->unk2C & 2) && ((temp_a0_16 = (u16) a0->unk24, temp_a1_7 = (u16) a0->unk28, temp_a3_2 = (u16) probe.x - ((s32) ((s16) temp_a0_16 + ((u32) (temp_a0_16 << 0x10) >> 0x1F)) >> 1), temp_v0_14 = (u16) probe.y - ((s32) ((s16) temp_a1_7 + ((u32) (temp_a1_7 << 0x10) >> 0x1F)) >> 1), (temp_v0_14 & 0x8000)) || ((s16) (temp_v0_14 + temp_a1_7) >= ((Move_UnkNode3*)temp_a2_3)->unk32) || (temp_a3_2 & 0x8000) || ((s16) (temp_a3_2 + temp_a0_16) >= ((Move_UnkNode3*)temp_a2_3)->unk30))) {
                    var_t8 = 1;
                }
            }
            if ((sp20 == 0) && (var_t8 == 0)) {
                a0->unk0 = (s32) (a0->unk0 + sp5C);
                var_v1 = a0->unk8 + sp60;
                a0->unk8 = var_v1;
            } else {
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
 * @see decomp.me (59.18%) TODO
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
    s8 *out;
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
    s32 var_v0;
    u32 var_s0;
    u32 var_t3_2;
    u32 var_a3_2;
    u32 var_t3_3;
    u32 var_t1_2;
    s32 var_a2_5;
    u32 temp_a0_4;
    u32 temp_v0_3;
    u32 temp_v1_2;
    s8 var_v1;
    u32 temp_v0_4;
    u32 temp_v1_3;
    u32 var_t3_4;
    u32 var_t1_3;
    s8 var_v1_2;
    s8 var_v1_3;
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
    out = (s8 *) scene->unk2C;
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
            var_a2 = (4 - (s32) out) & 3;
            var_t8 = (u16) scene->unk46 * 2;
            if (var_a2 != 0)
            {
loop_6:
                if (var_t8 != 0)
                {
                    *out = -1;
                    out += 1;
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
                    *(s32 *) out = -1;
                    var_a2_2 -= 1;
                    out += 4;
                } while (var_a2_2 != -1);
            }
            var_a2_3 = (var_t8 & 3) - 1;
            if (var_a2_3 != -1)
            {
                do
                {
                    *out = -1;
                    var_a2_3 -= 1;
                    out += 1;
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
                sp30 = sp10 * arg1 * 4;
                do
                {
                    temp_s1 = sp4;
                    sp4 = temp_s1 + (spC * 4);
                    temp_t2 = *(u32 *) (temp_s1 + 0);
                    temp_t0 = *(u32 *) (temp_s1 + 4);
                    var_t8_2 = ((u16) scene->unk46 - 1) - arg0;
                    var_s1 = temp_s1 + 8;
                    if (sp1C < 5U)
                    {
                        switch (sp1C)
                        {
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
                                goto block_25;
                            case 0:
                            default:
                                var_t6 = 0;
                                var_t3 = temp_t0;
                                var_t1 = temp_t2;
                                var_t4 = 0;
                                var_t5 = 0;
                                break;
                        }
                    }
                    else
                    {
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
block_25:
                        var_t4 = temp_t0;
                        var_t5 = temp_t2;
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
                    var_v0 = arg1 - 1;
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
                        if (arg1 == 1)
                        {
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
                                *out = var_v1_3;
                                out += 1;
                                var_t3_2 = var_t3_2 >> 1;
                                var_t0 -= 1;
                                var_t1 = var_t1 >> 1;
                            } while (var_t0 != 0);
                        }
                        else if (arg1 == 2)
                        {
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
                                    *out = var_v1_2;
                                    out += 1;
                                    var_t3_4 = var_t3_4 >> 1;
                                    var_t0 -= 1;
                                    var_t1_3 = var_t1_3 >> 1;
                                } while (var_t0 != 0);
                            }
                        }
                        else
                        {
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
                                    *out = var_v1;
                                    out += 1;
                                    var_t3_3 = var_t3_3 >> 1;
                                    var_t0 -= 1;
                                    var_t1_2 = var_t1_2 >> 1;
                                } while (var_t0 != 0);
                            }
                        }
                        var_v0 = arg1 - 1;
                        if (var_t8_2 != 0)
                        {
                            temp_t2_2 = *(u32 *) (var_s1 + 0);
                            temp_t0_2 = *(u32 *) (var_s1 + 4);
                            var_s1 += 8;
                            if (sp1C < 5U)
                            {
                                switch (sp1C)
                                {
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
block_95:
                                            var_t1 |= var_v0_2;
                                            goto block_96;
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
                                            goto block_95;
                                        }
                                        goto block_97;
                                    case 0:
                                    default:
                                        var_t1 = temp_t2_2;
                                        var_t3 = temp_t0_2;
                                        break;
                                }
                            }
                            else
                            {
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
block_96:
                                    var_t4 = temp_t0_2;
                                    var_t5 = temp_t2_2;
                                }
                                else
                                {
block_97:
                                    var_t4 = 0;
                                    var_t5 = 0;
                                }
                            }
                            var_t0 = 0x20;
                            if (var_t8_2 == 0)
                            {
                                var_v0 = arg1 - 1;
                            }
                            else
                            {
                                goto loop_38;
                            }
                        }
                    }
                    if (var_s4 >= var_v0)
                    {
                        var_t8_3 = arg0;
                        if (var_t8_3 != -1)
                        {
                            do
                            {
                                *out = -1;
                                var_t8_3 -= 1;
                                out += 1;
                            } while (var_t8_3 != -1);
                        }
                    }
                    var_s4 += 1;
                    temp_s6_5 = sp14 - 1;
                    sp14 = temp_s6_5;
                } while (temp_s6_5 != -1);
            }
            var_a2_9 = (4 - (s32) out) & 3;
            var_t8_4 = (u16) scene->unk46 * (arg1 + 1);
            if (var_a2_9 != 0)
            {
loop_107:
                if (var_t8_4 != 0)
                {
                    *out = -1;
                    out += 1;
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
                    *(s32 *) out = -1;
                    var_a2_10 -= 1;
                    out += 4;
                } while (var_a2_10 != -1);
            }
            var_a2_11 = (var_t8_4 & 3) - 1;
            if (var_a2_11 != -1)
            {
                do
                {
                    *out = -1;
                    var_a2_11 -= 1;
                    out += 1;
                } while (var_a2_11 != -1);
            }
            temp_s6_4 = sp8 - 1;
            sp8 = temp_s6_4;
        } while (temp_s6_4 != -1);
    }
}
