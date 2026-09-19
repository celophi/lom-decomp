#include "common.h"
#include "display.h"
#include "gpu_packet.h"
#include "pad.h"
#include "scene_state.h"
#include "sdk/libgpu.h"

#define WSEL_FADE_NEUTRAL 0x100
#define WSEL_FADE_ADDITIVE_THRESHOLD (WSEL_FADE_NEUTRAL + 1)
#define WSEL_FADE_ADDITIVE_DRAW_MODE 0x25
#define WSEL_FADE_SUBTRACTIVE_DRAW_MODE 0x45
#define WSEL_SCENE_STATE_ADDRESS 0x801ED480
#define WSEL_NEXT_FADE_PRIMITIVE(primitive, type) ((WselFadePrimitive*)((u8*)(primitive) + sizeof(type)))
#define M2C_FIELD(base, type, off) (*(type)((u8*)(base) + (off)))
#define W32(p, o) (*(u32*)((p) + (o)))
#define H16(p, o) (*(s16*)((p) + (o)))
#define U16(p, o) (*(u16*)((p) + (o)))
#define WSEL_STATE_BYTES ((u8*)D_800C6720)
#define WSEL_RECT_POINTS(rect) ((WselPoint*)&(rect))

typedef struct
{
    s16 x;
    s16 y;
} WselPoint;

typedef struct
{
    s16 x0;
    s16 y0;
    s16 x1;
    s16 y1;
} WselRect4;

typedef struct
{
    s16 x0, y0;
    s16 x1, y1;
    s16 x2, y2;
    s16 x3, y3;
} WselQuadCoords;

typedef struct
{
    u8 tp;
    u8 abr;
    u8 semi;
    u8 color;
    u16 tx;
    u16 ty;
    u16 clut_x;
    u16 clut_y;
    u16 u;
    u16 v;
    u16 w;
    u16 h;
    u16 x;
    u16 y;
} WselSpriteEntry;

typedef struct
{
    u8 _pad0[4];
    u16 x1;
    u16 y1;
    u16 x2;
    u16 y2;
    u8 _pad1[0x18 - 0xC];
} WselTexEntry;

typedef struct
{
    s32 red;
    s32 green;
    s32 blue;
} WselFadeCurrent;

typedef struct
{
    s32 red;
    s32 green;
    s32 blue;
    s32 steps;
} WselFadeTarget;

typedef struct
{
    char _pad[0x40];
    u_long otag_buffer[0x1000];
    DISPENV disp_env;
    DRAWENV draw_env;
    char _pad2[8];
    u_long prim_buffer[0x1000];
    u_long* next_prim_ptr;
} WselMenuContext;

typedef union
{
    TILE tile;
    DR_TPAGE draw_mode;
} WselFadePrimitive;

typedef struct
{
    u8 _pad0[0x4040];
    DISPENV disp_env;
    DRAWENV draw_env;
    u8 _pad1[0x80CC - 0x40B0];
} WselRenderHalf;

typedef struct
{
    u8 _pad0[0x406A];
    u8 front_draw_dither;
    u8 _pad1[0x40B0 - 0x406B];
    s16 front_display_x;
    s16 front_display_y;
    s16 front_display_width;
    s16 front_display_height;
    u8 _pad2[0xC136 - 0x40B8];
    u8 back_draw_dither;
    u8 _pad3[0xC17C - 0xC137];
    s16 back_display_x;
    s16 back_display_y;
    s16 back_display_width;
    s16 back_display_height;
} WselRenderLayout;

extern s32 D_80042FB4;
extern u8 D_80042FD8[];
extern u32 D_80043000;
extern u8 D_800435E0;
extern u8 D_80052608[];
extern u8 D_8007CC2C[];
extern u8 D_8008E650[];
extern u8 D_8009023C[];
extern u8 D_80098E80[];
extern u8 D_800A90A4[];
extern u8 D_800B92C8[];
extern u8 D_800C130C[];
extern u8 D_800C32F0[];
extern u8 D_800C345C[];
extern u8 D_800C3480[];
extern u8 D_800C3708[];
extern WselTexEntry D_800C6720[];
extern WselSpriteEntry D_800C6780;
extern WselSpriteEntry D_800C6798;
extern WselSpriteEntry D_800C67B0;
extern WselSpriteEntry D_800C67E0;
extern WselSpriteEntry D_800C6828;
extern u8* D_800C6870;
extern u8 D_800C6878;
extern WselFadeTarget D_800CA878;
extern WselFadeCurrent D_800CA888;
extern s32 D_800CA898;
extern s32 D_800CA89C;
extern s32 D_800CA8A0;
extern s32 D_800CA8A4;
extern WselRect4 D_800CA8A8;
extern s32 D_800CA8B0;
extern s32 D_800CA8B4;
extern WselPoint D_800CA8B8;
extern WselPoint D_800CA8BC;
extern WselRect4 D_800CA8C0;
extern WselPoint D_800CA8C8;
extern WselPoint D_800CA8CC;
extern s16 D_800CA8CE;
extern s32 D_800CA8D0;
extern s32 D_800CA8D4;
extern s32 D_800CA8D8;
extern s32 D_800CA8DC;
extern s32 D_800CA8E0;
extern u8 D_801ED600[];
extern void* jtbl_8004FC74[];

extern void func_800122C0(void);
extern void func_80013F2C(void);
extern void func_800141EC(u16, void*);
extern void func_800157B0(s32);
extern void func_800157DC(void);
extern void func_800158E0(void);
extern void func_80016E7C(const void*, void*, s32);
extern void func_800196F0(s32);
extern void func_80019788(s32);
extern void func_8001990C(void*, s32, s32, s32);
extern int func_80019A34(RECT*, u_long*);
extern void func_80019C74(void*, s32);
extern void func_80019D7C(void*);
extern void func_80019DEC(void*);
extern void func_80019FB8(void*);
extern void func_8001A5D4(void*, void*);
extern void func_8001C56C(void*, s32, s32, s32, s32);
extern void func_8001C62C(void*, s32, s32, s32, s32);
extern void func_8001D58C(s32, s32);
extern void func_8001D5AC(s32);
extern s32 func_8002054C(s32);
extern void func_80022040(void*);
extern void func_80022068(s32);
extern void func_8002216C(s32, s32, s32, s32);
extern void func_8002279C(s32, s32);
extern void func_80022AE8(void*, s32);

s32 func_8004FC8C();
void func_8004FD24();
void func_8004FE78();
void func_8004FFBC();
void func_80050030();
void func_80050050();
void func_80050080();
void func_800500A8();
void func_800500D8();
void func_800503D4();
void func_800503F0();
void func_80050944();
void* func_80050B40();
void* func_80050DB0();
void* func_80050F0C();
void* func_800513D0();
void* func_800514D8();
void func_800517BC();
POLY_FT4* func_80051D78();
void func_800520A8();
void func_80052154();
void func_800521D0();
s32 func_800522AC();
void func_80052384();
void func_80052510();

/**
 * @brief Initialize and run the WSEL overlay until an exit state is selected.
 * @param arg Context forwarded to the WSEL initialization and frame loop.
 * @return Selected WSEL exit state.
 */
s32 func_8004FC8C(void* arg)
{
    S_801ED480* scene_state = (S_801ED480*)WSEL_SCENE_STATE_ADDRESS;
    void* initial_context = arg;
    void* context;

    D_800C6870 = (context = initial_context);
    D_800CA898 = 0;
    func_8004FE78();

    scene_state->map_id = 0;
    scene_state->object_index = 0;
    scene_state->unk4 = 0;
    scene_state->unk8 = 0;
    scene_state->unkC = 0;

    do
    {
        func_8004FD24(context);
    } while (D_800CA89C == 0);

    D_80042FB4 = func_8002054C(-1);
    return D_800CA89C;
}

void func_8004FD24(void* arg)
{
    void* cur;
    u_long* ot;
    RECT rect;

    cur = arg;
    func_80019C74((u8*)arg + 0x40, 0x1000);
    func_80019C74((u8*)arg + 0x810C, 0x1000);
    func_8002054C(0);
    func_80019FB8((u8*)arg + 0x4040);
    func_800157DC();
    func_800196F0(1);
    do
    {
        ot = (u_long*)((u8*)cur + 0x40);
        func_80019C74(ot, 0x1000);
        *(u32*)((u8*)cur + 0x80B8) = (u32)((u8*)cur + 0x40B8);
        func_8002054C(1);
        func_800500D8(cur);
        func_800503F0(cur);
        func_800517BC();
        func_80019788(0);
        func_800157B0(2);
        func_8002054C(2);
        func_8001990C((u8*)cur + 0x40B0, 0, 0, 0);
        if (cur == arg)
        {
            cur = (u8*)cur + 0x80CC;
            D_800CA898 = 1;
        }
        else
        {
            cur = arg;
            D_800CA898 = 0;
        }
        func_80019FB8((u8*)cur + 0x4040);
        func_80019DEC((u8*)cur + 0x4054);
        func_80019D7C(ot + 0xFFF);
        func_800157DC();
        func_800122C0();
    } while (D_800CA89C == 0);
    func_800158E0();
    func_8002054C(0);
}

void func_8004FE78(void* arg)
{
    WselRenderHalf* ctx = (WselRenderHalf*)arg;
    WselRenderLayout* layout = (WselRenderLayout*)arg;
    RECT vram_rect;

    func_8001D5AC(0x5DC);
    func_8001D58C(0xA0, 0x78);

    layout->front_display_x = 0;
    layout->front_display_y = 0;
    layout->front_display_width = 0x140;
    layout->front_display_height = 0xF0;
    layout->back_display_y = 0xE8;
    layout->back_display_x = 0;
    layout->back_display_width = 0x140;
    layout->back_display_height = 0xF0;

    vram_rect.x = 0;
    vram_rect.y = 0;
    vram_rect.w = 0x400;
    vram_rect.h = 0x200;
    func_8001990C(&vram_rect, 0, 0, 0);

    func_800500A8();
    func_800503D4(0x100, 0x100, 0x100, 0x14);
    func_8001C62C(&ctx->disp_env, 0, 0, 0x140, 0xF0);
    func_8001C62C(&(ctx + 1)->disp_env, 0, 0xE8, 0x140, 0xF0);
    func_8001C56C(&ctx->draw_env, 0, 0xF0, 0x140, 0xE0);
    func_8001C56C(&(ctx + 1)->draw_env, 0, 0x8, 0x140, 0xE0);

    layout->back_draw_dither = 0;
    layout->front_draw_dither = 0;
    func_800520A8();
    D_800CA89C = 0;
}

void func_8004FFBC(s32 seq_variant)
{
    u32* off;
    u8* base;

    func_800141EC((seq_variant + 0x17) & 0xFFFF, (void*)0x80180000);
    func_80013F2C();

    off = (u32*)0x80180004;
    base = (u8*)0x80180000;

    func_80016E7C(base + off[0], (u8*)&D_800C6878, (s32)(off[1] - off[0]));
    func_80022AE8(base + off[1], 1);
}

/** @see TITLE stop_title_music */
void func_80050030(void)
{
    func_80022068(0);
}

/** @see TITLE start_title_music */
void func_80050050(void)
{
    func_80022040((void*)&D_800C6878);
    func_8002279C(0, 0x7F);
}

/** @see TITLE play_title_sfx */
void func_80050080(s32 sound_id, s32 pan)
{
    func_8002216C(sound_id, 0, pan, 0x7F);
}

/** @see TITLE reset_fade_state */
void func_800500A8(void)
{
    D_800CA888.red = 0;
    D_800CA888.green = 0;
    D_800CA888.blue = 0;

    D_800CA878.red = 0;
    D_800CA878.green = 0;
    D_800CA878.blue = 0;
    D_800CA878.steps = 0;
}

/** @see TITLE render_fade_overlay */
void func_800500D8(WselMenuContext* ctx)
{
    WselMenuContext* base = ctx;
    WselFadePrimitive* primitive = (WselFadePrimitive*)base->next_prim_ptr;
    u_long* ordering_table_tag = base->otag_buffer;
    s32 red_step;
    s32 green_step;
    s32 blue_step;
    s32 draw_mode;

    if (D_800CA878.steps != 0)
    {
        red_step = (D_800CA878.red - D_800CA888.red) / D_800CA878.steps;
        green_step = (D_800CA878.green - D_800CA888.green) / D_800CA878.steps;
        blue_step = (D_800CA878.blue - D_800CA888.blue) / D_800CA878.steps;
        D_800CA878.steps = D_800CA878.steps - 1;
        D_800CA888.red = D_800CA888.red + red_step;
        D_800CA888.green = D_800CA888.green + green_step;
        D_800CA888.blue = D_800CA888.blue + blue_step;
    }
    else
    {
        D_800CA888.red = D_800CA878.red;
        D_800CA888.green = D_800CA878.green;
        D_800CA888.blue = D_800CA878.blue;
    }
    if (!(((D_800CA888.red == WSEL_FADE_NEUTRAL) && (D_800CA888.green == WSEL_FADE_NEUTRAL)) &&
          (D_800CA888.blue == WSEL_FADE_NEUTRAL)))
    {
        if (D_800CA888.red >= WSEL_FADE_ADDITIVE_THRESHOLD)
        {
            primitive->tile.r0 = D_800CA888.red - 1;
            primitive->tile.g0 = D_800CA888.green - 1;
            primitive->tile.b0 = D_800CA888.blue - 1;
        }
        else
        {
            if (D_800CA888.red == WSEL_FADE_NEUTRAL)
            {
                primitive->tile.r0 = 0;
            }
            else
            {
                primitive->tile.r0 = ~D_800CA888.red;
            }
            if (D_800CA888.green == WSEL_FADE_NEUTRAL)
            {
                primitive->tile.g0 = 0;
            }
            else
            {
                primitive->tile.g0 = ~D_800CA888.green;
            }
            if (D_800CA888.blue == WSEL_FADE_NEUTRAL)
            {
                primitive->tile.b0 = 0;
            }
            else
            {
                primitive->tile.b0 = ~D_800CA888.blue;
            }
        }

        setTile(&primitive->tile);
        setSemiTrans(&primitive->tile, 1);
        SET_YX0(&primitive->tile, 0, 0);
        setWH(&primitive->tile, SCREEN_WIDTH, SCREEN_HEIGHT);
        addPrim(ordering_table_tag, &primitive->tile);

        draw_mode = WSEL_FADE_ADDITIVE_DRAW_MODE;
        primitive = WSEL_NEXT_FADE_PRIMITIVE(primitive, TILE);
        if (D_800CA888.red < WSEL_FADE_ADDITIVE_THRESHOLD)
        {
            draw_mode = WSEL_FADE_SUBTRACTIVE_DRAW_MODE;
        }
        setDrawTPage(&primitive->draw_mode, 0, 0, draw_mode);
        addPrim(ordering_table_tag, &primitive->draw_mode);

        primitive = WSEL_NEXT_FADE_PRIMITIVE(primitive, DR_TPAGE);
    }
    base->next_prim_ptr = (u_long*)primitive;
}

/** @see TITLE set_fade_target */
void func_800503D4(s32 red, s32 green, s32 blue, s32 steps)
{
    D_800CA878.red = red;
    D_800CA878.green = green;
    D_800CA878.blue = blue;
    D_800CA878.steps = steps;
}

void func_800503F0(void *arg0)
{
    WselQuadCoords q;
    s32 i;
    s32 state;
    u8 *prim;
    s32 *ot;

    static void *const keep[] = { &&case0, &&case1, &&case2, &&case3, &&case4, &&case5 };

    prim = *(u8 **)((u8 *)arg0 + 0x80B8);
    ot = (s32 *)((u8 *)arg0 + 0x40);

    state = D_800CA8D8;
    if ((u32)state < 6) {
        goto *jtbl_8004FC74[state];
    }
    goto finish;

case1:
        {
            s32 n = D_800CA8B4;
            if (n != 0) {
                u8 *state;
                n--;
                D_800CA8B4 = n;
                state = WSEL_STATE_BYTES;
                state[0x33] = state[3] = (0x10 - n) << 3;
                if (n == 0) {
                    state[2] = 0;
                    state[0x32] = 1;
                } else {
                    state[2] = 1;
                    state[1] = 1;
                    state[0x32] = 1;
                    state[0x31] = 1;
                }
            }
        }
        prim = func_800514D8(prim, ot, 2);
        prim = func_80050F0C(prim, (u_long *)ot);
        if (D_800CA8B4 != 0) {
            q.x0 = q.x2 = WSEL_RECT_POINTS(D_800CA8A8)[0].x;
            q.x1 = q.x3 = WSEL_RECT_POINTS(D_800CA8A8)[1].x;
            q.y0 = q.y1 = WSEL_RECT_POINTS(D_800CA8A8)[0].y;
            q.y2 = q.y3 = WSEL_RECT_POINTS(D_800CA8A8)[1].y;
            prim = func_80050DB0((POLY_FT4 *)prim, (u_long *)ot, &q, 1, D_800CA8B4 << 3);
        }
        prim = func_800514D8(prim, ot, 0);
        func_80050944();
        goto finish;

case2:
        for (i = 0; i < 2; i++) {
            WSEL_RECT_POINTS(D_800CA8A8)[i].x = (u16)WSEL_RECT_POINTS(D_800CA8A8)[i].x + (WSEL_RECT_POINTS(D_800CA8C0)[i].x - WSEL_RECT_POINTS(D_800CA8A8)[i].x) / D_800CA8B4;
            WSEL_RECT_POINTS(D_800CA8A8)[i].y = (u16)WSEL_RECT_POINTS(D_800CA8A8)[i].y + (WSEL_RECT_POINTS(D_800CA8C0)[i].y - WSEL_RECT_POINTS(D_800CA8A8)[i].y) / D_800CA8B4;
        }
        q.x0 = q.x2 = WSEL_RECT_POINTS(D_800CA8A8)[0].x;
        q.x1 = q.x3 = WSEL_RECT_POINTS(D_800CA8A8)[1].x;
        q.y0 = q.y1 = WSEL_RECT_POINTS(D_800CA8A8)[0].y;
        q.y2 = q.y3 = WSEL_RECT_POINTS(D_800CA8A8)[1].y;
        prim = func_80050DB0((POLY_FT4 *)prim, (u_long *)ot, &q, 0, 0x80);
        if (--D_800CA8B4 == 0) {
            D_800CA8D8 = 5;
            D_800CA8B4 = 0x10;
        }
        goto draw_tail;

case3:
        prim = func_800514D8(prim, ot, 2);
        prim = func_80050F0C(prim, (u_long *)ot);
        prim = func_80050B40(prim, ot);
        prim = func_800514D8(prim, ot, 0);
        func_80050944();
        goto finish;

case4:
        for (i = 0; i < 2; i++) {
            WSEL_RECT_POINTS(D_800CA8A8)[i].x = (u16)WSEL_RECT_POINTS(D_800CA8A8)[i].x + (WSEL_RECT_POINTS(D_800CA8C0)[i].x - WSEL_RECT_POINTS(D_800CA8A8)[i].x) / D_800CA8B4;
            WSEL_RECT_POINTS(D_800CA8A8)[i].y = (u16)WSEL_RECT_POINTS(D_800CA8A8)[i].y + (WSEL_RECT_POINTS(D_800CA8C0)[i].y - WSEL_RECT_POINTS(D_800CA8A8)[i].y) / D_800CA8B4;
        }
        q.x0 = q.x2 = WSEL_RECT_POINTS(D_800CA8A8)[0].x;
        q.x1 = q.x3 = WSEL_RECT_POINTS(D_800CA8A8)[1].x;
        q.y0 = q.y1 = WSEL_RECT_POINTS(D_800CA8A8)[0].y;
        q.y2 = q.y3 = WSEL_RECT_POINTS(D_800CA8A8)[1].y;
        prim = func_80050DB0((POLY_FT4 *)prim, (u_long *)ot, &q, 0, 0x80);
        if (--D_800CA8B4 == 0) {
            D_800CA8D8 = 0;
            D_800CA8B4 = 0;
        }
        goto draw_tail;

case5:
        for (i = 0; i < 2; i++) {
            WSEL_RECT_POINTS(D_800CA8A8)[i].x = WSEL_RECT_POINTS(D_800CA8C0)[i].x;
            WSEL_RECT_POINTS(D_800CA8A8)[i].y = WSEL_RECT_POINTS(D_800CA8C0)[i].y;
        }
        q.x0 = q.x2 = WSEL_RECT_POINTS(D_800CA8A8)[0].x;
        q.x1 = q.x3 = WSEL_RECT_POINTS(D_800CA8A8)[1].x;
        q.y0 = q.y1 = WSEL_RECT_POINTS(D_800CA8A8)[0].y;
        q.y2 = q.y3 = WSEL_RECT_POINTS(D_800CA8A8)[1].y;
        prim = func_800514D8(prim, ot, 7);
        prim = func_80050DB0((POLY_FT4 *)prim, (u_long *)ot, &q, 0, 0x80);
        if (D_800CA8B0 & 0x220) {
            func_80050080(0x7E, 0x80);
            D_800CA8D8 = 1;
            D_800CA8B4 = 0x10;
        }
        goto draw_tail;

case0:
draw_tail:
        prim = func_800514D8(prim, ot, 3);
        prim = (u8 *)func_80051D78((POLY_FT4 *)prim, (u_long *)ot, D_800435E0 & 0x7F);
        prim = func_800514D8(prim, ot, 1);

finish:
    *(u8 **)((u8 *)arg0 + 0x80B8) = prim;
}

void func_80050944(void)
{
    s32 x_step0, y_step0, x_step1, y_step1;
    if (D_800CA8DC != 0) {
        WselPoint *current = &D_800CA8B8;
        x_step0 = (D_800CA8C8.x - current->x) / D_800CA8DC;
        y_step0 = (D_800CA8C8.y - current->y) / D_800CA8DC;
        D_800CA8DC -= 1;
        current->x += x_step0;
        current->y += y_step0;
    } else { D_800CA8B8.x = D_800CA8C8.x; D_800CA8B8.y = D_800CA8C8.y; }
    if (D_800CA8E0 != 0) {
        WselPoint *current = &D_800CA8BC;
        x_step1 = (D_800CA8CC.x - current->x) / D_800CA8E0;
        y_step1 = (D_800CA8CC.y - current->y) / D_800CA8E0;
        D_800CA8E0 -= 1;
        current->x += x_step1;
        current->y += y_step1;
    } else { D_800CA8BC.x = D_800CA8CC.x; D_800CA8BC.y = D_800CA8CC.y; }
    { u8 *table = WSEL_STATE_BYTES; *(s16 *)(table + 0x44) = D_800CA8BC.x - 0xB; *(s16 *)(table + 0x46) = D_800CA8BC.y - 0xB; *(u16 *)(table + 0x14) = D_800CA8B8.x; *(u16 *)(table + 0x16) = D_800CA8B8.y; }
}

void *func_80050B40(u8 *p, s32 *arg1)
{
    u8 *state;
    if (D_800CA8D0 < 0x40) D_800CA8D0 += 4;
    state = WSEL_STATE_BYTES;
    p[3]=3; p[7]=0x60; { u8 shade; shade=D_800CA8D0; H16(p,0xC)=0x140; H16(p,0xA)=0; H16(p,8)=0; p[6]=shade; p[5]=shade; p[4]=shade; p[7]|=2; }
    H16(p,0xE)=U16(state,0x46)+0xB;
    addPrim(arg1, p); p+=0x10;
    p[7]=0x60; p[3]=3; {u8 shade=D_800CA8D0; p[6]=shade; p[5]=shade; p[4]=shade;} H16(p,8)=0; p[7]|=2;
    H16(p,0xA)=U16(state,0x46)+0x6B; H16(p,0xC)=0x140; H16(p,0xE)=0xE0-H16(p,0xA);
    addPrim(arg1, p); p+=0x10;
    p[7]=0x60; p[3]=3; {u8 shade=D_800CA8D0; p[6]=shade; p[5]=shade; p[4]=shade;} H16(p,8)=0; p[7]|=2;
    H16(p,0xA)=U16(state,0x46)+0xB; H16(p,0xC)=U16(state,0x44)+0xB; H16(p,0xE)=0x60;
    addPrim(arg1, p); p+=0x10;
    p[7]=0x60; p[3]=3; {u8 shade=D_800CA8D0; p[6]=shade; p[5]=shade; p[4]=shade;}  p[7]|=2;
    H16(p,8)=U16(state,0x44)+0x6B; H16(p,0xA)=U16(state,0x46)+0xB; H16(p,0xC)=0x140-U16(p,8); H16(p,0xE)=0x60;
    addPrim(arg1, p); p+=0x10;
    p[3]=1; W32(p,4)=0xE1000040;
    addPrim(arg1, p);
    return p+8;
}

void *func_80050DB0(POLY_FT4 *poly, u_long *ot, WselQuadCoords *coords, s32 semi, s32 color)
{
    setPolyFT4(poly);
    poly->b0 = color;
    poly->g0 = color;
    poly->r0 = color;
    setSemiTrans(poly, semi);

    poly->x0 = coords->x0;
    poly->x1 = coords->x1;
    poly->x2 = coords->x2;
    poly->x3 = coords->x3;
    poly->y0 = coords->y0;
    poly->y1 = coords->y1;
    poly->y2 = coords->y2;
    poly->y3 = coords->y3;

    poly->u2 = 0x60;
    poly->u0 = 0x60;
    poly->u3 = 0xE0;
    poly->u1 = 0xE0;
    poly->v1 = 0x30;
    poly->v0 = 0x30;
    poly->v3 = 0xA8;
    poly->v2 = 0xA8;

    setClut(poly, D_800C6720[1].x2, D_800C6720[1].y2);
    setTPage(poly, 1, 1, D_800C6720[1].x1, D_800C6720[1].y1);
    addPrim(ot, poly);
    return poly + 1;
}

void *func_80050F0C(void *prim, u_long *ot)
{
    DRAWENV draw_env;
    s32 col0;
    s32 cell_index;
    s32 dx;
    s32 dy;
    s32 qx;
    s32 qy;
    s32 qy2;
    s32 remx;
    s32 remy;
    s32 row;
    s32 col;
    volatile s32 saved_index;
    u8 *p = prim;

    dx = -D_800CA8B8.x;
    dx += D_800CA8BC.x;
    qx = dx - 0x10;
    if (qx < 0)
        qx = dx - 1;
    qx >>= 4;
    col0 = qx;

    dy = D_800CA8B8.y * -1 + D_800CA8BC.y;
    {
        s32 yoff = dy - 0x10;
        if (yoff < 0)
            yoff = dy - 1;
        qy = yoff >> 4;
    }

    {
        u8 *occupancy = D_800C32F0;
        cell_index = qy * 0x13;
        if (occupancy[cell_index + col0] != 0) {
        for (row = 0; row < 6; row++) {
            for (col = 0; col < 6; col++) {
                p = func_800513D0(p, ot,
                    *(u16 *)(WSEL_STATE_BYTES + 0x44) + col * 0x10 + 0xB,
                    *(u16 *)(WSEL_STATE_BYTES + 0x46) + row * 0x10 + 0xB,
                    0xA0);
            }
        }
        goto done;
        }
    }

    func_8001A5D4(p, D_800C6870 + ((D_800CA898 ^ 1) * 0x80CC) + 0x4054);
    addPrim(ot, p);
    p += 0x40;

    dx = -D_800CA8B8.x;
    dx += D_800CA8BC.x;
    remx = dx - 0x10;
    qx = remx;
    if (remx < 0)
        qx = dx - 1;
    remx -= (qx >> 4) * 0x10;

    dy = D_800CA8B8.y * -1 + D_800CA8BC.y;
    remy = dy - 0x10;
    qy2 = remy;
    if (remy < 0)
        qy2 = dy - 1;
    remy -= (qy2 >> 4) * 0x10;

    if (D_800CA8B0 & 0x10) {
        {
            s32 pos = 0;
            saved_index = cell_index;
            for (row = 0; row < 6; row++) {
                s32 base_cell = saved_index + col0;
                u8 * const grid = D_800C345C;
                for (col = 0; col < 6; col++) {
                    s32 off = base_cell << 3;
                    u8 *cellp;
                    off = (off + base_cell) << 2;
                    do {
                        do {
                            do {
                                cellp = grid + off;
                            } while (0);
                        } while (0);
                    } while (0);
                    if (cellp[pos + col] == 0) {
                        p = func_800513D0(p, ot, col * 0x10 - remx,
                                          row * 0x10 - remy, 0x130);
                    }
                }
                pos += 6;
            }
        }

        for (col = 0; col < 6; col++) {
            if (D_800C3708[(qy * 0x13 + col0) * 0x24 + col + 0x1E] == 0) {
                p = func_800513D0(p, ot, col * 0x10 - remx,
                                  0x60 - remy, 0x130);
            }
        }

        for (row = 0; row < 6; row++) {
            if (D_800C3480[(qy * 0x13 + col0) * 0x24 + row * 6 + 5] == 0) {
                p = func_800513D0(p, ot, 0x60 - remx,
                                  row * 0x10 - remy, 0x130);
            }
        }

        {
            u8 *grid2 = D_800C345C;
            s32 corner_index = (qy + 1) * 0x13 + 1;
            if (grid2[(corner_index + col0) * 0x24 + 0x23] == 0) {
                p = func_800513D0(p, ot, 0x60 - remx, 0x60 - remy, 0x130);
            }
        }
    }

    {
        u8 *ctx = WSEL_STATE_BYTES;
        s32 draw_y_base = *(u16 *)(ctx + 0x46);
        s32 draw_x = *(u16 *)(ctx + 0x44) + 0xC;
        s32 draw_y = draw_y_base + 0x14;
        if (D_800CA898 != 0)
            draw_y = draw_y_base + 0xFC;
        func_8001C56C(&draw_env, draw_x, draw_y, 0x60, 0x60);
    }
    func_8001A5D4(p, &draw_env);
    addPrim(ot, p);
    p += 0x40;
done:
    return p;
}

void *func_800513D0(TILE *tile, u_long *ot, s32 x, s32 y, s32 intensity)
{
    DR_TPAGE *draw_mode;

    setTile(tile);
    if (intensity < 0x100)
    {
        tile->b0 = -intensity;
        tile->g0 = -intensity;
        tile->r0 = -intensity;
    }
    else
    {
        tile->b0 = intensity;
        tile->g0 = intensity;
        tile->r0 = intensity;
    }
    setXY0(tile, x, y);
    setWH(tile, 0x10, 0x10);
    setSemiTrans(tile, 1);
    addPrim(ot, tile);

    draw_mode = (DR_TPAGE *)(tile + 1);
    if (intensity < 0x100)
    {
        setDrawTPage(draw_mode, 0, 0, 0x40);
    }
    else
    {
        setDrawTPage(draw_mode, 0, 0, 0x20);
    }
    addPrim(ot, draw_mode);
    draw_mode++;
    return draw_mode;
}

void *func_800514D8(void *arg0, s32 *arg1, s32 arg2)
{
    s32 sp0;
    s32 temp_a1;
    s32 temp_a3;
    s32 temp_s0;
    s32 temp_v1;
    s32 var_s4;
    s32 var_v0_3;
    s32 var_a2;
    s32 var_fp;
    s32 var_s3;
    s32 var_s7;
    s32 var_t2;
    s32 var_t3;
    s32 var_t4;
    s32 var_t5;
    s32 var_t7;
    s32 var_t8;
    s32 var_t9;
    u8 var_s1;
    u8 var_v0_2;
    u32 mask24;
    void *temp_a0;
    void *temp_t1;
    void *var_a0;
    void *var_v0;

    var_a0 = arg0;
    temp_t1 = (arg2 * 0x18) + WSEL_STATE_BYTES;
    if (arg2 == 2) {
        var_s4 = 2;
        var_s7 = M2C_FIELD(temp_t1, u16 *, 0x14) - 1;
        var_fp = M2C_FIELD(temp_t1, u16 *, 0x16) - 1;
    } else {
        var_s4 = 1;
        var_s7 = M2C_FIELD(temp_t1, u16 *, 0x14);
        var_fp = M2C_FIELD(temp_t1, u16 *, 0x16);
    }
    var_s1 = M2C_FIELD(temp_t1, u8 *, 3);
    var_v0 = var_a0;
    if (var_s4 != 0) {
        do { do { do { do { do { do { do { do { mask24 = 0xFFFFFF; } while (0); } while (0); } while (0); } while (0); } while (0); } while (0); } while (0); } while (0);
        do {
            var_s3 = var_fp;
            var_t8 = 0x100;
            do { var_t7 = M2C_FIELD(temp_t1, u16 *, 0x12); } while (0);
            var_a2 = M2C_FIELD(temp_t1, u16 *, 6);
            sp0 = (s32)M2C_FIELD(temp_t1, u16 *, 0xE);
            if ((s32)var_t7 < 0x101) var_t8 = var_t7;
            do {
                var_t9 = var_s7;
                var_t2 = M2C_FIELD(temp_t1, u16 *, 0x10);
                var_t4 = M2C_FIELD(temp_t1, u16 *, 4);
                var_t5 = M2C_FIELD(temp_t1, u16 *, 0xC);
                var_t3 = 0x80;
                if ((s32)var_t2 < 0x81) var_t3 = var_t2;
                temp_s0 = (s32)(var_a2 & 0x100) >> 4;
                temp_a1 = (var_a2 & 0x200) * 4;
                do {
                    M2C_FIELD(var_a0, s8 *, 3) = 4;
                    M2C_FIELD(var_a0, u8 *, 7) = 0x64U;
                    M2C_FIELD(var_a0, u8 *, 6) = var_s1;
                    M2C_FIELD(var_a0, u8 *, 5) = var_s1;
                    M2C_FIELD(var_a0, u8 *, 4) = var_s1;
                    if (M2C_FIELD(temp_t1, u8 *, 2) != 0) {
                        var_v0_2 = M2C_FIELD(var_a0, u8 *, 7) | 2;
                    } else {
                        var_v0_2 = M2C_FIELD(var_a0, u8 *, 7) & 0xFD;
                    }
                    M2C_FIELD(var_a0, u8 *, 7) = var_v0_2;
                    M2C_FIELD(var_a0, u16 *, 8) = var_t9;
                    M2C_FIELD(var_a0, u16 *, 0xA) = var_s3;
                    M2C_FIELD(var_a0, s8 *, 0xC) = (s8)var_t5;
                    M2C_FIELD(var_a0, u8 *, 0xD) = (u8)sp0;
                    M2C_FIELD(var_a0, u16 *, 0x10) = var_t3;
                    M2C_FIELD(var_a0, u16 *, 0x12) = var_t8;
                    do {
                        M2C_FIELD(var_a0, s16 *, 0xE) = (s16)((M2C_FIELD(temp_t1, u16 *, 0xA) << 6) | (((u16)M2C_FIELD(temp_t1, u16 *, 8) >> 4) & 0x3F));
                    } while (0);
                    M2C_FIELD(var_a0, s32 *, 0) = (s32)((M2C_FIELD(var_a0, s32 *, 0) & 0xFF000000) | (*arg1 & mask24));
                    *arg1 = (*arg1 & 0xFF000000) | ((s32)var_a0 & mask24);
                    var_a0 += 0x14;
                    M2C_FIELD(var_a0, s8 *, 3) = 1;
                    temp_v1 = (M2C_FIELD(temp_t1, u8 *, 0) & 3) << 7;
                    temp_a3 = (s32)(var_t4 & 0x3FF) >> 6;
                    temp_a0 = var_a0;
                    if ((arg2 != 2) || (var_s4 != 1)) {
                        var_v0_3 = temp_v1 | ((M2C_FIELD(temp_t1, u8 *, 1) & 3) << 5) | temp_s0 | temp_a3 | temp_a1;
                        var_v0_3 |= 0xE1000000;
                    } else {
                        var_v0_3 = temp_v1 | temp_s0 | temp_a3 | temp_a1;
                        var_v0_3 |= 0xE1000000;
                    }
                    M2C_FIELD(var_a0, s32 *, 4) = var_v0_3;
                    var_a0 = temp_a0 + 8;
                    var_t2 -= var_t3;
                    M2C_FIELD(temp_a0, s32 *, 0) = (s32)((M2C_FIELD(temp_a0, s32 *, 0) & 0xFF000000) | (*arg1 & mask24));
                    *arg1 = (*arg1 & 0xFF000000) | ((s32)temp_a0 & mask24);
                    if (var_t2 == 0) break;
                    var_t5 ^= 0x80;
                    if (M2C_FIELD(temp_t1, u8 *, 0) == 0) {
                        var_t4 += 0x20;
                    } else {
                        var_t4 += 0x40;
                        var_t5 = 0;
                    }
                    var_t3 = 0x80;
                    if ((s32)var_t2 < 0x81) var_t3 = var_t2;
                    var_t9 += 0x80;
                } while (1);
                var_t7 -= var_t8;
                var_a2 += 0x100;
                if (var_t7 != 0) {
                    sp0 = 0;
                    var_t8 = 0x100;
                    if ((s32)var_t7 < 0x101) var_t8 = var_t7;
                    var_s3 += 0x100;
                }
            } while (var_t7 != 0);
            var_s7 += 2;
            var_fp += 2;
            var_s4 -= 1;
            var_s1 = 0;
        } while (var_s4 != 0);
        var_v0 = var_a0;
    }
    return var_v0;
}

void func_800517BC(void)
{
    s32 temp_a0;
    s32 var_a2;
    s32 temp_v0;
    s32 temp_v1;
    s32 var_a0;
    s32 var_a1;

    func_80052384();
    switch (D_800CA8D8) {
    case 0:
        if (D_800CA8D4 & 0x220) {
            func_80050080(0x7E, 0x80);
            func_80052154();
            D_800CA8B4 = 0x12;
            D_800CA8A8.x0 = 0x60;
            D_800CA8A8.x1 = 0xE0;
            D_800CA8C0.x0 = -0x2E;
            D_800CA8C0.x1 = 0x152;
            D_800CA8A8.y0 = 0x30;
            D_800CA8A8.y1 = 0xA8;
            D_800CA8C0.y0 = -0x5A;
            D_800CA8C0.y1 = 0x11E;
            D_800CA8D8 = 2;
            return;
        }
        if ((D_800CA8D4 & 0x40) && !((D_80043000 >> 3) & 1)) {
            func_80050080(0x7F, 0x80);
            D_800CA89C = 3;
        }
        return;

    case 1:
        if ((D_800CA8E0 == 0) && (D_800CA8DC == 0)) {
            if (D_800CA8D4 & 0x40) {
                func_80050080(0x7F, 0x80);
                D_800CA8B4 = 0x12;
                D_800CA8C0.x0 = 0x60;
                D_800CA8C0.x1 = 0xE0;
                D_800CA8A8.x0 = -0x2E;
                D_800CA8A8.x1 = 0x152;
                D_800CA8C0.y0 = 0x30;
                D_800CA8C0.y1 = 0xA8;
                D_800CA8A8.y0 = -0x5A;
                D_800CA8A8.y1 = 0x11E;
                D_800CA8D8 = 4;
                goto after_action;
            }
            if (D_800CA8D4 & 0x220) {
                u8 *state = WSEL_STATE_BYTES;
                D_800CA8B4 = 0;
                state[0x33] = 0x80;
                state[3] = 0x80;
                state[2] = 0;
                temp_a0 = -D_800CA8B8.x;
                temp_a0 += D_800CA8BC.x;
                var_a1 = temp_a0 - 0x10;
                state[0x32] = 1;
                if (var_a1 < 0) {
                    var_a1 = temp_a0 - 1;
                }
                temp_v1 = D_800CA8B8.y * -1 + D_800CA8BC.y;
                temp_v0 = temp_v1 - 0x10;
                if (temp_v0 < 0) {
                    temp_v0 = temp_v1 - 1;
                }
                var_a2 = var_a1 >> 4;
                var_a1 = temp_v0 >> 4;
                var_a0 = 0x78;
                if (D_800C32F0[var_a1 * 0x13 + var_a2] == 0) {
                    func_80050080(0x7E, 0x80);
                    D_800CA8D8 = 3;
                    D_800CA8D0 = 0;
                    return;
                }
                goto play_move_sound;
            }

after_action:
            if (D_800CA8B0 & 0x1000) {
                if (D_800CA8CE < 0x41) {
                    if (D_800CA8C8.y < 0) {
                        D_800CA8C8.y = (u16)D_800CA8C8.y + 0x10;
                        D_800CA8DC = 4;
                    } else {
                        goto up_second;
                    }
                } else {
up_second:
                    if (D_800CA8CC.y >= 0x11) {
                        D_800CA8CC.y = (u16)D_800CA8CC.y - 0x10;
                        D_800CA8E0 = 4;
                    }
                }
            }
            if (D_800CA8B0 & 0x4000) {
                if ((D_800CA8CE >= 0x40) && (D_800CA8C8.y >= -0xBF)) {
                    D_800CA8C8.y = (u16)D_800CA8C8.y - 0x10;
                    D_800CA8DC = 4;
                } else if (D_800CA8CC.y < 0x70) {
                    D_800CA8CC.y = (u16)D_800CA8CC.y + 0x10;
                    D_800CA8E0 = 4;
                }
            }
            if (D_800CA8B0 & 0x8000) {
                if ((D_800CA8CC.x < 0x71) && (D_800CA8C8.x < 0)) {
                    D_800CA8C8.x = (u16)D_800CA8C8.x + 0x10;
                    D_800CA8DC = 4;
                } else if (D_800CA8CC.x >= 0x11) {
                    D_800CA8CC.x = (u16)D_800CA8CC.x - 0x10;
                    D_800CA8E0 = 4;
                }
            }
            if (D_800CA8B0 & 0x2000) {
                if (D_800CA8CC.x >= 0x70) {
                    if (D_800CA8C8.x >= -0x5F) {
                        D_800CA8C8.x = (u16)D_800CA8C8.x - 0x10;
                        D_800CA8DC = 4;
                    } else {
                        goto right_second;
                    }
                } else {
right_second:
                    if (D_800CA8CC.x < 0xD0) {
                        D_800CA8CC.x = (u16)D_800CA8CC.x + 0x10;
                        D_800CA8E0 = 4;
                    }
                }
            }
            if (D_800CA8B0 & 0xF000) {
                var_a0 = 0x7D;
                if ((D_800CA8E0 != 0) || (D_800CA8DC != 0)) {
play_move_sound:
                    func_80050080(var_a0, 0x80);
                    return;
                }
            }
        }
        break;

    case 3:
        if (D_800CA8D4 & 0xA20) {
            func_80050080(0x7E, 0x80);
            temp_v0 = -D_800CA8B8.x;
            temp_v0 += D_800CA8BC.x;
            var_a2 = temp_v0 - 0x10;
            if (var_a2 < 0) {
                var_a2 = temp_v0 - 1;
            }
            temp_v1 = D_800CA8B8.y * -1 + D_800CA8BC.y;
            temp_v0 = temp_v1 - 0x10;
            var_a2 >>= 4;
            if (temp_v0 < 0) {
                temp_v0 = temp_v1 - 1;
            }
            var_a1 = temp_v0 >> 4;
            {
                u8 *base = D_80042FD8;
                *(s32 *)(base + 0xE0) = var_a2 + (var_a1 * 0x13);
                D_800CA89C = 1;
                *(u32 *)(base + 0x28) &= ~8;
            }
            return;
        }
        if (D_800CA8D4 & 0x40) {
            func_80050080(0x7D, 0x80);
            D_800CA8D8 = 1;
        }
        break;
    }
}

POLY_FT4 *func_80051D78(POLY_FT4 *poly, u_long *ot, s32 which)
{
    WselSpriteEntry *frame;
    WselSpriteEntry *tex;

    if (which) {
        frame = &D_800C6828;
        tex = &D_800C6798;
    } else {
        frame = &D_800C67E0;
        tex = &D_800C6780;
    }

    *(u_long *)&poly->r0 = 0x00808080;
    setPolyFT4(poly);
    setSemiTrans(poly, tex->semi);

    poly->x2 = poly->x0 = tex->x + 0x20 - ((u8 *)frame)[4] * 8;
    poly->y1 = poly->y0 = tex->y + 0x28 - ((u8 *)frame)[5] * 8;
    poly->x1 = poly->x3 = poly->x0 + frame->semi * 8 - 1;
    poly->y2 = poly->y3 = poly->y0 + frame->color * 8 - 1;

    poly->u0 = poly->u2 = frame->tp * 8;
    poly->v1 = poly->v0 = frame->abr * 8;
    poly->u1 = poly->u3 = poly->u0 + frame->semi * 8 - 1;
    poly->v2 = poly->v3 = poly->v0 + frame->color * 8 - 1;

    setClut(poly, tex->clut_x, tex->clut_y);
    setTPage(poly, tex->tp, tex->abr, tex->tx, tex->ty);
    addPrim(ot, poly);
    poly++;

    poly->x2 = poly->x0 = tex->x;
    poly->y1 = poly->y0 = tex->y + 0x20;
    tex = &D_800C67B0;
    setlen(poly, 9);
    *(u_long *)&poly->r0 = 0x00808080;
    poly->code = 0x2c;
    setSemiTrans(poly, tex->semi);
    poly->u0 = poly->u2 = 0xb8;
    poly->v1 = poly->v0 = 6;
    poly->x1 = poly->x3 = poly->x0 + 0x20;
    poly->y2 = poly->y3 = poly->y0 + 0xa;
    poly->u1 = poly->u3 = poly->u0 + 0x20;
    poly->v2 = poly->v3 = poly->v0 + 0xa;
    setClut(poly, tex->clut_x, tex->clut_y);
    setTPage(poly, tex->tp, tex->abr, tex->tx, tex->ty);
    addPrim(ot, poly);
    return poly + 1;
}

void func_800520A8(void)
{
    D_800CA8D8 = 0;
    func_80052154();
    func_80052510();
    func_800521D0(D_80052608, 0);
    func_800521D0(D_8007CC2C, 1);
    func_800521D0(D_8008E650, 2);
    func_800521D0(D_8009023C, 3);
    func_800521D0(D_80098E80, 4);
    func_800521D0(D_800A90A4, 5);
    func_800521D0(D_800B92C8, 6);
    func_800521D0(D_800C130C, 7);
}

void func_80052154(void)
{
    D_800CA8C8.x = D_800CA8B8.x = -0x40;
    D_800CA8C8.y = D_800CA8B8.y = -0x70;
    D_800CA8DC = 0;
    D_800CA8CC.x = D_800CA8BC.x = 0x70;
    D_800CA8CC.y = D_800CA8BC.y = 0x40;
    D_800CA8E0 = 0;
    func_80050944();
}

void func_800521D0(u8* res, s32 index)
{
    RECT rect;
    u8 *base;
    WselTexEntry *entry;
    s16 x1, y1, x2, y2;
    s32 block_len;
    int skip;
    base = (u8 *)D_800C6720;
    entry = (WselTexEntry *)(base + index * 0x18);
    x1 = entry->x1; y1 = entry->y1; x2 = entry->x2; y2 = entry->y2;
    skip = 8;
    if (res[4] & skip)
    {
        block_len = *(s32*)(res + skip);
        rect.w = *(u16*)(res + 0x10) * *(u16*)(res + 0x12);
        rect.x = x2;
        rect.y = y2;
        rect.h = 1;
        func_80019A34(&rect, (u_long*)(res + 0x14));
        res = (res + skip) + block_len;
    }
    else
    {
        res = res + 8;
    }
    rect.x = x1; rect.y = y1; rect.w = *(u16*)(res + 8); rect.h = *(u16*)(res + 0xA);
    func_80019A34(&rect, (u_long*)(res + 0xC));
}

s32 func_800522AC(void)
{
    signed short axis_x_dup;
    unsigned char *ptr;
    unsigned char device_status;
    unsigned short raw_buttons;
    unsigned short raw_buttons_hi;
    unsigned long buttons;
    unsigned int raw_buttons_reread;
    signed short axis;

    ptr = (unsigned char *)0x801ED600;
    device_status = ptr[0];
    if (device_status >= 0xFE)
    {
        return 0;
    }
    raw_buttons = *((unsigned short *)(ptr + 2));
    raw_buttons_reread = *((unsigned short *)(ptr + 2));
    raw_buttons_hi = raw_buttons_reread;
    buttons = (raw_buttons >> 8) | (raw_buttons_hi << 8);
    buttons = PAD_REMAP_FACE_BITS(buttons);
    if (device_status)
    {
        axis = *((signed short *)(ptr + 0x2C));
        axis_x_dup = axis;
        if (axis < (-1))
        {
            buttons |= PAD_BTN_LEFT;
        }
        else if (axis_x_dup >= 2)
        {
            buttons |= PAD_BTN_RIGHT;
        }
        axis = *((signed short *)(ptr + 0x2E));
        if (axis < (-1))
        {
            buttons |= PAD_BTN_UP;
        }
        else if (axis >= 2)
        {
            buttons |= PAD_BTN_DOWN;
        }
    }
    return buttons;
}

void func_80052384(void)
{
    u8* ptr = (u8*)0x801ED600;
    u8 device_status = D_801ED600[0];
    u16 raw_buttons;
    u16 unused;
    u32 buttons;
    s16 axis;
    s32 state;
    if (device_status >= 0xFE)
    {
        state = 0;
    }
    else
    {
        raw_buttons = *((u16*)(ptr + 2));

        buttons = (raw_buttons >> 8) | (*((u16*)(2 + ptr)) << 8);
        buttons = PAD_REMAP_FACE_BITS(buttons);
        if ((*ptr) != 0)
        {
            axis = *((s16*)(ptr + 0x2C));
            if (axis < (-1))
            {
                buttons |= PAD_BTN_LEFT;
            }
            else if (axis >= 2)
            {
                buttons |= PAD_BTN_RIGHT;
            }
            axis = *((volatile s16*)(ptr + 0x2E));
            if (axis < (-1))
            {
                buttons |= PAD_BTN_UP;
            }
            else if (axis >= 2)
            {
                buttons |= PAD_BTN_DOWN;
            }
        }
        state = buttons;
    }
    {
        s32 current_state = state;
        D_800CA8B0 = current_state;
        D_800CA8D4 = 0;

        if (((current_state == D_800CA8A0) || ((D_800CA8A0 != 0) && (current_state & (D_800CA8A0 | 0xB6F)))) && current_state != 0)
    {
        u32 dpad = current_state & (PAD_BTN_UP | PAD_BTN_RIGHT | PAD_BTN_DOWN | PAD_BTN_LEFT);
        if (dpad != 0)
        {
            current_state = dpad;
        }
        if (D_800CA8A4 == 0)
        {
            D_800CA8D4 = current_state;
            D_800CA8A4 = 2;
        }
        else
        {
            D_800CA8A4--;
            D_800CA8D4 = 0;
        }
        return;
    }
        else if (current_state == 0)
    {
        (void)(&D_800CA8D4);
        *((s32*)(&D_800CA8A4)) = 0;
        *((s32*)(&D_800CA8A0)) = 0;
    }
    else
    {
            D_800CA8D4 = current_state;
            D_800CA8A0 = current_state;
        D_800CA8A4 = 15;
    }
    }
}

void func_80052510(void)
{
    SCDRegs *base = SCD_REGS;
    s32 state;
    u32 buttons;
    s16 axis;

    D_800CA8D4 = 0;
    if (D_801ED600[0] >= 254)
    {
        state = 0;
    }
    else
    {
        buttons = ((base->held_buttons >> 8) & 0xFF) | (base->held_buttons << 8);
        buttons = PAD_REMAP_FACE_BITS(buttons);
        if (base->device_type != 0)
        {
            axis = base->axis_x.signed_value;
            if (axis < (-1))
            {
                buttons |= PAD_BTN_LEFT;
            }
            else if (axis >= 2)
            {
                buttons |= PAD_BTN_RIGHT;
            }
            axis = base->axis_y.signed_value;
            if (axis < (-1))
            {
                buttons |= PAD_BTN_UP;
            }
            else if (axis >= 2)
            {
                buttons |= PAD_BTN_DOWN;
            }
        }
        state = buttons;
    }
    D_800CA8A0 = state;
    D_800CA8A4 = 15;
}
