#include "common.h"

/** @brief Coordinate fields at the head of a 0x124-byte world-map record. */
typedef struct
{
    s32 x;
    s32 y;
    s16 grid_x;
    s16 grid_y;
    s16 position_x;
    s16 position_y;
    s16 target_x;
    s16 target_y;
    u8 unknown_14[4];
    s32 previous_x;
    s32 previous_y;
    u8 unknown_20[0x104];
} WmapCoordinateRecord;

extern WmapCoordinateRecord D_8019D248[];

/**
 * @brief Initialize grid coordinates and corresponding pixel positions.
 * @param index World-map record index.
 * @param x Grid X coordinate.
 * @param y Grid Y coordinate.
 */
void func_80058FF4(s32 index, s32 x, s32 y)
{
    s16 pixel_x;
    s16 pixel_y;
    WmapCoordinateRecord *record;
    WmapCoordinateRecord *base;

    base = D_8019D248;
    record = &base[index];
    record->x = x;
    record->previous_x = x;
    record->grid_x = (s16) x;
    pixel_x = ((s16) x - 1) * 0xA0;
    record->y = y;
    record->previous_y = y;
    record->grid_y = (s16) y;
    record->target_x = pixel_x;
    record->position_x = pixel_x;
    pixel_y = ((s16) y - 1) * 0xA0;
    record->target_y = pixel_y;
    record->position_y = pixel_y;
}
