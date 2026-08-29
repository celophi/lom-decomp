#include "common.h"
#include "display.h"
#include "gpu_packet.h"
#include "sdk/libgpu.h"

#define ZUKAN_GPU_ADDRESS_MASK 0xFFFFFF
#define ZUKAN_GPU_TAG_HIGH_MASK 0xFF000000
#define ZUKAN_FADE_NEUTRAL 0x100
#define ZUKAN_FADE_ADDITIVE_THRESHOLD (ZUKAN_FADE_NEUTRAL + 1)
#define ZUKAN_FADE_ADDITIVE_DRAW_MODE 0x25
#define ZUKAN_FADE_SUBTRACTIVE_DRAW_MODE 0x45
#define ZUKAN_NEXT_FADE_PRIMITIVE(primitive, type) \
    ((ZukanFadePrimitive*)((u8*)(primitive) + sizeof(type)))

typedef struct
{
    s32 tag;
    s32 color_and_code;
    s16 x0;
    s16 y0;
    s16 x1;
    u16 y1;
} ZukanLinePacket;

typedef union
{
    TILE tile;
    DR_TPAGE draw_mode;
} ZukanFadePrimitive;

typedef struct
{
    s16 red;
    s16 green;
    s16 blue;
    s16 steps_remaining;
} ZukanFadeState;

extern ZukanFadeState D_80157D30;
extern ZukanFadeState D_80157D48;

/** @see GOLEM golem_emit_panel_outline (100%) */
ZukanLinePacket* func_80142374(ZukanLinePacket* packet, s32* ordering_table, s32 x, s32 y, s32 width, s32 height, s32 color)
{
    s32 temporary;

    packet->color_and_code = color;
    setlen(packet, 3);
    setcode(packet, 0x40);
    packet->x0 = x;
    packet->y0 = y;
    packet->x1 = x + width;
    packet->y1 = y;
    temporary = ZUKAN_GPU_TAG_HIGH_MASK;
    packet->tag = (packet->tag & ZUKAN_GPU_TAG_HIGH_MASK) | (*ordering_table & ZUKAN_GPU_ADDRESS_MASK);
    *ordering_table = (*ordering_table & temporary) | ((s32)packet & ZUKAN_GPU_ADDRESS_MASK);
    packet++;

    packet->color_and_code = color;
    setlen(packet, 3);
    setcode(packet, 0x40);
    packet->x0 = x + width;
    packet->y0 = y;
    packet->x1 = x + width;
    packet->y1 = y + height;
    addPrim(ordering_table, packet);
    packet++;

    packet->color_and_code = color;
    setlen(packet, 3);
    setcode(packet, 0x40);
    packet->x0 = x + width;
    temporary = y + height;
    packet->y0 = temporary;
    packet->x1 = x;
    packet->y1 = y + height;
    addPrim(ordering_table, packet);
    packet++;

    packet->color_and_code = color;
    setlen(packet, 3);
    setcode(packet, 0x40);
    packet->x0 = x;
    packet->y0 = y;
    packet->x1 = x;
    packet->y1 = y + height;
    addPrim(ordering_table, packet);
    return packet + 1;
}

/** @see GOLEM golem_set_fade_target (100%) */
void func_801424D0(s16 red, s16 green, s16 blue, s16 steps)
{
    D_80157D30.red = red;
    D_80157D30.green = green;
    D_80157D30.blue = blue;
    D_80157D30.steps_remaining = steps;
}

/** @see GOLEM golem_render_fade (100%) */
ZukanFadePrimitive* func_801424EC(ZukanFadePrimitive* primitive, u_long* ordering_table_tag)
{
    s32 red_step;
    s32 green_step;
    s32 blue_step;
    s32 draw_mode;

    if (D_80157D30.steps_remaining != 0)
    {
        red_step = (D_80157D30.red - D_80157D48.red) / D_80157D30.steps_remaining;
        green_step = (D_80157D30.green - D_80157D48.green) / D_80157D30.steps_remaining;
        blue_step = (D_80157D30.blue - D_80157D48.blue) / D_80157D30.steps_remaining;
        D_80157D30.steps_remaining = D_80157D30.steps_remaining - 1;
        D_80157D48.red = D_80157D48.red + red_step;
        D_80157D48.green = D_80157D48.green + green_step;
        D_80157D48.blue = D_80157D48.blue + blue_step;
    }
    else
    {
        D_80157D48.red = D_80157D30.red;
        D_80157D48.green = D_80157D30.green;
        D_80157D48.blue = D_80157D30.blue;
    }

    if ((D_80157D48.red != ZUKAN_FADE_NEUTRAL) || (D_80157D48.green != D_80157D48.red) ||
        (D_80157D48.blue != D_80157D48.green))
    {
        if (D_80157D48.red >= ZUKAN_FADE_ADDITIVE_THRESHOLD)
        {
            primitive->tile.r0 = D_80157D48.red - 1;
            primitive->tile.g0 = D_80157D48.green - 1;
            primitive->tile.b0 = D_80157D48.blue - 1;
        }
        else
        {
            if (D_80157D48.red == ZUKAN_FADE_NEUTRAL)
            {
                primitive->tile.r0 = 0;
            }
            else
            {
                primitive->tile.r0 = ~D_80157D48.red;
            }
            if (D_80157D48.green == ZUKAN_FADE_NEUTRAL)
            {
                primitive->tile.g0 = 0;
            }
            else
            {
                primitive->tile.g0 = ~D_80157D48.green;
            }
            if (D_80157D48.blue == ZUKAN_FADE_NEUTRAL)
            {
                primitive->tile.b0 = 0;
            }
            else
            {
                primitive->tile.b0 = ~D_80157D48.blue;
            }
        }

        setTile(&primitive->tile);
        setSemiTrans(&primitive->tile, 1);
        primitive->tile.w = SCREEN_WIDTH;
        draw_mode = ZUKAN_FADE_ADDITIVE_DRAW_MODE;
        SET_YX0(&primitive->tile, 0, 0);
        primitive->tile.h = SCREEN_HEIGHT;
        addPrim(ordering_table_tag, &primitive->tile);

        primitive = ZUKAN_NEXT_FADE_PRIMITIVE(primitive, TILE);
        if (D_80157D48.red < ZUKAN_FADE_ADDITIVE_THRESHOLD)
        {
            draw_mode = ZUKAN_FADE_SUBTRACTIVE_DRAW_MODE;
        }
        setDrawTPage(&primitive->draw_mode, 0, 0, draw_mode);
        addPrim(ordering_table_tag, &primitive->draw_mode);

        primitive = ZUKAN_NEXT_FADE_PRIMITIVE(primitive, DR_TPAGE);
    }
    return primitive;
}
