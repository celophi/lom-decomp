#include "common.h"

typedef struct
{
    u8 pad0[4];
    u8 object_id;
    u8 pad5;
    u16 flags;
} FieldRecordB5C54;

typedef struct
{
    s32 value;
} FieldEntryB5C54;

typedef struct
{
    u8 pad0[0x1C];
    FieldEntryB5C54 *entry;
    void *secondary;
    FieldRecordB5C54 *record;
} FieldStateB5C54;

extern FieldStateB5C54 *D_80123FB0;
void func_800B2B54(FieldRecordB5C54 *record, void *secondary, s32 arg2, s32 arg3, s32 arg4, s32 arg5);
void field_clear_record_state(FieldRecordB5C54 *record, u32 index);
void func_8008B500(s32 object_id, s32 value);

/**
 * @brief Process the active field record's state flags.
 * @return State result selected by the active record flags.
 */
s32 func_800B5C54(void)
{
    u16 flags;

    flags = D_80123FB0->record->flags;
    if (flags & 0x8000)
    {
        return 1;
    }
    if (flags & 0x4000)
    {
        return 2;
    }
    if (flags & 0x2000)
    {
        if ((D_80123FB0->entry->value & 0xF) == 0)
        {
            func_800B2B54(D_80123FB0->record, D_80123FB0->secondary, 3, 4, 0x100, 0x12C);
            field_clear_record_state(D_80123FB0->record, 6);
            return 2;
        }
    }
    if (D_80123FB0->record->flags & 0x1000)
    {
        if ((u32)((D_80123FB0->entry->value & 0xF) - 2) < 2)
        {
            func_8008B500(D_80123FB0->record->object_id, 0x90);
            return 2;
        }
    }
    return 0;
}
