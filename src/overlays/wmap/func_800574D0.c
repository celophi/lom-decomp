#include "common.h"
#include "sdk/libgpu.h"

typedef struct
{
    s32 x;
    s32 y;
    s32 scale;
} WmapProjection;

typedef struct
{
    u32 packed_xy;
    u8 pad04[0x10];
} WmapSpritePosition;

typedef struct
{
    u8 pad000[0x74];
    u_long ot_entry;
    u8 pad078[0x2C4];
    SPRT* prim_cursor;
} WmapRenderState;

extern WmapSpritePosition D_8004FD9C[8];
extern SPRT D_8004FE34[16];
extern SPRT D_8004FF74[16];
extern s32 D_800D921C;
extern s32 D_800DCEEC;
extern s32 D_800DCEF0;
extern s32 D_8011CF18;
extern s32 D_8011CF74;
extern s32 D_8011D4FC;
extern s32 D_80129550;
extern WmapRenderState* D_801398EC;
extern WmapProjection D_80139950;
extern s32 D_8013B268;

void func_8005C404(s32 x, s32 y, s32 mode, s32 map_x, s32 map_y, s32* indices);
void func_8005D6B8(s32 x, s32 y, s32* indices);
void func_8005D7A0(s32 value, s32* indices);
s32 func_8005D8FC(void);
void func_8006534C(s32 id, s32 mode);

/**
 * @brief Render the current world-map sprite set.
 */
void func_800574D0(void)
{
    s32 sprite_indices[8];
    s32 x;
    s32 y;
    s32 i;
    s32 color;

    if (D_8013B268 == 0)
    {
        return;
    }

    x = D_80139950.x / 48 + D_800DCEEC;
    y = D_80139950.y / 48 + D_800DCEF0;

    if (x < 0)
    {
        x = 0;
    }
    if (y < 0)
    {
        y = 0;
    }
    if (x >= 6)
    {
        x = 5;
    }
    if (y >= 6)
    {
        y = 5;
    }

    color = D_8013B268 + (D_8011CF74 & 8);
    if (color < 0)
    {
        color = 0;
    }

    if (D_8011CF18 == 2)
    {
        func_8005D7A0(func_8005D8FC(), sprite_indices);
    }
    else if (D_80129550 == 1)
    {
        func_8005C404(x, y, D_8011D4FC, x, y, sprite_indices);
    }
    else
    {
        func_8005D6B8(x, y, sprite_indices);
    }

    i = 0;
    do
    {
        SPRT* primary_sprite;
        SPRT* secondary_sprite;

        primary_sprite = D_801398EC->prim_cursor;
        *primary_sprite = D_8004FF74[sprite_indices[i]];
        primary_sprite->r0 = primary_sprite->g0 = primary_sprite->b0 = color;
        *(u32*)&primary_sprite->x0 = D_8004FD9C[i].packed_xy;
        primary_sprite->clut = getClut(0x2E0, i + 0x1A8);
        setSemiTrans(primary_sprite, 1);
        addPrim(&D_801398EC->ot_entry, primary_sprite);
        if (D_800D921C < 0x7D00)
        {
            D_800D921C += sizeof(SPRT);
            D_801398EC->prim_cursor++;
        }

        secondary_sprite = D_801398EC->prim_cursor;
        *secondary_sprite = D_8004FE34[sprite_indices[i]];
        secondary_sprite->r0 = secondary_sprite->g0 = secondary_sprite->b0 = D_8013B268;
        *(u32*)&secondary_sprite->x0 = D_8004FD9C[i].packed_xy;
        secondary_sprite->clut = getClut(0x2E0, i + 0x1A0);
        secondary_sprite->x0 += 4;
        secondary_sprite->y0 += 0x14;
        if (D_8013B268 < 0x40)
        {
            setSemiTrans(secondary_sprite, 1);
        }
        addPrim(&D_801398EC->ot_entry, secondary_sprite);
        if (D_800D921C < 0x7D00)
        {
            D_800D921C += sizeof(SPRT);
            D_801398EC->prim_cursor++;
        }

        i++;
    } while (i < 8);

    func_8006534C(0x3D, 1);
}
