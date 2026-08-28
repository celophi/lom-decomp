#include "common.h"

typedef struct
{
    s32 unk0;
    s16 unk4;
    s16 unk6;
    u8 unk8[0x18];
} NikiFileHeaderScratch;

extern NikiFileHeaderScratch D_80140090;
extern s32 D_80164B70;
extern char D_800ECF9C[];
extern char D_800ECFB0[];

extern void func_80016F9C(void *, void *);
extern void func_8001686C(void *);

void func_80144D44(void)
{
    NikiFileHeaderScratch buf;

    memcpy(&buf, &D_80140090, 6);
    ((u8 *)&buf)[2] += *(u8 *)&D_80164B70;
    func_80016F9C(&buf, &D_800ECF9C);
    func_8001686C(&buf);

    memcpy(&buf, &D_80140090, 6);
    ((u8 *)&buf)[2] += *(u8 *)&D_80164B70;
    func_80016F9C(&buf, &D_800ECFB0);
    func_8001686C(&buf);
}
