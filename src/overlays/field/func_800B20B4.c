#include "common.h"

typedef struct
{
    s32 active;
    u8 pad4[8];
} FieldEventEntryB20B4;

typedef struct
{
    u8 pad0[0x2C];
    s32 active_event;
    FieldEventEntryB20B4 events[8];
    u8 pad90[4];
} FieldRecordB20B4;

typedef struct
{
    u8 pad0[0xD70];
    FieldRecordB20B4 records[2];
} FieldStateB20B4;

extern FieldStateB20B4* D_80122B78;
extern void func_800B28E0(s32 arg0, s32 arg1, s32 arg2);
extern void field_script_run(void* script);

/**
 * @brief Advance the two field event records and re-arm their event flags.
 */
void func_800B20B4(void)
{
    s32 i;
    s32 event_index;
    s32 script_offset;
    u8* record;

    i = 0;
    do
    {
        script_offset = i * 0x94 + 0xD70;
        event_index = D_80122B78->records[i].active_event;
        if (D_80122B78->records[i].events[event_index].active != 0)
        {
            field_script_run((u8*)D_80122B78 + script_offset + 0x28);
        }
        record = (u8*)D_80122B78 + i * 0x94;
        func_800B28E0(i + 0x80, record[0xD74], record[0xD75]);
        record = (u8*)D_80122B78 + i * 0x94;
        record[0xD74] = 0xFF;
        func_800B28E0(0x80, 0xE, 0);
        record = (u8*)D_80122B78 + i * 0x94;
        record[0xD74] = 0xFF;
        i += 1;
    } while (i < 2);
}
