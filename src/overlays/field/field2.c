#include "common.h"

typedef struct Unk Unk;
struct Unk
{
  s32 unk0;
  s32 unk1;
  s32 unk2;
  s32 unk4;
  s32 unk6;
  s32 unk8;
  s32 unk9;
  s32 unkA;
  s32 unkB;
  s32 unkC;
  s32 unkD;
  s32 unkE;
  s32 unkF;
  s32 unk10;
  s32 unk12;
  s32 unk14;
  s32 unk16;
  s32 unk18;
  s32 unk1A;
  s32 unk1C;
  s32 unk1E;
  s32 unk1F;
  s32 unk20;
  s32 unk21;
  s32 unk22;
  s32 unk24;
  s32 unk26;
  s32 unk28;
  s32 unk2C;
  s32 unk30;
  s32 unk34;
  s32 unk36;
  s32 unk38;
  s32 unk3A;
  s32 unk3C;
  s32 unk3E;
  s32 unk40;
  s32 unk42;
  s32 unk44;
  s32 unk46;
  s32 unk48;
  s32 unk4A;
  s32 unk1F800000;
};
typedef struct Node44 Node44;
struct Node44
{
  Node44 *unk0;
  void *unk4;
  u8 pad08[0x08];
  s32 unk10;
  s32 unk14;
  s8 unk18;
  u8 pad19[0x03];
  s16 unk1C;
  s16 unk1E;
  s16 unk20;
  s16 unk22;
  s32 unk24;
  s32 unk28;
  s32 unk2C;
  s32 unk30;
  s32 unk34;
  s32 unk38;
  s32 unk3C;
  s32 unk40;
};
typedef struct SrcObj SrcObj;
struct SrcObj
{
  SrcObj *unk0;
  u8 unk4;
  u8 pad05[0x13];
  u16 unk18;
};
typedef struct ObjArg ObjArg;
struct ObjArg
{
  void *unk0;
  u8 pad04[0x04];
  SrcObj *unk8;
  void *unkC;
  u8 pad10[0x04];
  s32 unk14;
  s32 unk18;
  s32 unk1C;
  s32 unk20;
  u8 pad24[0x02];
  s16 unk26;
};
typedef struct SrcObj2 SrcObj2;
struct SrcObj2
{
  SrcObj2 *unk0;
  u16 unk4;
  u16 unk6;
  s16 unk8;
  u16 unkA;
  u16 unkC;
  u16 unkE;
};
typedef struct Node38 Node38;
struct Node38
{
  Node38 *unk0;
  void *unk4;
  s16 unk8;
  s16 unkA;
  s16 unkC;
  s16 unkE;
  s16 unk10;
  s16 unk12;
  s16 unk14;
  s16 unk16;
  s32 unk18;
  s32 unk1C;
  s32 unk20;
  s32 unk24;
  s32 unk28;
  s32 unk2C;
  s32 unk30;
  s32 unk34;
};
typedef struct SrcObj3 SrcObj3;
struct SrcObj3
{
  SrcObj3 *unk0;
  u8 pad04[0x08];
  u8 unkC;
  u8 pad0D[0x03];
  u16 unk10;
  u16 unk12;
  u16 unk14;
  s16 unk16;
  s16 unk18;
  s16 unk1A;
  u8 unk1C;
  u8 unk1D;
  u8 unk1E;
  u8 unk1F;
};
typedef struct InnerNode InnerNode;
struct InnerNode
{
  InnerNode *unk0;
  void *unk4;
  InnerNode *unk8;
  s32 *unkC;
  void *unk10;
  s32 unk14;
  s32 unk18;
  s32 unk1C;
  u8 pad20;
  u8 unk21;
  u8 pad22[4];
  u16 unk26;
};
typedef struct Src4 Src4;
struct Src4
{
  s32 unk0;
  u8 pad04[4];
  s32 unk8;
};
typedef struct Node30 Node30;
struct Node30
{
  Node30 *unk0;
  void *unk4;
  InnerNode *unk8;
  u8 unkC;
  u8 unkD;
  u8 unkE;
  u8 unkF;
  u16 unk10;
  u16 unk12;
  u16 unk14;
  s16 unk16;
  s16 unk18;
  s16 unk1A;
  s32 unk1C;
  s32 unk20;
  s32 unk24;
  s32 unk28;
  s32 unk2C;
};
typedef struct Node
{
  struct Node *unk0;
  u8 pad[32];
  u32 unk24;
  u32 unk28;
  u32 unk2C;
  u32 unk30;
} Node;
typedef struct
{
  u8 padding[8];
  Node *unk8;
} FieldScene;
extern Unk *g_field_scene;
extern void DecDCTReset(int mode);
extern void DecDCTvlcBuild(u_short *table);
void field_validate_and_rasterize_quads(s32, s32);
void func_80053C7C(s32, Unk **, void *);
void func_8005477C(s32, Unk *, s32, s32);
void func_80054904(s32, Unk *, s32, s32);
s32 func_80056824(void *, Unk *, Unk *, u8 *);
void func_8005A744(void *, s32);
void *func_8005AB4C(u8);
void *func_8005AB80(u8, u8);
void func_8005AC50(void *, u16, s32 *);
void func_8005AD20(u8, u16, s8 *);
extern s32 g_field_dyn_count;

/**
 * decomp.me (88.34%) https://decomp.me/scratch/i4GmA
 * THIS FUNCTION MAY NOT BE FUNCTIONALLY EQUIVALENT. BE CAUTIOUS TO MAKE ASSUMPTIONS.
 */
void field_build_render_records(ObjArg *arg0, u16 arg1)
{
  s8 sp20;
  Unk *sp24;
  s32 sp10[3];
  u16 sp28;
  Unk **sp30;
  Unk *sp34;
  Node30 *sp38;
  s32 sp3C;
  s32 sp50;
  Node30 * volatile sp54;
  s32 sp58;
  void * volatile sp5C;
  s32 sp60;
  s32 sp64;
  Unk *sp68;
  Unk **sp6C;
  s32 sp74;
  s32 sp80;
  Unk *temp_a1_5;
  Unk *temp_a1_6;
  Node44 *temp_s1;
  Unk *temp_s1_2;
  Unk *temp_s2;
  Node38 *temp_t0;
  Unk *var_s3;
  int new_var7;
  Unk *var_s7_2;
  unsigned int new_var14;
  Unk *var_s7_3;
  char new_var3;
  Node30 *var_t0_2;
  Node30 *var_t0_3;
  Node30 *var_t0_4;
  Node44 *var_t1;
  Node38 *var_t1_2;
  Unk *var_t1_5;
  Unk *var_t5;
  s16 *var_t5_2;
  s16 temp_a1;
  s16 temp_a1_2;
  s16 temp_a1_3;
  s16 temp_a1_4;
  s16 temp_a2;
  s32 temp_v0;
  u8 *new_var12;
  int new_var;
  s32 temp_v0_2;
  s32 temp_v0_3;
  s32 temp_v0_4;
  s32 var_a1_2;
  u8 *new_var2;
  s32 var_a1_3;
  s16 var_t1_3;
  s16 var_v0_2;
  s16 var_v0_3;
  s16 var_v0_4;
  s16 var_v0_5;
  s32 var_v1;
  s32 var_v1_2;
  s16 *var_a1;
  s16 *var_a2;
  s32 temp_a0;
  s32 temp_a2_3;
  s32 temp_a2_4;
  s32 temp_t1;
  s32 temp_t1_2;
  s32 temp_t3;
  s32 temp_t3_2;
  s32 temp_t4;
  s32 temp_t4_2;
  s32 temp_t7;
  s32 temp_t7_2;
  s32 temp_v0_10;
  s32 temp_v0_11;
  s32 temp_v0_12;
  s32 temp_v0_13;
  s32 temp_v0_5;
  s32 temp_v0_6;
  s32 temp_v1_13;
  s32 temp_v1_14;
  int new_var6;
  s32 temp_v1_7;
  s32 temp_v1_8;
  s32 var_fp_2;
  s32 var_s0;
  s32 var_s0_2;
  s32 var_s0_3;
  u8 *new_var13;
  s32 var_s0_4;
  s32 var_s0_5;
  s32 var_s0_6;
  s32 var_s0_7;
  s32 var_s0_8;
  s32 var_s1;
  s32 var_s1_3;
  s32 var_s1_4;
  s32 var_s5;
  s32 var_s5_2;
  s32 var_s6;
  s32 var_s6_2;
  s32 var_s6_3;
  s32 var_t1_4;
  s16 temp_a2_2;
  s32 var_t3;
  s32 var_t4;
  u16 *new_var9;
  s32 var_t8;
  s32 var_v0_7;
  int new_var18;
  s32 var_v1_3;
  s32 var_v1_4;
  s32 var_v1_5;
  s32 var_v1_6;
  s32 var_v1_7;
  unsigned char new_var10;
  s32 var_v1_8;
  s32 *var_s3_2;
  s32 *var_s3_3;
  u16 temp_v1;
  int new_var11;
  u16 temp_v1_2;
  u16 temp_v1_3;
  u16 temp_v1_4;
  u16 temp_v1_5;
  u16 temp_v1_6;
  u32 new_var19;
  u16 var_a0;
  u16 var_a0_2;
  u16 var_a0_3;
  u16 var_a0_4;
  u16 var_a0_5;
  u16 var_a0_6;
  u16 var_t2;
  Node38 *new_var16;
  u8 *new_var17;
  u16 var_t2_2;
  u16 *temp_a0_2;
  u16 *var_t0;
  u32 temp_a0_3;
  u32 temp_a0_4;
  u32 var_v0_6;
  u32 var_v0_8;
  SrcObj3 **new_var4;
  u8 temp_v0_7;
  short temp_v1_12;
  u8 temp_v1_9;
  u16 new_var5;
  int var_v0;
  Unk *var_a0_7;
  int new_var8;
  u8 *var_fp;
  Unk *temp_a3;
  Unk *temp_s4;
  Src4 *temp_s4_2;
  Unk *temp_t2;
  Unk *temp_v0_8;
  Unk *temp_v0_9;
  Unk *temp_v1_10;
  InnerNode *temp_v1_11;
  SrcObj *var_a3;
  SrcObj2 *var_a3_2;
  Unk *var_s1_2;
  Unk *var_s1_5;
  unsigned char new_var20;
  InnerNode *var_s2;
  s32 var_s2_2;
  SrcObj3 *var_t2_3;
  u8 *new_var15;
  Unk *var_t2_4;
  Unk **var_s7;
  SrcObj3 **var_t6;
  sp30 = (Unk **) 0x801ED000;
  var_t3 = 0;
  var_t4 = 0;
  var_t8 = 0;
  sp80 = 0;
  do
  {
  }
  while (0);
  sp3C = 0;
  sp24 = 0;
  sp34 = g_field_scene;
  g_field_scene->unk0 = arg0;
  *((s32 *) (((u8 *) g_field_scene) + 0xC)) = 0;
  var_a3 = arg0->unk8;
  sp24 = (Unk *) (((u8 *) g_field_scene) + 0x74);
  sp28 = arg1;
  var_t1 = (Node44 *) (((u8 *) g_field_scene) + 8);
  var_t5_2 = *((s16 **) (((u8 *) (&g_field_scene)) + 8));
  if (arg0->unk8 != (0 * 0))
  {
    do
    {
      temp_s1 = sp24;
      sp24 = (Unk *) (((u8 *) temp_s1) + 0x44);
      do
      {
      }
      while (0);
      var_t1->unk0 = temp_s1;
      var_t1 = temp_s1;
      var_t1->unk4 = var_a3;
      var_t1->unk10 = 0;
      var_t1->unk14 = 0;
      var_a0 = (u8) var_a3->unk4;
      var_t1->unk1C = 0x7FFF;
      var_t1->unk1E = 0;
      var_t1->unk20 = 0;
      var_t1->unk22 = 0x7FFF;
      var_t1->unk24 = 0;
      var_t1->unk28 = 0;
      var_t1->unk2C = 0;
      var_t1->unk30 = 0;
      new_var14 = var_a0 >> 7;
      var_t1->unk34 = 0;
      var_t1->unk38 = 0;
      var_t1->unk3C = 0;
      var_t1->unk40 = 0;
      var_t1->unk18 = (s8) new_var14;
      var_s0 = var_a3->unk18;
      var_s0 &= 0x7FFF;
      var_t0 = (u16 *) (((u8 *) var_a3) + 0x18);
      if (var_s0 != 0)
      {
        do
        {
          var_s0_2 = var_s0 - 1;
          new_var = -1;
          var_a1 = var_t5_2 + (var_t0[1] * 2);
          do
          {
            if (var_s0_2 != new_var)
            {
              var_a2 = var_a1 + 1;
              do
              {
                var_a0 = (u16) (*var_a1);
                if (temp_s1->unk1C < (*var_a1))
                {
                  var_a0 = (u16) temp_s1->unk1C;
                }
                temp_s1->unk1C = (s16) var_a0;
                var_a0_2 = (u16) (*var_a1);
                if ((*var_a1) < temp_s1->unk1E)
                {
                  var_a0_2 = (u16) temp_s1->unk1E;
                }
                temp_s1->unk1E = (s16) var_a0_2;
                var_a0_3 = (u16) (*var_a2);
                if ((*var_a2) < temp_s1->unk20)
                {
                  var_a0_3 = (u16) temp_s1->unk20;
                }
                temp_s1->unk20 = (s16) var_a0_3;
                var_a0_4 = (u16) (*var_a2);
                if ((*var_a2) > temp_s1->unk22)
                {
                  var_a0_4 = (u16) (*temp_s1).unk22;
                }
                temp_s1->unk22 = (s16) var_a0_4;
                var_a2 += 2;
                var_s0_2 -= 1;
                var_a1 += 2;
              }
              while (var_s0_2 != new_var);
            }
            var_t0 = var_t0 + 2;
            var_s0 = (*var_t0) & 0x7FFF;
          }
          while (0);
        }
        while (var_s0 != 0);
      }
      var_a3 = var_a3->unk0;
    }
    while (var_a3 != 0);
  }
  var_t1->unk0 = 0;
  var_a3_2 = arg0->unkC;
  var_t1_2 = (Node38 *) (((u8 *) sp34) + 0x10);
  if (var_a3_2 != 0)
  {
    do
    {
      temp_t0 = sp24;
      sp24 = (Unk *) (((u8 *) temp_t0) + 0x38);
      var_t1_2->unk0 = temp_t0;
      var_t1_2 = temp_t0;
      var_t1_2->unk4 = var_a3_2;
      var_t1_2->unk8 = (s16) (var_a3_2->unk4 + var_a3_2->unkC);
      var_t1_2->unkA = (s16) (var_a3_2->unk6 + var_a3_2->unkE);
      var_t1_2->unkC = (s16) (((u16) var_a3_2->unk8) + var_a3_2->unkC);
      var_t1_2->unkE = (s16) (var_a3_2->unkA + var_a3_2->unkE);
      var_a0_5 = var_a3_2->unk4;
      temp_v1 = (u16) var_a3_2->unk8;
      goto dummy_label_805487;
      dummy_label_805487:
      ;

      ;
      ;
      ;
      ;
      ;
      ;
      ;
      ;
      ;
      ;
      ;
      ;
      ;
      ;
      ;
      ;
      ;
      var_t2 = var_a0_5;
      if (((s16) var_a0_5) < var_a3_2->unk8)
      {
        var_t2 = temp_v1;
      }
      if (var_a3_2->unk8 < ((s16) var_a0_5))
      {
        var_a0_5 = temp_v1;
      }
      temp_a1 = var_t1_2->unk8;
      temp_v1_2 = (u16) var_t1_2->unk8;
      if (((s16) var_t2) < temp_a1)
      {
        var_t2 = temp_v1_2;
      }
      if (temp_a1 < ((s16) var_a0_5))
      {
        var_a0_5 = temp_v1_2;
      }
      temp_a1_2 = var_t1_2->unkC;
      temp_v1_3 = (u16) var_t1_2->unkC;
      if (((s16) var_t2) < temp_a1_2)
      {
        var_t2 = temp_v1_3;
      }
      temp_t4_2 = (s16) var_a0_5;
      if (temp_a1_2 < temp_t4_2)
      {
        var_a0_5 = temp_v1_3;
      }
      var_t1_2->unk10 = var_t2;
      var_t1_2->unk12 = var_a0_5;
      var_a0_6 = var_a3_2->unk6;
      temp_v1_4 = var_a3_2->unkA;
      var_t2_2 = var_a0_6;
      if (((s16) var_a0_6) < ((s16) var_a3_2->unkA))
      {
        var_t2_2 = temp_v1_4;
      }
      if (((s16) var_a3_2->unkA) < ((s16) var_a0_6))
      {
        var_a0_6 = temp_v1_4;
      }
      temp_a1_3 = var_t1_2->unkA;
      temp_v1_5 = (u16) var_t1_2->unkA;
      if (((s16) var_t2_2) < temp_a1_3)
      {
        var_t2_2 = temp_v1_5;
        if (1)
        {
        }
      }
      if (temp_a1_3 < ((s16) var_a0_6))
      {
        var_a0_6 = temp_v1_5;
      }
      temp_a1_4 = temp_t0->unkE;
      temp_v1_6 = (u16) temp_t0->unkE;
      if (((s16) var_t2_2) < temp_a1_4)
      {
        var_t2_2 = temp_v1_6;
      }
      if (temp_a1_4 < ((s16) var_a0_6))
      {
        var_a0_6 = temp_v1_6;
      }
      temp_t0->unk14 = var_t2_2;
      temp_t0->unk16 = var_a0_6;
      temp_t0->unk18 = (s32) (temp_t0->unk8 - ((s16) var_a3_2->unk4));
      temp_v1_7 = temp_t0->unkA - ((s16) var_a3_2->unk6);
      temp_t0->unk1C = temp_v1_7;
      temp_a2_3 = temp_t0->unk18;
      if (temp_a2_3 != 0)
      {
        var_a1_2 = ((s16) var_a3_2->unk6) - (((s32) (temp_v1_7 * ((s16) var_a3_2->unk4))) / temp_a2_3);
        var_v1 = ((s16) var_a3_2->unkA) - (((s32) (temp_v1_7 * ((s16) var_a3_2->unk8))) / temp_a2_3);
      }
      else
      {
        var_a1_2 = (s16) var_a3_2->unk4;
        var_v1 = (s16) var_a3_2->unk8;
      }
      if (var_v1 < var_a1_2)
      {
        temp_t0->unk2C = (s32) var_a1_2;
        temp_t0->unk28 = (s32) var_v1;
      }
      else
      {
        temp_t0->unk28 = (s32) var_a1_2;
        temp_t0->unk2C = (s32) var_v1;
      }
      temp_t0->unk20 = (s32) (((s16) var_a3_2->unk8) - ((s16) var_a3_2->unk4));
      temp_v1_8 = ((s16) var_a3_2->unkA) - ((s16) var_a3_2->unk6);
      temp_t0->unk24 = temp_v1_8;
      temp_a2_4 = temp_t0->unk20;
      if (temp_a2_4 != 0)
      {
        var_a1_3 = ((s16) var_a3_2->unk6) - (((s32) (temp_v1_8 * ((s16) var_a3_2->unk4))) / temp_a2_4);
        var_v1_2 = ((s16) temp_t0->unkA) - (((s32) (temp_v1_8 * ((s16) temp_t0->unk8))) / temp_a2_4);
      }
      else
      {
        var_a1_3 = (s16) var_a3_2->unk4;
        var_v1_2 = (s16) temp_t0->unk8;
      }
      if (var_v1_2 < var_a1_3)
      {
        temp_t0->unk34 = (s32) var_a1_3;
        temp_t0->unk30 = (s32) var_v1_2;
      }
      else
      {
        temp_t0->unk30 = (s32) var_a1_3;
        temp_t0->unk34 = (s32) var_v1_2;
      }
      var_a3_2 = var_a3_2->unk0;
    }
    while (var_a3_2 != 0);
  }
  new_var16 = var_t1_2;
  new_var16->unk0 = 0;
  var_t6 = arg0->unk0;
  sp38 = (Node30 *) (((u8 *) sp34) + 4);
  if ((*(new_var4 = var_t6)) != 0)
  {
    
    do
    {
      var_t0_2 = sp24;
      var_t2_3 = *new_var4;
      sp24 = (Unk *) (((u8 *) var_t0_2) + 0x30);
      sp38->unk0 = var_t0_2;
      var_t0_2->unk4 = var_t2_3;
      new_var9 = &var_t2_3->unk10;
      var_t0_2->unk10 = (s16) (((*new_var9) << 8) / 100);
      var_t0_2->unk12 = (s16) ((var_t2_3->unk12 << 8) / 100);
      var_s0_2 = ((0, var_t2_3))->unk14 << 8;
      var_t0_2->unk1A = 0x100;
      var_t0_2->unk18 = 0x100;
      var_t0_2->unk16 = 0x100;
      var_t0_2->unk14 = (s16) (var_s0_2 / 100);
      new_var13 = &var_t0_2->unkC;
      *((s32 *) new_var13) = (s32) (((*((s32 *) new_var13)) & (~1)) | (var_t2_3->unkC & 1));
      var_t0_2->unkD = 0;
      var_t0_2->unk1C = (s32) (var_t2_3->unk16 << 8);
      var_t0_2->unk20 = (s32) (var_t2_3->unk18 << 8);
      sp38 = var_t0_2;
      var_t0_2->unk24 = (s32) (var_t2_3->unk1A << 8);
      if (((*((s32 *) (&var_t2_3->unk1C))) & 0xFFFF0000) == 0x100000)
      {
        var_t0_2->unkE = 0U;
      }
      else
      {
        var_t0_2->unkE = (u8) var_t2_3->unk1E;
      }
      sp60 = (u8) var_t2_3->unk1F;
      var_t0_2->unk28 = 0;
      var_t0_2->unk2C = 0;
      var_t0_2->unkF = sp60;
      var_s7 = var_t2_3->unk0;
      var_t5 = (Unk *) (((u8 *) var_t0_2) + 8);
      if ((*var_s7) != 0)
      {
        do
        {
          temp_s2 = sp24;
          temp_s4 = *var_s7;
          sp24 = (Unk *) (((u8 *) temp_s2) + 0x4C);
          var_t5->unk0 = temp_s2;
          *((Unk **) (((u8 *) temp_s2) + 4)) = temp_s4;
          *(((u8 *) temp_s2) + 0x20) = (u8) ((*(((u8 *) temp_s4) + 8)) & 1);
          temp_a0 = *((s32 *) (((u8 *) temp_s4) + 8));
          var_t5 = temp_s2;
          if ((temp_a0 & 0xF00) == 0x100)
          {
            var_v0 = (temp_a0 & 0xE) + 1;
          }
          else
          {
            var_v0 = temp_a0 & 0xE;
          }
          *(((u8 *) temp_s2) + (new_var18 = 0x21)) = var_v0 & 0xFFFFu;
          *(((u8 *) temp_s2) + 0x22) = 0;
          *((s32 *) (((u8 *) temp_s2) + 0x28)) = (s32) ((*((s16 *) (((u8 *) temp_s4) + 0xC))) << 8);
          if (var_s2)
          {
          }
          *((s32 *) (((u8 *) temp_s2) + 0x2C)) = (s32) ((*((s16 *) (((u8 *) temp_s4) + 0xE))) << 8);
          *((s16 *) (((u8 *) temp_s2) + 0x34)) = 0;
          *((s32 *) (((u8 *) temp_s2) + 0x30)) = (s32) ((*((s16 *) (((u8 *) temp_s4) + 0x10))) << 8);
          *((s16 *) (((u8 *) temp_s2) + 0x38)) = 1;
          new_var5 = *((u16 *) (((u8 *) temp_s4) + 0x12));
          *((s16 *) (((u8 *) temp_s2) + 0x3A)) = 0;
          *((s16 *) (((u8 *) temp_s2) + 0x3C)) = 0;
          var_v1_3 = 0x1000;
          *((s16 *) (((u8 *) temp_s2) + 0x3E)) = 0;
          *((s16 *) (((u8 *) temp_s2) + 0x40)) = ((((((((var_v1_3 & 0xFFFFFFFFFFFFFFFFu) & 0xFFFFFFFFFFFFFFFFu) & 0xFFFFFFFFFFFFFFFFu) & 0xFFFFFFFFFFFFFFFFu) & 0xFFFFFFFFFFFFFFFFu) & 0xFFFFFFFFFFFFFFFFu) & 0xFFFFFFFFFFFFFFFFu) & 0xFFFFFFFFFFFFFFFFu) & 0xFFFFFFFFFFFFFFFFu;
          *((s16 *) (((u8 *) temp_s2) + 0x42)) = var_v1_3;
          *((s16 *) (((u8 *) temp_s2) + 0x36)) = (u16) new_var5;
          if ((*((s32 *) (((u8 *) temp_s4) + 8))) & 0x40)
          {
            temp_v0 = (var_t2_3->unk1A + (*((s16 *) (((u8 *) temp_s4) + 0x10)))) + (*((s16 *) (((u8 *) temp_s4) + 0x18)));
            if (temp_v0 > 0)
            {
              var_v0_2 = temp_v0;
              if (temp_v0 >= 0x800)
              {
                var_v0_2 = 0x7FF;
              }
            }
            else
            {
              var_v0_2 = 0;
            }
            *((s16 *) (((u8 *) temp_s2) + 0x44)) = var_v0_2;
            temp_v0_2 = (var_t2_3->unk1A + (*((s16 *) (((u8 *) temp_s4) + 0x10)))) + (*((s16 *) (((u8 *) temp_s4) + 0x1A)));
            var_s1_5 = temp_s4;
            if (temp_v0_2 > 0)
            {
              var_v0_3 = temp_v0_2;
              if (temp_v0_2 >= 0x800)
              {
                var_v0_3 = 0x7FF;
              }
            }
            else
            {
              var_v0_3 = 0;
            }
            new_var2 = (u8 *) temp_s2;
            *((s16 *) (new_var2 + 0x46)) = var_v0_3;
            temp_v0_3 = (var_t2_3->unk1A + (*((s16 *) (((u8 *) var_s1_5) + 0x10)))) + (*((s16 *) (((u8 *) var_s1_5) + 0x1C)));
            if (temp_v0_3 > 0)
            {
              var_v0_4 = temp_v0_3;
              if (temp_v0_3 >= 0x800)
              {
                var_v0_4 = 0x7FF;
              }
            }
            else
            {
              var_v0_4 = 0;
            }
            *((s16 *) (new_var2 + 0x48)) = var_v0_4;
            temp_v0_4 = (new_var8 = (var_t2_3->unk1A + (*((s16 *) (((u8 *) var_s1_5) + 0x10)))) + (*((s16 *) (((u8 *) var_s1_5) + 0x1E))));
            if (temp_v0_4 > 0)
            {
              var_v0_5 = temp_v0_4;
              if (temp_v0_4 >= 0x800)
              {
                var_v0_5 = 0x7FF;
                if (1)
                {
                }
                if (1)
                {
                }
                if (1)
                {
                }
              }
            }
            else
            {
              var_v0_5 = 0;
            }
          }
          else
          {
            *((s16 *) (((u8 *) temp_s2) + 0x44)) = (s16) (*((u16 *) (((u8 *) var_s1_5) + 0x18)));
            *((s16 *) (((u8 *) temp_s2) + 0x46)) = (s16) (*((u16 *) (((u8 *) var_s1_5) + 0x1A)));
            *((s16 *) (new_var2 + 0x48)) = (s16) (*((u16 *) (((u8 *) var_s1_5) + 0x1C)));
            var_v0_5 = (s16) (*((u16 *) (((u8 *) var_s1_5) + 0x1E)));
          }
          *((s16 *) (new_var2 + 0x4A)) = var_v0_5;
          var_s6 = 0;
          var_fp = var_s1_5->unk0;
          var_t1_5 = temp_s1_2;
          var_s5 = ((*(((u8 *) temp_s2) + 0x21)) > 0U) * 2;
          if (var_s1_5->unk0 != 0)
          {
            sp54 = var_t0_2;
            sp58 = (s32) var_t1_3;
            sp5C = var_t2_3;
            sp60 = var_t3;
            sp64 = var_t4;
            sp68 = var_t5;
            sp6C = new_var4;
            sp74 = var_t8;
            temp_v0_5 = func_80056824(sp34, var_t0_2, temp_s2, var_fp);
            *((Unk **) (((u8 *) temp_s2) + 8)) = temp_v0_5;
            if (temp_v0_5 == 0)
            {
              var_s0_3 = 1;
              if (((*((s32 *) (((u8 *) var_s1_5) + 8))) & 0xF00) != 0x100)
              {
                var_s0_3 = *(((u8 *) var_s1_5) + 0xA);
                var_s0_3 = var_s0_3 * (*((new_var15 = (u8 *) var_s1_5) + 0xB));
              }
              var_v1_3 = var_s0_3 + 0x1F;
              var_s3 = (*((Unk **) (new_var2 + 0xC)) = sp24);
              if (var_v1_3 < 0)
              {
                var_v1_3 = var_s0_3 + 0x3E;
              }
              var_s0_4 = var_s0_3 - 1;
              temp_v1_14 = 0;
              temp_v0_6 = var_v1_3 >> 5;
              sp3C = temp_v1_14;
              *((s32 *) (((u8 *) temp_s2) + 0x14)) = (s32) (temp_v0_6 * 4);
              sp24 = (Unk *) (((u8 *) var_s3) + (temp_v0_6 * 4));
              do
              {
              }
              while (0);
              var_s1 = 1;
              if (var_s0_4 != new_var)
              {
                var_a0_7 = (Unk *) (var_fp + 1);
                do
                {
                  if ((*var_fp) & 0x80)
                  {
                    sp3C |= var_s1;
                    if (var_s5 == 0)
                    {
                      temp_v0_7 = *((u8 *) var_a0_7);
                      var_s5 = 1;
                      var_t3 = temp_v0_7 & 0xF;
                      sp80 = (temp_v0_7 >> 4) & 3;
                    }
                    else
                      if ((var_s5 == 1) && (((temp_v1_9 = *((u8 *) var_a0_7), var_t3 != (temp_v1_9 & 0xF))) || (sp80 != ((temp_v1_9 >> 4) & 3))))
                    {
                      var_s5 = 2;
                    }
                    if (var_s6 == 0)
                    {
                      do
                      {
                        var_s6 = 1;
                        var_t4 = *((u8 *) var_a0_7 + 2);
                        var_t8 = ((*((u8 *) var_a0_7)) >> 6) & 1;
                      }
                      while (0);
                    }
                    else
                      if ((var_s6 == 1) && ((var_t4 != (*((u8 *) var_a0_7 + 2))) || (var_t8 != (((*((u8 *) var_a0_7)) >> 6) & 1))))
                    {
                      var_s6 = 2;
                    }
                  }
                  var_s1 *= 2;
                  if (var_s1 == 0)
                  {
                    var_s3->unk0 = (Unk *) sp3C;
                    var_s3 = (Unk *) (((u8 *) var_s3) + 4);
                    var_s1 = 1;
                    sp3C = 0;
                  }
                  var_a0_7 = (Unk *) (((u8 *) var_a0_7) + 4);
                  var_s0_4 -= 1;
                  var_fp += 4;
                }
                while (var_s0_4 != new_var);
              }
              if (var_s1 != 1)
              {
                if (1)
                {
                }
                var_s3->unk0 = (Unk *) sp3C;
              }
              if (var_s5 == 1)
              {
                *((s32 *) (new_var2 + 0x18)) = (s32) ((var_t3 + (sp80 * 0x10)) + 1);
              }
              else
              {
                *((s32 *) (((u8 *) temp_s2) + 0x18)) = 0;
              }
              if (var_s6 == 1)
              {
                do
                {
                }
                while (0);
                *((s32 *) (new_var2 + 0x1C)) = (s32) ((var_t4 + (var_t8 << 9)) + 1);
              }
              else
              {
                *((s32 *) (new_var2 + 0x1C)) = 0;
              }
            }
          }
          var_s7++;
        }
        while ((*var_s7) != 0);
      }
      var_t6 += 1;
      var_t5->unk0 = 0;
    }
    while ((*var_t6) != 0);
  }
  sp38->unk0 = 0;
  var_s1_2 = *((Unk **) (((u8 *) sp34) + 8));
  if (var_s1_2 != 0)
  {
    do
    {
      temp_a3 = *((Unk **) (((u8 *) var_s1_2) + 4));
      if ((((u16) (*((u16 *) 0x80180008))) >= 0x12U) && ((*(((u8 *) temp_a3) + 8)) != 0xFF))
      {
        if ((*(((u8 *) temp_a3) + 9)) != 0xFF)
        {
          *((Unk **) (((u8 *) var_s1_2) + 8)) = 0;
          temp_v0_8 = func_8005AB80(*(((u8 *) temp_a3) + 8), *(((u8 *) temp_a3) + 9));
          *((Unk **) (((u8 *) var_s1_2) + 0xC)) = temp_v0_8;
          if ((*((s32 *) (((u8 *) (*((Unk **) (((u8 *) temp_v0_8) + 4)))) + 8))) & 0xF000)
          {
            *((Unk **) (((u8 *) sp34) + 0xC)) = var_s1_2;
          }
          temp_v1_10 = *((Unk **) (((u8 *) var_s1_2) + 0xC));
          *(((u8 *) temp_v1_10) + 0x22) = (u8) ((*(((u8 *) temp_v1_10) + 0x22)) + 1);
        }
        else
        {
          temp_v0_9 = func_8005AB4C(*(((u8 *) temp_a3) + 8));
          *((Unk **) (((u8 *) var_s1_2) + 8)) = temp_v0_9;
          *(((u8 *) temp_v0_9) + 0xD) = (u8) ((*(((u8 *) temp_v0_9) + 0xD)) + 1);
          goto block_125;
        }
      }
      else
      {
        *((Unk **) (((u8 *) var_s1_2) + 8)) = 0;
        block_125:
        *((Unk **) (((u8 *) var_s1_2) + 0xC)) = 0;

      }
      var_s1_2 = var_s1_2->unk0;
    }
    while (var_s1_2 != 0);
  }
  field_validate_and_rasterize_quads((0, arg0->unk14), 0);
  field_validate_and_rasterize_quads(arg0->unk18, 1);
  field_validate_and_rasterize_quads(arg0->unk1C, 2);
  field_validate_and_rasterize_quads(arg0->unk20, 3);
  var_t0_3 = *((Node30 **) (((u8 *) sp34) + 4));
  if (var_t0_3 != 0)
  {
    do
    {
      temp_t2 = (Unk *) var_t0_3->unk4;
      sp10[0] = var_t0_3->unk10 << 8;
      sp10[1] = var_t0_3->unk12 << 8;
      sp10[2] = var_t0_3->unk14 << 8;
      temp_a0_2 = *((u16 **) (((u8 *) temp_t2) + 4));
      sp54 = var_t0_3;
      sp5C = temp_t2;
      func_8005AC50(temp_a0_2 + 2, *temp_a0_2, sp10);
      var_t0_4 = sp54;
      var_t2_4 = sp5C;
      var_s2 = var_t0_4->unk8;
      sp20 = 0;
      if (var_s2 != 0)
      {
        do
        {
          sp54 = var_t0_4;
          sp5C = var_t2_4;
          func_8005AD20((0, var_s2->unk21), *((u16 *) (*((void **) (((u8 *) var_t2_4) + 4)))), &sp20);
          temp_s4_2 = (Src4 *) var_s2->unk4;
          var_t0_4 = sp54;
          var_fp_2 = temp_s4_2->unk0;
          var_t2_4 = sp5C;
          if (var_fp_2 != 0)
          {
            temp_v1_11 = var_s2->unk8;
            if (temp_v1_11 != 0)
            {
              var_s2->unkC = (s32 *) temp_v1_11->unkC;
              var_s2->unk14 = (s32) temp_v1_11->unk14;
              var_s2->unk18 = (s32) temp_v1_11->unk18;
              var_s2->unk1C = (s32) temp_v1_11->unk1C;
              var_s2->unk10 = (Unk *) temp_v1_11->unk10;
              var_s2->unk26 = (u16) temp_v1_11->unk26;
            }
            else
            {
              temp_v1_12 = var_s2->unk21;
              var_t1_4 = 0;
              if (temp_v1_12 != 1)
              {
                if (((s32) temp_v1_12) < 2)
                {
                  if (!var_t0)
                  {
                  }
                  var_s5_2 = 0xC;
                  if (temp_v1_12 != 0)
                  {
                    var_s2->unk26 = 0U;
                  }
                  else
                  {
                    temp_v1_13 = var_s2->unk1C;
                    var_s7_2 = sp24;
                    var_s2->unk10 = var_s7_2;
                    if (temp_v1_13 != 0)
                    {
                      var_v0_6 = temp_v1_13 - 1;
                      do
                      {
                        temp_t4 = var_v0_6;
                        var_s2->unk1C = *((s32 *) (((temp_t4 & 0xFF) * 4) + 0x1F800000));
                        var_s6_2 = 1;
                        if (temp_t4 & 0x200)
                        {
                          *(((u8 *) var_s2) + 0x1F) = (u8) ((*(((u8 *) var_s2) + 0x1F)) | 2);
                        }
                      }
                      while (0);
                      var_s5_2 = 8;
                    }
                    else
                    {
                      var_s6_2 = 0;
                    }
                    temp_v0_10 = var_s2->unk18;
                    temp_t3 = temp_v0_10 - 1;
                    if (temp_v0_10 != 0)
                    {
                      temp_a0_3 = temp_t3 & 0xF;
                      temp_v0_11 = temp_t3 * 2;
                      new_var6 = 6;
                      var_v1_4 = ((8 * temp_s4_2->unk8) & 0x180) | (temp_v0_11 & 0x60);
                      new_var19 = temp_a0_3;
                      if (temp_a0_3 >= 0xAU)
                      {
                        var_v0_6 = ((u32) (((new_var19 << 6) - 0x80) & 0x3FF)) >> new_var6;
                      }
                      else
                      {
                        var_v0_6 = (((u32) ((new_var19 << new_var6) + 0x140)) >> 6) | 0x10;
                      }
                      var_a1_2 = var_v1_4 | var_v0_6;
                      var_s2->unk18 = (s32) ((var_a1_2 & 0x9FF) | 0xE1000400);
                      var_s6_2 |= 2;
                      var_s5_2 -= 4;
                    }
                    var_s3_2 = var_s2->unkC;
                    new_var12 = (u8 *) temp_s4_2;
                    var_s0_5 = ((*(new_var12 + 0xA)) * (*(new_var12 + 0xB))) - 1;
                    var_s1_3 = 0;
                    if (var_s0_5 != new_var)
                    {
                      var_v1_5 = new_var;
                      do
                      {
                        if (var_s1_3 == 0)
                        {
                          temp_t7 = *var_s3_2;
                          var_s3_2++;
                          var_s1_3 = 1;
                          sp3C = temp_t7;
                        }
                        if (sp3C & var_s1_3)
                        {
                          temp_a1_5 = var_s7_2;
                          var_s7_2 = (Unk *) (((u8 *) var_s7_2) + var_s5_2);
                          temp_t1 = var_t1_4 + 1;
                          sp50 = var_v1_5;
                          sp54 = var_t0_4;
                          new_var3 = new_var3;
                          sp58 = temp_t1;
                          sp5C = var_t2_4;
                          func_8005477C(var_fp_2, temp_a1_5, (((u32) temp_s4_2->unk8) >> 4) & 3, var_s6_2);
                          var_t1_4 = temp_t1;
                        }
                        var_s1_3 *= 2;
                        var_s0_5 -= 1;
                        var_fp_2 += 4;
                      }
                      while (var_s0_5 != var_v1_5);
                      var_v0_7 = var_t1_4 & 0xFFFF;
                    }
                    else
                    {
                      goto block_173;
                    }
                    goto block_174;
                  }
                }
                else
                {
                  var_s5_2 = 0xC;
                  if (((s32) temp_v1_12) < new_var6)
                  {
                    temp_v1_14 = var_s2->unk1C;
                    var_s7_3 = sp24;
                    var_s2->unk10 = var_s7_3;
                    if (temp_v1_14 != 0)
                    {
                      temp_t4_2 = temp_v1_14 - 1;
                      var_s2->unk1C = *((s32 *) (((temp_t4_2 & 0xFF) * 4) + 0x1F800000));
                      var_s6_3 = 1;
                      if (temp_t4_2 & 0x200)
                      {
                        *(((u8 *) var_s2) + 0x1F) = (u8) ((*(((u8 *) var_s2) + 0x1F)) | 2);
                      }
                      var_s5_2 = 8;
                    }
                    else
                    {
                      var_s6_3 = 0;
                    }
                    temp_v0_12 = var_s2->unk18;
                    temp_t3_2 = 1;
                    temp_t3_2 = temp_v0_12 - temp_t3_2;
                    if (temp_v0_12 != 0)
                    {
                      temp_a0_4 = temp_t3_2 & 0xF;
                      temp_v0_13 = temp_t3_2 * 2;
                      new_var11 = temp_v0_13 & 0x60;
                      var_v1_6 = ((temp_s4_2->unk8 * 8) & 0x180) | new_var11;
                      if (temp_a0_4 >= 0xAU)
                      {
                        var_v0_8 = ((u32) (((temp_a0_4 << 6) - 0x80) & 0x3FF)) >> new_var6;
                      }
                      else
                      {
                        var_v0_8 = (((u32) ((temp_a0_4 << 6) + 0x140)) >> (6 ^ 0)) | 0x10;
                      }
                      var_s2->unk18 = (s32) (var_v1_6 | var_v0_8);
                      var_s6_3 |= 2;
                      var_s5_2 -= 4;
                    }
                    var_s3_3 = var_s2->unkC;
                    var_s0_6 = ((*(new_var12 + 0xB)) * (*(new_var12 + 0xA))) - 1;
                    var_s1_4 = 0;
                    if (var_s0_6 != new_var)
                    {
                      var_v1_7 = new_var;
                      do
                      {
                        if (var_s1_4 == 0)
                        {
                          temp_t7_2 = *var_s3_3;
                          var_s3_3++;
                          var_s1_4 = 1;
                          sp3C = temp_t7_2;
                        }
                        if (sp3C & var_s1_4)
                        {
                          temp_a1_6 = var_s7_3;
                          var_s7_3 = (Unk *) (((u8 *) var_s7_3) + var_s5_2);
                          temp_t1_2 = var_t1_4 + 1;
                          sp50 = var_v1_7;
                          sp54 = var_t0_4;
                          sp58 = temp_t1_2;
                          sp5C = var_t2_4;
                          new_var8 = (((u32) temp_s4_2->unk8) >> 4) & 3;
                          func_80054904(var_fp_2, temp_a1_6, new_var8, var_s6_3);
                          var_t1_4 = temp_t1_2;
                        }
                        var_s1_4 *= 2;
                        var_s0_6 -= 1;
                        var_fp_2 += 4;
                      }
                      while (var_s0_6 != var_v1_7);
                    }
                    block_173:
                    var_v0_7 = var_t1_4 & 0xFFFF;

                    block_174:
                    sp24 = (Unk *) (((u8 *) sp24) + (var_v0_7 * var_s5_2));

                  }
                  goto block_175;
                }
              }
              else
              {
                block_175:
                var_s2->unk26 = (u16) var_t1_4;

              }
            }
          }
          else
          {
            var_s2->unk26 = 0U;
            var_s2->unk10 = 0;
          }
          var_s2 = var_s2->unk0;
        }
        while (var_s2 != 0);
      }
      var_t0_3 = var_t0_4->unk0;
    }
    while (var_t0_3 != 0);
  }
  *((void **) (((u8 *) sp34) + 0x38)) = 0;
  *((s32 *) (((u8 *) sp34) + 0x3C)) = 0;
  func_80053C7C(arg0->unk14, &sp24, ((u8 *) sp34) + 0x18);
  func_80053C7C(arg0->unk18, &sp24, ((u8 *) sp34) + 0x1C);
  func_80053C7C(arg0->unk1C, &sp24, ((u8 *) sp34) + 0x20);
  func_80053C7C(arg0->unk20, &sp24, ((u8 *) sp34) + 0x24);
  var_v1_8 = *((s32 *) (((u8 *) (&g_field_dyn_count)) + 8));
  var_s0_7 = g_field_dyn_count - 1;
  new_var3 = new_var;
  var_t1_5 = (Unk *) (((u8 *) sp34) + 0x14);
  if (var_s0_7 != new_var3)
  {
    do
    {
      temp_s1_2 = sp24;
      sp24 = (Unk *) (((u8 *) temp_s1_2) + 0x10);
      do
      {
        var_t1_5->unk0 = temp_s1_2;
        var_t1_5 = temp_s1_2;
        var_s0_7 = var_s0_7 - 1;
        *((s32 *) (((u8 *) var_t1_5) + 4)) = var_v1_8;
        var_v1_8 += 0xC;
        if (1)
        {
        }
        *((s32 *) (((u8 *) var_t1_5) + 8)) = (s32) ((*((s32 *) (((u8 *) var_t1_5) + 8))) & (~3));
      }
      while (0);
    }
    while (var_s0_7 != new_var3);
  }
  var_t1_5->unk0 = 0;
  var_s1_5 = *((Unk **) (((u8 *) sp34) + 0x14));
  var_s0_8 = 0;
  var_s2_2 = (1 << sp28);
  if (var_s1_5 != 0)
  {
    do
    {
      if ((*((u8 *) (((u8 *) (*((Unk **) (((u8 *) var_s1_5) + 4)))) + 1))) & var_s2_2)
      {
        func_8005A744(var_s1_5, var_s0_8 & 0xFF);
      }
      var_s1_5 = var_s1_5->unk0;
      var_s0_8 += 1;
    }
    while (var_s1_5 != 0);
  }
  if ((*((void **) (((u8 *) sp34) + 0x38))) != 0)
  {
    *((s32 *) (((u8 *) sp34) + 0x34)) = 0;
    new_var7 = 0x38;
    *((void **) (((u8 *) sp34) + new_var7)) = sp24;
    sp24 = (Unk *) (((u8 *) sp24) + 0x14C00);
    DecDCTReset(0);
    DecDCTvlcBuild(*((void **) (((u8 *) sp34) + (0x38 ^ 0))));
    *sp30 = sp24;
  }
  arg0->unk26 = 1;
}