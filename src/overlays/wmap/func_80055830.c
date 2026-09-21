#include "common.h"

typedef struct
{
    s32 object_id;
    u8 _pad04[0x24];
} WmapCell;

typedef struct
{
    u8 _pad00[5];
    u8 state;
    u8 _pad06[0x22];
    s16 timer;
    u8 _pad2A[2];
} WmapObject;

extern u8 D_800C4614[][9];
extern s32 D_800D7CC4;
extern s32 D_800D7CC8;
extern WmapCell D_80139290[][6];
extern s32 D_80139948;
extern s32 D_8013B298;
extern WmapObject D_80182248[];

extern void func_80058014(s32, s32);
extern void func_800652A8(s32, s32);
extern s32 rand(void);

/**
 * @brief Schedule randomized timer updates for nearby world-map objects.
 * @param delay_min Base value for the next update delay.
 * @param delay_range Randomized span added to the next update delay.
 * @param timer_min Base value assigned to selected object timers.
 * @param timer_range Randomized span added to selected object timers.
 */
void func_80055830(s32 delay_min, s32 delay_range, s32 timer_min, s32 timer_range)
{
    s32 row;
    s32 column;
    s32 pattern_index;
    s32 object_id;
    WmapObject* object;

    if (D_80139948-- > 0)
    {
        return;
    }

    D_80139948 = ((rand() * delay_range) >> 15) + delay_min;
    column = (rand() % 3) + D_800D7CC4;
    row = (rand() % 3) + D_800D7CC8;
    object_id = D_80139290[column][row].object_id;

    if (rand() & 0xFF)
    {
        WmapObject* object_base;

        object_base = D_80182248;
        object = &object_base[object_id];
        if (object->state != 1)
        {
            return;
        }

        object->timer = ((rand() * timer_range) >> 15) + timer_min;
        func_80058014(object_id, 2);
    }
    else
    {
        pattern_index = 0;
        for (row = D_800D7CC8; row < D_800D7CC8 + 3; row++)
        {
            for (column = D_800D7CC4; column < D_800D7CC4 + 3; column++)
            {
                object_id = D_80139290[column][row].object_id;
                if (D_800C4614[D_8013B298][pattern_index])
                {
                    WmapObject* object_base;

                    object_base = D_80182248;
                    object = &object_base[object_id];
                    if (object->state == 1)
                    {
                        object->timer = ((rand() * timer_range) >> 15) + timer_min;
                        func_80058014(object_id, 2);
                    }
                }
                pattern_index++;
            }
        }
    }

    func_800652A8(0x3D, 0x80);
}
