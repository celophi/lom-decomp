#include "common.h"
#include "vector.h"
/** @brief Position, state, and presence fields in a 0x54-byte actor record. */
typedef struct
{
    s32 x, y, z;
    u8 padc[0x15];
    u8 state;
    u8 pad22[3];
    u8 presence;
    u8 pad26[4];
    s16 value;
    u8 tail[0x28];
} FieldPanelActor;
/** @brief Display values, countdown, group, and linked actor in a 0x23C-byte slot. */
typedef struct
{
    u32 unknown0;
    u32 current;
    union
    {
        s32 word;
        u8 bytes[4];
    } previous;
    u32 flags;
    u32 group;
    u8 pad14[0x38];
    u32 options;
    u8 pad50[0x11D];
    u8 linked;
    u8 pad16e[0xA];
    u32 state;
    u8 tail[0xC0];
} FieldPanelSlot;
/** @brief Participation flag in a 0x268-byte player record. */
typedef struct
{
    u8 flags;
    u8 tail[0x267];
} FieldPanelPlayer;
extern FieldPanelActor D_800FDF58[], D_800FF658[];
extern FieldPanelSlot D_80105AE0[];
extern FieldPanelPlayer D_800FD818[];
extern s32 D_800EB04C[];
extern s32 D_800F22A0, D_800F22A4, D_800F22A8;
extern s32 D_800FE754, D_801158A0, D_80122B20;
extern void func_80084D08(s32, s32, s32, u8 *, u32);

/**
 * @brief Draw participant panels and temporary indicators for eligible actors.
 * @param render_context Rendering context forwarded to the panel drawing helper.
 * @note Actor positions use signed fixed-point division, followed by screen clamps.
 * @note Display-value bits 24 through 30 count down after a temporary panel draw.
 */
void func_80084700(u8 *render_context)
{
    Vec2s position;
    s32 i = 0;
    s32 count = i;
    s32 j;
    s32 absent = 0xFF;
    FieldPanelSlot *slot = D_80105AE0;
    FieldPanelPlayer *player = D_800FD818;
    FieldPanelActor *actor = D_800FDF58;
    FieldPanelActor *actor_one;
    FieldPanelPlayer *player_one;
    FieldPanelActor *actor_two;
    FieldPanelPlayer *player_two;
    s32 x_two;
    s32 *order;
    s32 x_three;
    s16 y_three;
    s32 boss_drawn;
    FieldPanelActor *enemy;
    FieldPanelSlot *enemy_slot;
    FieldPanelActor *linked;
    s32 group;
    s32 value;
    u32 current;
    u32 previous;
    s32 y;
    s32 z;
    s32 ticks;

    do
    {
        slot = &D_80105AE0[i];
        actor = &D_800FDF58[i];
        if (actor->presence != absent && (player->flags & 1))
        {
            if (actor->value != 0x85 && actor->value != 0x87)
            {
                slot->options |= 1;
            }
            count++;
        }
        player++;
        i++;
    } while (i < 3);
    switch (count)
    {
        case 1:
            i = 0;
            player_one = D_800FD818;
            actor_one = D_800FDF58;
            do
            {
                actor_one = &D_800FDF58[i];
                player_one = &D_800FD818[i];
                if (actor_one->presence != 0xFF && (player_one->flags & 1))
                {
                    func_80084D08(0x70, 0x10, i, render_context, 0x64);
                }
                i++;
            } while (i < 3);
            break;
        case 2:
            i = 0;
            player_two = D_800FD818;
            actor_two = D_800FDF58;
            x_two = 0x38;
            do
            {
                actor_two = &D_800FDF58[i];
                player_two = &D_800FD818[i];
                if (actor_two->presence != 0xFF && (player_two->flags & 1))
                {
                    func_80084D08(x_two, 0x10, i, render_context, 0x64);
                    x_two += 0x6C;
                }
                i++;
            } while (i < 3);
            break;
        case 3:
            i = 0;
            order = D_800EB04C;
            x_three = 8;
            do
            {
                if (D_800FDF58[*order].presence != 0xFF && (D_800FD818[*order].flags & 1))
                {
                    y_three = 0x1C;
                    if (i & 1)
                    {
                        y_three = 4;
                    }
                    func_80084D08(x_three, y_three, *order, render_context, 0x64);
                    x_three += 0x68;
                }
                i++;
                order++;
            } while (i < 3);
            break;
    }
    boss_drawn = 0;
    if (D_801158A0 != 0)
    {
        j = 3;
        if (D_80122B20 == 0)
        {
            enemy_slot = &D_80105AE0[3];
            enemy = &D_800FDF58[3];
            do
            {
                group = enemy_slot->group & 0xF;
                if (group == D_800FE754 && group != 0)
                {
                    value = enemy_slot->previous.word;
                    if (value < 0)
                    {
                        if (enemy->presence != 0xFF && (value & 0xFFFFFF) && boss_drawn == 0)
                        {
                            func_80084D08(0x20, 0xC0, j, render_context, 0x190);
                            boss_drawn = 1;
                        }
                    }
                    else if (enemy->presence != 0xFF)
                    {
                        current = enemy_slot->current;
                        previous = value & 0xFFFFFF;
                        if (current < previous || previous != current)
                        {
                            enemy_slot->previous.word = (value & 0x80FFFFFF) | 0x14000000;
                        }
                        if (enemy_slot->previous.bytes[3] & 0x7F)
                        {
                            if (!(enemy_slot->flags & 0x100) && (*(u8 *)&enemy_slot->state & 1) &&
                                (D_800FF658[enemy_slot->linked].state & 0x7F) != 0x2F)
                            {
                                linked = &D_800FF658[enemy_slot->linked];
                                position.x = (D_800F22A0 / 256) + (u32)(linked->x / 256 + 0xA0);
                                y = D_800F22A4 / 256 + (D_800FF658[enemy_slot->linked].y / 256 + 0x70);
                                z = D_800FF658[enemy_slot->linked].z;
                            }
                            else
                            {
                                position.x = (D_800F22A0 / 256) + (u32)(enemy->x / 256 + 0xA0);
                                y = D_800F22A4 / 256 + (enemy->y / 256 + 0x70);
                                z = enemy->z;
                            }
                            position.y = y - z / 512 - D_800F22A8 / 512;
                            if (position.x + 0x20 >= 0x141)
                            {
                                position.x = 0x120;
                            }
                            if (position.x < 0x20)
                            {
                                position.x = 0x20;
                            }
                            if (position.y >= 0xD1)
                            {
                                position.y = 0xD0;
                            }
                            if (position.y < 0x10)
                            {
                                position.y = 0x10;
                            }
                            func_80084D08(position.x - 0x1C, position.y, j, render_context, 0x64);
                            value = enemy_slot->previous.word;
                            ticks = ((u32)value >> 24) & 0x7F;
                            if (ticks != 0)
                            {
                                enemy_slot->previous.word = (value & 0x80FFFFFF) | (((ticks - 1) & 0x7F) << 24);
                            }
                        }
                    }
                }
                enemy_slot++;
                j++;
                enemy++;
            } while (j < 13);
        }
    }
}
