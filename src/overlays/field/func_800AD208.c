#include "common.h"
#include "sdk/libgpu.h"

void *func_800AD42C(void *, s32 *, s32, u16 *, s32); /* extern */

/**
 * @brief Draw a right-aligned decimal value and append its draw-mode primitive.
 * @param ordering_table Ordering-table entry receiving the emitted primitives.
 * @param packet_cursor First free primitive-buffer address.
 * @param value Decimal value to draw.
 * @param digit_count Number of digit positions available for the value.
 * @param position Packed x/y position; the x coordinate advances as digits are emitted.
 * @param flags Digit rendering flags passed to the sprite builder.
 * @return First free primitive-buffer address after the emitted primitives.
 */
void *func_800AD208(s32 *ordering_table, u8 *packet_cursor, s32 value, s32 digit_count, u16 *position, s32 flags)
{
    s32 base_x;
    s32 digit;
    s32 divisor;
    s32 digit_index;

    divisor = 1;
    digit_index = divisor;
    if (digit_index < digit_count)
    {
        do
        {
            divisor *= 10;
            digit_index += 1;
        } while (digit_index < digit_count);
    }
    for (digit_index = 0; digit_index < digit_count; digit_index++)
    {
        if (value >= divisor)
        {
            break;
        }
        divisor /= 10;
    }
    if (digit_index != digit_count)
    {
        *position += digit_index * 8;
        if (digit_index < digit_count)
        {
            do
            {
                digit = value / divisor;
                packet_cursor = func_800AD42C(packet_cursor, ordering_table, digit, position, flags);
                value -= digit * divisor;
                digit_index += 1;
                *position += 8;
                divisor /= 10;
            } while (digit_index < digit_count);
        }
    }
    else
    {
        base_x = *position - 8;
        *position = base_x + digit_count * 8;
        packet_cursor = func_800AD42C(packet_cursor, ordering_table, value, position, flags);
        *position += 8;
    }
    setlen((DR_TPAGE *)packet_cursor, 1);
    ((DR_TPAGE *)packet_cursor)->code[0] = 0xE1000007;
    addPrim(ordering_table, (DR_TPAGE *)packet_cursor);
    return (DR_TPAGE *)packet_cursor + 1;
}
