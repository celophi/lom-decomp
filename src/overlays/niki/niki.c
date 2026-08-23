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
} NikiRecord;

typedef struct
{
    u8 data[0x28];
} NikiEntry28;

/**
 * @brief 0xC-stride view of the D_80164B10 element array used by the reset loop.
 * @note Aliases NikiElement at a 0xC stride (the array's real element spacing);
 *       func_80141E84 clears the low 3 state bits of each entry's first word.
 */
typedef struct
{
    s32 unk0;
    s32 unk4;
    void (*unk8)();
} NikiPacket;

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
extern s32 D_80164B1C;
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
extern u8 D_80164DE7;
extern u8 D_80164B98;
extern u8 D_80164B9C;
extern u16 D_80147120;
extern u16 D_80147146;
extern u16 D_80147148;
extern u16 D_8014714C;
extern u16 D_801475C4[];
extern u8 D_800EC3F6[2];
extern u8 D_800EC3D0[];
extern s32 D_8012298C;


void func_80140D2C(void);
void func_80141F18(void);
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
void func_801433BC();
void func_80141E84();
s32 func_80144DF8(void);

void func_800A3938();
void func_80140C60();
void func_80140BF0();
void func_80145F68();
void func_80140CC8();
s32 func_8001714C();
NikiElement *func_80141EC4();
void func_80143284();
void func_80145994();
void func_8014262C();

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
    if ((D_80164B1C & 0x7F) == 2)
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

s32 func_800A88A0(s32 prim, s32 *ot, void *glyph, s32 a3, s32 x, s32 y, s32 mode);
s32 func_800A8A78(s32 *ot, s32 prim, s32 ch, s32 a3, Vec2s *pos, s32 mode);
u8 *func_801442C4(void *arg0);

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
extern u16 D_801471A8;
extern s32 D_80164E20[];
extern s32 D_80164EB0;
extern s32 D_80164EB8[];

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
                    do { flag_ptr = (s32 *)((u8 *)D_80164E20 + (i * 4)); } while (0);
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
        p->unk0 &= ~7;
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
        if ((p->unk0 & 7) == 0)
        {
            p->unk0 = (p->unk0 & ~7) | 1;
            return (NikiElement *)p;
        }
    }
    return &D_80164B10;
}
