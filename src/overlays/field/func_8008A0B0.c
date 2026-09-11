#include "common.h"

/** @brief Field actor record with position, animation state, and runtime index. */
typedef struct
{
    s32 unk0;
    s32 unk4;
    s32 unk8;
    u8 padc[0x18];
    u8 unk24;
    u8 pad25[5];
    s16 unk2a;
    u8 pad2c[2];
    s16 unk2e;
    u8 pad30[10];
    u8 unk3a;
    u8 pad3b[0x19];
} FieldRecord;

/** @brief Runtime actor state containing position and collision path fields. */
typedef struct
{
    u8 pad0[0x50];
    s32 unk50;
    s32 unk54;
    s32 unk58;
    u8 pad5c[0x1A4 - 0x5C];
    s16 unk1a4;
    s16 unk1a6;
    u8 pad1a8[4];
    s32 unk1ac;
    s32 unk1b0;
    u8 pad1b4[0x23C - 0x1B4];
} FieldState;

/** @brief Position view at runtime state offset 0x50, with the 0x23C-byte stride. */
typedef struct
{
    s32 unk0;
    s32 unk4;
    s32 unk8;
    u8 padc[0x23C - 12];
} StatePosition;

/** @brief Actor part definition view used to select the collision footprint. */
typedef struct
{
    u8 pad0[0x2E];
    u8 unk2e;
    u8 pad2f[0x48 - 0x2F];
} PartDef;

/** @brief Map width and depth fields at the fixed map-header address. */
typedef struct
{
    s16 unk0;
    u16 unk2;
} MapBounds;

/** @brief Collision probe position, horizontal footprint, and height tolerance. */
typedef struct
{
    s32 x;
    s32 y;
    s32 z;
    u16 width;
    s16 height;
    u16 depth;
} CollisionQuery;

extern FieldRecord D_800FDF58[];
extern PartDef D_800FE3A0[];
extern FieldState D_80105AE0[];
extern StatePosition D_80105B30[];
s32 func_80060F58(CollisionQuery *, CollisionQuery *, void *, s32);
void func_8006304C(CollisionQuery *);
void func_8006C3FC(FieldRecord *);

/**
 * @brief Compute a path from an actor to another record, with direct fallback.
 * @param record Actor record whose runtime path should be updated.
 * @param source_index Record supplying the destination coordinates.
 * @param update_animation Nonzero to reset and apply the actor animation.
 */
void func_8008A0B0(FieldRecord *record, s32 source_index, s32 update_animation)
{
    CollisionQuery start;
    CollisionQuery goal;
    s32 path_length;
    MapBounds *bounds;
    s32 x;
    s32 map_depth;
    s32 map_width;
    s32 state_x;
    s32 state_z;
    s32 z;
    FieldRecord *source;

    bounds = (MapBounds *)0x801ED400;
    x = record->unk0;
    if ((x < 0) ||
        (map_width = bounds->unk0 << 8, ((x < map_width) == 0)) ||
        (z = record->unk8, (z < 0)) ||
        (map_depth = (s32) (bounds->unk2 << 0x10) >> 7, ((z < map_depth) == 0)) ||
        (state_x = D_80105AE0[record->unk3a].unk50, (state_x < 0)) ||
        (state_x >= map_width) ||
        (state_z = D_80105AE0[record->unk3a].unk58, (state_z < 0)) ||
        (state_z >= map_depth))
    {
        (&D_80105AE0[record->unk3a])->unk1a6 = 0;
        (&D_80105AE0[record->unk3a])->unk1ac = D_800FDF58[source_index].unk0;
        (&D_80105AE0[record->unk3a])->unk1b0 = D_800FDF58[source_index].unk8;
        D_80105AE0[record->unk3a].unk1a4 = 1;
    }
    else
    {
        start.x = x;
        start.y = record->unk4;
        start.z = record->unk8;
        if ((&D_800FE3A0[record->unk3a])->unk2e == 0x40)
        {
            start.width = 0xC;
            start.depth = 8;
            goal.width = 0xC;
            goal.depth = 8;
        }
        else
        {
            start.width = 9;
            start.depth = 6;
            goal.width = 9;
            goal.depth = 6;
        }
        start.height = 0x10;
        goal.height = 0x10;
        func_8006304C(&start);
        D_80105B30[record->unk3a].unk0 = D_800FDF58[source_index].unk0;
        source = &D_800FDF58[source_index];
        (&D_80105B30[record->unk3a])->unk4 = (s32) source->unk4;
        (&D_80105B30[record->unk3a])->unk8 = (s32) source->unk8;
        goal.x = source->unk0;
        goal.y = source->unk4;
        goal.z = source->unk8;
        path_length = func_80060F58(&start, &goal, (u8 *)&D_80105B30[record->unk3a] + 0x15C, 0);
        if (path_length <= 0)
        {
            (&((FieldState *)((u8 *)D_80105B30 - 0x50))[record->unk3a])->unk1a6 = 0;
            (&((FieldState *)((u8 *)D_80105B30 - 0x50))[record->unk3a])->unk1ac = (s32) source->unk0;
            (&((FieldState *)((u8 *)D_80105B30 - 0x50))[record->unk3a])->unk1b0 = (s32) source->unk8;
            ((FieldState *)((u8 *)D_80105B30 - 0x50))[record->unk3a].unk1a4 = 1;
        }
        else
        {
            (&((FieldState *)((u8 *)D_80105B30 - 0x50))[record->unk3a])->unk1a4 = path_length;
            (&((FieldState *)((u8 *)D_80105B30 - 0x50))[record->unk3a])->unk1a6 = 0;
        }
    }
    if (update_animation != 0)
    {
        record->unk2a = 0xB5;
        record->unk2e = 0xFF;
        record->unk24 = 1;
        func_8006C3FC(record);
    }
}
