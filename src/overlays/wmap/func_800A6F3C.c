/* Partial WMAP decompilation: 87.906250% (gcc280_g0). */
#include "common.h"

/** @brief Route state and coordinate arrays within the world-map data block. */
typedef struct
{
    u8 pad_000[0x36C];
    s32 start_x;
    s32 start_y;
    s16 current_x;
    s16 current_y;
    u8 pad_378[4];
    s16 screen_x;
    s16 screen_y;
    s32 active;
    s32 target_x;
    s32 target_y;
    s32 state;
    s16 route_x[64];
    s16 route_y[64];
} WmapRoute;

extern WmapRoute D_8019D248;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_80182DF8;
extern s32 D_801B2E58;
extern void func_8006D0F0(s32, s32 *, s32 *);
extern void func_8005EB68(s32, s32, s32, s32, s16 *, s16 *);
extern void func_800652A8(s32, s32);
extern void func_800A8510(void);

/** @brief Build the effect route, initialize its position, and advance the sequence. */
void func_800A6F3C(void)
{
    s16 screen_x;

    func_8006D0F0(18, &D_800DCEF8, &D_800DCF00);
    func_8005EB68(D_8019D248.start_x, D_8019D248.start_y, D_800DCEF8, D_800DCF00, D_8019D248.route_x, D_8019D248.route_y);
    D_8019D248.target_x = D_800DCEF8;
    D_8019D248.target_y = D_800DCF00;
    D_80182DF8 = 1;
    D_8019D248.active = 1;
    D_8019D248.state = 1;
    D_8019D248.current_x = (u16)D_8019D248.route_x[2];
    screen_x = (D_8019D248.route_x[2] - 1) * 160;
    D_8019D248.current_y = (u16)D_8019D248.route_y[2];
    D_8019D248.screen_x = screen_x;
    D_8019D248.screen_y = (D_8019D248.route_y[2] - 1) * 160;
    func_800652A8(50, 128);
    D_801B2E58++;
    func_800A8510();
}
