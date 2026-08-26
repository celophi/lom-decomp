#include "common.h"

typedef struct
{
    u8 pad0[0x3160];
    u8 unk3160;   /* 0x3160 */
    u8 pad3161[0x3194 - 0x3161];
    s32 unk3194;  /* 0x3194 */
} BigStruct;

extern u8 D_80122C02;
extern u8 D_80046138[];

extern void func_800A8F8C(u8 *dst, u8 *src);
extern u8 *func_800A9060(void);
extern void func_800C8E2C(void);

void func_800C8F4C(void)
{
    s32 idx;
    s32 offset;
    u8 *handle;
    BigStruct *rec;

    idx = D_80122C02;
    handle = func_800A9060();
    offset = idx << 6;
    func_800A8F8C(handle, &D_80046138[offset]);
    rec = (BigStruct *) (D_80046138 - 0x3160 + offset);
    rec->unk3160 = 0;
    rec->unk3194 = 0;
    func_800C8E2C();
}
