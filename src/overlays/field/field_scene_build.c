#include "field_scene_internal.h"


struct Build_FieldAnimDefRasterView;
struct Build_FieldPartDef;
struct Build_FieldTileDesc;
struct Build_FieldPart;

/**
 * @brief Animation-definition view used while preparing tile masks.
 */
typedef struct Build_FieldAnimDefRasterView
{
    u8 pad_00[4];
    union
    {
        u32 word;
        struct
        {
            u8 pad_04[2];
            u8 frame_count;
            u8 handler_group;
        } bytes;
    } flags;
    struct Build_FieldAnimDefRasterView* next;
    u8 rect_x;
    u8 rect_y;
    u8 rect_width;
    u8 rect_height;
    struct Build_FieldPartDef* part_def;
    struct Build_FieldTileDesc* frame_tiles;
} Build_FieldAnimDefRasterView;

/**
 * @brief Tile-grid definition referenced by a runtime field part.
 */
typedef struct Build_FieldPartDef
{
    u8 pad_00[8];
    union
    {
        u32 word;
        struct
        {
            u8 pad_08[2];
            u8 cols;
            u8 rows;
        } bytes;
    } flags;
} Build_FieldPartDef;

/**
 * @brief Packed four-byte source descriptor for one field tile.
 */
typedef struct Build_FieldTileDesc
{
    u8 clut_slot;
    u8 texture_attrs;
    u8 packed_uv;
    u8 color_index;
} Build_FieldTileDesc;

/**
 * @brief Runtime field-part view used while validating shared tile attributes.
 */
typedef struct Build_FieldPart
{
    u8 pad_00[8];
    struct Build_FieldPart* linked_part;
    s32* bits;
    u8 pad_10[8];
    u32 tpage_word;
    u32 code_word;
} Build_FieldPart;
typedef struct Build_FieldAnimDef Build_FieldAnimDef;
typedef struct Build_FieldAnim Build_FieldAnim;
typedef struct Build_FieldAnimCel Build_FieldAnimCel;

typedef struct
{
    void *tiles; /* 0x00 */
    u8 _pad0[8 - 4];
    union
    {
        u32 word;
        struct
        {
            u8 _pad1[2];
            u8 cols; /* 0x0A */
            u8 rows; /* 0x0B */
        } b;
    } u;
} Build_FieldTileGrid;

struct Build_FieldAnimDef
{
    u8 unk0;  /* 0x00 */
    u8 unk1;  /* 0x01 */
    u8 unk2;  /* 0x02 */
    u8 _pad0;
    u8 flags; /* 0x04 */
    u8 unk5;  /* 0x05 */
    u8 unk6;  /* 0x06 */
    u8 handler_group; /* 0x07 */
    Build_FieldAnimDef *next; /* 0x08 */
    u8 unkC;  /* 0x0C */
    u8 unkD;  /* 0x0D */
    u8 unkE;  /* 0x0E */
    u8 unkF;  /* 0x0F */
    void *unk10; /* 0x10 */
    s32 *data;  /* 0x14 */
};

typedef union
{
    s32 word;
    struct
    {
        u8 unk0;
        u8 state; /* 0x25 */
        u8 keyframe;      /* 0x26 */
        u8 stop_keyframe; /* 0x27 */
    } b;
} Build_FieldAnimFlags;

struct Build_FieldAnimCel
{
    Build_FieldAnimCel *next;  /* 0x00 */
    Build_FieldTileGrid *grid; /* 0x04 */
    u8 _pad0[0xC - 8];
    u32 *mask;  /* 0x0C */
    u8 *tiles;  /* 0x10 */
    u8 _pad1[0x18 - 0x14];
    s32 tpage_word; /* 0x18 */
    s32 code_word;  /* 0x1C */
    s8 active;      /* 0x20 */
    u8 format;  /* 0x21 */
};

struct Build_FieldAnim
{
    Build_FieldAnim *next;      /* 0x00 */
    Build_FieldAnimDef *def;    /* 0x04 */
    u8 _pad0[0xC - 8];
    Build_FieldAnimCel *cels;   /* 0x0C */
    s32 unk10;            /* 0x10 */
    u8 _pad1[0x20 - 0x14];
    u8 *frame_data;       /* 0x20 */
    Build_FieldAnimFlags flags; /* 0x24 */
    u8 repeat_count;      /* 0x28 */
    u8 _pad2;
    u16 timer;            /* 0x2A */
    u16 frame_tile_count; /* 0x2C */
    u8 _pad3[0x30 - 0x2E];
};

typedef struct
{
    u8 _pad0[4];
    u16 *data; /* 0x04 */
} Build_FieldTintPal;

typedef struct
{
    u8 _pad0[4];
    Build_FieldTintPal *palette; /* 0x04 */
    u8 _pad1[0x10 - 8];
    u16 red;   /* 0x10 */
    u16 green; /* 0x12 */
    u16 blue;  /* 0x14 */
} Build_FieldTintSrc;

typedef struct
{
    u8 _pad0;
    u8 range_start; /* 0x01 */
    u16 duration;   /* 0x02 */
} Build_FieldTweenSpan;

/** @brief Minimal view of one sound keyframe entry. */
typedef struct
{
    u8 kind;  /* 0x00 */
    u8 _pad0;
    u16 sound_flags; /* 0x02 */
} Build_FieldSfxKey;

typedef struct
{
    u8 _pad0[0x38];
    s32 unk38; /* 0x38 */
} Build_FieldScene;

extern Build_FieldScene *g_field_scene_build __asm__("g_field_scene");

/**
 * @brief Find the runtime part whose definition pointer matches @p part_def.
 *
 * @param part_def Part definition to find in the scene object lists.
 * @param owner_out Optional output for the part's owning object/tint-source view.
 * @return Matching runtime part, or NULL when the definition is not in use.
 */
Build_FieldPart *func_8005ABD8(void *part_def, Build_FieldTintSrc **owner_out);

/**
 * @brief Prepare tile-animation definitions and their runtime presence masks.
 *
 * Assigns @p handler_group to every definition. For tile handlers in groups
 * zero and three, it verifies that the runtime part's shared TPage and
 * RGB/code words agree with every present source tile, clearing either shared
 * word when the descriptors disagree. It also rasterizes the definition's
 * source rectangle into the runtime part's row-major presence bitmap.
 *
 * @param head Head of the linked animation-definition list.
 * @param handler_group Scene animation-list group, in the range 0 through 3.
 *
 * @see decomp.me (95.80%) https://decomp.me/scratch/Kkiiv
 */


typedef struct Records_Unk Records_Unk;
struct Records_Unk
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
typedef struct Records_Node44 Records_Node44;
struct Records_Node44
{
  Records_Node44 *unk0;
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
typedef struct Records_SrcObj Records_SrcObj;
struct Records_SrcObj
{
  Records_SrcObj *unk0;
  u8 unk4;
  u8 pad05[0x13];
  u16 unk18;
};
typedef struct Records_ObjArg Records_ObjArg;
struct Records_ObjArg
{
  void *unk0;
  u8 pad04[0x04];
  Records_SrcObj *unk8;
  void *unkC;
  u8 pad10[0x04];
  s32 unk14;
  s32 unk18;
  s32 unk1C;
  s32 unk20;
  u8 pad24[0x02];
  s16 unk26;
};
typedef struct Records_SrcObj2 Records_SrcObj2;
struct Records_SrcObj2
{
  Records_SrcObj2 *unk0;
  u16 unk4;
  u16 unk6;
  s16 unk8;
  u16 unkA;
  u16 unkC;
  u16 unkE;
};
typedef struct Records_Node38 Records_Node38;
struct Records_Node38
{
  Records_Node38 *unk0;
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
typedef struct Records_SrcObj3 Records_SrcObj3;
struct Records_SrcObj3
{
  Records_SrcObj3 *unk0;
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
typedef struct Records_InnerNode Records_InnerNode;
struct Records_InnerNode
{
  Records_InnerNode *unk0;
  void *unk4;
  Records_InnerNode *unk8;
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
typedef struct Records_Src4 Records_Src4;
struct Records_Src4
{
  s32 unk0;
  u8 pad04[4];
  s32 unk8;
};
typedef struct Records_Node30 Records_Node30;
struct Records_Node30
{
  Records_Node30 *unk0;
  void *unk4;
  Records_InnerNode *unk8;
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
typedef struct Records_Node
{
  struct Records_Node *unk0;
  u8 pad[32];
  u32 unk24;
  u32 unk28;
  u32 unk2C;
  u32 unk30;
} Records_Node;
typedef struct
{
  u8 padding[8];
  Records_Node *unk8;
} Records_FieldScene;
extern Records_Unk *g_field_scene_records __asm__("g_field_scene");
extern void DecDCTReset(int mode);
extern void DecDCTvlcBuild(u_short *table);
void field_prepare_animation_definitions(void *, s32);
void field_build_animation_list(Build_FieldAnimDef *, u8 **, Build_FieldAnim **);
void field_build_sprite_tile_record(FieldTileDesc *, FieldTileRec *, s32, s32);
void field_build_quad_tile_record(FieldTileDesc *, FieldTileRec *, s32, s32);
s32 field_find_shareable_part(void *, Records_Unk *, Records_Unk *, u8 *);
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
void field_build_render_records(Records_ObjArg *arg0, u16 arg1)
{
  s8 sp20;
  Records_Unk *sp24;
  s32 sp10[3];
  u16 sp28;
  Records_Unk **sp30;
  Records_Unk *sp34;
  Records_Node30 *sp38;
  s32 sp3C;
  s32 sp50;
  Records_Node30 * volatile sp54;
  s32 sp58;
  void * volatile sp5C;
  s32 sp60;
  s32 sp64;
  Records_Unk *sp68;
  Records_Unk **sp6C;
  s32 sp74;
  s32 sp80;
  Records_Unk *temp_a1_5;
  Records_Unk *temp_a1_6;
  Records_Node44 *temp_s1;
  Records_Unk *temp_s1_2;
  Records_Unk *temp_s2;
  Records_Node38 *temp_t0;
  Records_Unk *var_s3;
  int new_var7;
  Records_Unk *var_s7_2;
  unsigned int new_var14;
  Records_Unk *var_s7_3;
  char new_var3;
  Records_Node30 *var_t0_2;
  Records_Node30 *var_t0_3;
  Records_Node30 *var_t0_4;
  Records_Node44 *var_t1;
  Records_Node38 *var_t1_2;
  Records_Unk *var_t1_5;
  Records_Unk *var_t5;
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
  Records_Node38 *new_var16;
  u8 *new_var17;
  u16 var_t2_2;
  u16 *temp_a0_2;
  u16 *var_t0;
  u32 temp_a0_3;
  u32 temp_a0_4;
  u32 var_v0_6;
  u32 var_v0_8;
  Records_SrcObj3 **new_var4;
  u8 temp_v0_7;
  short temp_v1_12;
  u8 temp_v1_9;
  u16 new_var5;
  int var_v0;
  Records_Unk *var_a0_7;
  int new_var8;
  u8 *var_fp;
  Records_Unk *temp_a3;
  Records_Unk *temp_s4;
  Records_Src4 *temp_s4_2;
  Records_Unk *temp_t2;
  Records_Unk *temp_v0_8;
  Records_Unk *temp_v0_9;
  Records_Unk *temp_v1_10;
  Records_InnerNode *temp_v1_11;
  Records_SrcObj *var_a3;
  Records_SrcObj2 *var_a3_2;
  Records_Unk *var_s1_2;
  Records_Unk *var_s1_5;
  unsigned char new_var20;
  Records_InnerNode *var_s2;
  s32 var_s2_2;
  Records_SrcObj3 *var_t2_3;
  u8 *new_var15;
  Records_Unk *var_t2_4;
  Records_Unk **var_s7;
  Records_SrcObj3 **var_t6;
  sp30 = (Records_Unk **) 0x801ED000;
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
  sp34 = g_field_scene_records;
  g_field_scene_records->unk0 = arg0;
  *((s32 *) (((u8 *) g_field_scene_records) + 0xC)) = 0;
  var_a3 = arg0->unk8;
  sp24 = (Records_Unk *) (((u8 *) g_field_scene_records) + 0x74);
  sp28 = arg1;
  var_t1 = (Records_Node44 *) (((u8 *) g_field_scene_records) + 8);
  var_t5_2 = *((s16 **) (((u8 *) (&g_field_scene_records)) + 8));
  if (arg0->unk8 != (0 * 0))
  {
    do
    {
      temp_s1 = sp24;
      sp24 = (Records_Unk *) (((u8 *) temp_s1) + 0x44);
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
  var_t1_2 = (Records_Node38 *) (((u8 *) sp34) + 0x10);
  if (var_a3_2 != 0)
  {
    do
    {
      temp_t0 = sp24;
      sp24 = (Records_Unk *) (((u8 *) temp_t0) + 0x38);
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
  sp38 = (Records_Node30 *) (((u8 *) sp34) + 4);
  if ((*(new_var4 = var_t6)) != 0)
  {
    
    do
    {
      var_t0_2 = sp24;
      var_t2_3 = *new_var4;
      sp24 = (Records_Unk *) (((u8 *) var_t0_2) + 0x30);
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
      var_t5 = (Records_Unk *) (((u8 *) var_t0_2) + 8);
      if ((*var_s7) != 0)
      {
        do
        {
          temp_s2 = sp24;
          temp_s4 = *var_s7;
          sp24 = (Records_Unk *) (((u8 *) temp_s2) + 0x4C);
          var_t5->unk0 = temp_s2;
          *((Records_Unk **) (((u8 *) temp_s2) + 4)) = temp_s4;
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
            temp_v0_5 = field_find_shareable_part(sp34, var_t0_2, temp_s2, var_fp);
            *((Records_Unk **) (((u8 *) temp_s2) + 8)) = temp_v0_5;
            if (temp_v0_5 == 0)
            {
              var_s0_3 = 1;
              if (((*((s32 *) (((u8 *) var_s1_5) + 8))) & 0xF00) != 0x100)
              {
                var_s0_3 = *(((u8 *) var_s1_5) + 0xA);
                var_s0_3 = var_s0_3 * (*((new_var15 = (u8 *) var_s1_5) + 0xB));
              }
              var_v1_3 = var_s0_3 + 0x1F;
              var_s3 = (*((Records_Unk **) (new_var2 + 0xC)) = sp24);
              if (var_v1_3 < 0)
              {
                var_v1_3 = var_s0_3 + 0x3E;
              }
              var_s0_4 = var_s0_3 - 1;
              temp_v1_14 = 0;
              temp_v0_6 = var_v1_3 >> 5;
              sp3C = temp_v1_14;
              *((s32 *) (((u8 *) temp_s2) + 0x14)) = (s32) (temp_v0_6 * 4);
              sp24 = (Records_Unk *) (((u8 *) var_s3) + (temp_v0_6 * 4));
              do
              {
              }
              while (0);
              var_s1 = 1;
              if (var_s0_4 != new_var)
              {
                var_a0_7 = (Records_Unk *) (var_fp + 1);
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
                    var_s3->unk0 = (Records_Unk *) sp3C;
                    var_s3 = (Records_Unk *) (((u8 *) var_s3) + 4);
                    var_s1 = 1;
                    sp3C = 0;
                  }
                  var_a0_7 = (Records_Unk *) (((u8 *) var_a0_7) + 4);
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
                var_s3->unk0 = (Records_Unk *) sp3C;
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
  var_s1_2 = *((Records_Unk **) (((u8 *) sp34) + 8));
  if (var_s1_2 != 0)
  {
    do
    {
      temp_a3 = *((Records_Unk **) (((u8 *) var_s1_2) + 4));
      if ((((u16) (*((u16 *) 0x80180008))) >= 0x12U) && ((*(((u8 *) temp_a3) + 8)) != 0xFF))
      {
        if ((*(((u8 *) temp_a3) + 9)) != 0xFF)
        {
          *((Records_Unk **) (((u8 *) var_s1_2) + 8)) = 0;
          temp_v0_8 = func_8005AB80(*(((u8 *) temp_a3) + 8), *(((u8 *) temp_a3) + 9));
          *((Records_Unk **) (((u8 *) var_s1_2) + 0xC)) = temp_v0_8;
          if ((*((s32 *) (((u8 *) (*((Records_Unk **) (((u8 *) temp_v0_8) + 4)))) + 8))) & 0xF000)
          {
            *((Records_Unk **) (((u8 *) sp34) + 0xC)) = var_s1_2;
          }
          temp_v1_10 = *((Records_Unk **) (((u8 *) var_s1_2) + 0xC));
          *(((u8 *) temp_v1_10) + 0x22) = (u8) ((*(((u8 *) temp_v1_10) + 0x22)) + 1);
        }
        else
        {
          temp_v0_9 = func_8005AB4C(*(((u8 *) temp_a3) + 8));
          *((Records_Unk **) (((u8 *) var_s1_2) + 8)) = temp_v0_9;
          *(((u8 *) temp_v0_9) + 0xD) = (u8) ((*(((u8 *) temp_v0_9) + 0xD)) + 1);
          goto block_125;
        }
      }
      else
      {
        *((Records_Unk **) (((u8 *) var_s1_2) + 8)) = 0;
        block_125:
        *((Records_Unk **) (((u8 *) var_s1_2) + 0xC)) = 0;

      }
      var_s1_2 = var_s1_2->unk0;
    }
    while (var_s1_2 != 0);
  }
  field_prepare_animation_definitions((0, arg0->unk14), 0);
  field_prepare_animation_definitions(arg0->unk18, 1);
  field_prepare_animation_definitions(arg0->unk1C, 2);
  field_prepare_animation_definitions(arg0->unk20, 3);
  var_t0_3 = *((Records_Node30 **) (((u8 *) sp34) + 4));
  if (var_t0_3 != 0)
  {
    do
    {
      temp_t2 = (Records_Unk *) var_t0_3->unk4;
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
          temp_s4_2 = (Records_Src4 *) var_s2->unk4;
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
              var_s2->unk10 = (Records_Unk *) temp_v1_11->unk10;
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
                          var_s7_2 = (Records_Unk *) (((u8 *) var_s7_2) + var_s5_2);
                          temp_t1 = var_t1_4 + 1;
                          sp50 = var_v1_5;
                          sp54 = var_t0_4;
                          new_var3 = new_var3;
                          sp58 = temp_t1;
                          sp5C = var_t2_4;
                          field_build_sprite_tile_record(var_fp_2, temp_a1_5, (((u32) temp_s4_2->unk8) >> 4) & 3, var_s6_2);
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
                          var_s7_3 = (Records_Unk *) (((u8 *) var_s7_3) + var_s5_2);
                          temp_t1_2 = var_t1_4 + 1;
                          sp50 = var_v1_7;
                          sp54 = var_t0_4;
                          sp58 = temp_t1_2;
                          sp5C = var_t2_4;
                          new_var8 = (((u32) temp_s4_2->unk8) >> 4) & 3;
                          field_build_quad_tile_record(var_fp_2, temp_a1_6, new_var8, var_s6_3);
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
                    sp24 = (Records_Unk *) (((u8 *) sp24) + (var_v0_7 * var_s5_2));

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
  field_build_animation_list(arg0->unk14, &sp24, ((u8 *) sp34) + 0x18);
  field_build_animation_list(arg0->unk18, &sp24, ((u8 *) sp34) + 0x1C);
  field_build_animation_list(arg0->unk1C, &sp24, ((u8 *) sp34) + 0x20);
  field_build_animation_list(arg0->unk20, &sp24, ((u8 *) sp34) + 0x24);
  var_v1_8 = *((s32 *) (((u8 *) (&g_field_dyn_count)) + 8));
  var_s0_7 = g_field_dyn_count - 1;
  new_var3 = new_var;
  var_t1_5 = (Records_Unk *) (((u8 *) sp34) + 0x14);
  if (var_s0_7 != new_var3)
  {
    do
    {
      temp_s1_2 = sp24;
      sp24 = (Records_Unk *) (((u8 *) temp_s1_2) + 0x10);
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
  var_s1_5 = *((Records_Unk **) (((u8 *) sp34) + 0x14));
  var_s0_8 = 0;
  var_s2_2 = (1 << sp28);
  if (var_s1_5 != 0)
  {
    do
    {
      if ((*((u8 *) (((u8 *) (*((Records_Unk **) (((u8 *) var_s1_5) + 4)))) + 1))) & var_s2_2)
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
    sp24 = (Records_Unk *) (((u8 *) sp24) + 0x14C00);
    DecDCTReset(0);
    DecDCTvlcBuild(*((void **) (((u8 *) sp34) + (0x38 ^ 0))));
    *sp30 = sp24;
  }
  arg0->unk26 = 1;
}

void field_prepare_animation_definitions(void* head, s32 handler_group)
{
    Build_FieldAnimDefRasterView* def = (Build_FieldAnimDefRasterView*)head;
    u32 shared_page_slot = 0;
    u32 shared_color_index = 0;
    u32 shared_semitrans = 0;
    u32 shared_blend_mode = 0;
    s32 tpage_status;
    s32 code_status;
    int minus_one;
    Build_FieldAnimDefRasterView* rec;
    Build_FieldPartDef* part_def;
    Build_FieldPart* part;
    Build_FieldTileDesc* tile;
    Build_FieldTileDesc* mask_tile;
    s32 frame;
    s32 tile_index;
    s32 mask_bit;
    s32* mask;
    s32 row;
    s32 col;
    s32 mask_word;
    if (def != 0)
    {
        do
        {
            def->flags.bytes.handler_group = handler_group;
            if (((handler_group == 0) && ((def->flags.word & 7U) < 2U)) || (handler_group == 3))
            {
                rec = def;

                part_def = def->part_def;
                part = func_8005ABD8(part_def, 0);
                if (part->linked_part != 0)
                {
                    part = part->linked_part;
                }
                if ((def->flags.word & 7U) == 1)
                {
                    if ((part_def->flags.word & 0xF00U) == 0x100U)
                    {
                        def->rect_width = 1;
                        def->rect_height = 1;
                    }
                    else
                    {
                        def->rect_width = part_def->flags.bytes.cols;
                        def->rect_height = part_def->flags.bytes.rows;
                    }
                }
                if (part->tpage_word != 0)
                {
                    u32 temp = part->tpage_word - 1;
                    tpage_status = 1;
                    shared_blend_mode = temp >> 4;
                    shared_page_slot = temp & 0xF;
                }
                else
                {
                    tpage_status = 0;
                }
                if (part->code_word != 0)
                {
                    u32 temp = part->code_word - 1;
                    code_status = 1;
                    shared_semitrans = temp >> 9;
                    shared_color_index = temp & 0xFF;
                }
                else
                {
                    code_status = 0;
                }
                if ((tpage_status != 0) || (code_status != 0))
                {
                    frame = def->flags.bytes.frame_count - 1;
                    tile = rec->frame_tiles;
                    if (frame != (-1))
                    {
                        do
                        {
                            tile_index = ((frame = rec->rect_width) * rec->rect_height) - 1;
                            if (tile_index != (-1))
                            {
                                do
                                {
                                    if (tile->clut_slot & 0x80)
                                    {
                                        if (tpage_status == 1)
                                        {
                                            u8 texture_attrs = tile->texture_attrs;
                                            if ((shared_page_slot != (texture_attrs & 0xF)) ||
                                                (shared_blend_mode != ((texture_attrs >> 4) & 3)))
                                            {
                                                tpage_status = 2;
                                            }
                                        }
                                        if (code_status == 1)
                                        {
                                            u8 color_index = tile->color_index;
                                            u8 texture_attrs = tile->texture_attrs;
                                            if ((shared_color_index != color_index) ||
                                                (shared_semitrans != ((texture_attrs >> 6) & 1)))
                                            {
                                                code_status = 2;
                                            }
                                        }
                                    }
                                    tile++;
                                    tile_index--;
                                } while (tile_index != (-1));
                            }
                            frame--;
                        } while (frame != (-1));
                    }
                    if (tpage_status != 1)
                    {
                        part->tpage_word = 0;
                    }
                    if (code_status != 1)
                    {
                        part->code_word = 0;
                    }
                }

                if (((handler_group == 0) && ((def->flags.word & 7U) == 0)) || (handler_group == 3))
                {
                    frame = def->flags.bytes.frame_count - 1;
                    tile = rec->frame_tiles;
                    if (frame != (-1))
                    {
                        do
                        {
                            mask_tile = tile;
                            mask_bit = 1;
                            mask = part->bits;
                            mask_word = *mask;
                            if (part_def->flags.bytes.rows != 0)
                            {
                                row = 0;
                                do
                                {
                                    if (row < rec->rect_y)
                                    {
                                        minus_one = -1;
                                        col = part_def->flags.bytes.cols - 1;
                                        if (col != minus_one)
                                        {
                                            do
                                            {
                                                mask_bit <<= 1;
                                                if (mask_bit == 0)
                                                {
                                                    *mask = mask_word;
                                                    mask++;
                                                    mask_bit = 1;
                                                    mask_word = *mask;
                                                }
                                                col--;
                                            } while (col != (-1));
                                        }
                                    }
                                    else if (row < (rec->rect_y + rec->rect_height))
                                    {
                                        if (part_def->flags.bytes.cols != 0)
                                        {
                                            col = 0;
                                            do
                                            {
                                                if ((col >= rec->rect_x) &&
                                                    (col < (rec->rect_x + rec->rect_width)))
                                                {
                                                    if (mask_tile->clut_slot & 0x80)
                                                    {
                                                        mask_word |= mask_bit;
                                                    }
                                                    mask_tile++;
                                                }
                                                mask_bit <<= 1;
                                                if (mask_bit == 0)
                                                {
                                                    *mask = mask_word;
                                                    mask++;
                                                    mask_bit = 1;
                                                    mask_word = *mask;
                                                }
                                                col++;
                                            } while (col != part_def->flags.bytes.cols);
                                        }
                                    }
                                    else
                                    {
                                        break;
                                    }
                                    row++;
                                } while (row != part_def->flags.bytes.rows);
                            }
                            if (mask_bit != 1)
                            {
                                *mask = mask_word;
                            }
                            frame--;
                            tile += rec->rect_width * rec->rect_height;
                        } while (frame != (-1));
                    }
                }
            }
            def = def->next;
        } while (def != 0);
    }
}

Build_FieldAnimCel *field_find_object_by_definition(void *definition);
u8 *field_find_count_table_span(Build_FieldAnimDef *, s32, u8 *);
void field_apply_animation_tween(Build_FieldAnimDef *, Build_FieldAnim *, s32);
void field_blit_animation_frame(Build_FieldAnimDef *, Build_FieldAnim *, s32);
void func_8005AC50(void *colors, u16 color_count, s32 *rgb_scale);
void func_8005AD20(u8 format, u16 color_count, s8 *primitive_code);
void field_build_sprite_tile_record(FieldTileDesc *, FieldTileRec *, s32, s32);
void field_build_quad_tile_record(FieldTileDesc *, FieldTileRec *, s32, s32);

/**
 * @brief Build the scene's animation node list from a definition chain.
 *
 * Walks @p def 's chain and, for each definition, bump-allocates a 0x30-byte
 * Build_FieldAnim out of the arena at @p arena and tail-appends it to the list at
 * @p tail. Each node is seeded from its definition: the play-mode flags at
 * Build_FieldAnim::flags, the starting keyframe cursor, the loop counter, and the
 * keyframe length from field_find_count_table_span. The handler kind - the low three bits of
 * the word at Build_FieldAnimDef::flags, qualified by
 * Build_FieldAnimDef::handler_group - then selects how the node's cel list is
 * resolved (func_8005ABD8 or field_find_object_by_definition) and what
 * extra setup runs.
 *
 * For the tinted kinds the definition's colour is expanded into the scratchpad
 * table (func_8005AC50 / func_8005AD20) and the per-frame GPU primitives are
 * built into the arena: every frame walks the cel's bit plane row-major, and
 * each set bit inside the definition's sub-rectangle emits one primitive through
 * field_build_sprite_tile_record or field_build_quad_tile_record depending on the cel's record format. The arena
 * cursor is advanced past whatever each kind consumed before moving to the next
 * definition, and the list is null-terminated on the way out.
 *
 * @param def   Head of the animation definition chain; @c next links it.
 * @param arena Bump-allocation cursor; advanced past every node and primitive.
 * @param tail  Where to store the next node pointer; walked along the list and
 *              finally cleared.
 *
 * @note NOT MATCHED - 89.12% (399/704 exact rows, 19 insns short, frame 0x88 vs
 *       0x90). This replaces an earlier 89.01% version that was raw m2c output
 *       and semantically broken (locals read before assignment, a switch with
 *       statements before its first case, a fall-through case with no break).
 *       The remaining gap is a single register-allocation flip: the target keeps
 *       @p def in t0 and the cel cursor in t3 - both caller-saved - and spills
 *       and reloads them around all 17 calls, while this version wins them
 *       callee-saved registers and so emits no spill traffic. That missing
 *       traffic is the whole 19-insn shortfall, the 8-byte frame difference and
 *       every remaining structural row. @p def needs to drop from 94 to 91
 *       weighted refs to lose s7 to the arena cursor. Raising pressure
 *       artificially is worth +51 to +66 exact rows, so the natural construct
 *       that does it is the only thing left to find. See
 *       working/func_80053C7C/status.md for the full evidence and the list of
 *       probe classes already retired.
 * @note The five @c flags masks must stay SEPARATE statements; fold-const
 *       collapses them into one @c and if written as a single expression.
 * @note Both @c cel->format switches need their empty @c case @c 1: / @c case
 *       @c 6: arms to emit the 7-entry jump tables, as in field_retarget_cel_cluts.
 * @note The three @c & @c 7 handler switches read @c def->flags as a byte; the
 *       @c & @c 0xFF000007 and @c & @c 0x40 / @c & @c 0x20 tests read the whole
 *       word. Both views of the same field are required.
 * @note @c rec is a local copy of @p def, needed twice - once in the
 *       @c handler_group
 *       @c == @c 0 arm and once before the record loop. It is what puts the
 *       definition pointer in s4 and is worth 2.8%.
 *
 * @see decomp.me (89.12%) TODO
 */
void field_build_animation_list(Build_FieldAnimDef *def, u8 **arena, Build_FieldAnim **tail)
{
    s32 rgb[3];       /* sp10 */
    u8 range_start;      /* sp20 */
    Build_FieldTintSrc *tint_src;/* sp24 */
    s8 primitive_code;   /* sp28 */
    Build_FieldScene *scene;/* sp2C */
    Build_FieldTileGrid *grid; /* sp30 */
    u16 stagger_timer;   /* sp38 */
    s32 record_stride;   /* sp40 */
    u16 tile_count;      /* sp48 */
    Build_FieldAnim *anim;
    Build_FieldAnimDef *rec;
    Build_FieldAnimCel *cel;
    Build_FieldSfxKey *key;
    Build_FieldTweenSpan *span;
    u8 *arena_cursor;
    u8 *tile_record;
    u16 *palette_data;
    s32 *tile_data;
    s32 *frame_descs;
    u32 *mask;
    u32 mask_word;
    u32 mask_bit;
    s32 handler_kind;
    s32 control_flags;
    s32 record_flags;
    s32 frame;
    s32 row;
    s32 col;
    u8 initial_state;
    u16 duration;
    u16 timer;

    cel = NULL;
    grid = NULL;
    record_stride = 0;
    tile_count = 0;
    stagger_timer = 1;
    tint_src = NULL;
    scene = g_field_scene_build;
    if (def != NULL)
    {
        do
        {
            anim = (Build_FieldAnim *) *arena;
            *arena = (u8 *) anim + 0x30;
            *tail = anim;
            tail = &anim->next;
            anim->def = def;
            if (!(*(u32 *) &def->unk0 & 0x7F))
            {
                anim->flags.word &= ~0x40;
            }
            else
            {
                anim->flags.word = (anim->flags.word & ~0x40) | ((def->flags >> 7) << 6);
            }
            anim->repeat_count = 0;
            control_flags = (anim->flags.word & ~1) | ((*(u32 *) &def->flags >> 3) & 1);
            control_flags &= ~2;
            control_flags &= ~4;
            control_flags &= ~8;
            control_flags &= ~0x10;
            control_flags &= ~0x20;
            anim->flags.word = control_flags;
            anim->flags.b.stop_keyframe = 0;
            if (*(s32 *) &def->flags & 0x40)
            {
                anim->flags.b.keyframe = 0;
                anim->flags.b.state = def->unk1;
            }
            else
            {
                initial_state = def->unk1;
                anim->flags.b.state = initial_state;
                anim->flags.b.keyframe = initial_state;
            }
            if (def->handler_group == 3)
            {
                anim->timer = 1;
            }
            else
            {
                span = (Build_FieldTweenSpan *) field_find_count_table_span(def, anim->flags.b.keyframe, &range_start);
                if (*(s32 *) &def->flags & 0x20)
                {
                    anim->timer = span->duration;
                }
                else
                {
                    duration = span->duration;
                    if (duration < stagger_timer)
                    {
                        anim->timer = duration;
                        stagger_timer = 1;
                    }
                    else
                    {
                        timer = stagger_timer;
                        stagger_timer = timer + 1;
                        anim->timer = timer;
                    }
                }
            }
            switch (def->handler_group)
            {
            case 0:
                rec = def;
                switch (rec->flags & 7)
                {
                case 0:
                case 1:
                    grid = (Build_FieldTileGrid *) rec->unk10;
                    cel = (Build_FieldAnimCel *) func_8005ABD8(grid, &tint_src);
                    anim->cels = cel;
                    break;
                case 2:
                    grid = (Build_FieldTileGrid *) rec->unk10;
                    cel = (Build_FieldAnimCel *) func_8005ABD8(grid, &tint_src);
                    anim->cels = cel;
                    if ((anim->flags.word & 0x40) &&
                        (range_start = 0, frame = def->unk5, frame != -1))
                    {
                        do
                        {
                            frame -= 1;
                            cel->active = range_start == anim->flags.b.state;
                            cel = cel->next;
                            range_start += 1;
                        }
                        while (frame != -1);
                    }
                    break;
                case 3:
                    if ((anim->flags.word & 0x40) && (anim->timer != 1))
                    {
                        anim->flags.word |= 0x20;
                    }
                    break;
                case 4:
                    grid = (Build_FieldTileGrid *) rec->unk10;
                    cel = (Build_FieldAnimCel *) func_8005ABD8(grid, &tint_src);
                    anim->cels = cel;
                    scene->unk38 = 1;
                    break;
                case 5:
                    grid = (Build_FieldTileGrid *) rec->unk10;
                    cel = (Build_FieldAnimCel *) func_8005ABD8(grid, &tint_src);
                    anim->cels = cel;
                    field_apply_animation_tween(def, anim, 0);
                    break;
                case 6:
                    cel = field_find_object_by_definition(rec->unk10);
                    tint_src = (Build_FieldTintSrc *) cel;
                    anim->cels = cel;
                    field_apply_animation_tween(def, anim, 0);
                    break;
                default:
                    grid = (Build_FieldTileGrid *) rec->unk10;
                    cel = (Build_FieldAnimCel *) func_8005ABD8(grid, &tint_src);
                    anim->cels = cel;
                    anim->unk10 = (s32) tint_src;
                    key = (Build_FieldSfxKey *) rec->data;
                    if (((key->kind & 7) == 1) && (key->sound_flags & 0x8000))
                    {
                        anim->timer = 1;
                        anim->flags.word |= 8;
                    }
                    break;
                }
                break;
            case 1:
                switch (def->flags & 7)
                {
                case 0:
                    grid = (Build_FieldTileGrid *) def->data;
                    cel = (Build_FieldAnimCel *) func_8005ABD8(grid, &tint_src);
                    anim->cels = cel;
                    break;
                case 1:
                    cel = field_find_object_by_definition(def->data);
                    tint_src = (Build_FieldTintSrc *) cel;
                    anim->cels = cel;
                    break;
                }
                break;
            case 2:
                switch (def->flags & 7)
                {
                case 0:
                    grid = (Build_FieldTileGrid *) def->unk10;
                    cel = (Build_FieldAnimCel *) func_8005ABD8(grid, &tint_src);
                    anim->cels = cel;
                    anim->unk10 = (s32) tint_src;
                    break;
                case 1:
                    cel = field_find_object_by_definition(def->unk10);
                    tint_src = (Build_FieldTintSrc *) cel;
                    anim->cels = cel;
                    break;
                }
                break;
            default:
                grid = (Build_FieldTileGrid *) def->unk10;
                cel = (Build_FieldAnimCel *) func_8005ABD8(grid, &tint_src);
                anim->cels = cel;
                break;
            }
            if (((u32) (*(u32 *) &def->flags & 0xFF000007) < 2) || (def->handler_group == 3))
            {
                rgb[0] = tint_src->red << 8;
                rgb[1] = tint_src->green << 8;
                rgb[2] = tint_src->blue << 8;
                palette_data = tint_src->palette->data;
                func_8005AC50(palette_data + 2, palette_data[0], rgb);
                primitive_code = 0;
                func_8005AD20(cel->format, tint_src->palette->data[0], &primitive_code);
                anim->frame_data = *arena;
                arena_cursor = *arena;
                switch (cel->format)
                {
                case 0:
                    record_stride = 0xC;
                    break;
                case 2:
                case 3:
                case 4:
                case 5:
                    record_stride = 0xC;
                    break;
                case 1:
                case 6:
                    break;
                }
                record_flags = 1;
                if (cel->code_word != 0)
                {
                    record_stride -= 4;
                }
                else
                {
                    record_flags = 0;
                }
                if (cel->tpage_word != 0)
                {
                    record_flags |= 2;
                    record_stride -= 4;
                }
                frame_descs = def->data;
                rec = def;
                if ((*(u32 *) &def->flags & 0xFF000007) == 1)
                {
                    anim->unk10 = (s32) cel->tiles;
                    frame = def->unk6 - 1;
                    if (frame != -1)
                    {
                        frame -= 1;
                        do
                        {
                            frame -= 1;
                        }
                        while (frame != -1);
                    }
                }
                else
                {
                    frame = def->unk6 - 1;
                    if (frame != -1)
                    {
                        do
                        {
                            tile_data = frame_descs;
                            mask_bit = 1;
                            row = 0;
                            tile_count = 0;
                            mask = cel->mask;
                            mask_word = *mask++;
                            if (grid->u.b.rows != 0)
                            {
                                do
                                {
                                    if (row < rec->unkD)
                                    {
                                        col = grid->u.b.cols - 1;
                                        if (col != -1)
                                        {
                                            do
                                            {
                                                mask_bit *= 2;
                                                if (mask_bit == 0)
                                                {
                                                    mask_word = *mask++;
                                                    mask_bit = 1;
                                                }
                                                col -= 1;
                                            }
                                            while (col != -1);
                                        }
                                    }
                                    else if (row < rec->unkD + rec->unkF)
                                    {
                                        col = 0;
                                        if (grid->u.b.cols != 0)
                                        {
                                            do
                                            {
                                                if ((col >= rec->unkC) && (col < rec->unkC + rec->unkE))
                                                {
                                                    if (mask_word & mask_bit)
                                                    {
                                                        switch (cel->format)
                                                        {
                                                        case 0:
                                                            tile_record = arena_cursor;
                                                            arena_cursor += record_stride;
                                                            field_build_sprite_tile_record(tile_data, tile_record,
                                                                                           (grid->u.word >> 4) & 3, record_flags);
                                                            break;
                                                        case 2:
                                                        case 3:
                                                        case 4:
                                                        case 5:
                                                            tile_record = arena_cursor;
                                                            arena_cursor += record_stride;
                                                            field_build_quad_tile_record(tile_data, tile_record,
                                                                                         (grid->u.word >> 4) & 3, record_flags);
                                                            break;
                                                        case 1:
                                                        case 6:
                                                            break;
                                                        }
                                                        tile_count += 1;
                                                    }
                                                    tile_data += 1;
                                                }
                                                mask_bit *= 2;
                                                if (mask_bit == 0)
                                                {
                                                    mask_word = *mask++;
                                                    mask_bit = 1;
                                                }
                                                col += 1;
                                            }
                                            while (col != grid->u.b.cols);
                                        }
                                    }
                                    else
                                    {
                                        break;
                                    }
                                    row += 1;
                                }
                                while (row != grid->u.b.rows);
                            }
                            frame -= 1;
                            frame_descs += rec->unkE * rec->unkF;
                        }
                        while (frame != -1);
                        anim->frame_tile_count = tile_count;
                        if (def->handler_group == 3)
                        {
                            if (*(u32 *) &def->flags & 0x20)
                            {
                                field_blit_animation_frame(def, anim, 0);
                            }
                        }
                        else if (anim->flags.word & 0x40)
                        {
                            field_blit_animation_frame(def, anim, 0);
                        }
                    }
                }
                *arena = arena_cursor;
            }
            handler_kind = *(u32 *) &def->flags & 0xFF000007;
            if (((u32) (handler_kind - 3) < 2) ||
                ((def->handler_group == 1) && ((u32) (def->flags & 7) >= 2)))
            {
                if (handler_kind == 0x01000002)
                {
                    if (def->unkC == 0)
                    {
                        *arena += 0x50;
                    }
                    else
                    {
                        *arena += 0x410;
                    }
                }
                else if (handler_kind == 0x01000005)
                {
                    if (def->unkC == 0)
                    {
                        *arena += (*(u8 *) &def->unk10 << 6) + 0x10;
                    }
                    else
                    {
                        *arena += (*(u8 *) &def->unk10 << 10) + 0x10;
                    }
                }
                else
                {
                    *arena += 0x10;
                }
            }
            def = def->next;
        }
        while (def != NULL);
    }
    *tail = NULL;
}

void field_build_sprite_tile_record(FieldTileDesc* desc, FieldTileRec* record, s32 texture_depth, s32 record_flags)
{
    u32 u_cell;
    s32 tpage;
    u8 texture_attrs;
    s32 page_slot;

    if (desc->clut_slot & FIELD_TILE_PRESENT)
    {
        u8 packed_uv = desc->packed_uv;

        u_cell = packed_uv & FIELD_TILE_U_MASK;
        record->v = packed_uv & FIELD_TILE_V_MASK;
        switch (texture_depth)
        {
        case FIELD_TEXTURE_4_BIT:
        {
            u32 clut_ref = desc->clut_slot;

            setClut(record, FIELD_TILE_4BIT_CLUT_X(clut_ref), FIELD_TILE_4BIT_CLUT_Y(clut_ref));
        }
        break;
        case FIELD_TEXTURE_8_BIT:
            setClut(record, 0, FIELD_TILE_8BIT_CLUT_Y(desc->clut_slot));
            break;
        default:
            record->clut = 0;
            break;
        }
        if (!(record_flags & FIELD_TILE_REC_SHARED_RGB_CODE))
        {
            record->rgb_code = FIELD_TILE_COLOR_WORDS[desc->color_index];
            if (desc->texture_attrs & FIELD_TILE_SEMITRANS)
            {
                setSemiTrans(record, 1);
            }
        }
        if (!(record_flags & FIELD_TILE_REC_SHARED_TPAGE))
        {
            texture_attrs = desc->texture_attrs;
            page_slot = texture_attrs & FIELD_TILE_TPAGE_SLOT_MASK;
            if ((u32)FIELD_TILE_TPAGE_COLUMN(page_slot, u_cell, texture_depth) >= FIELD_TILE_LOWER_BANK_SLOTS)
            {
                /*
                 * U crossed the ten-slot lower bank. Rebase it onto the upper
                 * bank's first TPage: getTPage(depth, abr, 512, 0).
                 */
                u_cell -= FIELD_TILE_UPPER_BANK_U_REBASE(page_slot);
                tpage = getTPage(texture_depth, FIELD_TILE_ABR(texture_attrs), FIELD_TILE_UPPER_BANK_VRAM_X, 0);
            }
            else
            {
                tpage = getTPage(texture_depth, FIELD_TILE_ABR(texture_attrs), FIELD_TILE_LOWER_BANK_PAGE_X(page_slot), FIELD_TILE_LOWER_BANK_VRAM_Y);
            }
            if (!(record_flags & FIELD_TILE_REC_SHARED_RGB_CODE))
            {
                record->tail.draw_mode = _get_mode(1, 0, tpage);
            }
            else
            {
                record->rgb_code = _get_mode(1, 0, tpage);
            }
        }
        record->u = u_cell * FIELD_TILE_SIZE;
    }
    else
    {
        *(s32*)record = -1;
    }
}

/**
 * @brief Build a two-coordinate field tile render record from its descriptor.
 *
 * Sibling of field_build_sprite_tile_record: same descriptor and the same CLUT/texture-page
 * selection, but it emits a second texture coordinate pair (u + 15, v) and a
 * texture-page halfword at the record tail instead of a GPU draw-mode word.
 *
 * @param desc  Packed 4-byte tile descriptor.
 * @param record Render record to fill in.
 * @param texture_depth PSX texture depth: 0 = 4bpp, 1 = 8bpp, 2 = 15bpp.
 *              It selects the CLUT packing and supplies TPage bits 7-8.
 * @param record_flags FIELD_TILE_REC_SHARED_RGB_CODE omits the per-tile
 *              scratchpad word and shortens the record by four bytes;
 *              FIELD_TILE_REC_SHARED_TPAGE omits the second UV/TPage tuple.
 *
 * @note The canonical PsyQ getTPage() form is a 100% match. Its upper-bank X
 *       argument must retain the `(s32)` cast: page slots are unsigned, and
 *       without the cast gcc emits `srl` instead of the target's `sra` twice
 *       (99.10%).
 * @note Assigning the first TPage directly with setTPage() changes gcc's
 *       expression ordering and adds one instruction (97.76%), so getTPage()
 *       must produce the value before the common tail store.
 * @note The tail stores must be ordered quad.u, quad.v, quad.tpage. Writing
 *       quad.u, quad.tpage, quad.v leaves the load-delay slot unfilled (98.21%).
 * @note `prev` must be a block-local declared inside the `record_flags & 1` arm.
 *       Hoisting it above the `if` as a function-scope variable colors it into
 *       the wrong register and flips the delay-slot fill (97.84%).
 * @note The `switch (texture_depth)` and `u32 clut_ref` requirements are as documented on
 *       field_build_sprite_tile_record; measured here too (94.07% and 99.85% respectively).
 *
 * @see decomp.me (100%) TODO
 */
void field_build_quad_tile_record(FieldTileDesc* desc, FieldTileRec* record, s32 texture_depth, s32 record_flags)
{
    s32 tpage;
    s32 second_tpage;
    u8 texture_attrs;
    u32 page_slot;

    if (desc->clut_slot & FIELD_TILE_PRESENT)
    {
        record->u = (desc->packed_uv & FIELD_TILE_U_MASK) * FIELD_TILE_SIZE;
        record->v = desc->packed_uv & FIELD_TILE_V_MASK;
        switch (texture_depth)
        {
        case FIELD_TEXTURE_4_BIT:
        {
            u32 clut_ref = desc->clut_slot;

            setClut(record, FIELD_TILE_4BIT_CLUT_X(clut_ref), FIELD_TILE_4BIT_CLUT_Y(clut_ref));
        }
        break;
        case FIELD_TEXTURE_8_BIT:
            setClut(record, 0, FIELD_TILE_8BIT_CLUT_Y(desc->clut_slot));
            break;
        default:
            record->clut = 0;
            break;
        }
        if (!(record_flags & FIELD_TILE_REC_SHARED_RGB_CODE))
        {
            record->rgb_code = FIELD_TILE_COLOR_WORDS[desc->color_index];
            if (desc->texture_attrs & FIELD_TILE_SEMITRANS)
            {
                setSemiTrans(record, 1);
            }
        }
        texture_attrs = desc->texture_attrs;
        page_slot = texture_attrs & FIELD_TILE_TPAGE_SLOT_MASK;
        if (page_slot >= FIELD_TILE_LOWER_BANK_SLOTS)
        {
            tpage = getTPage(texture_depth, FIELD_TILE_ABR(texture_attrs), FIELD_TILE_UPPER_BANK_PAGE_X(page_slot), 0);
        }
        else
        {
            tpage = getTPage(texture_depth, FIELD_TILE_ABR(texture_attrs), FIELD_TILE_LOWER_BANK_PAGE_X(page_slot), FIELD_TILE_LOWER_BANK_VRAM_Y);
        }
        record->tail.quad.tpage = tpage;
        if (!(record_flags & FIELD_TILE_REC_SHARED_TPAGE))
        {
            u8 second_texture_attrs = desc->texture_attrs;
            u32 second_page_slot = second_texture_attrs & FIELD_TILE_TPAGE_SLOT_MASK;

            if (second_page_slot >= FIELD_TILE_LOWER_BANK_SLOTS)
            {
                second_tpage = getTPage(texture_depth, FIELD_TILE_ABR(second_texture_attrs), FIELD_TILE_UPPER_BANK_PAGE_X(second_page_slot), 0);
            }
            else
            {
                second_tpage =
                    getTPage(texture_depth, FIELD_TILE_ABR(second_texture_attrs), FIELD_TILE_LOWER_BANK_PAGE_X(second_page_slot), FIELD_TILE_LOWER_BANK_VRAM_Y);
            }
            if (!(record_flags & FIELD_TILE_REC_SHARED_RGB_CODE))
            {
                record->tail.quad.u = ((desc->packed_uv & FIELD_TILE_U_MASK) * FIELD_TILE_SIZE) + 0xF;
                record->tail.quad.v = desc->packed_uv & FIELD_TILE_V_MASK;
                record->tail.quad.tpage = second_tpage;
            }
            else
            {
                FieldTileRec* prev = (FieldTileRec*)((u8*)record - 4);

                prev->tail.quad.u = ((desc->packed_uv & FIELD_TILE_U_MASK) * FIELD_TILE_SIZE) + 0xF;
                prev->tail.quad.v = desc->packed_uv & FIELD_TILE_V_MASK;
                prev->tail.quad.tpage = second_tpage;
            }
        }
    }
    else
    {
        *(s32*)record = -1;
    }
}


/**
 * @brief Size the field working buffer from the current scene's object list.
 *
 * Walks every object in the scene, derives a per-object multiplier from its
 * definition flags, then sums a per-part byte cost over each object's part
 * list. The total gets a 0xA000 header allowance, is clamped to a 0x12000
 * minimum, and is written back to the allocator state at 0x801ED000 as a
 * base / midpoint / top triple (the region is sized to twice the total).
 *
 * @note The four multipliers MUST be assigned to named locals inside the inner
 *       loop. Written inline as `part->instance_count * (n * 0x18)`, gcc reassociates to
 *       `(part->instance_count * 0x18) * n`, which is no longer loop-invariant, so
 *       nothing gets hoisted into the preheader and the multiplies are
 *       strength-reduced inside the loop instead (74.75%). Declaring them in
 *       the loop body lets loop.c hoist all four. See [CSE-05] in idioms.md.
 * @note The dispatch MUST be a `switch`, not an if/else chain (88.54%). gcc
 *       merges `case 2..5` into a single range node, giving a three-node
 *       decision tree that tests `== 1`, then `< 2`, then `< 6` -- and it emits
 *       the case bodies in case-label order after the tests, which an if/else
 *       chain cannot reproduce. No jump table is generated.
 * @note `kind` must be `s32`, not `u8`: `u8` compares unsigned (`sltiu`) where
 *       the target uses signed `slti` (98.23%).
 * @note `obj->def->flags` must be re-read for the `& 8` test rather than
 *       reusing the `flags` local; reusing it drops the second load (95.91%).
 * @note `total` must be accumulated in place (`total = total + 0xA000`, then
 *       clamped in place) rather than assigned to a second variable, which
 *       colors it into the wrong register (99.34%). See [ALLOC-17].
 * @note The 0x12000 limit must go through a variable. Compared directly,
 *       gcc rewrites `total < 0x12000` into the negated `0x11FFF < total`
 *       and flips the branch (99.28%).
 * @note Measured non-factor: `n * 2` vs `n << 1`, both 100%.
 *
 * @see decomp.me (100%) TODO
 */
void field_size_work_buffer(void)
{
    FieldMemState* state = (FieldMemState*)0x801ED000;
    FieldObj* obj;
    FieldPart* part;
    s32 n;
    u32 total;
    u32 base;
    u32 lim;

    total = 0;
    obj = g_field_scene.scene->objects;
    if (obj != 0)
    {
        do
        {
            s32 flags = obj->def->flags;

            n = 1;
            if (flags & 4)
            {
                n = 3;
                if (flags & 0x30)
                {
                    n = 2;
                }
            }
            if (obj->def->flags & 8)
            {
                n = n * 2;
            }
            part = obj->parts;
            if (part != 0)
            {
                do
                {
                    s32 m18 = n * 0x18;
                    s32 m1C = n * 0x1C;
                    s32 m28 = n * 0x28;
                    s32 m34 = n * 0x34;

                    if (part->instance_count != 0)
                    {
                        s32 kind = part->kind;

                        switch (kind)
                        {
                        case 0:
                            total += part->instance_count * m18;
                            break;
                        case 1:
                            total += m1C;
                            break;
                        case 2:
                        case 3:
                        case 4:
                        case 5:
                            total += part->instance_count * m28;
                            break;
                        default:
                            total += part->instance_count * m34;
                            break;
                        }
                    }
                    part = part->next;
                } while (part != 0);
            }
            obj = obj->next;
        } while (obj != 0);
    }
    total = total + 0xA000;
    base = state->top;
    lim = 0x12000;
    if (total < lim)
    {
        total = 0x12000;
    }
    state->midpoint = base + total;
    state->base = base;
    state->top = base + total * 2;
}
