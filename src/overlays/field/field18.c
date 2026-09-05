#include "common.h"

extern u32 g_field_resource_blob[];
extern u8 *D_801058D4;
extern u32 *D_801058D8;

/**
 * @brief Reset the field resource cursor pair to the base blob and its end.
 * @note D_801058D8 points at the blob base; D_801058D4 points one past the
 *       payload, using the length word stored at g_field_resource_blob[1].
 */
void func_80083948(void)
{
    D_801058D8 = g_field_resource_blob;
    D_801058D4 = (u8 *)g_field_resource_blob + g_field_resource_blob[1];
}
