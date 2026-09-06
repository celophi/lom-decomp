#include "common.h"

typedef struct StateB3160
{
    u8 pad0[0x48];
    u16 unk48;
} StateB3160;

typedef struct RecordB4D1C
{
    u8 pad0[4];
    u8 unk4;
    u8 pad5[0xB];
    StateB3160 *state;
} RecordB4D1C;

u32 func_800B4CE4(RecordB4D1C *arg0, s32 arg1);
void field_clear_record_state(RecordB4D1C *record, u32 index);
void func_800B2D64(RecordB4D1C *arg0, s32 arg1, s32 arg2, s32 arg3);

/**
 * @brief Clear selected record state and apply active field record actions.
 * @param arg0 Record whose active entries are processed.
 */
void func_800B4D1C(RecordB4D1C *arg0)
{
    s32 i;
    s32 j;

    if (func_800B4CE4(arg0, 5) != 0)
    {
        field_clear_record_state(arg0, 0xFF);
    }

    i = 0x60;
    do
    {
        if (func_800B4CE4(arg0, i) != 0)
        {
            field_clear_record_state(arg0, i - 0x60);
        }
        i++;
    } while (i < 0x6C);

    if (func_800B4CE4(arg0, 6) != 0)
    {
        if (arg0->unk4 == 0)
        {
            arg0->state->unk48 = 0xFF;
        }
    }

    j = 0x70;
    do
    {
        if (func_800B4CE4(arg0, j) != 0)
        {
            func_800B2D64(arg0, j - 0x70, 0xA, 0);
        }
        j++;
    } while (j < 0x80);
}
