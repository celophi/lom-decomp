#include "common.h"
#include "sdk/libgpu.h"

typedef struct
{
    s32 value;
    u8 pad[0x24];
} WmapCell;

typedef struct
{
    s32 x;
    s32 y;
} WmapPosition;

typedef struct
{
    u8 pad0[0x74];
    u_long ot_entry;
    u8 pad78[0x33C - 0x78];
    SPRT *prim_cursor;
} WmapRenderContext;

extern SPRT D_800C45B0;
extern SPRT D_800C45C4;
extern SPRT D_800C45D8;
extern SPRT D_800C45EC;
extern s32 D_800D7CC0;
extern s32 D_800D7CC4;
extern s32 D_800D7CC8;
extern s32 D_800D7CCC;
extern s32 D_800D921C;
extern s32 D_800D9220;
extern s32 D_800DBE78;
extern s32 D_800DCEC0;
extern s32 D_8011CF18;
extern s32 D_8011CF7C;
extern s32 D_8011D4FC;
extern s32 D_8013922C;
extern s32 D_80139230;
extern WmapCell D_80139290[][6];
extern s32 D_8013986C;
extern s32 D_80139880;
extern s32 D_801398C0;
extern WmapRenderContext *D_801398EC;
extern WmapPosition D_80139950;
extern s32 D_8013B258;
extern s32 D_8013B25C;
extern s32 D_8013B268;
extern s32 D_8013B298;
extern s32 D_80182230;
extern s32 D_80182238;
extern s32 D_801ADAEC;

extern void func_800551A8(void);
extern void func_8005536C(void);
extern void func_8005556C(void);
extern void func_800581A0(s32 x, s32 y, s32 value);
extern void func_80064F64(s32 id);
extern void func_8006534C(s16 x, s32 ot_index);

/**
 * @brief Update world-map state and draw the local map and status sprites.
 */
void func_80054A2C(void)
{
    s32 timer;
    s32 valid;
    s32 scan_x;
    s32 scan_y;
    s32 render_x;
    s32 render_y;
    SPRT *sprite;

    if (D_8013B25C != 0)
    {
        if ((D_801398C0 & 4) != 0)
        {
            timer = D_8013B25C - 1;
            D_8013B25C = timer;
            if (timer == 0)
            {
                valid = 1;
                D_800D7CC8 = D_80139950.y / 48;
                D_800D7CC4 = D_80139950.x / 48;

                for (scan_y = D_800D7CC8; scan_y < D_800D7CC8 + 3; scan_y++)
                {
                    for (scan_x = D_800D7CC4; scan_x < D_800D7CC4 + 3; scan_x++)
                    {
                        if (D_80139290[scan_x][scan_y].value == 0xFF)
                        {
                            valid = 0;
                        }
                    }
                }

                if ((D_800DBE78 != 0) || (D_8013986C != 0) || (D_8011CF18 != 0) || (D_80139230 != 0) || (D_8011D4FC != -1))
                {
                    valid = 0;
                }

                if (valid == 0)
                {
                    D_8013B25C = 60;
                    return;
                }

                D_800DCEC0 = 0;
                func_80064F64(0x1154);
                D_80182238 = 0;
                D_800D7CC0 = 0;
                D_8011CF7C = 0;
                D_80139880 = 0;
                D_80182230 = 0x708;
                D_800DBE78 = 1;
                D_801ADAEC = 0;
                D_8013B268 = 0;
                D_8013B258 = 1;
                D_8013B298 = 0;
                D_800D7CCC = 15;
                D_800D9220 = 0xF0E0;
            }
        }
        else
        {
            D_8013B25C = 60;
        }
        return;
    }

    switch (D_80139880)
    {
    case 0:
        func_800551A8();
        break;
    case 1:
        func_8005536C();
        break;
    case 2:
        func_8005556C();
        break;
    }

    for (render_y = D_800D7CC8; render_y < D_800D7CC8 + 3; render_y++)
    {
        for (render_x = D_800D7CC4; render_x < D_800D7CC4 + 3; render_x++)
        {
            func_800581A0(render_x, render_y, D_80139290[render_x][render_y].value);
        }
    }

    render_y = D_800D7CC0;
    render_x = 0xF0;
    if (D_80182238 == 0)
    {
        sprite = D_801398EC->prim_cursor;
        *sprite = D_800C45C4;
        addPrim(&D_801398EC->ot_entry, sprite);
        if (D_800D921C < 0x7D00)
        {
            D_800D921C += sizeof(SPRT);
            D_801398EC->prim_cursor++;
        }
    }
    else if (render_y != 0)
    {
        do
        {
            sprite = D_801398EC->prim_cursor;
            *sprite = D_800C45C4;
            sprite->x0 = render_x;
            sprite->u0 = (render_y % 10) * 16;
            render_y /= 10;
            addPrim(&D_801398EC->ot_entry, sprite);
            if (D_800D921C < 0x7D00)
            {
                D_800D921C += sizeof(SPRT);
                D_801398EC->prim_cursor++;
            }
            render_x -= 16;
        } while (render_y != 0);
    }

    sprite = D_801398EC->prim_cursor;
    *sprite = D_800C45B0;
    addPrim(&D_801398EC->ot_entry, sprite);
    if (D_800D921C < 0x7D00)
    {
        D_800D921C += sizeof(SPRT);
        D_801398EC->prim_cursor++;
    }

    sprite = D_801398EC->prim_cursor;
    *sprite = D_800C45EC;
    sprite->u0 = D_8013B298 * 16;
    addPrim(&D_801398EC->ot_entry, sprite);
    if (D_800D921C < 0x7D00)
    {
        D_800D921C += sizeof(SPRT);
        D_801398EC->prim_cursor++;
    }

    sprite = D_801398EC->prim_cursor;
    *sprite = D_800C45D8;
    addPrim(&D_801398EC->ot_entry, sprite);
    if (D_800D921C < 0x7D00)
    {
        D_800D921C += sizeof(SPRT);
        D_801398EC->prim_cursor++;
    }

    func_8006534C(0x55, 1);

    if ((D_800D7CC0 - D_80182238) >= 0)
    {
        if ((D_800D7CC0 - D_80182238) < 30)
        {
            goto move_small;
        }
        goto move_large;
    }

    if ((D_80182238 - D_800D7CC0) < 30)
    {
move_small:
        if (D_800D7CC0 < D_80182238)
        {
            D_800D7CC0++;
        }
        if (D_80182238 < D_800D7CC0)
        {
            D_800D7CC0--;
        }
    }
    else
    {
move_large:
        if (D_800D7CC0 < D_80182238)
        {
            D_800D7CC0 += 15;
        }
        if (D_80182238 < D_800D7CC0)
        {
            D_800D7CC0 -= 15;
        }
    }

    D_801398C0 = 0;
    D_8013922C = 0;
}
