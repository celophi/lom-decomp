#include "field_scene_internal.h"
#include "scene_state.h"

/**
 * @brief Tile-animation view of FieldAnimDef used while preparing presence masks.
 */
typedef struct FieldAnimRasterDef
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
    struct FieldAnimRasterDef* next;
    u8 rect_x;
    u8 rect_y;
    u8 rect_width;
    u8 rect_height;
    FieldTileGrid* part_def;
    FieldTileDesc* frame_tiles;
} FieldAnimRasterDef;

/**
 * @brief Screen-space placement of the grid being drawn.
 *
 * field_emit_sprite_grid only needs the origin; field_emit_rotated_sprite_grid also reads the
 * width and camera position to derive the rotation centre for its non-default placement modes.
 */
typedef struct
{
    /** Screen-space origin of the grid. */
    s32 x;
    s32 y;
    /** Scene width in pixels, from FieldSceneHeader::unk30. */
    s32 width;
    /** Camera position in screen pixels. */
    s32 camera_x;
    s32 camera_y;
} FieldViewport;

/**
 * @brief GPU primitive as field_emit_sprite_grid writes it: four raw words.
 *
 * Layout-compatible with SPRT_16 (tag / rgb+code / x0+y0 / u0+v0+clut) and,
 * for the 8-byte form, with DR_TPAGE. It is declared as plain words rather
 * than reusing those Psy-Q types because every field is written as one whole
 * 32-bit store; going through setaddr/setlen or the byte members turns each
 * tag write into a read-modify-write and costs the match.
 */
typedef struct
{
    u32 tag;  /* 0x00 */
    u32 code; /* 0x04 */
    u32 xy;   /* 0x08 packed (y << 16) | (x & 0xFFFF) */
    u32 uv;   /* 0x0C uv pair plus CLUT id */
} FieldPrim;

/**
 * @brief One entry of FieldPart::records, consumed per set bit plane bit.
 *
 * The stride is 0xC bytes, less 4 when the part carries a global code word and
 * another 4 when it carries a global texture page, so unk4/unk8 are only
 * present in the longer forms.
 */
typedef struct
{
    /** 0x00 uv pair plus CLUT id; -1 means the cell emits nothing. */
    s32 uv_clut;
    /** 0x04 rgb/code word used when the part has no global code word. */
    s32 rgb_code;
    /** 0x08 texture-page word tested against the running page code. */
    s32 tpage;
} FieldCellRec;

/**
 * @brief POLY_FT4 as field_emit_rotated_sprite_grid writes it: ten raw words.
 *
 * Layout-compatible with Psy-Q's POLY_FT4 (tag / rgb+code / four x,y pairs each
 * followed by its u,v pair). It is declared as plain words rather than reusing
 * POLY_FT4 because every field is written as one whole 32-bit store: the vertex
 * slots take a packed (x,y) pair straight out of the point buffer, and going
 * through the byte members or setXY0 would turn each into a read-modify-write.
 */
typedef struct
{
    u32 tag;  /* 0x00 */
    u32 code; /* 0x04 */
    u32 xy0;  /* 0x08 */
    u32 uv0;  /* 0x0C uv pair plus CLUT id, straight from the record */
    u32 xy1;  /* 0x10 */
    u32 uv1;  /* 0x14 uv pair plus texture page */
    u32 xy2;  /* 0x18 */
    u32 uv2;  /* 0x1C */
    u32 xy3;  /* 0x20 */
    u32 uv3;  /* 0x24 */
} FieldPolyPrim;

/**
 * @brief One column's rotated unit step, cached in the scratchpad at 0x1F800000.
 *
 * There are width + 1 of these, one per column edge. Each holds the column
 * offset already multiplied by the grid's sine and cosine, so the per-row pass
 * only has to add the row's contribution and shift.
 */
typedef struct
{
    s32 sin_term; /* 0x00 column offset * sin */
    s32 cos_term; /* 0x04 column offset * cos */
} FieldColStep;

/**
 * @brief A screen-space point in one of the two scratchpad row buffers.
 *
 * The pair is compared component-wise for the viewport reject but copied into
 * the primitive as a single word, so the two views have to share storage.
 */
typedef union
{
    /** Packed (y << 16) | (x & 0xFFFF), as stored into a POLY_FT4 vertex. */
    s32 word;
    struct
    {
        s16 x;
        s16 y;
    } p;
} FieldPoint;

/**
 * @brief Find the runtime part whose definition pointer matches @p part_def.
 *
 * @param part_def Part definition to find in the scene object lists.
 * @param owner_out Optional output for the part's owning object/tint-source view.
 * @return Matching runtime part, or NULL when the definition is not in use.
 */
FieldAnimCel *func_8005ABD8(FieldTileGrid *grid, FieldTintSrc **owner_out);

/**
 * @brief FieldObj as field_build_render_records fills it in.
 *
 * Same layout as FieldObj, but with the 0x10..0x1B block named as the tint
 * halfwords FieldTintSrc reads (FieldObj declares it as the two words
 * field_find_shareable_part compares).
 */
typedef struct FieldObjBuild FieldObjBuild;
typedef struct FieldPartBuild FieldPartBuild;
struct FieldObjBuild
{
    FieldObjBuild* next;   /* 0x00 */
    FieldObjDef* def;      /* 0x04 */
    FieldPartBuild* parts; /* 0x08 */
    FieldObjFlags flags;   /* 0x0C */
    /** 0x10 tint, 8.8 fixed point, from the definition's percentages. */
    u16 red;
    u16 green; /* 0x12 */
    u16 blue;  /* 0x14 */
    /** 0x16 second tint multiplier, 0x100 = unscaled. */
    u16 red_scale;
    u16 green_scale; /* 0x18 */
    u16 blue_scale;  /* 0x1A */
    s32 x;       /* 0x1C */
    s32 y;       /* 0x20 */
    s32 z;       /* 0x24 */
    s32 drift_x; /* 0x28 */
    s32 drift_y; /* 0x2C */
};

/**
 * @brief FieldPart as field_build_render_records fills it in.
 *
 * Same layout as FieldPart, plus the two members FieldPart leaves unnamed:
 * the bit-plane byte size at 0x14 and the halfword at 0x34.
 */
struct FieldPartBuild
{
    FieldPartBuild* next;   /* 0x00 */
    FieldPartDef* def;      /* 0x04 */
    /** 0x08 part whose records this one reuses, or NULL. */
    FieldPartBuild* shared;
    u32* bits;              /* 0x0C */
    u8* records;            /* 0x10 */
    /** 0x14 byte size of the bit plane at 0x0C. */
    s32 bits_size;
    s32 tpage_word;         /* 0x18 */
    s32 code_word;          /* 0x1C */
    u8 visible;             /* 0x20 */
    u8 kind;                /* 0x21 */
    u8 node_count;          /* 0x22 */
    u8 _pad2[0x26 - 0x23];
    u16 instance_count;     /* 0x26 */
    s32 x;                  /* 0x28 */
    s32 y;                  /* 0x2C */
    s32 z;                  /* 0x30 */
    s16 unk34;              /* 0x34 */
    u16 sweep_period;       /* 0x36 */
    u16 sweep_phase;        /* 0x38 */
    u16 row_angle;          /* 0x3A */
    u16 column_angle;       /* 0x3C */
    u16 rotation_angle;     /* 0x3E */
    u16 scale_x;            /* 0x40 */
    u16 scale_y;            /* 0x42 */
    s16 clut_bl;            /* 0x44 */
    s16 clut_tl;            /* 0x46 */
    s16 clut_br;            /* 0x48 */
    s16 clut_tr;            /* 0x4A */
};

/**
 * @brief Clamp a depth-biased CLUT row to 0..0x7FF.
 *
 * @param v Signed CLUT row.
 * @return @p v clamped to the CLUT range.
 */
#define CLAMP_CLUT(v) (((v) > 0) ? (((v) >= 0x800) ? 0x7FF : (v)) : 0)

/**
 * @brief Generic view of a scene list element: every list links through offset 0.
 *
 * The build passes append through one tail pointer of this type, starting from
 * the list head inside FieldScene.
 */
typedef struct FieldLink
{
    struct FieldLink* next; /* 0x00 */
} FieldLink;

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
extern void DecDCTReset(int mode);
extern void DecDCTvlcBuild(u_short *table);
void field_prepare_animation_definitions(FieldAnimRasterDef *, s32);
void field_build_animation_list(FieldAnimDef *, u8 **, FieldAnim **);
void field_build_sprite_tile_record(FieldTileDesc *, FieldTileRec *, s32, s32);
void field_build_quad_tile_record(FieldTileDesc *, FieldTileRec *, s32, s32);
FieldPart *field_find_shareable_part(FieldScene *scene, FieldObj *obj, FieldPart *part, s32 key);
void field_draw_part(FieldPart *part, u8 **cursor, FieldViewport *origin, u_long *ot);
void func_8005A744(void *, s32);
void *func_8005AB4C(u8);
void *func_8005AB80(u8, u8);
void func_8005AC50(void *, u16, s32 *);
void func_8005AD20(u8, u16, s8 *);
extern s32 g_field_dyn_count;
/* Shared work-area words at 0x80180008 / 0x80180018, adjacent to g_field_scene.
   Referencing them as symbols (rather than as offsets off a literal base) is
   what makes gcc emit the target's %hi/%lo relocations here. */
extern u16 D_80180008;

/**
 * @brief Build the scene's per-object render records.
 *
 * @param arg0 Object being built; its unk26 is set to 1 on completion.
 * @param arg1 TODO: caller passes the low halfword of field_scene_load's arg0;
 *             meaning not yet established.
 * @return Nothing.
 *
 * @note MATCHED - 100% (1174/1174 exact rows, gcc280_g4_noexpanddiv), verified
 *       in-tree 2026-08-25. Six former operand-only rows were a splat
 *       symbolization artifact: the retail bytes hold one literal 0x8018 page
 *       base shared across loads at +0x10/+0x14/+0x18/+0x1C, which splat
 *       reconstructed as %hi/%lo(g_field_scene / g_field_node_angle_table /
 *       g_field_dyn_count / D_80180018). Those false-positive relocations are
 *       now stripped per-address in config/relocations/field_reloc_addrs.txt
 *       (rom 0x29D1/0x2A09/0x2A35/0x3AC1/0x3AC9/0x3ACD).
 * @note The `global_page = (u8*)0x80180000` base is required to match and must
 *       NOT be rewritten to reference g_field_scene / g_field_node_angle_table
 *       directly: the literal base is what makes gcc share one %hi across the
 *       loads, exactly as the target does. Measured: each named-symbol form costs
 *       +1 insn per symbol pair (1174 -> 1175 -> 1176), because gcc 2.8.0 will
 *       not CSE %hi across two different symbols.
 * @note The remaining `do { ... } while (0)` wrappers are REQUIRED to match - do
 *       not delete them. They supply gcc's loop notes: replacing one with a plain
 *       block, `if (1)`, `switch (0)` or a label costs 62 exact rows, while
 *       `while (1) { ...; break; }` is byte-identical. Nest depth matters too (the
 *       `var_s7++` site needs at least 6 levels). 37 other wrappers were pure
 *       noise and have been removed; these 17 stand in for real loop structure in
 *       the original source that has not been recovered yet.
 * @note The body is byte-exact but still carries matching scaffolding rather
 *       than a recovered source shape: the `scratch` union aliases sp10[3] with
 *       two volatile slots to pin stack offsets, several member accesses are
 *       written as raw `*(T*)((u8*)p + 0xNN)` casts, and the loop-note wrappers
 *       above are stand-ins. All are to be replaced as the real structure is
 *       identified - with asm re-verification on every change.
 * @see decomp.me (95.60%) https://decomp.me/scratch/i4GmA - earlier scratch; the
 *      100% body below came from local permuter runs plus the Wave 24/25
 *      HOMING + BARRIER handoff, not from that link.
 * @see working/field_build_render_records/STATUS.md - measurements, the per-site
 *      wrapper costs, and the list of ruled-out source shapes.
 */
void field_build_render_records(FieldMapObject *map, u16 object_index)
{
  s32 rgb[3];
  s8 code;
  struct { Records_Unk *cur; } arena;
  FieldMemState *mem;
  FieldScene *sp34;
  FieldObjBuild *prev_obj;
  s32 sp3C;
  s32 sp50;
  s32 sp58;
  s32 sp60;
  s32 sp64;
  Records_Unk *sp68;
  Records_Unk **sp6C;
  s32 sp80;
  Records_Unk *new_var23;
  s16 *temp_a1_5;
  s32 axle_s1;
  FieldNode *node;
  Records_Unk *temp_s1_2;
  Records_Unk *var_s3;
  int new_var7;
  Records_Unk *var_s7_2;
  unsigned int new_var14;
  u32 temp_v0_early;
  Records_Unk *var_s7_3;
  char new_var3;
  Records_Node30 *var_t0_2;
  FieldObjBuild *var_t0_3;
  FieldLink *tail;
  FieldNode *node_prev;
  Records_Unk *var_t1_5;
  FieldPartBuild *prev_part;
  FieldPartDef **part_defs;
  s16 *var_t5_2;
  s16 *points;
  s16 temp_a1;
  s16 temp_a1_2;
  s16 temp_a1_3;
  s16 temp_a1_4;
  s32 temp_v0;
  s32 scale14;
  int new_var;
  s32 early_outer_end;
  s32 early_inner_end;
  s32 temp_v0_2;
  s32 temp_v0_3;
  s32 temp_v0_4;
  s32 var_a1_2;
  u8 *new_var2;
  s16 var_t1_3;
  s16 var_v0_2;
  s16 var_v0_3;
  s16 var_v0_4;
  s16 var_v0_5;
  s32 var_v1;
  s16 *var_a2;
  s32 temp_a0;
  s32 temp_a2_3;
  s32 temp_a2_4;
  s32 temp_t1;
  s32 temp_t1_2;
  s32 temp_t3;
  s32 temp_t4;
  s32 temp_t7;
  s32 temp_v0_10;
  s32 temp_v0_11;
  s32 temp_v0_12;
  s32 temp_v0_5;
  s32 temp_v0_6;
  s32 temp_v1_13;
  s32 temp_v1_14;
  s32 tail_end;
  s32 temp_v1_7;
  s32 temp_v1_8;
  s32 var_s0;
  s32 var_s0_6;
  u8 *new_var13;
  int new_var4;
  s32 var_s1_4;
  Records_Src4 *new_var12;
  s32 var_s5;
  s32 var_s6;
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
  u16 temp_v1;
  int new_var11;
  u16 temp_v1_2;
  u16 temp_v1_3;
  u16 temp_v1_4;
  u16 temp_v1_5;
  u16 temp_v1_6;
  u32 new_var19;
  u16 var_a0;
  s16 hi;
  s16 lo;
  u16 var_t2;
  Records_Node38 *new_var16;
  u8 *new_var17;
  u16 var_t2_2;
  u16 *palette;
  union
  {
    FieldNodeRun *run;
    FieldMarker *marker;
    FieldObjBuild *obj;
  } rec;
  FieldNodeRun *run;
  u32 temp_a0_3;
  s32 clut;
  s32 stride;
  s32 words;
  FieldSeq *seq;
  FieldSeqDef *seq_def;
  s32 clamped;
  s32 clamp_arm1;
  s32 clamp_arm2;
  s32 clamp_arm3;
  s32 clamp_arm4;
  int probe_const;
  u32 var_v0_6;
  u8 temp_v0_7;
  short temp_v1_12;
  u8 temp_v1_9;
  u16 new_var5;
  int new_var22;
  s32 var_v0;
  Records_Unk *var_a0_7;
  int new_var21;
  int new_var8;
  u8 *var_fp;
  Records_Unk *temp_a3;
  FieldPartDef *part_def;
  Records_Unk *temp_v0_8;
  Records_Unk *temp_v0_9;
  Records_Unk *temp_v1_10;
  FieldPartBuild *shared;
  Records_SrcObj *var_a3;
  FieldNodeDef *node_def;
  union
  {
    FieldMarkerDef *marker;
    FieldNodeDef *node;
  } def;
  Records_Unk *var_s1_2;
  unsigned char new_var20;
  FieldPartBuild *part;
  s32 var_s2_2;
  FieldObjDef *obj_def;
  u8 *new_var15;
  Records_Unk **var_s7;
  Records_SrcObj3 **var_t6;
  s32 carriage_quad_cursor;
  mem = FIELD_MEM_STATE;
  var_t3 = 0;
  var_t4 = 0;
  var_t8 = 0;
  sp80 = 0;
  sp34 = FIELD_RESOURCE->scene;
  sp3C = 0;
  arena.cur = 0;
  sp34->header = (FieldSceneHeader *) map;
  *((s32 *) (((u8 *) sp34) + 0xC)) = 0;
  node_def = map->node_defs;
  arena.cur = (Records_Unk *) (((u8 *) sp34) + 0x74);
  tail = (FieldLink *) &sp34->nodes;
  var_t5_2 = FIELD_RESOURCE->points;
  if (node_def != 0)
  {
    do
    {
      node = (FieldNode *) arena.cur;
      arena.cur = (Records_Unk *) (node + 1);
      tail->next = (FieldLink *) node;
      tail = (FieldLink *) node;
      node->def = node_def;
      node->spans = 0;
      node->unk14 = 0;
      node->unk18 = *(u8 *) &(node_def)->flags >> FIELD_NODE_DEF_ENABLE_SHIFT;
      node->x_min = 0x7FFF;
      node->x_max = 0;
      node->row_end = 0;
      node->row_start = 0x7FFF;
      node->unk24 = 0;
      node->delta_x = 0;
      node->delta_y = 0;
      node->unk30 = 0;
      node->unk34 = 0;
      node->x = 0;
      node->y = 0;
      node->unk40 = 0;
      rec.run = node_def->runs;
      var_s0 = rec.run->count & FIELD_NODE_RUN_COUNT_MASK;
      while (var_s0 != 0)
      {
        temp_a1_5 = var_t5_2 + rec.run->first * 2;
        while (--var_s0 != -1)
        {
          node->x_min = (*temp_a1_5 > node->x_min) ? node->x_min : *temp_a1_5;
          node->x_max = (*temp_a1_5 < node->x_max) ? node->x_max : *temp_a1_5;
          node->row_end = (temp_a1_5[1] < node->row_end) ? node->row_end : temp_a1_5[1];
          node->row_start = (temp_a1_5[1] > node->row_start) ? node->row_start : temp_a1_5[1];
          temp_a1_5 += 2;
        }
        rec.run++;
        var_s0 = rec.run->count & FIELD_NODE_RUN_COUNT_MASK;
      }
      node_def = node_def->next;
    }
    while (node_def != 0);
  }
  tail->next = 0;
  def.marker = map->edge_defs;
  tail = (FieldLink *) &sp34->markers;
  if (def.marker != 0)
  {
    do
    {
      rec.marker = (FieldMarker *) arena.cur;
      arena.cur = (Records_Unk *) (rec.marker + 1);
      tail->next = (FieldLink *) rec.marker;
      tail = (FieldLink *) rec.marker;
      rec.marker->def = def.marker;
      rec.marker->x2 = def.marker->x0 + def.marker->offset_x;
      rec.marker->y2 = def.marker->y0 + def.marker->offset_y;
      rec.marker->x3 = def.marker->x1 + def.marker->offset_x;
      rec.marker->y3 = def.marker->y1 + def.marker->offset_y;
      hi = lo = def.marker->x0;
      if (hi < (s16) def.marker->x1)
      {
        hi = def.marker->x1;
      }
      if ((s16) def.marker->x1 < lo)
      {
        lo = def.marker->x1;
      }
      if (hi < (s16) rec.marker->x2)
      {
        hi = rec.marker->x2;
      }
      if ((s16) rec.marker->x2 < lo)
      {
        lo = rec.marker->x2;
      }
      if (hi < (s16) rec.marker->x3)
      {
        hi = rec.marker->x3;
      }
      if ((s16) rec.marker->x3 < lo)
      {
        lo = rec.marker->x3;
      }
      rec.marker->x_max = hi;
      rec.marker->x_min = lo;
      hi = lo = def.marker->y0;
      if (hi < (s16) def.marker->y1)
      {
        hi = def.marker->y1;
      }
      if ((s16) def.marker->y1 < lo)
      {
        lo = def.marker->y1;
      }
      if (hi < (s16) rec.marker->y2)
      {
        hi = rec.marker->y2;
      }
      if ((s16) rec.marker->y2 < lo)
      {
        lo = rec.marker->y2;
      }
      if (hi < (s16) rec.marker->y3)
      {
        hi = rec.marker->y3;
      }
      if ((s16) rec.marker->y3 < lo)
      {
        lo = rec.marker->y3;
      }
      rec.marker->y_max = hi;
      rec.marker->y_min = lo;
      rec.marker->side_dx = (s16) rec.marker->x2 - (s16) def.marker->x0;
      rec.marker->side_dy = (s16) rec.marker->y2 - (s16) def.marker->y0;
      if (rec.marker->side_dx != 0)
      {
        var_a1_2 = (s16) def.marker->y0 - rec.marker->side_dy * (s16) def.marker->x0 / rec.marker->side_dx;
        var_v1 = (s16) def.marker->y1 - rec.marker->side_dy * (s16) def.marker->x1 / rec.marker->side_dx;
      }
      else
      {
        var_a1_2 = (s16) def.marker->x0;
        var_v1 = (s16) def.marker->x1;
      }
      if (var_v1 < var_a1_2)
      {
        rec.marker->side_hi = var_a1_2;
        rec.marker->side_lo = var_v1;
      }
      else
      {
        rec.marker->side_lo = var_a1_2;
        rec.marker->side_hi = var_v1;
      }
      rec.marker->edge_dx = (s16) def.marker->x1 - (s16) def.marker->x0;
      rec.marker->edge_dy = (s16) def.marker->y1 - (s16) def.marker->y0;
      if (rec.marker->edge_dx != 0)
      {
        var_a1_2 = (s16) def.marker->y0 - rec.marker->edge_dy * (s16) def.marker->x0 / rec.marker->edge_dx;
        var_v1 = (s16) rec.marker->y2 - rec.marker->edge_dy * (s16) rec.marker->x2 / rec.marker->edge_dx;
      }
      else
      {
        var_a1_2 = (s16) def.marker->x0;
        var_v1 = (s16) rec.marker->x2;
      }
      if (var_v1 < var_a1_2)
      {
        rec.marker->edge_hi = var_a1_2;
        rec.marker->edge_lo = var_v1;
      }
      else
      {
        rec.marker->edge_lo = var_a1_2;
        rec.marker->edge_hi = var_v1;
      }
      def.marker = def.marker->next;
    }
    while (def.marker != 0);
  }
  tail->next = 0;
  var_t6 = map->object_defs;
  prev_obj = (FieldObjBuild *) &sp34->objects;
  if ((*var_t6) != 0)
  {
    do
    {
      rec.obj = (FieldObjBuild *) arena.cur;
      obj_def = (FieldObjDef *) *var_t6;
      arena.cur = (Records_Unk *) (rec.obj + 1);
      prev_obj->next = rec.obj;
      rec.obj->def = obj_def;
      rec.obj->red = (obj_def->scale_x << 8) / 100;
      var_t1_4 = 1;
      rec.obj->green = (obj_def->scale_y << 8) / 100;
      rec.obj->blue = (obj_def->scale_z << 8) / 100;
      rec.obj->red_scale = rec.obj->green_scale = rec.obj->blue_scale = 0x100;
      rec.obj->flags.word = (rec.obj->flags.word & ~1) | (*(u8 *) &obj_def->flags & 1);
      rec.obj->flags.b.node_count = 0;
      rec.obj->x = obj_def->x << 8;
      rec.obj->y = obj_def->y << 8;
      prev_obj = rec.obj;
      var_s7_2 = (Records_Unk *) rec.obj;
      rec.obj->z = obj_def->z << 8;
      if ((*(s32 *) &obj_def->scroll_scale_x & 0xFFFF0000) == 0x100000)
      {
        rec.obj->flags.b.drift_speed = 0;
      }
      else
      {
        rec.obj->flags.b.drift_speed = obj_def->drift_speed;
      }
      ((FieldObjBuild *) var_s7_2)->flags.b.drift_angle = obj_def->drift_angle;
      ((FieldObjBuild *) var_s7_2)->drift_x = 0;
      ((FieldObjBuild *) var_s7_2)->drift_y = 0;
      part_defs = obj_def->part_defs;
      prev_part = (FieldPartBuild *) &((FieldObjBuild *) var_s7_2)->parts;
      if (*part_defs != 0)
      {
        do
        {
          part = (FieldPartBuild *) arena.cur;
          part_def = *part_defs;
          arena.cur = (Records_Unk *) (part + 1);
          prev_part->next = part;
          part->def = part_def;
          part->visible = *(u8 *) &part_def->u.word & 1;
          temp_a0 = part_def->u.word;
          prev_part = part;
          if ((temp_a0 & 0xF00) == 0x100)
          {
            var_v0 = (temp_a0 & 0xE) + 1;
          }
          else
          {
            var_v0 = temp_a0 & 0xE;
          }
          part->kind = var_v0;
          part->node_count = 0;
          part->x = part_def->x << 8;
          part->y = part_def->y << 8;
          part->z = part_def->z << 8;
          part->unk34 = 0;
          part->sweep_period = part_def->sweep_period;
          part->sweep_phase = var_t1_4;
          part->row_angle = 0;
          part->column_angle = 0;
          part->rotation_angle = 0;
          part->scale_x = 0x1000;
          part->scale_y = 0x1000;
          if (part_def->u.word & 0x40)
          {
            clut = obj_def->z + part_def->z + part_def->clut_bl;
            if (clut > 0)
            {
              clamped = clut;
              if (clut >= 0x800)
              {
                clamped = 0x7FF;
              }
              var_v0_5 = clamped;
            }
            else
            {
              var_v0_5 = 0;
            }
            part->clut_bl = var_v0_5;
            clut = obj_def->z + part_def->z + part_def->clut_tl;
            if (clut > 0)
            {
              clamped = clut;
              if (clut >= 0x800)
              {
                clamped = 0x7FF;
              }
              var_v0_5 = clamped;
            }
            else
            {
              var_v0_5 = 0;
            }
            part->clut_tl = var_v0_5;
            clut = obj_def->z + part_def->z + part_def->clut_br;
            if (clut > 0)
            {
              clamped = clut;
              if (clut >= 0x800)
              {
                clamped = 0x7FF;
              }
              var_v0_5 = clamped;
            }
            else
            {
              var_v0_5 = 0;
            }
            part->clut_br = var_v0_5;
            clut = obj_def->z + part_def->z + part_def->clut_tr;
            if (clut > 0)
            {
              clamped = clut;
              if (clut >= 0x800)
              {
                clamped = 0x7FF;
              }
              var_v0_5 = clamped;
            }
            else
            {
              var_v0_5 = 0;
            }
            part->clut_tr = var_v0_5;
          }
          else
          {
            part->clut_bl = part_def->clut_bl;
            part->clut_tl = part_def->clut_tl;
            part->clut_br = part_def->clut_br;
            part->clut_tr = part_def->clut_tr;
          }
          var_s6 = 0;
          var_fp = (u8 *) part_def->key;
          var_s5 = (part->kind > 0U) * 2;
          if (var_fp != 0)
          {
            part->shared = (FieldPartBuild *) field_find_shareable_part(sp34, (FieldObj *) var_s7_2, (FieldPart *) part, (s32) var_fp);
            if (part->shared == 0)
            {
              if ((part_def->u.word & 0xF00) == 0x100)
              {
                var_s0 = 1;
              }
              else
              {
                var_s0 = part_def->u.b.cols * part_def->u.b.rows;
              }
              var_s3 = (Records_Unk *) (part->bits = (u32 *) arena.cur);
              words = (var_s0 + 31) / 32;
              part->bits_size = words * 4;
              arena.cur = (Records_Unk *) ((u32 *) var_s3 + words);
              sp3C = 0;
              var_s1_4 = 1;
              while (--var_s0 != -1)
              {
                if (var_fp[0] & 0x80)
                {
                  sp3C |= var_s1_4;
                  if (var_s5 == 0)
                  {
                    temp_v0_7 = var_fp[1];
                    var_s5 = 1;
                    var_t3 = temp_v0_7 & 0xF;
                    sp80 = (temp_v0_7 >> 4) & 3;
                  }
                  else if ((var_s5 == 1) && ((var_t3 != (var_fp[1] & 0xF)) || (sp80 != ((var_fp[1] >> 4) & 3))))
                  {
                    var_s5 = 2;
                  }
                  if (var_s6 == 0)
                  {
                    var_s6 = 1;
                    var_t4 = var_fp[3];
                    var_t8 = (var_fp[1] >> 6) & 1;
                  }
                  else if ((var_s6 == 1) && ((var_t4 != var_fp[3]) || (var_t8 != ((var_fp[1] >> 6) & 1))))
                  {
                    var_s6 = 2;
                  }
                }
                var_s1_4 <<= 1;
                if (var_s1_4 == 0)
                {
                  *(u32 *) var_s3 = sp3C;
                  var_s3 = (Records_Unk *) ((u32 *) var_s3 + 1);
                  var_s1_4 = 1;
                  sp3C = 0;
                }
                var_fp += 4;
              }
              if (var_s1_4 != 1)
              {
                *(u32 *) var_s3 = sp3C;
              }
              if (var_s5 == 1)
              {
                part->tpage_word = (s32) ((var_t3 + (sp80 * 0x10)) + 1);
              }
              else
              {
                part->tpage_word = 0;
              }
              if (var_s6 == 1)
              {

                part->code_word = (s32) ((var_t4 + (var_t8 << 9)) + 1);
              }
              else
              {
                part->code_word = 0;
              }
            }
          }
          do { do { part_defs++; } while (0); } while (0);
        }
        while (*part_defs != 0);
      }
      var_t6 += 1;
      prev_part->next = 0;
    }
    while ((*var_t6) != 0);
  }
  prev_obj->next = 0;
  node = sp34->nodes;
  if (node != 0)
  {
    do
    {
      def.node = node->def;
      if ((D_80180008 >= 0x12) && (def.node->obj_index != 0xFF))
      {
        if (def.node->part_index != 0xFF)
        {
          node->obj = 0;
          node->part = func_8005AB80(def.node->obj_index, def.node->part_index);
          if (node->part->def->u.word & 0xF000)
          {
            sp34->secondary_nodes = node;
          }
          node->part->node_count++;
        }
        else
        {
          node->obj = func_8005AB4C(def.node->obj_index);
          node->obj->flags.b.node_count++;
          node->part = 0;
        }
      }
      else
      {
        node->obj = 0;
        node->part = 0;
      }
      node = node->next;
    }
    while (node != 0);
  }
  field_prepare_animation_definitions(map->anim_defs[0], 0);
  field_prepare_animation_definitions(map->anim_defs[1], 1);
  field_prepare_animation_definitions(map->anim_defs[2], 2);
  field_prepare_animation_definitions(map->anim_defs[3], 3);
  var_t0_3 = (FieldObjBuild *) sp34->objects;
  if (var_t0_3 != 0)
  {
    do
    {
      obj_def = var_t0_3->def;
      rgb[0] = var_t0_3->red << 8;
      rgb[1] = var_t0_3->green << 8;
      rgb[2] = var_t0_3->blue << 8;
      palette = (u16 *) obj_def->shared_source;
      func_8005AC50(palette + 2, *palette, rgb);
      part = var_t0_3->parts;
      code = 0;
      if (part != 0)
      {
        do
        {
          func_8005AD20(part->kind, *(u16 *) obj_def->shared_source, &code);
          part_def = part->def;
          var_fp = (u8 *) part_def->key;
          if (var_fp != 0)
          {
            shared = part->shared;
            if (shared != 0)
            {
              part->bits = (s32 *) shared->bits;
              part->bits_size = (s32) shared->bits_size;
              part->tpage_word = (s32) shared->tpage_word;
              part->code_word = (s32) shared->code_word;
              part->records = (Records_Unk *) shared->records;
              part->instance_count = (u16) shared->instance_count;
            }
            else
            {
              var_t1_4 = 0;
              switch (part->kind)
              {
                case 0:
                  stride = 0xC;
                  temp_v1_14 = part->code_word;
                  part->records = (u8 *) arena.cur;
                  var_s7 = (Records_Unk **) part->records;
                  if (temp_v1_14 != 0)
                  {
                    var_t4 = temp_v1_14 - 1;
                    part->code_word = FIELD_TILE_COLOR_WORDS[var_t4 & 0xFF];
                    var_s6 = FIELD_TILE_REC_SHARED_RGB_CODE;
                    if (var_t4 & 0x200)
                    {
                      ((u8 *) &part->code_word)[3] |= 2;
                    }
                    stride = 8;
                  }
                  else
                  {
                    var_s6 = 0;
                  }
                  temp_v0_10 = part->tpage_word;
                  var_t3 = temp_v0_10 - 1;
                  if (temp_v0_10 != 0)
                  {
                    temp_a0_3 = var_t3 & 0xF;
                    if (temp_a0_3 >= 10)
                    {
                      var_t3 = ((part_def->u.word * 8) & 0x180) | ((var_t3 * 2) & 0x60) | ((u32) (((temp_a0_3 << 6) - 0x80) & 0x3FF) >> 6);
                    }
                    else
                    {
                      new_var4 = 0x10;
                      var_t3 = ((part_def->u.word * 8) & 0x180) | ((var_t3 * 2) & 0x60) | (((u32) ((temp_a0_3 << 6) + 0x140) >> 6) | new_var4);
                    }
                    part->tpage_word = (var_t3 & 0x9FF) | 0xE1000400;
                    var_s6 |= FIELD_TILE_REC_SHARED_TPAGE;
                    stride -= 4;
                  }
                  var_s3 = (Records_Unk *) part->bits;
                  var_s0 = part_def->u.b.cols * part_def->u.b.rows;
                  var_s0--;
                  var_s1_4 = 0;
                  if (var_s0 != -1)
                  {
                    var_v1_5 = -1;
                    do
                    {
                      if (var_s1_4 == 0)
                      {
                        sp3C = *(u32 *) var_s3;
                        var_s3 = (Records_Unk *) ((u32 *) var_s3 + 1);
                        var_s1_4 = 1;
                      }
                      if (sp3C & var_s1_4)
                      {
                        field_build_sprite_tile_record((FieldTileDesc *) var_fp, (FieldTileRec *) var_s7, (part_def->u.word >> 4) & 3, var_s6);
                        var_s7 = (Records_Unk **) ((u8 *) var_s7 + stride);
                        var_t1_4++;
                      }
                      var_s1_4 <<= 1;
                      var_s0--;
                      var_fp += 4;
                    }
                    while (var_s0 != var_v1_5);
                  }
                  arena.cur = (Records_Unk *) ((u8 *) arena.cur + (u16) var_t1_4 * stride);
                  break;
                case 1:
                  break;
                case 2:
                case 3:
                case 4:
                case 5:
                  temp_v1_14 = part->code_word;
                  part->records = (u8 *) arena.cur;
                  var_s7_2 = (Records_Unk *) part->records;
                  var_s7 = (Records_Unk **) var_s7_2;
                  stride = 0xC;
                  if (temp_v1_14 != 0)
                  {
                    var_t4 = temp_v1_14 - 1;
                    part->code_word = FIELD_TILE_COLOR_WORDS[var_t4 & 0xFF];
                    var_s6 = FIELD_TILE_REC_SHARED_RGB_CODE;
                    if (var_t4 & 0x200)
                    {
                      ((u8 *) &part->code_word)[3] |= 2;
                    }
                    stride = 8;
                  }
                  else
                  {
                    var_s6 = 0;
                  }
                  temp_v0_12 = part->tpage_word;
                  var_t3 = temp_v0_12 - 1;
                  if (temp_v0_12 != 0)
                  {
                    temp_a0_3 = var_t3 & 0xF;
                    if (temp_a0_3 >= 10)
                    {
                      var_t3 = ((part_def->u.word * 8) & 0x180) | ((var_t3 * 2) & 0x60) | ((u32) (((temp_a0_3 << 6) - 0x80) & 0x3FF) >> 6);
                    }
                    else
                    {
                      var_t3 = ((part_def->u.word * 8) & 0x180) | ((var_t3 * 2) & 0x60) | (((u32) ((temp_a0_3 << 6) + 0x140) >> 6) | new_var4);
                    }
                    part->tpage_word = var_t3;
                    var_s6 |= FIELD_TILE_REC_SHARED_TPAGE;
                    stride -= 4;
                  }
                  var_s3 = (Records_Unk *) part->bits;
                  var_s0 = part_def->u.b.cols * part_def->u.b.rows;
                  var_s0--;
                  var_s1_4 = 0;
                  if (var_s0 != -1)
                  {
                    var_v1_7 = -1;
                    do
                    {
                      if (var_s1_4 == 0)
                      {
                        sp3C = *(u32 *) var_s3;
                        var_s3 = (Records_Unk *) ((u32 *) var_s3 + 1);
                        var_s1_4 = 1;
                      }
                      if (sp3C & var_s1_4)
                      {
                        field_build_quad_tile_record((FieldTileDesc *) var_fp, (FieldTileRec *) var_s7, (part_def->u.word >> 4) & 3, var_s6);
                        var_s7 = (Records_Unk **) ((u8 *) var_s7 + stride);
                        var_t1_4++;
                      }
                      var_s1_4 <<= 1;
                      var_s0--;
                      var_fp += 4;
                    }
                    while (var_s0 != var_v1_7);
                  }
                  arena.cur = (Records_Unk *) ((u8 *) arena.cur + (u16) var_t1_4 * stride);
                  break;
              }
              part->instance_count = (u16) var_t1_4;

            }
          }
          else
          {
            part->records = 0;
            part->instance_count = 0U;
          }
          part = part->next;
        }
        while (part != 0);
      }
      var_t0_3 = var_t0_3->next;
    }
    while (var_t0_3 != 0);
  }
  *((void **) (((u8 *) sp34) + 0x38)) = 0;
  *((s32 *) (((u8 *) sp34) + 0x3C)) = 0;
  field_build_animation_list(map->anim_defs[0], &arena.cur, ((u8 *) sp34) + 0x18);
  field_build_animation_list(map->anim_defs[1], &arena.cur, ((u8 *) sp34) + 0x1C);
  field_build_animation_list(map->anim_defs[2], &arena.cur, ((u8 *) sp34) + 0x20);
  field_build_animation_list(map->anim_defs[3], &arena.cur, ((u8 *) sp34) + 0x24);
  var_s0 = FIELD_RESOURCE->seq_count;
  seq_def = FIELD_RESOURCE->seq_defs;
  tail = (FieldLink *) &sp34->seqs;

  while (--var_s0 != -1)
  {
    seq = (FieldSeq *) arena.cur;
    arena.cur = (Records_Unk *) (seq + 1);
    tail->next = (FieldLink *) seq;
    tail = (FieldLink *) seq;
    seq->def = seq_def++;
    seq->flags &= ~3;
  }
  do { var_s2_2 = 1 << object_index; } while (0);
  tail->next = 0;
  seq = sp34->seqs;
  var_s0 = 0;
  while (seq != 0)
  {
    if (seq->def->unk1 & var_s2_2)
    {
      func_8005A744(seq, var_s0 & 0xFF);
    }
    seq = seq->next;
    var_s0++;
  }
  *((s32 *) (((u8 *) sp34) + 0x34)) = 0;
  if (sp34->unk38 != 0)
  {
    new_var2 = (u8 *) arena.cur;
    sp34->unk38 = (s32) new_var2;
    arena.cur = (Records_Unk *) (new_var2 + 0x14C00);
    DecDCTReset(0);
    DecDCTvlcBuild((u_short *) sp34->unk38);
  }
  mem->top = (u32) arena.cur;
  map->built = 1;
}

/**
 * @brief Prepare tile-animation definitions and their runtime presence masks.
 *
 * Assigns @p handler_group to every definition. For tile handlers in groups
 * zero and three, it verifies that the runtime part's shared TPage and
 * RGB/code words agree with every present source tile, clearing either shared
 * word when the descriptors disagree. It also rasterizes the definition's
 * source rectangle into the runtime part's row-major presence bitmap.
 *
 * @param def Head of the linked animation-definition list.
 * @param handler_group Scene animation-list group, in the range 0 through 3.
 *
 * @see decomp.me (95.80%) https://decomp.me/scratch/Kkiiv
 */
void field_prepare_animation_definitions(FieldAnimRasterDef* def, s32 handler_group)
{
    u32 shared_page_slot = 0;
    u32 shared_color_index = 0;
    u32 shared_semitrans = 0;
    u32 shared_blend_mode = 0;
    s32 tpage_status;
    s32 code_status;
    FieldAnimRasterDef* rec;
    FieldTileGrid* part_def;
    FieldAnimCel* part;
    FieldTileDesc* tile;
    FieldTileDesc* mask_tile;
    s32 frame;
    s32 tile_index;
    u32 mask_bit;
    u32* mask;
    s32 row;
    s32 col;
    u32 mask_word;

    for (; def != NULL; def = def->next)
    {
        def->flags.bytes.handler_group = handler_group;
        if (!(((handler_group == 0) && ((def->flags.word & 7) < 2)) || (handler_group == 3)))
        {
            continue;
        }
        rec = def; /* second pointer to the same record; the original keeps both live */
        part_def = def->part_def;
        part = func_8005ABD8(part_def, NULL);
        if (part->shared != NULL)
        {
            part = part->shared;
        }
        if ((def->flags.word & 7) == 1)
        {
            if ((part_def->u.word & 0xF00) == 0x100)
            {
                def->rect_width = 1;
                def->rect_height = 1;
            }
            else
            {
                def->rect_width = part_def->u.b.cols;
                def->rect_height = part_def->u.b.rows;
            }
        }
        if (part->tpage_word != 0)
        {
            u32 tpage = part->tpage_word - 1;

            tpage_status = 1;
            shared_blend_mode = tpage >> 4;
            shared_page_slot = tpage & 0xF;
        }
        else
        {
            tpage_status = 0;
        }
        if (part->code_word != 0)
        {
            u32 code = part->code_word - 1;

            code_status = 1;
            shared_semitrans = code >> 9;
            shared_color_index = code & 0xFF;
        }
        else
        {
            code_status = 0;
        }
        if ((tpage_status != 0) || (code_status != 0))
        {
            frame = def->flags.bytes.frame_count;
            tile = rec->frame_tiles;
            while (--frame != -1)
            {
                tile_index = rec->rect_width * rec->rect_height;
                while (--tile_index != -1)
                {
                    if (tile->clut_slot & 0x80)
                    {
                        if (tpage_status == 1)
                        {
                            u8 texture_attrs = tile->texture_attrs;

                            if ((shared_page_slot != (texture_attrs & 0xF)) || (shared_blend_mode != ((texture_attrs >> 4) & 3)))
                            {
                                tpage_status = 2;
                            }
                        }
                        if ((code_status == 1) && ((shared_color_index != tile->color_index) || (shared_semitrans != ((tile->texture_attrs >> 6) & 1))))
                        {
                            code_status = 2;
                        }
                    }
                    tile++;
                }
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
        if (((handler_group == 0) && ((def->flags.word & 7) == 0)) || (handler_group == 3))
        {
            frame = def->flags.bytes.frame_count;
            tile = rec->frame_tiles;
            while (--frame != -1)
            {
                mask_tile = tile;
                mask_bit = 1;
                mask = part->mask;
                mask_word = *mask;
                for (row = 0; row != part_def->u.b.rows; row++)
                {
                    if (row < rec->rect_y)
                    {
                        col = part_def->u.b.cols;
                        while (--col != -1)
                        {
                            mask_bit <<= 1;
                            if (mask_bit == 0)
                            {
                                *mask++ = mask_word;
                                mask_bit = 1;
                                mask_word = *mask;
                            }
                        }
                    }
                    else if (row < rec->rect_y + rec->rect_height)
                    {
                        for (col = 0; col != part_def->u.b.cols; col++)
                        {
                            if ((col >= rec->rect_x) && (col < rec->rect_x + rec->rect_width))
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
                                *mask++ = mask_word;
                                mask_bit = 1;
                                mask_word = *mask;
                            }
                        }
                    }
                    else
                    {
                        break;
                    }
                }
                if (mask_bit != 1)
                {
                    *mask = mask_word;
                }
                tile += rec->rect_width * rec->rect_height;
            }
        }
    }
}

FieldTintSrc *field_find_object_by_definition(void *definition);
void func_8005AC50(void *colors, u16 color_count, s32 *rgb_scale);
void func_8005AD20(u8 format, u16 color_count, s8 *primitive_code);
void field_build_sprite_tile_record(FieldTileDesc *, FieldTileRec *, s32, s32);
void field_build_quad_tile_record(FieldTileDesc *, FieldTileRec *, s32, s32);

/**
 * @brief Build the scene's animation node list from a definition chain.
 *
 * Walks @p def 's chain and, for each definition, bump-allocates a 0x30-byte
 * FieldAnim out of the arena at @p arena and tail-appends it to the list at
 * @p tail. Each node is seeded from its definition: the play-mode flags at
 * FieldAnim::flags, the starting keyframe cursor, the loop counter, and the
 * keyframe length from field_find_count_table_span. The handler kind - the low three bits of
 * the word at FieldAnimDef::flags, qualified by
 * FieldAnimDef::handler_group - then selects how the node's cel list is
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
 * @see decomp.me (100%) TODO
 */
void field_build_animation_list(FieldAnimDef *def, u8 **arena, FieldAnim **tail)
{
    s32 rgb[3];
    u8 range_start;
    FieldTintSrc *tint_src;
    s8 primitive_code;
    FieldScene *scene;
    FieldTileGrid *grid;
    u16 stagger_timer;
    s32 record_stride;
    u16 tile_count;
    FieldAnim *anim;
    FieldAnimDef *rec;
    FieldAnimCel *cel;
    FieldSfxKey *key;
    FieldTweenSpan *span;
    u8 *arena_cursor;
    u8 *tile_record;
    u16 *palette_data;
    FieldTileDesc *tile_data;
    FieldTileDesc *frame_descs;
    u32 *mask;
    u32 mask_word;
    u32 mask_bit;
    s32 handler_kind;
    u32 control_flags;
    u32 def_flags;
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
    scene = g_field_scene.scene;
    if (def != NULL)
    {
        do
        {
            anim = (FieldAnim *) *arena;
            *arena = (u8 *) &anim->upload;
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
            def_flags = *(u32 *) &def->flags;
            anim->repeat_count = 0;
            control_flags = (anim->flags.word & ~FIELD_ANIM_FLAG_PING_PONG) | ((def_flags >> 3) & 1);
            control_flags &= ~FIELD_ANIM_FLAG_STOP_AT_KEYFRAME;
            control_flags &= ~FIELD_ANIM_FLAG_REVERSE;
            control_flags &= ~FIELD_ANIM_FLAG_START_PENDING;
            control_flags &= ~FIELD_ANIM_FLAG_SECOND_BUFFER;
            control_flags &= ~FIELD_ANIM_FLAG_UPLOAD_PENDING;
            anim->flags.word = control_flags;
            anim->flags.b.stop_keyframe = 0;
            if (*(s32 *) &def->flags & 0x40)
            {
                anim->flags.b.state = def->unk1;
                anim->flags.b.keyframe = 0;
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
                span = (FieldTweenSpan *) field_find_count_table_span((u8 *) def, anim->flags.b.keyframe, &range_start);
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
                rec = def; /* second pointer to the definition; the original keeps both live */
                switch (rec->flags & 7)
                {
                case 0:
                case 1:
                    grid = ((FieldTileAnimDef *) rec)->grid;
                    cel = (FieldAnimCel *) func_8005ABD8(grid, &tint_src);
                    anim->cels = cel;
                    break;
                case 2:
                    grid = ((FieldTileAnimDef *) rec)->grid;
                    cel = (FieldAnimCel *) func_8005ABD8(grid, &tint_src);
                    anim->cels = cel;
                    if (anim->flags.word & 0x40)
                    {
                        range_start = 0;
                        frame = def->unk5 + 1;
                        while (--frame != -1)
                        {
                            cel->active = range_start == anim->flags.b.state;
                            cel = cel->next;
                            range_start += 1;
                        }
                    }
                    break;
                case 3:
                    if ((anim->flags.word & 0x40) && (anim->timer != 1))
                    {
                        anim->flags.word |= 0x20;
                    }
                    break;
                case 4:
                    grid = ((FieldTileAnimDef *) rec)->grid;
                    cel = (FieldAnimCel *) func_8005ABD8(grid, &tint_src);
                    anim->cels = cel;
                    scene->unk38 = 1;
                    break;
                case 5:
                    grid = ((FieldTileAnimDef *) rec)->grid;
                    cel = (FieldAnimCel *) func_8005ABD8(grid, &tint_src);
                    anim->cels = cel;
                    field_apply_animation_tween(def, anim, 0);
                    break;
                case 6:
                    tint_src = (FieldTintSrc *) field_find_object_by_definition(((FieldTileAnimDef *) rec)->grid);
                    anim->cels = (FieldAnimCel *) tint_src;
                    field_apply_animation_tween(def, anim, 0);
                    break;
                case 7:
                default:
                    grid = ((FieldTileAnimDef *) rec)->grid;
                    cel = (FieldAnimCel *) func_8005ABD8(grid, &tint_src);
                    anim->cels = cel;
                    anim->unk10 = (s32) tint_src;
                    key = (FieldSfxKey *) rec->data;
                    if (((key->control.b.lo & 7) == 1) && (key->sound.word & 0x8000))
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
                    grid = (FieldTileGrid *) def->data;
                    cel = (FieldAnimCel *) func_8005ABD8(grid, &tint_src);
                    anim->cels = cel;
                    break;
                case 1:
                    tint_src = (FieldTintSrc *) field_find_object_by_definition(def->data);
                    anim->cels = (FieldAnimCel *) tint_src;
                    break;
                }
                break;
            case 2:
                switch (def->flags & 7)
                {
                case 0:
                    grid = ((FieldTileAnimDef *) def)->grid;
                    cel = (FieldAnimCel *) func_8005ABD8(grid, &tint_src);
                    anim->cels = cel;
                    anim->unk10 = (s32) tint_src;
                    break;
                case 1:
                    tint_src = (FieldTintSrc *) field_find_object_by_definition(((FieldTileAnimDef *) def)->grid);
                    anim->cels = (FieldAnimCel *) tint_src;
                    break;
                }
                break;
            default:
                grid = ((FieldTileAnimDef *) def)->grid;
                cel = (FieldAnimCel *) func_8005ABD8(grid, &tint_src);
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
                frame_descs = (FieldTileDesc *) def->data;
                rec = def;
                if ((*(u32 *) &def->flags & 0xFF000007) == 1)
                {
                    anim->unk10 = (s32) cel->tiles;
                    frame = def->unk6;
                    while (--frame != -1)
                    {
                        /* no per-frame records for this kind; the original still runs the loop */
                    }
                }
                else
                {
                    frame = def->unk6;
                    while (--frame != -1)
                    {
                            tile_count = 0;
                            tile_data = frame_descs;
                            mask_bit = 1;
                            mask = cel->mask;
                            mask_word = *mask++;
                            for (row = 0; row != grid->u.b.rows; row += 1)
                            {
                                    if (row < rec->unkD)
                                    {
                                        col = grid->u.b.cols;
                                        while (--col != -1)
                                        {
                                            mask_bit *= 2;
                                            if (mask_bit == 0)
                                            {
                                                mask_word = *mask++;
                                                mask_bit = 1;
                                            }
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
                                                            field_build_sprite_tile_record(tile_data, (FieldTileRec *) tile_record,
                                                                                           (grid->u.word >> 4) & 3, record_flags);
                                                            arena_cursor += record_stride;
                                                            break;
                                                        case 2:
                                                        case 3:
                                                        case 4:
                                                        case 5:
                                                            tile_record = arena_cursor;
                                                            field_build_quad_tile_record(tile_data, (FieldTileRec *) tile_record,
                                                                                           (grid->u.word >> 4) & 3, record_flags);
                                                            arena_cursor += record_stride;
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
                            }
                            frame_descs += rec->unkE * rec->unkF;
                    }
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
                *arena = arena_cursor;
            }
            if ((((*(u32 *) &def->flags & 0xFF000007) >= 3) && ((*(u32 *) &def->flags & 0xFF000007) < 5)) ||
                ((def->handler_group == 1) && ((u32) (def->flags & 7) >= 2)))
            {
                if ((*(u32 *) &def->flags & 0xFF000007) == 0x01000002)
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
                else if ((*(u32 *) &def->flags & 0xFF000007) == 0x01000005)
                {
                    if (def->unkC == 0)
                    {
                        *arena += (def->unk10 << 6) + 0x10;
                    }
                    else
                    {
                        *arena += (def->unk10 << 10) + 0x10;
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

/**
 * @brief Build a compact sprite record from a packed field tile descriptor.
 *
 * Decodes the tile's UV and CLUT coordinates, copies its RGB/primitive-code
 * word when it is not shared, and emits its PSX draw-mode command. An absent
 * tile is represented by setting the record's first word to -1.
 *
 * @param desc Packed four-byte tile descriptor.
 * @param record Destination sprite record.
 * @param texture_depth PSX texture depth: 0 = 4bpp, 1 = 8bpp, 2 = 15bpp.
 * @param record_flags Combination of FIELD_TILE_REC_SHARED_RGB_CODE and
 *                     FIELD_TILE_REC_SHARED_TPAGE.
 *
 * @see decomp.me (100%) TODO
 */
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

/**
 * @brief Draw every visible part of every active field object.
 *
 * Walks the scene's object list; for each active object it derives a scroll
 * offset from the camera state (scaled per-axis by the object's definition,
 * optionally negated and wrapped to a power-of-two boundary), applies the
 * object's per-frame drift, then walks the object's part list and submits each
 * visible part to field_draw_part. Parts near a wrap boundary are submitted more
 * than once so they appear on both sides of the seam.
 *
 * @param cursor_ptr Address of the primitive-buffer cursor, forwarded to field_draw_part and
 *             field_draw_marker_overlay.
 * @param ot_base Ordering-table base, forwarded unchanged as the 4th arg of
 *             field_draw_part and 2nd of field_draw_marker_overlay.
 * @param update_mode Mode selector: 0 advances the per-frame drift; 2 forces the
 *             unscaled camera offsets.
 *
 * @see decomp.me (100%) 
 */
void field_draw_scene_objects(u8** cursor, u_long* ot, s32 update_mode)
{
    FieldViewport viewport;
    FieldObj* obj;
    FieldScene* scene;
    FieldObjDef* def;
    FieldPart* part;
    s32 scroll_x;
    s32 scroll_y;
    s32 scroll_z;
    s32 wrap_size;
    s32 wrap_height;
    s32 scroll_diff;

    wrap_size = 0;
    wrap_height = 0;
    scene = g_field_scene.scene;
    viewport.width = scene->header->unk30;
    {
        s32 t = g_field_camera_x;
        s32 q;
        s32 cam_y;
        s32 yq;
        s32 cam_z;

        if (t >= 0)
        {
            q = t >> 8;
        }
        else
        {
            q = (t + 0xFF) >> 8;
        }
        cam_y = g_field_camera_y;
        viewport.camera_x = q;
        if (cam_y < 0)
        {
            cam_y += 0xFF;
        }
        do
        {
            yq = cam_y >> 8;
        } while (0);
        cam_z = g_field_camera_z;
        if (cam_z < 0)
        {
            cam_z += 0x1FF;
        }
        viewport.camera_y = (yq - (cam_z >> 9)) + 0xE0;
    }
    obj = scene->objects;
    if (obj != 0)
    {
        do
        {
            if (obj->flags.word & 1)
            {
                def = obj->def;
                scroll_x = 0;
                if (def->flags & 2)
                {
                    scroll_y = 0;
                    scroll_z = 0;
                }
                else
                {
                    u8 sx = def->scroll_scale_x;

                    if ((sx == 0x10) || (update_mode == 2))
                    {
                        scroll_x = g_field_camera_x;
                    }
                    else
                    {
                        if (sx & 0x80)
                        {
                            scroll_x = (-g_field_camera_x * (sx & 0x7F)) / 16;
                        }
                        else
                        {
                            scroll_x = (g_field_camera_x * def->scroll_scale_x) / 16;
                        }
                    }
                    {
                        u8 sy = def->scroll_scale_y;

                        if ((sy == 0x10) || (update_mode == 2))
                        {
                            scroll_y = SCENE_STATE->camera_y;
                            scroll_z = SCENE_STATE->camera_z;
                        }
                        else if (sy & 0x80)
                        {
                            scroll_y = SHIFT_TOWARD_ZERO(-g_field_camera_y * (sy & 0x7F), 4);
                            scroll_z = (-g_field_camera_z * (def->scroll_scale_y & 0x7F)) / 16;
                        }
                        else
                        {
                            scroll_y = SHIFT_TOWARD_ZERO(g_field_camera_y * def->scroll_scale_y, 4);
                            scroll_z = (g_field_camera_z * def->scroll_scale_y) / 16;
                        }
                    }
                }
                if (obj->flags.b.drift_speed != 0)
                {
                    if (update_mode == 0)
                    {
                        obj->drift_x += (rcos(obj->flags.b.drift_angle * 0x10) * obj->flags.b.drift_speed) / 256;
                        obj->drift_y -= (rsin(obj->flags.b.drift_angle * 0x10) * obj->flags.b.drift_speed) / 256;
                    }
                    if (def->flags & 4)
                    {
                        s32 t;

                        wrap_size = 0x10000 << ((def->flags >> 4) & 3);
                        t = obj->drift_x;
                        if (t >= 0)
                        {
                            obj->drift_x = t & (wrap_size - 1);
                        }
                        else
                        {
                            obj->drift_x = -(-t & (wrap_size - 1));
                        }
                    }
                    if (def->flags & 8)
                    {
                        s32 t;

                        wrap_size = 0x20000 << ((def->flags >> 6) & 3);
                        t = obj->drift_y;
                        if (t >= 0)
                        {
                            obj->drift_y = t & (wrap_size - 1);
                        }
                        else
                        {
                            obj->drift_y = -(-t & (wrap_size - 1));
                        }
                    }
                }
                {
                    s32 px;
                    s32 py;
                    s32 pz;
                    scroll_x += obj->drift_x;
                    scroll_z += obj->drift_y;
                    if (scroll_x >= 0)
                    {
                        px = scroll_x >> 8;
                    }
                    else
                    {
                        px = (scroll_x + 0xFF) >> 8;
                    }
                    scroll_x = px;
                    if (scroll_y >= 0)
                    {
                        py = scroll_y >> 8;
                    }
                    else
                    {
                        py = (scroll_y + 0xFF) >> 8;
                    }
                    if (scroll_z >= 0)
                    {
                        pz = scroll_z >> 9;
                        scroll_diff = py - pz;
                    }
                    else
                    {
                        pz = (scroll_z + 0x1FF) >> 9;
                        scroll_diff = py - pz;
                    }
                }
                part = obj->parts;
                scroll_z = scroll_diff;
                if (part != 0)
                {
                    do
                    {
                        if ((part->visible != 0) && (part->instance_count != 0))
                        {
                            viewport.x = scroll_x + (obj->x + part->x) / 256;
                            {
                                s32 yq;
                                s32 a;
                                FieldPartDef* part_def;
                                s32 rows;
                                s32 z;
                                s32 mid;
                                do
                                {
                                    yq = (obj->y + part->y) / 256;
                                } while (0);
                                a = scroll_z + yq;
                                part_def = part->def;
                                rows = *(volatile u8*)&part_def->u.b.rows;
                                z = obj->z + part->z;
                                rows *= 0x10;
                                do
                                {
                                    mid = a - z / 512;
                                } while (0);
                                rows -= 0xE0;
                                viewport.y = mid - rows;
                            }
                            if (def->flags & 4)
                            {
                                s32 vx;
                                wrap_size = 0x100 << ((def->flags >> 4) & 3);
                                vx = *(volatile s32*)&viewport.x;
                                if (vx >= 0)
                                {
                                    viewport.x = vx & (wrap_size - 1);
                                }
                                else
                                {
                                    viewport.x = wrap_size - (-vx & (wrap_size - 1));
                                }
                            }
                            if (def->flags & 8)
                            {
                                wrap_height = 0x100 << ((def->flags >> 6) & 3);
                                if (viewport.y >= 0)
                                {
                                    viewport.y = viewport.y & (wrap_height - 1);
                                }
                                else
                                {
                                    viewport.y = wrap_height - (-viewport.y & (wrap_height - 1));
                                }
                            }
                            field_draw_part(part, cursor, &viewport, ot);
                            if (def->flags & 4)
                            {
                                if (viewport.x > 0)
                                {
                                    viewport.x -= wrap_size;
                                    field_draw_part(part, cursor, &viewport, ot);
                                    viewport.x += wrap_size;
                                }
                                if (!(def->flags & 0x30))
                                {
                                    s32 t = viewport.x + wrap_size;

                                    if (t < 0x140)
                                    {
                                        viewport.x = t;
                                        field_draw_part(part, cursor, &viewport, ot);
                                        viewport.x -= wrap_size;
                                    }
                                }
                            }
                            if (def->flags & 8)
                            {
                                if (viewport.y > 0)
                                {
                                    viewport.y -= wrap_height;
                                    field_draw_part(part, cursor, &viewport, ot);
                                }
                                if (def->flags & 4)
                                {
                                    if (viewport.x > 0)
                                    {
                                        viewport.x -= wrap_size;
                                        field_draw_part(part, cursor, &viewport, ot);
                                        viewport.x += wrap_size;
                                    }
                                    if (!(def->flags & 0x30))
                                    {
                                        s32 t = viewport.x + wrap_size;

                                        if (t < 0x140)
                                        {
                                            viewport.x = t;
                                            field_draw_part(part, cursor, &viewport, ot);
                                        }
                                    }
                                }
                            }
                        }
                        part = part->next;
                    } while (part != 0);
                }
            }
            obj = obj->next;
        } while (obj != 0);
    }
    if (g_field_marker_overlay_enabled[0] != 0)
    {
        field_draw_marker_overlay(cursor, ot);
    }
}

/**
 * @brief Draw the field's marker overlay: an outline and a numeric label per
 *        marker.
 *
 * Walks the scene's marker list (FieldScene offset 0x10). Each marker emits a
 * red LINE_F4 quad through its def's two points and its own two points, then a
 * red LINE_F2 closing the first point back to the third, then a numeric label
 * (func_800AD208) drawn at the first point with one digit below 10 and two
 * otherwise. All points are shifted by the camera scroll, and every vertical
 * coordinate is halved toward zero. The whole run is chained onto the ordering
 * table entry at @p ot[-1], ahead of whatever the cursor already pointed at.
 *
 * @param cursor Packet-buffer cursor; read for the first primitive address and
 *               written back with the address one past the last primitive.
 * @param ot     Ordering table pointer; the run is linked into @p ot[-1].
 *
 * @see decomp.me (100%) TODO
 */
void field_draw_marker_overlay(u8** cursor, u_long* ot)
{
    s16 pos[2];
    FieldScene* scene;
    FieldMarker* marker;
    FieldMarkerDef* def;
    LINE_F4* prim;
    void* prev;
    s32 sx;
    s32 sy;
    s32 cam_y;
    s32 base_y;
    s32 depth;
    s32 value;
    s32 digits;
    u_long* ot_entry;

    prim = (LINE_F4*)*cursor;
    prev = NULL;
    scene = g_field_scene.scene;
    sx = g_field_camera_x / 256;
    do
    {
        cam_y = g_field_camera_y / 256;
    } while (0);
    do
    {
        sy = cam_y - g_field_camera_z / 512;
    } while (0);
    marker = scene->markers;
    if (marker != NULL)
    {
        ot_entry = ot - 1;
        do
        {
            def = marker->def;
            depth = def->depth_bias + 0xE0;
            setLineF4(prim);
            setRGB0(prim, 0xFF, 0, 0);
            base_y = sy + depth;
            setXY4(prim,
                   def->x0 + sx, base_y - HALF_TOWARD_ZERO((s16)def->y0),
                   def->x1 + sx, base_y - HALF_TOWARD_ZERO((s16)def->y1),
                   marker->x3 + sx, base_y - HALF_TOWARD_ZERO((s16)marker->y3),
                   marker->x2 + sx, base_y - HALF_TOWARD_ZERO((s16)marker->y2));
            if (prev != NULL)
            {
                setaddr(prev, prim);
            }
            prev = prim;
            prim = prim + 1;
            setLineF2((LINE_F2*)prim);
            setRGB0(prim, 0xFF, 0, 0);
            setXY0(prim, def->x0 + sx, base_y - HALF_TOWARD_ZERO((s16)def->y0));
            prim->x1 = marker->x2 + sx;
            prim->y1 = base_y - HALF_TOWARD_ZERO((s16)marker->y2);
            setaddr(prev, prim);
            prev = prim;
            prim = (LINE_F4*)((LINE_F2*)prim + 1);
            pos[0] = def->x0 + sx;
            pos[1] = base_y - HALF_TOWARD_ZERO((s16)def->y0);
            value = def->label;
            digits = 2;
            if (def->label < 0xA)
            {
                digits = 1;
            }
            prim = (LINE_F4*)func_800AD208(ot_entry, prim, value, digits, pos);
            marker = marker->next;
        } while (marker != NULL);
    }
    if (prev != NULL)
    {
        addPrims(&ot[-1], (void*)*cursor, prev);
        *cursor = (u8*)prim;
    }
}

/**
 * @brief Emit GPU primitives for one bit-plane driven 16x16 sprite grid.
 *
 * Walks @p part 's bit plane row-major. Each set bit consumes one record from
 * the part's record stream and emits a 16-byte len-3 primitive at the current
 * grid cell, preceded by an 8-byte len-1 texture-page primitive whenever the
 * page code changes. Emitted primitives accumulate into a local chain that is
 * spliced into @p ot_base 's ordering-table head for the current CLUT with
 * addPrims, both whenever the interpolated CLUT changes and once at the end.
 * Rows and columns outside the 320x224 viewport are skipped by consuming their
 * bits without emitting.
 *
 * Sibling of field_emit_rotated_sprite_grid; both are reached from the field_draw_part dispatch
 * on FieldPart::kind.
 *
 * @param part Field part supplying the grid, the bit plane, the record stream
 *             and the four corner CLUT ids.
 * @param cursor_ptr In/out primitive-buffer cursor; advanced past everything
 *                   emitted.
 * @param origin Screen-space origin of the grid, in pixels.
 * @param ot_base Base of the 8-byte-per-entry ordering-table head array,
 *                indexed by CLUT id.
 *
 * @note `step` is the record stride: 0xC, less 4 when a global code word makes
 *       the per-record copy unnecessary, less another 4 for a global page word.
 *
 * @see decomp.me (100%) TODO
 */
void field_emit_sprite_grid(FieldPart* part, u8** cursor_ptr, FieldViewport* origin, u_long* ot)
{
    s32 uv_word;
    s32 tpage_word;
    s32 code_word;
    s32 height;
    s32 interp;
    u32 clut_right;
    s32 clut_cur;
    s32 clut_left;
    s32 last_code;
    s32 row;
    s32 col;
    s32 idx;
    s32 x;
    s32 y;
    s32 step;
    s32 bits;
    s32 bit;
    s32 count;
    u32 clut;
    u32 clut_b;
    s16 val_a;
    s16 val_b;
    FieldPrim* prim;
    u8* cursor;
    u8* chain;
    u8* recp;
    s32* bitp;
    s32 width;
    FieldPartDef* info;

    last_code = 0;
    bits = 0;
    clut_cur = part->clut_tl;
    clut_left = 0;
    clut_right = 0;
    if ((clut_cur == part->clut_tr) && (clut_cur == part->clut_bl) && (clut_cur == part->clut_br))
    {
        interp = 0;
    }
    else
    {
        clut_cur = 0xFFFF;
        interp = 1;
    }
    code_word = part->code_word;
    tpage_word = part->tpage_word;
    step = 0xC;
    if (code_word != 0)
    {
        step = 8;
    }
    if (tpage_word != 0)
    {
        step -= 4;
    }
    bit = 0;
    prim = NULL;
    bitp = part->bits;
    recp = part->records;
    info = part->def;
    cursor = *cursor_ptr;
    y = origin->y;
    height = info->u.b.rows;
    row = height;
    width = info->u.b.cols;
    chain = NULL;
    while (--row != -1)
    {
        if (y >= 0xE0)
        {
            break;
        }
        if (y < -0xF)
        {
            count = 0;
            do
            {
                for (col = width - 1; col != -1; col--)
                {
                    if (bit == 0)
                    {
                        bits = *bitp++;
                        bit = 1;
                    }
                    if ((bits & bit) != 0)
                    {
                        count++;
                    }
                    bit <<= 1;
                }
                y += 0x10;
            } while ((y < -0xF) && (--row != -1));
            recp += step * count;
            if (row <= 0)
            {
                break;
            }
            row--;
        }
        if (interp != 0)
        {
            val_a = part->clut_tl;
            val_b = part->clut_bl;
            if (val_a != val_b)
            {
                clut_left = ((val_a * (row + 1)) + (val_b * ((height - row) - 1))) / height;
            }
            else
            {
                clut_left = val_a;
            }
            val_a = part->clut_tr;
            val_b = part->clut_br;
            if (val_a != val_b)
            {
                clut_right = ((val_a * (row + 1)) + (val_b * ((height - row) - 1))) / height;
            }
            else
            {
                clut_right = val_a;
            }
        }
        x = origin->x;
        col = width;
        while (--col != -1)
        {
            if (x >= 0x140)
            {
                count = 0;
                do
                {
                    if (bit == 0)
                    {
                        bits = *bitp++;
                        bit = 1;
                    }
                    if ((bits & bit) != 0)
                    {
                        count++;
                    }
                    col--;
                    bit <<= 1;
                } while (col != -1);
                recp += step * count;
                break;
            }
            if (x < -0xF)
            {
                count = 0;
                do
                {
                    if (bit == 0)
                    {
                        bits = *bitp++;
                        bit = 1;
                    }
                    if ((bits & bit) != 0)
                    {
                        count++;
                    }
                    x += 0x10;
                    bit <<= 1;
                } while ((x < -0xF) && (--col != -1));
                recp += step * count;
                if (col <= 0)
                {
                    break;
                }
                col--;
            }
            if (bit == 0)
            {
                bits = *bitp++;
                bit = 1;
            }
            if ((bits & bit) != 0)
            {
                uv_word = ((FieldCellRec*)recp)->uv_clut;
                if (uv_word != -1)
                {
                    if (interp != 0)
                    {
                        idx = width;
                        idx -= col;
                        if (clut_left != clut_right)
                        {
                            clut = ((clut_left * (col + 1)) + (clut_right * (idx - 1))) / width;
                            clut_b = ((clut_left * col) + (clut_right * idx)) / width;
                            if (clut < clut_b)
                            {
                                clut = clut_b;
                            }
                        }
                        else
                        {
                            clut = clut_left;
                        }
                        if (clut != clut_cur)
                        {
                            if (chain != NULL)
                            {
                                addPrims(&ot[clut_cur * 2], chain, prim);
                                chain = NULL;
                            }
                            clut_cur = clut;
                        }
                    }
                    if (tpage_word != 0)
                    {
                        prim = (FieldPrim*)cursor;
                        if (chain == NULL)
                        {
                            chain = cursor;
                            cursor += 8;
                            prim->tag = ((u32)cursor & 0xFFFFFF) | 0x01000000;
                            prim->code = tpage_word;
                            prim = (FieldPrim*)cursor;
                        }
                    }
                    else
                    {
                        prim = (FieldPrim*)cursor;
                        if (chain == NULL)
                        {
                            chain = cursor;
                            cursor += 8;
                            prim->tag = ((u32)cursor & 0xFFFFFF) | 0x01000000;
                            if (code_word != 0)
                            {
                                last_code = (prim->code = ((FieldCellRec*)recp)->rgb_code);
                            }
                            else
                            {
                                last_code = (prim->code = ((FieldCellRec*)recp)->tpage);
                            }
                        }
                        else if (last_code != ((FieldCellRec*)recp)->tpage)
                        {
                            cursor += 8;
                            prim->tag = ((u32)cursor & 0xFFFFFF) | 0x01000000;
                            if (code_word != 0)
                            {
                                last_code = (prim->code = ((FieldCellRec*)recp)->rgb_code);
                            }
                            else
                            {
                                last_code = (prim->code = ((FieldCellRec*)recp)->tpage);
                            }
                        }
                        prim = (FieldPrim*)cursor;
                    }
                    cursor += 0x10;
                    prim->tag = ((u32)cursor & 0xFFFFFF) | 0x03000000;
                    if (code_word != 0)
                    {
                        prim->code = code_word;
                    }
                    else
                    {
                        prim->code = ((FieldCellRec*)recp)->rgb_code;
                    }
                    prim->xy = (x & 0xFFFF) | (y << 16);
                    prim->uv = uv_word;
                }
                recp += step;
            }
            bit <<= 1;
            x += 0x10;
        }
        y += 0x10;
    }
    if (chain != NULL)
    {
        addPrims(&ot[clut_cur * 2], chain, prim);
    }
    *cursor_ptr = cursor;
}

/**
 * @brief Emit rotated, scaled POLY_FT4 primitives for one bit-plane sprite grid.
 *
 * Rotated sibling of field_emit_sprite_grid, reached from the same field_draw_part
 * dispatch on FieldPart::kind. Walks @p part 's bit plane row-major and emits
 * one 40-byte POLY_FT4 per set bit, taking the quad's four corners from two
 * ping-pong row buffers of pre-rotated points.
 *
 * The rotation is precomputed in the PSX scratchpad: 0x1F800000 holds width + 1
 * FieldColStep entries (one per column edge), and 0x1F800200 / 0x1F800300 hold
 * width + 1 points each for the previous and current row. Each row advances the
 * vertical offset by 16, rebuilds the current row's points, then walks the
 * columns emitting a quad per set bit. Cells whose four corners all fall off one
 * side of the 320x224 viewport are skipped. Primitives accumulate into a local
 * chain spliced into @p ot_base 's ordering-table head for the current CLUT with
 * addPrims, both when the interpolated CLUT changes and once at the end.
 *
 * @param part Field part: def gives the grid size and the placement mode, bits
 *             the bit plane, records the record stream, row_angle/column_angle/rotation_angle the rotation
 *             angles, scale_x/scale_y the scales, unk44..4A the four corner CLUT ids.
 * @param cursor_ptr In/out primitive-buffer cursor; advanced past everything
 *                   emitted.
 * @param origin Screen-space placement; the mode selects which of its words
 *               form the rotation centre.
 * @param ot_base Base of the 8-byte-per-entry ordering-table head array,
 *                indexed by CLUT id.
 *
 * @see decomp.me (100%) TODO
 */
void field_emit_rotated_sprite_grid(FieldPart *part, u8 **cursor_ptr, FieldViewport *origin, u_long *ot)
{
    u8 *recp;
    s32 *bitp;
    s32 tpage_word;
    s32 code_word;
    s32 bits;
    s32 bit;
    s32 height;
    s32 interp;
    s32 step;
    s32 sin_c;
    s32 cos_c;
    s32 flip;
    s32 clut_cur;
    s32 clut_left;
    u32 clut_right;
    s32 clut_init;
    s32 row;
    s32 col;
    s32 idx;
    s32 width;
    s32 x_off;
    s32 y_off;
    s32 cx;
    s32 cy;
    s32 cos_a;
    s32 cos_b;
    s32 scaled;
    s32 dx;
    s32 dy;
    s32 visible;
    s32 uv_word;
    u32 clut;
    u32 clut_b;
    FieldColStep *steps;
    FieldPoint *pt;
    FieldPoint *prev_row;
    FieldPoint *this_row;
    FieldPolyPrim *prim;
    u8 *cursor;
    u8 *chain;

    bits = 0;
    clut_left = 0;
    clut_init = part->clut_tl;
    clut_right = 0;
    if ((clut_init == part->clut_tr) && (clut_init == part->clut_bl) && (clut_init == part->clut_br))
    {
        clut_cur = clut_init;
        interp = 0;
    }
    else
    {
        clut_cur = 0xFFFF;
        interp = 1;
    }
    step = 0xC;
    code_word = part->code_word;
    tpage_word = part->tpage_word;
    if (code_word != 0)
    {
        step = 8;
    }
    if (tpage_word != 0)
    {
        step -= 4;
    }
    bit = 0;
    bitp = part->bits;
    recp = part->records;
    cursor = *cursor_ptr;
    prim = NULL;
    width = part->def->u.b.cols;
    height = part->def->u.b.rows;
    cos_a = rcos(part->row_angle);
    cos_b = rcos(part->column_angle);
    sin_c = rsin(part->rotation_angle);
    chain = NULL;
    cos_c = rcos(part->rotation_angle);
    switch ((part->def->u.word >> 12) & 0xF)
    {
    case 1:
    case 2:
        cy = origin->camera_y;
        cx = origin->camera_x + (origin->width / 2);
        x_off = origin->x - cx;
        y_off = origin->y - cy;
        break;
    case 3:
        cx = origin->camera_x;
        cy = origin->camera_y;
        x_off = origin->x - cx;
        y_off = origin->y - cy;
        break;
    case 4:
        cx = origin->width + origin->camera_x;
        cy = origin->camera_y;
        x_off = origin->x - cx;
        y_off = origin->y - cy;
        break;
    case 5:
    default:
        x_off = -width * 8;
        y_off = -height * 8;
        cx = origin->x + (width * 8);
        cy = origin->y + (height * 8);
        break;
    }
    scaled = SHIFT_TOWARD_ZERO(SHIFT_TOWARD_ZERO(y_off * part->scale_y, 8) * cos_a, 12);
    steps = (FieldColStep *) 0x1F800000;
    for (col = width; col != -1; col--)
    {
        scaled = SHIFT_TOWARD_ZERO(SHIFT_TOWARD_ZERO(x_off * part->scale_x, 8) * cos_b, 12);
        x_off += 0x10;
        steps->sin_term = scaled * sin_c;
        steps->cos_term = scaled * cos_c;
        steps++;
    }
    steps = (FieldColStep *) 0x1F800000;
    pt = (FieldPoint *) 0x1F800200;
    scaled = SHIFT_TOWARD_ZERO(SHIFT_TOWARD_ZERO(y_off * part->scale_y, 8) * cos_a, 12);
    dx = scaled * sin_c;
    dy = scaled * cos_c;
    for (col = width; col != -1; col--)
    {
        pt->p.x = SHIFT_TOWARD_ZERO(steps->cos_term - dx, 16) + cx;
        pt->p.y = SHIFT_TOWARD_ZERO(steps->sin_term + dy, 16) + cy;
        steps++;
        pt++;
    }
    flip = 0;
    row = height;
    while (--row != -1)
    {
        if (interp != 0)
        {
            if (part->clut_tl != part->clut_bl)
            {
                clut_left = ((part->clut_tl * (row + 1)) + (part->clut_bl * ((height - row) - 1))) / height;
            }
            else
            {
                clut_left = part->clut_tl;
            }
            if (part->clut_tr != part->clut_br)
            {
                clut_right = ((part->clut_tr * (row + 1)) + (part->clut_br * ((height - row) - 1))) / height;
            }
            else
            {
                clut_right = part->clut_tr;
            }
        }
        if (flip == 0)
        {
            prev_row = (FieldPoint *) 0x1F800200;
            this_row = (FieldPoint *) 0x1F800300;
            flip = 1;
        }
        else
        {
            prev_row = (FieldPoint *) 0x1F800300;
            this_row = (FieldPoint *) 0x1F800200;
            flip = 0;
        }
        y_off += 0x10;
        steps = (FieldColStep *) 0x1F800000;
        pt = this_row;
        scaled = SHIFT_TOWARD_ZERO(SHIFT_TOWARD_ZERO(y_off * part->scale_y, 8) * cos_a, 12);
        dx = scaled * sin_c;
        dy = scaled * cos_c;
        for (col = width; col != -1; col--)
        {
            pt->p.x = SHIFT_TOWARD_ZERO(steps->cos_term - dx, 16) + cx;
            pt->p.y = SHIFT_TOWARD_ZERO(steps->sin_term + dy, 16) + cy;
            steps++;
            pt++;
        }
        for (col = width - 1; col != -1; col--)
        {
            if (bit == 0)
            {
                bits = *bitp++;
                bit = 1;
            }
            if ((bits & bit) != 0)
            {
                if (!((prev_row[0].p.x >= 0) || (prev_row[1].p.x >= 0) || (this_row[0].p.x >= 0) || (this_row[1].p.x >= 0)))
                {
                    visible = 0;
                }
                else if (!((prev_row[0].p.y >= 0) || (prev_row[1].p.y >= 0) || (this_row[0].p.y >= 0) || (this_row[1].p.y >= 0)))
                {
                    visible = 0;
                }
                else if (!((prev_row[0].p.x < 0x140) || (prev_row[1].p.x < 0x140) || (this_row[0].p.x < 0x140) || (this_row[1].p.x < 0x140)))
                {
                    visible = 0;
                }
                else if ((prev_row[0].p.y < 0xE0) || (prev_row[1].p.y < 0xE0) || (this_row[0].p.y < 0xE0) || (this_row[1].p.y < 0xE0))
                {
                    visible = 1;
                }
                else
                {
                    visible = 0;
                }
                if (visible != 0)
                {
                    uv_word = ((FieldCellRec *) recp)->uv_clut;
                    if (uv_word != -1)
                    {
                        if (interp != 0)
                        {
                            idx = width - col;
                            if (clut_left != clut_right)
                            {
                                clut = ((clut_left * (col + 1)) + (clut_right * (idx - 1))) / width;
                                clut_b = ((clut_left * col) + (clut_right * idx)) / width;
                                if (clut < clut_b)
                                {
                                    clut = clut_b;
                                }
                            }
                            else
                            {
                                clut = clut_left;
                            }
                            if (clut != clut_cur)
                            {
                                if (chain != NULL)
                                {
                                    addPrims(&ot[clut_cur * 2], chain, prim);
                                    chain = NULL;
                                }
                                clut_cur = clut;
                            }
                        }
                        if (chain == NULL)
                        {
                            prim = (FieldPolyPrim *) cursor;
                            chain = cursor;
                        }
                        else
                        {
                            prim = (FieldPolyPrim *) cursor;
                        }
                        cursor += sizeof(FieldPolyPrim);
                        prim->tag = ((u32) cursor & 0xFFFFFF) | 0x09000000;
                        if (code_word != 0)
                        {
                            prim->code = code_word;
                        }
                        else
                        {
                            prim->code = ((FieldCellRec *) recp)->rgb_code;
                        }
                        if (tpage_word != 0)
                        {
                            prim->uv1 = ((uv_word & 0xFFFF) + 0xF) | tpage_word;
                        }
                        else if (code_word != 0)
                        {
                            prim->uv1 = ((FieldCellRec *) recp)->rgb_code;
                        }
                        else
                        {
                            prim->uv1 = ((FieldCellRec *) recp)->tpage;
                        }
                        prim->uv0 = uv_word;
                        prim->uv2 = (uv_word & 0xFFFF) + 0xF00;
                        prim->uv3 = (uv_word & 0xFFFF) + 0xF0F;
                        prim->xy0 = prev_row[0].word;
                        prim->xy1 = prev_row[1].word;
                        prim->xy2 = this_row[0].word;
                        prim->xy3 = this_row[1].word;
                    }
                }
                recp += step;
            }
            prev_row++;
            this_row++;
            bit <<= 1;
        }
    }
    if (chain != NULL)
    {
        addPrims(&ot[clut_cur * 2], chain, prim);
    }
    *cursor_ptr = cursor;
}

/**
 * @brief Find an already-built part in the scene that this one can share.
 *
 * Scans every part of every object in @p scene for one whose definition key
 * matches @p key and whose owning object is interchangeable with @p obj - either
 * literally the same definition, or one with the same shared-source handle and
 * the same 0x10/0x14 pair. The caller uses the result to reuse an existing
 * part's build instead of doing the work twice.
 *
 * @param scene Scene whose object list is searched.
 * @param obj   Object the candidate must be interchangeable with.
 * @param part  Part being built; excluded from its own search, and skipped
 *              entirely when its definition is marked unshareable (bit 7).
 * @param key   Definition key to match on (FieldPartDef::key).
 * @return The matching FieldPart, or NULL if none qualifies - including when
 *         the only candidate found is @p part itself on @p obj.
 *
 * @see decomp.me (100%) TODO
 */
FieldPart* field_find_shareable_part(FieldScene* scene, FieldObj* obj, FieldPart* part, s32 key)
{
    FieldObj* o;
    FieldPart* p;
    FieldObjDef* want;
    FieldObjDef* have;

    if (!(part->def->u.word & 0x80))
    {
        for (o = scene->objects; o != NULL; o = o->next)
        {
            for (p = o->parts; p != NULL; p = p->next)
            {
                if (key == p->def->key)
                {
                    if ((obj == o) && (part == p))
                    {
                        return NULL;
                    }
                    if (!(p->def->u.word & 0x80))
                    {
                        want = obj->def;
                        have = o->def;
                        if ((want == have) || ((want->shared_source == have->shared_source) && (obj->unk10 == o->unk10) && (obj->unk14 == o->unk14)))
                        {
                            return p;
                        }
                    }
                }
            }
        }
    }
    return NULL;
}

/**
 * @brief Dispatch one field part to the emitter its kind selects.
 *
 * Kind 0 draws an axis-aligned grid, kinds 2 through 5 a rotated/scaled one.
 * Kind 1 and anything from 6 up draw nothing. All four arguments are forwarded
 * verbatim.
 *
 * @param part Field part to draw; its kind byte selects the emitter.
 * @param cursor_ptr Primitive-buffer cursor, forwarded as field_emit_sprite_grid's 2nd param.
 * @param origin Screen-space placement, forwarded as the 3rd param.
 * @param ot_base Ordering-table head array base, forwarded as the 4th param.
 *
 * @see decomp.me (100%) TODO
 */
void field_draw_part(FieldPart* part, u8** cursor, FieldViewport* origin, u_long* ot)
{
    switch (part->kind)
    {
    case 0:
        field_emit_sprite_grid(part, cursor, origin, ot);
        break;
    case 1:
        break;
    case 2:
    case 3:
    case 4:
    case 5:
        field_emit_rotated_sprite_grid(part, cursor, origin, ot);
        break;
    }
}

/**
 * @brief Flush the scene's pending VRAM uploads.
 *
 * Walks the scene's upload list, issues each node's LoadImage, then empties the
 * list. Nodes are not freed - the list head is simply cleared.
 *
 * @see decomp.me (100%) TODO
 */
void field_flush_vram_uploads(void)
{
    FieldScene* scene;
    FieldImageReq* req;

    scene = g_field_scene.scene;
    for (req = scene->uploads; req != NULL; req = req->next)
    {
        LoadImage(&req->rect, req->data);
    }
    scene->uploads = NULL;
}

/**
 * @brief Empty stub; nothing references it.
 *
 * @see decomp.me (100%) TODO
 */
void func_800569F4(void)
{
}

/**
 * @brief Empty stub, identical to func_800569F4; nothing references it.
 *
 * @see decomp.me (100%) TODO
 */
void func_800569FC(void)
{
}
