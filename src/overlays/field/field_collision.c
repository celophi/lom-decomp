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
