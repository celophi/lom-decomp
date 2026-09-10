#include "common.h"
/** @brief Three fixed-point position or displacement components. */
typedef struct
{
    s32 x, y, z;
} FieldMoveVector;
/** @brief Position, state and index in a 0x54-byte actor record. */
typedef struct
{
    s32 x, y, z;
    u8 padc[0x10];
    u32 flags;
    u8 surface, state;
    u8 pad22[8];
    s16 kind;
    u8 pad2c[0xE];
    u8 index;
    u8 pad3b[0x19];
} FieldMoveActor;
/** @brief Visual kind and flags in a 0x48-byte object record. */
typedef struct
{
    u8 pad0[0x2E];
    u8 visual_kind;
    u8 pad2f[5];
    u32 flags;
    u8 pad38[0x10];
} FieldMoveObject;
/** @brief Packed collision flags, height and contact in a 0x23C-byte state. */
typedef struct
{
    u8 pad0[0x174]; /** @brief Collision flags and signed height share one word. */
    union
    {
        u32 word;
        struct
        {
            u16 flags;
            s16 height;
        } h;
    } packed;
    u8 pad178[0x24];
    s32 contact, surface;
    u8 pad1a4[0x98];
} FieldMoveState;
/** @brief Scratchpad mover request and collision resolver output. */
typedef struct
{
    s32 x, y, z, unkc, unk10, unk14, unk18, contact, surface;
    s16 width, height_tolerance; /** @brief Step halfword and request bits also accessed as a full word. */
    union
    {
        s32 word;
        struct
        {
            s16 step;
            u16 flags;
        } h;
        struct
        {
            unsigned step : 16;
            unsigned bit16 : 1;
            unsigned bit17 : 1;
            unsigned high : 14;
        } bits;
    } packed;
} FieldMoveRequest;
/** @brief Scratchpad position and footprint used by the obstruction query. */
typedef struct
{
    s32 x, y, z;
    s16 unkc, unke, unk10;
} FieldMoveQuery;
/** @brief Map dimensions used for the fixed-point bounds check. */
typedef struct
{
    s16 x;
    u16 unk2;
} FieldMoveBounds;
s32 func_8005B368(FieldMoveQuery *);
s32 func_8005B6AC(FieldMoveRequest *);
s32 func_80092988(FieldMoveActor *, FieldMoveVector *);
s32 func_80098748(FieldMoveActor *, const void *);
s32 func_800987DC(FieldMoveActor *, const void *, s32);
extern FieldMoveObject D_800FE3A0[];
extern FieldMoveState D_80105AE0[];
extern s32 D_800FE754, D_8010D024;
/**
 * @brief Resolve a proposed actor move against map and actor collisions.
 * @param actor Actor whose position and persistent collision state are updated.
 * @param position Input movement vector, overwritten with the resolved position.
 * @param mode Collision response mode forwarded to the actor collision helper.
 * @return One when the proposed or resolved position is accepted, zero otherwise.
 * @note Uses mover and probe records at scratchpad addresses 0x1F800010 and
 *       0x1F800080. The query result is consumed as a full return-register value.
 */
s32 func_80097FA0(FieldMoveActor *actor, FieldMoveVector *position, s32 mode)
{
    FieldMoveVector delta;
    FieldMoveState *height_state;
    s32 hit;
    FieldMoveBounds *bounds = (FieldMoveBounds *)0x801ED400;
    FieldMoveRequest *mover = (FieldMoveRequest *)0x1F800010;
    FieldMoveQuery *query = (FieldMoveQuery *)0x1F800080;
    s16 var_v0;
    s32 temp_a0;
    s32 temp_a0_2;
    s32 temp_v1_2;
    s32 temp_v1_3;
    s32 var_a1;
    s32 var_a1_2;
    s32 var_v0_2;
    s32 var_v0_3;
    u16 temp_v1;
    u16 temp_v1_4;
    u16 temp_v1_5;
    FieldMoveState *temp_v0;
    FieldMoveState *temp_v0_2;
    FieldMoveState *temp_v0_3;
    FieldMoveState *temp_v1_6;
    FieldMoveState *temp_v1_7;
    FieldMoveState *temp_v1_8;

    if (((u32)D_800FE3A0[actor->index].flags >> 0x17) & 1)
    {
        actor->x += position->x;
        actor->y = (s32)(actor->y + position->y);
        actor->z = (s32)(actor->z + position->z);
        return 1;
    }
    if (D_800FE754 != 0)
    {
        temp_v1 = actor->kind;
        if (((u32)(temp_v1 - 0xB0) >= 2U) && ((s16)temp_v1 != 0xB5) && (func_80092988(actor, position) != 0))
        {
            position->x = 0;
        }
    }
    temp_v1_2 = actor->x;
    if ((temp_v1_2 >= 0) && (temp_v1_2 < (bounds->x << 8)))
    {
        temp_a0 = actor->z;
        if (temp_a0 >= 0)
        {
            if (temp_a0 < ((s32)(bounds->unk2 << 0x10) >> 7))
            {
                mover->x = temp_v1_2;
                mover->y = (s32)actor->y;
                mover->z = (s32)actor->z;
                temp_a0_2 = position->x;
                mover->unkc = temp_a0_2;
                mover->unk10 = (s32)position->y;
                temp_v1_3 = position->z;
                mover->height_tolerance = 0x10;
                query->unke = 0x10;
                mover->unk14 = temp_v1_3;
                if (D_800FE3A0[actor->index].visual_kind == 0x40)
                {
                    mover->width = 0xC;
                    query->unkc = 0xC;
                    var_v0 = 8;
                }
                else
                {
                    mover->width = 9;
                    query->unkc = 9;
                    var_v0 = 6;
                }
                mover->packed.h.step = var_v0;
                query->unk10 = var_v0;
                mover->height_tolerance = 0x10;
                mover->packed.bits.bit17 = 0;
                mover->packed.bits.bit16 = 0;
                mover->contact = (s32)D_80105AE0[actor->index].contact;
                mover->surface = (s32)D_80105AE0[actor->index].surface;
                func_8005B6AC(mover);
                D_80105AE0[actor->index].contact = (s32)mover->contact;
                D_80105AE0[actor->index].surface = (s32)mover->surface;
                temp_v1_4 = actor->kind;
                if (((u32)(temp_v1_4 - 0xB0) < 2U) || ((s16)temp_v1_4 == 0xB5))
                {
                    delta.x = temp_a0_2 - actor->x;
                    delta.y = 0;
                    var_v0_2 = temp_v1_3 - actor->z;
                }
                else
                {
                    delta.x = mover->x - actor->x;
                    delta.y = 0;
                    var_v0_2 = mover->z - actor->z;
                }
                delta.z = var_v0_2;
                query->x = (s32)mover->x;
                query->z = (s32)mover->z;
                query->y = (s32)(position->y + actor->y);
                if (((D_800FE754 == 0) || (actor->flags & 0x1FF) || (func_8005B368(query) == -1)) &&
                    ((temp_v1_5 = actor->kind, (((u32)(temp_v1_5 - 0xB0) < 2U) != 0)) || ((s16)temp_v1_5 == 0xB5) ||
                     (D_800FE754 == 0) || (func_80092988(actor, &delta) == 0)))
                {
                    position->x = mover->x;
                    if (((actor->state & 0x7F) == 0x3D) && ((u8)actor->index < 2U))
                    {
                        var_v0_3 = position->y + actor->y;
                    }
                    else
                    {
                        var_v0_3 = mover->y;
                    }
                    position->y = var_v0_3;
                    position->z = (s32)mover->z;
                    height_state = &D_80105AE0[actor->index];
                    var_a1 = mover->unk18;
                    if (var_a1 < 0)
                    {
                        var_a1 += 0xFF;
                    }
                    height_state->packed.h.height = (s16)(var_a1 >> 8);
                }
                else
                {
                    goto block_34;
                }
            }
            else
            {
                goto block_33;
            }
        }
        else
        {
            goto block_33;
        }
    }
    else
    {
    block_33:
        D_80105AE0[actor->index].packed.h.height = 0;
        D_80105AE0[actor->index].contact = -1;
        D_80105AE0[actor->index].surface = 0;
    block_34:
        position->x = actor->x;
        position->y = (s32)actor->y;
        position->z = (s32)actor->z;
    }
    D_8010D024 = 0;
    if (((u32)D_80105AE0[actor->index].packed.word >> 0xD) & 1)
    {
        if (func_80098748(actor, actor) == 0)
        {
            temp_v0 = &D_80105AE0[actor->index];
            temp_v0->packed.word = (s32)(temp_v0->packed.word & ~0x2000);
        }
        var_a1_2 = 1;
    }
    else if (func_80098748(actor, position) == 0)
    {
        var_a1_2 = 1;
        temp_v0_2 = &D_80105AE0[actor->index];
        temp_v0_2->packed.word = (s32)(temp_v0_2->packed.word & ~0x2000);
    }
    else
    {
        hit = func_80098748(actor, actor);
        var_a1_2 = 0;
        if (hit != 0)
        {
            temp_v1_6 = &D_80105AE0[actor->index];
            temp_v1_6->packed.word = (s32)(temp_v1_6->packed.word | 0x2000);
        }
    }
    if (var_a1_2 != 0)
    {
        if (((u32)D_80105AE0[actor->index].packed.word >> 0xE) & 1)
        {
            actor->x = position->x;
            actor->y = (s32)position->y;
            actor->z = (s32)position->z;
            if (func_800987DC(actor, actor, mode) == 0)
            {
                temp_v0_3 = &D_80105AE0[actor->index];
                temp_v0_3->packed.word = (s32)(temp_v0_3->packed.word & ~0x4000);
            }
            return 1;
        }
        if (func_800987DC(actor, position, mode) == 0)
        {
            actor->x = position->x;
            actor->y = (s32)position->y;
            actor->z = (s32)position->z;
            temp_v1_7 = &D_80105AE0[actor->index];
            temp_v1_7->packed.word = (s32)(temp_v1_7->packed.word & ~0x4000);
            return 1;
        }
        if (func_800987DC(actor, actor, mode) != 0)
        {
            temp_v1_8 = &D_80105AE0[actor->index];
            temp_v1_8->packed.word = (s32)(temp_v1_8->packed.word | 0x4000);
        }
        return 0;
    }
    return 0;
}
