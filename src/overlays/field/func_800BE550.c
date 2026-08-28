#include "common.h"

/** Field command record containing a selector and resolved position. */
typedef struct
{
    u16 unk0;
    u16 pad2;
    s32 unk4;
    s32 unk8;
    s32 unkC;
} FieldPositionCommand;

/** Three-component field position returned by func_80087F44. */
typedef struct
{
    s32 x;
    s32 y;
    s32 z;
} FieldPosition;

extern u8 *D_80123FB8;

void func_80087F44(s32 index, FieldPosition *position);

/**
 * @brief Resolves a field position and stores it in a command record.
 *
 * @param arg0 Unused command argument.
 * @param command Destination record; selector 0xFF uses the active selector.
 */
void func_800BE550(s32 arg0, FieldPositionCommand *command)
{
    s32 index;
    FieldPosition position;

    if (command->unk0 == 0xFF)
    {
        index = *D_80123FB8;
    }
    else
    {
        index = command->unk0;
    }

    func_80087F44(index, &position);
    command->unk4 = position.x;
    command->unk8 = -position.y;
    command->unkC = position.z;
}
