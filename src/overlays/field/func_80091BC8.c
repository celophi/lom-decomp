#include "common.h"
/** @brief Actor position and presence fields in a 0x54-byte record. */
typedef struct
{
    s32 x;
    s32 pad4;
    s32 z;
    u8 pad_c[0x19];
    u8 presence;
    u8 pad26[0x2E];
} Actor;
/** @brief Actor slot flags and vertical offset in a 0x23C-byte record. */
typedef struct
{
    u8 pad0[12];
    s32 flags;
    u8 pad10[0x166];
    s16 height;
    u8 pad178[0xC4];
} Slot;
extern int abs(int);
extern Actor D_800FDF58[];
extern Slot D_80105AE0[];
extern s32 D_8010AE4C;
extern s32 D_8010AE50;
extern s32 D_8010D010;
extern s32 D_8010D014;

/**
 * @brief Update the actor-derived target and approach each coordinate by at most 0x800.
 */
void func_80091BC8(void)
{
    s32 divisor;
    s32 target_z;
    s32 target_x;
    s32 count;
    s32 index;
    Actor *actor;
    Slot *slot;

    count = 0;
    target_z = 0;
    target_x = 0;
    index = 1;
    do
    {
        actor = &D_800FDF58[index];
        slot = &D_80105AE0[index];
        if ((actor->presence != 0xFF) && !(slot->flags & 0x23E4))
        {
            target_x -= actor->x;
            target_z -= actor->z;
            target_z += slot->height << 9;
            if (count != 0)
            {
                divisor = count + 1;
                target_x = target_x / divisor;
                target_z = target_z / divisor;
            }
            count += 1;
        }
        index -= 1;
    } while (index >= 0);
    if (count != 0)
    {
        D_8010AE4C = target_x;
        D_8010AE50 = target_z;
    }

    {
        s32 target = D_8010AE4C;
        s32 current = D_8010D010;

        if (target != current)
        {
            s32 delta = target - current;
            s32 magnitude = abs(delta);

            if (magnitude < 0x800)
            {
                D_8010D010 = target;
            }
            else
            {
                s32 value;
                s32 *write_position = &D_8010D010;

                if (delta < 0)
                {
                    value = current - 0x800;
                }
                else
                {
                    value = current + 0x800;
                }
                *write_position = value;
            }
        }
    }

    {
        s32 target = D_8010AE50;
        s32 current = D_8010D014;

        if (target != current)
        {
            s32 delta = target - current;
            s32 magnitude = abs(delta);

            if (magnitude < 0x800)
            {
                D_8010D014 = target;
            }
            else
            {
                s32 value;
                s32 *write_position = &D_8010D014;

                if (delta < 0)
                {
                    value = current - 0x800;
                }
                else
                {
                    value = current + 0x800;
                }
                *write_position = value;
            }
        }
    }
}
