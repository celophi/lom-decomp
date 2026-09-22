#include "wmap_sequence_runtime.h"
/* Partial WMAP decompilation: 97.346664% (gcc280_g0). */
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
    s32 field_14;
    s32 previous_x;
    s32 previous_y;
    u8 unknown_20[0x104];
} WmapCoordinateRecord;

#include "cdrom.h"


/** @brief World-map actor configuration. */
typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad_04[2];
    u8 field_06;
    u8 pad_07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad_12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad_28[4];
} WmapConfigA;

/** @brief Per-actor motion and animation parameters. */
typedef struct
{
    s16 state;
    s16 angle;
    s32 x;
    s32 z;
    s16 scale;
    s16 field_0E;
    s32 field_10;
} WmapMotion;

/** @brief Animation resource slot. */
typedef struct
{
    s32 field_00;
    void *resource;
} WmapResource;

typedef struct
{
    s32 tile;
    u8 pad_04[36];
} WmapTile;
extern WmapCoordinateRecord D_8019D248[];
extern WmapConfigA D_800D9268[];
extern WmapResource D_80139988[];
extern WmapTile D_80139290[6][6];
extern u8 D_800DBE98[];
extern u8 D_800DC298[];
extern u8 D_800DC698[];
extern s16 D_800D926A;
extern s16 D_800D9296;
extern s16 D_800D92C2;
extern s32 func_8005D850(s32 *, u32 *);
extern void func_80058FF4(s32, s32, s32);
extern void func_80064F64(s32);
extern s32 D_800D9224;
extern s32 D_800DBE78;
extern s32 D_8011CF20;
extern s32 D_8011CF50;
extern s32 D_8013922C;
extern s32 D_801398B8;
extern s32 D_801398C0;
extern s32 D_80182D5C;
extern s32 D_80182E1C;
extern s32 D_80182E34;

/** @brief Load map actor resources and restore the active map coordinates. */
void func_80058D9C(void)
{
    s32 first_x, first_y, second_x, second_y;
    s32 i;
    s32 resource_id;
    u8 *resource;
    WmapCoordinateRecord *record;
    WmapConfigA *actor;

    cdrom_queue_read(0x10C9, D_800DBE98);
    cdrom_wait_queue_empty();
    resource = D_800DBE98;
    D_800D9268[0].field_02 = 0;
    for (i = 0; i < 4; i++)
    {
        record = &D_8019D248[i];
        actor = &D_800D9268[i];
        record->y = 1;
        record->x = 1;
        record->grid_y = 1;
        record->grid_x = 1;
        record->previous_y = 1;
        record->previous_x = 1;
        record->target_y = 0;
        record->target_x = 0;
        record->position_y = 0;
        record->position_x = 0;
        record->field_14 = 0;
        actor->field_06 = 15;
        actor->field_10 = -1;
        actor->field_22 = 128;
        actor->field_24 = 128;
        D_80139988[i].resource = resource;
        resource += 0x400;
    }
    D_80182D5C = func_8005D850(&D_8019D248[0].x, (u32 *)&D_8019D248[0].y);
    func_80058FF4(0, D_8019D248[0].x, D_8019D248[0].y);
    if (D_80139290[D_8019D248[0].x][D_8019D248[0].y].tile == 24)
    {
        D_8011CF50 = 1;
        D_801398C0 = 0;
        D_8013922C = 0;
        D_80182E34 = 3;
        D_800DBE78 = 3;
        D_8011CF20 = 1;
        D_800D926A = -1;
        D_800D9224++;
    }
    resource_id = 0x10CA;
    if (D_80182D5C != 0)
    {
        resource_id = 0x10CB;
    }
    func_80064F64(resource_id);
    if (D_801398B8 != 0)
    {
        cdrom_queue_read(0x10CC, D_800DC298);
        D_800D9296 = 1;
        func_8006D0F0(27, &first_x, &first_y);
        func_80058FF4(1, first_x, first_y);
        cdrom_wait_queue_empty();
    }
    if (D_80182E1C != 0)
    {
        cdrom_queue_read(0x10CD, D_800DC698);
        D_800D92C2 = 2;
        func_8006D0F0(3, &second_x, &second_y);
        func_80058FF4(2, second_x, second_y);
        cdrom_wait_queue_empty();
    }
}
