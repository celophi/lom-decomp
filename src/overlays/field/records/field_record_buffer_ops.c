/**
 * @file field_record_buffer_ops.c
 * @brief Copy or clear record buffers and sort packed value/key lists.
 */

#include "common.h"
#include "../internal/field_calls.h"
#include "../internal/field_records.h"

/**
 * @brief Copy or clear a buffer in 32-bit words.
 * @param src Source buffer, or NULL to clear the destination.
 * @param dest Destination buffer.
 * @param size Byte count; only complete 32-bit words are processed.
 * @return Pointer to the first destination word after the processed range.
 */
void* field_copy_words(const void* src, void* dest, s32 size)
{
    const s32* from;
    s32* to;
    s32 count;

    from = src;
    to = dest;
    count = size / 4;

    if (from != NULL)
    {
        while (count != 0)
        {
            *to++ = *from++;
            count--;
        }
    }
    else
    {
        while (count != 0)
        {
            *to++ = 0;
            count--;
        }
    }

    return to;
}

/**
 * @brief Reorder an id/distance list by unsigned distance: nearest first, the rest descending.
 * @param list List to reorder.
 * @note Every pass compares entry @c i with entries 1 to count - 1 (not i + 1 on), so after
 *       the first pass puts the smallest distance in entry 0 the remaining passes leave entries
 *       1 on in descending distance order.
 */
void field_sort_keyed_list(FieldKeyedList* list)
{
    u32 i;
    u32 j;
    u32 distance;
    u32 id;

    for (i = 0; i < list->count; i++)
    {
        for (j = 1; j < list->count; j++)
        {
            distance = list->entries[i].distance;

            if (list->entries[j].distance < distance)
            {
                id = list->entries[i].id;
                list->entries[i].id = list->entries[j].id;
                list->entries[i].distance = list->entries[j].distance;
                list->entries[j].id = id;
                list->entries[j].distance = distance;
            }
        }
    }
}
