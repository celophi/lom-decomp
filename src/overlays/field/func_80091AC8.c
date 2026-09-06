#include "common.h"

typedef struct
{
    u8 state;
    u8 pad1;
    u16 input;
    u8 pad4[0xAE - 4];
} FieldInputRecord91AC8;

typedef struct
{
    u8 pad0[0x24];
    u8 unk24;
    u8 pad25[0x33 - 0x25];
    u8 unk33;
    u8 pad34[0x3B - 0x34];
    u8 resource_index;
} FieldActor91AC8;

typedef struct
{
    u8 pad0[0x10];
    u32 flags;
} FieldResourceEntry91AC8;

extern FieldResourceEntry91AC8 g_field_resource_entries[];

#define FIELD_INPUT_RECORDS ((FieldInputRecord91AC8 *)0x801ED600)

/**
 * @brief Update an actor's input state from its field input record.
 * @param actor Actor state to update.
 * @param index Input-record index to read.
 */
void func_80091AC8(FieldActor91AC8 *actor, s32 index)
{
    u16 raw;
    s32 input;
    FieldInputRecord91AC8 *records;

    records = FIELD_INPUT_RECORDS;
    if (!(g_field_resource_entries[actor->resource_index].flags & 1))
    {
        input = 0;
        if (records[index].state < 0xFE)
        {
            raw = records[index].input;
            input = ((raw << 8) & 0xFF00) | (raw >> 8);
        }

        input = ((u32)(input & 0x40) >> 1) | ((input & 0x20) * 2) | ((u32)(input & 0x80) >> 3) | ((input & 0x10) * 8) | (input & ~0xF0);
        if (input & 0x44)
        {
            if (actor->unk33 == 0)
            {
                actor->unk33 = 1;
                actor->unk24 = 0;
            }
        }
        else if (actor->unk33 != 0)
        {
            actor->unk33 = 0;
            actor->unk24 = 0;
        }
    }
}
