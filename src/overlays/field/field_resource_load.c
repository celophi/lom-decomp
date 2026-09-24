#include "field_scene_transition.h"
#include "cdrom.h"
#include "common.h"
#include "field_actor_runtime.h"
#include "field_calls.h"
#include "field_actor_tables.h"
/** @brief Queued change to one actor: pose, animation track and sound; -1 leaves a field unchanged. */
typedef struct
{
    s16 actor_index;
    s16 pose;
    s16 animation;
    s16 sound;
} FieldPendingActorChange;

FieldActor* func_80087C9C(s32 arg0);

extern s32 D_80122B10;
extern FieldPendingActorChange D_80122B28[];

extern s32 D_80122B68[];
extern s32 D_80122B10;
extern s32 D_80122B20;

/** @brief Clear pending actors, load state, and both pending resource IDs. */
void func_800B01FC(void)
{
    s32 i;

    D_80122B10 = 0;
    D_80122B20 = 0;

    for (i = 1; i >= 0; i--)
    {
        D_80122B68[i] = 0;
    }
}

extern s32 D_80122B20;

/** @brief Start processing pending actor and resource changes.
 * @return The initial processing state, one.
 */
s32 func_800B0234(void)
{
    return D_80122B20 = 1;
}

extern s32 g_field_active_group;
void func_800B0A08(s32);
s32 func_800B0888(void);
void func_800B08FC(s32, s32);
void field_restart_actor_animation(u8*);
s32 VSync(s32);
s32 DrawSync(s32);
void func_800B34D0(s32);
/* Defined as (void) in field_actor_runtime.c; the original call still loads 1 into $a0. */
void field_restore_default_action_animation_mappings(s32);

/** @brief Advance pending actor changes through resource loading and installation. */
void func_800B0244(void)
{
    s32 state;
    s32 i;

    state = D_80122B20;
    if (state == 0)
    {
        return;
    }
    switch (state)
    {
    case 1:
        if ((g_field_actor_bindings[0].state | g_field_actor_bindings[1].state | g_field_actor_bindings[2].state) == 0)
        {
            D_80122B20 = 2;
        }
        break;

    case 2:
        func_800B0A08(1);
        D_80122B20 = 3;
        break;

    case 3:
    {
        FieldActor* object;
        s32 actor_track;
        u8 flags;

        for (i = 0; i < D_80122B10; i++)
        {
            object = g_field_actors + D_80122B28[i].actor_index;
            if (D_80122B28[i].pose != -1)
            {
                object->presence = 0;
                object->command = 0x8D;
                flags = (u8)D_80122B28[i].pose | (object->animation & 0x80);
                object->animation = flags;
                if (flags & 0x80)
                {
                    object->unk1B = 0;
                }
                else
                {
                    object->unk1B = 0x80;
                }
                object->unk2E = 1;
                object->unk24 = 1;
                object->control.word &= ~0x800;
                object->script_offset += 3;
                field_restart_actor_animation((u8*)object);
            }
            if (D_80122B28[i].animation != -1)
            {
                actor_track = func_800839F8(object->object_index, 0);
                if ((actor_track != -1) && (func_80083EEC(object->object_index, actor_track, D_80122B28[i].animation) != 0))
                {
                    field_start_actor_animation(actor_track, 0, 0);
                    g_field_object_states[object->object_index].contact.bytes.animation_actor_index = (u8)actor_track;
                }
            }
            if (D_80122B28[i].sound != -1)
            {
                func_800A3938(D_80122B28[i].sound, 0x80);
                VSync(0);
            }
        }
        D_80122B20 = 4;
        break;
    }

    case 4:
    {
        s16 scan_object_type;

        if (func_800B0888() == 0)
        {
            for (i = 0; i < D_80122B10; i++)
            {
                if (field_object_has_active_actor_tracks(D_80122B28[i].actor_index) != 0)
                {
                    break;
                }
            }
            if (i == D_80122B10)
            {
                for (i = 3; i < 0xD; i++)
                {
                    if (g_field_actors[i].presence != 0xFF && g_field_actors[i].command == 0x8D)
                    {
                        break;
                    }
                }
                if (i == 0xD)
                {
                    for (i = 1; i < 3; i++)
                    {
                        if (g_field_player_records[i].flags & 1)
                        {
                            scan_object_type = g_field_actors[i].command;
                            if ((scan_object_type == 0xAF) || (scan_object_type == 0xB1))
                            {
                                break;
                            }
                        }
                    }
                    if (i == 3)
                    {
                        i = 0;
                        do
                        {
                            g_field_object_parts[i].flags &= ~0x800000;
                            g_field_actors[i].command = 0;
                            i += 1;
                        } while (i < 3);
                        func_800A3938(0x79, 0x80);
                        i = 0;
                        do
                        {
                            if (D_80122B68[i] != 0)
                            {
                                func_800B08FC(1, i);
                                g_field_actors[i].command = 0x99;
                                g_field_actors[i].unk2E = 1;
                                g_field_actors[i].unk27 = 0;
                                g_field_actors[i].unk24 = 1;
                                g_field_actors[i].animation = (g_field_actors[i].animation & 0x80) + 0x11;
                                g_field_actors[i].control.word &= ~0x800;
                                g_field_object_states[i].movement.word &= ~0x1800;
                                field_restart_actor_animation((u8*)&g_field_actors[i]);
                            }
                            i += 1;
                        } while (i < 2);
                        i = 0;
                        do
                        {
                            if (g_field_player_records[i].flags & 1)
                            {
                                g_field_object_states[i].tint_timer = 0x3C;
                                g_field_object_states[i].movement.word |= 0x8000;
                            }
                            i += 1;
                        } while (i < 3);
                        DrawSync(0);
                        func_80084240();
                        field_restore_default_action_animation_mappings(1);
                        func_800B34D0(g_field_active_group);
                        D_80122B20 = 0;
                    }
                }
            }
        }
        break;
    }
    }
}

/**
 * @brief Update an existing pending actor entry or append a new one.
 * @param arg0 Actor identifier used to resolve the source record.
 * @param arg1 Value stored in the entry's second field.
 * @param arg2 Value stored in the entry's third field and used to select its reset value.
 * @param arg3 Value stored in the entry's fourth field.
 * @return Zero on success, or -1 when no entry can be created.
 */
s32 func_800B0710(s32 arg0, s32 arg1, s32 arg2, s32 arg3)
{
    FieldActor* rec;
    s32 i;
    FieldPendingActorChange* p;

    if (D_80122B10 == 8)
    {
        return -1;
    }
    rec = func_80087C9C(arg0);
    if (rec == (FieldActor*)-1)
    {
        return -1;
    }
    if (rec->presence == 0xFF)
    {
        return -1;
    }
    for (i = 0; i < D_80122B10; i++)
    {
        p = &D_80122B28[i];
        if (p->actor_index == rec->object_index)
        {
            p->actor_index = (s16)rec->object_index;
            p->pose = arg1;
            p->animation = arg2;
            if (arg2 != -1)
            {
                p->pose = 0;
            }
            p->sound = arg3;
            return 0;
        }
    }
    {
        FieldPendingActorChange* base = D_80122B28;
        s32 idx = D_80122B10;

        p = &base[idx];
    }
    p->actor_index = (s16)rec->object_index;
    p->pose = arg1;
    p->animation = arg2;
    if (arg2 != -1)
    {
        p->pose = 0xA;
    }
    D_80122B28[D_80122B10].sound = arg3;
    D_80122B10 += 1;
    return 0;
}

extern s32 D_80122B68[];

/** @brief Check whether either field resource slot is active.
 * @return One if a resource is active, otherwise zero.
 */
s32 func_800B0850(void)
{
    s32 i;

    for (i = 0; i < 2; i++)
    {
        if (D_80122B68[i] != 0)
        {
            return 1;
        }
    }

    return 0;
}

extern s32 D_80122B68[];

/**
 * @brief Checks whether either active field resource is already queued.
 *
 * @return 1 if an active resource is already queued, otherwise 0.
 */
s32 func_800B0888(void)
{
    s32* resource_index;
    s32 i;

    i = 0;
    resource_index = D_80122B68;
    do
    {
        if (*resource_index != 0)
        {
            if (cdrom_can_queue_resource((u16)*resource_index) == 0)
            {
                return 1;
            }
        }
        i++;
        resource_index++;
    } while (i < 2);

    return 0;
}

extern u8* D_8010D038;
extern s32 D_80122B18[];
extern s32 g_field_resource_cursor;

/**
 * @brief Install a queued resource and record its allocated memory range.
 * @param arg0 Flags whose low bit selects the resource state.
 * @param arg1 Resource slot index.
 */
void func_800B08FC(s32 arg0, s32 arg1)
{
    FieldResourceEntry* entry;
    FieldResourceEntry* base;
    u32 flags;

    if (D_80122B68[arg1] != 0)
    {
        field_release_resource_entry(arg1);
        base = g_field_resource_entries;
        entry = base + arg1;
        entry->slot_index = (u8)arg1;
        entry->unk8 = 0;
        field_set_party_palettes();
        entry->unkE = 0x2F;
        flags = entry->flags;
        flags &= ~1;
        flags |= arg0 & 1;
        entry->flags = flags;
        entry->start = (u8*)g_field_resource_cursor;
        field_unpack_resource_package((struct FieldCdBuffer*)(D_8010D038 + (0x8000 + arg1 * 0x18000)), D_80122B18[arg1], arg1, arg1);
        entry->end = (u8*)g_field_resource_cursor;
        entry->flags |= 2;
        D_80122B68[arg1] = 0;
    }
}

extern s32 D_80122B68[];
extern s32 D_80122B18[];
extern u8* D_8010D038;

/**
 * @brief Queue CD reads for each active field resource slot.
 * @param arg0 Resource-selection mode forwarded to field_get_actor_resource_id.
 */
void func_800B0A08(s32 arg0)
{
    s32 i;
    u8* buffer;

    for (i = 0; i < 2; i++)
    {
        if (g_field_player_records[i].flags & 1)
        {
            D_80122B68[i] = field_get_actor_resource_id(i, (struct FieldActorResourceSlot*)&g_field_player_records[i], arg0);
            buffer = D_8010D038 + 0x8000 + i * 0x18000;
            g_field_player_records[i].unk254 = (u16)D_80122B68[i];
            D_80122B18[i] = cdrom_queue_read((u16)D_80122B68[i], buffer);
        }
        else
        {
            D_80122B18[i] = 0;
            D_80122B68[i] = 0;
        }
    }
}
