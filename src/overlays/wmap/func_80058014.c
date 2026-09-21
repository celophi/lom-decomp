#include "common.h"

/** @brief Actor mode, transition, and signed intensity within a 44-byte record. */
typedef struct
{
    u8 pad_00[4];
    u8 mode;
    u8 transition;
    s8 intensity;
    u8 pad_07[0x25];
} WmapActor;

extern WmapActor D_80182248[];

/**
 * @brief Clamp the actor intensity and select the requested mode transition.
 * @param index Actor index.
 * @param mode Requested mode; five forces mode two at full intensity.
 */
void func_80058014(s32 index, s32 mode)
{
    s8 intensity;
    u8 old_mode;
    WmapActor *actor;

    actor = &D_80182248[index];
    intensity = actor->intensity;
    if (intensity >= 0x10)
    {
        actor->intensity = 0xF;
    }
    else if (intensity < 0)
    {
        actor->intensity = 0;
    }
    if (mode == 5)
    {
        actor->mode = 2U;
        actor->transition = 2U;
        actor->intensity = 0xF;
        return;
    }
    old_mode = actor->mode;
    if (mode != old_mode)
    {
        switch (old_mode)
        {                       
        case 0:
            actor->mode = mode;
            if (mode == 1)
            {
                actor->transition = mode;
                return;
            }
            actor->transition = 3U;
            return;
        case 1:
            actor->mode = mode;
            if (mode != 0)
            {
                actor->transition = 3;
                return;
            }
            actor->transition = 0;
            actor->intensity = 0;
            return;
        case 2:
            actor->mode = mode;
            if (mode != 0)
            {
                actor->transition = 4U;
                return;
            }
            actor->transition = 0U;
           
            actor->intensity = 0;
            return;
        }
    }
}
