#include "common.h"

/* ------------------------------------------------------------------ */
/* Shared types                                                       */
/* ------------------------------------------------------------------ */

typedef struct
{
    s16 x;
    s16 y;
    s16 w;
    s16 h;
} RECT;

typedef struct
{
    s16 x;
    s16 y;
    s16 w;
    s16 h;
} AddheroRect;

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

typedef struct
{
    u32 tag;
    u8 r0;
    u8 g0;
    u8 b0;
    u8 code;
    s16 x0;
    s16 y0;
    s16 w;
    s16 h;
} AddheroTile;

/**
 * @brief Menu draw/cursor element, 0x10 bytes, laid out as an 8-entry array at
 *        D_80160940 (0xC stride is used by the packet views below).
 */
typedef struct AddheroElement
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
    void *draw_handler;
    s32 unkC;
} AddheroElement;

/** @brief Plain 3-word (0xC) view of a menu element. */
typedef struct
{
    s32 attr;
    s32 flags;
    s32 draw;
} AddheroPacket;

typedef struct
{
    u32 word;
} AddheroAttrWord;

/** @brief attr-word view of a menu element used by the claim scan. */
typedef struct
{
    AddheroAttrWord attr;
    u32 unk4;
    u32 unk8;
} AddheroWordPacket;

/** @brief Two-element window over D_80160940 used by func_80141F00. */
typedef struct
{
    AddheroWordPacket first;
    AddheroWordPacket second;
} AddheroPacketBlock;

/** @brief Cursor view (0xC) with a bitfield attr and a draw handler pointer. */
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
} AddheroCursor;

typedef struct AddheroRecord
{
    u8 pad0[0x17];
    u8 unk17;
    u8 pad18[0xCF - 0x18];
    u8 unkCF;
    u8 padD0[4];
    u16 unkD4;
    u16 unkD6;
} AddheroRecord;

typedef struct
{
    u8 data[0x28];
} AddheroEntry28;

typedef struct AddheroFallbackText
{
    u8 pad[0x24];
    u8 text[0x20];
} AddheroFallbackText;

/* func_80141F00 element-draw pipeline types */
typedef struct
{
    s32 unk0;
    s32 unk4;
    s16 unk8;
    s16 unkA;
    s16 unkC;
    u16 unkE;
} AddheroGpuPacket;

typedef struct
{
    s32 unk0;
    u8 pad4[0x40AE];
    s16 unk40B2;
    u8 pad40B4[4];
    AddheroGpuPacket *unk40B8;
} AddheroDrawState;

typedef AddheroGpuPacket *(*AddheroElemDrawFunc)();

/** @brief POLY_G4 words used to draw the timer bar. */
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
} AddheroPolyG4Words;

/** @brief 0x28-byte textured-quad primitive built for a save-slot glyph. */
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
} GlyphPrim;

/* ------------------------------------------------------------------ */
/* Globals                                                            */
/* ------------------------------------------------------------------ */

extern AddheroElement D_80160940;
extern AddheroElement D_8016094C;
extern AddheroEntry28 D_80164B60[][20];
extern AddheroRecord D_80165388;
extern u8 *D_8012271C;
extern u8 *D_80165488;

extern s32 D_8003EC9C;
extern s32 D_80122718;
extern s32 D_80122988;
extern s32 D_8012298C;
extern s32 D_80160580;
extern s32 D_80160920;
extern s32 D_80160924;
extern s32 D_80160928;
extern s32 D_8016092C;
extern s32 D_80160930;
extern s32 D_80160934;
extern s32 D_80160938;
extern s32 D_8016093C;
extern s32 D_801609A0;
extern s32 D_801609A4;
extern s32 D_801609A8;
extern s32 D_801609AC;
extern s32 D_801609B0;
extern s32 D_801609B4;
extern s32 D_801609B8;
extern s32 D_801609BC;
extern s32 D_801609C0;
extern s32 D_801609C4;
extern s32 D_801609E8;
extern s32 D_80164A40;
extern s32 D_80164A4C;
extern s32 D_80164A54;
extern s32 D_80164A60;
extern s32 D_8016545C;
extern s32 D_80165200;
extern s32 D_80165520;
extern s32 D_80147658[];
extern s32 D_801651B0[];
extern s32 D_80165490[];

extern u8 D_80160574;
extern u8 D_80160588[];
extern u8 D_80160590[];
extern u8 D_80160598;
extern u8 D_801605A1;
extern u8 D_801609C8[];
extern u8 D_801609F0[];
extern u8 D_80165208;
extern u8 D_8016520C;
extern u8 D_80165457;
extern u8 D_800EC3F6[2];
extern u8 D_800EC3FA[];

extern char D_800ECF7C[];
extern char D_800ECF8C[];
extern char D_800ECFC4[];

extern u16 D_80146FA4;
extern u16 D_80146FA6;
extern u16 D_80146FA8;
extern u16 D_80146FAA;
extern u16 D_80146FAC;
extern u16 D_80146FB0;
extern u16 D_80146FB2;
extern u16 D_80146FB4;
extern u16 D_80146FB6;
extern u16 D_80146FB8;
extern u16 D_80146FC0;
extern u16 D_80146FCC;
extern u16 D_80146FD2;
extern u16 D_80146FD4;
extern u16 D_80146FD6;
extern u16 D_80146FD8;
extern u16 D_80146FDE;
extern u16 D_80146FE0;
extern u16 D_80146FE2;
extern u16 D_80146FE4;
extern u16 D_80146FE6;
extern u16 D_80146FE8;
extern u16 D_80146FEA;
extern u16 D_80146FF4;
extern u16 D_80146FF8;
extern u16 D_80147012;
extern u16 D_8014700C;
extern u16 D_8014700E;
extern u16 D_80147054;
extern u16 D_80147470[];

/* ------------------------------------------------------------------ */
/* Forward declarations                                               */
/* ------------------------------------------------------------------ */

/* In-file functions */
void func_8014011C(s32 arg0, s32 arg1);
s32 func_801401F8(s32 arg0);
void func_8014028C(void);
void func_801406A8();
s32 func_80140790(void);
s32 func_801408B0(void);
void func_80140C18(void);
void func_80140C94(void);
void func_80140CFC(void);
void func_80140D60(void);
s32 func_80140D80(s32 *ot, s32 prim, s32 arg2, s32 arg3);
s32 func_80141430(s32 *ot, s32 prim, s32 arg2, s32 arg3);
s32 func_801414DC(s32 *ot, s32 prim, s32 arg2, s32 arg3);
s32 func_801415B8(s32 *ot, s32 prim, s32 arg2, s32 arg3);
s32 func_80141694(s32 *ot, s32 prim, s32 arg2, s32 arg3);
u8 *func_80141DA4(void *arg0);
void func_80141DF0(void *arg0);
void func_80141E54();
AddheroElement *func_80141EAC(void);
void func_80141F00();
void func_801424AC(void);
void func_801424C4(u8 *arg0, u8 *arg1);
s32 func_80142548(u8 *arg0);
void func_80142594(u8 *arg0, u8 *arg1);
s32 func_80142618(s32 *ot, s32 prim, s32 arg2, s32 arg3);
s32 func_8014280C(s32 *ot, s32 prim, s32 arg2, s32 arg3);
s32 func_80142A0C(s32 arg0, s32 *arg1);
void func_80142B1C(s32 arg0);
void func_80142C08(s32 arg0);
s32 func_80142CE8(s32 *ot, s32 prim, s32 arg2, s32 arg3);
s32 func_80142E6C(s32 *ot, s32 prim, s32 arg2, s32 arg3);
s32 func_80143044(s32 *ot, s32 prim, s32 arg2, s32 arg3);
s32 func_80143DC0(s32 result, s32 *ot, s32 x, s32 y, s32 adjust, s32 slot, s32 i, s32 j);
void func_80144008(void);
s32 func_80144018(s32 prim, s32 *ot, s32 x, s32 y);
s32 func_80144140(u8 *base);
s32 func_80144194(u8 *data);
s8 *func_801441CC(s8 *out, s32 value);
void func_801442E0(s8 *out, s32 value, s32 max_chars);
void func_801443A8(s8 *out, s32 value);
u32 func_801443CC(u8 *s, s32 len);
s32 func_80144478(u8 *text, s32 unused1, s32 unused2);

/* External functions */
s32 func_800A88A0(s32 prim, s32 *ot, void *glyph, s32 a3, s32 x, s32 y, s32 mode);
s32 func_800A8A78(s32 *ot, s32 prim, s32 ch, s32 a3, Vec2s *pos, s32 mode);
s32 func_8001714C();
void func_800A3938();
void func_800AA02C();
void func_80067F28(void);
void func_80067F8C(void);
void func_80067F5C(s32 arg0);
void func_80063194(void);
void func_80019788(s32 arg0);
void func_8001990C(RECT *rect, s32 a1, s32 a2, s32 a3);
void func_80019A34(RECT *rect, void *str);
void func_800A55E4(void *buf, s32 arg1);
void func_800A5638(void *buf, s32 arg1);
void func_8001A5D4(s32 arg0, s32 *arg1);
void func_8001C56C(s32 *arg0, s32 a1, s32 a2, s32 a3, s32 a4);
s32 func_800AD850();
s32 func_800AE76C();
s32 func_8002054C(s32 arg0);
void func_80016E7C(void *dst, void *src, s32 len);
void field_text_reset_scratch(void);
void field_text_reset_windows(void);
void func_80145A9C(void);
void func_80146D64(void);
void func_80146DA0(void);
void func_80146DE0(void);
void func_801449F0(void);
void func_801458D0(void);
void func_80144008(void);
void func_80145824(void);
s32 func_80145878(void);
void func_80145E14(void);
s32 func_8014686C(s32 result, s32 *ot, u8 *name, s32 x, s32 y, s32 a5, s32 a6);
s32 func_80144C28();

#define SET_ELEM_CODE(e, c) ((e)->attr.word = ((e)->attr.word & 0x00FFFFFF) | ((u32)(c) << 24))
#define GLYPH_SYM(sym, off) ((void *)(((u8 *)&(sym) - (off)) + (sym)))
#define GLYPH_OFF(base, off) ((void *)((base) + *(u16 *)((base) + (off))))

/* ------------------------------------------------------------------ */
/* Functions                                                          */
/* ------------------------------------------------------------------ */

/** @see decomp.me (100%) */
void func_8014011C(s32 arg0, s32 arg1)
{
    RECT rect;

    D_8016093C = arg1;
    D_801609A4 = 0xFF;
    D_801609A8 = 0;
    func_801449F0();
    D_8016092C = 3;
    func_801458D0();
    D_80160920 = 0;
    func_80067F8C();
    rect.x = 0x140;
    rect.y = 0;
    rect.w = 0x40;
    rect.h = 0x100;
    func_8001990C(&rect, 0, 0, 0);
    func_80146DE0();
    D_80165200 = 0;
    D_80160934 = 0;
    D_801609B8 = 0;
    D_80164A40 = 0;
    D_801609C4 = 0;
    D_801609A0 = 0;
    func_800AA02C();
    func_8014028C();
    D_80160930 = arg0;
}

/** @see decomp.me (100%) */
s32 func_801401F8(s32 arg0)
{
    if (D_801609A0 != 0)
    {
        func_80145A9C();
        field_text_reset_windows();
        func_80019788(0);
        return D_801609A0;
    }
    field_text_reset_scratch();
    func_80146D64();
    func_801406A8(arg0);
    func_80146DA0();
    func_80063194();
    D_801609C4 ^= 1;
    return 0;
}

void func_8014028C(void)
{
    AddheroElement *p;
    D_801609BC = 0;
    D_80160938 = 0;
    D_80160928 = 0;
    D_801609AC = 0;
    D_801609B8 = 0;
    D_80160924 = (s32)D_8012271C + 0xCE0;
    if (0) func_80141E54(0,0,0,0,0);
    func_80141E54();
    D_801609B4 = 0;
    if (D_8016093C != 0)
    {
        D_80160940.attr.f.state = 1;
        p = func_80141EAC();
        p->draw_handler = (void *)func_80143044;
        p->attr.f.unk0_3 = 1;
        p->attr.f.x = 0x10;
        p->attr.f.unk0_16 = 0x61;
        p->unk4_0 = 1;
        p->y = 0x2C;
        SET_ELEM_CODE(p, 0x20);

        p = func_80141EAC();
        p->draw_handler = (void *)func_801414DC;
        p->attr.f.unk0_3 = 1;
        p->attr.f.x = 0x18;
        p->attr.f.unk0_16 = 0x4D;
        p->unk4_0 = 0;
        p->y = 0x10;
        SET_ELEM_CODE(p, 0x80);

        p = func_80141EAC();
        p->draw_handler = (void *)func_801415B8;
        p->attr.f.unk0_3 = 1;
        p->attr.f.x = 0xA0;
        p->attr.f.unk0_16 = 0x4D;
        p->unk4_0 = 0;
        p->y = 0x10;
        SET_ELEM_CODE(p, 0x80);
        D_80160940.attr.f.state = 0;
        return;
    }

    D_80160940.attr.f.state = 1;
    p = func_80141EAC();
    p->draw_handler = (void *)func_80140D80;
    p->attr.f.unk0_3 = 1;
    p->attr.f.x = 0x1C;
    p->attr.f.unk0_16 = 0x32;
    p->unk4_0 = 1;
    p->y = 0x58;
    SET_ELEM_CODE(p, 8);
    /* ADDHERO-specific flag */
    *(u32 *)((u8 *)p + 4) |= 0x200;

    p = func_80141EAC();
    p->draw_handler = (void *)func_80141430;
    p->attr.f.unk0_3 = 1;
    p->attr.f.x = 0x24;
    p->attr.f.unk0_16 = 0x0A;
    p->unk4_0 = 0;
    p->y = 0x10;
    SET_ELEM_CODE(p, 0xF0);

    p = func_80141EAC();
    p->draw_handler = (void *)func_801414DC;
    p->attr.f.unk0_3 = 1;
    p->attr.f.x = 0x18;
    p->attr.f.unk0_16 = 0x1E;
    p->unk4_0 = 0;
    p->y = 0x10;
    SET_ELEM_CODE(p, 0x80);

    p = func_80141EAC();
    p->draw_handler = (void *)func_801415B8;
    p->attr.f.unk0_3 = 1;
    p->attr.f.x = 0xA0;
    p->attr.f.unk0_16 = 0x1E;
    p->unk4_0 = 0;
    p->y = 0x10;
    SET_ELEM_CODE(p, 0x80);

    p = func_80141EAC();
    p->draw_handler = (void *)func_80141694;
    p->attr.f.unk0_3 = 1;
    p->attr.f.x = 0x1E;
    p->attr.f.unk0_16 = 0x8E;
    p->unk4_0 = 1;
    p->y = 0x34;
    SET_ELEM_CODE(p, 4);
    D_80160940.attr.f.state = 0;
}

void func_801406A8(void)
{
    s32 delta;

    func_80140D60();
    D_80160920 += 2;
    if ((D_8016094C.attr.word & 0x7F) == 2)
    {
        func_80140790();
    }
    if ((u16)D_80122988 == 0xFFFF)
    {
        D_80122988 = 0;
    }
    func_801408B0();
    if (D_801609BC != 0)
    {
        s32 base = D_80160928;
        delta = (D_80160938 - D_80160928) / D_801609BC;
        D_801609BC -= 1;
        D_80160928 += delta;
    }
    else
    {
        D_80160928 = D_80160938;
    }
}

/** @see decomp.me (100%) */
s32 func_80140790(void)
{
    s32 result;

    if (D_801609A4 >= 0x10)
    {
        if (D_80165488 == 0)
        {
            D_80165488 = (u8 *)&D_80160574;
        }
    }

    do
    {
        result = func_80144C28();
    } while (result == 3);

    if ((D_801609B4 != 0) && (D_80122988 & 0x220))
    {
        if (D_8016093C == 0)
        {
            D_801609A4 = 0xF9;
        }
        else
        {
            D_801609A4 = 0xF8;
        }
        D_80165488 = (u8 *)&D_80160588;
    }
    else
    {
        switch (result)
        {
        case 0:
            break;
        case 4:
            D_80165488 = (u8 *)&D_80160580;
            D_801609B4 = 0;
            break;
        case 5:
            if (D_8016093C == 0)
            {
                D_801609A4 = 0xF9;
            }
            else
            {
                D_801609A4 = 0xF8;
            }
            /* fallthrough */
        case 2:
            D_80165488 = (u8 *)&D_80160588;
            break;
        }
    }
}

s32 func_801408B0(void)
{
    s32 pending;
    s32 status;
    s32 count;
    s32 term1;
    s32 term2;
    AddheroElement *p;

    if ((D_80160940.unkC & 7) == 0) {
        D_801609A0 = D_8016092C;
        return;
    }
    if (D_801609A0 != 0) {
        return;
    }
    if ((D_80160940.unkC & 7) >= 3) {
        return;
    }
    if ((D_80160940.attr.word & 7) != 0) {
        return;
    }
    pending = D_801609A4;
    if (pending == 0xFF) {
        return;
    }
    if (D_80164A60 != 0) {
        return;
    }
    if (D_80164A40 != 0) {
        return;
    }
    if ((u32)(*D_80165488 - 6) < 2U) {
        return;
    }
    if (D_8016093C != 0) {
        return;
    }

    status = D_80122988;
    if (status & 0x40) {
        D_80122718 = 3;
        func_800A3938(0x78, 0x80);
        func_80140C94();
        return;
    }
    if (status & 0xA100) {
        func_800A3938(0x7D, 0x80);
        func_80140C18();
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
            D_801609AC -= 1;
            if (D_801609AC < 0) {
                D_801609AC = D_801609A4 - 1;
            }
        }
        if (D_80122988 & 0x4000) {
            D_801609AC += 1;
            if (D_801609AC >= D_801609A4) {
                D_801609AC = 0;
            }
        }
        count -= 1;
    }

    if (D_80122988 & 0x5000) {
        func_80145E14();
        func_800A3938(0x7D, 0x80);
        func_80140CFC();
        return;
    }

    if (D_80122988 & 0x220) {
        term1 = D_801609A8 * 0x320;
        term2 = (D_801609AC * 0x28) + (s32)D_80164B60;
        if (func_8001714C(D_800ECF7C, (char *)(term1 + term2), 0xC) == 0) {
            if ((D_80165388.unkD4 != ((AddheroRecord *)D_8012271C)->unkD4) &&
                ((D_8003EC9C == 0xFF) || (D_80165388.unkCF == D_8003EC9C))) {
                p = func_80141EAC();
                p->attr.f.unk0_3 = 1;
                p->attr.f.x = 0x10;
                p->attr.f.unk0_16 = 0x61;
                p->unk4_0 = 1;
                p->y = 0x1E;
                SET_ELEM_CODE(p, 0x20);
                func_80144008();
                p->draw_handler = func_80142618;
                func_80145824();
                func_800A3938(0x7E, 0x80);
                return;
            }
        }
        func_800A3938(0x78, 0x80);
    }
}

/** @see decomp.me (100%) */
void func_80140C18(void)
{
    D_801609B4 = 0;
    D_80165488 = 0;
    D_801609A4 = 0xFF;
    D_801609BC = 0;
    D_80160938 = 0;
    D_80160928 = 0;
    D_801609AC = 0;
    D_801609B8 = 0;
    D_801609A8 ^= 1;
    func_801449F0();
    func_800AA02C();
    D_80122988 = 0;
}

void func_80140C94(void)
{
    s32 temp_v1;
    s32 var_a1;
    s32 *var_a0;
    s32 temp;

    func_80067F28();
    var_a0 = (s32 *)&D_80160940;
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

void func_80140CFC(void)
{
    s32 index;
    s32 temp;
    s32 base;
    s32 pos;
    s32 diff;

    index = D_801609AC;
    temp = (index << 3) - index;
    base = D_80160928;
    pos = temp << 1;
    diff = pos - base;

    if (diff >= 0x4B)
    {
        D_80160938 = pos - 0x46;
        D_801609BC = 4;
    }
    if (diff < 0)
    {
        D_80160938 = pos;
        D_801609BC = 4;
    }
}

void func_80140D60(void)
{
    func_80141F00();
}

s32 func_80140D80(s32 *ot, s32 prim, s32 arg2, s32 arg3)
{
    s32 state = D_801609A4;

    switch (state)
    {
    case 0xF8:
        do { prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80146FD8, 0x34), 4, -arg2 + 0x84, -arg3, 2); } while (0);
        break;
    case 0xF9:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80146FD8, 0x34), 4, -arg2 + 0x84, -arg3, 2);
        break;
    case 0xFA:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80146FA6, 2), 4, -arg2 + 0x84, -arg3, 2);
        break;
    case 0xFD:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80146FA8, 4), 4, -arg2 + 0x84, -arg3, 2);
        break;
    case 0xFB:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80146FB4, 0x10), 4, -arg2 + 0x84, -arg3, 2);
        break;
    case 0xFC:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80146FB6, 0x12), 4, -arg2 + 0x84, -arg3, 2);
        break;
    default:
        {
            s32 row_y;
            s32 i;

        if (D_80164A60 != 0)
        {
            s32 x;
            u8 *base;
        case 0xFF:
            x = -arg2 + 0x84;
            base = (u8 *)&D_80146FA4;
            prim = func_800A88A0(prim, ot, base + D_80146FA4, 4, x, -arg3, 2);
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

            base = (u8 *)&D_80146FA4;
            base_x = -arg2;
            do
            {
                row = ((i * 14) - arg3) - D_80160928;
                row_y = row + 1;
                if ((u32)(row + 0xE) < 0x65U)
                {
                    flag_ptr = (s32 *)((u8 *)D_80165490 + (i * 4));
                    if (*flag_ptr >= 0)
                    {
                        pos.x = base_x + 0x86;
                        pos.y = row_y;
                        prim = func_800A88A0(func_800A8A78(ot, prim, *(s32 *)((u8 *)D_801651B0 + (i * 4)), 4, &pos, 0), ot, (void *)((s32)D_80146FD2 + (s32)base), 4, base_x + 0x70, row_y, 0);
                        if ((D_80165520 - 1) == *flag_ptr)
                        {
                            misc_glyph = *(u16 *)(base + 0x36);
                            prim = func_800A88A0(prim, ot, (void *)((s32)misc_glyph + (s32)base), 4, base_x + 0xC0, row_y, 0);
                        }
                        else if (*flag_ptr < 2)
                        {
                            misc_glyph = *(u16 *)(base + 0x38);
                            prim = func_800A88A0(prim, ot, (void *)((s32)misc_glyph + (s32)base), 4, base_x + 0xC0, row_y, 0);
                        }
                        if (*func_80141DA4((void *)((s32)&D_80164B60[D_801609A8][i] + 0xC)) == 0x2B)
                        {
                            prim = func_800A88A0(prim, ot, (void *)((s32)D_80147054 + (s32)base), 4, 0xF2 - arg2, row_y, 1);
                        }
                    }
                    if (func_8001714C(D_800ECF7C, (char *)((s32)&D_80164B60[D_801609A8][i]), 0xC) == 0)
                    {
                        prim = func_800A88A0(prim, ot, (void *)((s32)D_80146FAA + (s32)base), 4, 1 - arg2, row_y, 0);
                    }
                    else if (func_8001714C(D_800ECF8C, (char *)((s32)&D_80164B60[D_801609A8][i]), 0xC) == 0)
                    {
                        prim = func_800A88A0(prim, ot, (void *)((s32)D_80146FDE + (s32)base), 4, 1 - arg2, row_y, 0);
                    }
                    else if (func_8001714C(D_800ECFC4, (char *)((s32)&D_80164B60[D_801609A8][i]), 8) == 0)
                    {
                        prim = func_800A88A0(prim, ot, (void *)((s32)D_80146FB8 + (s32)base), 4, 1 - arg2, row_y, 0);
                    }
                    else
                    {
                        prim = func_800A88A0(prim, ot, (void *)((s32)D_80146FAC + (s32)base), 4, 1 - arg2, row_y, 0);
                    }
                }
                i++;
            } while (i < D_801609A4);
        }
            row_y = ((D_801609AC * 14) - arg3) - D_80160928;

            if (D_80164A60 == 0)
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

s32 func_80141430(s32 *ot, s32 prim, s32 arg2, s32 arg3)
{
    AddheroRect pos;

    if (D_8016093C == 1)
    {
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80146FEA, 0x46), 4, -arg2 + 0x78, -arg3, 2);
    }
    else
    {
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80146FE8, 0x44), 4, -arg2 + 0x78, -arg3, 2);
    }
    return prim;
}

s32 func_801414DC(s32 *ot, s32 prim, s32 arg2, s32 arg3)
{
    AddheroRect pos;
    AddheroTile *tile;

    if (D_801609A8 != 0)
    {
        tile = (AddheroTile *)prim;
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
    return func_800A88A0(prim, ot, GLYPH_SYM(D_80146FB0, 0xC), 4, -arg2 + 0x40, -arg3, 2);
}

s32 func_801415B8(s32 *ot, s32 prim, s32 arg2, s32 arg3)
{
    AddheroRect pos;
    AddheroTile *tile;

    if (D_801609A8 == 0)
    {
        tile = (AddheroTile *)prim;
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
    return func_800A88A0(prim, ot, GLYPH_SYM(D_80146FB2, 0xE), 4, -arg2 + 0x40, -arg3, 2);
}

s32 func_80141694(s32 *ot, s32 prim, s32 arg2, s32 arg3)
{
    s32 result;
    Vec2s pos;
    u8 name[0x21];
    char unused_pad[212];
    s32 slot[3];

    result = prim;
    if (D_801609B8 == 0)
    {
        return result;
    }
    if (D_80164A60 != 0)
    {
        return result;
    }
    if (D_801609B8 != 3 && D_801609A4 < 0x10)
    {
        if (D_801609B8 == 2)
        {
            s32 x = -arg2;
            u8 *base;

            result = func_800A88A0(prim, ot, GLYPH_SYM(D_80146FCC, 0x28), 4, x, -arg3, 0);
            base = (u8 *)&D_80146FCC - 0x28;
            return func_800A88A0(result, ot, GLYPH_OFF(base, 0x2A), 4, x, 0x10 - arg3, 0);
        }
        else
        {
            s32 term1 = D_801609A8 * 0x320;
            s32 term2 = (D_801609AC * 0x28) + (s32)D_80164B60;

            if (func_8001714C(D_800ECF7C, (char *)(term1 + term2), 0xC) == 0)
            {
                if (D_8003EC9C == 0xFF || D_80165457 == D_8003EC9C)
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
                    s32 time_val;

                    {
                        u8 *record = (u8 *)&D_80165388;
                        slot[0] = (u32)(*(s32 *)(record + 0x18)) >> 0x19;
                        slot[1] = ((u32)(*(s32 *)(record + 0x20)) >> 0x12) & 0x7F;
                        slot[2] = (u32)(*(s32 *)(record + 0x20)) >> 0x19;
                        D_801609C0 = (s32)record[0x1F];
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
                        time_val = D_80160920;
                        if (D_80160920 < 0)
                        {
                            time_val = D_80160920 + 0x1F;
                        }
                        D_80160920 -= (time_val >> 5) << 5;
                        break;
                    case 3:
                        step = 0x10;
                        half_step = 0x20;
                        D_80160920 %= 0x60;
                        break;
                    default:
                        step = 0x10;
                        half_step = 0x20;
                        D_80160920 = 0x1F;
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

                            if ((D_80160920 >= base_y && D_80160920 < base_x && (delta = D_80160920 - base_y, 1))
                                || (rem = base_x % (half_step * present_count), D_80160920 >= rem && D_80160920 < (hi = rem + half_step) && (delta = hi - D_80160920, 1)))
                            {
                                adjust += delta;
                            }
                            result = func_80143DC0(result, ot, total - arg2, -arg3, adjust, slot[j], i, j);
                            i += 1;
                            total += adjust;
                        }
                    }

                    {
                        u8 *base90 = (u8 *)&D_80165388;
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
                            do { result = func_800A88A0(result, ot, GLYPH_SYM(D_80146FF4, 0x50), 4, x + 0x54, y + 0x20, 0); } while (0);
                        }
                        else
                        {
                            result = func_800A88A0(result, ot, GLYPH_OFF((u8 *)D_80147470, (*(s32 *)(base90 + 0x20) & 0x3FFFF) * 2), 4,
                                x + 0x54, y + 0x20, 0);
                        }
                    }
                }
                else
                {
                    result = func_800A88A0(result, ot, GLYPH_SYM(D_80146FF8, 0x54), 4, -arg2, -arg3, 0);
                }
            }
            else
            {
                s32 j;
                u8 *record;

                func_80141DF0(&D_8016520C);
                record = &D_8016520C;
                record -= 4;
                if ((u32)(record[0x24] - 1) >= 0x7FU)
                {
                    for (j = 0; j < 0x20; j++)
                    {
                        name[j] = record[4 + j];
                    }
                    name[j] = 0;
                    result = func_8014686C(result, ot, name, -arg2, -arg3, 4, 0);

                    for (j = 0; j < 0x20; j++)
                    {
                        name[j] = ((AddheroFallbackText *)&D_80165208)->text[j];
                    }
                    name[j] = 0;
                    result = func_8014686C(result, ot, name, -arg2, -arg3 + 0x10, 4, 0);
                }
            }
        }
    }
    return result;
}

u8 *func_80141DA4(void *arg0)
{
    u8 *p = arg0;

    while ((u32)(*p - 0x30) < 10 || (u32)(*p - 0x61) < 6 || (u32)(*p - 0x41) < 6)
    {
        p++;
    }
    return p;
}

void func_80141DF0(void *arg0)
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

/** @see decomp.me (100%) */
void func_80141E54(void)
{
    AddheroPacket *p;
    s32 i;

    D_8012298C = 0x20;
    p = (AddheroPacket *)&D_80160940;
    for (i = 0; i < 8; i++)
    {
        p->flags &= ~0x200;
        p->attr &= ~7;
        p++;
    }
}

AddheroElement *func_80141EAC(void)
{
    AddheroWordPacket *p;
    s32 i;

    p = (AddheroWordPacket *)&D_80160940;
    for (i = 0; i < 8; i++, p++)
    {
        if ((p->attr.word & 7) == 0)
        {
            p->attr.word = (p->attr.word & ~7) | 1;
            return (AddheroElement *)p;
        }
    }
    return (AddheroElement *)&D_80160940;
}

void func_80141F00(AddheroDrawState *arg0)
{
    AddheroGpuPacket *var_s0;
    AddheroDrawState *var_s5;
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

    count = D_801609A4;
    if ((count < 0x10) &&
        ((((AddheroPacketBlock *)&D_80160940)->second.attr.word & 7) == 2) &&
        (((((AddheroPacketBlock *)&D_80160940)->second.unk4 >> 9) & 1) != 0))
    {
        count *= 0xE;
        if ((D_80160928 + 0x58) < count)
        {
            var_s0 = (AddheroGpuPacket *)func_800AE76C(var_s0, var_s5, 0x114, 0x82, 0);
        }
        if (D_80160928 != 0)
        {
            var_s0 = (AddheroGpuPacket *)func_800AE76C(var_s0, var_s5, 0x114, 0x3A, 1);
        }
    }

    if (arg0->unk40B2 != 0)
    {
        func_8001C56C(sp20, 0, 0xF0, 0x140, 0xE0);
    }
    else
    {
        func_8001C56C(sp20, 0, 8, 0x140, 0xE0);
    }

    var_s3 = (volatile u32 *)&D_80160940;
    var_s6 = 0;

    for (; var_s6 < 8; var_s6++, var_s3 += 3)
    {
        if (*var_s3 & 7)
        {
            func_8001A5D4((s32)var_s0, sp20);

            var_s0->unk0 = (var_s0->unk0 & 0xFF000000) | (var_s5->unk0 & 0x00FFFFFF);
            var_s5->unk0 = (s32)((var_s5->unk0 & 0xFF000000) | ((s32)var_s0 & 0x00FFFFFF));

            temp_a0_2 = *var_s3;
            temp_v1_2 = temp_a0_2 & 7;

            var_s0 = (AddheroGpuPacket *)((u8 *)var_s0 + 0x40);

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

                var_s0 = (*(AddheroElemDrawFunc *)((u8 *)var_s3 + 8))(var_s5, var_s0, (s32)(temp_a2 - temp_s1) / 2, temp_a3_3 / 2);
                {
                    u32 post_word;
                    u32 field;
                    u32 high;
                    post_word = *var_s3;
                    field = (post_word >> 7) & 0x1FF;
                    high = post_word >> 24;
                    var_s0 = (AddheroGpuPacket *)func_800AD850(var_s0, var_s5,
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
                var_s0 = (*(AddheroElemDrawFunc *)((u8 *)var_s3 + 8))(var_s5, var_s0, 0, 0);
                {
                    u32 case_word;
                    u32 high;
                    case_word = *var_s3;
                    high = case_word >> 24;
                    var_s0 = (AddheroGpuPacket *)func_800AD850(var_s0, var_s5,
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

                var_s0 = (*(AddheroElemDrawFunc *)((u8 *)var_s3 + 8))(var_s5, var_s0, (s32)(temp_a2 - temp_s1) / 2, temp_a3_6 / 2);
                {
                    u32 post_word;
                    u32 field;
                    u32 high;
                    post_word = *var_s3;
                    field = (post_word >> 7) & 0x1FF;
                    high = post_word >> 24;
                    var_s0 = (AddheroGpuPacket *)func_800AD850(var_s0, var_s5,
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

void func_801424AC(void)
{
    D_80160940.attr.word &= ~7;
}

void func_801424C4(u8 *arg0, u8 *arg1)
{
    s32 temp_s0;
    s32 temp_v0;
    s32 i;

    temp_s0 = func_80142548(arg0);
    temp_v0 = func_80142548(arg1);
    for (i = 0; i < temp_v0; i++)
    {
        arg0[temp_s0 + i] = arg1[i];
    }
    arg0[temp_s0 + i] = 0;
}

s32 func_80142548(u8 *arg0)
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

void func_80142594(u8 *arg0, u8 *arg1)
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
            p += 1;
            len += 1;
        }
    }
    for (i = 0; i < len; i++)
    {
        arg0[i] = arg1[i];
    }
    arg0[i] = 0;
}

s32 func_80142618(s32 *ot, s32 prim, s32 arg2, s32 arg3)
{
    RECT pos;
    s32 result;
    s32 x;
    s32 status;
    AddheroElement *p;

    x = -arg2 + 0x90;
    result = func_80144018(
        func_800A88A0(prim, ot,
                      (u8 *)&D_80146FD4 + D_80146FD4 - 0x30,
                      4, x, -arg3, 2),
        ot, x, 0xE - arg3);

    if ((u32)(func_80145878() - 1) < 2U)
    {
        D_80160940.attr.f.state = 0;
        func_800AA02C();
        func_800A3938(0x78, 0x80);
        D_801609A4 = 0xFF;
        func_801449F0();
        D_80165488 = 0;
    }
    else
    {
        status = D_80122988;
        if (status & 0x40)
        {
            D_80160940.attr.f.state = 0;
            func_800AA02C();
            func_800A3938(0x78, 0x80);
            D_80165488 = D_80160588;
        }
        else if (status & 0x220)
        {
            if (D_801609B0 != 0)
            {
                D_80160940.attr.f.state = 0;
                func_800AA02C();
                func_800A3938(0x78, 0x80);
                D_80165488 = D_80160588;
            }
            else
            {
                func_800A3938(0x7E, 0x80);
                D_80160934 = 1;
                D_80165488 = D_80160590;
                p = &D_80160940;
                p->draw_handler = func_8014280C;
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
 * @brief Draw the "loading new hero" prompt and, once the load has finished,
 *        validate and commit the freshly loaded save data.
 * @param ot   Ordering table the prompt primitives are linked into.
 * @param prim Current primitive pointer / index within the ordering table.
 * @param arg2 Horizontal offset used to place the prompt (screen X = 0x90 - arg2).
 * @param arg3 Vertical offset used to place the prompt rows.
 * @return The updated primitive pointer / index after linking the prompt.
 */
s32 func_8014280C(s32 *ot, s32 prim, s32 arg2, s32 arg3)
{
    RECT pos;
    u8 *base;
    u8 *resource;
    AddheroCursor *p;
    AddheroCursor *cursor;
    s32 result;
    s32 x;
    s32 i;
    u32 saved;

    x = -arg2 + 0x90;
    result = func_800A88A0(prim, ot, (void *)((s32)&D_80146FD6 - 0x32 + D_80146FD6), 4, x, -arg3, 2);
    base = (u8 *)&D_80146FD6 - 0x32;
    result = func_800A88A0(result, ot, base + *(u16 *)(base + 0x1E), 4, x, 0xE - arg3, 2);
    result = func_800A88A0(result, ot, base + *(u16 *)(base + 0xB2), 4, x, 0x1C - arg3, 2);
    result = func_80142A0C(result, ot);

    if (D_80160934 == 0)
    {
        resource = D_801609F0;
        p = (AddheroCursor *)&D_80160940;
        p->attr.f.state = 0;
        if (func_80144140(resource) == 0)
        {
            func_80142B1C(4);
            return result;
        }

        func_800A3938(0x7B, 0x80);
        saved = D_8012271C[0x858] >> 7;
        func_80016E7C(resource + 0x770, D_8012271C + 0x840, 0x250);
        *(u32 *)(D_8012271C + 0x858) = (*(u32 *)(D_8012271C + 0x858) & ~0x80) | (saved << 7);
        *(u16 *)(D_8012271C + 0xD8) = *(u16 *)(resource + 0x254);
        *(u16 *)(D_8012271C + 0xDA) = *(u16 *)(resource + 0x256);
        *(u16 *)(D_8012271C + 0xDE) = 1;
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
        D_8016092C = 1;
    }

    return result;
}

s32 func_80142A0C(s32 arg0, s32 *arg1)
{
    AddheroPolyG4Words *g;
    s32 elapsed;
    s32 extent;
    s32 color;

    g = (AddheroPolyG4Words *)arg0;
    if (D_80164A4C != 0)
    {
        elapsed = func_8002054C(-1) - D_80164A54;
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

void func_80142B1C(s32 arg0)
{
    func_800A3938(0x78, 0x80);
    D_80160940.draw_handler = (void *)func_80142CE8;
    D_80160940.attr.f.unk0_3 = 1;
    D_80160940.attr.f.state = 1;
    D_80160940.attr.f.x = 0x20;
    D_80160940.attr.f.unk0_16 = 0x70;
    D_80160940.unk4_0 = 1;
    D_80160940.y = 0x14;
    SET_ELEM_CODE(&D_80160940, 0);
    func_800AA02C();
    D_80165200 = 0;
    D_80160934 = 0;
    D_801609B8 = 0;
    D_80164A40 = 0;
    D_801609A4 = 0xFF;
    func_801449F0();
    D_80165488 = 0;
    D_801609E8 = arg0;
}

/**
 * @brief Initialize the secondary choice element and reset its transition state.
 * @see decomp.me (100%)
 */
void func_80142C08(s32 arg0)
{
    func_800A3938(0x78, 0x80);
    D_8016094C.draw_handler = (void *)func_80142E6C;
    D_8016094C.attr.f.unk0_3 = 1;
    D_8016094C.attr.f.state = 1;
    D_8016094C.attr.f.x = 0x20;
    D_8016094C.attr.f.unk0_16 = 0x70;
    D_8016094C.unk4_0 = 1;
    D_8016094C.y = 0x14;
    SET_ELEM_CODE(&D_8016094C, 0);
    func_800AA02C();
    D_80165200 = 0;
    D_80160934 = 0;
    D_801609B8 = 0;
    D_80164A40 = 0;
    func_801449F0();
    D_80165488 = 0;
    D_801609E8 = arg0;
}

s32 func_80142CE8(s32 *ot, s32 prim, s32 arg2, s32 arg3)
{
    RECT pos;

    switch (D_801609E8)
    {
    case 0:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80146FE0, 0x3C), 4, -arg2 + 0x80, -arg3, 2);
        break;
    case 2:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80146FE4, 0x40), 4, -arg2 + 0x80, -arg3, 2);
        break;
    case 3:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80146FE6, 0x42), 4, -arg2 + 0x80, -arg3, 2);
        break;
    case 1:
    case 4:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80146FE2, 0x3E), 4, -arg2 + 0x80, -arg3, 2);
        break;
    }
    if (D_80122988 & 0x220)
    {
        D_80160940.attr.f.state = 0;
        func_800AA02C();
    }
    return prim;
}

/** @see decomp.me (100%) */
s32 func_80142E6C(s32 *ot, s32 prim, s32 arg2, s32 arg3)
{
    RECT pos;
    AddheroPacket *p;
    s32 i;

    switch (D_801609E8)
    {
    case 0:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80146FE0, 0x3C), 4, -arg2 + 0x80, -arg3, 2);
        break;
    case 2:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80146FE4, 0x40), 4, -arg2 + 0x80, -arg3, 2);
        break;
    case 3:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80146FE6, 0x42), 4, -arg2 + 0x80, -arg3, 2);
        break;
    case 1:
    case 4:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80146FE2, 0x3E), 4, -arg2 + 0x80, -arg3, 2);
        break;
    }
    if (D_80122988 & 0x220)
    {
        D_8016092C = 3;
        D_8012298C = 0x20;
        p = &D_80160940;
        for (i = 0; i < 8; i++)
        {
            p->flags &= ~0x200;
            p->attr &= ~7;
            p++;
        }
        func_80067F5C(8);
        func_800AA02C();
    }
    return prim;
}

s32 func_80143044(s32 *ot, s32 prim, s32 arg2, s32 arg3)
{
    RECT pos;
    switch (D_801609A4)
    {
    case 0xF8:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80146FD8, 0x34), 4, -arg2 + 0x90, -arg3, 2);
        break;
    case 0xF9:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80146FD8, 0x34), 4, -arg2 + 0x90, -arg3, 2);
        break;
    case 0xFF:
        {
            s32 x; u8 *base;
            x = -arg2 + 0x90;
            base = (u8 *)&D_80146FA4;
            prim = func_800A88A0(prim, ot, base + D_80146FA4, 4, x, -arg3, 2);
            prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0x1E), 4, x, 0xE - arg3, 2);
            prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0xB2), 4, x, 0x1C - arg3, 2);
        }
        break;
    case 0xFA:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80146FD8, 0x34), 4, -arg2 + 0x90, -arg3, 2);
        break;
    case 0xFD:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80146FA8, 4), 4, -arg2 + 0x90, -arg3, 2);
        break;
    case 0xFB:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80146FB4, 0x10), 4, -arg2 + 0x90, -arg3, 2);
        break;
    case 0xFC:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80146FB6, 0x12), 4, -arg2 + 0x90, -arg3, 2);
        break;
    case 0xF7:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014700C, 0x68), 4, -arg2 + 0x90, -arg3, 2);
        break;
    case 0xF6:
        {
            s32 x; u8 *base; AddheroPolyG4Words *g; s32 next, elapsed, extent, color, finalmode;
            x = -arg2 + 0x90;
            prim = func_800A88A0(prim, ot, (void *)((s32)&D_80146FD6 - 0x32 + D_80146FD6), 4, x, -arg3, 2);
            base = (u8 *)&D_80146FD6 - 0x32;
            prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0x1E), 4, x, 0xE - arg3, 2);
            prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0xB2), 4, x, 0x1C - arg3, 2);
            next = prim; g = (AddheroPolyG4Words *)prim;
            if (D_80164A4C != 0)
            {
                elapsed = func_8002054C(-1) - D_80164A54;
                if (elapsed >= 0x101) elapsed = 0x100;
                color = 0xFFFF00; extent = elapsed * 0x120;
                g->unk4=0xFF; g->unkC=0xFFFF; g->unk1C=0xFF0000; ((u8 *)g)[3]=8;
                g->unk14=color; ((u8 *)g)[7]=0x38; g->unk18=0; g->unk8=0;
                if (extent < 0) extent += 0xFF;
                g->unk20=extent>>8; g->unk10=extent>>8; g->unk12=0; g->unkA=0; g->unk22=0x2C; g->unk1A=0x2C;
                g->unk0=(g->unk0 & 0xFF000000)|(*ot & 0xFFFFFF);
                *ot=(*ot & 0xFF000000)|(prim & 0xFFFFFF); next=prim+0x24;
            }
            prim = next;
            if (D_80160934 == 0)
            {
                if (func_80144140(D_801609F0) == 0)
                {
                    func_800A3938(0x78, 0x80);
                    D_80160940.draw_handler=(void *)func_80142CE8;
                    D_80160940.attr.f.unk0_3=1; D_80160940.attr.f.state=1; D_80160940.attr.f.x=0x20; D_80160940.attr.f.unk0_16=0x70;
                    D_80160940.unk4_0=1; D_80160940.y=0x14; SET_ELEM_CODE(&D_80160940,0);
                    func_800AA02C();
                    D_80165200=0; D_801609B8=0; D_80164A40=0; D_80160934=0; D_801609A4=0xFF;
                    func_801449F0(); finalmode=4; D_80165488=0; D_801609E8=finalmode; return prim;
                }
                func_800A3938(0x7B,0x80); D_801609A4=0xF4; func_80144008(); func_800AA02C();
            }
        }
        break;
    case 0xF3:
        {
            s32 x; AddheroPacket *packet; s32 i;
            x = -arg2 + 0x90;
            prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80147012,0x6E), 4, x, -arg3, 2);
            prim = func_80144018(prim, ot, x, 0xE -arg3);
            if (D_80122988 & 0x40)
            {
                func_800A3938(0x78, 0x80);
                func_80144008();
                D_801609A4 = 0xF4;
                func_800AA02C();
            }
            else if (D_80122988 & 0x220)
            {
                if (D_801609B0 != 0)
                {
                    func_800A3938(0x78, 0x80);
                    func_80144008();
                    D_801609A4 = 0xF4;
                    func_800AA02C();
                }
                else
                {
                    func_800A3938(0x7D, 0x80);
                    D_8016092C = 3;
                    D_8012298C = 0x20;
                    packet = (AddheroPacket *)&D_80160940;
                    for (i = 0; i < 8; i++, packet++)
                    {
                        packet->flags &= ~0x200;
                        packet->attr &= ~7;
                    }
                    func_80067F5C(8);
                    func_800AA02C();
                }
            }
        }
        break;
    case 0xF4:
        {
            s32 x; u8 *base; s32 temp;
            x=-arg2+0x90;
            prim=func_800A88A0(prim,ot,(void *)((s32)&D_8014700E-0x6A+D_8014700E),4,x,-arg3,2);
            base=(u8 *)&D_8014700E-0x6A;
            prim=func_800A88A0(prim,ot,GLYPH_OFF(base,0x6C),4,x,0xE -arg3,2);
            prim=func_80144018(prim,ot,x,0x1C-arg3);
            if (D_80122988 & 0x40)
            {
                func_80144008();
                D_801609A4 = 0xF3;
                func_800A3938(0x78, 0x80);
                func_800AA02C();
            }
            else if (D_80122988 & 0x220)
            {
                if (D_801609B0 != 0)
                {
                    func_80144008();
                    D_801609A4 = 0xF3;
                    func_800A3938(0x78, 0x80);
                    func_800AA02C();
                }
                else
                {
                    func_800A3938(0x7E,0x80);
                    base = D_801609F0;
                    func_80016E7C(D_8012271C + 0x840, base + 0x770, 0x250);
                    *(s32 *)(base + 0x788) |= 0x80;
                    temp = func_80144194(base);
                    *(s32 *)(base + 0x33E4) = 0x414E41;
                    *(s32 *)(base + 0x33E0) = temp;
                    D_80165200 = 1;
                    D_80165488 = &D_801605A1;
                    D_801609A4 = 0xF5;
                }
            }
        }
        break;
    case 0xF5:
        {
            s32 x; u8 *base; AddheroPolyG4Words *g; s32 next,elapsed,extent,color; AddheroPacket *packet; s32 i;
            x=-arg2+0x90;
            prim=func_800A88A0(prim,ot,(void *)((s32)&D_80146FC0-0x1C+D_80146FC0),4,x,-arg3,2);
            base=(u8 *)&D_80146FC0-0x1C;
            prim=func_800A88A0(prim,ot,GLYPH_OFF(base,0x1E),4,x,0xE -arg3,2);
            prim=func_800A88A0(prim,ot,GLYPH_OFF(base,0xB2),4,x,0x1C-arg3,2);
            next=prim; g=(AddheroPolyG4Words *)prim;
            if(D_80164A4C!=0){
                elapsed=func_8002054C(-1)-D_80164A54; if(elapsed>=0x101)elapsed=0x100; color=0xFFFF00; extent=elapsed*0x120;
                g->unk4=0xFF;g->unkC=0xFFFF;g->unk1C=0xFF0000;((u8*)g)[3]=8;g->unk14=color;((u8*)g)[7]=0x38;g->unk18=0;g->unk8=0;
                if(extent<0)extent+=0xFF;g->unk20=extent>>8;g->unk10=extent>>8;g->unk12=0;g->unkA=0;g->unk22=0x2C;g->unk1A=0x2C;
                g->unk0=(g->unk0&0xFF000000)|(*ot&0xFFFFFF);*ot=(*ot&0xFF000000)|(prim&0xFFFFFF);next=prim+0x24;
            }
            prim=next;
            if(D_80165200==0){
                D_8012271C[0x840]=0; func_800A3938(0x7A,0x80); D_8012298C=0x20;
                packet=(AddheroPacket *)&D_80160940;
                for(i=0;i<8;i++,packet++){ packet->flags &= ~0x200; packet->attr &= ~7; }
                func_80067F5C(8); D_8016092C=2;
            }
        }
        break;
    default:
        {
            s32 x,posv,diff; u8 *base;
            x=-arg2+0x90; base=(u8 *)&D_80146FA4;
            prim=func_800A88A0(prim,ot,base+D_80146FA4,4,x,-arg3,2);
            prim=func_800A88A0(prim,ot,GLYPH_OFF(base,0x1E),4,x,0xE -arg3,2);
            prim=func_800A88A0(prim,ot,GLYPH_OFF(base,0xB2),4,x,0x1C-arg3,2);
            if(D_80164A60==0){
                if(D_80164A40!=0)return prim;
                if((u32)(*D_80165488-6)<2U)return prim;
                if((func_8001714C(D_800ECF7C,&D_80164B60[D_801609A8][D_801609AC],0xC)!=0) ||
                   (D_8016545C != *(s32 *)(D_8012271C+0xD8))) {
                    D_801609AC++;
                    if(D_801609AC>=D_801609A4){ if(D_801609A4!=0)D_801609A4=0xF7; else D_801609A4=0xF8; }
                    else { func_80145E14(); posv=D_801609AC*0xE; diff=posv-D_80160928;
                        if(diff>=0x4B){D_80160938=posv-0x46;D_801609BC=4;} if(diff<0){D_80160938=posv;D_801609BC=4;}
                    }
                } else { D_80164A54=func_8002054C(-1);D_80160934=1;D_80165488=&D_80160598;D_801609A4=0xF6; }
            }
        }
        break;
    case 0xFE:
        break;
    }
    if(D_80164A40!=0)return prim;
    if(D_801609A4==0xF6)return prim; if(D_801609A4==0xF5)return prim; if(D_801609A4==0xF4)return prim; if(D_801609A4==0xF3)return prim;
    if(D_80122988&0x40){
        s32 *p; s32 i,word; D_80122718=3;func_800A3938(0x78,0x80);func_80067F28();p=(s32 *)&D_80160940;i=0;
        do{word=*p;if(word&7)*p=(((word&~7)|3)&~0x78)|0x40;i++;p+=3;}while(i<8);return prim;
    }
    if((D_80122988&0xA100)&&(D_801609A4!=0xFF)){
        func_800A3938(0x7D,0x80);D_801609B4=0;D_80165488=0;D_801609BC=0;D_80160938=0;D_80160928=0;D_801609AC=0;D_801609A4=0xFF;D_801609B8=0;
        D_801609A8^=1;func_801449F0();func_800AA02C();D_80164A4C=0;D_80122988=0;D_80165488=&D_80160574;
    }
    return prim;
}

s32 func_80143DC0(s32 result, s32 *ot, s32 x, s32 y, s32 adjust, s32 slot, s32 i, s32 j)
{
    RECT rect;
    s32 temp;
    s8 shade;

    if (slot == 0x7F) return result;
    rect.x = i * 0x10;
    rect.y = 0x1F2;
    rect.w = 0x10;
    rect.h = 1;
    if ((j == 1) && (slot < 2)) {
        func_800A5638(D_801609C8, slot);
        func_80019A34(&rect, D_801609C8);
        func_80019788(0);
    } else if (slot >= 0x4F) {
        func_800A55E4(D_801609C8, D_801609C0);
        func_80019A34(&rect, D_801609C8);
        func_80019788(0);
    } else {
        func_80019A34(&rect, (void *)((u8 *)&D_80147658 - 4 + D_80147658[slot]));
    }
    temp = i * 3;
    rect.x = temp * 4 + 0x140;
    rect.y = 0xD0;
    rect.w = 0xC;
    rect.h = 0x30;
    func_80019A34(&rect, (void *)((u8 *)&D_80147658 + 0x1C + D_80147658[slot]));
    ((GlyphPrim *)result)->unk4 = 0x808080;
    ((u8 *)result)[3] = 9;
    ((u8 *)result)[7] = 0x2C;
    ((GlyphPrim *)result)->unk18 = x;
    ((GlyphPrim *)result)->unk8 = x;
    ((GlyphPrim *)result)->unk12 = y;
    ((GlyphPrim *)result)->unkA = y;
    ((GlyphPrim *)result)->unk20 = x + adjust;
    shade = temp * 0x10;
    ((GlyphPrim *)result)->unk1C = shade;
    ((GlyphPrim *)result)->unkC = shade;
    shade += 0x2F;
    ((GlyphPrim *)result)->unk24 = shade;
    ((GlyphPrim *)result)->unk14 = shade;
    ((GlyphPrim *)result)->unk15 = 0xD0;
    ((GlyphPrim *)result)->unkD = 0xD0;
    ((GlyphPrim *)result)->unk10 = x + adjust;
    ((GlyphPrim *)result)->unk22 = y + 0x2F;
    ((GlyphPrim *)result)->unk1A = y + 0x2F;
    ((GlyphPrim *)result)->unk25 = 0xFF;
    ((GlyphPrim *)result)->unk1D = 0xFF;
    ((GlyphPrim *)result)->unkE = (i & 0x3F) | 0x7C80;
    ((GlyphPrim *)result)->unk16 = 5;
    ((GlyphPrim *)result)->unk0 = (((GlyphPrim *)result)->unk0 & 0xFF000000) | (*ot & 0xFFFFFF);
    *ot = (*ot & 0xFF000000) | (result & 0xFFFFFF);
    return result + 0x28;
}

void func_80144008(void)
{
    D_801609B0 = 1;
}

s32 func_80144018(s32 prim, s32 *ot, s32 x, s32 y)
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
    if (D_801609B0 != 0)
    {
        a3 = 5;
    }
    prim = func_800A88A0(prim, ot, (void *)g1, a3, x - 0x10, y, 1);
    a3 = 4;
    g2 = base[0x38] + ((base[0x39] << 8) + (s32)base);
    if (D_801609B0 == 0)
    {
        a3 = 5;
    }
    prim = func_800A88A0(prim, ot, (void *)g2, a3, x + 8, y, 0);
    if (D_80122988 & 0xA000)
    {
        D_801609B0 ^= 1;
        func_800A3938(0x7D, 0x80);
        D_80122988 = 0;
    }
    return prim;
}

s32 func_80144140(u8 *base)
{
    if (*(s32 *)(base + 0x33E0) == func_80144194(base))
    {
        if (*(s32 *)(base + 0x33E4) == 0x414E41)
        {
            return 1;
        }
    }
    return 0;
}

s32 func_80144194(u8 *data)
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

s8 *func_801441CC(s8 *out, s32 value)
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

void func_801442E0(s8 *out, s32 value, s32 max_chars)
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
                func_801443A8(cursor, nibble);
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

void func_801443A8(s8 *out, s32 value)
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

u32 func_801443CC(u8 *s, s32 len)
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

s32 func_80144478(u8 *text, s32 unused1, s32 unused2)
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
