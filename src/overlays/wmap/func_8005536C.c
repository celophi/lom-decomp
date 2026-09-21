#include "common.h"
#include "sdk/libgpu.h"

typedef struct
{
    u8 _pad00[0x74];
    u_long ordering_table_tag;
    u8 _pad78[0x2C4];
    u8* packet_cursor;
} WmapRenderContext;

typedef struct
{
    s32 object_id;
    u8 _pad04[0x24];
} WmapCell;

typedef struct
{
    u8 _pad00[0x28];
    s16 timer;
    u8 _pad2A[2];
} WmapObject;

extern SPRT D_800C4600;
extern s32 D_800D7CC4;
extern s32 D_800D7CC8;
extern s32 D_800D921C;
extern WmapCell D_80139290[][6];
extern s32 D_80139880;
extern WmapRenderContext* D_801398EC;
extern s32 D_80139948;
extern s32 D_80182230;
extern WmapObject D_80182248[];

extern void func_80058014(s32, s32);
extern void func_8005FF88(s32);
extern void func_8006534C(s32, s32);

/**
 * @brief Advance the active world-map transition and refresh nearby object timers.
 */
void func_8005536C(void)
{
    SPRT* sprite;
    s32 i;
    s32 j;
    s32 object_id;

    sprite = (SPRT*)D_801398EC->packet_cursor;
    *sprite = D_800C4600;
    addPrim(&D_801398EC->ordering_table_tag, sprite);

    if (D_800D921C < 0x7D00)
    {
        D_800D921C += sizeof(SPRT);
        D_801398EC->packet_cursor += sizeof(SPRT);
    }

    D_80182230--;
    if (D_80182230 == 60)
    {
        for (i = D_800D7CC8; i < D_800D7CC8 + 3; i++)
        {
            for (j = D_800D7CC4; j < D_800D7CC4 + 3; j++)
            {
                object_id = D_80139290[j][i].object_id;
                func_80058014(object_id, 1);
                D_80182248[object_id].timer = 0;
            }
        }
    }

    if (D_80182230 == 0)
    {
        D_80182230 = 0x384;
        D_80139880 = 2;
        D_80139948 = 30;
    }

    func_8006534C(0x55, 1);
    func_8005FF88(-1);
}
