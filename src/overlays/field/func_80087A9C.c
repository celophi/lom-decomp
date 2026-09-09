#include "common.h"

/** @brief Actor entry with position, preserved state bits, and slot index. */
typedef struct
{
    s32 unk0, unk4, unk8;
    u8 padC[16];
    union
    {
        s32 word;
        u16 half[2];
    } state;
    u8 pad20;
    s8 unk21;
    u8 pad22[3];
    u8 unk25;
    u8 pad26[20];
    u8 unk3A;
} Entry;
/** @brief Actor slot owner and group flags in a 0x23C-byte record. */
typedef struct
{
    u8 pad0[16];
    s32 unk10, unk14;
    u8 pad18[0x23C - 0x18];
} Actor;
/** @brief Resource flags in a 0x14-byte entry. */
typedef struct
{
    u8 pad0[16];
    s32 unk10;
} Resource;
extern Actor D_80105AE0[];
extern Resource g_field_resource_entries[];
extern Entry *func_80087C9C(void);
extern void func_8006B240(s32, s32, s32);
extern void func_8006B4D0(u8, s32);
extern void func_8006B7A0(u8, s32);
extern s32 func_8006C3FC(Entry *);
/**
 * @brief Reinitialize an actor resource while preserving selected state bits.
 * @param owner_id Owner identifier assigned to the actor slot.
 * @param resource_id Resource entry to initialize.
 * @param parameter_a First resource initialization parameter.
 * @param parameter_b Second resource initialization parameter.
 * @param group Group bits written to the actor slot.
 * @param x X coordinate; all three coordinates at -1 preserve the old position.
 * @param y Y coordinate.
 * @param z Z coordinate.
 * @param direction Direction byte to assign.
 * @param resource_flag Low bit assigned to the resource flags.
 * @return Initialization result, or -1 if no actor entry is available.
 */
s32 func_80087A9C(s32 owner_id, s32 resource_id, s32 parameter_a, s32 parameter_b, s32 group, s32 x,
                  s32 y, s32 z, s32 direction, s32 resource_flag)
{
    s32 position[3];
    s32 saved_state;
    s32 var_v0;
    Entry *entry;
    Actor *actor;
    Resource *resource;
    Resource *resource_base;
    Actor *actor_base;
    Actor *final_base;

    entry = func_80087C9C();
    if (entry == (Entry *)-1)
    {
        return -1;
    }
    position[0] = entry->unk0;
    position[1] = entry->unk4;
    position[2] = entry->unk8;
    saved_state = entry->state.half[0] & 0x1FF;
    func_8006B240(parameter_a, parameter_b, resource_id);
    func_8006B4D0(entry->unk3A, resource_id);
    func_8006B7A0(entry->unk3A, 0);
    resource_base = g_field_resource_entries;
    resource = &resource_base[resource_id];
    resource->unk10 = (s32)((resource->unk10 & ~1) | (resource_flag & 1));
    actor_base = D_80105AE0;
    actor_base[entry->unk3A].unk14 = owner_id;
    entry->unk25 = 0;
    if (x == -1 && y == x && z == y)
    {
        entry->unk0 = position[0];
        entry->unk4 = position[1];
        entry->unk8 = position[2];
    }
    else
    {
        entry->unk0 = x << 8;
        entry->unk4 = y << 8;
        entry->unk8 = z << 8;
    }
    final_base = D_80105AE0;
    actor = &final_base[entry->unk3A];
    actor->unk10 = (s32)((actor->unk10 & ~0xF) | group);
    entry->unk21 = (s8)direction;
    entry->state.word = (s32)(((s32)entry->state.word & ~0x1FF) | saved_state);
    return func_8006C3FC(entry);
}
