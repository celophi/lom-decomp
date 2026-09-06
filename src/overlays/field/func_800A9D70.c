#include "common.h"

typedef struct
{
    u8 id;
    u8 pad1;
    u16 flags;
    u8 pad4[0x2C - 4];
    s16 state_a;
    s16 state_b;
    u8 pad30[0xAE - 0x30];
} FieldResourceRecord;

#define FIELD_RESOURCE_RECORDS ((FieldResourceRecord *)0x801ED600)

/**
 * @brief Build the status flags for one field resource record.
 * @param index Record index to inspect.
 * @return Encoded status flags, or zero when the record is inactive.
 */
s32 func_800A9D70(s32 index)
{
    FieldResourceRecord *base;
    FieldResourceRecord *record;
    u16 flags;
    s32 value;
    s16 state;
    s32 offset;

    base = FIELD_RESOURCE_RECORDS;
    record = &base[index];
    if (record->id >= 0xFE)
    {
        return 0;
    }

    flags = record->flags;
    value = (flags >> 8) | ((flags & 0xFF) << 8);
    value = ((u32)(value & 0x40) >> 1) | ((value & 0x20) << 1) | ((u32)(value & 0x80) >> 3) | ((value & 0x10) << 3) | (value & 0xFF0F);

    if (record->id != 0)
    {
        state = record->state_a;
        if (state < -1)
        {
            value |= 0x8000;
        }
        else if (state >= 2)
        {
            value |= 0x2000;
        }

        offset = index * sizeof(*base);
        state = ((FieldResourceRecord *)((u8 *)base + offset))->state_b;
        if (state < -1)
        {
            value |= 0x1000;
        }
        else if (state >= 2)
        {
            value |= 0x4000;
        }
    }

    return value;
}
