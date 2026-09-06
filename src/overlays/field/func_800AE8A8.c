#include "common.h"

s32 func_800AEAC0(s32 handle, void *arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5);
s32 func_800A88A0(void *arg0, void *arg1, void *arg2, s32 arg3, s32 arg4, s32 arg5, s32 arg6);

extern s32 D_80122698;
extern u8 *D_801228F8[];

/**
 * @brief Draw the coordinate panel and its optional label rows.
 * @param arg0 Ordering-table context passed to each label draw.
 * @param arg1 Initial packet handle.
 * @param arg2 Horizontal panel offset.
 * @param arg3 Vertical panel offset.
 * @return The final packet handle after all enabled rows are emitted.
 */
s32 func_800AE8A8(void *arg0, s32 arg1, s32 arg2, s32 arg3)
{
    s32 handle;
    s32 pad[2];

    handle = func_800AEAC0(arg1, arg0, D_80122698, 2 - arg2, -arg3, 1);
    if (D_801228F8[0] != 0)
    {
        handle = func_800A88A0((void *)handle, arg0, D_801228F8[0], 4, 0x38 - arg2, 1 - arg3, 0);
    }
    if (D_801228F8[1] != 0)
    {
        handle = func_800A88A0((void *)handle, arg0, D_801228F8[1], 4, 0x38 - arg2, 0x11 - arg3, 0);
    }
    if (D_801228F8[2] != 0)
    {
        handle = func_800A88A0((void *)handle, arg0, D_801228F8[2], 4, 0x38 - arg2, 0x21 - arg3, 0);
    }
    return handle;
}
