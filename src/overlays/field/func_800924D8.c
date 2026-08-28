#include "common.h"

typedef struct
{
    u8 pad0[0x2A];
    s16 unk2A;
    u8 pad2C[0x2E - 0x2C];
    u16 unk2E;
    u8 pad30[0x3A - 0x30];
    u8 unk3A;
} FieldRecord;

typedef struct
{
    u8 pad0[0x174];
    s32 unk174;
    u8 pad178[0x23C - 0x178];
} FieldState;

extern FieldState D_80105AE0[];

void func_800952DC(FieldRecord *record, s32 value);
void func_80096334(FieldRecord *record);

/**
 * @brief Initializes an inactive field record and clears two state flags.
 *
 * @param record Field record whose state entry is selected by byte 0x3A.
 */
void func_800924D8(FieldRecord *record)
{
    if (record->unk2E == 0)
    {
        record->unk2A = 0;
        func_800952DC(record, 1);
        D_80105AE0[record->unk3A].unk174 &= ~0x1800;
        func_80096334(record);
    }
}
