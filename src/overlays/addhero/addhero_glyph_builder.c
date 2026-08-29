#include "common.h"

typedef struct { s16 x; s16 y; s16 w; s16 h; } RECT;
typedef struct {
    s32 unk0; s32 unk4; s16 unk8; s16 unkA; u8 unkC; u8 unkD; s16 unkE;
    s16 unk10; s16 unk12; u8 unk14; u8 unk15; s16 unk16; s16 unk18;
    s16 unk1A; u8 unk1C; u8 unk1D; u8 pad1E[2]; s16 unk20; s16 unk22;
    u8 unk24; u8 unk25; u8 pad26[2];
} GlyphPrim;
extern u8 D_801609C8[];
extern s32 D_801609C0;
extern s32 D_80147658[];
void func_800A5638(void *buf, s32 arg1);
void func_800A55E4(void *buf, s32 arg1);
void func_80019A34(RECT *rect, void *str);
void func_80019788(s32 arg0);

s32 func_80143DC0(s32 result, s32 *ot, s32 x, s32 y, s32 adjust, s32 slot, s32 i, s32 j)
{
    RECT rect;
    s32 temp;
    s8 shade;

    if (slot == 0x7F) return result;
    rect.x = i * 0x10;
    rect.y = 0x1F2;
    rect.w = 0x10;
    rect.h = 1;
    if ((j == 1) && (slot < 2)) {
        func_800A5638(D_801609C8, slot);
        func_80019A34(&rect, D_801609C8);
        func_80019788(0);
    } else if (slot >= 0x4F) {
        func_800A55E4(D_801609C8, D_801609C0);
        func_80019A34(&rect, D_801609C8);
        func_80019788(0);
    } else {
        func_80019A34(&rect, (void *)((u8 *)&D_80147658 - 4 + D_80147658[slot]));
    }
    temp = i * 3;
    rect.x = temp * 4 + 0x140;
    rect.y = 0xD0;
    rect.w = 0xC;
    rect.h = 0x30;
    func_80019A34(&rect, (void *)((u8 *)&D_80147658 + 0x1C + D_80147658[slot]));
    ((GlyphPrim *)result)->unk4 = 0x808080;
    ((u8 *)result)[3] = 9;
    ((u8 *)result)[7] = 0x2C;
    ((GlyphPrim *)result)->unk18 = x;
    ((GlyphPrim *)result)->unk8 = x;
    ((GlyphPrim *)result)->unk12 = y;
    ((GlyphPrim *)result)->unkA = y;
    ((GlyphPrim *)result)->unk20 = x + adjust;
    shade = temp * 0x10;
    ((GlyphPrim *)result)->unk1C = shade;
    ((GlyphPrim *)result)->unkC = shade;
    shade += 0x2F;
    ((GlyphPrim *)result)->unk24 = shade;
    ((GlyphPrim *)result)->unk14 = shade;
    ((GlyphPrim *)result)->unk15 = 0xD0;
    ((GlyphPrim *)result)->unkD = 0xD0;
    ((GlyphPrim *)result)->unk10 = x + adjust;
    ((GlyphPrim *)result)->unk22 = y + 0x2F;
    ((GlyphPrim *)result)->unk1A = y + 0x2F;
    ((GlyphPrim *)result)->unk25 = 0xFF;
    ((GlyphPrim *)result)->unk1D = 0xFF;
    ((GlyphPrim *)result)->unkE = (i & 0x3F) | 0x7C80;
    ((GlyphPrim *)result)->unk16 = 5;
    ((GlyphPrim *)result)->unk0 = (((GlyphPrim *)result)->unk0 & 0xFF000000) | (*ot & 0xFFFFFF);
    *ot = (*ot & 0xFF000000) | (result & 0xFFFFFF);
    return result + 0x28;
}
