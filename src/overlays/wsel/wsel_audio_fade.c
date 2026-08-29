#include "common.h"
#include "display.h"
#include "gpu_packet.h"
#include "sdk/libgpu.h"

#define WSEL_FADE_NEUTRAL 0x100
#define WSEL_FADE_ADDITIVE_THRESHOLD (WSEL_FADE_NEUTRAL + 1)
#define WSEL_FADE_ADDITIVE_DRAW_MODE 0x25
#define WSEL_FADE_SUBTRACTIVE_DRAW_MODE 0x45
#define WSEL_NEXT_FADE_PRIMITIVE(primitive, type) \
    ((WselFadePrimitive*)((u8*)(primitive) + sizeof(type)))

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

extern u8 D_800C6878;
extern WselFadeTarget D_800CA878;
extern WselFadeCurrent D_800CA888;

extern void func_800141EC(u16, void*);
extern void func_80013F2C(void);
extern void func_80016E7C(const void*, void*, s32);
extern void func_80022AE8(void*, s32);
extern void func_80022068(s32);
extern void func_80022040(void*);
extern void func_8002279C(s32, s32);
extern void func_8002216C(s32, s32, s32, s32);

/** @see TITLE load_title_seq (100%) */
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

/** @see TITLE stop_title_music (100%) */
void func_80050030(void)
{
    func_80022068(0);
}

/** @see TITLE start_title_music (100%) */
void func_80050050(void)
{
    func_80022040((void*)&D_800C6878);
    func_8002279C(0, 0x7F);
}

/** @see TITLE play_title_sfx (100%) */
void func_80050080(s32 sound_id, s32 pan)
{
    func_8002216C(sound_id, 0, pan, 0x7F);
}

/** @see TITLE reset_fade_state (100%) */
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

/** @see TITLE render_fade_overlay (100%) */
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

/** @see TITLE set_fade_target (100%) */
void func_800503D4(s32 red, s32 green, s32 blue, s32 steps)
{
    D_800CA878.red = red;
    D_800CA878.green = green;
    D_800CA878.blue = blue;
    D_800CA878.steps = steps;
}
