#include "common.h"

/**
 * @brief Minimal command state used by func_800B286C.
 */
typedef struct FieldCommandState
{
    u8 unk0[4];
    u8 unk4;
    u8 unk5;
    u16 unk6;
} FieldCommandState;

FieldCommandState *func_800C1B60(void);

/**
 * @brief Claims an enabled command slot when it is currently unassigned.
 *
 * @param arg0 Unused command context value.
 * @param arg1 Bit index and value stored in the claimed slot.
 * @param arg2 Secondary value stored in the claimed slot.
 * @return The low byte of @p arg1 on success, otherwise -1.
 */
s32 func_800B286C(s32 arg0, u8 arg1, s8 arg2)
{
    s32 index;
    FieldCommandState *state;

    state = func_800C1B60();
    index = arg1 & 0xFF;
    if (((s32)state->unk6 >> index) & 1)
    {
        if (state->unk4 == 0xFF)
        {
            state->unk4 = arg1;
            state->unk5 = arg2;
            return index;
        }
    }

    return -1;
}
