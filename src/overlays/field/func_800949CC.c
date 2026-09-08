#include "common.h"

typedef struct
{
    u8 pad0[0xC];
    s32 unkC;
    u8 pad10[0x18 - 0x10];
    s32 unk18;
} FieldTrackEntry;

typedef struct
{
    u8 pad0[0x24];
    u8 unk24;
    u8 pad25[0x23A - 0x25];
    u8 unk23A;
    u8 pad23B[0x244 - 0x23B];
} FieldActorState;

typedef struct
{
    u8 pad0[0x20];
    u8 scale;
    u8 pad21[0x2A - 0x21];
    s16 state;
    u8 pad2C[0x2E - 0x2C];
    u16 transform_mode;
    u8 pad30[0x3A - 0x30];
    u8 track;
} FieldActorRecord;

extern FieldTrackEntry D_80105880[];
extern FieldActorState g_field_actor_slots[];

s32 func_80097FA0(FieldActorRecord *record, s32 *vector, s32 mode);

/**
 * @brief Validate an actor's active track state or submit a scaled scratchpad vector.
 * @param record Actor record to update.
 * @param x X component used by the transform path.
 * @param y Y component used by the transform path.
 * @param z Z component used by the transform path.
 */
void func_800949CC(FieldActorRecord *record, s32 x, s32 y, s32 z)
{
    s32 offset;
    s32 key;
    s32 selector;
    u8 *first_base;
    u8 *second_base;
    u8 *third_base;
    FieldActorState *actors;
    FieldActorState *actor;
    s32 *scratch;

    scratch = (s32 *)0x1F800000;
    if (record->transform_mode == 0)
    {
        first_base = (u8 *)D_80105880;
        if ((u8)record->track < 2U)
        {
            offset = record->track * 0x1C;
        }
        else
        {
            offset = 0x38;
        }
        selector = record->track;
        key = *(s32 *)(first_base + offset + 0xC);
        if (key == selector)
        {
            actors = g_field_actor_slots;
            second_base = (u8 *)D_80105880;
            if ((u32)(key & 0xFF) < 2U)
            {
                offset = key * 0x1C;
            }
            else
            {
                offset = 0x38;
            }
            actor = actors + *(s32 *)(second_base + offset + 0x18);
            if (actor->unk24 != 0)
            {
                actors = g_field_actor_slots;
                third_base = (u8 *)D_80105880;
                if ((u8)record->track < 2U)
                {
                    offset = record->track * 0x1C;
                }
                else
                {
                    offset = 0x38;
                }
                actor = actors + *(s32 *)(third_base + offset + 0x18);
                if (actor->unk23A == 0)
                {
                    record->state = 0;
                }
            }
            else
            {
                record->state = 0;
            }
        }
        else
        {
            record->state = 0;
        }
    }
    else
    {
        scratch[0] = x * record->scale;
        scratch[1] = y * record->scale;
        scratch[2] = z * record->scale;
        func_80097FA0(record, scratch, 0);
    }
}
