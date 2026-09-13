#include "common.h"

/**
 * @file field_actor_transition_reset.c
 * @brief Field actor transition and reset logic (vram 0x800966F0..0x800970B0).
 *
 * Merged translation unit for the field actor transition/reset group.
 * Each member function keeps its original declaration environment at block
 * scope: several externs (D_800FDF58, g_field_actor_slots, D_80105AE0,
 * D_800FD818) and the local Entry/Actor typedefs are viewed with different,
 * conflicting types by different functions, so they must stay isolated per
 * function to reproduce the original per-file codegen exactly.
 */

/**
 * @brief Reset field actors or prepare their state for a field transition.
 * @param mode Zero resets the three actor slots; nonzero prepares transition state.
 * @param actor_data Actor data supplied by lifecycle callers; unused here.
 * @note GCC 2.7.2 CDK match: 92.362240%, 158/196 exact instructions.
 * @note Explicit loops preserve the original pointer lifetimes; both flag stores
 *       at offset 0x178 are retained through the volatile field.
 */
void func_800966F0(s32 mode, void *actor_data)
{
    /** @brief Actor resource entry with a 0x54-byte stride. */
    typedef struct
    {
        u8 pad0[0x1C];
        s32 unk1c;
        u8 unk20;
        u8 unk21;
        u8 pad22[2];
        u8 unk24;
        u8 pad25[2];
        u8 unk27;
        u8 pad28[2];
        s16 unk2a;
        u8 pad2c[2];
        u16 unk2e;
        u16 unk30;
        u8 pad32[0x22];
    } FieldTransitionEntry;

    /** @brief Sparse actor state with a 0x23C-byte stride. */
    typedef struct
    {
        u8 pad0[0xC];
        s32 unkc;
        u8 pad10[0x164];
        s32 unk174;
        volatile s32 unk178;
        u8 pad17c[0x11];
        u8 unk18d;
        u8 pad18e[0x1d];
        u8 unk1ab;
        u8 pad1ac[0x90];
    } FieldTransitionActor;

    /** @brief Controller slot state with a 0x268-byte stride. */
    typedef struct
    {
        u8 pad0[0x25A];
        u8 unk25a, unk25b, unk25c, unk25d;
        u8 pad25e[10];
    } FieldTransitionSlot;

    /** @brief Actor part state with a 0x48-byte stride. */
    typedef struct
    {
        u8 pad0[0x34];
        s32 unk34;
        u8 pad38[0x10];
    } FieldTransitionPart;

    extern void func_8006C3FC(FieldTransitionEntry *);
    extern void func_80083BC0(FieldTransitionEntry *, void *, s32);
    extern void func_80086494(s32);
    extern void func_8008A0B0(FieldTransitionEntry *, s32, s32);
    extern void func_80092124(void);
    extern void func_80096B54(void);
    extern void func_800A2DD8(s32);
    extern void func_800A6204(void);
    extern void func_800AB710(void);
    extern void func_800B0234(void);
    extern void func_800B34D0(s32);
    extern u8 D_800FB3C8[];
    extern FieldTransitionSlot D_800FD818;
    extern FieldTransitionEntry D_800FDF58;
    extern FieldTransitionPart D_800FE3A0;
    extern s32 D_800FE754;
    extern FieldTransitionActor D_80105AE0;
    extern s32 D_8010AE54, D_8010AE5C, D_8010CFD0, D_8010D020, D_8011F420, D_8012291C;
    extern u32 D_801229A0;
    extern u8 g_field_actor_slots[];
    extern u8 *g_pad_ctx;

    FieldTransitionActor *actor;
    FieldTransitionEntry *entry;
    FieldTransitionSlot *slot;
    s32 animation;
    s32 flags;
    s32 pad_offset;
    s32 index;
    s32 actor_offset;
    s32 template_offset;
    u32 *saved_buttons;
    u32 buttons;
    u8 *slot_bytes;
    u8 animation_flags;
    void *actor_template;
    u8 *actor_slot;
    u8 *actor_base;
    u8 **pad_base;
    FieldTransitionEntry *entry_arg;
    s32 force, mask;
    FieldTransitionEntry *companion;
    FieldTransitionPart *part;

    func_800A6204();
    if (mode == 0)
    {
        index = 0;
        func_80096B54();
        D_8010AE5C = 0;
        func_800B34D0(0);
        actor_base = g_field_actor_slots;
        template_offset = index;
        entry = &D_800FDF58;
        actor_offset = 0x9100;
        actor = &D_80105AE0;
        D_8010AE54 = 1;
    reset_actor:
    {
        mask = ~0x8000;
        entry_arg = entry;
        force = 1;
        actor_template = template_offset + D_800FB3C8;
        template_offset += 0x244;
        actor_slot = actor_offset + actor_base;
        actor_offset += 0x244;
        actor->unk1ab = 0;
        actor->unkc = (s32)(actor->unkc & 0x200);
        actor->unk174 = (s32)(actor->unk174 & mask);
        flags = actor->unk178 & ~0x20;
        actor->unk178 = flags;
        actor->unk178 = (s32)(flags & ~0x80);
        actor_slot[0x225] = 0;
        func_80083BC0(entry_arg, actor_template, force);
        actor->unk18d = 0;
        entry->unk30 = 0;
        func_800A2DD8(index);
        func_80086494(index);
        entry = (FieldTransitionEntry *)((u32)entry + 0x54);
        index += 1;
        actor = (FieldTransitionActor *)((u32)actor + 0x23C);
    }
        if (index < 3)
        {
            goto reset_actor;
        }
        D_8010CFD0 = 0;
        return;
    }
    if (D_8010D020 != 0)
    {
        func_800AB710();
    }
    D_800FE754 = mode;
    func_80092124();
    index = 0;
    pad_base = &g_pad_ctx;
    slot = &D_800FD818;
    saved_buttons = &D_801229A0;
    pad_offset = index;
    do
    {
        index += 1;
        buttons = *(u32 *)(*pad_base + pad_offset + 0x610);
        pad_offset += 0x250;
        *saved_buttons = buttons >> 8;
        slot->unk25d = 0;
        slot->unk25c = 0;
        slot->unk25b = 0;
        slot->unk25a = 0;
        slot = (FieldTransitionSlot *)((u32)slot + 0x268);
        saved_buttons++;
    } while (index < 3);
    D_8012291C = 1;
    D_8011F420 = *(s32 *)(g_pad_ctx + 0x2C);
    func_800B0234();
    D_800FDF58.unk24 = 1;
    D_800FDF58.unk2e = 1;
    D_800FDF58.unk27 = 0;
    D_800FDF58.unk1c = (s32)(D_800FDF58.unk1c & ~0x800);
    animation = D_800FDF58.unk21 & 0x7F;
    animation %= 5;
    animation_flags = (D_800FDF58.unk21 & 0x80) + animation;
    D_800FDF58.unk21 = animation_flags;
    func_8006C3FC(&D_800FDF58);
    index = 1;
    if (D_8010D020 == 0)
    {
        part = &D_800FE3A0;
        part = (FieldTransitionPart *)((u32)part + index * 0x48);
        companion = (FieldTransitionEntry *)((u32)&D_800FDF58 + 0x54);
        slot_bytes = (u8 *)&D_800FD818;
        slot_bytes += index * 0x268;
    reset_companion:
    {
        if (*slot_bytes & 1)
        {
            if (*(u16 *)&companion->unk1c & 0x1FF)
            {
                companion->unk2a = 0xAF;
                companion->unk2e = 0xFFFF;
                part->unk34 = (s32)(part->unk34 | 0x800000);
            }
            else
            {
                func_8008A0B0(companion, 0, 1);
                part->unk34 = (s32)(part->unk34 | 0x800000);
                if (companion->unk2a == 0xB5)
                {
                    companion->unk2a = 0xB1;
                }
            }
        }
        part = (FieldTransitionPart *)((u32)part + 0x48);
        companion = (FieldTransitionEntry *)((u32)companion + 0x54);
        index += 1;
        slot_bytes += 0x268;
    }
        if (index < 3)
        {
            goto reset_companion;
        }
    }
}

/**
 * @brief Scan the first 13 actor entries for one flagged and in a transition anim.
 * @return The entry index plus 0x100 for the first matching actor, else 0.
 */
s32 func_80096A00(void)
{
    typedef struct
    {
        u8 pad0[0x25];
        u8 unk25;    /* 0x25 */
        u8 pad26[0x2A - 0x26];
        s16 unk2A;   /* 0x2A */
        u8 pad2C[0x54 - 0x2C];
    } Entry;

    extern Entry D_800FDF58[];

    s32 i;

    for (i = 0; i < 13; i++)
    {
        if (D_800FDF58[i].unk25 != 0xFF)
        {
            if (D_800FDF58[i].unk2A == 0x90 || D_800FDF58[i].unk2A == 0x94 ||
                D_800FDF58[i].unk2A == 0x93 || D_800FDF58[i].unk2A == 0xAE ||
                D_800FDF58[i].unk2A == 0x94 || D_800FDF58[i].unk2A == 0x92)
            {
                return i + 0x100;
            }
        }
    }
    return 0;
}

/**
 * @brief Report whether an active transitioning actor still has a live slot.
 * @return 1 if a matching actor with a nonzero slot flag is found, else 0.
 */
s32 func_80096A90(void)
{
    typedef struct {
        u8 pad0[0x25];
        u8 unk25;
        u8 pad26[0x2A - 0x26];
        s16 unk2A;
        u8 pad2C[0x54 - 0x2C];
    } Entry;

    typedef struct {
        u8 pad0[0x23A];
        u8 unk23A;
        u8 pad23B[0x244 - 0x23B];
    } Actor;

    extern Entry D_800FDF58[];
    extern Actor g_field_actor_slots[];
    extern s32 D_8010D020;

    s32 i;

    if (D_8010D020 == 0)
    {
        return 0;
    }

    for (i = 0; i < 2; i++)
    {
        if (D_800FDF58[i].unk25 != 0xFF)
        {
            if (D_800FDF58[i].unk2A == 0x90 || D_800FDF58[i].unk2A == 0x94 ||
                D_800FDF58[i].unk2A == 0x93 || D_800FDF58[i].unk2A == 0xAE ||
                D_800FDF58[i].unk2A == 0x94 || D_800FDF58[i].unk2A == 0x8E ||
                D_800FDF58[i].unk2A == 0x92)
            {
                if (g_field_actor_slots[i + 64].unk23A != 0)
                {
                    return 1;
                }
            }
        }
    }
    return 0;
}

/**
 * @brief Process pending track cleanup requests and reset their rendering state.
 * @note WIP: 96.333336% gcc272_cdk, with address and allocation differences.
 * @note Preserve the labeled scans to avoid extra compiler-generated loop pointers.
 */
void func_80096B54(void)
{
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

/**
 * @brief Complete a pending field reset after fifteen consecutive idle checks.
 */
void func_80096E60(void)
{
    /** @brief Position and animation fields in a 0x54-byte actor entry. */
    typedef struct Entry
    {
        s32 x, y, z;
        u8 padC[16];
        s32 flags;
        u8 pad20;
        u8 state;
        u8 pad22[2];
        u8 enabled, slot, pad26, unk27;
        u8 pad28[2];
        s16 anim;
        u8 pad2C[2];
        s16 timer;
        u8 tail[0x54 - 0x30];
    } Entry;
    /** @brief Resource, status, and overlapping flag fields in a 0x23C-byte actor record. */
    typedef struct Actor
    {
        u8 pad0[12];
        s32 resource;
        u8 pad10[0x2C];
        s32 value;
        u8 pad40[0x134];
        s32 flags174;
        union
        {
            s32 word;
            u8 bytes[4];
        } flags178;
        u8 tail[0x23C - 0x17C];
    } Actor;

    void akao_cmd_f1(void);                       /* extern */
    void field_clear_actor_slots(void);           /* extern */
    s32 field_find_active_special_attack_actor(); /* extern */
    void field_initialize_actor_slots(void);      /* extern */
    void field_reset_global_color_scale(void);    /* extern */
    void func_8005A0D0(s32, s32, s32, s32);       /* extern */
    s32 func_8005B218();                          /* extern */
    void func_80067AA4(void);                     /* extern */
    void func_80068028(void);                     /* extern */
    void func_8006C3FC(u8 *);                     /* extern */
    void func_80084240(void);                     /* extern */
    s32 func_80096A00();                          /* extern */
    s32 func_80096A90();                          /* extern */
    void func_800A3938(s32, s32);                 /* extern */
    void func_800A6204(void);                     /* extern */
    extern s32 D_800F2278;
    extern s32 D_800F227C;
    extern s32 D_800F2280;
    extern u8 D_800FD818[];
    extern u8 D_800FDF58[];
    extern s32 D_800FE754;
    extern u8 D_80105AE0[];
    extern s32 D_8010AE54;
    extern s32 D_8010AE5C;
    extern s32 D_8010CFD0;
    extern s32 D_8010D020;
    extern s32 D_801227C8;

    Entry *entry;
    Actor *actor;
    u8 *slot_cursor;
    u8 *slot_base;

    if (D_8010AE54 != 0)
    {
        if ((field_find_active_special_attack_actor() == 0) && (func_80096A00() == 0) &&
            (D_801227C8 == 0) && (func_8005B218() == 0) && (func_80096A90() == 0))
        {
            D_8010CFD0 += 1;
        }
        else
        {
            D_8010CFD0 = 0;
        }
        if (D_8010CFD0 == 0xF)
        {
            field_initialize_actor_slots();
            field_clear_actor_slots();
            func_80067AA4();
            func_80084240();
            D_800F2280 = 0;
            D_800F227C = 0;
            D_800F2278 = 0;
            D_800FE754 = D_8010AE5C;
            func_80068028();
            akao_cmd_f1();
            field_reset_global_color_scale();
            func_8005A0D0(-1, 0x100, 0x100, 0x100);
            func_800A6204();
            func_800A3938(0x24, 0x80);
            actor = (Actor *)D_80105AE0;
            entry = (Entry *)D_800FDF58;
            slot_base = D_800FD818;
            slot_cursor = slot_base;
        loop:
        {
            if (*slot_cursor & 1)
            {
                actor->resource = 0;
                actor->flags178.word &= ~1;
                actor->flags178.word &= ~2;
                actor->flags178.word &= ~0x20;
                actor->flags178.bytes[3] = 0;
                actor->value = 0xFFFF;
                entry->y = 0;
                if ((D_8010D020 != 0) && (entry->anim == 0x8E))
                {
                    entry->state = (entry->state & 0x80) + 0x31;
                }
                else
                {
                    entry->state = (entry->state & 0x80) + 0x13;
                }
                entry->timer = 1;
                entry->enabled = 1;
                entry->anim = 0;
                entry->slot = 0;
                entry->unk27 = 0;
                entry->flags = (s32)(entry->flags & ~0x800);
                actor->flags174 = (s32)(actor->flags174 & ~0x1800);
                func_8006C3FC((u8 *)entry);
            }
            actor++;
            slot_cursor += 0x268;
            entry++;
        }
            if ((s32)slot_cursor < ((s32)slot_base + 0x738))
            {
                goto loop;
            }
            slot_base = 0;
            D_8010AE54 = (s32)slot_base;
        }
    }
}
