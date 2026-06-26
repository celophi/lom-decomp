#include "common.h"

typedef struct {
    u8 _pad[0x2C];
    s32 unk2C;
    s32 unk30;
} FieldState;

typedef struct Node
{
    struct Node *unk0;   /* 0x00 next pointer */
    u8 _pad[0x14];
    s8 unk18;            /* 0x18 byte written by func_8005B228 */
} Node;

typedef struct
{
    u8 _pad0[8];
    Node *unk8;          /* 0x08 head of the node list */
    u8 _pad1[0x28 - 0xC];
    s32 unk28;           /* 0x28 flag gating the func_8005F5BC call */
} FieldScene;

typedef struct
{
    FieldScene *scene;
} FieldSceneGlobals;

extern unsigned int D_801ED02C;
extern FieldSceneGlobals g_field_scene;
extern u8 D_800CBF44[];
extern volatile s32 D_801ED490;

void func_8005F5BC(s32 arg0);

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
 * @see decomp.me (94.38%) TODO
 */
void func_8005B228(s32 arg0, s8 arg1)
{
    FieldScene *scene;
    Node *node;
    s32 i;

    scene = g_field_scene.scene;
    node = scene->unk8;
    i = arg0 - 1;
    if (arg0 != 0)
    {
        do
        {
            node = node->unk0;
            i -= 1;
        } while (i != -1);
    }
    node->unk18 = arg1;
    if (scene->unk28 != 0)
    {
        func_8005F5BC(0);
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
