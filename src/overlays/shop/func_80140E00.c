#include "common.h"

typedef struct { s32 tag; u32 color; s16 x; s16 y; s16 w; u16 h; } ShopGpuPacket;
typedef struct { s32 tag; u8 pad4[0x40AE]; s16 frame_flag; } ShopDrawState;
typedef struct { u8 pad0[0x4000]; ShopGpuPacket *prim_cursor; } ShopPrimState;
typedef ShopGpuPacket *(*ShopElemDrawFunc)();

extern u32 D_801451D8;
extern s32 D_80145238;
extern s32 D_80145248;
extern s32 D_80145CD8;
extern void func_801415F4();

void func_80140E00(ShopDrawState *arg0, ShopPrimState *arg1)
{
    ShopGpuPacket *var_s0;
    ShopDrawState *var_s4;
    u32 *var_s5;
    s32 temp_s1;
    s32 temp_s2;
    s32 var_s7;
    s32 sp20[24];
    u32 address_mask;
    u32 tag_mask;
    u32 temp_a0_2;
    s32 temp_v1_2;
    u32 temp_a1;
    s32 temp_a2;
    s32 temp_a0_3;
    s32 var_v1;
    s32 temp_a3_2;
    s32 var_v0;
    s32 temp_a3_3;
    u32 temp_v0_3;
    u32 temp_a0_4;
    s32 temp_a0_5;
    s32 temp_a3_5;
    s32 var_v0_2;
    s32 temp_a3_6;
    s32 element_height;
    s32 count;
    s32 packet_address;

    var_s4 = arg0;
    var_s0 = arg1->prim_cursor;

    if (arg0->frame_flag != 0)
        func_8001C56C(sp20, 0, 0xF0, 0x140, 0xE0);
    else
        func_8001C56C(sp20, 0, 8, 0x140, 0xE0);

    var_s5 = (u32 *)&D_801451D8;
    var_s7 = 0;
    do { do { address_mask = 0x00FFFFFF; } while (0); } while (0);
    tag_mask = 0xFF000000;

    for (; var_s7 < 8; var_s7++, var_s5 += 3)
    {
        if (*var_s5 & 7)
        {
            ShopGpuPacket *draw_cursor;
            draw_cursor = var_s0;

            if (*(ShopElemDrawFunc *)((u8 *)var_s5 + 8) == (ShopElemDrawFunc)func_801415F4)
            {
                count = D_80145CD8;
                element_height = count << 4;
                if ((D_80145248 + 0x74) < element_height)
                    var_s0 = (ShopGpuPacket *)func_800AE76C(var_s0, var_s4, 0x11E, 0xAC, 0);
                if (D_80145248 != 0)
                    var_s0 = (ShopGpuPacket *)func_800AE76C(var_s0, var_s4, 0x11E, 0x38, 1);

                func_8001A5D4((s32)var_s0, sp20);
                var_s0->tag = (var_s0->tag & tag_mask) | (var_s4->tag & address_mask);
                var_s4->tag = (s32)((var_s4->tag & tag_mask) | ((s32)var_s0 & address_mask));
                var_s0 = (ShopGpuPacket *)((u8 *)var_s0 + 0x40);

                ((u8 *)var_s0)[3] = 3;
                var_s0->color = 0xFFFF00;
                ((u8 *)var_s0)[7] = 0x60;
                var_s0->w = 6;
                element_height = (*(u32 *)((u8 *)var_s5 + 4) >> 1) & 0xFF;
                var_s0->h = (u16)((element_height * (element_height / 0x10)) / D_80145CD8);
                if ((s16)var_s0->h >= (s32)((*(u32 *)((u8 *)var_s5 + 4) >> 1) & 0xFF) - 2)
                    var_s0->h = (u16)((*(u32 *)((u8 *)var_s5 + 4) >> 1) & 0xFF);
                var_s0->x = 1;
                var_s0->y = (s16)(((((s32)((*(u32 *)((u8 *)var_s5 + 4) >> 1) & 0xFF)) - 3) * (D_80145248 / 0x10)) / D_80145CD8);
                var_s0->tag = (var_s0->tag & tag_mask) | (var_s4->tag & address_mask);
                var_s4->tag = (var_s4->tag & tag_mask) | ((s32)var_s0 & address_mask);
                var_s0 = (ShopGpuPacket *)((u8 *)var_s0 + 0x10);

                {
                    u32 panel_word;
                    u32 field;
                    u32 high;
                    panel_word = *var_s5;
                    field = (panel_word >> 7) & 0x1FF;
                    high = panel_word >> 24;
                    var_s0 = (ShopGpuPacket *)func_800AD850(var_s0, var_s4,
                        field + ((((*(u32 *)((u8 *)var_s5 + 4)) & 1) << 8) | high) + 3,
                        *((u8 *)var_s5 + 2), 0xA, ((*(u32 *)((u8 *)var_s5 + 4)) >> 1) & 0xFF,
                        arg0->frame_flag, var_s7 == 0);
                }
                draw_cursor = var_s0;
            }

            func_8001A5D4((s32)draw_cursor, sp20);
            var_s0->tag = (var_s0->tag & tag_mask) | (var_s4->tag & address_mask);
            var_s4->tag = (s32)((var_s4->tag & tag_mask) | ((s32)var_s0 & address_mask));

            temp_a0_2 = *var_s5;
            temp_v1_2 = temp_a0_2 & 7;
            var_s0 = (ShopGpuPacket *)((u8 *)var_s0 + 0x40);

            switch (temp_v1_2)
            {
            case 1:
                temp_a1 = *(u32 *)((u8 *)var_s5 + 4);
                temp_a0_4 = temp_a0_2 >> 24;
                temp_a2 = ((temp_a1 & 1) << 8) | temp_a0_4;
                temp_a0_3 = (temp_a0_2 >> 3) & 0xF;
                var_v1 = temp_a2 * temp_a0_3;
                if (var_v1 < 0) var_v1 += 7;
                temp_a3_2 = (temp_a1 >> 1) & 0xFF;
                var_v0 = temp_a3_2 * temp_a0_3;
                temp_s1 = var_v1 >> 3;
                if (var_v0 < 0) var_v0 += 7;
                temp_s2 = var_v0 >> 3;
                temp_a3_3 = temp_a3_2 - temp_s2;
                var_s0 = (*(ShopElemDrawFunc *)((u8 *)var_s5 + 8))(var_s4, var_s0,
                    (temp_a2 - temp_s1) / 2, temp_a3_3 / 2);
                {
                    u32 post_word;
                    u32 field;
                    u32 high;
                    post_word = *var_s5;
                    field = (post_word >> 7) & 0x1FF;
                    high = post_word >> 24;
                    var_s0 = (ShopGpuPacket *)func_800AD850(var_s0, var_s4,
                        field + (s32)((((*(u32 *)((u8 *)var_s5 + 4) & 1) << 8) | high) - temp_s1) / 2,
                        *((u8 *)var_s5 + 2) + (s32)(((*(u32 *)((u8 *)var_s5 + 4) >> 1) & 0xFF) - temp_s2) / 2,
                        temp_s1, temp_s2, arg0->frame_flag, var_s7 == 0);
                }
                {
                    u32 old_word, new_word;
                    old_word = *var_s5;
                    new_word = old_word & ~0x78;
                    new_word |= (((((old_word >> 3) & 0xF) + 1) & 0xF) * 8);
                    *var_s5 = new_word;
                    if (((new_word >> 3) & 0xF) == 8)
                    {
                        *var_s5 = (new_word & ~7) | 2;
                        func_800AA02C();
                    }
                }
                break;

            case 2:
                var_s0 = (*(ShopElemDrawFunc *)((u8 *)var_s5 + 8))(var_s4, var_s0, 0, 0);
                {
                    u32 case_word;
                    u32 high;
                    case_word = *var_s5;
                    high = case_word >> 24;
                    var_s0 = (ShopGpuPacket *)func_800AD850(var_s0, var_s4,
                        (case_word >> 7) & 0x1FF, *((u8 *)var_s5 + 2),
                        ((*(u32 *)((u8 *)var_s5 + 4) & 1) << 8) | high,
                        (*(u32 *)((u8 *)var_s5 + 4) >> 1) & 0xFF, arg0->frame_flag, var_s7 == 0);
                }
                break;

            case 3:
                temp_a1 = *(u32 *)((u8 *)var_s5 + 4);
                temp_a2 = ((temp_a1 & 1) << 8) | (temp_a0_2 >> 24);
                temp_a0_5 = (temp_a0_2 >> 3) & 0xF;
                temp_s1 = (temp_a2 * temp_a0_5) / 8;
                temp_a3_5 = (temp_a1 >> 1) & 0xFF;
                temp_s2 = (temp_a3_5 * temp_a0_5) / 8;
                temp_a3_6 = temp_a3_5 - temp_s2;
                var_s0 = (*(ShopElemDrawFunc *)((u8 *)var_s5 + 8))(var_s4, var_s0,
                    (temp_a2 - temp_s1) / 2, temp_a3_6 / 2);
                {
                    u32 post_word;
                    u32 field;
                    u32 high;
                    post_word = *var_s5;
                    field = (post_word >> 7) & 0x1FF;
                    high = post_word >> 24;
                    var_s0 = (ShopGpuPacket *)func_800AD850(var_s0, var_s4,
                        field + (s32)((((*(u32 *)((u8 *)var_s5 + 4) & 1) << 8) | high) - temp_s1) / 2,
                        *((u8 *)var_s5 + 2) + (s32)(((*(u32 *)((u8 *)var_s5 + 4) >> 1) & 0xFF) - temp_s2) / 2,
                        temp_s1, temp_s2, arg0->frame_flag, var_s7 == 0);
                }
                {
                    u32 old_word;
                    old_word = *var_s5;
                    if (!(((*var_s5 = (old_word & ~0x78) |
                        (((((old_word >> 3) & 0xF) - 1) & 0xF) << 3)) >> 3) & 0xF))
                    {
                        *var_s5 = *var_s5 & ~7;
                    }
                    D_80145238 = 0;
                }
                break;
            }
        }
    }
    arg1->prim_cursor = var_s0;
}
