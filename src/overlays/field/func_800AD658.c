#include "common.h"
#include "sdk/libgpu.h"
#include "sdk/memory.h"

/**
 * @brief Add four black one-pixel-offset copies of a sprite group.
 * @param ordering_table Ordering-table entry receiving the copied sprites.
 * @param sprite_cursor First free sprite after the template group.
 * @param sprite_count Number of sprites in the template group.
 * @return First free sprite after the four copied groups.
 */
SPRT *func_800AD658(s32 *ordering_table, SPRT *sprite_cursor, s32 sprite_count)
{
    SPRT *source = sprite_cursor;
    s32 direction = 0;
    s32 count;
    SPRT *sprite;

    do
    {
        bcopy((u8 *)(source - sprite_count), (u8 *)sprite_cursor, sprite_count * sizeof(SPRT));
        count = 0;
        if (sprite_count > 0)
        {
            sprite = sprite_cursor;
            do
            {
                sprite->b0 = 0;
                sprite->g0 = 0;
                sprite->r0 = 0;
                switch (direction)
                {
                    case 0:
                        sprite->x0++;
                        break;
                    case 1:
                        sprite->x0--;
                        break;
                    case 2:
                        sprite->y0++;
                        break;
                    default:
                        sprite->y0--;
                        break;
                }
                sprite++;
                count++;
                addPrim(ordering_table, sprite_cursor);
                sprite_cursor++;
            } while (count < sprite_count);
        }
        direction++;
    } while (direction < 4);

    return sprite_cursor;
}
