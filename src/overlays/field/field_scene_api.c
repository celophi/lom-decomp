#include "common.h"

typedef struct {
    u8 _pad[0x2C];
    s32 unk2C;
    s32 unk30;
} FieldState;

typedef struct Node
{
    struct Node *unk0;   /* 0x00 next pointer */
    s32 definition;      /* 0x04 compared by field_find_object_by_definition */
    u8 _pad[0x10];       /* 0x08-0x17 */
    s8 unk18;            /* 0x18 byte written by func_8005B228 */
} Node;

struct CollNode;

typedef struct
{
    u8 _pad0[4];           /* 0x00-0x03 */
    Node *objects;          /* 0x04 head of the scene object list */
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
 * @brief Find a field object by its definition pointer.
 *
 * Walks the scene object list and returns the first object whose definition
 * pointer matches @p definition.
 *
 * @param definition Definition pointer to compare at object offset 0x04.
 * @return Pointer to the matching field object, or NULL if not found.
 *
 * @note Matches 100% with gcc280_g4 and gcc272_cdk.
 * @see decomp.me (100%) https://decomp.me/scratch/FThyS
 */
void* field_find_object_by_definition(s32 definition)
{
    Node* node;

    node = g_field_scene.scene->objects;
    if (node != 0)
    {
        do
        {
            if (definition == node->definition)
            {
                return node;
            }
            node = node->unk0;
        }
        while (node != 0);
    }
    return 0;
}
