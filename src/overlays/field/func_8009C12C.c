#include "common.h"
/** @brief Position and presence fields of a 0x54-byte field actor. */
typedef struct
{
    s32 x, y, z;
    u8 pad_c[0x19];
    u8 presence;
    u8 pad26[0x2E];
} Actor;
/** @brief Collision result fields in a 0x23C-byte actor slot. */
typedef struct
{
    u8 pad[0x176];
    s16 height;
    u8 pad178[0x24];
    s32 contact, surface;
    u8 pad1A4[0x98];
} Slot;
/** @brief Scratchpad collision request and resolved position. */
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
} Mover;
/** @brief Map dimensions used to validate actor coordinates. */
typedef struct
{
    s16 width;
    u16 height;
} Bounds;
extern Actor D_800FDF58[];
extern Slot D_80105AE0[];
extern s32 func_8005B6AC(Mover *);
/**
 * @brief Refresh collision state and positions for the thirteen field actors.
 * @note Uses the scratchpad mover at 0x1F800000 and bounds at 0x801ED400.
 * @note 100% match with GCC 2.7.2 CDK: 109 instructions, 436 bytes.
 */
void func_8009C12C(void)
{
    Actor *actor;
    Slot *slot;
    Bounds *bounds = (Bounds *)0x801ED400;
    Mover *mover = (Mover *)0x1F800000;
    s32 i, x, z;
    actor = D_800FDF58;
    slot = D_80105AE0;
    for (i = 0; i < 13; i++, slot++, actor++)
    {
        if (actor->presence != 0xFF)
        {
            x = actor->x;
            if (x >= 0 && x < (bounds->width << 8) && (z = actor->z) >= 0 &&
                z < ((s32)(bounds->height << 16) >> 7))
            {
                mover->x = x;
                mover->y = actor->y;
                mover->z = actor->z;
                mover->radius = 12;
                mover->mode.bits.step = 8;
                mover->dx = 0;
                mover->dy = 0;
                mover->dz = 0;
                mover->depth = 16;
                mover->contact = -1;
                mover->surface = 0;
                mover->mode.bits.bit17 = 0;
                mover->mode.bits.bit16 = 0;
                func_8005B6AC(mover);
                slot->contact = mover->contact;
                slot->surface = mover->surface;
                slot->height = mover->height >> 8;
                actor->x = mover->x;
                actor->z = mover->z;
                actor->y = mover->y;
            }
            else
            {
                slot->height = 0;
                slot->contact = -1;
                slot->surface = 0;
            }
        }
    }
}
