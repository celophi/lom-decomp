#include "common.h"

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
