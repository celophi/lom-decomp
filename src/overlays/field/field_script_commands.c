#include "game_audio.h"
#include "common.h"
#include "field_calls.h"
#include "field_actor_runtime.h"

#define FIELD_STATE_RECORD_COUNT 11
#define FIELD_STATE_RECORD_SIZE 0x68
#define FIELD_STATE_RECORD(offset) ((FieldStateRecord*)(g_field_battle + (offset)))

typedef void (*FieldCommandHandler)(s32 value, u8* params);

/** @brief Script request containing actor selection, operation, and value words. */
typedef struct
{
    u16 index;
    u16 operation;
    u32 a, b, c, d, e, f, g;
} Request;

/** @brief Actor fields read or written by the script operations. */
typedef struct
{
    u32 maximum, current, pad8, flags;
    u8 pad10[8];
    u16 flags18;
    u8 pad_1a[0x2E];
    u16 x, y;
    u8 active;
    u8 pad_4d[3];
    u32 a, b, c;
    u8 pad_5c[4];
    u8 channels[4];
} Actor;

/** @brief One 0x94-byte element in the g_field_runtime table; unk0 is its id. */
typedef struct Elem
{
    u8 unk0; /* 0x00 */
    u8 pad[0x93];
} Elem;

/** @brief g_field_runtime table: count at 0x400 (u16/u32 union), elements at 0x430. */
typedef struct Foo
{
    u8 pad0[0x400];
    union
    {
        u16 count; /* 0x400 as lhu */
        u32 flags; /* 0x400 as lw */
    } f400;
    u8 pad1[0x430 - 0x404];
    Elem elem[1]; /* 0x430 */
} Foo;

/** @brief Command payload for a two-dimensional bounds test. */
typedef struct
{
    u16 result, mode;
    u32 x, unused8, z;
    u32 left, top, width, height;
} BoundsCommand;

typedef struct FieldNibbleRecord
{
    u16 offset;
    u16 row;
    s32 low;
    s32 high;
} FieldNibbleRecord;

typedef struct SourceEntry
{
    u8 pad[4];
    u8 packed;
} SourceEntry;

typedef struct
{
    /** 0x00 horizontal position. */
    s16 x;
    /** 0x02 vertical position. */
    s16 y;
    /** 0x04 depth. */
    s16 z;
} FieldPos;

typedef struct
{
    /* 0x0 */ u8 pad0[0x4];
    /* 0x4 */ s32 unk4;
    /* 0x8 */ s32 unk8;
    /* 0xC */ s32 unkC;
    /* 0x10 */ s32 unk10;
    /* 0x14 */ s32 unk14;
} UnkStruct800BDC40;

/** @brief Script command result, opcode, and three word-sized arguments. */
typedef struct Command
{
    u16 result, opcode;
    s32 arg4, arg8;
    u32 argC;
} Command;

/** @brief Partial UnkStruct800BDF00 layout used by func_800BDF00. */
typedef struct
{
    u16 unk0;
    u16 unk2;
} UnkStruct800BDF00;

/** @brief Partial LocalRecord layout used by func_800BDF00. */
typedef struct
{
    s32 status;
    u16 unk4;
    u16 pad6;
} LocalRecord;

typedef struct
{
    s32 x;
    s32 y;
    s32 z;
} FieldPosition;

typedef struct
{
    u8 pad0[4];
    s32 active;
} FieldRecordState;

typedef struct
{
    u8 pad0[0x28];
    u8 flags;
    u8 pad29[3];
    u8 object_id;
    u8 pad2D[11];
    FieldRecordState* state;
} FieldStateRecord;

typedef struct
{
    u8 pad0[4];
    u8 reference_id;
} FieldDistanceSource;

typedef struct
{
    u8 pad0[4];
    s32 flag_mask;
} FieldRecordFilter;

typedef struct
{
    s32 object_id;
    s32 distance;
} FieldDistanceEntry;

typedef struct
{
    s32 count;
    FieldDistanceEntry entries[1];
} FieldDistanceList;

/** @brief Camera command mode, actor reference, and explicit coordinates. */
typedef struct
{
    u16 unk0, unk2;
    s32 unk4, unk8, unkC;
} CameraCommand;

/** @brief Camera position fields used to preserve the current scroll origin. */
typedef struct
{
    s32 unk0, unk4, unk8, unkC;
} Camera;

/** @brief Saved scroll coordinates in the field state. */
typedef struct
{
    u8 pad[0x424];
    s32 unk424, unk428;
} State;

/** @brief Scene dimensions used to clamp the camera position. */
typedef struct
{
    s16 width;
    u16 height;
} Bounds;

typedef struct
{
    s16 unk0;
    s16 unk2;
    s32 unk4;
    s32 unk8;
    s8 unkC[4];
    s32 unk10;
} UnkStruct800BE2F0;

typedef struct
{
    /* 0x0 */ u16 unk0;
    /* 0x4 */ s32 unk4;
    /* 0x8 */ s32 unk8;
    /* 0xC */ s32 unkC;
} UnkStruct800BE324;

typedef struct
{
    u16 unk0;
    u16 unk2;
    s32 unk4;
    s32 unk8;
    s32 unkC;
} SomeStruct;

/** @brief Field-script command payload consumed by func_800BE404. */
typedef struct
{
    u16 unk0;
    s32 unk4;
    s32 unk8;
    s32 unkC;
    s32 unk10;
    s32 unk14;
    s32 unk18;
} ArgB800BE404;

/** @brief Actor state returned by func_800C1B60; unk90 holds status flags. */
typedef struct
{
    u8 pad0[0x90];
    u32 unk90;
} CmdB800BE404;

/** @brief View of g_field_runtime exposing the byte consumed at offset 0x403. */
typedef struct
{
    u8 pad0[0x403];
    u8 unk403;
} StructB78B800BE404;

/** @brief Field command record containing a selector and resolved position. */
typedef struct
{
    u16 unk0;
    u16 pad2;
    s32 unk4;
    s32 unk8;
    s32 unkC;
} FieldPositionCommand;

void func_800C1EC8(s32, void*, s32);
extern FieldCommandHandler D_800F0E10[];
extern u8* g_field_script;
extern Actor* field_find_object_state(s32);
extern u8* g_field_runtime;
extern s32 g_field_interaction_active;
/* Int parameters on purpose: with the (s32, u8, s8) definition the calls would narrow their arguments. */
s32 field_queue_actor_event(s32 owner_id, s32 event_id, s32 argument);
s32 field_get_actor_position(s32, s32*);
extern u8* func_800C1E40(s32 arg0);
extern void func_8005AF5C(s32 obj_index, s32 part_index, FieldPos* out);
extern u8* field_find_free_inventory_record(void);
extern void func_800BD520(s32, s32, s32);
extern void func_800C1EC8(s32, void*, s32);
extern u8 *g_field_game_state, *g_field_runtime, *D_80123FC4, *g_field_script;
extern u8* func_800B2A9C(s32 value);
extern void func_800C1F28(u32* arg0);
extern void (*D_800F0E54[])(s32* arg0, void* arg1, void* arg2);
s32 func_800C1FBC(FieldPosition* first, FieldPosition* second);
extern u8* g_field_battle;
s32 field_set_actor_position(s32, s32, s32, s32);
extern s32 field_get_actor_position(s32, s32*);
extern s32 g_field_scripted_scroll_frames, g_field_scripted_scroll_target_x, g_field_scripted_scroll_target_z;
extern Camera* g_field_scene_state;
extern s32 field_set_actor_position(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
s32 field_set_actor_position(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
void field_revive_actor(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
void* func_800B2B08(void);
CmdB800BE404* func_800C1B60(s32 arg0);
void func_800C1EC8(s32 arg0, void* arg1, s32 arg2);

/**
 * @brief Dispatches a small field command or reports an unsupported command.
 *
 * @param value Command value or function-table index.
 * @param params Parameter block forwarded to table-dispatched commands.
 * @note 100% match. The second argument is part of the original handler ABI;
 *       preserving it also reproduces the target index register lifetime.
 */
void func_800BD6F4(s32 value, u8* params)
{
    if (value < 0x11)
    {
        D_800F0E10[value](value, params);
        return;
    }
    record_game_diagnostic(0x8001, 3, value, 0);
}

/**
 * @brief Thin stack-frame wrapper forwarding an offset pointer into func_800C1EC8.
 * @param arg0 Unused.
 * @param arg1 Base value; func_800C1EC8 is called with arg1 + 4.
 */
void func_800BD750(s32 arg0, s32 arg1)
{
    func_800C1EC8(0, (void*)(arg1 + 4), 0x20);
}

/**
 * @brief Dispatch eight script operations that read or update actor fields.
 * @param unused Unused incoming argument.
 * @param request Actor selector, operation code, and input/output values.
 */
void func_800BD778(s32 unused, Request* request)
{
    s32 index;
    u32 divisor;
    Actor* actor;
    if (request->index == 0xFF)
    {
        index = *g_field_script;
    }
    else
    {
        index = request->index;
    }
    actor = field_find_object_state(index);
    if (actor != (Actor*)-1)
    {
        switch (request->operation)
        {
        case 0:
            actor->a = request->a;
            actor->b = request->b;
            actor->c = request->c;
            break;
        case 1:
            actor->current = request->a;
            actor->maximum = request->b;
            break;
        case 2:
            actor->flags |= request->a;
            actor->flags18 |= (u16)request->b;
            break;
        case 3:
            actor->flags &= request->a;
            actor->flags18 &= (u16)request->b;
            break;
        case 4:
            request->a = actor->x;
            request->b = actor->y;
            request->c = actor->active & 1;
            break;
        case 5:
            request->a = actor->current;
            request->b = actor->maximum;
            divisor = actor->maximum;
            if (divisor != 0)
            {
                request->c = (actor->current * 8) / divisor;
            }
            else
            {
                request->c = 0;
            }
            break;
        case 6:
            actor->channels[0] = request->d;
            actor->channels[1] = request->e;
            actor->channels[2] = request->f;
            actor->channels[3] = request->g;
            break;
        case 7:
            actor->x = request->a;
            break;
        }
    }
}

void func_800BD95C(void)
{
    func_800B168C(1);
}

/**
 * @brief Thin stack-frame wrapper around func_800B168C with a fixed arg.
 */
void func_800BD97C(void)
{
    func_800B168C(2);
}

/**
 * @brief Re-issue every active table element and clear the batch-dirty flags.
 *
 * After func_800B177C, walks the @c count live elements of @c g_field_runtime,
 * dispatching field_queue_actor_event for each element's id, then clears the 0x60000 bits
 * of the flag word at 0x400 and resets @c g_field_interaction_active.
 *
 */
void func_800BD99C(void)
{
    s32 i;

    func_800B177C();
    for (i = 0; i < (s32)((Foo*)g_field_runtime)->f400.count; i++)
    {
        field_queue_actor_event(((Foo*)g_field_runtime)->elem[i].unk0, 0xD, 0x82);
    }
    ((Foo*)g_field_runtime)->f400.flags &= 0xFFF9FFFF;
    g_field_interaction_active = 0;
}

void func_800BDA48(s32 unused, u8* params)
{
    /* Int arguments on purpose: the original passes the words without narrowing them to s16. */
    ((void (*)(s32, s32, s32, s32))field_set_fade_target)(*(s32*)(params + 0x4), *(s32*)(params + 0x8), *(s32*)(params + 0xC), *(u16*)(params + 0x0));
}

/**
 * @brief Tests an explicit or resolved actor position against command bounds.
 * @param arg0 Auxiliary command value used by the bounds test.
 * @param command Bounds-test command and result destination.
 * @param arg2 Auxiliary command value used by the bounds test.
 * @param arg3 Auxiliary command value used by the bounds test.
 */
void func_800BDA7C(u32 arg0, BoundsCommand* command, u32 arg2, u32 arg3)
{
    s32 position[3];
    u32 x = arg2;
    u32 z;
    u32 left;
    u32 top;
    u32 right = arg3;
    u32 bottom = arg0;
    s16 result;

    switch (command->mode)
    {
    case 0:
        left = command->left;
        top = command->top;
        x = command->x >> 8;
        z = command->z >> 8;
        right = left + command->width;
        bottom = top + command->height;
        break;
    case 1:
        x = command->x >> 8;
        z = command->z >> 8;
        left = command->left - command->width;
        top = command->top - command->height;
        right = command->left + command->width;
        bottom = command->top + command->height;
        break;
    case 2:
        if (command->x != 0xFF)
        {
            x = command->x;
        }
        else
        {
            x = *g_field_script;
        }
        field_get_actor_position(x, position);
        left = command->left;
        top = command->top;
        right = command->width;
        bottom = command->height;
        x = position[0] >> 8;
        z = position[2] >> 8;
        break;
    }
    result = 0;
    if (x >= left && x <= right && z >= top)
    {
        result = bottom >= z;
    }
    command->result = result;
}

void func_800BDBAC(s32 unused, FieldNibbleRecord* record)
{
    u8* base;

    base = func_800C1E40(0x13);
    if (base != NULL)
    {
        {
            SourceEntry* entry;
            entry = (SourceEntry*)(base + ((record->offset - 0x60) + (record->row * 0x30)));
            record->low = entry->packed & 0xF;
        }
        {
            SourceEntry* entry;
            entry = (SourceEntry*)(base + ((record->offset - 0x60) + (record->row * 0x30)));
            record->high = entry->packed >> 4;
        }
    }
}

void func_800BDC40(s32 arg0, UnkStruct800BDC40* arg1)
{
    FieldPos pos;
    s32 index;

    index = -1;
    if (arg1->unk14 != 0xFF)
    {
        index = arg1->unk14;
    }

    func_8005AF5C(arg1->unk10, index, &pos);

    arg1->unk4 = pos.x;
    arg1->unk8 = pos.y;
    arg1->unkC = pos.z;
}

/**
 * @brief Dispatch record setup, counter consumption, and related script operations.
 * @param arg0 Unused dispatcher context.
 * @param arg1 Command whose result halfword receives an index or error code.
 */
void func_800BDCA4(s32 arg0, Command* arg1)
{
    u8* buffer;

    u8** context;
    s32 remaining;
    s32 repeat_left;
    u16 opcode;
    u32 item_id;
    u8* record;
    s32 count;
    s32 stock;
    u8* frame;
    u8* inventory;
    u8* counts;
    u32 allocation_offset;

    opcode = arg1->opcode;
    switch (opcode)
    {
    case 0:
        buffer = g_field_runtime + 0x104;
        D_80123FC4 = buffer;
        func_800C1EC8(0, buffer, 0x60);
        return;
    case 1:
        record = field_find_free_inventory_record();
        if (record != 0)
        {
            context = &g_field_game_state;
            item_id = arg1->argC;
            if (item_id < 0x40U)
            {
                inventory = *context + item_id;
                count = inventory[0x25E0];
                if (count != 0)
                {
                    inventory[0x25E0] = (u8)(count - 1);
                    func_800BE888((struct FieldItemRecord*)record, arg1->arg4, arg1->arg8, arg1->argC);
                    allocation_offset = (u32)record - 0xCE0;
                    arg1->result = (u16)((allocation_offset - (u32)*context) >> 12);
                    return;
                }
                else
                {
                    arg1->result = 0xFD;
                    return;
                }
            }
            else
            {
                arg1->result = 0xFE;
                return;
            }
        }
        arg1->result = 0xFA;
        return;
    case 2:
        record = g_field_game_state + ((arg1->arg4 << 6) + 0xCE0);
        if (*record != 0)
        {
            counts = g_field_game_state + arg1->arg8;
            stock = counts[0x25E0];
            if (stock != 0)
            {
                counts[0x25E0] = (u8)(stock - 1);
                func_800BEC44((struct FieldItemRecord*)record, arg1->arg8);
                arg1->result = (u16)arg1->arg4;
                return;
            }
            arg1->result = 0xFD;
            return;
        }
        arg1->result = 0xFB;
        return;
    case 3:
        if (arg1->arg8 != 0)
        {
            do
            {
                func_800BF9A0(arg1->arg4);
                remaining = arg1->arg8 - 1;
                arg1->arg8 = remaining;
            } while (remaining != 0);
            return;
        }
        break;
    case 4:
        if (arg1->arg8 != 0)
        {
            do
            {
                func_800BF880(arg1->arg4);
                repeat_left = arg1->arg8 - 1;
                arg1->arg8 = repeat_left;
            } while (repeat_left != 0);
            return;
        }
        break;
    case 5:
        remaining = func_800BF9F0(arg1->arg4);
        frame = g_field_script + (*(s32*)(g_field_script + 4) * 0xC);
        *(s32*)(frame + 0xC) = (s32)((*(s32*)(frame + 0xC) & ~1) | (remaining & 1));
        return;
    case 6:
        func_800BD520(0, 0x7100, func_800BF68C(arg1->arg4, arg1->arg8, arg1->argC));
        return;
    case 7:
    default:
        arg1->result = 0xFF;
        return;
    }
    return;
}

/**
 * @brief Dispatch a field record query and copy the selected handler result back.
 * @param unused Unused dispatcher argument.
 * @param request Record containing the actor ID, handler index, and result-mode flag.
 */
void func_800BDF00(s32 unused, UnkStruct800BDF00* request)
{
    LocalRecord records[17];
    s32 actor_id;
    u16 selected_id;
    u8* source_record;

    actor_id = request->unk0;
    selected_id = actor_id;
    if (actor_id == 0xFF)
    {
        selected_id = *g_field_script;
        source_record = func_800B2A9C(selected_id);
    }
    else
    {
        source_record = func_800B2A9C(selected_id);
    }
    D_800F0E54[request->unk2 & 0x7FFF](&records[0].status, source_record, request);

    if (records[0].status != 0)
    {
        func_800C1F28((u32*)&records[0].status);
        if (request->unk2 & 0x8000)
        {
            request->unk0 = records[records[0].status - 1].unk4;
        }
        else
        {
            request->unk0 = records[0].unk4;
        }
    }
    else
    {
        request->unk0 = 0xFFFF;
    }
}

/**
 * @brief Collect active field records selected by a flag mask and their distances from a reference object.
 * @param output Output list whose first word is the number of collected records.
 * @param source Supplies the reference object identifier used for distance measurements.
 * @param filter Supplies the flag mask used to select field records.
 */
void func_800BDFD4(FieldDistanceList* output, FieldDistanceSource* source, FieldRecordFilter* filter)
{
    FieldPosition reference_position;
    FieldPosition record_position;
    s32 i;
    s32 offset;

    field_get_actor_position(source->reference_id, (s32*)&reference_position);
    i = 0;
    output->count = 0;
    do
    {
        offset = i * FIELD_STATE_RECORD_SIZE;
        if ((FIELD_STATE_RECORD(offset)->flags & filter->flag_mask) && FIELD_STATE_RECORD(offset)->state->active != 0)
        {
            field_get_actor_position(FIELD_STATE_RECORD(offset)->object_id, (s32*)&record_position);
            output->entries[output->count].object_id = FIELD_STATE_RECORD(offset)->object_id;
            output->entries[output->count].distance = func_800C1FBC(&record_position, &reference_position);
            output->count += 1;
        }
        i++;
    } while (i < FIELD_STATE_RECORD_COUNT);
}

/**
 * @brief Apply a camera command using explicit, actor-relative, or saved coordinates.
 * @param arg0 Unused script dispatcher argument.
 * @param arg1 Camera command and parameters.
 */
void func_800BE0E0(s32 arg0, CameraCommand* arg1)
{
    s32 position[3];
    Bounds* bounds;
    s32* destination;
    State* state;
    s32 offset_x;
    s32 offset_y;
    s32 view_y;
    s32 half_y;
    s32 view_x;
    s32 limit_x;
    s32 limit_y;
    u16 mode;
    s32 actor;

    mode = arg1->unk2;
    switch (mode)
    { /* irregular */
    case 0:
        view_x = (s32)-g_field_scene_state->unk4 >> 8;
        ((State*)g_field_runtime)->unk424 = view_x;
        g_field_scripted_scroll_target_x = view_x;
        view_y = (s32)-g_field_scene_state->unkC >> 9;
        ((State*)g_field_runtime)->unk428 = view_y;
        g_field_scripted_scroll_target_z = view_y;
        if ((view_x | view_y) == 0)
        {
            g_field_scripted_scroll_target_z = 1;
        }
        g_field_scripted_scroll_frames = 0;
        return;
    case 1:
        g_field_scripted_scroll_target_x = arg1->unk4;
        g_field_scripted_scroll_target_z = arg1->unk8;
        g_field_scripted_scroll_frames = arg1->unkC;
        return;
    case 2:
        bounds = (Bounds*)0x801ED400;
        if (arg1->unk0 == 0xFF)
        {
            actor = *g_field_script;
        }
        else
        {
            actor = arg1->unk0;
        }
        field_get_actor_position(actor, position);
        offset_x = (position[0] >> 8) - 0xA0;
        destination = &g_field_scripted_scroll_target_x;
        if (offset_x > 0)
        {
            limit_x = bounds->width - 0x140;
            if (offset_x < limit_x)
            {
                limit_x = offset_x;
            }
            *destination = limit_x;
        }
        else
        {
            *destination = 0;
        }
        offset_y = ((s32)(position[2] - position[1]) >> 8) - 0xE0;
        destination = &g_field_scripted_scroll_target_z;
        if (offset_y > 0)
        {
            limit_y = (s16)bounds->height - 0x1C0;
            if (offset_y < limit_y)
            {
                limit_y = offset_y;
            }
            *destination = limit_y;
        }
        else
        {
            *destination = 0;
        }
        half_y = g_field_scripted_scroll_target_z / 2;
        g_field_scripted_scroll_target_z = half_y;
        if ((g_field_scripted_scroll_target_x | half_y) == 0)
        {
            g_field_scripted_scroll_target_z = 1;
        }
        g_field_scripted_scroll_frames = arg1->unkC;
        return;
    case 3:
        state = (State*)g_field_runtime;
        g_field_scripted_scroll_frames = arg1->unkC;
        g_field_scripted_scroll_target_x = state->unk424;
        g_field_scripted_scroll_target_z = state->unk428;
        return;
    }
}

void func_800BE2F0(s32 arg0, UnkStruct800BE2F0* arg1)
{
    arg1->unk0 = func_800C33E4(arg1->unk4, arg1->unk8, &arg1->unk10);
}

void func_800BE324(s32 arg0, UnkStruct800BE324* arg1)
{
    s32 actor;

    if (arg1->unk0 == 0xFF)
    {
        actor = *g_field_script;
    }
    else
    {
        actor = arg1->unk0;
    }

    field_set_actor_position(actor, arg1->unk4, -arg1->unk8, arg1->unkC);
}

/**
 * @brief Dispatch a command record, decoding its priority flag.
 *
 * Resolves the record's target index (@c g_field_script[0] when @c unk0 is the 0xFF
 * sentinel), decodes @c unk2: when bit 7 is set the priority flag is 1 and the
 * value is the low 7 bits, otherwise the flag is 0 and the value is @c unk2 in
 * full. Forwards the record's three payload words plus flag, value, and target
 * index to field_set_actor_render_state.
 *
 * @param arg0 Unused.
 * @param arg1 CameraCommand record.
 */
void func_800BE37C(s32 arg0, SomeStruct* arg1)
{
    s32 actor;
    u16 packed;
    s32 index;
    s32 flag;

    if (arg1->unk0 == 0xFF)
    {
        actor = *g_field_script;
    }
    else
    {
        actor = arg1->unk0;
    }
    packed = arg1->unk2;
    flag = 1;
    if (packed & 0x80)
    {
        index = packed & 0x7F;
    }
    else
    {
        flag = 0;
        index = arg1->unk2;
    }
    field_set_actor_render_state(arg1->unk4, arg1->unk8, arg1->unkC, flag, index, actor);
}

/**
 * @brief Dispatch a field-script command when its resolved actor is active.
 * @param arg0 Unused leading argument preserved from the original call shape.
 * @param arg1 CameraCommand payload containing the actor id, coordinates, and three optional overrides.
 */
void func_800BE404(s32 arg0, ArgB800BE404* arg1)
{
    s32 id;
    s32 handle;
    s32 override_0;
    s32 override_1;
    s32 override_2;

    if (arg1->unk0 == 0xFF)
    {
        id = *g_field_script;
    }
    else
    {
        id = arg1->unk0;
    }
    if (id != 0 && ((func_800C1B60(id)->unk90 >> 0x1E) & 1))
    {
        handle = (s32)func_800B2B08();
        if (handle != 0)
        {
            field_set_actor_position(id, arg1->unk4, arg1->unk8, arg1->unkC);
            override_0 = -1;
            field_set_actor_group(id, ((StructB78B800BE404*)g_field_runtime)->unk403);
            if (arg1->unk10 != 0xFF)
            {
                override_0 = arg1->unk10;
            }
            override_1 = -1;
            if (arg1->unk14 != 0xFF)
            {
                override_1 = arg1->unk14;
            }
            override_2 = -1;
            if (arg1->unk18 != 0xFF)
            {
                override_2 = arg1->unk18;
            }
            func_800C1EC8(0, (void*)handle, 0x68);
            field_init_monster_record(id, (struct FieldStatusRecord*)handle, (struct FieldStatusState*)field_find_object_state(id));
            field_revive_actor(id, override_0, override_1, override_2);
        }
    }
}

/**
 * @brief Resolves a field position and stores it in a command record.
 *
 * @param arg0 Unused command argument.
 * @param command Destination record; selector 0xFF uses the script owner.
 */
void func_800BE550(s32 arg0, FieldPositionCommand* command)
{
    s32 index;
    FieldPosition position;

    if (command->unk0 == 0xFF)
    {
        index = *g_field_script;
    }
    else
    {
        index = command->unk0;
    }

    field_get_actor_position(index, (s32*)&position);
    command->unk4 = position.x;
    command->unk8 = -position.y;
    command->unkC = position.z;
}
