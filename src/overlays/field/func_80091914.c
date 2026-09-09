#include "common.h"

/** @brief Partial actor state used to choose the next action animation. */
typedef struct
{
    u8 pad[0x21];
    u8 mode;
    u8 pad22[14];
    s16 counter;
    u8 pad32[8];
    u8 slot;
} Actor;
/** @brief Partial 0x23C-byte actor slot with action-control flags. */
typedef struct
{
    u8 pad[12];
    u32 flags;
    u8 pad10[0x17D];
    u8 active;
    u8 pad18E[0xAE];
} Slot;
/** @brief Eleven animation IDs followed by their disabled flags. */
typedef struct
{
    u16 animations[11];
    u8 disabled[11];
    u8 pad;
} ActionMap;
extern Slot D_80105AE0[];
extern u8 g_field_action_animation_maps[];
extern s32 D_800473F8;
extern s32 D_8010AE54;
s32 func_800A29F8(s32, s32, s32);
void func_800A3938(s32, s32);

/**
 * @brief Resolve an actor action into an enabled animation ID.
 * @param actor Actor state to query and update.
 * @param map_index Action-animation map passed to the action selector.
 * @return Enabled animation ID, or zero when no animation is available.
 * @note Materialize map_base before applying the map stride; the ordering is
 * required for the exact address setup. Keep the repeated animation load.
 * @note GCC 2.7.2 CDK: 100% match, 109 instructions (436 bytes).
 */
u16 func_80091914(Actor *actor, s32 map_index)
{
    s32 action;
    s32 in_range;
    ActionMap *map;
    u16 *animation;
    s32 map_base;

    action = func_800A29F8(map_index, (actor->mode >> 7) ^ 1, 0);
    if (action != 0xFF)
    {
        D_800473F8 = (D_800473F8 & ~0xFF) | action;
    }
    else if ((actor->mode & 0x7F) != 0x3D)
    {
        actor->counter = 0;
        D_80105AE0[actor->slot].flags &= 0xFFFF7FFF;
        D_80105AE0[actor->slot].active = 0;
    }
    if (D_80105AE0[actor->slot].flags & 0x400)
    {
        if ((u32)(action - 2) >= 2)
        {
            if (action != 0xFF)
            {
                func_800A3938(0x78, 0x80);
                return 0;
            }
            goto no_animation;
        }
        goto check_range;
    }
check_range:
    if (action < 11)
    {
        if (D_8010AE54 == 0 || action < 4)
        {
            map_base = (s32)g_field_action_animation_maps;
            map = (ActionMap *)(map_index * 0x22 + map_base);
            animation = (u16 *)(action * 2 + (s32)map);
            if (*animation != 0)
            {
                if (map->disabled[action] == 0)
                {
                    return *animation;
                }
                return 0;
            }
            goto no_animation;
        }
        return 0;
    }
no_animation:
    return 0;
}
