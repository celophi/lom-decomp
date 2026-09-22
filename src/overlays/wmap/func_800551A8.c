#include "wmap_resource_support.h"
#include "wmap_main.h"
#include "common.h"
#include "sdk/libgpu.h"

typedef struct
{
    u8 pad_000[0x74];
    u_long ordering_table_entry;
    u8 pad_078[0x2C4];
    void* prim_cursor;
} WmapRenderState;

extern SPRT D_800C4588;
extern SPRT D_800C459C;
extern s32 D_800D7CCC;
extern s32 D_800D921C;
extern s32 D_800DBE78;
extern s32 D_800DCEC0;
extern s32 D_8011CF7C;
extern s32 D_8013922C;
extern s32 D_80139880;
extern WmapRenderState* D_801398EC;
extern s32 D_8013B25C;
extern s32 D_8013B298;
extern s32 D_80182230;

/**
 * @brief Update and enqueue the world-map sprite for the current state.
 */
void func_800551A8(void)
{
    SPRT* sprite;

    sprite = D_801398EC->prim_cursor;

    if (D_8013B298 < 8 && D_800D7CCC >= 12)
    {
        s32 state_flags;

        state_flags = D_8013922C;
        *sprite = D_800C4588;

        if (state_flags & 0x80)
        {
            D_80139880 = 1;
            D_80182230 = 0x78;
            func_800652A8(0x3C, 0x80);
            D_800D7CCC = 0;
            D_8013B298++;
        }
        else if (state_flags & 0x20)
        {
            D_8013B25C = 0x3C;
            D_80139880 = 0;
            D_8011CF7C = 1;
            D_800DCEC0 = 1;
            D_800DBE78 = 2;
            func_80064094();
        }
    }
    else
    {
        s32 state_flags;

        state_flags = D_8013922C;
        *sprite = D_800C459C;

        if (state_flags & 0x20)
        {
            D_8013B25C = 0x3C;
            D_80139880 = 0;
            D_8011CF7C = 1;
            D_800DCEC0 = 1;
            D_800DBE78 = 2;
            func_80064094();
        }
    }

    addPrim(&D_801398EC->ordering_table_entry, sprite);

    if (D_800D921C < 0x7D00)
    {
        D_800D921C += sizeof(SPRT);
        D_801398EC->prim_cursor = (u8*)D_801398EC->prim_cursor + sizeof(SPRT);
    }

    func_8006534C(0x55, 1);
}
