#include "common.h"
#include "vector.h"

/**
 * @brief FIELD context field holding the current primitive-chain handle.
 */
typedef struct
{
    u8 pad0[0x40B8];
    s32 primitive;
} FieldContext;

void field_restore_fade_target_with_duration(s16 duration);
void func_800A3938(s32 event_id, s32 value);
s32 func_800A6060(s32 primitive, void *context);
s32 func_800A5960(s32 primitive, void *context, s32 index);
extern u8 D_800EF85C[];
extern s32 D_800F2298;
extern Vec2s D_801077FC;
extern s16 D_8011F3D0;
extern u16 D_80122904;
extern s32 D_801229F0;
extern s32 g_pad_input;

/**
 * @brief Advance a timed FIELD panel, update its fade, and render its contents.
 * @param context FIELD context containing the primitive-chain handle.
 * @note Selected pad buttons shorten the middle portion of the countdown.
 * @note The volatile timer read preserves the reload in the original fade path.
 * @note GCC 2.7.2 CDK matches all 115 instructions (460 bytes).
 */
void func_800A5794(FieldContext *context)
{
    s32 timer_or_event;
    s32 primitive;
    u16 decremented_timer;
    u32 timer;
    u8 event_selector;
    u8 render_selector;

    if (D_800F2298 != 0)
    {
        decremented_timer = D_80122904 - 1;
        D_80122904 = decremented_timer;
        if ((decremented_timer & 0xFFFF) == 0x14)
        {
            field_restore_fade_target_with_duration(0x14);
        }
        timer_or_event = D_80122904;
        timer = timer_or_event & 0xFFFF;
        if (timer == 0)
        {
            D_800F2298 = 0;
            return;
        }
        if (timer < 0x20U)
        {
            if (timer == 0x1F)
            {
                event_selector = D_800EF85C[D_801229F0];
                if (event_selector & 0x80)
                {
                    switch (event_selector & 0x7F)
                    {
                    case 0:
                        timer_or_event = 0x11D;
                        goto dispatch_event;
                    case 1:
                        timer_or_event = 0x11B;
                        goto dispatch_event;
                    case 2:
                        timer_or_event = 0x11F;
                        goto dispatch_event;
                    }
                }
                else
                {
                    timer_or_event = 0x119;
dispatch_event:
                    func_800A3938(timer_or_event, 0x80);
                }
            }
            D_8011F3D0 = D_80122904 * 4;
        }
        else if (timer >= 0x76U)
        {
            D_8011F3D0 = (0x96 - *(volatile u16 *)&D_80122904) * 4;
        }
        else if (((u32) (timer_or_event - 0x21) < 0x46U) && (g_pad_input & 0x220))
        {
            D_80122904 = 0x20;
        }
        D_801077FC.y = 0;
        D_801077FC.x = 0;
        render_selector = D_800EF85C[D_801229F0];
        primitive = context->primitive;
        if (render_selector != 0)
        {
            primitive = func_800A5960(primitive, context, render_selector & 0x7F);
        }
        else
        {
            primitive = func_800A6060(primitive, context);
        }
        context->primitive = primitive;
    }
}
