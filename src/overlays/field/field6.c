#include "common.h"

typedef struct {
    u8 _pad[0x2C];
    s32 unk2C;
    s32 unk30;
} FieldState;

typedef struct Node
{
    struct Node *unk0;   /* 0x00 next pointer */
    s32 key;             /* 0x04 compared by func_8005B31C */
    u8 _pad[0x10];       /* 0x08-0x17 */
    s8 unk18;            /* 0x18 byte written by func_8005B228 */
} Node;

struct CollNode;

typedef struct
{
    u8 _pad0[4];           /* 0x00-0x03 */
    Node *head;             /* 0x04 head of list searched by func_8005B31C */
    Node *unk8;             /* 0x08 head of the node list */
    u8 _pad1[0x10 - 0xC];   /* 0x0C-0x0F */
    struct CollNode *coll_list; /* 0x10 collision-node list traversed by func_8005B368 */
    u8 _pad2[0x28 - 0x14];  /* 0x14-0x27 */
    s32 unk28;              /* 0x28 flag gating the func_8005F5BC call */
} FieldScene;

typedef struct
{
    FieldScene *scene;
} FieldSceneGlobals;

extern unsigned int D_801ED02C;
extern FieldSceneGlobals g_field_scene;
extern u8 D_800CBF44[];
extern volatile s32 D_801ED490;

void func_8005F5BC(s32, Node*, FieldScene*, s32);

/**
 * @brief If D_801ED02C is zero, set it to 1 and write 0x100 to D_801ED030.
 * @see decomp.me (100%) TODO
 */
void func_8005B1EC(void) {
    volatile FieldState *s = (volatile FieldState *)0x801ED000;
    if (s->unk2C == 0) {
        s->unk2C = 1;
        s->unk30 = 0x100;
    }
}

/**
 * @brief Return non-zero if D_801ED02C is set.
 * @return 1 if D_801ED02C != 0, 0 otherwise.
 * @see decomp.me (100%) TODO
 */
s32 func_8005B218(void) {
    return D_801ED02C != 0;
}

/**
 * @brief Walk the field scene's node list @p arg0 steps and store @p arg1 at
 *        node->unk18, then poke func_8005F5BC if the scene flag is set.
 * @param arg0 Number of ->unk0 links to follow from the list head.
 * @param arg1 Byte value stored at the reached node's unk18.
 * @note WIP - NOT byte-perfect yet. Structure and types are exact (a single
 *       `register Node *node asm("$5")` pin matches all but one schedule slot).
 *       The remaining gap is register coloring: the target colors `node` into
 *       $a1 (evacuating arg1 to $a3) with the loop sentinel -1 in $v1; natural C
 *       colors node into $v1 and the sentinel into $a0. Per GCC 2.8 global.c
 *       allocno_compare, node and the sentinel have near-equal priority and the
 *       tie breaks by allocno (creation) order, so node (born at scene->unk8,
 *       before the loop) wins $v1. No pin-free shape found yet that flips this
 *       without changing another instruction.
 * @see decomp.me (100%) https://decomp.me/scratch/lN7ye
 */
void func_8005B228(s32 arg0, s32 arg1) {
    Node* var_a1;
    s32 var_v0;
    FieldScene* scene = g_field_scene.scene; 

    var_a1 = scene->unk8;
    var_v0 = arg0 - 1;
    while (var_v0 != -1) {
        var_a1 = var_a1->unk0; 
        var_v0 -= 1;
    }
    var_a1->unk18 = arg1;
    if (scene->unk28 != 0) {
        func_8005F5BC(0, var_a1, scene, arg1);
    }
}

/**
 * @brief Set D_801ED490 to the given value.
 * @param arg0 Value to store.
 * @see decomp.me (100%) TODO
 */
void func_8005B288(s32 arg0) {
    D_801ED490 = arg0;
}

/**
 * @brief Apply a color-correction lookup to a range of 16-bit pixels.
 *
 * For each pixel in @p pixels[0..pixel_count-1], extracts the maximum of the
 * three 5-bit color components (B: bits 0-4, G: bits 5-9, R: bits 10-14),
 * uses that maximum as an index into a 64-entry lookup table selected by
 * @p table_index, adds the looked-up value to the STP bit (bit 15) of the
 * original pixel, and writes the result back.
 *
 * @param pixels      Pointer to an array of 16-bit pixel values.
 * @param pixel_count Number of pixels to process (0 = no-op).
 * @param table_index Lookup-table selector; table is D_800CBF44[table_index * 64 ..].
 * @param unused      Unused (FieldObject* in caller).
 *
 * @note Called by field_load_map with pixel_count = object->unk2A and
 *       table_index = D_801ED490 - 1.
 * @note Matches 100% with gcc280_g4 and gcc272_cdk.
 * @see decomp.me (100%) TODO
 */
void field_apply_pixel_lookup(u16* pixels, s32 pixel_count, s32 table_index, void* unused)
{
    u16* ptr;
    s32 count;
    u16 pixel;
    u32 b;
    u32 g;
    u32 r;
    u32 max_component;
    u32 table_base;

    ptr = pixels;
    count = pixel_count - 1;
    table_base = (u32)&D_800CBF44[table_index * 64];
    if (pixel_count != 0)
    {
        do
        {
            pixel = *ptr;
            max_component = pixel & 0x1F;
            g = (pixel >> 5) & 0x1F;
            if (max_component < g)
            {
                max_component = g;
            }
            r = (pixel >> 10) & 0x1F;
            if (max_component < r)
            {
                max_component = r;
            }
            count -= 1;
            *ptr = *(u16*)(table_base + max_component * 2) + (pixel & 0x8000);
            ptr += 1;
        }
        while (count != -1);
    }
}

/**
 * @brief Find a node in the field scene's linked list by key.
 *
 * Walks the linked list starting at g_field_scene->head, returning the first
 * node whose key field matches @p arg0.
 *
 * @param arg0 Key to search for (compared against node->key at offset 0x4).
 * @return Pointer to the matching Node, or NULL if not found.
 *
 * @note Matches 100% with gcc280_g4 and gcc272_cdk.
 * @see decomp.me (100%) https://decomp.me/scratch/FThyS
 */
void* func_8005B31C(s32 arg0)
{
    Node* node;

    node = g_field_scene.scene->head;
    if (node != 0)
    {
        do
        {
            if (arg0 == node->key)
            {
                return node;
            }
            node = node->unk0;
        }
        while (node != 0);
    }
    return 0;
}

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

    for (node = scene->coll_list; node != 0; node = node->next)
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
