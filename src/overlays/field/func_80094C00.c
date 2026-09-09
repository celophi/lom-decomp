#include "common.h"

/** @brief Position, speed, and slot-index prefix of a field actor. */
typedef struct
{
    s32 x, y, z;
    u8 pad_c[0x14];
    u8 speed;
    u8 pad_21[0x19];
    u8 slot;
} FieldMovingActor;
/** @brief FieldObjectVisualKind-kind byte in a 0x48-byte field object record. */
typedef struct
{
    u8 pad[0x2E];
    u8 kind;
    u8 tail[0x19];
} FieldObjectVisualKind;
/** @brief Collision result fields in a 0x23C-byte actor slot. */
typedef struct
{
    u8 pad[0x176];
    s16 height;
    u8 pad178[0x24];
    s32 contact, surface;
    u8 pad1a4[0x98];
} FieldActorCollisionResult;
/** @brief Scratchpad collision request and resolver output. */
typedef struct
{
    s32 x, y, z, dx, dy, dz, height, contact, surface;
    s16 radius, depth;
    union
    {
        s32 flags;
        struct
        {
            unsigned step : 16;
            unsigned bit16 : 1;
            unsigned bit17 : 1;
            unsigned high : 14;
        } bits;
    } mode;
} FieldActorCollisionMover;
/** @brief Map dimensions used to validate fixed-point actor coordinates. */
typedef struct
{
    s16 width;
    u16 height;
} FieldActorCollisionBounds;
extern FieldObjectVisualKind D_800FE3A0[];
extern FieldActorCollisionResult D_80105AE0[];
extern s32 func_8005B6AC(FieldActorCollisionMover *);

/**
 * @brief Move an actor by its speed and refresh its collision contact and height.
 * @param actor Actor position, speed, and destination slot index.
 * @param dx Horizontal direction or displacement multiplier.
 * @param dz Depth direction or displacement multiplier.
 * @note Uses collision scratchpad memory at 0x1F800000 and map bounds at 0x801ED400.
 * @note 100% match with GCC 2.7.2 CDK: 169 instructions, 676 bytes.
 */
void func_80094C00(FieldMovingActor *actor, s32 dx, s32 dz)
{
    FieldActorCollisionBounds *bounds = (FieldActorCollisionBounds *)0x801ED400;
    FieldActorCollisionMover *mover = (FieldActorCollisionMover *)0x1F800000;
    s32 x, z;
    u8 speed;

    speed = actor->speed;
    actor->x += dx * speed;
    actor->z += dz * speed;
    x = actor->x;
    z = actor->z;
    if (x >= 0 && x < (bounds->width << 8) && z >= 0 &&
        z < ((s32)(bounds->height << 16) >> 7))
    {
        mover->x = x;
        mover->y = actor->y;
        mover->z = actor->z;
        mover->dx = 0;
        mover->dy = 0;
        mover->dz = 0;
        if (D_800FE3A0[actor->slot].kind == 0x40)
        {
            mover->radius = 12;
            mover->mode.bits.step = 8;
        }
        else
        {
            mover->radius = 9;
            mover->mode.bits.step = 6;
        }
        mover->depth = 16;
        /* Separate bitfield clears preserve the two target mask operations. */
        mover->mode.bits.bit17 = 0;
        mover->mode.bits.bit16 = 0;
        mover->contact = D_80105AE0[actor->slot].contact;
        mover->surface = D_80105AE0[actor->slot].surface;
        func_8005B6AC(mover);
        D_80105AE0[actor->slot].contact = mover->contact;
        D_80105AE0[actor->slot].surface = mover->surface;
        D_80105AE0[actor->slot].height = mover->height / 256;
    }
    else
    {
        D_80105AE0[actor->slot].contact = -1;
        D_80105AE0[actor->slot].surface = 0;
        D_80105AE0[actor->slot].height = 0;
    }
}
