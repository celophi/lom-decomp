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
