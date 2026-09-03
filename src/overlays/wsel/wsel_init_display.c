#include "common.h"
#include "sdk/libgpu.h"

typedef struct {
    u8 _pad0[0x4040];
    DISPENV disp_env;
    DRAWENV draw_env;
    u8 _pad1[0x80CC - 0x40B0];
} WselRenderHalf;

typedef struct {
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

extern s32 D_800CA89C;

extern void func_8001D5AC(s32);
extern void func_8001D58C(s32, s32);
extern void func_8001990C(void*, s32, s32, s32);
extern void func_800500A8(void);
extern void func_800503D4(s32, s32, s32, s32);
extern void func_8001C62C(void*, s32, s32, s32, s32);
extern void func_8001C56C(void*, s32, s32, s32, s32);
extern void func_800520A8(void);

/**
 * @brief WSEL (world select) one-time display init: projection geometry, the
 *        two display/draw environments for the double-buffered render halves,
 *        and initial state reset.
 * @param arg Base of the two-element render context (0x80CC-byte buffers).
 * @see field_init_display (analogous field-overlay init)
 * @see (100%)
 */
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
