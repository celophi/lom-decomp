#include "common.h"

extern u8 *func_800C1E40(s32);
extern u8 *D_80123FC0;

typedef struct
{
    u8 bytes[0x58];
    unsigned int flags : 4;
    unsigned int other_flags : 28;
} FieldRecordState;

typedef struct
{
    u8 value;
    u8 padding;
} FieldBytePair;

typedef struct
{
    u8 prefix[0xC];
    FieldBytePair pairs[8];
} FieldPairState;

extern FieldRecordState *D_80123FC4;

/**
 * @brief Copy the selected resource record into the shared field state.
 */
void func_800BF158(void)
{
    s32 i;
    u8 *record;
    FieldRecordState *state;

    record = func_800C1E40(4);
    do
    {
        state = D_80123FC4;
    } while (0);
    D_80123FC0 = record;
    i = 0;
    record += state->bytes[6] * 20;
    *(u16 *)(state->bytes + 0x3A) = *(u16 *)(record + 0x186);
    do
    {
        s32 source_offset;
        u8 *dest;

        source_offset = i + D_80123FC4->bytes[6] * 20;
        dest = D_80123FC4->bytes + i;
        dest[0x3C] = *(D_80123FC0 + source_offset + 0x188);
        i++;
    } while (i < 4);
    i = 0;
    do
    {
        s32 source_offset;
        u8 *dest;

        source_offset = i + D_80123FC4->bytes[6] * 20;
        dest = D_80123FC4->bytes + i;
        dest[0x40] = *(D_80123FC0 + source_offset + 0x18C);
        i++;
    } while (i < 4);
    i = 0;
    do
    {
        s32 source_offset;
        u8 *clear_dest;

        source_offset = i + D_80123FC4->bytes[6] * 20;
        ((FieldPairState *)D_80123FC4)->pairs[i].value = *(D_80123FC0 + source_offset + 0x190);
        clear_dest = D_80123FC4->bytes + i;
        clear_dest[0x44] = 0;
        i++;
    } while (i < 8);
    i = 1;
    D_80123FC4->flags = 0xF;
    do
    {
        u8 *entry;

        entry = D_80123FC4->bytes + i;
        if (entry[0x28] < 0x10U)
        {
            D_80123FC4->flags = entry[0x28];
        }
        i++;
    } while (i < 5);
}
