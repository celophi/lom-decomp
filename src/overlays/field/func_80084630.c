#include "common.h"

typedef union { s32 w; struct { u32 low15:15; u32 b15:1; u32 hi16:16; } b; } W174;
typedef union { s32 w; struct { u32 b0:1; u32 p1:4; u32 b5:1; u32 b6:1; u32 b7:1; u32 rest:24; } b; } W178;
typedef struct {
 u8 pad0[0xC]; s32 unkC; u8 pad10[0x3C - 0x10]; s32 unk3C; u8 pad40[0x64 - 0x40]; s32 unk64;
 u8 pad68[0x16F - 0x68]; u8 unk16F; u8 pad170[0x174 - 0x170]; W174 unk174; W178 unk178; s32 unk17C;
 u8 pad180[0x18E - 0x180]; u8 unk18E; u8 pad18F[0x1A8 - 0x18F]; u8 unk1A8,unk1A9,unk1AA,unk1AB;
 u8 pad1AC[0x23C - 0x1AC];
} Slot;
typedef struct {u8 pad0[0xE]; u8 unkE,unkF,unk10; u8 pad11[0x48 - 0x11];} Part;
typedef struct {u8 pad0[0x259]; u8 unk259; u8 pad25A[0x268 - 0x25A];} FD;
extern Slot D_80105AE0[]; extern Part D_800FE3A0[]; extern FD D_800FD818[]; extern s32 D_8010A000;
void func_80084630(void)
{
 s32 i; u8 v; u8 ff;
 i=0;
 do {
   D_80105AE0[i].unk1A8 = D_800FE3A0[i].unkE;
   D_80105AE0[i].unk1A9 = D_800FE3A0[i].unkF;
   v = D_800FE3A0[i].unk10;
   D_80105AE0[i].unk1AB = 0;
   D_80105AE0[i].unkC = 0;
   D_80105AE0[i].unk17C = 0;
   D_80105AE0[i].unk3C = 0;
   D_80105AE0[i].unk64 = 0;
   D_80105AE0[i].unk16F = 0;
   D_80105AE0[i].unk18E = 0;
   D_80105AE0[i].unk174.b.b15 = 0;
   D_80105AE0[i].unk178.b.b7 = 0;
   D_80105AE0[i].unk178.b.b0 = 0;
   D_80105AE0[i].unk1AA = v;
   D_80105AE0[i].unk178.b.b5 = 0;
   D_80105AE0[i].unk178.b.b6 = 0;
   i++;
 } while (i<13);
 ff=0xFF;
 i=2;
 do { D_800FD818[i].unk259=ff; i--; } while(i>=0);
 D_8010A000=0xFF;
}
