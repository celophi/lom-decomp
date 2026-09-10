#include "common.h"

void func_800C1EC8(s32, void *, s32);

typedef void (*FieldCommandHandler)(s32 value, u8 *params);

extern FieldCommandHandler D_800F0E10[];
extern void akao_set_song_params(s32 command, s32 arg1, s32 arg2, s32 arg3);

/**
 * @brief Dispatches a small field command or forwards it to the audio system.
 *
 * @param value Command value or function-table index.
 * @param params Parameter block forwarded to table-dispatched commands.
 * @note 100% match. The second argument is part of the original handler ABI;
 *       preserving it also reproduces the target index register lifetime.
 */
void func_800BD6F4(s32 value, u8 *params)
{
    if (value < 0x11)
    {
        D_800F0E10[value](value, params);
        return;
    }
    akao_set_song_params(0x8001, 3, value, 0);
}

/**
 * @brief Thin stack-frame wrapper forwarding an offset pointer into func_800C1EC8.
 * @param arg0 Unused.
 * @param arg1 Base value; func_800C1EC8 is called with arg1 + 4.
 */
void func_800BD750(s32 arg0, s32 arg1)
{
    func_800C1EC8(0, (void *)(arg1 + 4), 0x20);
}

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
extern u8 *g_field_script;
extern Actor *func_80087F0C(s32);
/**
 * @brief Dispatch eight script operations that read or update actor fields.
 * @param unused Unused incoming argument.
 * @param request Actor selector, operation code, and input/output values.
 */
void func_800BD778(s32 unused, Request *request)
{
    s32 index;
    u32 divisor;
    Actor *actor;
    if (request->index == 0xFF)
    {
        index = *g_field_script;
    }
    else
    {
        index = request->index;
    }
    actor = func_80087F0C(index);
    if (actor != (Actor *)-1)
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

/** @brief One 0x94-byte element in the D_80122B78 table; unk0 is its id. */
typedef struct Elem
{
    u8 unk0;    /* 0x00 */
    u8 pad[0x93];
} Elem;

/** @brief D_80122B78 table: count at 0x400 (u16/u32 union), elements at 0x430. */
typedef struct Foo
{
    u8 pad0[0x400];
    union
    {
        u16 count;  /* 0x400 as lhu */
        u32 flags;  /* 0x400 as lw */
    } f400;
    u8 pad1[0x430 - 0x404];
    Elem elem[1];   /* 0x430 */
} Foo;

extern u8 *D_80122B78;
extern s32 D_8010AE78;
void func_800B177C(void);
void func_800B286C(u8 arg0, s32 arg1, s32 arg2);

/**
 * @brief Re-issue every active table element and clear the batch-dirty flags.
 *
 * After func_800B177C, walks the @c count live elements of @c D_80122B78,
 * dispatching func_800B286C for each element's id, then clears the 0x60000 bits
 * of the flag word at 0x400 and resets @c D_8010AE78.
 *
 * @see decomp.me (100%) TODO
 */
void func_800BD99C(void)
{
    s32 i;

    func_800B177C();
    do
    {
        i = 0;
    } while (0);
    if (((Foo *)D_80122B78)->f400.count != 0)
    {
        do
        {
            func_800B286C(((Foo *)D_80122B78)->elem[i].unk0, 0xD, 0x82);
            i += 1;
        } while (i < (s32)((Foo *)D_80122B78)->f400.count);
    }
    ((Foo *)D_80122B78)->f400.flags &= 0xFFF9FFFF;
    D_8010AE78 = 0;
}

void func_800BDA48(s32 unused, u8 *params)
{
    field_set_fade_target(*(s32 *)(params + 0x4), *(s32 *)(params + 0x8), *(s32 *)(params + 0xC), *(u16 *)(params + 0x0));
}

/** @brief Command payload for a two-dimensional bounds test. */
typedef struct
{
    u16 result, mode;
    u32 x, unused8, z;
    u32 left, top, width, height;
} BoundsCommand;
extern u8 *g_field_script;
s32 func_80087F44(s32, s32 *);
/**
 * @brief Tests an explicit or actor position against command bounds.
 * @note Initial nonmatching C. Modes outside 0-2 leave target registers unset;
 *       the corresponding C locals deliberately have no invented defaults.
 */
void func_800BDA7C(u32 arg0, BoundsCommand *command, u32 arg2, u32 arg3)
{
    s32 position[3];
    u32 x = arg2;
    u32 z;
    u32 left;
    u32 top;
    u32 right = arg3;
    u32 bottom = arg0;
    u32 actor;
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
        actor = command->x;
        if (actor == 0xFF)
        {
            actor = *g_field_script;
        }
        func_80087F44(actor, position);
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


typedef struct FieldNibbleRecord {
    u16 offset;
    u16 row;
    s32 low;
    s32 high;
} FieldNibbleRecord;

typedef struct SourceEntry {
    u8 pad[4];
    u8 packed;
} SourceEntry;

extern u8 *func_800C1E40(s32 arg0);

void func_800BDBAC(s32 unused, FieldNibbleRecord *record)
{
    u8 *base;

    base = func_800C1E40(0x13);
    if (base != NULL) {
        {
            SourceEntry *entry;
            entry = (SourceEntry *)(base + ((record->row * 0x30) +
                                           (*(volatile u16 *)&record->offset - 0x60)));
            record->low = entry->packed & 0xF;
        }
        {
            SourceEntry *entry;
            entry = (SourceEntry *)(base + ((record->row * 0x30) +
                                           (*(volatile u16 *)&record->offset - 0x60)));
            record->high = entry->packed >> 4;
        }
    }
}


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

extern void func_8005AF5C(s32 obj_index, s32 part_index, FieldPos *out);

void func_800BDC40(s32 arg0, UnkStruct800BDC40 *arg1)
{
    FieldPos pos;
    s32 var_a1;

    var_a1 = -1;
    if (arg1->unk14 != 0xFF)
    {
        var_a1 = arg1->unk14;
    }

    func_8005AF5C(arg1->unk10, var_a1, &pos);

    arg1->unk4 = pos.x;
    arg1->unk8 = pos.y;
    arg1->unkC = pos.z;
}

/** @brief Script command result, opcode, and three word-sized arguments. */
typedef struct Command
{
    u16 result, opcode;
    s32 arg4, arg8;
    u32 argC;
} Command;
extern u8 *func_800A9060(void);
extern void func_800BD520(s32, s32, s32);
extern void func_800BE888(u8 *, s32, s32, u32);
extern void func_800BEC44(u8 *, s32);
extern s32 func_800BF68C(s32, s32, u32);
extern void func_800BF880(s32);
extern void func_800BF9A0(s32);
extern s32 func_800BF9F0(s32);
extern void func_800C1EC8(s32, void *, s32);
extern u8 *D_80122B74, *D_80122B78, *D_80123FC4, *g_field_script;
/**
 * @brief Dispatch record setup, counter consumption, and related script operations.
 * @param arg0 Unused dispatcher context.
 * @param arg1 Command whose result halfword receives an index or error code.
 */
void func_800BDCA4(s32 arg0, Command *arg1)
{
    u8 *temp_a1;

    u8 **context;
    s32 temp_v0_4;
    s32 temp_v0_5;
    u16 temp_v1;
    u32 temp_v1_2;
    u32 var_v0;
    u8 *temp_s1;
    s32 temp_v0_2;
    s32 temp_v0_3;
    u8 *temp_a1_2;
    u8 *temp_v1_3;
    u8 *temp_v1_4;

    temp_v1 = arg1->opcode;
    switch (temp_v1)
    {
    case 0:
        temp_a1 = D_80122B78 + 0x104;
        D_80123FC4 = temp_a1;
        func_800C1EC8(0, temp_a1, 0x60);
        return;
    case 1:
        temp_s1 = func_800A9060();
        var_v0 = 0xFA;
        if (temp_s1 != 0)
        {
            context = &D_80122B74;
            temp_v1_2 = arg1->argC;
            if (temp_v1_2 < 0x40U)
            {
                temp_v1_3 = *context + temp_v1_2;
                temp_v0_2 = temp_v1_3[0x25E0];
                if (temp_v0_2 != 0)
                {
                    temp_v1_3[0x25E0] = (u8)(temp_v0_2 - 1);
                    func_800BE888(temp_s1, arg1->arg4, arg1->arg8, arg1->argC);
                    var_v0 = (u32)temp_s1 - 0xCE0;
                    var_v0 = (var_v0 - (u32)*context) >> 12;
                }
                else
                {
                    var_v0 = 0xFD;
                }
            }
            else
            {
                var_v0 = 0xFE;
            }
        }
        goto block_22;
    case 2:
        temp_s1 = D_80122B74 + ((arg1->arg4 << 6) + 0xCE0);
        var_v0 = 0xFB;
        if (*temp_s1 != 0)
        {
            temp_v1_4 = D_80122B74 + arg1->arg8;
            temp_v0_3 = temp_v1_4[0x25E0];
            if (temp_v0_3 != 0)
            {
                temp_v1_4[0x25E0] = (u8)(temp_v0_3 - 1);
                func_800BEC44(temp_s1, arg1->arg8);
                arg1->result = (u16)arg1->arg4;
                return;
            }
            var_v0 = 0xFD;
            goto block_22;
        }
        goto block_22;
    case 3:
        if (arg1->arg8 != 0)
        {
            do
            {
                func_800BF9A0(arg1->arg4);
                temp_v0_4 = arg1->arg8 - 1;
                arg1->arg8 = temp_v0_4;
            } while (temp_v0_4 != 0);
            return;
        }
        break;
    case 4:
        if (arg1->arg8 != 0)
        {
            do
            {
                func_800BF880(arg1->arg4);
                temp_v0_5 = arg1->arg8 - 1;
                arg1->arg8 = temp_v0_5;
            } while (temp_v0_5 != 0);
            return;
        }
        break;
    case 5:
        temp_v0_4 = func_800BF9F0(arg1->arg4);
        temp_a1_2 = g_field_script + (*(s32 *)(g_field_script + 4) * 0xC);
        *(s32 *)(temp_a1_2 + 0xC) = (s32)((*(s32 *)(temp_a1_2 + 0xC) & ~1) | (temp_v0_4 & 1));
        return;
    case 6:
        func_800BD520(0, 0x7100, func_800BF68C(arg1->arg4, arg1->arg8, arg1->argC));
        return;
    case 7:
    default:
        var_v0 = 0xFF;
        goto block_22;
    }
    return;
block_22:
    arg1->result = (u16)var_v0;
}


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

extern u8 *g_field_script;

extern u8 *func_800B2A9C(s32 value);
extern void func_800C1F28(u32 *arg0);
extern void (*D_800F0E54[])(s32 *arg0, void *arg1, void *arg2);

/**
 * @brief Dispatch a field record query and copy the selected handler result back.
 * @param unused Unused dispatcher argument.
 * @param request Record containing the actor ID, handler index, and result-mode flag.
 */
void func_800BDF00(s32 unused, UnkStruct800BDF00 *request)
{
    LocalRecord records[17];
    s32 actor_id;
    u16 selected_id;
    u8 *source_record;

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
        func_800C1F28((u32 *)&records[0].status);
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


#define FIELD_STATE_RECORD_COUNT 11
#define FIELD_STATE_RECORD_SIZE 0x68
#define FIELD_STATE_RECORD(offset) ((FieldStateRecord *)(D_80123FB0 + (offset)))

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
    FieldRecordState *state;
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


s32 func_800C1FBC(FieldPosition *first, FieldPosition *second);

extern u8 *D_80123FB0;

/**
 * @brief Collect active field records selected by a flag mask and their distances from a reference object.
 * @param output Output list whose first word is the number of collected records.
 * @param source Supplies the reference object identifier used for distance measurements.
 * @param filter Supplies the flag mask used to select field records.
 */
void func_800BDFD4(FieldDistanceList *output, FieldDistanceSource *source, FieldRecordFilter *filter)
{
    FieldPosition reference_position;
    FieldPosition record_position;
    s32 i;
    s32 offset;

    func_80087F44(source->reference_id, (s32 *)&reference_position);
    i = 0;
    output->count = 0;
    do
    {
        offset = i * FIELD_STATE_RECORD_SIZE;
        if ((FIELD_STATE_RECORD(offset)->flags & filter->flag_mask) &&
            FIELD_STATE_RECORD(offset)->state->active != 0)
        {
            func_80087F44(FIELD_STATE_RECORD(offset)->object_id, (s32 *)&record_position);
            output->entries[output->count].object_id = FIELD_STATE_RECORD(offset)->object_id;
            output->entries[output->count].distance = func_800C1FBC(&record_position, &reference_position);
            output->count += 1;
        }
        i++;
    } while (i < FIELD_STATE_RECORD_COUNT);
}

extern u8 *D_80122B78;
extern u8 *g_field_script;
s32 func_80087F44(s32, s32 *);
s32 func_80087D8C(s32, s32, s32, s32);


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
extern s32 func_80087F44(s32, s32 *);
extern s32 D_8010AE74, D_8010CFD8, D_8010CFDC;
extern Camera *D_80122B70;

extern u8 *g_field_script;
/**
 * @brief Apply a camera command using explicit, actor-relative, or saved coordinates.
 * @param arg0 Unused script dispatcher argument.
 * @param arg1 Camera command and parameters.
 */
void func_800BE0E0(s32 arg0, CameraCommand *arg1)
{
    s32 position[3];
    Bounds *bounds;
    s32 *destination;
    State *state;
    s32 temp_a0;
    s32 temp_a0_2;
    s32 temp_v0;
    s32 temp_v0_2;
    s32 temp_v1_2;
    s32 var_v1;
    s32 var_v1_2;
    u16 temp_v1;
    s32 var_a0;

    temp_v1 = arg1->unk2;
    switch (temp_v1)
    { /* irregular */
    case 0:
        temp_v1_2 = (s32)-D_80122B70->unk4 >> 8;
        ((State *)D_80122B78)->unk424 = temp_v1_2;
        D_8010CFD8 = temp_v1_2;
        temp_v0 = (s32)-D_80122B70->unkC >> 9;
        ((State *)D_80122B78)->unk428 = temp_v0;
        D_8010CFDC = temp_v0;
        if ((temp_v1_2 | temp_v0) == 0)
        {
            D_8010CFDC = 1;
        }
        D_8010AE74 = 0;
        return;
    case 1:
        D_8010CFD8 = arg1->unk4;
        D_8010CFDC = arg1->unk8;
        goto block_26;
    case 2:
        bounds = (Bounds *)0x801ED400;
        if (arg1->unk0 == 0xFF)
        {
            var_a0 = *g_field_script;
        }
        else
        {
            var_a0 = arg1->unk0;
        }
        func_80087F44(var_a0, position);
        temp_a0 = (position[0] >> 8) - 0xA0;
        destination = &D_8010CFD8;
        if (temp_a0 > 0)
        {
            var_v1 = bounds->width - 0x140;
            if (temp_a0 < var_v1)
            {
                var_v1 = temp_a0;
            }
            *destination = var_v1;
        }
        else
        {
            *destination = 0;
        }
        temp_a0_2 = ((s32)(position[2] - position[1]) >> 8) - 0xE0;
        destination = &D_8010CFDC;
        if (temp_a0_2 > 0)
        {
            var_v1_2 = (s16)bounds->height - 0x1C0;
            if (temp_a0_2 < var_v1_2)
            {
                var_v1_2 = temp_a0_2;
            }
            *destination = var_v1_2;
        }
        else
        {
            *destination = 0;
        }
        temp_v0_2 = D_8010CFDC / 2;
        D_8010CFDC = temp_v0_2;
        if ((D_8010CFD8 | temp_v0_2) == 0)
        {
            D_8010CFDC = 1;
        }
    block_26:
        D_8010AE74 = arg1->unkC;
        return;
    case 3:
        state = (State *)D_80122B78;
        D_8010AE74 = arg1->unkC;
        D_8010CFD8 = state->unk424;
        D_8010CFDC = state->unk428;
        return;
    }
}


typedef struct
{
    s16 unk0;
    s16 unk2;
    s32 unk4;
    s32 unk8;
    s8 unkC[4];
    s32 unk10;
} UnkStruct800BE2F0;

void func_800BE2F0(s32 arg0, UnkStruct800BE2F0* arg1)
{
    arg1->unk0 = func_800C33E4(arg1->unk4, arg1->unk8, &arg1->unk10);
}

typedef struct
{
    /* 0x0 */ u16 unk0;
    /* 0x4 */ s32 unk4;
    /* 0x8 */ s32 unk8;
    /* 0xC */ s32 unkC;
} UnkStruct800BE324;

extern u8 *g_field_script;

extern s32 func_80087D8C(s32 arg0, s32 arg1, s32 arg2, s32 arg3);

void func_800BE324(s32 arg0, UnkStruct800BE324 *arg1)
{
    s32 var_a0;

    if (arg1->unk0 == 0xFF)
    {
        var_a0 = *g_field_script;
    }
    else
    {
        var_a0 = arg1->unk0;
    }

    func_80087D8C(var_a0, arg1->unk4, -arg1->unk8, arg1->unkC);
}


typedef struct {
    u16 unk0;
    u16 unk2;
    s32 unk4;
    s32 unk8;
    s32 unkC;
} SomeStruct;

extern u8 *g_field_script;
extern void func_8006B984(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5);

/**
 * @brief Dispatch a command record, decoding its priority flag.
 *
 * Resolves the record's target index (@c g_field_script[0] when @c unk0 is the 0xFF
 * sentinel), decodes @c unk2: when bit 7 is set the priority flag is 1 and the
 * value is the low 7 bits, otherwise the flag is 0 and the value is @c unk2 in
 * full. Forwards the record's three payload words plus flag, value, and target
 * index to func_8006B984.
 *
 * @param arg0 Unused.
 * @param arg1 CameraCommand record.
 * @see decomp.me (100%) TODO
 */
void func_800BE37C(s32 arg0, SomeStruct *arg1)
{
    s32 var_a0;
    u16 temp_v1;
    s32 var_v0;
    s32 var_a3;

    if (arg1->unk0 == 0xFF)
    {
        var_a0 = *g_field_script;
    }
    else
    {
        var_a0 = arg1->unk0;
    }
    temp_v1 = arg1->unk2;
    var_a3 = 1;
    if (temp_v1 & 0x80)
    {
        var_v0 = temp_v1 & 0x7F;
    }
    else
    {
        var_a3 = 0;
        var_v0 = arg1->unk2;
    }
    func_8006B984(arg1->unk4, arg1->unk8, arg1->unkC, var_a3, var_v0, var_a0);
}


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

/** @brief View of D_80122B78 exposing the byte consumed at offset 0x403. */
typedef struct
{
    u8 pad0[0x403];
    u8 unk403;
} StructB78B800BE404;

extern u8 *g_field_script;


void func_80087614(s32 arg0, s32 arg1);
s32 func_80087D8C(s32 arg0, s32 arg1, s32 arg2, s32 arg3);

void func_80089D44(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
void *func_800B2B08(void);
void func_800B3F1C(s32 arg0, s32 arg1, s32 arg2);
CmdB800BE404 *func_800C1B60(s32 arg0);
void func_800C1EC8(s32 arg0, void *arg1, s32 arg2);

/**
 * @brief Dispatch a field-script command when its resolved actor is active.
 * @param arg0 Unused leading argument preserved from the original call shape.
 * @param arg1 CameraCommand payload containing the actor id, coordinates, and three optional overrides.
 */
void func_800BE404(s32 arg0, ArgB800BE404 *arg1)
{
    s32 id;
    s32 handle;
    s32 s4;
    s32 s3;
    s32 s1;

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
        handle = (s32) func_800B2B08();
        if (handle != 0)
        {
            func_80087D8C(id, arg1->unk4, arg1->unk8, arg1->unkC);
            s4 = -1;
            func_80087614(id, ((StructB78B800BE404 *)D_80122B78)->unk403);
            if (arg1->unk10 != 0xFF)
            {
                s4 = arg1->unk10;
            }
            s3 = -1;
            if (arg1->unk14 != 0xFF)
            {
                s3 = arg1->unk14;
            }
            s1 = -1;
            if (arg1->unk18 != 0xFF)
            {
                s1 = arg1->unk18;
            }
            func_800C1EC8(0, (void *) handle, 0x68);
            func_800B3F1C(id, handle, (s32)func_80087F0C(id));
            func_80089D44(id, s4, s3, s1);
        }
    }
}

/** @brief Field command record containing a selector and resolved position. */
typedef struct
{
    u16 unk0;
    u16 pad2;
    s32 unk4;
    s32 unk8;
    s32 unkC;
} FieldPositionCommand;




/**
 * @brief Resolves a field position and stores it in a command record.
 *
 * @param arg0 Unused command argument.
 * @param command Destination record; selector 0xFF uses the script owner.
 */
void func_800BE550(s32 arg0, FieldPositionCommand *command)
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

    func_80087F44(index, (s32 *)&position);
    command->unk4 = position.x;
    command->unk8 = -position.y;
    command->unkC = position.z;
}
