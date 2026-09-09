#include "common.h"

/** @brief Actor position, facing, and animation state in a 0x54-byte entry. */
typedef struct
{
    s32 unk0, unk4, unk8;
    u8 padC[0x1B - 0xC];
    u8 unk1B;
    u8 pad1C[5];
    u8 unk21;
    u8 pad22[2];
    u8 unk24;
    u8 pad25[2];
    u8 unk27;
    u8 pad28[2];
    s16 unk2A;
    u8 pad2C[2];
    s16 unk2E;
    u8 pad30[11];
    u8 unk3B;
    u8 pad3C[0x54 - 0x3C];
} Entry;
/** @brief Collision interaction fields in a 0x23C-byte actor record. */
typedef struct
{
    u8 pad0[0x18];
    u16 unk18;
    u8 pad1A[0x18E - 0x1A];
    u8 unk18E;
    u8 pad18F[0x23C - 0x18F];
} Actor;
/** @brief Resource data pointer and direction flags. */
typedef struct
{
    u8 *start;
    u8 pad4[12];
    u32 flags;
} Resource;
extern Entry D_800FDF58[];
extern Actor D_80105AE0[];
extern Resource g_field_resource_entries[];
extern void func_8006C3FC(Entry *, u8 *);
extern s32 func_800987DC(Entry *, s32 *, s32);
extern void func_80098FC4(Entry *, s32);
extern void func_800A3938(s32, s32);
extern void func_800AF824(s32);
extern s32 rcos(s32);
extern s32 rsin(s32);
/**
 * @brief Probe ahead of an idle actor and begin its available interaction.
 * @param entry Actor whose position and facing select the interaction target.
 */
void func_80098DD4(Entry *entry)
{
    s32 position[3];
    s32 actor_index;
    s32 result_or_address;
    u8 direction;
    s32 masked;
    Actor *actor;
    Actor *actor_base;

    if (entry->unk2A != 0)
    {
        return;
    }
    position[0] = entry->unk0;
    position[1] = entry->unk4;
    position[2] = entry->unk8;
    position[0] += rcos(entry->unk1B * 0x10);
    position[2] -= rsin(entry->unk1B * 0x10);
    result_or_address = func_800987DC(entry, position, 1);
    actor_index = result_or_address & 0x7FFF;
    if (result_or_address != 0)
    {
        actor_base = D_80105AE0;
        /* Reuse the result carrier for the actor address to preserve register allocation. */
        result_or_address = (s32)&actor_base[actor_index];
        actor = (Actor *)result_or_address;
        if (actor->unk18E != 0)
        {
            func_800A3938(0x7D, 0x80);
            func_800AF824(actor_index);
            return;
        }
        if (actor->unk18 & 2)
        {
            func_80098FC4(&D_800FDF58[actor_index], 0);
            entry->unk2A = 0x81;
            entry->unk2E = 1;
            if (g_field_resource_entries[entry->unk3B].flags & 1)
            {
                entry->unk21 = (u8)(entry->unk21 & 0x80);
            }
            else
            {
                direction = entry->unk21;
                masked = direction & 0x7F;
                entry->unk21 = (masked % 5) | (direction & 0x80);
            }
            entry->unk27 = 0;
            entry->unk24 = 1;
            func_8006C3FC(entry, g_field_resource_entries[entry->unk3B].start);
        }
    }
}
