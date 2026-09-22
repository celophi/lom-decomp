/* Partial WMAP decompilation: 86.301650% (gcc280_g0). */
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
    u8 pad00[4];
    s16 unk04;
    u8 pad06[0x22];
} WmapTileEntry;

typedef struct
{
    u8 pad000[0x74];
    u_long ot_entry;
    u8 pad078[0x2C4];
    SPRT* prim_cursor;
} WmapRenderState;

extern s32 D_8004FC74[];
extern SPRT D_8004FE34[16];
extern s32 D_800D921C;
extern WmapProjection D_800DCEC8;
extern s32 D_800DCEEC;
extern s32 D_800DCEF0;
extern s32 D_8011CF74;
extern s32 D_8011D4FC;
extern s32 D_80129550;
extern WmapTileEntry D_80139290[][6];
extern WmapRenderState* D_801398EC;
extern s32 D_8013B268;

void func_8005C404(s32 x, s32 y, s32 mode, s32 map_x, s32 map_y, s32* indices);
void func_8005D6B8(s32 x, s32 y, s32* indices);
void func_8006534C(s32 id, s32 mode);

void func_8005784C(s32 arg0)
{
    s32 sprite_indices[8];
    s32 target_x;
    s32 target_y;
    s32 cur_x;
    s32 cur_y;
    s32 base_index;
    SPRT* sprite;

    if (D_8013B268 == 0)
    {
        return;
    }

    target_x = D_800DCEC8.x / 48 + D_800DCEEC;
    target_y = D_800DCEC8.y / 48 + D_800DCEF0;

    cur_y = 0;
    base_index = 0;
    do
    {
        cur_x = 0;
        do
        {
            if (D_80129550 == 1 && D_80139290[target_x][target_y].unk04 != 0)
            {
                func_8005C404(cur_x, cur_y, D_8011D4FC, target_x, target_y, sprite_indices);
            }
            else
            {
                func_8005D6B8(cur_x, cur_y, sprite_indices);
            }

            if (cur_x == target_x && cur_y == target_y && (D_8011CF74 & 4))
            {
                sprite = D_801398EC->prim_cursor;
                *sprite = D_8004FE34[sprite_indices[arg0]];
                *(u32*)&sprite->x0 = D_8004FC74[base_index + cur_x];
                sprite->u0 = 0xD0;
                sprite->v0 = 0;
                sprite->r0 = sprite->g0 = sprite->b0 = (u8)D_8013B268;
                sprite->clut = 0x6AAE;
                if (D_8013B268 < 0x40)
                {
                    setSemiTrans(sprite, 1);
                }
                addPrim(&D_801398EC->ot_entry, sprite);
                if (D_800D921C < 0x7D00)
                {
                    D_800D921C += sizeof(SPRT);
                    D_801398EC->prim_cursor++;
                }
            }

            sprite = D_801398EC->prim_cursor;
            *sprite = D_8004FE34[sprite_indices[arg0]];
            *(u32*)&sprite->x0 = D_8004FC74[base_index + cur_x];
            sprite->r0 = sprite->g0 = sprite->b0 = (u8)D_8013B268;
            sprite->clut = getClut(0x2E0, arg0 + 0x1A0);
            if (D_8013B268 < 0x40)
            {
                setSemiTrans(sprite, 1);
            }
            addPrim(&D_801398EC->ot_entry, sprite);
            if (D_800D921C < 0x7D00)
            {
                D_800D921C += sizeof(SPRT);
                D_801398EC->prim_cursor++;
            }

            cur_x++;
        } while (cur_x < 6);

        cur_y++;
        base_index += 6;
    } while (cur_y < 6);

    func_8006534C(0x3D, 1);
}
