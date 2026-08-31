#include "common.h"

typedef struct AddheroElement {
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
    void *draw_handler;
    s32 unkC;
} AddheroElement;

extern AddheroElement D_80160940;
extern s32 D_80165200;
extern s32 D_80160934;
extern s32 D_801609B8;
extern s32 D_80164A40;
extern s32 D_801609A4;
extern void *D_80165488;
extern s32 D_801609E8;

void func_800A3938();
void func_800AA02C();
void func_801449F0(void);
s32 func_80142CE8(s32 *ot, s32 prim, s32 arg2, s32 arg3);

#define SET_ELEM_CODE(e, c) ((e)->attr.word = ((e)->attr.word & 0x00FFFFFF) | ((u32)(c) << 24))

void func_80142B1C(s32 arg0)
{
    func_800A3938(0x78, 0x80);
    D_80160940.draw_handler = (void *)func_80142CE8;
    D_80160940.attr.f.unk0_3 = 1;
    D_80160940.attr.f.state = 1;
    D_80160940.attr.f.x = 0x20;
    D_80160940.attr.f.unk0_16 = 0x70;
    D_80160940.unk4_0 = 1;
    D_80160940.y = 0x14;
    SET_ELEM_CODE(&D_80160940, 0);
    func_800AA02C();
    D_80165200 = 0;
    D_80160934 = 0;
    D_801609B8 = 0;
    D_80164A40 = 0;
    D_801609A4 = 0xFF;
    func_801449F0();
    D_80165488 = 0;
    D_801609E8 = arg0;
}

extern AddheroElement D_8016094C;
s32 func_80142E6C(s32 *ot, s32 prim, s32 arg2, s32 arg3);

/**
 * @brief Initialize the secondary choice element and reset its transition state.
 * @see decomp.me (100%)
 */
void func_80142C08(s32 arg0)
{
    func_800A3938(0x78, 0x80);
    D_8016094C.draw_handler = (void *)func_80142E6C;
    D_8016094C.attr.f.unk0_3 = 1;
    D_8016094C.attr.f.state = 1;
    D_8016094C.attr.f.x = 0x20;
    D_8016094C.attr.f.unk0_16 = 0x70;
    D_8016094C.unk4_0 = 1;
    D_8016094C.y = 0x14;
    SET_ELEM_CODE(&D_8016094C, 0);
    func_800AA02C();
    D_80165200 = 0;
    D_80160934 = 0;
    D_801609B8 = 0;
    D_80164A40 = 0;
    func_801449F0();
    D_80165488 = 0;
    D_801609E8 = arg0;
}
