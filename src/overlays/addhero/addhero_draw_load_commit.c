#include "common.h"

typedef struct { s16 x; s16 y; s16 w; s16 h; } RECT;

/**
 * @brief One "addhero" draw/cursor packet.
 * @note Field layout mirrors the AddheroElement struct in
 *       addhero_draw_load_prompt.c; only the field offsets matter for codegen.
 */
typedef struct {
    union {
        u32 word;
        struct {
            u32 state : 3;
            u32 unk0_3 : 4;
            u32 x : 9;
            u32 unk0_16 : 8;
        } f;
    } attr;
    u32 unk4_0 : 1;
    u32 y : 8;
    u32 unk4_9 : 23;
    void (*draw_handler)();
} AddheroPacket;

extern u8 *D_8012271C;
extern u16 D_80146FD6;
extern s32 D_8016092C;
extern s32 D_80160934;
extern AddheroPacket D_80160940;
extern u8 D_801609F0[];

s32 func_80142A0C(s32 result, s32 *ot);
void func_80142B1C(s32 arg);
s32 func_80144140(u8 *base);

/**
 * @brief Draw the "loading new hero" prompt and, once the load has finished,
 *        validate and commit the freshly loaded save data.
 *
 * Draws three prompt sprites plus the timer bar every frame. When the pending
 * load has completed (D_80160934 == 0) it validates the loaded resource; on
 * success it plays a sound, copies the loaded save block into the live save
 * RAM (preserving one flag bit), refreshes the cached name/id fields, retires
 * every active cursor packet, and raises the commit flag D_8016092C.
 *
 * @param ot   Ordering table the prompt primitives are linked into.
 * @param prim Current primitive pointer / index within the ordering table.
 * @param arg2 Horizontal offset used to place the prompt (screen X = 0x90 - arg2).
 * @param arg3 Vertical offset used to place the prompt rows.
 * @return The updated primitive pointer / index after linking the prompt.
 * @see addhero_draw_load_prompt.c (func_80142618) which installs this handler.
 */
s32 func_8014280C(s32 *ot, s32 prim, s32 arg2, s32 arg3)
{
    RECT pos;
    u8 *base;
    u8 *resource;
    AddheroPacket *p;
    AddheroPacket *cursor;
    s32 result;
    s32 x;
    s32 i;
    u32 saved;

    x = -arg2 + 0x90;
    result = func_800A88A0(prim, ot, (void *)((s32)&D_80146FD6 - 0x32 + D_80146FD6), 4, x, -arg3, 2);
    base = (u8 *)&D_80146FD6 - 0x32;
    result = func_800A88A0(result, ot, base + *(u16 *)(base + 0x1E), 4, x, 0xE - arg3, 2);
    result = func_800A88A0(result, ot, base + *(u16 *)(base + 0xB2), 4, x, 0x1C - arg3, 2);
    result = func_80142A0C(result, ot);

    if (D_80160934 == 0)
    {
        resource = D_801609F0;
        p = &D_80160940;
        p->attr.f.state = 0;
        if (func_80144140(resource) == 0)
        {
            func_80142B1C(4);
            return result;
        }

        func_800A3938(0x7B, 0x80);
        saved = D_8012271C[0x858] >> 7;
        func_80016E7C(resource + 0x770, D_8012271C + 0x840, 0x250);
        *(u32 *)(D_8012271C + 0x858) = (*(u32 *)(D_8012271C + 0x858) & ~0x80) | (saved << 7);
        *(u16 *)(D_8012271C + 0xD8) = *(u16 *)(resource + 0x254);
        *(u16 *)(D_8012271C + 0xDA) = *(u16 *)(resource + 0x256);
        *(u16 *)(D_8012271C + 0xDE) = 1;
        func_80067F28();

        cursor = p;
        for (i = 0; i < 8; i++, cursor++)
        {
            if (cursor->attr.f.state != 0)
            {
                cursor->attr.f.state = 3;
                cursor->attr.f.unk0_3 = 8;
            }
        }
        func_80067F5C(8);
        D_8016092C = 1;
    }

    return result;
}
