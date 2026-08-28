#include "common.h"

typedef struct
{
    s32 unk0;
    s16 unk4;
    s16 unk6;
    u8 unk8[0x18];
} AddheroFileHeaderScratch;

extern AddheroFileHeaderScratch D_80140090;
extern s32 D_801609A8;
extern char D_800ECF9C[];
extern char D_800ECFB0[];
extern void func_80016F9C(void *, void *);
extern void func_8001686C(void *);

void func_80144B74(void)
{
    AddheroFileHeaderScratch buf;

    memcpy(&buf, &D_80140090, 6);
    ((u8 *)&buf)[2] += *(u8 *)&D_801609A8;
    func_80016F9C(&buf, &D_800ECF9C);
    func_8001686C(&buf);

    memcpy(&buf, &D_80140090, 6);
    ((u8 *)&buf)[2] += *(u8 *)&D_801609A8;
    func_80016F9C(&buf, &D_800ECFB0);
    func_8001686C(&buf);
}
