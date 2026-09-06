#include "common.h"

extern s32 *D_80123FC4;
extern s32 D_801227F0;
extern s32 g_gosub_result_count;
extern s32 g_gosub_result_values[];
extern u8 *D_80122B74;

extern u8 *func_800A9060(void);
extern void func_800BD520(s32 arg0, s32 arg1, s32 arg2);
extern void func_800BE888(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
extern void func_800BEA10(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4);
extern void func_800BEC44(u8 *arg0, s32 arg1);
extern s32 *func_800C1EC8(s32 *src, s32 *dest, s32 n);

/**
 * @brief Dispatch a queued gosub result to the handler selected by its kind.
 * @param arg0 Gosub-result kind selector.
 */
void func_800BE710(s32 arg0)
{
    u8 *handle;

    D_801227F0 = 0;
    func_800C1EC8(NULL, D_80123FC4, 0x60);

    if (g_gosub_result_count == 0)
    {
        goto count_zero;
    }

    switch (arg0)
    {
        case 2:
            handle = func_800A9060();
            if (handle != NULL)
            {
                func_800BEA10((s32)handle, 2, g_gosub_result_values[0], g_gosub_result_values[1],
                              g_gosub_result_values[2]);
                {
                    s32 offset = (s32)handle - 0xCE0;
                    func_800BD520(0, 0x7100, (offset - (s32)D_80122B74) >> 6);
                }
            }
            else
            {
                func_800BD520(0, 0x7100, 0xFE);
            }
            break;
        case 3:
        {
            s32 offset = (g_gosub_result_values[0] << 6) + 0xCE0;
            func_800BEC44(D_80122B74 + offset, g_gosub_result_values[1]);
            func_800BD520(0, 0x7100, g_gosub_result_values[0]);
            break;
        }
        default:
            handle = func_800A9060();
            if (handle != NULL)
            {
                func_800BE888((s32)handle, arg0, g_gosub_result_values[0], g_gosub_result_values[1]);
                {
                    s32 offset = (s32)handle - 0xCE0;
                    func_800BD520(0, 0x7100, (offset - (s32)D_80122B74) >> 6);
                }
            }
            else
            {
                func_800BD520(0, 0x7100, 0xFE);
            }
            break;
    }
    return;

count_zero:
    func_800BD520(0, 0x7100, 0xFF);
}
