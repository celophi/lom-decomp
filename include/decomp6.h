#ifndef _DECOMP6_H
#define _DECOMP6_H

#include "common.h"

extern s32 D_801ED7A4;

typedef struct arg0_struct
{
  u8 pad0[0x20];
  u8 unk20;
  u8 unk21;
  u16 unk22;
  u16 unk24;
  u16 unk26;
  s16 unk28;
  s16 unk2A;
  s16 unk2C;
  s16 unk2E;
  u8 pad1[0x90 - 0x30];
  u8 unk90;
  u8 unk91;
  u16 unk92;
  u8 unk94;
  u8 unk95;
  u8 unk96;
  u8 unk97;
  u8 unk98;
  u8 unk99;
  u8 unk9A;
  u8 unk9B;
  u8 unk9C;
  u8 unk9D;
  u8 unk9E;
  u8 unk9F;
  u8 unkA0;
  u8 unkA1;
  u8 unkA2;
  u8 unkA3;
  u8 unkA4;
  u8 unkA5;
  u8 unkA6;
  u8 unkA7;
  u8 unkA8;
  u8 unkA9;
  u8 unkAA;
  u8 unkAB;
  u8 unkAC;
  u8 unkAD;
} arg0_struct;

extern void func_80015674(void);
extern void func_80015708(void *);
extern void func_8002E958(int, int, int, int);
extern void func_80030DF8(void *, void *);
extern u32 func_8002E9E4(u8);
extern s32 func_8002EAB0(u8, u8, s32);
extern int func_8002EBA8(u8, s32, u8);
extern void func_8002ED24(u8, void *);
extern void func_8002ED5C(u8, s32, u8);
extern void func_8002EDA4(u8, void *, int);

#endif