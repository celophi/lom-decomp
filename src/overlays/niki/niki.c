#include "common.h"

typedef struct {
    s16 x;
    s16 y;
    s16 w;
    s16 h;
} RECT;

typedef struct NikiElement {
    union {
        u32 word;
        struct {
            u32 state : 3;
            u32 unk0_3 : 4;
            u32 x : 9;
            u32 unk0_16 : 8;
        } f;
    } attr;
    u32 unk4_0 : 1;
    u32 y : 8;
    u32 unk4_9 : 23;
    void *draw_handler;
    s32 unkC;
} NikiElement;

typedef struct NikiRecord {
    u8 pad0[0x17];
    u8 unk17;
    u8 pad18[0xCF - 0x18];
    u8 unkCF;
    u8 padD0[4];
    u16 unkD4;
    u16 unkD6;
} NikiRecord;

typedef struct
{
    u8 data[0x28];
} NikiEntry28;

/**
 * @brief 0xC-stride element of the D_80164B10 array (attr word + handler).
 * @note Same layout as NikiElement minus its trailing unkC, so a NikiPacket*
 *       walks the array at its real 0xC stride while still exposing attr.f.
 */
typedef struct
{
    union
    {
        u32 word;
        struct
        {
            u32 state : 3;
            u32 unk0_3 : 4;
            u32 x : 9;
            u32 unk0_16 : 8;
        } f;
    } attr;
    u32 unk4_0 : 1;
    u32 y : 8;
    u32 unk4_9 : 23;
    void (*draw_handler)();
} NikiPacket;

/**
 * @brief One GPU primitive packet the element draw functions emit into.
 */
typedef struct
{
    s32 unk0;
    s32 unk4;
    s16 unk8;
    s16 unkA;
    s16 unkC;
    u16 unkE;
} NikiGpuPacket;

/**
 * @brief Per-frame draw context passed to func_80141F18.
 * @note unk40B8 is the running GPU-packet write cursor; unk40B2 selects the
 *       clip/window variant.
 */
typedef struct
{
    s32 unk0;
    u8 pad4[0x40AE];
    s16 unk40B2;
    u8 pad40B4[4];
    NikiGpuPacket *unk40B8;
} NikiDrawState;

/** @brief Element draw callback: returns the advanced GPU-packet cursor. */
typedef NikiGpuPacket *(*NikiElemDrawFunc)();

/** @brief Field layout of the POLY_G4 timer-bar packet built by func_80142A1C. */
typedef struct
{
    s32 unk0;
    s32 unk4;
    s16 unk8;
    s16 unkA;
    s32 unkC;
    s16 unk10;
    s16 unk12;
    s32 unk14;
    s16 unk18;
    s16 unk1A;
    s32 unk1C;
    s16 unk20;
    s16 unk22;
} NikiPolyG4Words;

/**
 * @brief Fallback name/second-line text carried alongside the save-slot record.
 * @note Consumed only when the primary slot compare (func_8001714C) fails.
 */
typedef struct NikiFallbackText
{
    u8 pad[0x24];
    u8 text[0x20];
} NikiFallbackText;


extern s32 D_80164A78;
extern s32 D_80164AD0;
extern s32 D_80164AD4;
extern s32 D_80164AE4;
extern s32 D_80164AE8;
extern s32 D_80164B70;
extern s32 D_80164B74;
extern s32 D_80164B78;
extern s32 D_80164B84;
extern s32 D_80164B8C;
extern s32 D_80164B90;
extern s32 D_80164ADC;
extern s32 D_80164B7C;
extern NikiElement D_80164B1C;
extern s32 D_80122988;
extern s32 D_80164B88;
extern s32 D_80164AE0;
extern s32 D_80164AEC;

extern s32 D_80164B80;
extern s32 D_801606C8;
extern s32 D_801606D4;
extern s32 D_801606DC;

extern NikiElement D_80164B10;
extern s32 D_80164F18;
extern s32 D_80164A78;
extern u8 *D_80164E18;
extern s32 D_80122994;
extern s32 D_80164B70;
extern char D_800ECF7C[];
extern NikiEntry28 D_80165018[][20];
extern NikiRecord D_80164D18;
extern NikiRecord *D_8012271C;
extern s32 D_8003EC9C;

extern s32 D_80164AD8;
extern s32 D_80160A70;
extern u8 D_80164DE7;
extern u8 D_80164B98;
extern u8 D_80164B9C;
extern u16 D_80147120;
extern u16 D_80147146;
extern u16 D_80147148;
extern u16 D_8014714C;
extern u16 D_801475C4[];
extern u8 D_800EC3F6[2];
extern u8 D_800EC3FA[];
extern u8 D_800EC3D0[];
extern s32 D_8012298C;
extern u16 D_80147128;
extern s32 D_80164A7C;
extern s32 D_801606E4;
extern u16 D_8014712A;
extern u8 D_80160A78[];
extern u8 D_8011F3D8[];
extern u8 D_80164E70[];
extern s32 D_8011F428;
extern s32 D_801227CC;
extern s32 D_801227F4;
extern s32 D_8011F418;
extern u8 D_80122A08[];
extern s32 D_80164F08;
extern s32 D_80164F10;


void func_80140D2C(void);
void func_80141F18();
s32 func_80140774(void);
s32 func_80140868(void);
s32 func_80140D4C(s32 *ot, s32 prim, s32 arg2, s32 arg3);
s32 func_801413FC(s32 *ot, s32 prim, s32 arg2, s32 arg3);
s32 func_801414A8(s32 *ot, s32 prim, s32 arg2, s32 arg3);
s32 func_80141584(s32 *ot, s32 prim, s32 arg2, s32 arg3);
s32 func_80141660(s32 *ot, s32 prim, s32 arg2, s32 arg3);
s32 func_8014303C(s32 result, s32 *ot, s32 x, s32 y, s32 adjust, s32 slot, s32 i, s32 j);
s32 func_801469C0(s32 result, s32 *ot, u8 *name, s32 x, s32 y, s32 a5, s32 a6);
void func_80141DB8(void *arg0);
s32 func_80141E1C(s32 *ot, s32 prim, s32 arg2, s32 arg3);
s32 func_801433BC(s32 *ot, s32 prim, s32 arg2, s32 arg3);
void func_80141E84();
s32 func_80144DF8(void);

void func_800A3938();
s32 func_80142D04(s32 *ot, s32 prim, s32 arg2, s32 arg3);
s32 func_80142E88(s32 *ot, s32 prim, s32 arg2, s32 arg3);
void func_80140C60();
void func_80140BF0();
void func_80145F68();
void func_80140CC8();
s32 func_8001714C();
NikiElement *func_80141EC4();
void func_80143284();
void func_80145994();
s32 func_80142820(s32 *ot, s32 prim, s32 arg2, s32 arg3);
s32 func_8014262C(s32 *ot, s32 prim, s32 arg2, s32 arg3);

#define SET_ELEM_CODE(e, c) ((e)->attr.word = ((e)->attr.word & 0x00FFFFFF) | ((u32)(c) << 24))

void func_8014011C(s32 arg0, s32 arg1)
{
    RECT rect;

    D_80164AE8 = arg1;
    D_80164B78 = 0xFF;
    D_80164B70 = 0;
    func_80144BC0();
    func_80145A40();
    D_80164AD0 = 0;
    func_80067F8C();
    rect.x = 0x140;
    rect.y = 0;
    rect.w = 0x40;
    rect.h = 0x100;
    func_8001990C(&rect, 0, 0, 0);
    func_80146F34();
    D_80164B90 = 0;
    D_80164AD4 = 0;
    D_80164B84 = 0;
    D_80164A78 = 0;
    D_80164B8C = 0;
    D_80164B74 = 0;
    func_800AA02C();
    func_8014027C();
    D_80164AE4 = arg0;
}

s32 func_801401F0(s32 arg0)
{
    if (D_80164B74 != 0)
    {
        func_80145C0C();
        field_text_reset_windows();
        func_80019788(0);
        return 1;
    }
    field_text_reset_scratch();
    func_80146EB8();
    func_8014068C(arg0);
    func_80146EF4();
    func_80063194();
    D_80164B8C ^= 1;
    return 0;
}

void func_8014027C(void)
{
    NikiElement *p;
    D_80164B88 = 0;
    D_80164AEC = 0;
    D_80164AE0 = 0;
    D_80164B7C = 0;
    D_80164B84 = 0;
    D_80164ADC = (s32)D_8012271C + 0xCE0;
    if (0) func_80141E84(0,0,0,0,0);
    func_80141E84();
    D_80164B80 = 0;
    if (D_80164AE8 != 0)
    {
        D_80164B10.attr.f.state = 1;
        p = func_80141EC4();
        p->draw_handler = (void *)func_801433BC;
        p->attr.f.unk0_3 = 1;
        p->attr.f.x = 0x10;
        p->attr.f.unk0_16 = 0x61;
        p->unk4_0 = 1;
        p->y = 0x2C;
        SET_ELEM_CODE(p, 0x20);

        p = func_80141EC4();
        p->draw_handler = (void *)func_801414A8;
        p->attr.f.unk0_3 = 1;
        p->attr.f.x = 0x18;
        p->attr.f.unk0_16 = 0x4D;
        p->unk4_0 = 0;
        p->y = 0x10;
        SET_ELEM_CODE(p, 0x80);

        p = func_80141EC4();
        p->draw_handler = (void *)func_80141584;
        p->attr.f.unk0_3 = 1;
        p->attr.f.x = 0xA0;
        p->attr.f.unk0_16 = 0x4D;
        p->unk4_0 = 0;
        p->y = 0x10;
        SET_ELEM_CODE(p, 0x80);
        D_80164B10.attr.f.state = 0;
        return;
    }

    D_80164B10.attr.f.state = 1;
    p = func_80141EC4();
    p->draw_handler = (void *)func_80140D4C;
    p->attr.f.unk0_3 = 1;
    p->attr.f.x = 0x1C;
    p->attr.f.unk0_16 = 0x32;
    p->unk4_0 = 1;
    p->y = 0x58;
    SET_ELEM_CODE(p, 8);

    p = func_80141EC4();
    p->draw_handler = (void *)func_801413FC;
    p->attr.f.unk0_3 = 1;
    p->attr.f.x = 0x24;
    p->attr.f.unk0_16 = 0x0A;
    p->unk4_0 = 0;
    p->y = 0x10;
    SET_ELEM_CODE(p, 0xF0);

    p = func_80141EC4();
    p->draw_handler = (void *)func_801414A8;
    p->attr.f.unk0_3 = 1;
    p->attr.f.x = 0x18;
    p->attr.f.unk0_16 = 0x1E;
    p->unk4_0 = 0;
    p->y = 0x10;
    SET_ELEM_CODE(p, 0x80);

    p = func_80141EC4();
    p->draw_handler = (void *)func_80141584;
    p->attr.f.unk0_3 = 1;
    p->attr.f.x = 0xA0;
    p->attr.f.unk0_16 = 0x1E;
    p->unk4_0 = 0;
    p->y = 0x10;
    SET_ELEM_CODE(p, 0x80);

    p = func_80141EC4();
    p->draw_handler = (void *)func_80141660;
    p->attr.f.unk0_3 = 1;
    p->attr.f.x = 0x1E;
    p->attr.f.unk0_16 = 0x8E;
    p->unk4_0 = 1;
    p->y = 0x34;
    SET_ELEM_CODE(p, 4);
    D_80164B10.attr.f.state = 0;
}

void func_8014068C(void)
{
    s32 delta;

    func_80140D2C();
    D_80164AD0 += 2;
    if ((D_80164B1C.attr.word & 0x7F) == 2)
    {
        func_80140774();
    }
    if ((u16)D_80122988 == 0xFFFF)
    {
        D_80122988 = 0;
    }
    func_80140868();
    if (D_80164B88 != 0)
    {
        s32 base = D_80164AE0;
        delta = (D_80164AEC - D_80164AE0) / D_80164B88;
        D_80164B88 -= 1;
        D_80164AE0 += delta;
    }
    else
    {
        D_80164AE0 = D_80164AEC;
    }
}

s32 func_80140774(void)
{
    s32 result;

    if (D_80164B78 >= 0x10)
    {
        if (D_80164E18 == NULL)
        {
            D_80164E18 = &D_801606C8;
        }
    }

    do
    {
        result = func_80144DF8();
    } while (result == 3);

    if ((D_80164B80 != 0) && (D_80122988 & 0x220))
    {
        D_80164B78 = 0xF8;
        D_80164E18 = &D_801606DC;
    }
    else
    {
        switch (result)
        {
        case 0:
            break;
        case 4:
            D_80164E18 = &D_801606D4;
            D_80164B80 = 0;
            break;
        case 5:
            D_80164B78 = 0xF8;
            /* fallthrough */
        case 2:
            D_80164E18 = &D_801606DC;
            break;
        }
    }
}

s32 func_80140868(void)
{
    s32 pending;
    s32 status;
    s32 count;
    s32 term1;
    s32 term2;
    NikiElement *p;

    if ((D_80164B10.unkC & 7) == 0) {
        D_80164B74 = 1;
        return;
    }
    if (D_80164B74 != 0) {
        return;
    }
    if ((D_80164B10.unkC & 7) >= 3) {
        return;
    }
    if ((D_80164B10.attr.word & 7) != 0) {
        return;
    }
    pending = D_80164B78;
    if (pending == 0xFF) {
        return;
    }
    if (D_80164F18 != 0) {
        return;
    }
    if (D_80164A78 != 0) {
        return;
    }
    if ((u32)(*D_80164E18 - 6) < 2U) {
        return;
    }
    if (D_80164AE8 != 0) {
        return;
    }

    status = D_80122988;
    if (status & 0x40) {
        D_80122994 = 3;
        func_800A3938(0x78, 0x80);
        func_80140C60();
        return;
    }
    if (status & 0xA100) {
        func_800A3938(0x7D, 0x80);
        func_80140BF0();
        return;
    }
    if (pending >= 0x10) {
        return;
    }

    count = 1;
    if (status & 8) {
        D_80122988 = 0x4000;
        count = 1;
    }
    if (D_80122988 & 4) {
        D_80122988 = 0x1000;
        count = 1;
    }

    while (count != 0) {
        if (D_80122988 & 0x1000) {
            D_80164B7C -= 1;
            if (D_80164B7C < 0) {
                D_80164B7C = D_80164B78 - 1;
            }
        }
        if (D_80122988 & 0x4000) {
            D_80164B7C += 1;
            if (D_80164B7C >= D_80164B78) {
                D_80164B7C = 0;
            }
        }
        count -= 1;
    }

    if (D_80122988 & 0x5000) {
        func_80145F68();
        func_800A3938(0x7D, 0x80);
        func_80140CC8();
        return;
    }

    if (D_80122988 & 0x220) {
        if (D_80164AE8 != 0) {
            return;
        }
        term1 = D_80164B70 * 0x320;
        term2 = (D_80164B7C * 0x28) + (s32)D_80165018;
        if (func_8001714C(D_800ECF7C, (char *)(term1 + term2), 0xC) == 0) {
            if ((D_80164D18.unkD4 != D_8012271C->unkD4) &&
                (D_80164D18.unk17 != 0) &&
                ((D_8003EC9C == 0xFF) || (D_80164D18.unkCF == D_8003EC9C))) {
                p = func_80141EC4();
                p->attr.f.unk0_3 = 1;
                p->attr.f.x = 0x10;
                p->attr.f.unk0_16 = 0x61;
                p->unk4_0 = 1;
                p->y = 0x1E;
                SET_ELEM_CODE(p, 0x20);
                func_80143284();
                p->draw_handler = func_8014262C;
                func_80145994();
                func_800A3938(0x7E, 0x80);
                return;
            }
        }
        func_800A3938(0x78, 0x80);
    }
}

void func_80140BF0(void) {
    D_80164B80 = 0;
    D_80164E18 = 0;
    D_80164B78 = 0xFF;
    D_80164B88 = 0;
    D_80164AEC = 0;
    D_80164AE0 = 0;
    D_80164B7C = 0;
    D_80164B84 = 0;
    D_80164B70 ^= 1;
    func_80144BC0();
}

void func_80140C60(void)
{
    s32 temp_v1;
    s32 var_a1;
    s32 *var_a0;
    s32 temp;

    func_80067F28();
    var_a0 = &D_80164B10;
    var_a1 = 0;
    do
    {
        temp_v1 = *var_a0;
        if (temp_v1 & 7)
        {
            temp = (temp_v1 & ~7) | 3;
            *var_a0 = (temp & ~0x78) | 0x40;
        }
        var_a1 += 1;
        var_a0 += 3;
    } while (var_a1 < 8);
}

void func_80140CC8(void)
{
    s32 index;
    s32 temp;
    s32 base;
    s32 pos;
    s32 diff;

    index = D_80164B7C;
    temp = (index << 3) - index;
    base = D_80164AE0;
    pos = temp << 1;
    diff = pos - base;

    if (diff >= 0x4B) {
        D_80164AEC = pos - 0x46;
        D_80164B88 = 4;
    }
    if (diff < 0) {
        D_80164AEC = pos;
        D_80164B88 = 4;
    }
}

void func_80140D2C(void)
{
    func_80141F18();
}

/* ----- Decls for func_80140D4C (niki row/status list renderer) ----- */
typedef struct
{
    s16 x;
    s16 y;
} Vec2s;

typedef struct
{
    u32 tag;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    s16 w, h;
} TILE;

/**
 * @brief 0x28-byte textured-quad primitive built by func_8014303C for a
 *        save-slot glyph (a shadow tile plus the main tile).
 */
typedef struct
{
    s32 unk0;
    s32 unk4;
    s16 unk8;
    s16 unkA;
    u8 unkC;
    u8 unkD;
    s16 unkE;
    s16 unk10;
    s16 unk12;
    u8 unk14;
    u8 unk15;
    s16 unk16;
    s16 unk18;
    s16 unk1A;
    u8 unk1C;
    u8 unk1D;
    u8 pad1E[2];
    s16 unk20;
    s16 unk22;
    u8 unk24;
    u8 unk25;
    u8 pad26[2];
} NikiGlyphPrim;

s32 func_800A88A0(s32 prim, s32 *ot, void *glyph, s32 a3, s32 x, s32 y, s32 mode);
s32 func_800A8A78(s32 *ot, s32 prim, s32 ch, s32 a3, Vec2s *pos, s32 mode);
u8 *func_801442C4(void *arg0);
void func_80019A34(RECT *rect, void *str);
void func_800A55E4(void *buf, s32 arg1);
void func_800A5638(void *buf, s32 arg1);

extern char D_800ECF8C[];
extern char D_800ECFC4[];
extern u16 D_801470F8;
extern u16 D_801470FA;
extern u16 D_801470FC;
extern u16 D_801470FE;
extern u16 D_80147100;
extern u16 D_80147108;
extern u16 D_8014710A;
extern u16 D_8014710C;
extern u16 D_80147126;
extern u16 D_8014712C;
extern u16 D_80147132;
extern u16 D_80147134;
extern u16 D_80147136;
extern u16 D_80147138;
extern u16 D_8014713A;
extern u16 D_801471A8;
extern s32 D_80164E20[];
extern s32 D_80164EB0;
extern s32 D_80164EB8[];
extern s32 D_801477AC[];
extern u8 D_80164AF0[];

#define GLYPH_SYM(sym, off) ((void *)(((u8 *)&(sym) - (off)) + (sym)))
#define GLYPH_OFF(base, off) ((void *)((base) + *(u16 *)((base) + (off))))

/**
 * @brief Render the niki row/status list: per-entry glyphs, markers and the
 *        highlight tile, dispatched by the D_80164B78 list-state selector.
 * @param ot Ordering-table pointer.
 * @param prim Primitive-buffer write cursor.
 * @param arg2 Horizontal scroll offset (subtracted from every x).
 * @param arg3 Vertical scroll offset (subtracted from every row y).
 * @return Advanced primitive-buffer write cursor.
 * @see decomp.me (100%)
 */
s32 func_80140D4C(s32 *ot, s32 prim, s32 arg2, s32 arg3)
{
    s32 state = D_80164B78;

    switch (state)
    {
    case 0xF8:
        do { prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014712C, 0x34), 4, -arg2 + 0x84, -arg3, 2); } while (0);
        break;
    case 0xF9:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014712C, 0x34), 4, -arg2 + 0x84, -arg3, 2);
        break;
    case 0xFA:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_801470FA, 2), 4, -arg2 + 0x84, -arg3, 2);
        break;
    case 0xFD:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_801470FC, 4), 4, -arg2 + 0x84, -arg3, 2);
        break;
    case 0xFB:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80147108, 0x10), 4, -arg2 + 0x84, -arg3, 2);
        break;
    case 0xFC:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014710A, 0x12), 4, -arg2 + 0x84, -arg3, 2);
        break;
    default:
        {
            s32 row_y;
            s32 i;

        if (D_80164F18 != 0)
        {
            s32 x;
            u8 *base;
        case 0xFF:
            x = -arg2 + 0x84;
            base = (u8 *)&D_801470F8;
            prim = func_800A88A0(prim, ot, base + D_801470F8, 4, x, -arg3, 2);
            prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0x1E), 4, x, 0xE - arg3, 2);
            prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0xB2), 4, x, 0x1C - arg3, 2);
            break;
        }
        i = 0;
        if (state > 0)
        {
            s32 base_x;
            s32 *flag_ptr;
            u16 misc_glyph;
            Vec2s pos;
            s32 row;
            u8 *base;

            base = (u8 *)&D_801470F8;
            base_x = -arg2;
            do
            {
                row = ((i * 14) - arg3) - D_80164AE0;
                row_y = row + 1;
                if ((u32)(row + 0xE) < 0x65U)
                {
                    flag_ptr = &D_80164E20[i];
                    if (*flag_ptr >= 0)
                    {
                        pos.x = base_x + 0x86;
                        pos.y = row_y;
                        prim = func_800A88A0(func_800A8A78(ot, prim, *(s32 *)((u8 *)D_80164EB8 + (i * 4)), 4, &pos, 0), ot, (void *)((s32)D_80147126 + (s32)base), 4, base_x + 0x70, row_y, 0);
                        if ((D_80164EB0 - 1) == *flag_ptr)
                        {
                            misc_glyph = *(u16 *)(base + 0x36);
                            prim = func_800A88A0(prim, ot, (void *)((s32)misc_glyph + (s32)base), 4, base_x + 0xC0, row_y, 0);
                        }
                        else if (*flag_ptr < 2)
                        {
                            misc_glyph = *(u16 *)(base + 0x38);
                            prim = func_800A88A0(prim, ot, (void *)((s32)misc_glyph + (s32)base), 4, base_x + 0xC0, row_y, 0);
                        }
                        if (*func_801442C4((void *)((s32)&D_80165018[D_80164B70][i] + 0xC)) == 0x2B)
                        {
                            prim = func_800A88A0(prim, ot, (void *)((s32)D_801471A8 + (s32)base), 4, 0xF2 - arg2, row_y, 1);
                        }
                    }
                    if (func_8001714C(D_800ECF7C, (char *)((s32)&D_80165018[D_80164B70][i]), 0xC) == 0)
                    {
                        prim = func_800A88A0(prim, ot, (void *)((s32)D_801470FE + (s32)base), 4, 1 - arg2, row_y, 0);
                    }
                    else if (func_8001714C(D_800ECF8C, (char *)((s32)&D_80165018[D_80164B70][i]), 0xC) == 0)
                    {
                        prim = func_800A88A0(prim, ot, (void *)((s32)D_80147132 + (s32)base), 4, 1 - arg2, row_y, 0);
                    }
                    else if (func_8001714C(D_800ECFC4, (char *)((s32)&D_80165018[D_80164B70][i]), 8) == 0)
                    {
                        prim = func_800A88A0(prim, ot, (void *)((s32)D_8014710C + (s32)base), 4, 1 - arg2, row_y, 0);
                    }
                    else
                    {
                        prim = func_800A88A0(prim, ot, (void *)((s32)D_80147100 + (s32)base), 4, 1 - arg2, row_y, 0);
                    }
                }
                i++;
            } while (i < D_80164B78);
        }
            row_y = ((D_80164B7C * 14) - arg3) - D_80164AE0;

            if (D_80164F18 == 0)
            {
                TILE *tile = (TILE *)prim;

                *(u32 *)&tile->r0 = 0xF080F0;
                *((u8 *)tile + 3) = 3;
                tile->code = 0x62;
                tile->w = 0x108;
                tile->x0 = 0;
                tile->y0 = row_y;
                tile->h = 0xE;
                tile->tag = (tile->tag & 0xFF000000) | (*ot & 0xFFFFFF);
                *ot = (*ot & 0xFF000000) | ((s32)tile & 0xFFFFFF);
                prim += sizeof(TILE);
            }
        }
        break;
    }
    return prim;
}

/* ----- Decls for func_801413FC (niki mode banner) ----- */
extern u16 D_8014713C;
extern u16 D_8014713E;

/**
 * @brief Draw the niki header banner glyph, picking one of two captions
 *        according to the D_80164AE8 mode selector.
 * @param ot Ordering-table pointer.
 * @param prim Primitive-buffer write cursor.
 * @param arg2 Horizontal scroll offset (subtracted from the banner x).
 * @param arg3 Vertical scroll offset (subtracted from the banner y).
 * @return Advanced primitive-buffer write cursor.
 * @see decomp.me (100%)
 */
s32 func_801413FC(s32 *ot, s32 prim, s32 arg2, s32 arg3)
{
    RECT pos;

    if (D_80164AE8 == 1)
    {
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014713E, 0x46), 4, -arg2 + 0x78, -arg3, 2);
    }
    else
    {
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014713C, 0x44), 4, -arg2 + 0x78, -arg3, 2);
    }
    return prim;
}

/* ----- Decls for func_801414A8 (niki page-header banner) ----- */
extern u16 D_80147104;

/**
 * @brief Draw the niki page header: an optional dark backdrop tile for pages
 *        past the first, then the header caption glyph.
 *
 * The backdrop is a 0x80 x 0x10 flat tile (code 0x62, color 0x101010) linked
 * into @p ot ahead of the caption, and is emitted only when D_80164B70 selects
 * a non-zero page.
 *
 * @param ot Ordering-table pointer.
 * @param prim Primitive-buffer write cursor.
 * @param arg2 Horizontal scroll offset (subtracted from the caption x).
 * @param arg3 Vertical scroll offset (subtracted from the caption y).
 * @return Advanced primitive-buffer write cursor.
 * @see decomp.me (100%)
 */
s32 func_801414A8(s32 *ot, s32 prim, s32 arg2, s32 arg3)
{
    RECT pos;
    TILE *tile;

    if (D_80164B70 != 0)
    {
        tile = (TILE *)prim;
        *(u32 *)&tile->r0 = 0x101010;
        *((u8 *)tile + 3) = 3;
        tile->code = 0x62;
        tile->x0 = 0;
        tile->y0 = 0;
        tile->w = 0x80;
        tile->h = 0x10;
        tile->tag = (tile->tag & 0xFF000000) | (*ot & 0xFFFFFF);
        *ot = (*ot & 0xFF000000) | ((s32)tile & 0xFFFFFF);
        prim += 0x10;
    }
    return func_800A88A0(prim, ot, GLYPH_SYM(D_80147104, 0xC), 4, -arg2 + 0x40, -arg3, 2);
}

/* ----- Decls for func_80141584 (niki first-page header banner) ----- */
extern u16 D_80147106;

/**
 * @brief Draw the niki first-page header: an optional dark backdrop tile,
 *        then the header caption glyph.
 *
 * Mirror of func_801414A8: identical 0x80 x 0x10 flat backdrop tile (code
 * 0x62, color 0x101010), but emitted only when D_80164B70 selects page zero,
 * and paired with a different caption glyph.
 *
 * @param ot Ordering-table pointer.
 * @param prim Primitive-buffer write cursor.
 * @param arg2 Horizontal scroll offset (subtracted from the caption x).
 * @param arg3 Vertical scroll offset (subtracted from the caption y).
 * @return Advanced primitive-buffer write cursor.
 * @see decomp.me (100%)
 */
s32 func_80141584(s32 *ot, s32 prim, s32 arg2, s32 arg3)
{
    RECT pos;
    TILE *tile;

    if (D_80164B70 == 0)
    {
        tile = (TILE *)prim;
        *(u32 *)&tile->r0 = 0x101010;
        *((u8 *)tile + 3) = 3;
        tile->code = 0x62;
        tile->x0 = 0;
        tile->y0 = 0;
        tile->w = 0x80;
        tile->h = 0x10;
        tile->tag = (tile->tag & 0xFF000000) | (*ot & 0xFFFFFF);
        *ot = (*ot & 0xFF000000) | ((s32)tile & 0xFFFFFF);
        prim += 0x10;
    }

    return func_800A88A0(prim, ot, GLYPH_SYM(D_80147106, 0xE), 4, -arg2 + 0x40, -arg3, 2);
}

/**
 * @brief Draw the niki save-slot detail panel: element glyphs, the playtime
 *        clock, the slot marker row, and a fallback name/second-line block.
 *
 * Runs only while the panel is active (D_80164B84 non-zero) and not suppressed
 * (D_80164F18 zero). Depending on D_80164B84 it either emits a two-line caption
 * (state 2), or renders the full slot detail: up to three party markers laid out
 * by func_8014303C with an animated highlight (D_80164AD0), the playtime split
 * into hours/minutes via func_800A8A78, and one of three status glyphs. If the
 * slot compare fails it falls back to drawing the stored name and second line.
 *
 * @param ot Ordering-table pointer.
 * @param prim Primitive-buffer write cursor.
 * @param arg2 Horizontal scroll offset (subtracted from every x).
 * @param arg3 Vertical scroll offset (subtracted from every row y).
 * @return Advanced primitive-buffer write cursor.
 * @see decomp.me (100%)
 */
s32 func_80141660(s32 *ot, s32 prim, s32 arg2, s32 arg3)
{
    s32 result;
    Vec2s pos;
    u8 name[0x21];
    char unused_pad[212];
    s32 slot[3];

    result = prim;
    if (D_80164B84 == 0)
    {
        return result;
    }
    if (D_80164F18 != 0)
    {
        return result;
    }
    if (D_80164B84 != 3 && D_80164B78 < 0x10)
    {
        if (D_80164B84 == 2)
        {
            s32 x = -arg2;
            u8 *base;

            result = func_800A88A0(prim, ot, GLYPH_SYM(D_80147120, 0x28), 4, x, -arg3, 0);
            base = (u8 *)&D_80147120 - 0x28;
            return func_800A88A0(result, ot, GLYPH_OFF(base, 0x2A), 4, x, 0x10 - arg3, 0);
        }
        else
        {
            s32 term1 = D_80164B70 * 0x320;
            s32 term2 = (D_80164B7C * 0x28) + (s32)D_80165018;

            if (func_8001714C(D_800ECF7C, (char *)(term1 + term2), 0xC) == 0)
            {
                if (D_8003EC9C == 0xFF || D_80164DE7 == D_8003EC9C)
                {
                    s32 present_count;
                    s32 i;
                    s32 j;
                    s32 step;
                    s32 half_step;
                    s32 base_x;
                    s32 base_y;
                    s32 total;
                    s32 hours;
                    s32 minutes;
                    s32 time_val;

                    {
                        u8 *record = (u8 *)&D_80164D18;
                        slot[0] = (u32)(*(s32 *)(record + 0x18)) >> 0x19;
                        slot[1] = ((u32)(*(s32 *)(record + 0x20)) >> 0x12) & 0x7F;
                        slot[2] = (u32)(*(s32 *)(record + 0x20)) >> 0x19;
                        D_80164AD8 = (s32)record[0x1F];
                    }

                    total = 0;
                    present_count = 0;
                    for (i = 0; i < 3; i++)
                    {
                        if (slot[i] != 0x7F)
                        {
                            present_count += 1;
                        }
                    }

                    switch (present_count)
                    {
                    case 2:
                        step = 0x20;
                        half_step = 0x10;
                        time_val = D_80164AD0;
                        if (D_80164AD0 < 0)
                        {
                            time_val = D_80164AD0 + 0x1F;
                        }
                        D_80164AD0 -= (time_val >> 5) << 5;
                        break;
                    case 3:
                        step = 0x10;
                        half_step = 0x20;
                        D_80164AD0 %= 0x60;
                        break;
                    default:
                        step = 0x10;
                        half_step = 0x20;
                        D_80164AD0 = 0x1F;
                        break;
                    }

                    i = 0;
                    j = i;
                    for (; j < 3; j++)
                    {
                        base_y = i * half_step;
                        base_x = base_y + half_step;
                        if (slot[j] != 0x7F)
                        {
                            s32 adjust = step;
                            s32 rem;
                            s32 hi;
                            s32 delta;

                            if ((D_80164AD0 >= base_y && D_80164AD0 < base_x && (delta = D_80164AD0 - base_y, 1))
                                || (rem = base_x % (half_step * present_count), D_80164AD0 >= rem && D_80164AD0 < (hi = rem + half_step) && (delta = hi - D_80164AD0, 1)))
                            {
                                adjust += delta;
                            }
                            result = func_8014303C(result, ot, total - arg2, -arg3, adjust, slot[j], i, j);
                            i += 1;
                            total += adjust;
                        }
                    }

                    {
                        u8 *base90 = (u8 *)&D_80164D18;
                        s32 x = -arg2;
                        s32 y = -arg3;

                        base_y = *(s32 *)(base90 + 0x30);
                        pos.x = (s16)(x + 0x70);
                        pos.y = (s16)y;
                        hours = base_y / 216000;
                        result = func_800A8A78(ot, result, hours, 4, &pos, 1);
                        result = func_800A88A0(result, ot,
                            D_800EC3F6[0] + ((s32)&D_800EC3F6 - 0x32) + (D_800EC3F6[1] << 8), 4, x + 0x6F, y, 0);
                        base_y = (base_y / 3600) - (hours * 0x3C);
                        if (base_y < 0xA)
                        {
                            pos.x = (s16)(x + 0x7D);
                            pos.y = (s16)y;
                            result = func_800A8A78(ot, result, 0, 4, &pos, 1);
                        }
                        pos.x = (s16)(x + 0x85);
                        pos.y = (s16)y;
                        result = func_800A8A78(ot, result, base_y, 4, &pos, 1);
                        result = func_800A88A0(result, ot, base90, 4, x + 0x54, y + 0x10, 0);

                        if (*(u16 *)(base90 + 0xD4) == *(u16 *)((u8 *)D_8012271C + 0xD4))
                        {
                            result = func_800A88A0(result, ot, GLYPH_SYM(D_80147148, 0x50), 4, x + 0x54, y + 0x20, 0);
                        }
                        else if (base90[0x17] == 0)
                        {
                            result = func_800A88A0(result, ot, GLYPH_SYM(D_80147146, 0x4E), 4, x + 0x54, y + 0x20, 0);
                        }
                        else
                        {
                            result = func_800A88A0(result, ot, GLYPH_OFF((u8 *)D_801475C4, (*(s32 *)(base90 + 0x20) & 0x3FFFF) * 2), 4,
                                x + 0x54, y + 0x20, 0);
                        }
                    }
                }
                else
                {
                    result = func_800A88A0(result, ot, GLYPH_SYM(D_8014714C, 0x54), 4, -arg2, -arg3, 0);
                }
            }
            else
            {
                s32 j;
                u8 *record;

                func_80141DB8(&D_80164B9C);
                record = &D_80164B9C;
                record -= 4;
                if ((u32)(record[0x24] - 1) >= 0x7FU)
                {
                    for (j = 0; j < 0x20; j++)
                    {
                        name[j] = record[4 + j];
                    }
                    name[j] = 0;
                    result = func_801469C0(result, ot, name, -arg2, -arg3, 4, 0);

                    for (j = 0; j < 0x20; j++)
                    {
                        name[j] = ((NikiFallbackText *)&D_80164B98)->text[j];
                    }
                    name[j] = 0;
                    result = func_801469C0(result, ot, name, -arg2, -arg3 + 0x10, 4, 0);
                }
            }
        }
    }
    return result;
}

/**
 * @brief Zero-fill the tail of a 0x40-byte record once a terminator is seen.
 *
 * Walks a variable-width byte stream up to logical index 0x40: a lead byte
 * >= 0x80 consumes two positions, otherwise one. On the first zero byte the
 * remaining bytes through index 0x40 are cleared.
 *
 * @param arg0 Pointer to the record buffer to scan and pad.
 * @see decomp.me (100%)
 */
void func_80141DB8(void *arg0)
{
    u8 *p;
    s32 i;

    p = (u8 *)arg0;
    i = 0;
    for (;;)
    {
        if (i >= 0x40)
        {
            return;
        }
        if (*p == 0)
        {
            while (i < 0x40)
            {
                *p = 0;
                i++;
                p++;
            }
            return;
        }
        if (*p >= 0x80)
        {
            p += 2;
            i += 2;
        }
        else
        {
            p += 1;
            i += 1;
        }
    }
}

/**
 * @brief Draw the niki footer glyph, anchored to the right edge of the panel.
 *
 * Resolves the glyph pointer from the D_800EC3D0 header (a 16-bit offset stored
 * across bytes [0] and [1], added to the header base less 0xC), then submits it
 * at x = 0x80 - arg2, y = -arg3.
 *
 * @param ot Ordering-table pointer.
 * @param prim Primitive-buffer write cursor.
 * @param arg2 Horizontal scroll offset (subtracted from the anchor x).
 * @param arg3 Vertical scroll offset (subtracted from the anchor y).
 * @return Advanced primitive-buffer write cursor.
 * @see decomp.me (100%)
 */
s32 func_80141E1C(s32 *ot, s32 prim, s32 arg2, s32 arg3)
{
    RECT pos;

    return func_800A88A0(prim, ot,
        (void *)((u8 *)D_800EC3D0 - 0xC + D_800EC3D0[0] + (D_800EC3D0[1] << 8)),
        5, 0x80 - arg2, -arg3, 2);
}

/**
 * @brief Reset the niki element array: clear the low 3 state bits of each of
 *        the eight D_80164B10 entries and reload the D_8012298C counter.
 *
 * @see decomp.me (100%)
 */
void func_80141E84(void)
{
    NikiPacket *p;
    s32 i;

    D_8012298C = 0x20;
    p = (NikiPacket *)&D_80164B10;
    for (i = 0; i < 8; i++)
    {
        p->attr.word &= ~7;
        p++;
    }
}

/**
 * @brief Claim the first free niki element slot, marking its state bits to 1.
 *
 * Scans the eight D_80164B10 entries for one whose low 3 state bits are clear,
 * sets them to 1, and returns it. Falls back to the first entry if none free.
 *
 * @return Pointer to the claimed (or fallback) element.
 * @see decomp.me (100%)
 */
NikiElement *func_80141EC4(void)
{
    NikiPacket *p;
    s32 i;

    p = (NikiPacket *)&D_80164B10;
    for (i = 0; i < 8; i++, p++)
    {
        if ((p->attr.word & 7) == 0)
        {
            p->attr.word = (p->attr.word & ~7) | 1;
            return (NikiElement *)p;
        }
    }
    return &D_80164B10;
}

/**
 * @brief Advance and draw the eight niki element packets for one frame.
 *
 * Reinitialises the GPU primitive window (clip variant selected by
 * arg0->unk40B2), then walks the eight D_80164B10 element slots. Active slots
 * (low 3 state bits set) are stepped through their animation phase (switch on
 * the state code) and rendered via each slot's draw callback plus func_800AD850,
 * advancing the shared GPU-packet cursor which is written back to arg0->unk40B8.
 *
 * @param arg0 Per-frame draw state (see NikiDrawState).
 * @see decomp.me (100%)
 */
void func_80141F18(NikiDrawState *arg0)
{
    NikiGpuPacket *var_s0;
    NikiDrawState *var_s5;
    volatile u32 *var_s3;
    s32 temp_s1;
    s32 temp_s2;
    s32 var_s6;
    s32 sp20[24];
    u32 temp_a0_2;
    s32 temp_v1_2;
    u32 temp_a1;
    u32 temp_a2;
    s32 temp_a0_3;
    s32 var_v1;
    s32 temp_a3_2;
    s32 var_v0;
    s32 temp_a3_3;
    u32 temp_v0_3;
    u32 temp_a0_4;
    s32 temp_a0_5;
    s32 var_v1_2;
    s32 temp_a3_5;
    s32 var_v0_2;
    s32 temp_a3_6;
    u32 temp_v0_5;
    u32 temp_v1_3;
    s32 count;

    var_s0 = arg0->unk40B8;
    var_s5 = arg0;

    if (arg0->unk40B2 != 0)
    {
        func_8001C56C(sp20, 0, 0xF0, 0x140, 0xE0);
    }
    else
    {
        func_8001C56C(sp20, 0, 8, 0x140, 0xE0);
    }

    var_s3 = (volatile u32 *)&D_80164B10;
    var_s6 = 0;

    for (; var_s6 < 8; var_s6++, var_s3 += 3)
    {
        if (*var_s3 & 7)
        {
            count = D_80164B78;
            if ((count < 0x10) &&
                (*(NikiElemDrawFunc *)((u8 *)var_s3 + 8) == (NikiElemDrawFunc)func_80140D4C) &&
                ((D_80164B1C.attr.word & 7) == 2))
            {
                count *= 0xE;
                if ((D_80164AE0 + 0x58) < count)
                {
                    var_s0 = (NikiGpuPacket *)func_800AE76C(var_s0, var_s5, 0x114, 0x82, 0);
                }
                if (D_80164AE0 != 0)
                {
                    var_s0 = (NikiGpuPacket *)func_800AE76C(var_s0, var_s5, 0x114, 0x3A, 1);
                }
            }

            func_8001A5D4((s32)var_s0, sp20);

            var_s0->unk0 = (var_s0->unk0 & 0xFF000000) | (var_s5->unk0 & 0x00FFFFFF);
            var_s5->unk0 = (s32)((var_s5->unk0 & 0xFF000000) | ((s32)var_s0 & 0x00FFFFFF));

            temp_a0_2 = *var_s3;
            temp_v1_2 = temp_a0_2 & 7;

            var_s0 = (NikiGpuPacket *)((u8 *)var_s0 + 0x40);

            switch (temp_v1_2)
            {
            case 1:
                temp_v0_3 = *var_s3;
                temp_a1 = *(u32 *)((u8 *)var_s3 + 4);
                temp_a0_4 = temp_v0_3 >> 24;
                temp_a2 = ((temp_a1 & 1) << 8) | temp_a0_4;
                temp_a0_3 = (temp_v0_3 >> 3) & 0xF;
                var_v1 = temp_a2 * temp_a0_3;
                D_80122988 = 0;
                if (var_v1 < 0)
                {
                    var_v1 += 7;
                }
                temp_a3_2 = (temp_a1 >> 1) & 0xFF;
                var_v0 = temp_a3_2 * temp_a0_3;
                temp_s1 = var_v1 >> 3;
                if (var_v0 < 0)
                {
                    var_v0 += 7;
                }
                temp_s2 = var_v0 >> 3;
                temp_a3_3 = (s32)(temp_a3_2 - temp_s2);

                var_s0 = (*(NikiElemDrawFunc *)((u8 *)var_s3 + 8))(var_s5, var_s0, (s32)(temp_a2 - temp_s1) / 2, temp_a3_3 / 2);
                {
                    u32 post_word;
                    u32 field;
                    u32 high;
                    post_word = *var_s3;
                    field = (post_word >> 7) & 0x1FF;
                    high = post_word >> 24;
                    var_s0 = (NikiGpuPacket *)func_800AD850(var_s0, var_s5,
                                           field + (s32)((((*(u32 *)((u8 *)var_s3 + 4) & 1) << 8) | high) - temp_s1) / 2,
                                           (*((u8 *)var_s3 + 2)) + ((s32)((*(u32 *)((u8 *)var_s3 + 4) >> 1) & 0xFF) - temp_s2) / 2,
                                           temp_s1, temp_s2, arg0->unk40B2, var_s6 == 0);
                }
                {
                    u32 old_word;
                    u32 new_word;
                    old_word = *var_s3;
                    new_word = (old_word & ~0x78) | (((((old_word >> 3) & 0xF) + 1) & 0xF) * 8);
                    *(u32 *)var_s3 = new_word;
                    if (((new_word >> 3) & 0xF) == 8)
                    {
                        func_800AA02C();
                        *(u32 *)var_s3 = (*var_s3 & ~7) | 2;
                    }
                }
                break;

            case 2:
                var_s0 = (*(NikiElemDrawFunc *)((u8 *)var_s3 + 8))(var_s5, var_s0, 0, 0);
                {
                    u32 case_word;
                    u32 high;
                    case_word = *var_s3;
                    high = case_word >> 24;
                    var_s0 = (NikiGpuPacket *)func_800AD850(var_s0, var_s5,
                                           (case_word >> 7) & 0x1FF, *((u8 *)var_s3 + 2),
                                           ((*(u32 *)((u8 *)var_s3 + 4) & 1) << 8) | high,
                                           (*(u32 *)((u8 *)var_s3 + 4) >> 1) & 0xFF, arg0->unk40B2, var_s6 == 0);
                }
                temp_v1_3 = *var_s3;
                if (((temp_v1_3 >> 3) & 0xF) != 0)
                {
                    *(u32 *)var_s3 = (temp_v1_3 & ~0x78) | (((((temp_v1_3 >> 3) & 0xF) - 1) & 0xF) * 8);
                }
                break;

            case 3:
                temp_a0_5 = *var_s3;
                temp_a1 = *(u32 *)((u8 *)var_s3 + 4);
                var_v1_2 = (u32)temp_a0_5 >> 24;
                temp_a2 = ((temp_a1 & 1) << 8) | var_v1_2;
                temp_a0_5 = (u32)temp_a0_5 >> 3;
                temp_a0_5 &= 0xF;
                var_v1_2 = temp_a2 * temp_a0_5;
                D_80122988 = 0;
                if (var_v1_2 < 0)
                {
                    var_v1_2 += 7;
                }
                temp_a3_5 = (temp_a1 >> 1) & 0xFF;
                var_v0_2 = temp_a3_5 * temp_a0_5;
                temp_s1 = var_v1_2 >> 3;
                if (var_v0_2 < 0)
                {
                    var_v0_2 += 7;
                }
                temp_s2 = var_v0_2 >> 3;
                temp_a3_6 = (s32)(temp_a3_5 - temp_s2);

                var_s0 = (*(NikiElemDrawFunc *)((u8 *)var_s3 + 8))(var_s5, var_s0, (s32)(temp_a2 - temp_s1) / 2, temp_a3_6 / 2);
                {
                    u32 post_word;
                    u32 field;
                    u32 high;
                    post_word = *var_s3;
                    field = (post_word >> 7) & 0x1FF;
                    high = post_word >> 24;
                    var_s0 = (NikiGpuPacket *)func_800AD850(var_s0, var_s5,
                                           field + (s32)((((*(u32 *)((u8 *)var_s3 + 4) & 1) << 8) | high) - temp_s1) / 2,
                                           (*((u8 *)var_s3 + 2)) + ((s32)((*(u32 *)((u8 *)var_s3 + 4) >> 1) & 0xFF) - temp_s2) / 2,
                                           temp_s1, temp_s2, arg0->unk40B2, var_s6 == 0);
                }
                {
                    u32 old_word;
                    old_word = *var_s3;
                    var_v1_2 = old_word & ~0x78;
                    old_word >>= 3;
                    old_word &= 0xF;
                    old_word--;
                    old_word &= 0xF;
                    old_word <<= 3;
                    var_v1_2 |= old_word;
                    *(u32 *)var_s3 = var_v1_2;
                    if (!(((u32)var_v1_2 >> 3) & 0xF))
                    {
                        *(u32 *)var_s3 = ((((u32)var_v1_2 & ~0x78) | 0x18) & ~7) | 4;
                    }
                }
                break;

            case 4:
                temp_v0_5 = *(u32 *)var_s3;
                D_80122988 = 0;
                temp_v1_3 = (temp_v0_5 & ~0x78) | (((((temp_v0_5 >> 3) & 0xF) - 1) & 0xF) * 8);
                *(u32 *)var_s3 = temp_v1_3;
                if (!((temp_v1_3 >> 3) & 0xF))
                {
                    *(u32 *)var_s3 = temp_v1_3 & ~7;
                }
                break;
            }
        }
    }

    arg0->unk40B8 = var_s0;
}

/**
 * @brief Clear the low 3 state bits of the first niki element slot.
 * @see decomp.me (100%)
 */
void func_801424C0(void)
{
    ((NikiPacket *)&D_80164B10)->attr.word &= ~7;
}

/**
 * @brief Append the string @p arg1 to the end of string @p arg0 (strcat).
 *
 * Uses func_8014255C to find each string's length, copies arg1's bytes onto the
 * tail of arg0, and writes a terminating zero.
 *
 * @param arg0 Destination string, extended in place.
 * @param arg1 Source string to append.
 * @see decomp.me (100%)
 */
void func_801424D8(u8 *arg0, u8 *arg1)
{
    s32 temp_s0;
    s32 temp_v0;
    s32 i;

    temp_s0 = func_8014255C(arg0);
    temp_v0 = func_8014255C(arg1);
    for (i = 0; i < temp_v0; i++)
    {
        arg0[temp_s0 + i] = arg1[i];
    }
    arg0[temp_s0 + i] = 0;
}

/**
 * @brief Measure a niki string's length, counting bytes 0x19-0x1F as 2 units.
 *
 * Walks @p arg0 to its terminating zero. Lead bytes in the range 0x19..0x1F are
 * two-byte sequences and advance the length by 2; all others by 1.
 *
 * @param arg0 String to measure.
 * @return Logical length in layout units.
 * @see decomp.me (100%)
 */
s32 func_8014255C(u8 *arg0)
{
    u8 *p;
    u8 c;
    s32 len;

    p = arg0;
    c = *p;
    len = 0;
    while (c != 0)
    {
        if ((u32)(c - 0x19) < 7)
        {
            p += 2;
            len += 2;
        }
        else
        {
            p += 1;
            len += 1;
        }
        c = *p;
    }
    return len;
}

/**
 * @brief Copy niki string @p arg1 into @p arg0, terminating it (strcpy).
 *
 * First measures arg1's length (bytes 0x19..0x1F count as 2 units, via a
 * volatile read of the lead byte), then copies that many bytes and appends a
 * terminating zero.
 *
 * @param arg0 Destination buffer.
 * @param arg1 Source string to copy.
 * @see decomp.me (100%)
 */
void func_801425A8(u8 *arg0, u8 *arg1)
{
    u8 *p;
    u8 c;
    s32 len;
    s32 i;

    p = arg1;
    len = 0;

    while (*p != 0)
    {
        c = *(volatile u8 *)p;

        if ((u32)(c - 0x19) < 7)
        {
            p += 2;
            len += 2;
        }
        else
        {
            p++;
            len++;
        }
    }

    for (i = 0; i < len; i++)
    {
        arg0[i] = arg1[i];
    }

    arg0[i] = 0;
}

/**
 * @brief Draw and advance the niki confirmation element.
 *
 * Emits the confirmation caption glyph, then reads input: L1/R1-class cancels
 * (func_801459E8 result 1 or 2) and the cancel button tear down the element and
 * restore the prior menu; the confirm button spawns the confirmation element at
 * a fixed position with its draw handler set to func_80142820.
 *
 * @param ot Ordering-table pointer.
 * @param prim Primitive-buffer write cursor.
 * @param arg2 Horizontal scroll offset (subtracted from the caption x).
 * @param arg3 Vertical scroll offset (subtracted from the caption y).
 * @return Advanced primitive-buffer write cursor.
 * @see decomp.me (100%)
 */
s32 func_8014262C(s32 *ot, s32 prim, s32 arg2, s32 arg3)
{
    RECT pos;
    s32 result;
    s32 x;
    s32 status;
    NikiElement *p;

    x = -arg2 + 0x90;
    result = func_80143294(
        func_800A88A0(prim, ot,
                      (u8 *)&D_80147128 + D_80147128 - 0x30,
                      4, x, -arg3, 2),
        ot, x, 0xE - arg3);

    if ((u32)(func_801459E8() - 1) < 2U)
    {
        D_80164B10.attr.f.state = 0;
        func_800AA02C();
        func_800A3938(0x78, 0x80);
        D_80164B78 = 0xFF;
        func_80144BC0();
        D_80164E18 = 0;
    }
    else
    {
        status = D_80122988;
        if (status & 0x40)
        {
            D_80164B10.attr.f.state = 0;
            func_800AA02C();
            func_800A3938(0x78, 0x80);
            D_80164E18 = &D_801606DC;
        }
        else if (status & 0x220)
        {
            if (D_80164A7C != 0)
            {
                D_80164B10.attr.f.state = 0;
                func_800AA02C();
                func_800A3938(0x78, 0x80);
                D_80164E18 = &D_801606DC;
            }
            else
            {
                func_800A3938(0x7E, 0x80);
                D_80164AD4 = 1;
                D_80164E18 = &D_801606E4;
                p = &D_80164B10;
                p->draw_handler = func_80142820;
                p->attr.f.unk0_3 = 1;
                p->attr.f.state = 1;
                p->attr.f.x = 0x10;
                p->attr.f.unk0_16 = 0x61;
                p->unk4_0 = 1;
                p->y = 0x2C;
                SET_ELEM_CODE(p, 0x20);
            }
        }
    }
    return result;
}

/**
 * @brief Draw the niki save-confirm dialog and, on accept, commit the save.
 *
 * Emits the three caption glyphs and the sub-panel (func_80142A1C). When the
 * confirm latch D_80164AD4 is clear, reads the selected save resource: if the
 * slot is empty (func_80144310 == 0) it just re-arms the chooser, otherwise it
 * copies the save payload out, kicks the write, and flips every active element
 * to its closing animation state.
 *
 * @param ot Ordering-table pointer.
 * @param prim Primitive-buffer write cursor.
 * @param arg2 Horizontal scroll offset (subtracted from every caption x).
 * @param arg3 Vertical scroll offset (subtracted from every caption y).
 * @return Advanced primitive-buffer write cursor.
 * @see decomp.me (100%)
 */
s32 func_80142820(s32 *ot, s32 prim, s32 arg2, s32 arg3)
{
    RECT pos;
    u8 *base;
    u8 *resource;
    NikiPacket *p;
    NikiPacket *cursor;
    s32 result;
    s32 x;
    s32 i;

    x = -arg2 + 0x90;
    result = func_800A88A0(prim, ot, (void *)((s32)&D_8014712A - 0x32 + D_8014712A), 4, x, -arg3, 2);
    base = (u8 *)&D_8014712A - 0x32;
    result = func_800A88A0(result, ot, base + *(u16 *)(base + 0x1E), 4, x, 0xE - arg3, 2);
    result = func_800A88A0(result, ot, base + *(u16 *)(base + 0xB2), 4, x, 0x1C - arg3, 2);
    result = func_80142A1C(result, ot);

    if (D_80164AD4 == 0)
    {
        resource = D_80160A78;
        p = (NikiPacket *)&D_80164B10;
        p->attr.f.state = 0;
        if (func_80144310(resource) == 0)
        {
            func_80142B2C(4);
            return result;
        }

        func_800A3938(0x7B, 0x80);
        D_8011F428 = 1;
        D_801227CC = *(u16 *)&resource[0x254];
        D_801227F4 = *(u16 *)&resource[0x256];
        D_8011F418 = D_80164B70;
        func_800170BC(D_8011F3D8, D_80164E70);
        func_80016E7C(&resource[0x32E0], D_80122A08, 0x100);
        func_80067F28();

        cursor = p;
        for (i = 0; i < 8; i++, cursor++)
        {
            if (cursor->attr.f.state != 0)
            {
                cursor->attr.f.state = 3;
                cursor->attr.f.unk0_3 = 8;
            }
        }
        func_80067F5C(8);
    }

    return result;
}

/**
 * @brief Build the niki save-progress timer bar (a POLY_G4) when active.
 *
 * While the timer is running (D_80164F08 set), fills a POLY_G4 packet at @p arg0
 * with a yellow-to-red gradient whose horizontal extent tracks the elapsed time
 * (clamped to 0x100 ticks), links it into the ordering table, and advances the
 * packet cursor by 0x24.
 *
 * @param arg0 GPU packet cursor (POLY_G4 written here when the timer is active).
 * @param arg1 Ordering-table entry the packet is linked into.
 * @return Advanced packet cursor (unchanged when the timer is inactive).
 * @see decomp.me (100%)
 */
s32 func_80142A1C(s32 arg0, s32 *arg1)
{
    NikiPolyG4Words *g;
    s32 elapsed;
    s32 extent;
    s32 color;

    g = (NikiPolyG4Words *)arg0;
    if (D_80164F08 != 0)
    {
        elapsed = func_8002054C(-1) - D_80164F10;
        if (elapsed >= 0x101)
        {
            elapsed = 0x100;
        }
        color = 0xFFFF00;
        extent = elapsed * 0x120;
        g->unk4 = 0xFF;
        g->unkC = 0xFFFF;
        g->unk1C = 0xFF0000;
        ((u8 *)g)[3] = 8;
        g->unk14 = color;
        ((u8 *)g)[7] = 0x38;
        g->unk18 = 0;
        g->unk8 = 0;
        if (extent < 0)
        {
            extent += 0xFF;
        }
        g->unk20 = extent >> 8;
        g->unk10 = extent >> 8;
        g->unk12 = 0;
        g->unkA = 0;
        g->unk22 = 0x2C;
        g->unk1A = 0x2C;
        g->unk0 = (g->unk0 & 0xFF000000) | (*arg1 & 0xFFFFFF);
        *arg1 = (*arg1 & 0xFF000000) | (arg0 & 0xFFFFFF);
        arg0 += 0x24;
    }
    return arg0;
}

/**
 * @see decomp.me (100%)
 */
void func_80142B2C(s32 arg0)
{
    func_800A3938(0x78, 0x80);
    D_80164B10.draw_handler = (void *)func_80142D04;
    D_80164B10.attr.f.unk0_3 = 1;
    D_80164B10.attr.f.state = 1;
    D_80164B10.attr.f.x = 0x20;
    D_80164B10.attr.f.unk0_16 = 0x70;
    D_80164B10.unk4_0 = 1;
    D_80164B10.y = 0x14;
    SET_ELEM_CODE(&D_80164B10, 0);
    func_800AA02C();
    D_80164B90 = 0;
    D_80164AD4 = 0;
    D_80164B84 = 0;
    D_80164A78 = 0;
    D_80164B78 = 0xFF;
    func_80144BC0();
    D_80164E18 = 0;
    D_80160A70 = arg0;
}

/**
 * @see decomp.me (100%)
 */
void func_80142C18(s32 arg0)
{
    func_800A3938(0x78, 0x80);
    D_80164B1C.draw_handler = (void *)func_80142E88;
    D_80164B1C.attr.f.unk0_3 = 1;
    D_80164B1C.attr.f.state = 1;
    D_80164B1C.attr.f.x = 0x20;
    D_80164B1C.attr.f.unk0_16 = 0x70;
    D_80164B1C.unk4_0 = 1;
    D_80164B1C.y = 0x14;
    SET_ELEM_CODE(&D_80164B1C, 0);
    func_800AA02C();
    D_8011F428 = 2;
    D_80164B90 = 0;
    D_80164AD4 = 0;
    D_80164B84 = 0;
    D_80164A78 = 0;
    func_80144BC0();
    D_80164E18 = 0;
    D_80160A70 = arg0;
}

/**
 * @see decomp.me (100%)
 */
s32 func_80142D04(s32 *ot, s32 prim, s32 arg2, s32 arg3)
{
    RECT pos;

    switch (D_80160A70)
    {
    case 0:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80147134, 0x3C), 4, -arg2 + 0x80, -arg3, 2);
        break;
    case 2:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80147138, 0x40), 4, -arg2 + 0x80, -arg3, 2);
        break;
    case 3:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014713A, 0x42), 4, -arg2 + 0x80, -arg3, 2);
        break;
    case 1:
    case 4:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80147136, 0x3E), 4, -arg2 + 0x80, -arg3, 2);
        break;
    }
    if (D_80122988 & 0x220)
    {
        D_80164B10.attr.f.state = 0;
        func_800AA02C();
    }
    return prim;
}

/**
 * @see decomp.me (100%)
 */
s32 func_80142E88(s32 *ot, s32 prim, s32 arg2, s32 arg3)
{
    RECT pos;
    NikiPacket *p;
    s32 i;

    switch (D_80160A70)
    {
    case 0:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80147134, 0x3C), 4, -arg2 + 0x80, -arg3, 2);
        break;
    case 2:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80147138, 0x40), 4, -arg2 + 0x80, -arg3, 2);
        break;
    case 3:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014713A, 0x42), 4, -arg2 + 0x80, -arg3, 2);
        break;
    case 1:
    case 4:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80147136, 0x3E), 4, -arg2 + 0x80, -arg3, 2);
        break;
    }
    if (D_80122988 & 0x220)
    {
        D_8012298C = 0x20;
        p = (NikiPacket *)&D_80164B10;
        for (i = 0; i < 8; i++)
        {
            p->attr.word &= ~7;
            p++;
        }
        func_80067F5C(8);
        func_800AA02C();
    }
    return prim;
}

/**
 * @see decomp.me (100%)
 */
s32 func_8014303C(s32 result, s32 *ot, s32 x, s32 y, s32 adjust, s32 slot, s32 i, s32 j)
{
    RECT rect;
    s32 temp;
    s8 shade;

    if (slot == 0x7F)
    {
        return result;
    }
    rect.x = i * 0x10;
    rect.y = 0x1F2;
    rect.w = 0x10;
    rect.h = 1;
    if ((j == 1) && (slot < 2))
    {
        func_800A5638(D_80164AF0, slot);
        func_80019A34(&rect, D_80164AF0);
        func_80019788(0);
    }
    else if (slot >= 0x4F)
    {
        func_800A55E4(D_80164AF0, D_80164AD8);
        func_80019A34(&rect, D_80164AF0);
        func_80019788(0);
    }
    else
    {
        func_80019A34(&rect, (void *)((u8 *)&D_801477AC - 4 + D_801477AC[slot]));
    }

    temp = i * 3;
    rect.x = temp * 4 + 0x140;
    rect.y = 0xD0;
    rect.w = 0xC;
    rect.h = 0x30;
    func_80019A34(&rect, (void *)((u8 *)&D_801477AC + 0x1C + D_801477AC[slot]));
    ((NikiGlyphPrim *)result)->unk4 = 0x808080;
    ((u8 *)result)[3] = 9;
    ((u8 *)result)[7] = 0x2C;
    ((NikiGlyphPrim *)result)->unk18 = x;
    ((NikiGlyphPrim *)result)->unk8 = x;
    ((NikiGlyphPrim *)result)->unk12 = y;
    ((NikiGlyphPrim *)result)->unkA = y;
    ((NikiGlyphPrim *)result)->unk20 = x + adjust;
    shade = temp * 0x10;
    ((NikiGlyphPrim *)result)->unk1C = shade;
    ((NikiGlyphPrim *)result)->unkC = shade;
    shade += 0x2F;
    ((NikiGlyphPrim *)result)->unk24 = shade;
    ((NikiGlyphPrim *)result)->unk14 = shade;
    ((NikiGlyphPrim *)result)->unk15 = 0xD0;
    ((NikiGlyphPrim *)result)->unkD = 0xD0;
    ((NikiGlyphPrim *)result)->unk10 = x + adjust;
    ((NikiGlyphPrim *)result)->unk22 = y + 0x2F;
    ((NikiGlyphPrim *)result)->unk1A = y + 0x2F;
    ((NikiGlyphPrim *)result)->unk25 = 0xFF;
    ((NikiGlyphPrim *)result)->unk1D = 0xFF;
    ((NikiGlyphPrim *)result)->unkE = (i & 0x3F) | 0x7C80;
    ((NikiGlyphPrim *)result)->unk16 = 5;
    ((NikiGlyphPrim *)result)->unk0 = (((NikiGlyphPrim *)result)->unk0 & 0xFF000000) | (*ot & 0xFFFFFF);
    *ot = (*ot & 0xFF000000) | (result & 0xFFFFFF);
    return result + 0x28;
}

/**
 * @see decomp.me (100%)
 */
void func_80143284(void)
{
    D_80164A7C = 1;
}

/**
 * @see decomp.me (100%)
 */
s32 func_80143294(s32 prim, s32 *ot, s32 x, s32 y)
{
    u8 *p;
    u8 *base;
    s32 g1;
    s32 g2;
    s32 hi;
    s32 a3;

    p = (u8 *)&D_800EC3FA;
    hi = p[1] << 8;
    base = p - 0x36;
    a3 = 4;
    g1 = p[0] + (hi + (s32)base);
    if (D_80164A7C != 0)
    {
        a3 = 5;
    }
    prim = func_800A88A0(prim, ot, (void *)g1, a3, x - 0x10, y, 1);
    a3 = 4;
    g2 = base[0x38] + ((base[0x39] << 8) + (s32)base);
    if (D_80164A7C == 0)
    {
        a3 = 5;
    }
    prim = func_800A88A0(prim, ot, (void *)g2, a3, x + 8, y, 0);
    if (D_80122988 & 0xA000)
    {
        D_80164A7C ^= 1;
        func_800A3938(0x7D, 0x80);
        D_80122988 = 0;
    }
    return prim;
}

extern u16 D_80147114;
extern u16 D_80147160;
extern u16 D_80147162;
extern u16 D_8014716A;
extern u8 D_801606EC;
extern u8 D_801606F5;
extern void *jtbl_80140054[];

/**
 * @brief Draw/update dispatcher for the niki save-menu state machine.
 *
 * Dispatches on the state code in D_80164B78 (0xF3..0xFF) through the rodata
 * jump table jtbl_80140054, emitting the glyph primitives for the active
 * dialog page and advancing the state on pad input.
 *
 * @param ot Ordering-table entry the glyph primitives are linked into.
 * @param prim GPU packet write cursor.
 * @param arg2 X scroll offset subtracted from all glyph positions.
 * @param arg3 Y scroll offset subtracted from all glyph positions.
 * @return Advanced GPU packet cursor.
 *
 * @note The `switch (0)` wrapper, the static `keep[]` label-address array and
 *       `goto *jtbl_80140054[dispatch]` reproduce the original rodata jump
 *       table dispatch; same computed-goto pattern as the menu.c matches.
 * @note Verified 100.000000% (962/962 exact, gcc272_cdk) in-tree 2026-08-25;
 *       scratch history in working/func_801433BC/.
 */
s32 func_801433BC(s32 *ot, s32 prim, s32 arg2, s32 arg3)
{
    RECT pos;
    s32 dispatch;
    static void *const keep[] = {
        &&niki_f3, &&niki_f4, &&niki_f5, &&niki_f6, &&niki_f7, &&niki_f8, &&niki_f9,
        &&niki_fa, &&niki_fb, &&niki_fc, &&niki_fd, &&niki_fe, &&niki_ff
    };
    switch (0)
    {
    case 0:
        dispatch = D_80164B78 - 0xF3;
        if ((u32)dispatch >= 0xD)
        {
            goto niki_default;
        }
        goto *jtbl_80140054[dispatch];
    niki_f8:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014712C, 0x34), 4, -arg2 + 0x90, -arg3, 2);
        break;
    niki_f9:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014712C, 0x34), 4, -arg2 + 0x90, -arg3, 2);
        break;
    niki_ff:
        {
            s32 x;
            u8 *base;
            x = -arg2 + 0x90;
            base = (u8 *)&D_801470F8;
            prim = func_800A88A0(prim, ot, base + D_801470F8, 4, x, -arg3, 2);
            prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0x1E), 4, x, 0xE - arg3, 2);
            prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0xB2), 4, x, 0x1C - arg3, 2);
        }
        break;
    niki_fa:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014712C, 0x34), 4, -arg2 + 0x90, -arg3, 2);
        break;
    niki_fd:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_801470FC, 4), 4, -arg2 + 0x90, -arg3, 2);
        break;
    niki_fb:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80147108, 0x10), 4, -arg2 + 0x90, -arg3, 2);
        break;
    niki_fc:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014710A, 0x12), 4, -arg2 + 0x90, -arg3, 2);
        break;
    niki_f7:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80147160, 0x68), 4, -arg2 + 0x90, -arg3, 2);
        break;
    niki_f6:
        {
            s32 x;
            u8 *base;
            NikiPolyG4Words *g;
            s32 next;
            s32 elapsed;
            s32 extent;
            s32 color;
            s32 finalmode;

            x = -arg2 + 0x90;
            prim = func_800A88A0(prim, ot, (void *)((s32)&D_8014712A - 0x32 + D_8014712A), 4, x, -arg3, 2);
            base = (u8 *)&D_8014712A - 0x32;
            prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0x1E), 4, x, 0xE - arg3, 2);
            prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0xB2), 4, x, 0x1C - arg3, 2);

            next = prim;
            g = (NikiPolyG4Words *)prim;
            if (D_80164F08 != 0)
            {
                elapsed = func_8002054C(-1) - D_80164F10;
                if (elapsed >= 0x101)
                {
                    elapsed = 0x100;
                }
                color = 0xFFFF00;
                extent = elapsed * 0x120;
                g->unk4 = 0xFF;
                g->unkC = 0xFFFF;
                g->unk1C = 0xFF0000;
                ((u8 *)g)[3] = 8;
                g->unk14 = color;
                ((u8 *)g)[7] = 0x38;
                g->unk18 = 0;
                g->unk8 = 0;
                if (extent < 0)
                {
                    extent += 0xFF;
                }
                g->unk20 = extent >> 8;
                g->unk10 = extent >> 8;
                g->unk12 = 0;
                g->unkA = 0;
                g->unk22 = 0x2C;
                g->unk1A = 0x2C;
                g->unk0 = (g->unk0 & 0xFF000000) | (*ot & 0xFFFFFF);
                *ot = (*ot & 0xFF000000) | (prim & 0xFFFFFF);
                next = prim + 0x24;
            }
            prim = next;

            if (D_80164AD4 == 0)
            {
                if (func_80144310(D_80160A78) == 0)
                {
                    func_800A3938(0x78, 0x80);
                    D_80164B10.draw_handler = (void *)func_80142D04;
                    D_80164B10.attr.f.unk0_3 = 1;
                    D_80164B10.attr.f.state = 1;
                    D_80164B10.attr.f.x = 0x20;
                    D_80164B10.attr.f.unk0_16 = 0x70;
                    D_80164B10.unk4_0 = 1;
                    D_80164B10.y = 0x14;
                    SET_ELEM_CODE(&D_80164B10, 0);
                    func_800AA02C();
                    D_80164B90 = 0;
                    D_80164B84 = 0;
                    D_80164A78 = 0;
                    D_80164AD4 = 0;
                    D_80164B78 = 0xFF;
                    func_80144BC0();
                    finalmode = 4;
                    D_80164E18 = 0;
                    D_80160A70 = finalmode;
                    return prim;
                }
                func_800A3938(0x7B, 0x80);
                D_80164B78 = 0xF4;
                D_80164A7C = 1;
                func_800AA02C();
            }
        }
        break;
    niki_f3:
        {
            s32 x;
            s32 result;
            s32 y;
            u8 *p;
            u8 *base;
            s32 g1;
            s32 g2;
            s32 hi;
            s32 a3;
            NikiPacket *packet;
            s32 i;

            x = -arg2;
            prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014716A, 0x72), 4, x + 0x90, -arg3, 2);
            y = 0xE - arg3;
            p = (u8 *)&D_800EC3FA;
            hi = p[1] << 8;
            base = p - 0x36;
            a3 = 4;
            g1 = p[0] + (hi + (s32)base);
            if (D_80164A7C != 0)
            {
                a3 = 5;
            }
            result = func_800A88A0(prim, ot, (void *)g1, a3, x + 0x80, y, 1);
            a3 = 4;
            g2 = base[0x38] + ((base[0x39] << 8) + (s32)base);
            if (D_80164A7C == 0)
            {
                a3 = 5;
            }
            result = func_800A88A0(result, ot, (void *)g2, a3, x + 0x98, y, 0);
            if (D_80122988 & 0xA000)
            {
                D_80164A7C ^= 1;
                func_800A3938(0x7D, 0x80);
                D_80122988 = 0;
            }

            prim = result;

            if (D_80122988 & 0x40)
            {
                func_800A3938(0x78, 0x80);
                D_80164A7C = 1;
                D_80164B78 = 0xF4;
                func_800AA02C();
            }
            else if (D_80122988 & 0x220)
            {
                if (D_80164A7C != 0)
                {
                    func_800A3938(0x78, 0x80);
                    D_80164A7C = 1;
                    D_80164B78 = 0xF4;
                    func_800AA02C();
                }
                else
                {
                    func_800A3938(0x7D, 0x80);
                    packet = (NikiPacket *)&D_80164B10;
                    D_8011F428 = 2;
                    D_8012298C = 0x20;
                    for (i = 0; i < 8; i++, packet++)
                    {
                        packet->attr.f.state = 0;
                    }
                    func_80067F5C(8);
                    func_800AA02C();
                }
            }
        }
        break;
    niki_f4:
        {
            s32 x;
            s32 result;
            s32 one;
            s32 y;
            u8 *glyphbase;
            u8 *p;
            u8 *base;
            s32 g1;
            s32 g2;
            s32 hi;
            s32 a3;
            s32 count;
            s32 i;
            u8 *cursor;
            u8 *resource;
            s32 temp;

            x = -arg2;
            prim = func_800A88A0(prim, ot, (void *)((s32)&D_80147162 - 0x6A + D_80147162), 4, x + 0x90, -arg3, 2);
            glyphbase = (u8 *)&D_80147162 - 0x6A;
            prim = func_800A88A0(prim, ot, GLYPH_OFF(glyphbase, 0x70), 4, x + 0x90, 0xE - arg3, 2);

            y = 0x1C - arg3;
            p = (u8 *)&D_800EC3FA;
            hi = p[1] << 8;
            base = p - 0x36;
            a3 = 4;
            g1 = p[0] + (hi + (s32)base);
            if (D_80164A7C != 0)
            {
                a3 = 5;
            }
            one = 1;
            result = func_800A88A0(prim, ot, (void *)g1, a3, x + 0x80, y, one);
            a3 = 4;
            g2 = base[0x38] + ((base[0x39] << 8) + (s32)base);
            if (D_80164A7C == 0)
            {
                a3 = 5;
            }
            result = func_800A88A0(result, ot, (void *)g2, a3, x + 0x98, y, 0);
            if (D_80122988 & 0xA000)
            {
                D_80164A7C ^= 1;
                func_800A3938(0x7D, 0x80);
                D_80122988 = 0;
            }

            prim = result;

            if (D_80122988 & 0x40)
            {
                goto f4_accept;
            }
            if (D_80122988 & 0x220)
            {
                if (D_80164A7C != 0)
                {
                f4_accept:
                    D_80164A7C = one;
                    D_80164B78 = 0xF3;
                    func_800A3938(0x78, 0x80);
                    func_800AA02C();
                }
                else
                {
                    func_800A3938(0x7E, 0x80);
                    cursor = D_80122A08;
                    resource = D_80160A78;
                    func_80016E7C(D_80122A08, resource + 0x32E0, 0x100);
                    count = 0;
                    for (i = 0; i < 4; i++)
                    {
                        if (cursor[i * 0x40] != 0)
                        {
                            count++;
                        }
                    }
                    resource[0x197] = count;
                    temp = func_80144364(resource, count);
                    *(s32 *)(resource + 0x33E4) = 0x414E41;
                    *(s32 *)(resource + 0x33E0) = temp;
                    D_80164B90 = 1;
                    D_80164E18 = &D_801606F5;
                    D_80164B78 = 0xF5;
                }
            }
        }
        break;
    niki_f5:
        {
            s32 x;
            u8 *base;
            NikiPolyG4Words *g;
            s32 next;
            s32 elapsed;
            s32 extent;
            s32 color;
            NikiPacket *packet;
            s32 i;

            x = -arg2 + 0x90;
            prim = func_800A88A0(prim, ot, (void *)((s32)&D_80147114 - 0x1C + D_80147114), 4, x, -arg3, 2);
            base = (u8 *)&D_80147114 - 0x1C;
            prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0x1E), 4, x, 0xE - arg3, 2);
            prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0xB2), 4, x, 0x1C - arg3, 2);

            next = prim;
            g = (NikiPolyG4Words *)prim;
            if (D_80164F08 != 0)
            {
                elapsed = func_8002054C(-1) - D_80164F10;
                if (elapsed >= 0x101)
                {
                    elapsed = 0x100;
                }
                color = 0xFFFF00;
                extent = elapsed * 0x120;
                g->unk4 = 0xFF;
                g->unkC = 0xFFFF;
                g->unk1C = 0xFF0000;
                ((u8 *)g)[3] = 8;
                g->unk14 = color;
                ((u8 *)g)[7] = 0x38;
                g->unk18 = 0;
                g->unk8 = 0;
                if (extent < 0)
                {
                    extent += 0xFF;
                }
                g->unk20 = extent >> 8;
                g->unk10 = extent >> 8;
                g->unk12 = 0;
                g->unkA = 0;
                g->unk22 = 0x2C;
                g->unk1A = 0x2C;
                g->unk0 = (g->unk0 & 0xFF000000) | (*ot & 0xFFFFFF);
                *ot = (*ot & 0xFF000000) | (prim & 0xFFFFFF);
                next = prim + 0x24;
            }
            prim = next;

            if (D_80164B90 == 0)
            {
                func_800A3938(0x7A, 0x80);
                D_8012298C = 0x20;
                packet = (NikiPacket *)&D_80164B10;
                for (i = 0; i < 8; i++, packet++)
                {
                    packet->attr.f.state = 0;
                }
                func_80067F5C(8);
                D_8011F428 = 0;
            }
        }
        break;
    niki_default:
        {
            s32 x;
            u8 *base;
            s32 pos;
            s32 diff;

            x = -arg2 + 0x90;
            base = (u8 *)&D_801470F8;
            prim = func_800A88A0(prim, ot, base + D_801470F8, 4, x, -arg3, 2);
            prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0x1E), 4, x, 0xE - arg3, 2);
            prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0xB2), 4, x, 0x1C - arg3, 2);

            if (D_80164F18 == 0)
            {
                if (D_80164A78 != 0)
                {
                    return prim;
                }
                if ((u32)(*D_80164E18 - 6) < 2U)
                {
                    return prim;
                }
                if ((func_8001714C(D_800ECF7C, &D_80165018[D_80164B70][D_80164B7C], 0xC) != 0) ||
                    (D_80164D18.unkD4 != D_801227CC) ||
                    (D_80164D18.unkD6 != D_801227F4))
                {
                    D_80164B7C++;
                    if (D_80164B7C >= D_80164B78)
                    {
                        if (D_80164B78 != 0)
                        {
                            D_80164B78 = 0xF7;
                        }
                        else
                        {
                            D_80164B78 = 0xF8;
                        }
                    }
                    else
                    {
                        func_80145F68();
                        pos = D_80164B7C * 0xE;
                        diff = pos - D_80164AE0;
                        if (diff >= 0x4B)
                        {
                            D_80164AEC = pos - 0x46;
                            D_80164B88 = 4;
                        }
                        if (diff < 0)
                        {
                            D_80164AEC = pos;
                            D_80164B88 = 4;
                        }
                    }
                }
                else
                {
                    D_80164F10 = func_8002054C(-1);
                    D_80164AD4 = 1;
                    D_80164E18 = &D_801606EC;
                    D_80164B78 = 0xF6;
                }
            }
        }
        break;
    }

niki_fe:
    if (D_80164A78 != 0)
    {
        return prim;
    }
    if (D_80164B78 == 0xF6)
    {
        return prim;
    }
    if (D_80164B78 == 0xF5)
    {
        return prim;
    }
    if (D_80164B78 == 0xF4)
    {
        return prim;
    }
    if (D_80164B78 == 0xF3)
    {
        return prim;
    }

    if (D_80122988 & 0x40)
    {
        s32 *p;
        s32 i;
        s32 word;
        D_80122994 = 3;
        func_800A3938(0x78, 0x80);
        func_80067F28();
        p = (s32 *)&D_80164B10;
        i = 0;
        do
        {
            word = *p;
            if (word & 7)
            {
                *p = (((word & ~7) | 3) & ~0x78) | 0x40;
            }
            i++;
            p += 3;
        } while (i < 8);
        return prim;
    }

    if ((D_80122988 & 0xA100) && (D_80164B78 != 0xFF))
    {
        func_800A3938(0x7D, 0x80);
        D_80164B80 = 0;
        D_80164E18 = 0;
        D_80164B88 = 0;
        D_80164AEC = 0;
        D_80164AE0 = 0;
        D_80164B7C = 0;
        D_80164B78 = 0xFF;
        D_80164B84 = 0;
        D_80164B70 ^= 1;
        func_80144BC0();
        D_80164F08 = 0;
        D_80164E18 = (u8 *)&D_801606C8;
    }

    return prim;
}

/**
 * @brief Advance past a run of ASCII hexadecimal-digit characters.
 * @param arg0 Pointer to the start of the scan.
 * @return Pointer to the first byte that is not a hex digit
 *         (@c '0'-'9', @c 'a'-'f' or @c 'A'-'F').
 * @note The three unsigned range checks reproduce the original codegen; the
 *       upper-hex test only covers @c 'A'-'F' (0x41..0x46), not the full
 *       alphabet.
 * @see decomp.me (100.00%)
 */
u8 *func_801442C4(void *arg0)
{
    u8 *p = arg0;
    u32 c;

    while (1)
    {
        c = *p;
        p++;
        if ((u32)(c - '0') < 10)
            continue;
        p--;
        if (p) { p++; p--; }
        p++;
        if ((u32)(c - 'a') < 6)
            continue;
        p--;
        if (p) { p++; p--; }
        p++;
        if ((u32)(c - 'A') < 6)
            continue;
        p--;
        if (p) { p++; p--; }
        break;
    }

    return p;
}

/**
 * @brief Validate a resource blob by checksum and magic word.
 * @param base Base of the resource blob.
 * @return 1 when the stored checksum at @c base+0x33E0 matches
 *         @c func_80144364(base) and the magic word at @c base+0x33E4 equals
 *         0x414E41 ("ANA"); 0 otherwise.
 * @see decomp.me (100.00%)
 */
s32 func_80144310(u8 *base)
{
    if (*(s32 *)(base + 0x33E0) == func_80144364(base))
    {
        if (*(s32 *)(base + 0x33E4) == 0x414E41)
        {
            return 1;
        }
    }
    return 0;
}

/**
 * @brief Compute the resource-blob checksum used by func_80144310.
 * @param data Base of the resource blob.
 * @return Twice the unsigned-byte sum over the first 0x33E0 bytes, biased by
 *         the constant 0x0414E410.
 * @note Defined after its callers so no prototype is visible to them: this
 *       lets both func_80144310's one-argument call and the two-argument
 *       caller elsewhere in this file compile under K&R implicit declaration.
 * @see decomp.me (100.00%)
 */
s32 func_80144364(u8 *data)
{
    s32 sum;
    u32 i;
    u8 *p;

    p = data;
    sum = 0;
    i = 0;
    do
    {
        i += 1;
        sum += *p;
        p += 1;
    } while (i < 0x33E0U);
    return (sum * 2) + 0x0414E410;
}

/**
 * @brief Format a value as a right-aligned 6-digit shift-JIS decimal string.
 * @param out Destination byte cursor.
 * @param value Value to format (expected to fit in 6 digits).
 * @return On overflow (value >= 1000000) the cursor advanced by 6 after
 *         copying the 7-byte fixed overflow glyph run from D_80140088;
 *         otherwise the cursor left at the NUL terminator after the digits.
 * @note Each decimal digit is emitted as its two-byte shift-JIS full-width
 *       code via the @c 0x824F / @c 0x4F bias; leading zeros are suppressed
 *       until the tens place. The @c struct @c Copy7 block copy reproduces the
 *       original codegen.
 * @see decomp.me (100.00%)
 */
s8 *func_8014439C(s8 *out, s32 value)
{
    struct Copy7 { s8 data[7]; };
    extern s8 D_80140088[];
    s32 digit;
    s32 divisor;
    s32 started;
    s8 *p;

    p = out;
    divisor = 100000;
    if (value < divisor * 10)
    {
        goto format;
    }
    *(struct Copy7 *)p = *(struct Copy7 *)D_80140088;
    return p + 6;

format:
    started = 0;
    do
    {
        digit = value / divisor;
        if (digit != 0 || started != 0)
        {
            *p++ = (digit + 0x824F) >> 8;
            *p++ = digit + 0x4F;
            started = 1;
        }
        if (divisor == 1)
        {
            break;
        }
        if (divisor == 10)
        {
            started = 1;
        }
        value -= digit * divisor;
        divisor /= 10;
    } while (1);
    *p = 0;
    return p;
}

/**
 * @brief Format a value as up to @p max_chars hexadecimal digits (leading
 *        zeros suppressed) into @p out, NUL-terminated.
 * @param out Destination byte cursor.
 * @param value Value to format.
 * @param max_chars Maximum number of digits to emit.
 * @note The do{}while(0) wrappers and the goto loop reproduce the original
 *       codegen and are required to match. Defined before func_80144578 (its
 *       per-digit emitter), which stays K&R-implicit at the call site.
 * @see decomp.me (100.00%)
 */
void func_801444B0(s8 *out, s32 value, s32 max_chars)
{
    s32 nibble;
    s32 shift_index;
    s32 remaining_chars;
    s32 remaining_value;
    s32 started;
    s8 *cursor;
    s32 end_index;

    cursor = out;
    remaining_value = value;
    remaining_chars = max_chars;
    shift_index = 7;
    started = 0;
    if (remaining_chars != 0)
    {
        end_index = -1;
loop_2:
        nibble = (remaining_value >> (shift_index * 4)) & 0xF;
        do
        {
            if ((nibble != 0) || (started != 0))
            {
                func_80144578(cursor, nibble);
                cursor += 1;
                remaining_chars -= 1;
                started = 1;
                remaining_value -= nibble << (shift_index * 4);
            }
        } while (0);
        do
        {
            shift_index -= 1;
        } while (0);
        if (shift_index != end_index)
        {
            if (shift_index == 0)
            {
                started = 1;
            }
            do
            {
                if (remaining_chars != 0)
                {
                    goto loop_2;
                }
            } while (0);
        }
    }
    *cursor = 0;
}

/**
 * @brief Convert a 0-15 value to its ASCII hexadecimal digit.
 * @param out Destination byte.
 * @param value Nibble value; 0-9 -> '0'-'9', 10-15 -> 'A'-'F', else '_'.
 * @return None.
 * @see decomp.me (100.00%)
 */
void func_80144578(s8 *out, s32 value)
{
    if (value < 10)
    {
        *out = value + 0x30;
    }
    else if (value < 16)
    {
        *out = value + 0x37;
    }
    else
    {
        *out = 0x5F;
    }
}

/**
 * @brief Parse up to @p len ASCII hexadecimal digits into an unsigned value.
 * @param s Pointer to the digit run.
 * @param len Maximum number of digits to consume.
 * @return The accumulated value; parsing stops at @p len digits or the first
 *         non-hex-digit byte ('0'-'9', 'a'-'f', 'A'-'F').
 * @note The per-branch (result - bias + *s) accumulation reproduces the
 *       original codegen; the bias is 0x30/0x37/0x57 for the three digit
 *       classes.
 * @see decomp.me (100.00%)
 */
u32 func_8014459C(u8 *s, s32 len)
{
    u32 result;
    u32 tmp0;
    u32 tmp1;
    u32 tmp2;

    result = 0;
    while (((u8)(*s - '0') < 10) || ((u8)(*s - 'a') < 6) || ((u8)(*s - 'A') < 6))
    {
        if (len == 0)
        {
            break;
        }
        result <<= 4;
        if ((u8)(*s - '0') < 10)
        {
            tmp0 = result - 0x30;
            result = tmp0 + *s;
        }
        else if ((u8)(*s - 'A') < 6)
        {
            tmp1 = result - 0x37;
            result = tmp1 + *s;
        }
        else if ((u8)(*s - 'a') < 6)
        {
            tmp2 = result - 0x57;
            result = tmp2 + *s;
        }
        s++;
        len--;
    }
    return result;
}

/**
 * @brief Skip to the first non-hex-digit byte, then parse the following two
 *        hex digits into an unsigned value.
 * @param text Pointer to the scan start.
 * @param unused1 Unused (present in the original signature).
 * @param unused2 Unused (present in the original signature).
 * @return The 2-digit hex value parsed after the skipped run.
 * @note The `text--; if (text) { text++; text--; }` sequences are opaque
 *       no-ops that block cross-jump tail-merging and are required to match;
 *       do not remove them. See [[reference_crossjump_optical_noop_fix]].
 * @see decomp.me (100.00%)
 */
s32 func_80144648(u8 *text, s32 unused1, s32 unused2)
{
    u32 c;
    s32 count;
    u32 result;
    u32 tmp0;
    u32 tmp1;
    u32 tmp2;

    while (1)
    {
        c = *text;
        text++;
        if ((u32)(c - '0') < 10)
        {
            continue;
        }
        text--;
        if (text)
        {
            text++;
            text--;
        }

        text++;
        if ((u32)(c - 'a') < 6)
        {
            continue;
        }
        text--;
        if (text)
        {
            text++;
            text--;
        }

        text++;
        if ((u32)(c - 'A') < 6)
        {
            continue;
        }
        text--;
        if (text)
        {
            text++;
            text--;
        }
        break;
    }

    text++;
    count = 2;
    result = 0;
    while (((u8)(*text - '0') < 10) || ((u8)(*text - 'a') < 6) || ((u8)(*text - 'A') < 6))
    {
        if (count == 0)
        {
            break;
        }
        result <<= 4;
        if ((u8)(*text - '0') < 10)
        {
            tmp0 = result - 0x30;
            result = tmp0 + *text;
        }
        else if ((u8)(*text - 'A') < 6)
        {
            tmp1 = result - 0x37;
            result = tmp1 + *text;
        }
        else if ((u8)(*text - 'a') < 6)
        {
            tmp2 = result - 0x57;
            result = tmp2 + *text;
        }
        text++;
        count--;
    }
    return result;
}

/* ---- Consolidated from func_80144740.c ---- */

extern s32 D_80164B70;
extern s32 D_80164B78;
extern char D_800ECF7C[];
extern s32 D_80164F20[];
extern s32 D_80164EB8[];

s32 func_8001714C();
s32 func_80144648();

/**
 * @brief Parse the hex rank value out of each NIKI entry whose name matches the
 *        D_800ECF7C prefix, store it, and return the maximum.
 * @note NON-MATCHING (99.82%). Sibling of addhero func_80144570 (same body).
 *       The lone residue is a sched2 arg-order swap at the func_8001714C call:
 *       the target emits `addiu a0, %lo(D_800ECF7C)` before `li a2, 0xC`, ours
 *       after. sched_oracle classifies it as a post-allocation (sched2) reorder,
 *       not an emit-order fix; the do/while(0) fence on `pattern` is the best of
 *       the forms tried (direct-pass regresses to 98.62%).
 *       TODO: recover the exact sched2 ordering.
 * @see (99.82%)
 */
s32 func_80144740(void)
{
    s32 i;
    s32 max;
    u8 *p;
    u8 *field;
    s32 count;
    s32 acc;
    u32 tmp0;
    u32 tmp1;
    u32 tmp2;
    s32 r;

    i = 0;
    max = i;
    while (i < D_80164B78)
    {
        u8 *pattern;
        do { pattern = (u8 *)&D_800ECF7C; } while (0);
        if (func_8001714C(pattern, (u8 *)&D_80165018[D_80164B70][i], 0xC) == 0)
        {
            count = 5;
            p = (u8 *)(D_80164B70 * 0x320 + ((i << 4) + (i << 4) + (i << 3)) + (s32)D_80165018 + 0xC);
            acc = 0;
            while (((u8)(*p - '0') < 10) || ((u8)(*p - 'a') < 6) || ((u8)(*p - 'A') < 6))
            {
                if (count == 0)
                    break;
                acc <<= 4;
                if ((u8)(*p - '0') < 10)
                {
                    tmp0 = acc - 0x30;
                    acc = tmp0 + *p;
                }
                else if ((u8)(*p - 'A') < 6)
                {
                    tmp1 = acc - 0x37;
                    acc = tmp1 + *p;
                }
                else if ((u8)(*p - 'a') < 6)
                {
                    tmp2 = acc - 0x57;
                    acc = tmp2 + *p;
                }
                p++;
                count--;
            }
            field = &D_80165018[D_80164B70][i].data[0xC];
            {
                s32 addr;
                addr = D_80164B70 * 0x50 + (s32)D_80164F20;
                *(s32 *)(addr + i * 4) = acc;
            }
            r = func_80144648(field, acc, count);
            D_80164EB8[i] = r;
            if (max < r)
                max = r;
        }
        else
        {
            s32 addr;
            addr = D_80164B70 * 0x50 + (s32)D_80164F20;
            *(s32 *)(addr + i * 4) = -1;
            D_80164EB8[i] = 0;
        }
        i++;
    }
    return max;
}

/* ---- Consolidated from niki_rank_entries.c ---- */

extern s32 D_80164B78;
extern s32 D_80164E20[];
extern s32 D_80164B70;
extern s32 D_80164F20[];
extern s32 D_80164EB0;
extern s32 D_80164F14;
extern s32 D_80164EB8[];
extern char D_800ECFC4[];

s32 func_80144740();
void func_801462EC();
void func_80144BC0(void);
s32 func_8001714C();

s32 func_80144984(s32 unused0, s32 unused1, s32 unused2)
{
    s32 *row;
    s32 *elem;
    s32 *rank_ptr;
    s32 *cmp_ptr;
    s32 *inc_ptr;
    s32 *base_rank;
    s32 *ecopy;
    s32 *max_ptr;
    s32 *field_base;
    s32 *field1;
    s32 slot;
    s32 *out_ptr;
    char *ent_ptr;
    s32 t0v;
    s32 i;
    s32 s3v;
    s32 count;
    s32 handle;
    s32 less_count;
    s32 j;

    func_80144740();
    s3v = -1;
    func_801462EC();
    i = 0;
    handle = func_80144740();
    func_80144BC0();
    t0v = 1;
    if (D_80164B78 > 0)
    {
        count = D_80164B78;
        base_rank = &D_80164E20[0];
        rank_ptr = base_rank;
        slot = D_80164B70;
        field1 = D_80164F20;
        row = field1 + slot * 20;
        elem = row;
        do
        {
            if (*elem >= 0)
            {
                j = 0;
                if (i > 0)
                {
                    j += 1; j -= 1;
                }
                if (*elem >= s3v)
                {
                    *rank_ptr = t0v;
                    s3v = *elem;
                    t0v += 1;
                }
                else
                {
                    less_count = j;
                    if (i > 0)
                    {
                        ecopy = elem;
                        inc_ptr = base_rank;
                        cmp_ptr = row;
                        do
                        {
                            if (*ecopy < *cmp_ptr)
                            {
                                less_count += 1;
                                *inc_ptr += 1;
                            }
                            inc_ptr += 1;
                            j += 1;
                            cmp_ptr += 1;
                        } while (j < i);
                    }
                    {
                        s32 rank_value;
                        do { do { do { rank_value = t0v - less_count; } while (0); } while (0); } while (0);
                        *rank_ptr = rank_value;
                    }
                    t0v += 1;
                }
            }
            rank_ptr += 1;
            i += 1;
            elem += 1;
        } while (i < count);
    }
    cmp_ptr = base_rank;
    inc_ptr = row;
    D_80164EB0 = t0v;
    t0v = -1;
    i = 0;
    s3v = 0;
    if (D_80164B78 > 0)
    {
        s32 max_count;
        max_count = D_80164B78;
        slot = D_80164B70;
        field_base = D_80164F20;
        max_ptr = (s32 *)((slot * 0x50) + (s32)field_base);
        do
        {
            if (t0v < *max_ptr)
            {
                t0v = *max_ptr;
                s3v = i;
            }
            i += 1;
            max_ptr += 1;
        } while (i < max_count);
        i = 0;
    }
    D_80164F14 = t0v + 1;
    if (D_80164B78 > 0)
    {
        out_ptr = &D_80164EB8[0];
        ent_ptr = (char *)D_80165018;
    loop_20:
        if (func_8001714C(&D_800ECFC4[0], (void *)((D_80164B70 * 0x320) + (s32)ent_ptr), 8) == 0)
        {
            *out_ptr = handle + 1;
        }
        else
        {
            out_ptr += 1;
            ent_ptr += 0x28;
            i += 1;
            if (i < D_80164B78)
            {
                goto loop_20;
            }
        }
    }
    return s3v;
}

/* ---- Consolidated from niki_reset_entry_ranks.c ---- */

extern s32 D_80164EB0;
extern s32 D_80164E20[];

void func_80144BC0(void)
{
    s32 i;
    s32 val;

    D_80164EB0 = 0x28;
    val = -1;
    for (i = 14; i >= 0; i--)
    {
        D_80164E20[i] = val;
    }
}

/* ---- Consolidated from niki_known_entry_type.c ---- */

extern s32 D_80164B78;
extern s32 D_80164B70;
extern char D_800ECF7C[];
extern char D_800ECF8C[];

extern s32 func_8001714C(void *, void *, s32);

s32 func_80144BF8(void)
{
    s32 i;
    u8 *entry;

    i = 0;
    if (D_80164B78 > 0)
    {
        do
        {
            entry = (u8 *)D_80165018 + i * 0x28;
            if (func_8001714C(&D_800ECF7C, (void *)(D_80164B70 * 0x320 + (s32)entry), 0xC) == 0 ||
                func_8001714C(&D_800ECF8C, (void *)(D_80164B70 * 0x320 + (s32)entry), 0xC) == 0)
            {
                return 1;
            }
            i++;
        } while (i < D_80164B78);
    }
    return 0;
}

/* ---- Consolidated from niki_entry_blocks_reach_limit.c ---- */

typedef struct
{
    char name[20];
    s32 attr;
    s32 size;
    void *next;
    s32 head;
    char system[4];
} Entry;

extern s32 D_80164B78;
extern s32 D_80164B70;

s32 func_80144CC8(void)
{
    s32 i;
    s32 sum;
    s32 offset;

    i = 0;
    sum = 0;
    if (D_80164B78 > 0)
    {
        offset = D_80164B70 * 0x320;
        do
        {
            do {
                sum += ((Entry *)((u8 *)D_80165018 + offset))->size / 8192;
            } while (0);
            i++;
            offset += 0x28;
        } while (i < D_80164B78);
    }
    return sum >= 0xE;
}

/* ---- Consolidated from niki_fixed_prompts.c ---- */

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

extern s32 func_80016F9C(void *, void *);
extern s32 func_8001686C(void *);

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

/* ---- Consolidated from func_80144DF8.c ---- */

typedef struct {
    s32 unk0;
    s16 unk4;
    u8 pad[0x62];
} NikiLoadScratch;

extern void *jtbl_80140098[];
extern char D_800ECF9C[];
extern char D_800ECFB0[];
extern s32 D_80164B78;
extern s32 D_80164B70;
extern s32 D_80164B7C;
extern s32 D_80164B84;
extern s32 D_80164AD4;
extern s32 D_80164AE8;
extern u8 *D_80164E18;
extern s32 D_80164E1C;
extern s32 D_80164E20[];
extern s32 D_80164EB0;
extern s32 D_80164A78;
extern s32 D_80164F08;
extern s32 D_80164B94;
extern s32 D_80164EB4;
extern s32 D_80164F10;
extern s32 D_80164F0C;
extern s32 D_80164F18;
extern s32 D_80164FC0;
extern s32 D_80164FD4;
extern u8 D_80164FD8[];
extern s32 D_80164B90;
extern u8 D_80164E70[];
extern u8 D_80160A78[];
extern u8 D_801606D0[];

s32 func_80016F9C(void *, void *);
s32 func_8001680C(void *, s32);
s32 func_8001681C(s32, void *, s32);
s32 func_8001682C(s32, void *, s32);
s32 func_8001683C(s32);
s32 func_8001685C(void *, void *);
s32 func_8001686C(void *);
s32 func_800170BC(void *, void *, ...);
s32 func_8001724C(s32);
s32 func_8001725C(s32);
s32 func_8001729C(s32);
s32 func_800172AC(s32);
s32 func_8002054C(s32);
s32 func_80032174(s32, void *, s32 *);
s32 func_800342CC(s32);
s32 func_80145CBC(s32);
s32 func_80145DA4(s32);
void func_80145F68(void);
void func_80146114(void);
void func_8014616C(void);
s32 func_801461C4(void);
s32 func_80146258(void);
void func_80142B2C(s32);
void func_80142C18(s32);

static inline void niki_probe_render_two(void)
{
    NikiFileHeaderScratch p;

    memcpy(&p, &D_80140090, 6);
    ((u8 *)&p)[2] += *(u8 *)&D_80164B70;
    func_80016F9C(&p, &D_800ECF9C);
    func_8001686C(&p);

    memcpy(&p, &D_80140090, 6);
    ((u8 *)&p)[2] += *(u8 *)&D_80164B70;
    func_80016F9C(&p, &D_800ECFB0);
    func_8001686C(&p);
}

s32 func_80144DF8(void)
{
    NikiLoadScratch buf;
    s32 status0;
    s32 status1;
    s32 phase_result;
    s32 wait_attempts;
    s32 poll_result;
    s32 poll_result20;
    s32 rank_index;
    s32 rank_value;
    s32 dispatch;
    static void *const keep[] = {
        &&cl_case_0, &&cl_case_1, &&cl_case_2, &&cl_case_3,
        &&cl_case_4, &&cl_case_5, &&cl_case_6, &&block_return,
        &&cl_case_8, &&cl_case_9, &&cl_case_10, &&block_return,
        &&block_return, &&block_return, &&block_return, &&cl_case_15,
        &&cl_case_16, &&cl_case_17, &&cl_case_18, &&cl_case_19,
        &&cl_case_20, &&block_return, &&block_return, &&block_return,
        &&cl_case_24, &&cl_case_25, &&cl_case_26, &&cl_case_27,
        &&cl_case_28, &&block_return, &&cl_case_30
    };

    memcpy(&buf, &D_80140090, 6);
    phase_result = 1;
    ((u8 *)&buf)[2] += *(u8 *)&D_80164B70;

    if (D_80164E18 == NULL)
    {
        return phase_result;
    }

    switch (0)
    {
    case 0:
        dispatch = *D_80164E18;
        if ((u32)dispatch >= 0x1F)
        {
            goto block_return;
        }
        goto *jtbl_80140098[dispatch];

    cl_case_1:
        phase_result = 3;
        func_8001729C(D_80164B70);
        func_8001724C(D_80164B70 * 0x10);
        D_80164E18 = D_80164E18 + 1;
        goto block_return;

    cl_case_2:
        poll_result = func_801461C4();
        if (poll_result >= 3)
        {
            goto c2_ge3;
        }
        if (poll_result > 0)
        {
            goto c2_pos;
        }
        if (poll_result == 0)
        {
            goto block_increment;
        }
        return phase_result;
    c2_ge3:
        if (poll_result == 3)
        {
            goto c2_eq3;
        }
        return phase_result;
    c2_pos:
        phase_result = 4;
        D_80164B84 = 0;
        D_80164B78 = 0xFD;
        D_80164E18 = D_80164E18 + 1;
        goto block_return;
    c2_eq3:
        D_80164EB0 = 0x28;
        rank_value = -1;
        for (rank_index = 14; rank_index >= 0; rank_index--)
        {
            D_80164E20[rank_index] = rank_value;
        }
        goto block_status_ff;

    cl_case_3:
        func_80146114();
        goto block_increment;

    cl_case_4:
        do
        {
            poll_result = func_80146258();
        } while (poll_result == -1);
        if (poll_result == 0)
        {
            goto block_increment;
        }
        if (poll_result < 0)
        {
            return phase_result;
        }
        if (poll_result >= 4)
        {
            return phase_result;
        }
        phase_result = 4;
        goto block_status_fd;

    cl_case_5:
        func_8014616C();
        goto block_increment;

    cl_case_6:
        niki_probe_render_two();
        D_80164F18 = 1;
        if (func_80145CBC(D_80164B70) == 0)
        {
            phase_result = 2;
            D_80164E18 = NULL;
            D_80164B78 = 0xF8;
            D_80164F18 = 0;
            goto block_return;
        }
        wait_attempts = 0;
        D_80164E18 = D_80164E18 + 1;
        do
        {
            if (func_80145DA4(D_80164B70) == 0)
            {
                if (D_80164AE8 != 0)
                {
                    D_80164B7C = 0;
                }
                D_80164F18 = 0;
                if (D_80164B78 == 0xF8)
                {
                    return phase_result;
                }
                if (D_80164B78 == 0xFA)
                {
                    goto block_return;
                }
                func_80145F68();
                goto block_return;
            }
            wait_attempts = wait_attempts + 1;
        } while (wait_attempts < 0x14);
        goto block_return;

    cl_case_8:
        phase_result = 3;
        func_8001729C(D_80164B70);
        func_800172AC(D_80164B70 * 0x10);
        D_80164E18 = D_80164E18 + 1;
        goto block_return;

    cl_case_9:
        phase_result = 3;
        func_8001729C(D_80164B70);
        func_8001725C(D_80164B70 * 0x10);
        D_80164F0C = 0x10;
        D_80164FC0 = 0x10;
        D_80164E18 = D_80164E18 + 1;
        goto block_return;

    cl_case_0:
        phase_result = 2;
        D_80164B90 = 0;
        goto block_return;

    cl_case_10:
        func_80016F9C(&buf, (u8 *)D_80165018 + (D_80164B70 * 0x320) + (D_80164B7C * 0x28));
        wait_attempts = 0;
        func_8001729C(D_80164B70);
        do
        {
            poll_result = func_8001686C(&buf);
            wait_attempts = wait_attempts + 1;
            if (poll_result != 0)
            {
                break;
            }
        } while (wait_attempts < 0x14);
        goto block_increment;

    cl_case_15:
        poll_result = func_801461C4();
        if (poll_result >= 3)
        {
            goto c15_ge3;
        }
        if (poll_result > 0)
        {
            goto c15_pos;
        }
        if (poll_result == 0)
        {
            goto block_increment;
        }
        goto block_return;
    c15_ge3:
        if (poll_result == 3)
        {
            goto c15_eq3;
        }
        goto block_return;
    c15_pos:
        D_80164FC0 = D_80164FC0 - 1;
        if (D_80164FC0 != 0)
        {
            goto block_reissue;
        }
        phase_result = 4;
    block_status_fd:
        D_80164B84 = 0;
        D_80164B78 = 0xFD;
        goto block_return;
    c15_eq3:
        D_80164F0C = D_80164F0C - 1;
        if (D_80164F0C == 0)
        {
            goto c15_d70zero;
        }
    block_reissue:
        func_8001729C(D_80164B70);
        func_800172AC(D_80164B70 * 0x10);
        func_8001729C(D_80164B70);
        func_8001725C(D_80164B70 * 0x10);
        goto block_return;
    c15_d70zero:
        phase_result = 5;
        D_80164B78 = 0xFC;
        D_80164E18 = D_801606D0;
        goto block_return;

    cl_case_16:
        do
        {
            poll_result = func_80146258();
        } while (poll_result == -1);
        goto block_increment;

    cl_case_17:
        D_80164A78 = 1;
        D_80164B84 = 0;
        func_8001729C(D_80164B70);
        D_80164E1C = func_8001680C(D_80164E70, 0x8001);
        if (D_80164E1C == -1)
        {
            goto block_return;
        }
        func_80146114();
        func_8001729C(D_80164B70);
        if (func_8001681C(D_80164E1C, &D_80164B98,
                           D_80164EB4 != 0 ? 0x280 : 0x80) == -1)
        {
            func_8001683C(D_80164E1C);
            goto block_return;
        }
        goto block_increment;

    cl_case_18:
        poll_result = func_801461C4();
        if (poll_result == 0)
        {
            D_80164A78 = 0;
            D_80164B84 = 1;
            D_80164E18 = D_80164E18 + 1;
            func_8001683C(D_80164E1C);
            goto block_return;
        }
        if (poll_result == -1)
        {
            goto block_return;
        }
        D_80164A78 = 0;
        func_8001683C(D_80164E1C);
    block_status_ff:
        D_80164B78 = 0xFF;
        D_80164E18 = (u8 *)&D_801606C8;
        goto block_return;

    cl_case_19:
        D_80164AD4 = 1;
        D_80164F08 = 1;
        D_80164F10 = func_8002054C(-1);
        func_8001729C(D_80164B70);
        D_80164E1C = func_8001680C(D_80164E70, 0x8001);
        func_80146114();
        func_8001729C(D_80164B70);
        if (func_8001681C(D_80164E1C, D_80160A78, 0x4000) == -1)
        {
            D_80164B94 = D_80164B94 - 1;
            if (D_80164B94 == 0)
            {
            block_dialog_read:
                func_80142B2C(1);
                goto block_return;
            }
            goto block_return;
        }
        goto block_increment;

    cl_case_20:
        poll_result20 = func_801461C4();
        if (poll_result20 == 0)
        {
            D_80164AD4 = 0;
            D_80164E18 = D_80164E18 + 1;
            func_8001683C(D_80164E1C);
            goto block_return;
        }
        if (poll_result20 < 0)
        {
            goto block_return;
        }
        if (poll_result20 >= 4)
        {
            goto block_return;
        }
        D_80164B94 = D_80164B94 - 1;
        if (D_80164B94 == 0)
        {
            D_80164F08 = 0;
            goto block_dialog_read;
        }
        goto block_decrement_step;

    cl_case_24:
        wait_attempts = 0;
        do
        {
            if (func_800342CC(D_80164B70 * 0x10) == 1)
            {
                break;
            }
            func_8002054C(0);
            wait_attempts = wait_attempts + 1;
        } while (wait_attempts < 0x14);
        if (wait_attempts != 0x14)
        {
            func_80032174(0, &status0, &status1);
            if (status1 == 0)
            {
                goto block_increment;
            }
        }
        func_80142B2C(3);
        goto block_return;

    cl_case_27:
        D_80164AD4 = 1;
        D_80164F08 = 1;
        D_80164F10 = func_8002054C(-1);
        func_8001729C(D_80164B70);
        D_80164E1C = func_8001680C(D_80164E70, 0x8001);
        func_80146114();
        func_8001729C(D_80164B70);
        if (func_8001681C(D_80164E1C, D_80160A78, 0x4000) == -1)
        {
            func_8001683C(D_80164E1C);
            D_80164B94 = D_80164B94 - 1;
            if (D_80164B94 == 0)
            {
            block_dialog_write_read:
                func_80142C18(1);
                goto block_return;
            }
            goto block_return;
        }
        goto block_increment;

    cl_case_28:
        poll_result20 = func_801461C4();
        if (poll_result20 == 0)
        {
            D_80164AD4 = 0;
            D_80164E18 = D_80164E18 + 1;
            func_8001683C(D_80164E1C);
            goto block_return;
        }
        if (poll_result20 < 0)
        {
            goto block_return;
        }
        if (poll_result20 >= 4)
        {
            goto block_return;
        }
        D_80164B94 = D_80164B94 - 1;
        if (D_80164B94 == 0)
        {
            func_8001683C(D_80164E1C);
            D_80164F08 = 0;
            func_80142C18(1);
            return phase_result;
        }
        goto block_close_decrement;

    cl_case_30:
        D_80164B94 = 5;
        D_80164E18 = D_80164E18 + 1;
        goto block_return;

    cl_case_25:
        if (D_80164FD4 == 0)
        {
            wait_attempts = 0;
            do
            {
                if (func_8001686C(D_80164E70) != 0)
                {
                    break;
                }
                wait_attempts = wait_attempts + 1;
            } while (wait_attempts < 0x14);
        }
        func_80016F9C(&buf, D_800ECF9C);
        func_8001729C(D_80164B70);
        D_80164E1C = func_8001680C(&buf, 0x20200);
        if (D_80164E1C != -1)
        {
            goto block_write_opened;
        }
        func_8001683C(-1);
        wait_attempts = 0;
        do
        {
            if (func_8001686C(&buf) != 0)
            {
                break;
            }
            wait_attempts = wait_attempts + 1;
        } while (wait_attempts < 0x14);
    block_write_retry:
        D_80164B94 = D_80164B94 - 1;
        if (D_80164B94 == 0)
        {
        block_dialog_write:
            func_80142C18(0);
            goto block_return;
        }
        goto block_return;

    block_write_opened:
        func_8001683C(D_80164E1C);
        func_800170BC(D_80164FD8, &buf);
        func_8001729C(D_80164B70);
        D_80164E1C = func_8001680C(D_80164FD8, 0x8002);
        func_80146114();
        D_80164F08 = 1;
        D_80164F10 = func_8002054C(-1);
        func_8001729C(D_80164B70);
        if (func_8001682C(D_80164E1C, D_80160A78, 0x4000) == -1)
        {
            func_8001683C(D_80164E1C);
            wait_attempts = 0;
            do
            {
                if (func_8001686C(D_80164FD8) != 0)
                {
                    break;
                }
                wait_attempts = wait_attempts + 1;
            } while (wait_attempts < 0x14);
            goto block_write_retry;
        }
        goto block_increment;

    block_increment:
        D_80164E18 = D_80164E18 + 1;
        goto block_return;

    cl_case_26:
        poll_result20 = func_801461C4();
        if (poll_result20 != 0)
        {
            if (poll_result20 < 0)
            {
                goto block_return;
            }
            if (poll_result20 >= 4)
            {
                goto block_return;
            }
            goto block_case26_retry;
        }
        if (D_80164FD4 != 0)
        {
            func_8001729C(D_80164B70);
            wait_attempts = 0;
            do
            {
                if (func_8001686C(D_80164E70) != 0)
                {
                    break;
                }
                wait_attempts = wait_attempts + 1;
            } while (wait_attempts < 0x14);
        }
        func_8001729C(D_80164B70);
        wait_attempts = 0;
        do
        {
            if (func_8001685C(D_80164FD8, D_80164E70) != 0)
            {
                break;
            }
            wait_attempts = wait_attempts + 1;
        } while (wait_attempts < 0x14);
        D_80164B90 = 0;
        D_80164E18 = D_80164E18 + 1;
        func_8001683C(D_80164E1C);
        goto block_return;

    }

block_case26_retry:
    D_80164B94 = D_80164B94 - 1;
    if (D_80164B94 == 0)
    {
        goto block_case26_exhausted;
    }

block_close_decrement:
    func_8001683C(D_80164E1C);
block_decrement_step:
    D_80164E18 = D_80164E18 - 1;
    goto block_return;

block_case26_exhausted:
    D_80164F08 = 0;
    func_80142C18(0);
    wait_attempts = 0;
    do
    {
        if (func_8001686C(D_80164FD8) != 0)
        {
            break;
        }
        wait_attempts = wait_attempts + 1;
    } while (wait_attempts < 0x14);

block_return:
    return phase_result;
}

/* ---- Consolidated from niki_stream_reset.c ---- */

extern s32 D_80164B70;
extern u8 D_801606D0[];

extern s32 func_8001724C(s32);
extern s32 func_8001729C(s32);
extern void func_80146114(void);
extern s32 func_801461C4(void);

/** @see decomp.me (100.00%) */
void func_80145994(void)
{
    func_8001729C(D_80164B70);
    func_80146114();
    func_8001724C(D_80164B70 * 0x10);
    D_80164E18 = D_801606D0;
}

/** @see decomp.me (100.00%) */
s32 func_801459E8(void)
{
    s32 busy_slot;

    busy_slot = func_801461C4();
    if (busy_slot != -1)
    {
        func_8001729C(D_80164B70);
        func_8001724C(D_80164B70 * 0x10);
    }
    return busy_slot;
}

/* ---- Consolidated from niki_init_stream_handles.c ---- */

extern s32 D_80164F08;
extern s32 D_80164F18;
extern s32 D_80164FC4;
extern s32 D_80164FC8;
extern s32 D_80164FCC;
extern s32 D_80164FD0;
extern s32 D_80165658;
extern s32 D_8016565C;
extern s32 D_80165660;
extern s32 D_80165664;

extern void func_800158E0(void);
extern s32 func_800167AC(s32, s32, s32, s32);
extern void func_800167DC(s32);
extern void func_800167EC(void);
extern void func_800167FC(void);

/** @see decomp.me (100.00%) */
void func_80145A40(void)
{
    func_800158E0();
    func_800167EC();
    D_80164FC4 = func_800167AC(0xF4000001, 4, 0x2000, 0);
    D_80164FC8 = func_800167AC(0xF4000001, 0x8000, 0x2000, 0);
    D_80164FCC = func_800167AC(0xF4000001, 0x100, 0x2000, 0);
    D_80164FD0 = func_800167AC(0xF4000001, 0x2000, 0x2000, 0);
    D_80165658 = func_800167AC(0xF0000011, 4, 0x2000, 0);
    D_8016565C = func_800167AC(0xF0000011, 0x8000, 0x2000, 0);
    D_80165660 = func_800167AC(0xF0000011, 0x100, 0x2000, 0);
    D_80165664 = func_800167AC(0xF0000011, 0x2000, 0x2000, 0);
    func_800167DC(D_80164FC4);
    func_800167DC(D_80164FC8);
    func_800167DC(D_80164FCC);
    func_800167DC(D_80164FD0);
    func_800167DC(D_80165658);
    func_800167DC(D_8016565C);
    func_800167DC(D_80165660);
    func_800167DC(D_80165664);
    func_800167FC();
    D_80164F08 = 0;
    D_80164F18 = 0;
}

/* ---- Consolidated from niki_shutdown_handles.c ---- */

extern s32 D_80164FC4;
extern s32 D_80164FC8;
extern s32 D_80164FCC;
extern s32 D_80164FD0;
extern s32 D_80165658;
extern s32 D_8016565C;
extern s32 D_80165660;
extern s32 D_80165664;

extern void func_800158E0(void);
extern void func_800167BC(s32);
extern void func_800167EC(void);
extern void func_800167FC(void);

/** @see decomp.me (100.00%) */
void func_80145C0C(void)
{
    func_800158E0();
    func_800167EC();
    func_800167BC(D_80164FC4);
    func_800167BC(D_80164FC8);
    func_800167BC(D_80164FCC);
    func_800167BC(D_80164FD0);
    func_800167BC(D_80165658);
    func_800167BC(D_8016565C);
    func_800167BC(D_80165660);
    func_800167BC(D_80165664);
    func_800167FC();
}

/* ---- Consolidated from niki_begin_entry_scan.c ---- */

typedef struct EntryHeader7 {
    s32 unk0;
    s16 unk4;
    s8 unk6;
    u8 pad[9];
} EntryHeader7;

extern EntryHeader7 D_80140114;
extern s32 D_80164AE0;
extern s32 D_80164AEC;
extern s32 D_80164B78;
extern s32 D_80164B7C;
extern s32 D_80164B88;

/** @see decomp.me (100.00%) */
s32 func_80145CBC(s32 page)
{
    EntryHeader7 buf;

    memcpy(&buf, &D_80140114, 7);
    D_80164B7C = 0;
    D_80164B88 = 0;
    D_80164AEC = 0;
    D_80164AE0 = 0;
    D_80164B78 = 0;
    ((u8 *)&buf)[2] += page;
    if (func_80016BCC(&buf, (u8 *)D_80165018 + page * 0x320) != 0)
    {
        func_800B0170((u8 *)D_80165018 + page * 0x320 + D_80164B78 * 0x28);
        D_80164B78 += 1;
        return 1;
    }
    return 0;
}

/* ---- Consolidated from func_80145DA4.c ---- */

extern s32 D_80164AE8;
extern s32 D_80164B78;
extern s32 D_80164B70;
extern s32 D_80164B7C;
extern s32 D_80164F14;
extern s32 D_80164FD4;

/**
 * @brief Directory entry as seen by the entry-scan pass: only the size field
 *        at offset 0x18 is used here.
 */
typedef struct NikiDirEntry {
    u8 pad[0x18];
    s32 size;
} NikiDirEntry;

/**
 * @brief Advance one step of the NIKI entry load scan for the given page.
 *
 * If the current entry's streamed resource is ready, hand it off and advance
 * the entry index. Otherwise poll the stream, and once the scan has stalled or
 * completed, total the number of loaded blocks across all scanned entries and
 * either rank/commit a selection or defer, depending on how much has loaded.
 *
 * @param page Page index whose entry block is being scanned.
 * @return 1 if an entry was consumed this step, 0 otherwise.
 * @see decomp.me (100.00%)
 */
s32 func_80145DA4(s32 page)
{
    s32 i;
    s32 sum;
    s32 offset;
    s32 selected;
    s32 page_offset;
    s32 count;
    s32 cond;

    page_offset = page * 0x320;
    if (func_8001684C((void *)((u8 *)D_80165018 + page_offset + D_80164B78 * 0x28)) != 0)
    {
        func_800B0170((void *)((u8 *)D_80165018 + page_offset + D_80164B78 * 0x28));
        D_80164B78 += 1;
        return 1;
    }

    func_800AA02C();
    if ((D_80164AE8 == 0) && (func_80144BF8() == 0))
    {
        D_80164B78 = 0xF8;
    }
    else
    {
        i = 0;
        sum = 0;
        D_80164FD4 = 0;
        count = D_80164B78;
        if (count > 0)
        {
            u8 *entries;
            do { entries = (u8 *)D_80165018; } while (0);
            offset = D_80164B70 * 0x320;
            do
            {
                sum += ((NikiDirEntry *)(offset + (s32)entries))->size / 8192;
                i++;
                offset += 0x28;
            } while (i < count);
        }
        cond = sum >= 0xE;
        if (cond != 0)
        {
            selected = func_80144984(sum, i, count);
            if (func_80144BF8() == 0)
            {
                D_80164B78 = 0xFA;
                D_80164F14 = 0;
            }
            else
            {
                D_80164B7C = selected;
                func_80140CC8();
            }
        }
        else
        {
            D_80164FD4 = 1;
            selected = func_80144984(sum, i, count);
            if (func_80144BF8() == 0)
            {
                D_80164B7C = 0;
                func_80140CC8();
                D_80164F14 = 0;
            }
            else
            {
                D_80164B7C = selected;
                func_80140CC8();
            }
        }
    }
    return 0;
}

/* ---- Consolidated from niki_commit_selected_entry.c ---- */

typedef struct FileHeader100 {
    s32 unk0;
    s16 unk4;
    u8 pad[0xFA];
} FileHeader100;

extern s32 D_80164B78;
extern s32 D_80164B70;
extern s32 D_80164B7C;
extern s32 D_80164B84;
extern s32 D_80164A78;
extern s32 D_80164EB4;
extern char D_800ECFC4[];
extern char D_800ECF7C[];
extern u8 D_80164E70[];
extern u8 *D_80164E18;
extern u8 D_801606E0[];

/** @see decomp.me (100.00%) */
void func_80145F68(void)
{
    FileHeader100 local;
    u8 *p;

    if (D_80164B78 == 0)
    {
        D_80164B84 = 3;
        return;
    }
    {
        s32 term1;
        s32 term2;
        term1 = D_80164B70 * 0x320;
        term2 = (D_80164B7C * 0x28) + (s32)D_80165018;
        if (func_8001714C(&D_800ECFC4[0], (void *)(term1 + term2), 8) == 0)
        {
            D_80164B84 = 2;
            return;
        }
    }
    memcpy(&local, &D_80140090, 6);
    p = (u8 *)&local;
    {
        s32 term1;
        s32 term2;
        term1 = D_80164B70 * 0x320;
        term2 = (D_80164B7C * 0x28) + (s32)D_80165018;
        func_80016F9C(p, (void *)(term1 + term2));
    }
    {
        s32 slot;
        s32 value;
        value = *((u8 *)&local + 2);
        slot = (u8)D_80164B70;
        D_80164B84 = 0;
        value += slot;
        *((u8 *)&local + 2) = value;
        func_800170BC(&D_80164E70[0], p, slot);
    }
    D_80164E18 = &D_801606E0[0];
    {
        s32 term1;
        s32 term2;
        term1 = D_80164B70 * 0x320;
        term2 = (D_80164B7C * 0x28) + (s32)D_80165018;
        if (func_8001714C(&D_800ECF7C[0], (void *)(term1 + term2), 0xC) == 0)
            D_80164EB4 = 1;
        else
            D_80164EB4 = 0;
    }
    D_80164A78 = 1;
}

/* ---- Consolidated from niki_handles.c ---- */

extern s32 D_80164FC4;
extern s32 D_80164FC8;
extern s32 D_80164FCC;
extern s32 D_80164FD0;
extern s32 D_80165658;
extern s32 D_8016565C;
extern s32 D_80165660;
extern s32 D_80165664;

extern s32 func_800167CC(s32);

/** @see decomp.me (100.00%) */
void func_80146114(void)
{
    func_800167CC(D_80164FC4);
    func_800167CC(D_80164FC8);
    func_800167CC(D_80164FCC);
    func_800167CC(D_80164FD0);
}

/** @see decomp.me (100.00%) */
void func_8014616C(void)
{
    func_800167CC(D_80165658);
    func_800167CC(D_8016565C);
    func_800167CC(D_80165660);
    func_800167CC(D_80165664);
}

/** @see decomp.me (100.00%) */
s32 func_801461C4(void)
{
    if (func_800167CC(D_80164FC4) == 1)
    {
        return 0;
    }
    if (func_800167CC(D_80164FC8) == 1)
    {
        return 1;
    }
    if (func_800167CC(D_80164FCC) == 1)
    {
        return 2;
    }
    if (func_800167CC(D_80164FD0) == 1)
    {
        return 3;
    }
    return -1;
}

/** @see decomp.me (100.00%) */
s32 func_80146258(void)
{
    if (func_800167CC(D_80165658) == 1)
    {
        return 0;
    }
    if (func_800167CC(D_8016565C) == 1)
    {
        return 1;
    }
    if (func_800167CC(D_80165660) == 1)
    {
        return 2;
    }
    if (func_800167CC(D_80165664) == 1)
    {
        return 3;
    }
    return -1;
}

/* ---- Consolidated from func_801462EC.c ---- */

#define NENT 20
extern s32 D_80164B70;
extern s32 D_80164B78;
extern s32 D_80164EB8[];
extern char D_800ECF7C[];
extern char D_800ECF8C[];
extern char D_800ECFC4[];
extern s32 func_8001714C();
extern void func_80016E7C();

void func_801462EC(void)
{
    NikiEntry28 sorted[NENT];
    s32 out = 0;
    s32 group = 0;
    s32 i;
    do {
        i = 0;
        if (i < D_80164B78) {
            do {
                if (D_80164EB8[i] == group &&
                    func_8001714C(D_800ECF7C, &D_80165018[D_80164B70][i], 0xC) == 0) {
                    func_80016E7C(&D_80165018[D_80164B70][i], &sorted[out], 0x28);
                    out++;
                }
                i++;
            } while (i < D_80164B78);
        }
        group++;
    } while (group < 8);

    group = 0;
    do {
        i = 0;
        if (i < D_80164B78) {
            do {
                if (D_80164EB8[i] == group &&
                    func_8001714C(D_800ECF8C, &D_80165018[D_80164B70][i], 0xC) == 0) {
                    func_80016E7C(&D_80165018[D_80164B70][i], &sorted[out], 0x28);
                    out++;
                }
                i++;
            } while (i < D_80164B78);
        }
        group++;
    } while (group < 8);

    i = 0;
    if (D_80164B78 > 0) {
        do {
            if (func_8001714C(D_800ECFC4, &D_80165018[D_80164B70][i], 8) == 0) {
                func_80016E7C(&D_80165018[D_80164B70][i], &sorted[out], 0x28);
                out++;
            }
            i++;
        } while (i < D_80164B78);
    }

    if (*(volatile s32 *)&D_80164B78 > 0) {
        i = 0;
        do {
            if (func_8001714C(D_800ECF7C, &D_80165018[D_80164B70][i], 0xC) != 0 &&
                func_8001714C(D_800ECF8C, &D_80165018[D_80164B70][i], 0xC) != 0 &&
                func_8001714C(D_800ECFC4, &D_80165018[D_80164B70][i], 8) != 0) {
                func_80016E7C(&D_80165018[D_80164B70][i], &sorted[out], 0x28);
                out++;
            }
            i++;
        } while (i < D_80164B78);
    }

    i = 0;
    if (D_80164B78 > 0) {
        do {
            func_80016E7C(&sorted[i], &D_80165018[D_80164B70][i], 0x28);
            i++;
        } while (i < D_80164B78);
    }
}

/* ---- Consolidated from niki_draw_signed_decimal.c ---- */

extern u16 D_80160A34[];
extern s32 func_801469C0(s32, s32 *, u8 *, s32, s32, s32, s32);

s32 func_8014671C(s32 prim, s32 *ot, s32 value, s32 x, s32 y, s32 palette, s32 alignment)
{
    u16 buf[7];
    s32 first_digit;
    s32 magnitude;
    s32 negative;

    magnitude = value;
    if (magnitude < 0)
    {
        magnitude = -magnitude;
        negative = 1;
    }
    else
    {
        negative = 0;
    }
    buf[1] = D_80160A34[magnitude / 10000];
    buf[2] = D_80160A34[(magnitude % 10000) / 1000];
    buf[3] = D_80160A34[(magnitude % 1000) / 100];
    buf[4] = D_80160A34[(magnitude % 100) / 10];
    buf[5] = D_80160A34[magnitude % 10];

    first_digit = 1;
    buf[6] = 0;

    while (first_digit < 5 && buf[first_digit] == 0x4F82)
    {
        first_digit++;
    }

    if (negative != 0)
    {
        first_digit--;
        buf[first_digit] = 0x5B81;
    }
    prim = func_801469C0(prim, ot, (u8 *)&buf[first_digit], x, y, palette, alignment);
    return prim;
}

/* ---- Consolidated from niki_nibble_pair.c ---- */

extern u16 D_80160A4C[];

void func_8014693C(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5)
{
    u16 pair[3];
    s32 row;
    s32 adjusted;
    s32 off;
    u16 *base;

    adjusted = arg2;
    if (arg2 < 0)
        adjusted = arg2 + 15;
    row = adjusted >> 4;
    off = row * 2;
    base = D_80160A4C;
    pair[0] = *(u16 *)((u8 *)base + off);
    off = (arg2 - row * 16) * 2;
    pair[1] = *(u16 *)((u8 *)base + off);
    pair[2] = 0;
    func_801469C0(arg0, arg1, pair, arg3, arg4, 0, arg5);
}

/* ---- Consolidated from niki_text_render.c ---- */

#define GLYPH_CACHE_SLOTS 0x100
#define GLYPH_CACHE_COLUMNS 16
#define GLYPH_CACHE_ROW_MASK 0xF0
#define GLYPH_RASTER_BYTES 0x80
#define GPU_ADDR_MASK 0xFFFFFF
#define GPU_TAG_HIGH_MASK 0xFF000000

typedef struct
{
    u32 tag;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    u8 u0, v0;
    u16 clut;
} NikiSprt16;

typedef struct
{
    unsigned addr : 24;
    unsigned len : 8;
    u8 r0, g0, b0, code;
} NikiPrimTag;

#define setlen(prim, length) (((NikiPrimTag *)(prim))->len = (u8)(length))
#define setaddr(prim, address) (((NikiPrimTag *)(prim))->addr = (u32)(address))
#define setcode(prim, command) (((NikiPrimTag *)(prim))->code = (u8)(command))
#define getaddr(prim) ((u32)(((NikiPrimTag *)(prim))->addr))
#define addPrim(ordering_table, prim) \
    (setaddr((prim), getaddr(ordering_table)), setaddr((ordering_table), (prim)))

typedef struct
{
    s32 tag;
    s32 word4;
    s16 x0;
    s16 y0;
    s16 unkC;
    u16 unkE;
} GenericGpuPacket;

typedef union
{
    u32 raw;
    struct
    {
        u16 code;
        u16 flags;
    } data;
} GlyphCacheEntry;

typedef struct
{
    NikiSprt16 packet;
    u32 padding;
} GlyphSprite;

extern s32 D_8016DA78;
extern s32 D_80165668;
extern s32 D_8016DA7C;
extern GlyphCacheEntry D_8016D678[];
extern u8 *D_80165674;
extern s32 D_8016566C;
extern s32 D_80165670;

extern s32 func_8001687C(s32);
extern void func_80019A34(RECT *, void *);
extern void func_80019788(s32);

s32 func_801469C0(s32 prim, s32 *ot, u8 *text, s32 x, s32 y, s32 palette, s32 alignment);
s32 func_80146B90(s32 prim, s32 *ot, s32 character_code, s32 palette);
s32 func_80146DB0(GlyphSprite *sprite, s32 *ot, s32 cache_slot, s32 palette);

s32 func_801469C0(s32 prim, s32 *ot, u8 *text, s32 x, s32 y, s32 palette, s32 alignment)
{
    u8 *cursor;
    s32 count;
    u16 code;
    u8 *scan;

    cursor = text;
    count = 0;
    if (*cursor >= 0x20)
    {
        scan = cursor;
        do
        {
            code = *scan;
            if (code >= 0x80)
            {
                scan++;
            }
            scan++;
            count++;
        } while (*scan >= 0x20);
    }

    switch (alignment)
    {
    case 1:
        x -= count * 0x10;
        break;
    case 2:
        x -= count * 8;
        break;
    case 0:
    default:
        break;
    }
    D_80165668 = x;
    D_8016DA78 = x;
    D_8016DA7C = y;

    while (1)
    {
        u32 lead = *cursor;

        if ((u8)lead == 0x20)
        {
            cursor++;
            D_8016DA78 += 0x10;
            continue;
        }
        if ((u8)lead >= 0x80)
        {
            code = cursor[0];
            code = (code << 8) | cursor[1];
            cursor += 2;
        }
        else
        {
            if ((u8)lead < 0x20)
            {
                break;
            }
            if ((u32)(lead - 0x30) < 0x50)
            {
                code = *cursor - 0x7DE1;
                cursor++;
            }
            else
            {
                code = *cursor - 0x7AE1;
                cursor++;
            }
        }
        prim = func_80146B90(prim, ot, code, palette);
    }

    setlen(prim, 1);
    ((GenericGpuPacket *)prim)->word4 = 0xE1000005;
    addPrim(ot, prim);
    return prim + 8;
}

s32 func_80146B90(s32 prim, s32 *ot, s32 character_code, s32 palette)
{
    GlyphCacheEntry *entry;
    u8 *font_data;
    s32 font_address;
    u32 requested_code;
    s32 slot;
    s32 high_pixel_set;
    s32 code;
    RECT rect;

    u8 *raster;
    s32 color_index;
    s32 high_nibble_color;
    s32 row;
    s32 source_byte;

    u16 mask;
    volatile u8 *raster_byte;
    u8 packed_pixels;

    code = character_code;
    slot = 0;
    requested_code = code & 0xFFFF;
    entry = D_8016D678;

    while (slot < GLYPH_CACHE_SLOTS)
    {
        if (requested_code == entry->data.code)
        {
            return func_80146DB0((GlyphSprite *)prim, ot, slot, palette);
        }
        slot++;
        entry++;
    }

    font_address = func_8001687C(code & 0xFFFF);
    font_data = (u8 *)font_address;
    if (font_address == -1)
    {
        return prim;
    }

    raster = D_80165674;
    row = 0;
    color_index = (palette + 1) * 2;
    high_nibble_color = color_index * 16;
    for (; row < 15; row++)
    {
        for (source_byte = 0; source_byte < 2; source_byte++)
        {
            mask = 0x80;

            for (slot = 0; slot < 4; slot++)
            {
                *raster = ((*font_data) & mask) ? color_index : 0;

                mask >>= 1;
                high_pixel_set = (*font_data) & mask;

                raster_byte = raster;
                packed_pixels = *raster_byte;
                if (high_pixel_set)
                {
                    packed_pixels += high_nibble_color;
                }

                *raster_byte = packed_pixels;

                mask >>= 1;
                raster++;
            }

            font_data++;
        }
    }

    slot = 0;
    while ((slot < GLYPH_CACHE_SLOTS) && (D_8016D678[slot].raw != 0))
    {
        slot++;
    }

    if (slot == GLYPH_CACHE_SLOTS)
    {
        return prim;
    }
    D_8016D678[slot].raw = code & 0xFFFF;
    prim = func_80146DB0((GlyphSprite *)prim, ot, slot, palette);

    D_8016566C = (slot % GLYPH_CACHE_COLUMNS) * 4;
    D_80165670 = slot & GLYPH_CACHE_ROW_MASK;

    rect.w = 4;
    rect.h = 15;
    rect.x = D_8016566C + 0x140;
    rect.y = D_80165670;

    func_80019A34(&rect, D_80165674);
    func_80019788(0);

    D_80165674 += GLYPH_RASTER_BYTES;
    return prim;
}

s32 func_80146DB0(GlyphSprite *sprite, s32 *ot, s32 cache_slot, s32 palette)
{
    u32 ot_tag_high_byte;
    s32 normalized_slot;
    u32 packet_address;
    s32 old_x;
    s32 new_x;
    s32 fits_line;

    D_8016D678[cache_slot].raw |= 0x10000;

    setlen(sprite, 3);
    setcode(sprite, 0x7C);
    sprite->packet.g0 = 0x80;
    sprite->packet.b0 = 0x80;
    sprite->packet.r0 = 0x80;
    normalized_slot = cache_slot;
    sprite->packet.x0 = D_8016DA78;
    sprite->packet.y0 = D_8016DA7C;

    if (cache_slot < 0)
    {
        normalized_slot = cache_slot + 15;
    }

    sprite->packet.u0 = (cache_slot - ((normalized_slot >> 4) * 16)) * 16;
    sprite->packet.v0 = cache_slot & GLYPH_CACHE_ROW_MASK;
    sprite->packet.clut = 0x7FD3;
    sprite->packet.tag = (sprite->packet.tag & GPU_TAG_HIGH_MASK) | (*ot & GPU_ADDR_MASK);

    packet_address = ((u32)sprite) & GPU_ADDR_MASK;
    ot_tag_high_byte = *ot & GPU_TAG_HIGH_MASK;

    sprite++;
    old_x = D_8016DA78;
    new_x = old_x + 16;
    fits_line = (old_x + 32) < 0x280;
    D_8016DA78 = new_x;

    *ot = ot_tag_high_byte | packet_address;

    if (!fits_line)
    {
        D_8016DA78 = D_80165668;
        D_8016DA7C += 16;
    }

    return (s32)sprite;
}

/* ---- Consolidated from niki_cache_table.c ---- */

extern u8 D_80165678[];

/** @see decomp.me (100.00%) */
void func_80146EB8(void)
{
    s32 i;
    s32 *p;

    D_80165674 = D_80165678;
    i = 0;
    p = (s32 *)D_8016D678;
    do
    {
        *p = (u16)*p;
        i++;
        p++;
    } while (i < 0x100);
}

/** @see decomp.me (100.00%) */
void func_80146EF4(void)
{
    s32 i;
    s32 *p;
    s32 flag;

    i = 0;
    flag = 0x10000;
    p = (s32 *)D_8016D678;
    do
    {
        if (!(*p & flag))
        {
            *p = 0;
        }
        i++;
        p++;
    } while (i < 0x100);
}

/** @see decomp.me (100.00%) */
void func_80146F34(void)
{
    s32 i;
    s32 *p;
    u8 *q;

    i = 0xFF;
    p = (s32 *)D_8016D678;
    p += 0xFF;
    do
    {
        *p = 0;
        i--;
        p--;
    } while (i >= 0);

    i = 0;
    q = D_80165678;
    do
    {
        *(u8 *)(i + (s32)q) = 0;
        i++;
    } while (i <= 0x7FFF);
}

/* ---- Consolidated from niki_expand_text_glyph_codes.c ---- */

extern u8 D_8015D508[];
extern u8 D_801606FC[];

void func_80146F84(u8 *out, u8 *in)
{
    u32 c;
    s32 index;
    s16 lead;

    for (;;)
    {
        c = *in;
        if ((u8)c == 0)
        {
            goto done;
        }
        if ((u32)(c - 0x19) < 7)
        {
            u32 b1;
            s32 off;
            u8 *pa;
            u8 *pb;

            b1 = in[1];
            off = b1 >> 4;
            b1 &= 0xF;
            pa = D_8015D508 + b1 * 2;
            pa += off * 33;
            lead = *(volatile u8 *)in;
            pa += lead * 528;
            *out = *pa;
            out++;
            b1 = in[1];
            off = b1 >> 4;
            b1 &= 0xF;
            pb = D_8015D508 + 1 + b1 * 2;
            pb += off * 33;
            lead = *(volatile u8 *)in;
            pb += lead * 528;
            *out = *pb;
            out++;
            in += 2;
        }
        else if ((u8)c >= 0x21)
        {
            lead = *(volatile u8 *)in;
            index = lead - 0x20;
            *out = D_801606FC[(index / 16) * 33 + (index & 0xF) * 2];
            out++;
            lead = *(volatile u8 *)in;
            index = lead - 0x20;
            *out = D_801606FC[(index / 16) * 33 + (index & 0xF) * 2 + 1];
            out++;
            in += 1;
        }
        else
        {
            *out = D_801606FC[0];
            out++;
            *out = D_801606FC[1];
            out++;
            in += 1;
        }
    }
done:
    *out = 0;
}
