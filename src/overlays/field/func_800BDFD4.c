#include "common.h"

#define FIELD_STATE_RECORD_COUNT 11
#define FIELD_STATE_RECORD_SIZE 0x68
#define FIELD_STATE_RECORD(offset) ((FieldStateRecord *)(D_80123FB0 + (offset)))

typedef struct
{
    s32 x;
    s32 y;
    s32 z;
} FieldPosition;

typedef struct
{
    u8 pad0[4];
    s32 active;
} FieldRecordState;

typedef struct
{
    u8 pad0[0x28];
    u8 flags;
    u8 pad29[3];
    u8 object_id;
    u8 pad2D[11];
    FieldRecordState *state;
} FieldStateRecord;

typedef struct
{
    u8 pad0[4];
    u8 reference_id;
} FieldDistanceSource;

typedef struct
{
    u8 pad0[4];
    s32 flag_mask;
} FieldRecordFilter;

typedef struct
{
    s32 object_id;
    s32 distance;
} FieldDistanceEntry;

typedef struct
{
    s32 count;
    FieldDistanceEntry entries[1];
} FieldDistanceList;

s32 func_80087F44(s32 object_id, FieldPosition *position);
s32 func_800C1FBC(FieldPosition *first, FieldPosition *second);

extern u8 *D_80123FB0;

/**
 * @brief Collect active field records selected by a flag mask and their distances from a reference object.
 * @param output Output list whose first word is the number of collected records.
 * @param source Supplies the reference object identifier used for distance measurements.
 * @param filter Supplies the flag mask used to select field records.
 */
void func_800BDFD4(FieldDistanceList *output, FieldDistanceSource *source, FieldRecordFilter *filter)
{
    FieldPosition reference_position;
    FieldPosition record_position;
    s32 i;
    s32 offset;

    func_80087F44(source->reference_id, &reference_position);
    i = 0;
    output->count = 0;
    do
    {
        offset = i * FIELD_STATE_RECORD_SIZE;
        if ((FIELD_STATE_RECORD(offset)->flags & filter->flag_mask) &&
            FIELD_STATE_RECORD(offset)->state->active != 0)
        {
            func_80087F44(FIELD_STATE_RECORD(offset)->object_id, &record_position);
            output->entries[output->count].object_id = FIELD_STATE_RECORD(offset)->object_id;
            output->entries[output->count].distance = func_800C1FBC(&record_position, &reference_position);
            output->count += 1;
        }
        i++;
    } while (i < FIELD_STATE_RECORD_COUNT);
}
