/**
 * @file field_record_buffer_ops.c
 * @brief Copy or clear record buffers and sort packed value/key lists.
 */

#include "common.h"

/** @brief One (value, key) pair of a FieldKeyedList. */
typedef struct FieldKeyedPair
{
    u32 value;
    u32 key;
} FieldKeyedPair;

/** @brief Counted list of (value, key) pairs. */
typedef struct FieldKeyedList
{
    u32 count;
    FieldKeyedPair pairs[1];
} FieldKeyedList;

/**
 * @brief Copy or clear a buffer in 32-bit words.
 * @param src Source buffer, or NULL to clear the destination.
 * @param dest Destination buffer.
 * @param size Byte count; only complete 32-bit words are processed.
 * @return Pointer to the first destination word after the processed range.
 */
s32* func_800C1EC8(s32* src, s32* dest, s32 size)
{
    s32 count;

    count = size / 4;
    if (src != NULL)
    {
        while (count != 0)
        {
            *dest++ = *src++;
            count--;
        }
    }
    else
    {
        while (count != 0)
        {
            *dest++ = 0;
            count--;
        }
    }
    return dest;
}

/**
 * @brief Sort a (value, key) pair list in place by ascending key.
 * @param list Pair list to sort.
 */
void func_800C1F28(FieldKeyedList* list)
{
    u32 i;
    u32 j;
    u32 key;
    u32 value;

    for (i = 0; i < list->count; i++)
    {
        for (j = 1; j < list->count; j++)
        {
            key = list->pairs[i].key;
            if (list->pairs[j].key < key)
            {
                value = list->pairs[i].value;
                list->pairs[i].value = list->pairs[j].value;
                list->pairs[i].key = list->pairs[j].key;
                list->pairs[j].value = value;
                list->pairs[j].key = key;
            }
        }
    }
}
