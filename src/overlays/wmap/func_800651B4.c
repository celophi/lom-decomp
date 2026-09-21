#include "common.h"
#include "sdk/libgpu.h"

/** @brief Unaligned eight-byte rectangle in a texture block. */
typedef struct
{
    u8 bytes[8];
} WmapTextureRect;

/**
 * @brief Upload an optional palette and the enabled pixel data from a TIM resource.
 * @param data Texture resource header.
 */
void func_800651B4(u8 *data)
{
    WmapTextureRect rectangle;

    if (data[4] & 8)
    {
        data += 8;
        rectangle = *(WmapTextureRect *)(data + 4);
        LoadImage((RECT *)&rectangle, (u_long *)(data + 12));
        data += *(s32 *)data;
        rectangle = *(WmapTextureRect *)(data + 4);
        if (((RECT *)&rectangle)->x != -1)
        {
            LoadImage((RECT *)&rectangle, (u_long *)(data + 12));
            DrawSync(0);
        }
    }
    else
    {
        data += 8;
        rectangle = *(WmapTextureRect *)(data + 4);
        if (((RECT *)&rectangle)->x != -1)
        {
            LoadImage((RECT *)&rectangle, (u_long *)(data + 12));
            DrawSync(0);
        }
    }
}
