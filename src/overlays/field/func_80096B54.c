#include "common.h"

/** @brief Partial 0x54-byte field record with cleanup state fields. */
typedef struct
{
    u8 pad0[0x25];
    u8 unk25;
    u8 pad26[4];
    s16 unk2a;
    u8 pad2c[0x54 - 0x2C];
} FieldCleanupRecord;

/** @brief Partial 0x23C-byte slot state with behavior and activity flags. */
typedef struct
{
    u8 pad0[0xC];
    u32 flags;
    u8 pad10[0x178 - 0x10];
    u32 status;
    u8 pad17c[0x23C - 0x17C];
} FieldCleanupSlot;

/** @brief Partial actor state with owner slot and linked slot indices. */
typedef struct
{
    u8 pad0[0x24];
    u8 active;
    u8 pad25[0x222 - 0x25];
    s16 unk222;
    u8 pad224[4];
    u8 slot;
    u8 links[9];
    u8 link_count;
    u8 pad233[7];
    u8 mask;
    u8 pad23b[0x244 - 0x23B];
} FieldCleanupActor;

/** @brief Pending track cleanup request and the field slot it references. */
typedef struct
{
    s32 pending;
    u8 pad4[8];
    s32 slot;
    u8 pad10[0x1C - 0x10];
} FieldCleanupTrack;

extern FieldCleanupRecord D_800FDF58[];
extern FieldCleanupSlot D_80105AE0[];
extern FieldCleanupActor g_field_actor_slots[];
extern FieldCleanupTrack D_80105880[];
extern s32 D_800F2278, D_800F227C, D_800F2280;
s32 func_8005B218(void);
void func_800A3B78(s32);
void func_8006D21C(FieldCleanupActor *);
void field_set_global_color_scale(s32, s32, s32);

/**
 * @brief Process pending track cleanup requests and reset their rendering state.
 * @note WIP: 96.333336% gcc272_cdk, with address and allocation differences.
 * @note Preserve the labeled scans to avoid extra compiler-generated loop pointers.
 */
void func_80096B54(void)
{
    u8 *render = (u8 *)0x801ED600;
    FieldCleanupRecord *records;
    FieldCleanupActor *actors;
    FieldCleanupTrack *tracks;
    FieldCleanupSlot *slots;
    FieldCleanupTrack *track;
    FieldCleanupActor *actor;
    FieldCleanupSlot *slot_base;
    u8 *link_actor;
    s16 action;
    s32 i;
    s32 index;
    u8 link;

    if (func_8005B218() != 0)
    {
        slot_base = D_80105AE0;
        records = D_800FDF58;
        actors = g_field_actor_slots;
        tracks = D_80105880;
        track = tracks;
    outer_loop:
    {
        if (track->pending != 0)
        {
            slots = D_80105AE0;
            track->pending = 0;
            records[track->slot].unk25 = 0;
            actor = actors;
            records[track->slot].unk2a = 0;
        actor_loop:
        {
            if (actor->active != 0)
            {
                index = actor->slot;
                if (index == track->slot)
                {
                    actor->unk222 = 0;
                    records[index].unk2a = 0;
                    func_800A3B78(actor->slot);
                    func_8006D21C(actor);
                    index = actor->slot;
                    actor->active = 0;
                    actor->mask = 0;
                    action = records[index].unk2a;
                    if ((action != 0x90 && action != 0x94) || (slots[index].flags & 0x200))
                    {
                        records[actor->slot].unk25 = 0;
                    }
                    slots[actor->slot].status &= ~1;
                    for (i = 0; i < actor->link_count; i++)
                    {
                        link_actor = (u8 *)actor + i;
                        {
                            link = link_actor[0x229];
                            if (link != 0xFF)
                            {
                                records[link].unk25 = 0;
                                slots[link_actor[0x229]].status &= ~1;
                            }
                        }
                    }
                }
            }
            actor++;
        }
            if ((s32)actor < (s32)(actors + 80))
            {
                goto actor_loop;
            }
            D_800F2280 = 0;
            D_800F227C = 0;
            D_800F2278 = 0;
            slot_base[track->slot].status &= ~1;
            field_set_global_color_scale(0x100, 0x100, 0x100);
            render[0x13F] = 0;
            render[0x91] = 0;
            render[0x140] = 0;
            render[0x92] = 0;
        }
        track++;
    }
        if ((s32)track < (s32)(tracks + 3))
        {
            goto outer_loop;
        }
    }
}
