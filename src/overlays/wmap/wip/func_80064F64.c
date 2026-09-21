#include "common.h"
#include "sdk/libgpu.h"

#define M2C_FIELD(expr, type_ptr, offset) (*(type_ptr)((s8 *)(expr) + (offset)))

/** @brief Rectangle used by world-map image transfers. */
typedef struct
{
    s16 x;
    s16 y;
    s16 w;
    s16 h;
} WmapRect;

extern u8 D_800DCF18[];
extern s32 D_801ADAFC;

/**
 * @brief Queue a TIM read from CD and upload the decoded image(s) to VRAM.
 * @param arg0 CD resource id to queue.
 * @note Best match ~73% (gcc280_g0); residual is a cross-jump/reg-alloc tie
 *       (target keeps both pointer regs live; the identical branch tails merge).
 */
void func_80064F64(s32 arg0)
{
    WmapRect rect;
    u8 *var_s1;
    u8 *temp_s0;

    var_s1 = D_800DCF18;
    cdrom_queue_read(arg0 & 0xFFFF, var_s1);
    cdrom_wait_queue_empty();
    temp_s0 = var_s1;
    var_s1 += 8;
    if (M2C_FIELD(temp_s0, u8 *, 4) & 8)
    {
        rect = *(WmapRect *)((u8 *)var_s1 + 4);
        LoadImage(&rect, var_s1 + 0xC);
        var_s1 += *(s32 *)var_s1;
        rect = *(WmapRect *)((u8 *)var_s1 + 4);
        if (rect.x == -1)
        {
            return;
        }
        goto draw;
    }
    rect = *(WmapRect *)((u8 *)var_s1 + 4);
    if (rect.x == -1)
    {
        return;
    }
draw:
    LoadImage(&rect, var_s1 + 0xC);
    DrawSync(0);
    D_801ADAFC = 1;
}
