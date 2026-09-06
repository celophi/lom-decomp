#include "common.h"
#include "sdk/libgpu.h"

typedef struct
{
    u8 pad0[0x8];
    s32 unk8;
    u8 pad12[4];
    u16 unk10;
    u16 unk12;
    u8 unk14[1];
} FieldImgHeader;

typedef struct
{
    u8 pad0[0x8];
    u16 unk8;
    u16 unkA;
    u8 unkC[1];
} FieldImgSub;

/**
 * @brief Load a field VRAM image, optionally preceded by a header strip.
 * @param arg0 Pointer to a FieldImgHeader; unk8 is the size of the header's
 *        variable-length data, unk10/unk12 its width/height, and unk14 the
 *        start of its pixel data.
 * @param arg1 Frame/page index selecting the destination VRAM x/y offset.
 * @param arg2 Extra Y offset added when the header strip is loaded.
 * @param arg3 When non-zero, also loads the header strip before the main
 *        image.
 */
void func_8009BCF8(void *arg0, s32 arg1, s32 arg2, s32 arg3)
{
    RECT rect;
    u16 width;
    u16 height;
    s32 size;

    width = ((FieldImgHeader *) arg0)->unk10;
    height = ((FieldImgHeader *) arg0)->unk12;
    size = ((FieldImgHeader *) arg0)->unk8;
    if (arg3 != 0)
    {
        rect.y = arg2 + 0x1F4;
        rect.x = 0;
        rect.h = 1;
        rect.w = width * height;
        LoadImage(&rect, (u_long *) &((FieldImgHeader *) arg0)->unk14);
    }

    arg0 = (u8 *) arg0 + size + 8;
    width = ((FieldImgSub *) arg0)->unk8;
    height = ((FieldImgSub *) arg0)->unkA;
    if (arg1 >= 0xA)
    {
        rect.x = 0x3C0 - ((arg1 - 9) << 6);
        rect.y = 0x100;
    }
    else
    {
        rect.x = 0x340 - (arg1 << 6);
        rect.y = 0;
    }
    do
    {
        rect.w = width;
    } while (0);
    rect.h = height;
    LoadImage(&rect, (u_long *) &((FieldImgSub *) arg0)->unkC);
    DrawSync(0);
}
