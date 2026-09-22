#include "wmap_resource_support.h"
#include "common.h"

typedef struct
{
    s32 object_id;
    u8 pad04[0x24];
} WmapGridCell;

typedef struct
{
    u8 pad00[5];
    u8 state;
    s8 transition;
    u8 pad07[0x21];
    s16 timer;
    u8 pad2A[2];
} WmapObject;

extern s32 D_800D7CC4;
extern s32 D_800D7CC8;
extern s32 D_800D7CCC;
extern s32 D_800D922C;
extern s32 D_800D9234;
extern s32 D_800DCEEC;
extern s32 D_800DCEF0;
extern s32 D_8013922C;
extern WmapGridCell D_80139290[][6];
extern s32 D_80139880;
extern s32 D_801398C0;
extern s32 D_8013B298;
extern s32 D_80182230;
extern s32 D_80182238;
extern WmapObject D_80182248[];

void func_80055AF4(void);
void func_80058014(s32 object_id, s32 state);

/**
 * @brief Update world-map object transitions in the active cell neighborhood.
 */
void func_8005556C(void)
{
    s32 x;
    s32 y;
    s32 object_id;
    s32 step;
    WmapObject* object;

    D_80182230--;
    if (D_80182230 == 0)
    {
        D_80139880 = 0;
    }

    D_800D9234 = D_801398C0;
    func_80055AF4();

    y = D_800D7CC8 + D_800DCEF0;
    x = D_800D7CC4 + D_800DCEEC;
    object_id = D_80139290[x][y].object_id;
    object = &D_80182248[object_id];

    if (D_8013922C & 0x40)
    {
        if ((u32)(object->state - 2) < 2)
        {
            step = (D_8013B298 + 1) * 5;
            D_80182238 += step;
            D_800D7CCC++;
            if (object_id == 0x1F)
            {
                D_80182238 += step;
            }
            D_800D922C = D_80182238;
            func_80058014(object_id, 1);
            object->transition = 1;
            func_800652A8(0x3E, 0x80);
        }
        else
        {
            D_80182238 -= (D_8013B298 + 1) * 2;
            D_800D7CCC--;
            if (D_80182238 < 0)
            {
                D_80182238 = 0;
            }
            func_800652A8(0x3F, 0x80);
        }
    }

    for (y = D_800D7CC8; y < D_800D7CC8 + 3; y++)
    {
        for (x = D_800D7CC4; x < D_800D7CC4 + 3; x++)
        {
            object_id = D_80139290[x][y].object_id;
            object = &D_80182248[object_id];
            if (object->timer != 0)
            {
                object->timer--;
                if ((object->timer == 0) && (object->state != 1))
                {
                    func_80058014(D_80139290[x][y].object_id, 1);
                    object->transition = 4;
                    func_800652A8(2, 0x80);
                }
            }
        }
    }
}
