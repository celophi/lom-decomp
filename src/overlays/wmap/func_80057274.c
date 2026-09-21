#include "common.h"
#include "sdk/libgpu.h"

typedef struct
{
    u8 pad_000[0x70];
    u_long ordering_table[179];
    void* prim_cursor;
} WmapRenderState;

extern SPRT D_8004FD94[];
extern u8 D_800CC0C4[];
extern u8 D_800CC0D0[];
extern u8 D_800D7CD0[];
extern u8 D_800D7CD8[];
extern s32 D_800D921C;
extern WmapRenderState* D_801398EC;
extern s32 D_8013B268;

void func_8006534C(s32 type, s32 depth);

/**
 * @brief Advance selected world-map animation timers and draw all eight sprites.
 * @param selected_index Animation index to advance, or -1 to advance every animation.
 */
void func_80057274(s32 selected_index)
{
    s32 i;

    for (i = 0; i < 8; i++)
    {
        SPRT* sprite;
        u8 frame;

        if (selected_index == -1 || selected_index == i)
        {
            D_800D7CD0[i]--;
        }

        if ((s8)D_800D7CD0[i] <= 0)
        {
            frame = D_800D7CD8[i] + 2;
            D_800D7CD8[i] = frame;
            if ((s8)frame >= D_800CC0C4[i + 1])
            {
                D_800D7CD8[i] = D_800CC0C4[i];
            }
            D_800D7CD0[i] = D_800CC0D0[(s8)D_800D7CD8[i] + 1];
        }

        sprite = D_801398EC->prim_cursor;
        *sprite = D_8004FD94[i];
        sprite->r0 = sprite->g0 = sprite->b0 = D_8013B268;
        sprite->u0 = (D_800CC0D0[(s8)D_800D7CD8[i]] % 5) * 24;
        sprite->v0 = (D_800CC0D0[(s8)D_800D7CD8[i]] / 5) * 24;

        if (D_8013B268 < 0x40)
        {
            setSemiTrans(sprite, 1);
        }

        addPrim(&D_801398EC->ordering_table[4], sprite);

        if (D_800D921C < 0x7D00)
        {
            D_801398EC->prim_cursor = (u8*)D_801398EC->prim_cursor + sizeof(SPRT);
            D_800D921C += sizeof(SPRT);
        }
    }

    func_8006534C(0x3D, 4);
}
