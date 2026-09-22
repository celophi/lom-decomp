#include "common.h"
#include "sdk/libgte.h"

typedef struct
{
    s32 object_id;
    s16 flag;
    u8 pad06[0x22];
} WmapGridCell;

typedef struct
{
    u8 pad00[4];
    u8 state;
    u8 phase;
    s8 transition;
    u8 pad07[0x25];
} WmapObject;

typedef struct
{
    u8 pad00[0xC];
    s32 active;
    u8 pad10[4];
} WmapPartRecord;

extern s32 D_800DBE70;
extern WmapPartRecord D_8011CF88[];
extern s32 D_8011D4FC;
extern SVECTOR D_80139278;
extern WmapGridCell D_80139290[][6];
extern s32 D_8013986C;
extern SVECTOR D_801398C8;
extern s32 D_80139958;
extern s32 D_80139978;
extern WmapObject D_80182248[];
extern VECTOR D_80182D48;
extern VECTOR D_80182DC0;
extern s32 D_801ADAE0;

s32 func_80055BB0(WmapObject* object);
void func_80055E5C(s32 x, s32 y, WmapObject* object);
void func_800561F8(s32 x, s32 y, WmapObject* object, s32 part_index);
void func_80056C30(s32 x, s32 y);
void func_8005833C(s32 x, s32 y, s32 state);
s32 func_80058400(s32 x, s32 y);

/**
 * @brief Update world-map object display states for the visible map grid.
 */
void func_80056824(void)
{
    MATRIX matrix;
    VECTOR translation;
    SVECTOR rotation;
    VECTOR world_position;
    s32 x;
    s32 y;
    s32 object_id;
    s32 state;
    s32 part_index;
    s32 distance_state;

    rotation.vx = D_80139278.vx + D_801398C8.vx;
    rotation.vy = D_80139278.vy + D_801398C8.vy;
    rotation.vz = D_80139278.vz + D_801398C8.vz;

    world_position.vx = D_80182DC0.vx + D_80182D48.vx;
    world_position.vy = D_80182DC0.vy + D_80182D48.vy;
    world_position.vz = D_80182DC0.vz + D_80182D48.vz;

    translation = world_position;
    translation.vz = (translation.vz * D_80139958) / 0x6000;

    RotMatrix(&rotation, &matrix);
    TransMatrix(&matrix, &translation);
    SetRotMatrix(&matrix);
    SetTransMatrix(&matrix);

    for (y = 0; y < 6; y++)
    {
        for (x = 0; x < 6; x++)
        {
            object_id = D_80139290[x][y].object_id;

            if (D_8013986C == 1)
            {
                state = 1;
            }
            else if (object_id == D_80139978)
            {
                state = 2;
            }
            else
            {
                s32 base_state;

                distance_state = func_80058400(x * 0x30, y * 0x30);
                base_state = D_800DBE70;
                if (base_state != 2)
                {
                    state = base_state;
                }
                if ((base_state == 2) || (distance_state < state))
                {
                    state = distance_state;
                }
            }

            if (object_id >= 0x100)
            {
                state = 5;
                object_id &= 0xFF;
            }

            if (object_id != 0xFF)
            {
                WmapObject* current_object;

                current_object = &D_80182248[object_id];
                if (current_object->transition >= 0x10)
                {
                    current_object->transition = 0xF;
                }
                else if (current_object->transition < 0)
                {
                    current_object->transition = 0;
                }

                if (state == 5)
                {
                    current_object->state = 2;
                    current_object->phase = 2;
                    current_object->transition = 0xF;
                }
                else if (state != current_object->state)
                {
                    switch (current_object->state)
                    {
                    case 0:
                        current_object->state = state;
                        if (state == 1)
                        {
                            current_object->phase = state;
                        }
                        else
                        {
                            current_object->phase = 3;
                        }
                        break;
                    case 1:
                        current_object->state = state;
                        if (state != 0)
                        {
                            current_object->phase = 3;
                        }
                        else
                        {
                            current_object->phase = 0;
                            current_object->transition = 0;
                        }
                        break;
                    case 2:
                        current_object->state = state;
                        if (state != 0)
                        {
                            current_object->phase = 4;
                        }
                        else
                        {
                            current_object->phase = 0;
                            current_object->transition = 0;
                        }
                        break;
                    }
                }

                if (state != 0)
                {
                    WmapObject* active_object;

                    active_object = &D_80182248[object_id];
                    if (active_object->phase != 1)
                    {
                        part_index = func_80055BB0(active_object);
                        if ((part_index != -1) && (D_8011CF88[part_index].active == 0))
                        {
                            func_800561F8(x, y, active_object, part_index);
                        }
                        else
                        {
                            func_80055E5C(x, y, active_object);
                        }
                    }
                    else
                    {
                        func_80055E5C(x, y, active_object);
                    }
                }
            }

            if ((D_8011D4FC == -1) || (D_801ADAE0 != 0))
            {
                state = 0;
            }
            func_8005833C(x, y, state);

            if ((D_80139290[x][y].flag != 0) && (D_8013986C == 0))
            {
                func_80056C30(x, y);
            }
        }
    }
}
