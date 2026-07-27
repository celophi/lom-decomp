#include "common.h"

struct S1;
struct S2;
struct Block;
struct T;
typedef struct S1
{
    u8 pad_00[4];
    union
    {
        u32 word;
        struct
        {
            u8 pad_04[2];
            u8 field_06;
            u8 field_07;
        } bytes;
    } u;
    struct S1* next;
    u8 field_0C;
    u8 field_0D;
    u8 field_0E;
    u8 field_0F;
    struct S2* ptr_s2;
    struct Block* blocks;
} S1;
typedef struct S2
{
    u8 pad_00[8];
    union
    {
        u32 word;
        struct
        {
            u8 pad_08[2];
            u8 field_0A;
            u8 field_0B;
        } bytes;
    } u;
} S2;
typedef struct Block
{
    u8 b0;
    u8 b1;
    u8 b2;
    u8 b3;
} Block;
typedef struct T
{
    u8 pad_00[8];
    struct T* next;
    s32* data;
    u8 pad_10[8];
    u32 field_18;
    u32 field_1C;
} T;
typedef struct FieldAnimDef FieldAnimDef;
typedef struct FieldAnim FieldAnim;
typedef struct FieldAnimCel FieldAnimCel;

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
} FieldTileGrid;

struct FieldAnimDef
{
    u8 unk0;  /* 0x00 */
    u8 unk1;  /* 0x01 */
    u8 unk2;  /* 0x02 */
    u8 _pad0;
    u8 unk4;  /* 0x04 */
    u8 unk5;  /* 0x05 */
    u8 unk6;  /* 0x06 */
    u8 unk7;  /* 0x07 */
    FieldAnimDef *next; /* 0x08 */
    u8 unkC;  /* 0x0C */
    u8 unkD;  /* 0x0D */
    u8 unkE;  /* 0x0E */
    u8 unkF;  /* 0x0F */
    void *unk10; /* 0x10 */
    s32 *unk14;  /* 0x14 */
};

typedef union
{
    s32 word;
    struct
    {
        u8 unk0;
        u8 state; /* 0x25 */
        u8 unk2;  /* 0x26 */
        u8 unk3;  /* 0x27 */
    } b;
} FieldAnimFlags;

struct FieldAnimCel
{
    FieldAnimCel *next;  /* 0x00 */
    FieldTileGrid *grid; /* 0x04 */
    u8 _pad0[0xC - 8];
    u32 *mask;  /* 0x0C */
    u8 *tiles;  /* 0x10 */
    u8 _pad1[0x18 - 0x14];
    s32 unk18;  /* 0x18 */
    s32 unk1C;  /* 0x1C */
    s8 unk20;   /* 0x20 */
    u8 format;  /* 0x21 */
};

struct FieldAnim
{
    FieldAnim *next;      /* 0x00 */
    FieldAnimDef *def;    /* 0x04 */
    u8 _pad0[0xC - 8];
    FieldAnimCel *cels;   /* 0x0C */
    s32 unk10;            /* 0x10 */
    u8 _pad1[0x20 - 0x14];
    u8 *frames;           /* 0x20 */
    FieldAnimFlags flags; /* 0x24 */
    u8 unk28;             /* 0x28 */
    u8 _pad2;
    u16 counter;          /* 0x2A */
    u16 frame_tiles;      /* 0x2C */
    u8 _pad3[0x30 - 0x2E];
};

typedef struct
{
    u8 _pad0[4];
    u16 *unk4; /* 0x04 */
} FieldTintPal;

typedef struct
{
    u8 _pad0[4];
    FieldTintPal *unk4; /* 0x04 */
    u8 _pad1[0x10 - 8];
    u16 unk10; /* 0x10 */
    u16 unk12; /* 0x12 */
    u16 unk14; /* 0x14 */
} FieldTintSrc;

typedef struct
{
    u8 _pad0;
    u8 unk1;  /* 0x01 */
    u16 unk2; /* 0x02 */
} FieldTweenSpan;

typedef struct
{
    u8 unk0;  /* 0x00 */
    u8 _pad0;
    u16 unk2; /* 0x02 */
} FieldAnimKey;

typedef struct
{
    u8 _pad0[0x38];
    s32 unk38; /* 0x38 */
} FieldScene;

extern FieldScene *g_field_scene;

T *func_8005ABD8(void *, FieldTintSrc **);

/**
 * decomp.me (95.80%) https://decomp.me/scratch/Kkiiv
 */
void field_validate_and_rasterize_quads(void* arg0, s32 arg1)
{
    S1* s1 = (S1*)arg0;
    u32 var_t6 = 0;
    u32 var_s7 = 0;
    u32 var_s8 = 0;
    u32 var_s6 = 0;
    s32 flag_a3;
    s32 flag_a2;
    int new_var;
    S1* s0;
    S2* temp_s2;
    T* var_t4;
    Block* var_t3;
    Block* var_t2;
    s32 var_t1;
    s32 var_a1;
    s32 var_a1_2;
    s32* var_a2;
    s32 var_t0;
    s32 var_a0;
    s32 var_a3_val;
    if (s1 != 0)
    {
        do
        {
            s1->u.bytes.field_07 = arg1;
            if (((arg1 == 0) && ((s1->u.word & 7U) < 2U)) || (arg1 == 3))
            {
                s0 = s1;

                temp_s2 = s1->ptr_s2;
                var_t4 = func_8005ABD8(temp_s2, 0);
                if (var_t4->next != 0)
                {
                    var_t4 = var_t4->next;
                }
                if ((s1->u.word & 7U) == 1)
                {
                    if ((temp_s2->u.word & 0xF00U) == 0x100U)
                    {
                        s1->field_0E = 1;
                        s1->field_0F = 1;
                    }
                    else
                    {
                        s1->field_0E = temp_s2->u.bytes.field_0A;
                        s1->field_0F = temp_s2->u.bytes.field_0B;
                    }
                }
                if (var_t4->field_18 != 0)
                {
                    u32 temp = var_t4->field_18 - 1;
                    flag_a3 = 1;
                    var_s6 = temp >> 4;
                    var_t6 = temp & 0xF;
                }
                else
                {
                    flag_a3 = 0;
                }
                if (var_t4->field_1C != 0)
                {
                    u32 temp = var_t4->field_1C - 1;
                    flag_a2 = 1;
                    var_s8 = temp >> 9;
                    var_s7 = temp & 0xFF;
                }
                else
                {
                    flag_a2 = 0;
                }
                if ((flag_a3 != 0) || (flag_a2 != 0))
                {
                    var_t1 = s1->u.bytes.field_06 - 1;
                    var_t3 = s0->blocks;
                    if (var_t1 != (-1))
                    {
                        do
                        {
                            var_a1 = ((var_t1 = s0->field_0E) * s0->field_0F) - 1;
                            if (var_a1 != (-1))
                            {
                                do
                                {
                                    if (var_t3->b0 & 0x80)
                                    {
                                        if (flag_a3 == 1)
                                        {
                                            u8 b1_val = var_t3->b1;
                                            if ((var_t6 != (b1_val & 0xF)) || (var_s6 != ((b1_val >> 4) & 3)))
                                            {
                                                flag_a3 = 2;
                                            }
                                        }
                                        if (flag_a2 == 1)
                                        {
                                            u8 b3_val = var_t3->b3;
                                            u8 b1_val = var_t3->b1;
                                            if ((var_s7 != b3_val) || (var_s8 != ((b1_val >> 6) & 1)))
                                            {
                                                flag_a2 = 2;
                                            }
                                        }
                                    }
                                    var_t3++;
                                    var_a1--;
                                } while (var_a1 != (-1));
                            }
                            var_t1--;
                        } while (var_t1 != (-1));
                    }
                    if (flag_a3 != 1)
                    {
                        var_t4->field_18 = 0;
                    }
                    if (flag_a2 != 1)
                    {
                        var_t4->field_1C = 0;
                    }
                }

                if (((arg1 == 0) && ((s1->u.word & 7U) == 0)) || (arg1 == 3))
                {
                    var_t1 = s1->u.bytes.field_06 - 1;
                    var_t3 = s0->blocks;
                    if (var_t1 != (-1))
                    {
                        do
                        {
                            var_t2 = var_t3;
                            var_a1_2 = 1;
                            var_a2 = var_t4->data;
                            var_a3_val = *var_a2;
                            if (temp_s2->u.bytes.field_0B != 0)
                            {
                                var_t0 = 0;
                                do
                                {
                                    if (var_t0 < s0->field_0D)
                                    {
                                        new_var = -1;
                                        var_a0 = temp_s2->u.bytes.field_0A - 1;
                                        if (var_a0 != new_var)
                                        {
                                            do
                                            {
                                                var_a1_2 <<= 1;
                                                if (var_a1_2 == 0)
                                                {
                                                    *var_a2 = var_a3_val;
                                                    var_a2++;
                                                    var_a1_2 = 1;
                                                    var_a3_val = *var_a2;
                                                }
                                                var_a0--;
                                            } while (var_a0 != (-1));
                                        }
                                    }
                                    else if (var_t0 < (s0->field_0D + s0->field_0F))
                                    {
                                        if (temp_s2->u.bytes.field_0A != 0)
                                        {
                                            var_a0 = 0;
                                            do
                                            {
                                                if ((var_a0 >= s0->field_0C) &&
                                                    (var_a0 < (s0->field_0C + s0->field_0E)))
                                                {
                                                    if (var_t2->b0 & 0x80)
                                                    {
                                                        var_a3_val |= var_a1_2;
                                                    }
                                                    var_t2++;
                                                }
                                                var_a1_2 <<= 1;
                                                if (var_a1_2 == 0)
                                                {
                                                    *var_a2 = var_a3_val;
                                                    var_a2++;
                                                    var_a1_2 = 1;
                                                    var_a3_val = *var_a2;
                                                }
                                                var_a0++;
                                            } while (var_a0 != temp_s2->u.bytes.field_0A);
                                        }
                                    }
                                    else
                                    {
                                        break;
                                    }
                                    var_t0++;
                                } while (var_t0 != temp_s2->u.bytes.field_0B);
                            }
                            if (var_a1_2 != 1)
                            {
                                *var_a2 = var_a3_val;
                            }
                            var_t1--;
                            var_t3 += s0->field_0E * s0->field_0F;
                        } while (var_t1 != (-1));
                    }
                }
            }
            s1 = s1->next;
        } while (s1 != 0);
    }
}

FieldAnimCel *func_8005B31C(void *);
u8 *func_80059224(FieldAnimDef *, s32, u8 *);
void func_80057E88(FieldAnimDef *, FieldAnim *, s32);
void func_80057CA4(FieldAnimDef *, FieldAnim *, s32);
void func_8005AC50(void *, u16, s32 *);
void func_8005AD20(u8, u16, s8 *);
void func_8005477C(s32 *, u8 *, s32, s32);
void func_80054904(s32 *, u8 *, s32, s32);

/**
 * @brief Build the scene's animation node list from a definition chain.
 *
 * Walks @p def 's chain and, for each definition, bump-allocates a 0x30-byte
 * FieldAnim out of the arena at @p arena and tail-appends it to the list at
 * @p tail. Each node is seeded from its definition: the play-mode flags at
 * FieldAnim::flags, the starting keyframe cursor, the loop counter, and the
 * keyframe length from func_80059224. The handler kind - the low three bits of
 * the word at FieldAnimDef::unk4, qualified by FieldAnimDef::unk7 - then selects
 * how the node's cel list is resolved (func_8005ABD8 or func_8005B31C) and what
 * extra setup runs.
 *
 * For the tinted kinds the definition's colour is expanded into the scratchpad
 * table (func_8005AC50 / func_8005AD20) and the per-frame GPU primitives are
 * built into the arena: every frame walks the cel's bit plane row-major, and
 * each set bit inside the definition's sub-rectangle emits one primitive through
 * func_8005477C or func_80054904 depending on the cel's record format. The arena
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
 *       @c 6: arms to emit the 7-entry jump tables, as in func_800584DC.
 * @note The three @c & @c 7 handler switches read @c def->unk4 as a byte; the
 *       @c & @c 0xFF000007 and @c & @c 0x40 / @c & @c 0x20 tests read the whole
 *       word. Both views of the same field are required.
 * @note @c rec is a local copy of @p def, needed twice - once in the @c unk7
 *       @c == @c 0 arm and once before the record loop. It is what puts the
 *       definition pointer in s4 and is worth 2.8%.
 *
 * @see decomp.me (89.12%) TODO
 */
void func_80053C7C(FieldAnimDef *def, u8 **arena, FieldAnim **tail)
{
    s32 rgb[3];       /* sp10 */
    u8 span_off;      /* sp20 */
    FieldTintSrc *src;/* sp24 */
    s8 code;          /* sp28 */
    FieldScene *scene;/* sp2C */
    FieldTileGrid *grid; /* sp30 */
    u16 delay;        /* sp38 */
    s32 stride;       /* sp40 */
    u16 count;        /* sp48 */
    FieldAnim *anim;
    FieldAnimDef *rec;
    FieldAnimCel *cel;
    FieldAnimKey *key;
    FieldTweenSpan *span;
    u8 *cursor;
    u8 *prim;
    u16 *tab;
    s32 *recp;
    s32 *base;
    u32 *maskp;
    u32 word;
    u32 bit;
    s32 kind;
    s32 flags;
    s32 mode;
    s32 frame;
    s32 row;
    s32 col;
    u8 state;
    u16 dur;
    u16 slot;

    cel = NULL;
    grid = NULL;
    stride = 0;
    count = 0;
    delay = 1;
    src = NULL;
    scene = g_field_scene;
    if (def != NULL)
    {
        do
        {
            anim = (FieldAnim *) *arena;
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
                anim->flags.word = (anim->flags.word & ~0x40) | ((def->unk4 >> 7) << 6);
            }
            anim->unk28 = 0;
            flags = (anim->flags.word & ~1) | ((*(u32 *) &def->unk4 >> 3) & 1);
            flags &= ~2;
            flags &= ~4;
            flags &= ~8;
            flags &= ~0x10;
            flags &= ~0x20;
            anim->flags.word = flags;
            anim->flags.b.unk3 = 0;
            if (*(s32 *) &def->unk4 & 0x40)
            {
                anim->flags.b.unk2 = 0;
                anim->flags.b.state = def->unk1;
            }
            else
            {
                state = def->unk1;
                anim->flags.b.state = state;
                anim->flags.b.unk2 = state;
            }
            if (def->unk7 == 3)
            {
                anim->counter = 1;
            }
            else
            {
                span = (FieldTweenSpan *) func_80059224(def, anim->flags.b.unk2, &span_off);
                if (*(s32 *) &def->unk4 & 0x20)
                {
                    anim->counter = span->unk2;
                }
                else
                {
                    dur = span->unk2;
                    if (dur < delay)
                    {
                        anim->counter = dur;
                        delay = 1;
                    }
                    else
                    {
                        slot = delay;
                        delay = slot + 1;
                        anim->counter = slot;
                    }
                }
            }
            switch (def->unk7)
            {
            case 0:
                rec = def;
                switch (rec->unk4 & 7)
                {
                case 0:
                case 1:
                    grid = (FieldTileGrid *) rec->unk10;
                    cel = (FieldAnimCel *) func_8005ABD8(grid, &src);
                    anim->cels = cel;
                    break;
                case 2:
                    grid = (FieldTileGrid *) rec->unk10;
                    cel = (FieldAnimCel *) func_8005ABD8(grid, &src);
                    anim->cels = cel;
                    if ((anim->flags.word & 0x40) &&
                        (span_off = 0, frame = def->unk5, frame != -1))
                    {
                        do
                        {
                            frame -= 1;
                            cel->unk20 = span_off == anim->flags.b.state;
                            cel = cel->next;
                            span_off += 1;
                        }
                        while (frame != -1);
                    }
                    break;
                case 3:
                    if ((anim->flags.word & 0x40) && (anim->counter != 1))
                    {
                        anim->flags.word |= 0x20;
                    }
                    break;
                case 4:
                    grid = (FieldTileGrid *) rec->unk10;
                    cel = (FieldAnimCel *) func_8005ABD8(grid, &src);
                    anim->cels = cel;
                    scene->unk38 = 1;
                    break;
                case 5:
                    grid = (FieldTileGrid *) rec->unk10;
                    cel = (FieldAnimCel *) func_8005ABD8(grid, &src);
                    anim->cels = cel;
                    func_80057E88(def, anim, 0);
                    break;
                case 6:
                    cel = func_8005B31C(rec->unk10);
                    src = (FieldTintSrc *) cel;
                    anim->cels = cel;
                    func_80057E88(def, anim, 0);
                    break;
                default:
                    grid = (FieldTileGrid *) rec->unk10;
                    cel = (FieldAnimCel *) func_8005ABD8(grid, &src);
                    anim->cels = cel;
                    anim->unk10 = (s32) src;
                    key = (FieldAnimKey *) rec->unk14;
                    if (((key->unk0 & 7) == 1) && (key->unk2 & 0x8000))
                    {
                        anim->counter = 1;
                        anim->flags.word |= 8;
                    }
                    break;
                }
                break;
            case 1:
                switch (def->unk4 & 7)
                {
                case 0:
                    grid = (FieldTileGrid *) def->unk14;
                    cel = (FieldAnimCel *) func_8005ABD8(grid, &src);
                    anim->cels = cel;
                    break;
                case 1:
                    cel = func_8005B31C(def->unk14);
                    src = (FieldTintSrc *) cel;
                    anim->cels = cel;
                    break;
                }
                break;
            case 2:
                switch (def->unk4 & 7)
                {
                case 0:
                    grid = (FieldTileGrid *) def->unk10;
                    cel = (FieldAnimCel *) func_8005ABD8(grid, &src);
                    anim->cels = cel;
                    anim->unk10 = (s32) src;
                    break;
                case 1:
                    cel = func_8005B31C(def->unk10);
                    src = (FieldTintSrc *) cel;
                    anim->cels = cel;
                    break;
                }
                break;
            default:
                grid = (FieldTileGrid *) def->unk10;
                cel = (FieldAnimCel *) func_8005ABD8(grid, &src);
                anim->cels = cel;
                break;
            }
            if (((u32) (*(u32 *) &def->unk4 & 0xFF000007) < 2) || (def->unk7 == 3))
            {
                rgb[0] = src->unk10 << 8;
                rgb[1] = src->unk12 << 8;
                rgb[2] = src->unk14 << 8;
                tab = src->unk4->unk4;
                func_8005AC50(tab + 2, tab[0], rgb);
                code = 0;
                func_8005AD20(cel->format, src->unk4->unk4[0], &code);
                anim->frames = *arena;
                cursor = *arena;
                switch (cel->format)
                {
                case 0:
                    stride = 0xC;
                    break;
                case 2:
                case 3:
                case 4:
                case 5:
                    stride = 0xC;
                    break;
                case 1:
                case 6:
                    break;
                }
                mode = 1;
                if (cel->unk1C != 0)
                {
                    stride -= 4;
                }
                else
                {
                    mode = 0;
                }
                if (cel->unk18 != 0)
                {
                    mode |= 2;
                    stride -= 4;
                }
                base = def->unk14;
                rec = def;
                if ((*(u32 *) &def->unk4 & 0xFF000007) == 1)
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
                            recp = base;
                            bit = 1;
                            row = 0;
                            count = 0;
                            maskp = cel->mask;
                            word = *maskp++;
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
                                                bit *= 2;
                                                if (bit == 0)
                                                {
                                                    word = *maskp++;
                                                    bit = 1;
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
                                                    if (word & bit)
                                                    {
                                                        switch (cel->format)
                                                        {
                                                        case 0:
                                                            prim = cursor;
                                                            cursor += stride;
                                                            func_8005477C(recp, prim, (grid->u.word >> 4) & 3, mode);
                                                            break;
                                                        case 2:
                                                        case 3:
                                                        case 4:
                                                        case 5:
                                                            prim = cursor;
                                                            cursor += stride;
                                                            func_80054904(recp, prim, (grid->u.word >> 4) & 3, mode);
                                                            break;
                                                        case 1:
                                                        case 6:
                                                            break;
                                                        }
                                                        count += 1;
                                                    }
                                                    recp += 1;
                                                }
                                                bit *= 2;
                                                if (bit == 0)
                                                {
                                                    word = *maskp++;
                                                    bit = 1;
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
                            base += rec->unkE * rec->unkF;
                        }
                        while (frame != -1);
                        anim->frame_tiles = count;
                        if (def->unk7 == 3)
                        {
                            if (*(u32 *) &def->unk4 & 0x20)
                            {
                                func_80057CA4(def, anim, 0);
                            }
                        }
                        else if (anim->flags.word & 0x40)
                        {
                            func_80057CA4(def, anim, 0);
                        }
                    }
                }
                *arena = cursor;
            }
            kind = *(u32 *) &def->unk4 & 0xFF000007;
            if (((u32) (kind - 3) < 2) ||
                ((def->unk7 == 1) && ((u32) (def->unk4 & 7) >= 2)))
            {
                if (kind == 0x01000002)
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
                else if (kind == 0x01000005)
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
