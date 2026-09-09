#include "common.h"

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
extern void func_800C1EC8(s32, u8 *, s32);
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
    /* 0x0 */ u16 unk0;
    /* 0x2 */ u16 unk2;
} UnkStruct800BDF00;

/** @brief Partial LocalRecord layout used by func_800BDF00. */
typedef struct
{
    /* 0x0 */ s32 status;
    /* 0x4 */ u16 unk4;
    /* 0x6 */ u16 pad6;
} LocalRecord;

extern u8 *g_field_script;

extern u8 *func_800B2A9C(s32 value);
extern void func_800C1F28(u32 *arg0);
extern void (*D_800F0E54[])(s32 *arg0, void *arg1, void *arg2);

/**
 * @brief Dispatch a field record and copy the selected handler result back.
 * @param arg0 Unused.
 * @param arg1 Record containing the actor ID, handler index and result-mode flag.
 * @note WIP: separate ID and handler-argument copies remain coalesced by GCC.
 */
void func_800BDF00(s32 arg0, UnkStruct800BDF00 *arg1)
{
    LocalRecord records[17];
    u16 var_a0;
    u8 *var_a1;

    var_a0 = arg1->unk0;
    if (arg1->unk0 == 0xFF)
    {
        var_a0 = *g_field_script;
    }

    var_a1 = func_800B2A9C(var_a0);
    D_800F0E54[arg1->unk2 & 0x7FFF](&records[0].status, var_a1, arg1);

    if (records[0].status != 0)
    {
        func_800C1F28((u32 *)&records[0].status);
        if (arg1->unk2 & 0x8000)
        {
            arg1->unk0 = records[records[0].status - 1].unk4;
            return;
        }
        arg1->unk0 = records[0].unk4;
        return;
    }
    arg1->unk0 = 0xFFFF;
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
