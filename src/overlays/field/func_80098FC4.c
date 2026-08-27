#include "common.h"

typedef struct FieldObject80098FC4
{
    u8 pad0[0x3A];
    u8 stateIndex;
} FieldObject80098FC4;

typedef struct FieldState80098FC4
{
    u8 pad0[0x14];
    s32 value;
    u8 pad18[2];
    u16 entries[2];
    u8 pad1E[0x23C - 0x1E];
} FieldState80098FC4;

extern FieldState80098FC4 D_80105AE0[];
extern void func_800B22F0(s32 value, u16 entry, FieldState80098FC4 *states);

/**
 * @brief Forwards an object-state value and selected halfword to the field
 *        state handler.
 *
 * @param object Field object containing the state-array index.
 * @param entryIndex Index of the halfword entry to forward.
 */
void func_80098FC4(FieldObject80098FC4 *object, s32 entryIndex)
{
    FieldState80098FC4 *state;

    state = &D_80105AE0[object->stateIndex];
    func_800B22F0(state->value, state->entries[entryIndex], D_80105AE0);
}
