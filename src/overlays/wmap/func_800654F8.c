#include "common.h"
#include "sdk/libgpu.h"

/** @brief World-map rendering buffer and its preallocated polygon packets. */
typedef struct
{
    u8 pad_000[0x340];
    POLY_FT4 tiles[26 * 26];
    POLY_F4 fade[184];
    u8 tail[0x20];
} WmapRenderBuffer;

extern WmapRenderBuffer D_80129560[2];
extern WmapRenderBuffer *D_801398EC;
extern s32 D_801ADAE8;

/** @brief Initialize both map polygon buffers and update the fade packet colors. */
void func_800654F8(void)
{
    s32 buffer_index;
    s32 row;
    s32 column;
    WmapRenderBuffer *buffer;
    POLY_FT4 *tile;
    POLY_F4 *fade;

    for (buffer_index = 0; buffer_index < 2; buffer_index++)
    {
        buffer = &D_80129560[buffer_index];
        *(s16 *)(buffer->tail + 0x16) = 0x45;
        D_801398EC = buffer;
        *(s32 *)(buffer->tail + 0x18) = 0;
        *(s32 *)(buffer->tail + 0x10) = 0;
        *(s32 *)(buffer->tail + 8) = 0;
        *(s32 *)(buffer->tail + 4) = 0;
        buffer->tail[3] = 7;
        D_801398EC->tail[7] = 0x24;
        for (row = 0; row < 26; row++)
        {
            for (column = 0; column < 26; column++)
            {
                tile = &D_801398EC->tiles[row * 26 + column];
                *(s32 *)&tile->r0 = 0;
                tile->tpage = 9;
                tile->clut = 0x6024;
                ((u8 *)tile)[3] = 9;
                tile->code = 0x2C;
            }
        }
        if (D_801ADAE8 != 0)
        {
            if (D_801ADAE8 < 64)
            {
                D_801ADAE8++;
            }
            for (column = 0; column < 184; column++)
            {
                fade = &D_801398EC->fade[column];
                fade->r0 = D_801ADAE8;
                fade->g0 = D_801ADAE8;
                fade->b0 = D_801ADAE8;
                ((u8 *)fade)[3] = 5;
                fade->code = 0x2A;
            }
        }
    }
}
