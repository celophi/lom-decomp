/** @file field_actor_action_commands.c
 * @brief Key-based actor action requests, interaction parameters, and action-state changes.
 */

/* field308 */
#include "common.h"

typedef struct
{
    u8 pad0[0x14];
    s32 unk14; /* 0x14 */
    u8 pad18[0x23C - 0x18];
} RecA80105AE0;

typedef struct
{
    u8 pad0[0x25];
    u8 unk25; /* 0x25 */
    u8 pad26[0x54 - 0x26];
} RecB800FDF58;

extern RecA80105AE0 D_80105AE0[];
extern RecB800FDF58 D_800FDF58[];

/**
 * @see decomp.me (100%)
 */
s32 func_80089A68(s32 arg0)
{
    RecA80105AE0 *ra;
    RecB800FDF58 *rb;
    RecB800FDF58 *found;
    s32 i;

    rb = D_800FDF58;
    ra = D_80105AE0;
    for (i = 0; i < 0xD; i++, ra++, rb++)
    {
        if (ra->unk14 == arg0)
        {
            goto found_it;
        }
    }
    found = (RecB800FDF58 *) -1;
check:
    if (found != (RecB800FDF58 *) -1)
    {
        goto body;
    }
    return -1;
found_it:
    found = rb;
    goto check;
body:
    if (found->unk25 == 0xFE)
    {
        found->unk25 = 0;
    }
    else
    {
        found->unk25 = 0xFE;
    }
    return 0;
}


/* func_80089AE4 */
#include "common.h"

typedef struct
{
    u8 pad0[0x14];
    s32 unk14;
    u8 pad18[0x224];
} StateB80089AE4;

typedef struct
{
    u8 pad0[0x10];
    s16 unk10;
    u8 pad12[0xA];
    s32 unk1C;
    u8 pad20[8];
    u8 unk28;
    u8 pad29[1];
    s16 unk2A;
    u8 pad2C[0xE];
    u8 unk3A;
    u8 pad3B[2];
    u8 unk3D;
    u8 pad3E[0x16];
} RecordB80089AE4;




s32 func_800839F8();
s32 func_80083EEC();
void field_start_actor_animation(s32 arg0, s32 arg1, s32 arg2);

/**
 * @brief Find a matching field actor slot and update its animation state.
 * @param arg0 Field-state key to locate.
 * @param arg1 Animation argument, or -1 to skip animation dispatch.
 * @return Zero when a matching slot is updated, or -1 when no slot matches.
 */
s32 func_80089AE4(s32 arg0, s32 arg1)
{
    StateB80089AE4 *ra;
    RecordB80089AE4 *rb;
    s32 anim_id;
    RecordB80089AE4 *found;
    s32 i;

    rb = ((RecordB80089AE4 *)D_800FDF58);
    ra = ((StateB80089AE4 *)D_80105AE0);
    for (i = 0; i < 0xD; i++, ra++, rb++)
    {
        if (ra->unk14 == arg0)
        {
            goto found_it;
        }
    }
    found = (RecordB80089AE4 *)-1;
check:
    do
    {
        if (found != (RecordB80089AE4 *)-1)
        {
            goto body;
        }
    } while (0);
    return -1;
found_it:
    found = rb;
    goto check;
body:
    if (arg1 != -1)
    {
        anim_id = func_800839F8(found->unk3A, 0, rb);
        if ((anim_id != -1) && (func_80083EEC(found->unk3A, anim_id, arg1) != 0))
        {
            field_start_actor_animation(anim_id, 0, 0);
        }
    }

    found->unk2A = 0xBB;
    found->unk3D = 2;
    found->unk10 = 1;
    found->unk28 = 0;
    found->unk1C = (found->unk1C & ~0x1FF) | 2;
    return 0;
}


/* func_80089BE8 */
#include "common.h"

/** @brief Per-actor slot in ((SlotA *)D_80105AE0); stride 0x23C. */
typedef struct
{
    u8 pad0[0x14];
    s32 unk14;
    u8 pad18[0x224];
} SlotA;

/** @brief Per-actor record in ((EntryB *)D_800FDF58); stride 0x54. */
typedef struct
{
    u8 pad0[0x3A];
    u8 unk3A;
    u8 pad3B[0x19];
} EntryB;

/** @brief Record in the D_800FD818 table indexed by EntryB::unk3A; stride 0x268. */
typedef struct
{
    u8 pad0[0x25E];
    s16 unk25E;
    s16 unk260;
    s16 unk262;
    s16 unk264;
    s16 unk266;
} RecFD818;



extern RecFD818 D_800FD818[];

/**
 * @brief Update the indexed field record associated with an actor lookup key.
 * @param arg0 Actor lookup key.
 * @param arg1 Value written to the record's first configurable field.
 * @param arg2 Value written to the record's second configurable field.
 * @param arg3 Value written to the record's third configurable field.
 * @param arg4 Value written to the record's fourth configurable field.
 * @return Zero on success, or -1 when the actor or target record is unavailable.
 */
s32 func_80089BE8(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4)
{
    EntryB *scan;
    EntryB *found;
    SlotA *e;
    s32 i;

    scan = ((EntryB *)D_800FDF58);
    e = ((SlotA *)D_80105AE0);
    i = 0;
loop:
    i++;
    if (e->unk14 == arg0)
    {
        goto found_label;
    }
    e++;
    scan++;
    if (i < 13)
    {
        goto loop;
    }
    found = (EntryB *)-1;
check:
    if (found == (EntryB *)-1)
    {
        return -1;
    }
    goto success;

found_label:
    found = scan;
    goto check;

success:
    if (found->unk3A >= 3)
    {
        return -1;
    }
    D_800FD818[found->unk3A].unk260 = arg4;
    D_800FD818[found->unk3A].unk25E = 0;
    D_800FD818[found->unk3A].unk262 = arg1;
    D_800FD818[found->unk3A].unk264 = arg2;
    D_800FD818[found->unk3A].unk266 = arg3;
    return 0;
}


/* func_80089D44 */
#include "common.h"

/**
 * @brief Actor record fields used by state reset and companion repositioning.
 */
typedef struct
{
    s32 unk0;
    s32 unk4;
    s32 unk8;
    u8 padC[4];
    s16 unk10;
    u8 pad12[0x21 - 0x12];
    u8 unk21;
    u8 pad22[2];
    u8 unk24;
    u8 unk25;
    u8 pad26;
    u8 unk27;
    u8 pad28[2];
    s16 unk2A;
    u8 pad2C[2];
    s16 unk2E;
    u8 pad30[0xA];
    u8 unk3A;
    u8 pad3B[0x54 - 0x3B];
} FieldRecord;

/**
 * @brief Runtime actor state with packed resource and animation fields.
 */
typedef struct
{
    u32 unk0;
    u32 unk4;
    u32 unk8;
    s32 unkC;
    u8 pad10[4];
    s32 unk14;
    u8 pad18[0x174 - 0x18];
    u32 unk174;
    /** @brief Word and byte views of the packed animation state. */
    union

    {
        u32 word;
        struct

        {
            u8 byte0;
            u8 animation;
            u8 byte2;
            u8 byte3;
        } bytes;
    } flags;
    u8 pad17C[0x1AB - 0x17C];
    u8 unk1AB;
    u8 pad1AC[0x23C - 0x1AC];
} FieldState;

/** @brief Camera displacement words used by the visible-area bounds checks. */
typedef struct
{
    s32 unused0;
    s32 x;
    s32 unused8;
    s32 z;
} CameraPosition;



void field_start_actor_animation(s32, s32, s32);
void func_8006C3FC();
s32 func_800839F8();
s32 func_80083EEC();
void func_800A3938(s32, s32);
void func_800B48B8(s32);
void func_8008A0B0();

/**
 * @brief Reset a matching actor's state, animation resources and companion position.
 * @param key Runtime actor key to locate among thirteen slots.
 * @param requested_state New low state bits, or -1 to use state zero.
 * @param animation Animation request, or -1 to leave animation selection unchanged.
 * @param event_id Event to dispatch at value 0x80, or -1 to skip dispatch.
 * @return Zero on success, or -1 when no actor matches the key.
 * @note Keep repeated slot-index reads and the signed scan sentinel for matching.
 * @note GCC 2.7.2 CDK matches all 219 instructions (876 bytes).
 */
s32 func_80089D44(s32 key, s32 requested_state, s32 animation, s32 event_id)
{
    FieldRecord *scan_record;
    FieldState *scan_state;
    FieldState *lookup_state;
    FieldRecord *record;
    FieldRecord *candidate;
    s32 state;
    s32 slot;
    s32 i;
    s32 unavailable;
    s32 animation_slot;
    CameraPosition *camera;
    FieldState *runtime_flags;
    FieldState *resources;
    FieldState *runtime_mode;

    camera = (CameraPosition *)0x801ED480;
    state = requested_state;
    scan_record = ((FieldRecord *)D_800FDF58);
    lookup_state = ((FieldState *)D_80105AE0);
    for (i = 0; i < 0xD; i++, lookup_state++, scan_record++)

    {
        if (lookup_state->unk14 == key)

        {
            goto found_it;
        }
    }
    record = (FieldRecord *)-1;
check:
    if (record != (FieldRecord *)-1)

    {
        goto body;
    }
    return (s32)record;
found_it:
    record = scan_record;
    goto check;
body:
    record->unk25 = 0;
    record->unk2A = 0;
    record->unk10 = 0;
    record->unk2E = 1;
    if (state == -1)
    {
        state = 0;
    }
    record->unk27 = 0;
    record->unk24 = 1;
    record->unk21 = (u8) ((record->unk21 & 0x80) | state);
    func_8006C3FC(record);
    if (animation != -1)
    {
        animation_slot = func_800839F8(record->unk3A, 0);
        if ((animation_slot != -1) && (func_80083EEC(record->unk3A, animation_slot, animation) != 0))
        {
            field_start_actor_animation(animation_slot, 0, 0);
            ((FieldState *)D_80105AE0)[record->unk3A].flags.bytes.animation = animation_slot;
        }
    }
    if (event_id != -1)
    {
        func_800A3938(event_id, 0x80);
    }
    func_800B48B8(((FieldState *)D_80105AE0)[record->unk3A].unk14);
    ((FieldState *)D_80105AE0)[record->unk3A].unkC = 0;
    runtime_flags = &((FieldState *)D_80105AE0)[record->unk3A];
    runtime_flags->flags.word = (s32) (runtime_flags->flags.word & ~0x20);
    resources = &((FieldState *)D_80105AE0)[record->unk3A];
    resources->unk8 = (s32) ((resources->unk8 & 0xFF000000) | (resources->unk0 & 0xFFFFFF));
    resources->unk4 = (s32) (resources->unk0 & 0xFFFFFF);
    ((FieldState *)D_80105AE0)[record->unk3A].unk1AB = 0x3C;
    runtime_mode = &((FieldState *)D_80105AE0)[record->unk3A];
    runtime_mode->unk174 = (s32) (runtime_mode->unk174 | 0x8000);
    if (record->unk3A < 3U &&
        (record->unk0 <= -camera->x + 0xA00 ||
         record->unk0 >= -camera->x + 0x13600 ||
         record->unk8 <= -camera->z + 0xA00 ||
         record->unk8 >= -camera->z + 0x1B600))
         {
        slot = 0;
        unavailable = 0xFF;
        scan_state = ((FieldState *)D_80105AE0);
        candidate = ((FieldRecord *)D_800FDF58);
        for (; slot < 3; scan_state++, slot++, candidate++)

        {
            if (candidate->unk25 != unavailable && scan_state->unk4 != 0 && record->unk3A != slot)

            {
                break;
            }
        }
        if (slot != 3)
        {
            func_8008A0B0(record, slot, 1);
        }
    }
    return 0;
}


/* func_8008A0B0 */
#include "common.h"

/** @brief Field actor record with position, animation state, and runtime index. */
typedef struct
{
    s32 unk0;
    s32 unk4;
    s32 unk8;
    u8 padc[0x18];
    u8 unk24;
    u8 pad25[5];
    s16 unk2a;
    u8 pad2c[2];
    s16 unk2e;
    u8 pad30[10];
    u8 unk3a;
    u8 pad3b[0x19];
} CommandView5_FieldRecord;

/** @brief Runtime actor state containing position and collision path fields. */
typedef struct
{
    u8 pad0[0x50];
    s32 unk50;
    s32 unk54;
    s32 unk58;
    u8 pad5c[0x1A4 - 0x5C];
    s16 unk1a4;
    s16 unk1a6;
    u8 pad1a8[4];
    s32 unk1ac;
    s32 unk1b0;
    u8 pad1b4[0x23C - 0x1B4];
} CommandView5_FieldState;

/** @brief Position view at runtime state offset 0x50, with the 0x23C-byte stride. */
typedef struct
{
    s32 unk0;
    s32 unk4;
    s32 unk8;
    u8 padc[0x23C - 12];
} StatePosition;

/** @brief Actor part definition view used to select the collision footprint. */
typedef struct
{
    u8 pad0[0x2E];
    u8 unk2e;
    u8 pad2f[0x48 - 0x2F];
} PartDef;

/** @brief Map width and depth fields at the fixed map-header address. */
typedef struct
{
    s16 unk0;
    u16 unk2;
} MapBounds;

/** @brief Collision probe position, horizontal footprint, and height tolerance. */
typedef struct
{
    s32 x;
    s32 y;
    s32 z;
    u16 width;
    s16 height;
    u16 depth;
} CollisionQuery;


extern PartDef D_800FE3A0[];

extern StatePosition D_80105B30[];
s32 func_80060F58(CollisionQuery *, CollisionQuery *, void *, s32);
void func_8006304C(CollisionQuery *);
void func_8006C3FC();

/**
 * @brief Compute a path from an actor to another record, with direct fallback.
 * @param record Actor record whose runtime path should be updated.
 * @param source_index Record supplying the destination coordinates.
 * @param update_animation Nonzero to reset and apply the actor animation.
 */
void func_8008A0B0(CommandView5_FieldRecord *record, s32 source_index, s32 update_animation)
{
    CollisionQuery start;
    CollisionQuery goal;
    s32 path_length;
    MapBounds *bounds;
    s32 x;
    s32 map_depth;
    s32 map_width;
    s32 state_x;
    s32 state_z;
    s32 z;
    CommandView5_FieldRecord *source;

    bounds = (MapBounds *)0x801ED400;
    x = record->unk0;
    if ((x < 0) ||
        (map_width = bounds->unk0 << 8, ((x < map_width) == 0)) ||
        (z = record->unk8, (z < 0)) ||
        (map_depth = (s32) (bounds->unk2 << 0x10) >> 7, ((z < map_depth) == 0)) ||
        (state_x = ((CommandView5_FieldState *)D_80105AE0)[record->unk3a].unk50, (state_x < 0)) ||
        (state_x >= map_width) ||
        (state_z = ((CommandView5_FieldState *)D_80105AE0)[record->unk3a].unk58, (state_z < 0)) ||
        (state_z >= map_depth))
    {
        (&((CommandView5_FieldState *)D_80105AE0)[record->unk3a])->unk1a6 = 0;
        (&((CommandView5_FieldState *)D_80105AE0)[record->unk3a])->unk1ac = ((CommandView5_FieldRecord *)D_800FDF58)[source_index].unk0;
        (&((CommandView5_FieldState *)D_80105AE0)[record->unk3a])->unk1b0 = ((CommandView5_FieldRecord *)D_800FDF58)[source_index].unk8;
        ((CommandView5_FieldState *)D_80105AE0)[record->unk3a].unk1a4 = 1;
    }
    else
    {
        start.x = x;
        start.y = record->unk4;
        start.z = record->unk8;
        if ((&D_800FE3A0[record->unk3a])->unk2e == 0x40)
        {
            start.width = 0xC;
            start.depth = 8;
            goal.width = 0xC;
            goal.depth = 8;
        }
        else
        {
            start.width = 9;
            start.depth = 6;
            goal.width = 9;
            goal.depth = 6;
        }
        start.height = 0x10;
        goal.height = 0x10;
        func_8006304C(&start);
        D_80105B30[record->unk3a].unk0 = ((CommandView5_FieldRecord *)D_800FDF58)[source_index].unk0;
        source = &((CommandView5_FieldRecord *)D_800FDF58)[source_index];
        (&D_80105B30[record->unk3a])->unk4 = (s32) source->unk4;
        (&D_80105B30[record->unk3a])->unk8 = (s32) source->unk8;
        goal.x = source->unk0;
        goal.y = source->unk4;
        goal.z = source->unk8;
        path_length = func_80060F58(&start, &goal, (u8 *)&D_80105B30[record->unk3a] + 0x15C, 0);
        if (path_length <= 0)
        {
            (&((CommandView5_FieldState *)((u8 *)D_80105B30 - 0x50))[record->unk3a])->unk1a6 = 0;
            (&((CommandView5_FieldState *)((u8 *)D_80105B30 - 0x50))[record->unk3a])->unk1ac = (s32) source->unk0;
            (&((CommandView5_FieldState *)((u8 *)D_80105B30 - 0x50))[record->unk3a])->unk1b0 = (s32) source->unk8;
            ((CommandView5_FieldState *)((u8 *)D_80105B30 - 0x50))[record->unk3a].unk1a4 = 1;
        }
        else
        {
            (&((CommandView5_FieldState *)((u8 *)D_80105B30 - 0x50))[record->unk3a])->unk1a4 = path_length;
            (&((CommandView5_FieldState *)((u8 *)D_80105B30 - 0x50))[record->unk3a])->unk1a6 = 0;
        }
    }
    if (update_animation != 0)
    {
        record->unk2a = 0xB5;
        record->unk2e = 0xFF;
        record->unk24 = 1;
        func_8006C3FC(record);
    }
}


/* func_8008A4D0 */
#include "common.h"

typedef struct {
    u8 pad0[0xC];
    s32 unkC;
} FieldA4D0State;

typedef struct {
    u16 x;
    u16 y;
    u16 w;
    u16 h;
} FieldA4D0Rect;

void func_8008A4D0(FieldA4D0State *arg0, FieldA4D0Rect *rect, s32 arg2, s32 arg3)
{
    s32 xoff;
    s32 yoff;
    s32 mode;

    mode = arg0->unkC;
    if (mode >= 2)
    {
        yoff = 0;
        if (mode >= 9)
        {
            yoff = 0x100;
            xoff = 0x3C0 - ((mode - 9) << 6);
        }
        else
        {
            xoff = 0x340 - (mode << 6);
        }
    }
    else
    {
        yoff = 0;
        xoff = 0x380 - (mode << 7);
    }

    rect->x = (s16)rect->x >> 2;
    rect->x += xoff;
    rect->w = (s16)rect->w >> 2;
    rect->y += yoff;
    MoveImage2(rect, (arg2 >> 2) + xoff, arg3 + yoff);
}


/* func_8008A580 */
#include "common.h"

/** @brief Per-actor animation/geometry slot; array element stride 0x23C. */
typedef struct
{
    u8 pad0[0x14];
    s32 unk14;
    u8 pad18[0x3C - 0x18];
    u32 unk3C;
    u8 pad40[0x23C - 0x40];
} FieldActorSlot;

/** @brief Parallel per-actor record; array element stride 0x54. */
typedef struct
{
    u8 pad0[0x3A];
    u8 unk3A;
    u8 pad3B[0x54 - 0x3B];
} FieldActorRecord;



extern s32 D_8010A020[];

s32 func_8008404C();

/**
 * @brief Find an actor by key and request its slot update.
 * @param key Actor-slot lookup key.
 * @param arg1 Value forwarded to the actor update helper; meaning unknown.
 * @return -1 when absent, 1 when the helper returns zero, or 0 after marking the slot.
 * @see decomp.me (100%) TODO
 */
s32 func_8008A580(s32 key, s32 arg1)
{
    FieldActorRecord *scan;
    FieldActorRecord *found;
    FieldActorSlot *e;
    s32 i;
    s32 *slot;
    s32 *table;

    scan = ((FieldActorRecord *)D_800FDF58);
    e = ((FieldActorSlot *)D_80105AE0);
    i = 0;
loop:
    i++;
    if (e->unk14 == key)
    {
        goto found_label;
    }
    e++;
    scan++;
    if (i < 13)
    {
        goto loop;
    }
    found = (FieldActorRecord *)-1;
check:
    if (found == (FieldActorRecord *)-1)
    {
        return (s32)found;
    }
    goto body;
found_label:
    found = scan;
    goto check;
body:
    if (func_8008404C(found->unk3A, arg1) != 0)
    {
        table = D_8010A020;
        if (found->unk3A < 2)
        {
            slot = &table[found->unk3A];
        }
        else
        {
            slot = table + 2;
        }
        *slot = 1;
        ((FieldActorSlot *)D_80105AE0)[found->unk3A].unk3C = 0xFFFF;
    }
    else
    {
        return 1;
    }
    return 0;
}


/* func_8008A678 */
#include "common.h"

/** @brief Field state prefix and queued interaction targets in a 0x23C-byte slot. */
typedef struct
{
    u8 pad0[4];
    s32 active;
    u8 pad8[0x14 - 8];
    s32 actor;
    u8 pad18[0x16F - 0x18];
    u8 action;
    u8 pad170[8];
    union
    {
        s32 flags;
        struct
        {
            u8 pad[3];
            u8 count;
        } bytes;
    } state;
    u8 pad17c[4];
    u8 targets[0x23C - 0x180];
} CommandView8_FieldState;

/** @brief Seven-word interaction request consumed by func_800B5534. */
typedef struct
{
    s32 actor;
    s32 action;
    s32 param;
    s32 target;
    s32 unk10;
    s32 unk14;
    s32 mode;
} Request;


s32 func_800B5534();

/**
 * @brief Process queued actor interactions and clear the source queue count.
 * @param index Source field-state slot index.
 * @note Preserve the loop-carried stride and address casts for matching codegen.
 */
void func_8008A678(s32 index)
{
    Request request;
    CommandView8_FieldState *base;
    CommandView8_FieldState *loop_base;
    CommandView8_FieldState *source;
    CommandView8_FieldState *saved_source;
    CommandView8_FieldState *current;
    CommandView8_FieldState *target;
    CommandView8_FieldState *first_target;
    s32 i;
    s32 stride8;

    base = ((CommandView8_FieldState *)D_80105AE0);
    stride8 = index * 8;
    source = (CommandView8_FieldState *)((u8 *)base + ((stride8 + index) * 16 - index) * 4);
    i = 0;
    if (source->state.bytes.count != 0)
    {
        /* Keep separate copies for values that must survive the request call. */
        loop_base = base;
        saved_source = source;
        stride8 = index * 8;
        do
        {
            /* Integer address arithmetic preserves the target operand order. */
            current = (CommandView8_FieldState *)((((stride8 + index) * 16 - index) * 4) + (u32)loop_base);
            first_target = (CommandView8_FieldState *)(current->targets[i] * 0x23C + (u32)loop_base);
            first_target->state.flags &= ~0x80;
            target = (CommandView8_FieldState *)(current->targets[i] * 0x23C + (u32)loop_base);
            if (!(((u32)target->state.flags >> 5) & 1))
            {
                stride8 = index * 8;
                if (target->active != 0)
                {
                    request.actor = current->actor;
                    if ((u8)current->action < 0xB)
                    {
                        request.action = current->action;
                    }
                    else
                    {
                        request.action = 0xA;
                    }
                    request.target = loop_base[saved_source->targets[i]].actor;
                    request.param = 0;
                    request.unk10 = 0;
                    request.unk14 = 0;
                    request.mode = 1;
                    func_800B5534(&request);
                    goto stride_update;
                }
            }
            else
            {
stride_update:
                stride8 = index * 8;
            }
            i++;
        } while (i < ((CommandView8_FieldState *)((u8 *)loop_base + (((stride8 + index) * 16 - index) * 4)))->state.bytes.count);
    }
    ((CommandView8_FieldState *)D_80105AE0)[index].state.bytes.count = 0;
}


/* func_8008A840 */
#include "common.h"

typedef struct
{
    u8 pad0[0x2A];
    s16 unk2A;
    u8 pad2C[0x30 - 0x2C];
    u16 unk30;
    u8 pad32[0x54 - 0x32];
} Rec54;

typedef struct
{
    u8 pad0[4];
    s32 unk4;
    u8 pad8[0x14 - 8];
    s32 unk14;
    u8 pad18[0x16F - 0x18];
    u8 unk16F;
    u8 pad170[0x178 - 0x170];
    s32 unk178;
    u8 pad17C[0x23C - 0x17C];
} State23C;

typedef struct
{
    s32 unk0;
    s32 unk4;
    s32 unk8;
    s32 unkC;
    s32 unk10;
    s32 unk14;
    s32 unk18;
} ArgBlock;




void func_8008C4A8();
s32 func_800B5534();

/**
 * @brief Configure an interaction request between two field-state slots.
 * @param arg0 Source slot index.
 * @param arg1 Destination slot index.
 * @return Result returned by func_800B5534, or zero when the request cannot be started.
 */
s32 func_8008A840(s32 arg0, s32 arg1)
{
    ArgBlock arg_block;
    State23C *slot;
    State23C *other;
    State23C *base;
    s32 mask;

    if ((((Rec54 *)D_800FDF58)[arg0].unk2A == 0x91) || (((Rec54 *)D_800FDF58)[arg0].unk2A == 0x87))
    {
        return 0;
    }
    base = ((State23C *)D_80105AE0);
    other = &base[arg1];
    other->unk178 = other->unk178 & ~0x80;
    if (other->unk4 == 0)
    {
        return 0;
    }
    slot = &base[arg0];
    arg_block.unk0 = slot->unk14;
    if ((u8)slot->unk16F < 0xB)
    {
        arg_block.unk4 = (s32)slot->unk16F;
    }
    else
    {
        arg_block.unk4 = 0;
    }
    arg_block.unkC = ((State23C *)D_80105AE0)[arg1].unk14;
    if (arg0 < 2)
    {
        arg_block.unk8 = (s32)((Rec54 *)D_800FDF58)[arg0].unk30;
    }
    else
    {
        arg_block.unk8 = 0;
    }
    arg_block.unk10 = 0;
    arg_block.unk14 = 0;
    mask = ((u32)((State23C *)D_80105AE0)[arg0].unk178 >> 2) & 7;
    if (mask == 0)
    {
        mask = 1;
    }
    arg_block.unk18 = mask;
    func_8008C4A8(arg0);
    return func_800B5534(&arg_block);
}


/* func_8008A9D8 */
#include "common.h"

typedef struct
{
    u8 pad0[0x2A];
    s16 unk2A;
    u8 pad2C[0x54 - 0x2C];
} CommandView10_Rec54;

typedef struct
{
    u8 pad0[4];
    s32 unk4;
    u8 pad8[0x14 - 0x8];
    s32 unk14;
    u8 pad18[0x178 - 0x18];
    s32 unk178;
    u8 pad17C[0x23C - 0x17C];
} CommandView10_State23C;

typedef struct
{
    s32 unk0;
    s32 unk4;
    s32 unk8;
    s32 unkC;
    s32 unk10;
    s32 unk14;
    s32 unk18;
} CommandView10_ArgBlock;




/**
 * @brief Update an actor slot and dispatch the associated field action.
 * @param arg0 Source actor index.
 * @param arg1 Destination slot index.
 * @param arg2 Action argument.
 * @return Result from the dispatched action, or 0 when no action is performed.
 */
s32 func_8008A9D8(s32 arg0, s32 arg1, s32 arg2)
{
    CommandView10_State23C *base;
    CommandView10_State23C *slot;
    CommandView10_ArgBlock arg_block;

    if ((((CommandView10_Rec54 *)D_800FDF58)[arg0].unk2A != 0x91) && (((CommandView10_Rec54 *)D_800FDF58)[arg0].unk2A != 0x87))
    {
        base = ((CommandView10_State23C *)D_80105AE0);
        slot = &base[arg1];
        slot->unk178 = slot->unk178 & ~0x80;
        if (slot->unk4 != 0)
        {
            arg_block.unk0 = base[arg0].unk14;
            arg_block.unk4 = arg2;
            arg_block.unkC = slot->unk14;
            arg_block.unk8 = 0;
            arg_block.unk10 = 0;
            arg_block.unk14 = 0;
            arg_block.unk18 = 1;
            func_8008C4A8(arg0, slot, arg2);
            return func_800B5534(&arg_block);
        }
    }
    return 0;
}


/* func_8008AABC */
#include "common.h"

/**
 * @brief Per-actor animation/geometry slot; array element stride 0x23C.
 */
typedef struct
{
    u8 pad0[0x14];
    s32 unk14;
    u8 pad18[0x23C - 0x18];
} Struct_D80105AE0;

/**
 * @brief Argument block passed by pointer to func_800B5F60.
 */
typedef struct
{
    s32 unk0;
    s32 unk4;
    s32 unk8;
    s32 unkC;
    s32 unk10;
    s32 unk14;
    s32 unk18;
    s32 unk1C;
} ArgB5F60;


void func_800B5F60(ArgB5F60 *);

/**
 * @brief Builds a two-actor argument block and dispatches func_800B5F60.
 *
 * Copies the 0x14 field of actors @p a and @p b into a stack argument block,
 * marks it active (unk18 = 1), and passes it to func_800B5F60.
 */
void func_8008AABC(s32 a, s32 b)
{
    ArgB5F60 s;

    s.unk0 = ((Struct_D80105AE0 *)D_80105AE0)[a].unk14;
    s.unkC = ((Struct_D80105AE0 *)D_80105AE0)[b].unk14;
    s.unk18 = 1;
    func_800B5F60(&s);
}


/* func_8008AB2C */
#include "common.h"

/** @brief Field entry with animation state and actor index. */
typedef struct
{
    u8 pad0[0x2A];
    s16 unk2A;
    u8 pad2C[14];
    u8 unk3A;
    u8 pad3B[0x54 - 0x3B];
} Entry;
/** @brief Actor record with lookup ID, state flags, and action byte. */
typedef struct
{
    s32 unk0, unk4, unk8, unkC, unk10, unk14;
    u8 pad18[0x16F - 0x18];
    u8 unk16F;
    u8 pad170[8];
    s32 unk178;
    u8 pad17C[0x23C - 0x17C];
} Actor;
/** @brief Actor data record containing the pending action byte. */
typedef struct
{
    u8 pad0[0x259];
    u8 unk259;
    u8 pad25A[14];
} Data;
/** @brief Input context view containing the actor-zero event counter. */
typedef struct
{
    u8 pad0[0x3154];
    s32 unk3154;
} Pad;



extern Pad *g_pad_ctx;
extern s32 D_8010A000;
extern void func_8008B870(Entry *, s32);
/**
 * @brief Mark an actor for an action and dispatch its state-dependent update.
 * @param arg0 Actor ID to locate among the field records.
 * @param arg1 Reaction-mode selector forwarded to func_8008B870.
 * @return Zero when found, or -1 when no record has the requested ID.
 */
s32 func_8008AB2C(s32 arg0, s32 arg1)
{
    Entry *entry_cursor;
    Actor *actor_cursor;
    Entry *entry;
    s16 action;
    s32 pad_counter;
    s32 entry_count;
    Actor *actor;
    u8 *actor_base;

    entry_cursor = ((Entry *)D_800FDF58);
    actor_cursor = ((Actor *)D_80105AE0);
    entry_count = 0;
loop_1:
    entry_count += 1;
    if (actor_cursor->unk14 != arg0)
    {
        actor_cursor++;
        entry_cursor++;
        if (entry_count >= 0xD)
        {
            entry = (Entry *)-1;
        }
        else
        {
            goto loop_1;
        }
    }
    else
    {
        goto found;
    }
check:
    if (entry != (Entry *)-1)
    {
        goto body;
    }
    return -1;
found:
    entry = entry_cursor;
    goto check;
body:
    if (entry->unk3A == 0)
    {
        pad_counter = g_pad_ctx->unk3154;
        if (pad_counter != -1)
        {
            g_pad_ctx->unk3154 = (s32)(pad_counter + 1);
        }
    }
    actor_base = (u8 *)((Actor *)D_80105AE0);
    actor = (Actor *)(actor_base + entry->unk3A * 0x23C);
    actor->unkC = (s32)(actor->unkC | 0x10000000);
    if ((u8)entry->unk3A < 3U)
    {
        ((Data *)D_800FD818)[entry->unk3A].unk259 = 5;
    }
    else if (((Actor *)(actor_base + entry->unk3A * 0x23C))->unk8 < 0)
    {
        D_8010A000 = 5;
    }
    action = entry->unk2A;
    if (action == 0x87)
    {
        return 0;
    }
    if (action >= 0x88)
    {
        if (action != 0x91)
        {
            func_8008B870(entry, arg1);
        }
        return 0;
    }
    if (action >= 0x85)
    {
        switch (((Actor *)((u8 *)((Actor *)D_80105AE0) + entry->unk3A * 0x23C))->unk16F)
        {
        case 0:
        case 1:
        case 2:
        case 3:
        case 8:
        case 9:
        case 10:
            break;
        default:
            if (!(((u32)((Actor *)((u8 *)((Actor *)D_80105AE0) + entry->unk3A * 0x23C))->unk178 >> 6) & 1))
            {
                return 0;
            }
            break;
        }
    }
    func_8008B870(entry, arg1);
    return 0;
}
