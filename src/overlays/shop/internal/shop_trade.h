#ifndef SHOP_TRADE_H
#define SHOP_TRADE_H

#include "shop_internal.h"

u8* shop_draw_notice_window(u32* ot, u8* prim, s32 x_inset, s32 y_inset);
void shop_open_buy_prompt(void);
void shop_open_sell_prompt(void);

#endif
