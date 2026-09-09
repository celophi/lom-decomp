#include "common.h"

#define FIELD_POSITION_HISTORY_LENGTH 48
#define FIELD_FIXED_POINT_SHIFT 8
#define FIELD_FIXED_POINT_ROUND_BIAS 0xFF

typedef struct
{
    s16 x;
    s16 y;
} FieldPosition16;

typedef struct
{
    s32 x;
    s32 unk4;
    s32 y;
    u8 padC[0x21 - 0xC];
    u8 status;
    u8 pad22[0x3A - 0x22];
    u8 actor_index;
} FieldPositionRecord;

typedef struct
{
    u8 pad0[0x128];
    FieldPosition16 position;
    u8 pad12C[0x23C - 0x12C];
} FieldActorState;

typedef struct
{
    FieldPosition16 entries[FIELD_POSITION_HISTORY_LENGTH];
    u8 pad[0x23C - FIELD_POSITION_HISTORY_LENGTH * sizeof(FieldPosition16)];
} FieldPositionHistory;

extern FieldActorState D_80105AE0[];
extern FieldPositionHistory D_80105B4C[];
extern u8 D_8010CFE0[];

/**
 * @brief Update an actor's position history when its rounded position changes.
 * @param record Actor position and status record to sample.
 */
void func_8008D174(FieldPositionRecord* record)
{
    s32 x;
    s32 y;
    s32 index;
    s32 value;
    FieldPosition16* history;
    u8* status_history;

    x = record->x;
    if (x < 0)
    {
        x += FIELD_FIXED_POINT_ROUND_BIAS;
    }
    if ((x >> FIELD_FIXED_POINT_SHIFT) == D_80105AE0[record->actor_index].position.x)
    {
        y = record->y;
        if (y < 0)
        {
            y += FIELD_FIXED_POINT_ROUND_BIAS;
        }
        if ((y >> FIELD_FIXED_POINT_SHIFT) == D_80105AE0[record->actor_index].position.y)
        {
            return;
        }
    }

    index = 0;
    history = D_80105B4C[record->actor_index].entries;
    do
    {
        index++;
        *(s32*)history = *(s32*)(history + 1);
        history++;
    } while (index < FIELD_POSITION_HISTORY_LENGTH - 1);

    value = record->x;
    if (value < 0)
    {
        value += FIELD_FIXED_POINT_ROUND_BIAS;
    }
    history->x = (s16)(value >> FIELD_FIXED_POINT_SHIFT);

    value = record->y;
    if (value < 0)
    {
        value += FIELD_FIXED_POINT_ROUND_BIAS;
    }
    history->y = (s16)(value >> FIELD_FIXED_POINT_SHIFT);

    if (record->actor_index == 0)
    {
        status_history = D_8010CFE0;
        index = 0;
        do
        {
            index++;
            status_history[0] = status_history[1];
            status_history++;
        } while (index < FIELD_POSITION_HISTORY_LENGTH - 1);
        *status_history = record->status;
    }
}
