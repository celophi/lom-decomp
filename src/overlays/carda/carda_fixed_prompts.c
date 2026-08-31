#include "common.h"

typedef struct
{
    s32 unk0;
    s16 unk4;
    s16 unk6;
    u8 unk8[0x18];
} CardaFileHeaderScratch;

extern CardaFileHeaderScratch D_801401C0;
extern s32 D_801660A0;
extern char D_800ECF9C[];
extern char D_800ECFB0[];

extern void func_80016F9C(void *, void *);
extern void func_8001686C(void *);

void func_80147E88(void)
{
    CardaFileHeaderScratch buf;

    memcpy(&buf, &D_801401C0, 6);
    ((u8 *)&buf)[2] += *(u8 *)&D_801660A0;
    func_80016F9C(&buf, &D_800ECF9C);
    func_8001686C(&buf);

    memcpy(&buf, &D_801401C0, 6);
    ((u8 *)&buf)[2] += *(u8 *)&D_801660A0;
    func_80016F9C(&buf, &D_800ECFB0);
    func_8001686C(&buf);
    func_80149FEC();
    func_8014A044();
}
