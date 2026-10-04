#ifndef SHOP_RENDER_H
#define SHOP_RENDER_H

#include "shop_internal.h"

void shop_draw_windows(ShopFrameContext* ctx, ShopPacketBuffer* buffer);
u8* shop_draw_money_window(u32* ot, u8* prim, s32 x_inset, s32 y_inset);
u8* shop_draw_list_window(u32* ot, u8* prim, s32 x_inset, s32 y_inset);
u8* shop_draw_detail_window(u32* ot, u8* prim, s32 x_inset, s32 y_inset);
u8* shop_draw_title_window(u32* ot, u8* prim, s32 x_inset, s32 y_inset);

#endif
