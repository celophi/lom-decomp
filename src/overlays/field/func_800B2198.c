#include "common.h"

typedef struct
{
    s32 unk0;
    u8 pad4[8];
} FieldEventEntry;

typedef struct
{
    u8 unk0;
    u8 pad1[3];
    u8 unk4;
    u8 unk5;
    u8 pad6[0x28 - 6];
    s32 unk28;
    s32 unk2C;
    FieldEventEntry entries[8];
    s32 unk90;
} FieldRecordB2198;

typedef struct
{
    u32 x;
    u32 y;
    u32 z;
} FieldPositionB2198;

typedef struct
{
    u8 pad0[0x54];
    u32 x;
    u32 z;
} FieldBoundsB2198;

extern FieldBoundsB2198 *D_80122B78;

FieldRecordB2198 *func_800C1B98();
void func_800B28E0(s32 arg0, s32 arg1, s32 arg2);
s32 func_80087F44(s32 arg0, FieldPositionB2198 *out);
void field_script_run(void *script);

/**
 * @brief Update an active field record and dispatch its pending event or script.
 * @param arg0 Record id forwarded to the record lookup and position query.
 * @param arg1 Unused actor-table argument supplied by the caller.
 */
void func_800B2198(s32 arg0, void *arg1)
{
    FieldRecordB2198 *record;
    FieldPositionB2198 position;

    record = func_800C1B98();
    if ((record != NULL) && (record->unk90 < 0))
    {
        if (record->unk4 != 0xFF)
        {
            func_800B28E0(record->unk0, record->unk4, record->unk5);
            record->unk4 = 0xFF;
        }
        if (!(((u32)record->unk90 >> 30) & 1))
        {
            func_80087F44(arg0, &position);
            if ((position.x > D_80122B78->x) &&
                (position.z > D_80122B78->z) &&
                (position.x < D_80122B78->x + 0x140) &&
                (position.z < D_80122B78->z + 0x1C0))
            {
                func_800B28E0(record->unk0, 2, 0);
            }
            else
            {
                func_800B28E0(record->unk0, 3, 0);
            }
            func_800B28E0(record->unk0, 0xE, 0);
            if (record->entries[record->unk2C].unk0 == 0)
            {
                func_800B28E0(record->unk0, 8, 0);
                return;
            }
            field_script_run(&record->unk28);
        }
    }
}
