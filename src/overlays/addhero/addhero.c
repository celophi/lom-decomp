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
    u8 *p;
    u32 c;

    p = arg0;
    while (1)
    {
        c = *p;
        p++;
        if ((u32)(c - '0') < 10)
        {
            continue;
        }
        p--;
        if (p)
        {
            p++;
            p--;
        }

        p++;
        if ((u32)(c - 'a') < 6)
        {
            continue;
        }
        p--;
        if (p)
        {
            p++;
            p--;
        }

        p++;
        if ((u32)(c - 'A') < 6)
        {
            continue;
        }
        p--;
        if (p)
        {
            p++;
            p--;
        }
        break;
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

/* ------------------------------------------------------------------ */
/* Former func_80144570.c */
/* ------------------------------------------------------------------ */

extern s32 D_80164A68[];

s32 func_80144570(void)
{
    s32 i; s32 max; u8 *p; u8 *field; s32 count; s32 acc;
    u32 tmp0, tmp1, tmp2; s32 r; u8 *pattern;
    i = 0; max = i;
    while (i < D_801609A4) {
        do { pattern = (u8 *)&D_800ECF7C; } while (0);
        if (func_8001714C(pattern, (u8 *)&((AddheroEntry28 (*)[20])D_80164B60)[D_801609A8][i], 0xC) == 0) {
            count = 5;
            p = (u8 *)(D_801609A8 * 0x320 + ((i << 4) + (i << 4) + (i << 3)) + (s32)D_80164B60 + 0xC);
            acc = 0;
            while (((u8)(*p-'0') < 10) || ((u8)(*p-'a') < 6) || ((u8)(*p-'A') < 6)) {
                if (count == 0) break;
                acc <<= 4;
                if ((u8)(*p-'0') < 10) { tmp0=acc-0x30; acc=tmp0+*p; }
                else if ((u8)(*p-'A') < 6) { tmp1=acc-0x37; acc=tmp1+*p; }
                else if ((u8)(*p-'a') < 6) { tmp2=acc-0x57; acc=tmp2+*p; }
                p++; count--;
            }
            field = (u8 *)&((AddheroEntry28 (*)[20])D_80164B60)[D_801609A8][i].data[0xC];
            { s32 addr; addr = D_801609A8 * 0x50 + (s32)D_80164A68; *(s32 *)(addr + i*4) = acc; }
            r = func_80144478(field, acc, count);
            D_801651B0[i] = r;
            if (max < r) max = r;
        } else {
            { s32 addr; addr = D_801609A8 * 0x50 + (s32)D_80164A68; *(s32 *)(addr + i*4) = -1; }
            D_801651B0[i] = 0;
        }
        i++;
    }
    return max;
}

/* ------------------------------------------------------------------ */
/* ADDHERO continuation declarations                                  */
/* ------------------------------------------------------------------ */

#include "gpu_packet.h"
#include "sdk/libgte.h"

/* addhero.c already has local RECT/TILE ABI views used by the earlier code. */
#define RECT AddheroSdkRect
#define TILE AddheroSdkTile
#include "sdk/libgpu.h"
#undef TILE
#undef RECT

typedef struct EntryHeader7
{
    s32 unk0;
    s16 unk4;
    s8 unk6;
    u8 pad[9];
} EntryHeader7;

/** @brief Save-file header block at D_80140090 (only the first 6 bytes used). */
typedef struct
{
    s32 unk0;
    s16 unk4;
    u8 pad[0xFA];
} AddheroFileHeader;

typedef struct
{
    s32 unk0;
    s16 unk4;
    s16 unk6;
    u8 unk8[0x18];
} AddheroFileHeaderScratch;

typedef struct
{
    s32 unk0;
    s16 unk4;
    u8 pad[0x62];
} AddheroLoadScratch;

/** @brief Directory entry as laid out in a D_80164B60 slot (0x28 stride). */
typedef struct
{
    char name[20];
    s32 attr;
    s32 size;
    void *next;
    s32 head;
    char system[4];
} Entry;

/** @brief Directory entry view used by the scan pass (size field at 0x18). */
typedef struct
{
    u8 pad[0x18];
    s32 size;
} AddheroDirEntry;

/* --- text renderer (func_8014686C family) --- */

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
    SPRT_16 packet;
    u32 padding;
} GlyphSprite;

#define GLYPH_CACHE_SLOTS 0x100
#define GLYPH_CACHE_COLUMNS 16
#define GLYPH_CACHE_ROW_MASK 0xF0
#define GLYPH_RASTER_BYTES 0x80
#define GPU_ADDR_MASK 0xFFFFFF
#define GPU_TAG_HIGH_MASK 0xFF000000

/* ------------------------------------------------------------------ */
/* Globals                                                            */
/* ------------------------------------------------------------------ */

extern AddheroEntry28 D_80164B60[][20];
extern AddheroFileHeader D_80140090;
extern EntryHeader7 D_80140114;
extern GlyphCacheEntry D_8016D528[];

extern s32 D_80160928;
extern s32 D_80160934;
extern s32 D_80160938;
extern s32 D_8016093C;
extern s32 D_801609A4;
extern s32 D_801609A8;
extern s32 D_801609AC;
extern s32 D_801609B8;
extern s32 D_801609BC;
extern s32 D_80164A40;
extern s32 D_80164A48;
extern s32 D_80164A4C;
extern s32 D_80164A50;
extern s32 D_80164A54;
extern s32 D_80164A58;
extern s32 D_80164A5C;
extern s32 D_80164A60;
extern s32 D_80164A68[];
extern s32 D_80164B08;
extern s32 D_80164B0C;
extern s32 D_80164B10;
extern s32 D_80164B14;
extern s32 D_80164B18;
extern s32 D_80164B1C;
extern s32 D_80165200;
extern s32 D_8016548C;
extern s32 D_80165490[];
extern s32 D_801651A0;
extern s32 D_801651A4;
extern s32 D_801651A8;
extern s32 D_801651AC;
extern s32 D_801651B0[];
extern s32 D_80165520;

extern s32 D_8016D928;
extern s32 D_8016D92C;
extern s32 D_8016D930;
extern s32 D_8016D934;
extern s32 D_8016D938;

extern u8 *D_80165488;
extern u8 *D_8016D93C;

extern u8 D_8016057C[];
extern u8 D_8016058C[];
extern u8 D_801605A8[];
extern u8 D_8015D3B4[];
extern u8 D_801609F0[];
extern u8 D_80164B20[];
extern u8 D_80165528[];
extern u8 D_801654E0[];

extern u16 D_801608E0[];
extern u16 D_801608F8[];

extern char D_800ECF7C[];
extern char D_800ECF8C[];
extern char D_800ECF9C[];
extern char D_800ECFB0[];
extern char D_800ECFC4[];

/* ------------------------------------------------------------------ */
/* Forward declarations                                               */
/* ------------------------------------------------------------------ */

/* In-file functions */
s32 func_801447B4(s32 unused0, s32 unused1, s32 unused2);
void func_801449F0(void);
s32 func_80144A28(void);
s32 func_80144AF8(void);
void func_80144B74(void);
s32 func_80144C28(void);
void func_80145824(void);
s32 func_80145878(void);
void func_801458D0(void);
void func_80145A9C(void);
s32 func_80145B4C(s32 page);
s32 func_80145C34(s32 page);
void func_80145E14(void);
void func_80145FC0(void);
void func_80146018(void);
s32 func_80146070(void);
s32 func_80146104(void);
void func_80146198(void);
s32 func_801465C8(s32 prim, s32 *ot, s32 value, s32 x, s32 y, s32 palette, s32 alignment);
void func_801467E8(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5);
s32 func_8014686C(s32 prim, s32 *ot, u8 *text, s32 x, s32 y, s32 palette, s32 alignment);
s32 func_80146A3C(s32 prim, s32 *ot, s32 character_code, s32 palette);
s32 func_80146C5C(GlyphSprite *sprite, s32 *ot, s32 cache_slot, s32 palette);
void func_80146D64(void);
void func_80146DA0(void);
void func_80146DE0(void);
void func_80146E30(u8 *out, u8 *in);

/* External functions (defined in addhero.c or elsewhere) */
s32 func_80144570();
void func_80140CFC(void);
void func_80142B1C(s32 arg0);
void func_80142C08(s32 arg0);
void func_800AA02C(void);
s32 func_8001714C(void *a, void *b, s32 n);
s32 func_80016F9C(void *a, void *b);
s32 func_8001680C(void *a, s32 b);
s32 func_8001681C(s32 a, void *b, s32 c);
s32 func_8001682C(s32 a, void *b, s32 c);
s32 func_8001683C(s32 a);
s32 func_8001685C(void *a, void *b);
s32 func_8001686C(void *a);
s32 func_800170BC(void *a, void *b, ...);
s32 func_8001724C(s32 a);
s32 func_8001725C(s32 a);
s32 func_8001729C(s32 a);
s32 func_800172AC(s32 a);
s32 func_8002054C(s32 a);
s32 func_80032174(s32 a, void *b, s32 *c);
s32 func_800342CC(s32 a);
s32 func_80016BCC(void *a, void *b);
void func_800B0170(void *a);
s32 func_8001684C(void *a);
void func_80016E7C(void *a, void *b, s32 c);
s32 func_8001687C(s32 a);
void func_80019A34(RECT *rect, void *str);
void func_80019788(s32 arg0);
void func_800158E0(void);
s32 func_800167AC(s32 a, s32 b, s32 c, s32 d);
void func_800167BC(s32 a);
s32 func_800167CC(s32 a);
void func_800167DC(s32 a);
void func_800167EC(void);
void func_800167FC(void);

/* ------------------------------------------------------------------ */
/* Former addhero2.c functions */
/* ------------------------------------------------------------------ */

s32 func_801447B4(s32 unused0, s32 unused1, s32 unused2)
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

    func_80144570();
    s3v = -1;
    func_80146198();
    i = 0;
    handle = func_80144570();
    func_801449F0();
    t0v = 1;
    if (D_801609A4 > 0)
    {
        count = D_801609A4;
        base_rank = &D_80165490[0];
        rank_ptr = base_rank;
        slot = D_801609A8;
        field1 = D_80164A68;
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
    D_80165520 = t0v;
    t0v = -1;
    i = 0;
    s3v = 0;
    if (D_801609A4 > 0)
    {
        s32 max_count;
        max_count = D_801609A4;
        slot = D_801609A8;
        field_base = D_80164A68;
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
    D_80164A5C = t0v + 1;
    if (D_801609A4 > 0)
    {
        out_ptr = &D_801651B0[0];
        ent_ptr = (char *)&D_80164B60[0];
    loop_20:
        if (func_8001714C(&D_800ECFC4[0], (void *)((D_801609A8 * 0x320) + (s32)ent_ptr), 8) == 0)
        {
            *out_ptr = handle + 1;
        }
        else
        {
            out_ptr += 1;
            ent_ptr += 0x28;
            i += 1;
            if (i < D_801609A4)
            {
                goto loop_20;
            }
        }
    }
    return s3v;
}

void func_801449F0(void)
{
    s32 i;
    s32 val;

    D_80165520 = 0x28;
    val = -1;
    for (i = 14; i >= 0; i--)
    {
        D_80165490[i] = val;
    }
}

s32 func_80144A28(void)
{
    s32 i;
    u8 *entry;

    i = 0;
    if (D_801609A4 > 0)
    {
        do
        {
            entry = (u8 *)D_80164B60 + i * 0x28;
            if (func_8001714C(&D_800ECF7C, (void *)(D_801609A8 * 0x320 + (s32)entry), 0xC) == 0 ||
                func_8001714C(&D_800ECF8C, (void *)(D_801609A8 * 0x320 + (s32)entry), 0xC) == 0)
            {
                return 1;
            }
            i++;
        } while (i < D_801609A4);
    }
    return 0;
}

s32 func_80144AF8(void)
{
    s32 i;
    s32 sum;
    s32 offset;

    i = 0;
    sum = 0;
    if (D_801609A4 > 0)
    {
        offset = D_801609A8 * 0x320;
        do
        {
            do {
                sum += ((Entry *)((u8 *)D_80164B60 + offset))->size / 8192;
            } while (0);
            i++;
            offset += 0x28;
        } while (i < D_801609A4);
    }
    return sum >= 0xE;
}

void func_80144B74(void)
{
    AddheroFileHeaderScratch buf;

    memcpy(&buf, &D_80140090, 6);
    ((u8 *)&buf)[2] += *(u8 *)&D_801609A8;
    func_80016F9C(&buf, &D_800ECF9C);
    func_8001686C(&buf);

    memcpy(&buf, &D_80140090, 6);
    ((u8 *)&buf)[2] += *(u8 *)&D_801609A8;
    func_80016F9C(&buf, &D_800ECFB0);
    func_8001686C(&buf);
}

static inline void addhero_probe_render_two(void)
{
    AddheroFileHeaderScratch p;

    memcpy(&p, &D_80140090, 6);
    ((u8 *)&p)[2] += *(u8 *)&D_801609A8;
    func_80016F9C(&p, &D_800ECF9C);
    func_8001686C(&p);

    memcpy(&p, &D_80140090, 6);
    ((u8 *)&p)[2] += *(u8 *)&D_801609A8;
    func_80016F9C(&p, &D_800ECFB0);
    func_8001686C(&p);
}

s32 func_80144C28(void)
{
    AddheroLoadScratch buf;
    s32 status0;
    s32 status1;
    s32 phase_result;
    s32 wait_attempts;
    s32 poll_result;
    s32 poll_result20;
    s32 rank_index;
    s32 rank_value;

    memcpy(&buf, &D_80140090, 6);
    phase_result = 1;
    ((u8 *)&buf)[2] += *(u8 *)&D_801609A8;

    if (D_80165488 == NULL)
    {
        goto block_return;
    }

    switch (*D_80165488)
    {
    case 1:
        phase_result = 3;
        func_8001729C(D_801609A8);
        func_8001724C(D_801609A8 * 0x10);
        D_80165488 = D_80165488 + 1;
        break;

    case 2:
        poll_result = func_80146070();
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
            goto c2_increment;
        }
        break;
    c2_ge3:
        if (poll_result == 3)
        {
            goto c2_eq3;
        }
        break;
    c2_increment:
        D_80165488 = D_80165488 + 1;
        break;
    c2_pos:
        phase_result = 4;
        D_801609B8 = 0;
        D_801609A4 = 0xFD;
        D_80165488 = D_80165488 + 1;
        break;
    c2_eq3:
        D_80165520 = 0x28;
        rank_value = -1;
        for (rank_index = 14; rank_index >= 0; rank_index--)
        {
            D_80165490[rank_index] = rank_value;
        }
        D_801609A4 = 0xFF;
        D_80165488 = &D_80160574;
        break;

    case 3:
        func_80145FC0();
        D_80165488 = D_80165488 + 1;
        break;

    case 4:
        do
        {
            poll_result = func_80146104();
        } while (poll_result == -1);
        if (poll_result == 0)
        {
            D_80165488 = D_80165488 + 1;
            break;
        }
        if (poll_result < 0)
        {
            break;
        }
        if (poll_result >= 4)
        {
            break;
        }
        phase_result = 4;
        D_801609B8 = 0;
        D_801609A4 = 0xFD;
        break;

    case 5:
        func_80146018();
        D_80165488 = D_80165488 + 1;
        break;

    case 6:
        addhero_probe_render_two();
        D_80164A60 = 1;
        if (func_80145B4C(D_801609A8) == 0)
        {
            phase_result = 2;
            D_80165488 = NULL;
            D_801609A4 = 0xF8;
            D_80164A60 = 0;
            break;
        }
        wait_attempts = 0;
        D_80165488 = D_80165488 + 1;
        do
        {
            if (func_80145C34(D_801609A8) == 0)
            {
                if (D_8016093C != 0)
                {
                    D_801609AC = 0;
                }
                D_80164A60 = 0;
                if (D_801609A4 == 0xF8)
                {
                    break;
                }
                if (D_801609A4 == 0xFA)
                {
                    break;
                }
                func_80145E14();
                break;
            }
            wait_attempts = wait_attempts + 1;
        } while (wait_attempts < 0x14);
        break;

    case 8:
        phase_result = 3;
        func_8001729C(D_801609A8);
        func_800172AC(D_801609A8 * 0x10);
        D_80165488 = D_80165488 + 1;
        break;

    case 9:
        phase_result = 3;
        func_8001729C(D_801609A8);
        func_8001725C(D_801609A8 * 0x10);
        D_80164A58 = 0x10;
        D_80164B08 = 0x10;
        D_80165488 = D_80165488 + 1;
        break;

    case 0:
        phase_result = 2;
        D_80165200 = 0;
        break;

    case 10:
        func_80016F9C(&buf, (u8 *)D_80164B60 + (D_801609A8 * 0x320) + (D_801609AC * 0x28));
        wait_attempts = 0;
        do
        {
            if (func_8001686C(&buf) != 0)
            {
                break;
            }
            wait_attempts = wait_attempts + 1;
        } while (wait_attempts < 0x14);
        D_80165488 = D_80165488 + 1;
        break;

    case 15:
        poll_result = func_80146070();
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
            goto c15_increment;
        }
        break;
    c15_ge3:
        if (poll_result == 3)
        {
            goto c15_eq3;
        }
        break;
    c15_increment:
        D_80165488 = D_80165488 + 1;
        break;
    c15_pos:
        D_80164B08 = D_80164B08 - 1;
        if (D_80164B08 != 0)
        {
            goto block_reissue;
        }
        phase_result = 4;
        D_801609B8 = 0;
        D_801609A4 = 0xFD;
        break;
    c15_eq3:
        D_80164A58 = D_80164A58 - 1;
        if (D_80164A58 == 0)
        {
            goto c15_d70zero;
        }
    block_reissue:
        func_8001729C(D_801609A8);
        func_800172AC(D_801609A8 * 0x10);
        func_8001729C(D_801609A8);
        func_8001725C(D_801609A8 * 0x10);
        break;
    c15_d70zero:
        phase_result = 5;
        D_801609A4 = 0xFC;
        D_80165488 = D_8016057C;
        break;

    case 16:
        do
        {
            poll_result = func_80146104();
        } while (poll_result == -1);
        D_80165488 = D_80165488 + 1;
        break;

    case 17:
        D_80164A40 = 1;
        D_801609B8 = 0;
        func_8001729C(D_801609A8);
        D_8016548C = func_8001680C(D_801654E0, 0x8001);
        if (D_8016548C == -1)
        {
            break;
        }
        func_80145FC0();
        func_8001729C(D_801609A8);
        if (func_8001681C(D_8016548C, &D_80165208,
                           D_80164A50 != 0 ? 0x280 : 0x80) == -1)
        {
            func_8001683C(D_8016548C);
            break;
        }
        D_80165488 = D_80165488 + 1;
        break;

    case 18:
        if (D_80164A40 != 0)
        {
            poll_result = func_80146070();
            if (poll_result == 0)
            {
                D_80164A40 = 0;
                D_801609B8 = 1;
                func_8001683C(D_8016548C);
                break;
            }
            if (poll_result == -1)
            {
                break;
            }
            func_8001683C(D_8016548C);
            D_801609A4 = 0xFF;
            D_80165488 = &D_80160574;
        }
        else
        {
            D_80165488 = D_80165488 + 1;
        }
        break;

    case 19:
        D_80160934 = 1;
        D_80164A54 = func_8002054C(-1);
        D_80164A4C = 1;
        func_8001729C(D_801609A8);
        D_8016548C = func_8001680C(D_801654E0, 0x8001);
        func_80145FC0();
        func_8001729C(D_801609A8);
        if (func_8001681C(D_8016548C, D_801609F0, 0x4000) == -1)
        {
            func_8001683C(D_8016548C);
            D_80164A48 = D_80164A48 - 1;
            if (D_80164A48 == 0)
            {
            block_dialog_read:
                func_80142B1C(1);
                break;
            }
            break;
        }
        D_80165488 = D_80165488 + 1;
        break;

    case 20:
        poll_result20 = func_80146070();
        if (poll_result20 == 0)
        {
            D_80160934 = 0;
            D_80165488 = D_80165488 + 1;
            func_8001683C(D_8016548C);
            break;
        }
        if (poll_result20 < 0)
        {
            break;
        }
        if (poll_result20 >= 4)
        {
            break;
        }
        func_8001683C(D_8016548C);
        D_80164A48 = D_80164A48 - 1;
        if (D_80164A48 == 0)
        {
            D_80164A4C = 0;
            goto block_dialog_read;
        }
        D_80165488 = D_80165488 - 1;
        break;

    case 24:
        wait_attempts = 0;
        do
        {
            if (func_800342CC(D_801609A8 * 0x10) == 1)
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
                D_80165488 = D_80165488 + 1;
                break;
            }
        }
        func_80142B1C(3);
        break;

    case 30:
        D_80164A48 = 5;
        D_80165488 = D_80165488 + 1;
        break;

    case 27:
        D_80160934 = 1;
        D_80164A54 = func_8002054C(-1);
        D_80164A4C = 1;
        func_8001729C(D_801609A8);
        D_8016548C = func_8001680C(D_801654E0, 0x8001);
        func_80145FC0();
        func_8001729C(D_801609A8);
        if (func_8001681C(D_8016548C, D_801609F0, 0x4000) == -1)
        {
            func_8001683C(D_8016548C);
            D_80164A48 = D_80164A48 - 1;
            if (D_80164A48 == 0)
            {
            block_dialog_write_read:
                func_80142C08(1);
                break;
            }
            break;
        }
        D_80165488 = D_80165488 + 1;
        break;

    case 28:
        poll_result20 = func_80146070();
        if (poll_result20 == 0)
        {
            D_80160934 = 0;
            D_80165488 = D_80165488 + 1;
            func_8001683C(D_8016548C);
            break;
        }
        if (poll_result20 < 0)
        {
            break;
        }
        if (poll_result20 >= 4)
        {
            break;
        }
        D_80164A48 = D_80164A48 - 1;
        if (D_80164A48 == 0)
        {
            D_80164A4C = 0;
            goto block_dialog_write_read;
        }
        goto block_close_decrement;

    case 25:
        if (D_80164B1C == 0)
        {
            func_8001729C(D_801609A8);
            wait_attempts = 0;
            do
            {
                if (func_8001686C(D_801654E0) != 0)
                {
                    break;
                }
                wait_attempts = wait_attempts + 1;
            } while (wait_attempts < 0x14);
        }
        func_80016F9C(&buf, D_800ECF9C);
        func_8001729C(D_801609A8);
        D_8016548C = func_8001680C(&buf, 0x20200);
        if (D_8016548C != -1)
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
        D_80164A48 = D_80164A48 - 1;
        if (D_80164A48 == 0)
        {
        block_dialog_write:
            func_80142C08(0);
            break;
        }
        break;

    block_write_opened:
        func_8001683C(D_8016548C);
        func_800170BC(D_80164B20, &buf);
        func_8001729C(D_801609A8);
        D_8016548C = func_8001680C(D_80164B20, 0x8002);
        func_80145FC0();
        D_80164A54 = func_8002054C(-1);
        D_80164A4C = 1;
        func_8001729C(D_801609A8);
        if (func_8001682C(D_8016548C, D_801609F0, 0x4000) == -1)
        {
            func_8001683C(D_8016548C);
            wait_attempts = 0;
            do
            {
                if (func_8001686C(D_80164B20) != 0)
                {
                    break;
                }
                wait_attempts = wait_attempts + 1;
            } while (wait_attempts < 0x14);
            goto block_write_retry;
        }
        D_80165488 = D_80165488 + 1;
        break;

    case 26:
        poll_result20 = func_80146070();
        if (poll_result20 != 0)
        {
            if (poll_result20 < 0)
            {
                break;
            }
            if (poll_result20 >= 4)
            {
                break;
            }
            goto block_case26_retry;
        }
        if (D_80164B1C != 0)
        {
            func_8001729C(D_801609A8);
            wait_attempts = 0;
            do
            {
                if (func_8001686C(D_801654E0) != 0)
                {
                    break;
                }
                wait_attempts = wait_attempts + 1;
            } while (wait_attempts < 0x14);
        }
        func_8001729C(D_801609A8);
        wait_attempts = 0;
        do
        {
            if (func_8001685C(D_80164B20, D_801654E0) != 0)
            {
                break;
            }
            wait_attempts = wait_attempts + 1;
        } while (wait_attempts < 0x14);
        D_80165200 = 0;
        D_80165488 = D_80165488 + 1;
        func_8001683C(D_8016548C);
        break;

    block_case26_retry:
        D_80164A48 = D_80164A48 - 1;
        if (D_80164A48 == 0)
        {
            D_80164A4C = 0;
            goto block_dialog_write;
        }
        goto block_close_decrement;

    default:
        break;
    }

    goto block_return;

block_close_decrement:
    func_8001683C(D_8016548C);
    D_80165488 = D_80165488 - 1;

block_return:
    return phase_result;
}

/** @see decomp.me (100.00%) */
void func_80145824(void)
{
    func_8001729C(D_801609A8);
    func_80145FC0();
    func_8001724C(D_801609A8 * 0x10);
    D_80165488 = D_8016057C;
}

/** @see decomp.me (100.00%) */
s32 func_80145878(void)
{
    s32 busy_slot;

    busy_slot = func_80146070();
    if (busy_slot != -1)
    {
        func_8001729C(D_801609A8);
        func_8001724C(D_801609A8 * 0x10);
    }
    return busy_slot;
}

/** @see decomp.me (100.00%) */
void func_801458D0(void)
{
    func_800158E0();
    func_800167EC();
    D_80164B0C = func_800167AC(0xF4000001, 4, 0x2000, 0);
    D_80164B10 = func_800167AC(0xF4000001, 0x8000, 0x2000, 0);
    D_80164B14 = func_800167AC(0xF4000001, 0x100, 0x2000, 0);
    D_80164B18 = func_800167AC(0xF4000001, 0x2000, 0x2000, 0);
    D_801651A0 = func_800167AC(0xF0000011, 4, 0x2000, 0);
    D_801651A4 = func_800167AC(0xF0000011, 0x8000, 0x2000, 0);
    D_801651A8 = func_800167AC(0xF0000011, 0x100, 0x2000, 0);
    D_801651AC = func_800167AC(0xF0000011, 0x2000, 0x2000, 0);
    func_800167DC(D_80164B0C);
    func_800167DC(D_80164B10);
    func_800167DC(D_80164B14);
    func_800167DC(D_80164B18);
    func_800167DC(D_801651A0);
    func_800167DC(D_801651A4);
    func_800167DC(D_801651A8);
    func_800167DC(D_801651AC);
    func_800167FC();
    D_80164A4C = 0;
    D_80164A60 = 0;
}

/** @see decomp.me (100.00%) */
void func_80145A9C(void)
{
    func_800158E0();
    func_800167EC();
    func_800167BC(D_80164B0C);
    func_800167BC(D_80164B10);
    func_800167BC(D_80164B14);
    func_800167BC(D_80164B18);
    func_800167BC(D_801651A0);
    func_800167BC(D_801651A4);
    func_800167BC(D_801651A8);
    func_800167BC(D_801651AC);
    func_800167FC();
}

/** @see decomp.me (100.00%) */
s32 func_80145B4C(s32 page)
{
    EntryHeader7 buf;

    memcpy(&buf, &D_80140114, 7);
    D_801609AC = 0;
    D_801609BC = 0;
    D_80160938 = 0;
    D_80160928 = 0;
    D_801609A4 = 0;
    ((u8 *)&buf)[2] += page;
    if (func_80016BCC(&buf, (u8 *)D_80164B60 + page * 0x320) != 0)
    {
        func_800B0170((u8 *)D_80164B60 + page * 0x320 + D_801609A4 * 0x28);
        D_801609A4 += 1;
        return 1;
    }
    return 0;
}

/**
 * @brief Advance one step of the add-hero entry load scan for the given page.
 * @param page Page index whose entry block is being scanned.
 * @return 1 if an entry was consumed this step, 0 otherwise.
 * @see decomp.me (100.00%)
 */
s32 func_80145C34(s32 page)
{
    s32 i;
    s32 sum;
    s32 offset;
    s32 selected;
    s32 page_offset;
    s32 count;
    s32 cond;

    page_offset = page * 0x320;
    if (func_8001684C((void *)((u8 *)D_80164B60 + page_offset + D_801609A4 * 0x28)) != 0)
    {
        func_800B0170((void *)((u8 *)D_80164B60 + page_offset + D_801609A4 * 0x28));
        D_801609A4 += 1;
        return 1;
    }

    func_800AA02C();
    if ((D_8016093C == 0) && (func_80144A28() == 0))
    {
        D_801609A4 = 0xF8;
    }
    else
    {
        i = 0;
        sum = 0;
        D_80164B1C = 0;
        count = D_801609A4;
        if (count > 0)
        {
            u8 *entries;
            do { entries = (u8 *)D_80164B60; } while (0);
            offset = D_801609A8 * 0x320;
            do
            {
                sum += ((AddheroDirEntry *)(offset + (s32)entries))->size / 8192;
                i++;
                offset += 0x28;
            } while (i < count);
        }
        cond = sum >= 0xE;
        if (cond != 0)
        {
            selected = func_801447B4(sum, i, count);
            if (func_80144A28() == 0)
            {
                D_801609A4 = 0xFA;
                D_80164A5C = 0;
            }
            else
            {
                if (D_8016093C != 0)
                {
                    D_801609AC = 0;
                }
                D_801609AC = selected;
                func_80140CFC();
            }
        }
        else
        {
            D_80164B1C = 1;
            selected = func_801447B4(sum, i, count);
            if (func_80144A28() == 0)
            {
                D_801609AC = 0;
                func_80140CFC();
                D_80164A5C = 0;
            }
            else
            {
                if (D_8016093C != 0)
                {
                    D_801609AC = 0;
                }
                D_801609AC = selected;
                func_80140CFC();
            }
        }
    }
    return 0;
}

/** @see decomp.me (100.00%) */
void func_80145E14(void)
{
    AddheroFileHeader local;
    u8 *p;

    if (D_801609A4 == 0)
    {
        D_801609B8 = 3;
        return;
    }
    {
        s32 term1;
        s32 term2;
        term1 = D_801609A8 * 0x320;
        term2 = (D_801609AC * 0x28) + (s32)D_80164B60;
        if (func_8001714C(&D_800ECFC4[0], (void *)(term1 + term2), 8) == 0)
        {
            D_801609B8 = 2;
            return;
        }
    }
    memcpy(&local, &D_80140090, 6);
    p = (u8 *)&local;
    {
        s32 term1;
        s32 term2;
        term1 = D_801609A8 * 0x320;
        term2 = (D_801609AC * 0x28) + (s32)D_80164B60;
        func_80016F9C(p, (void *)(term1 + term2));
    }
    {
        s32 slot;
        s32 value;
        value = *((u8 *)&local + 2);
        slot = (u8)D_801609A8;
        D_801609B8 = 0;
        value += slot;
        *((u8 *)&local + 2) = value;
        func_800170BC(&D_801654E0[0], p, slot);
    }
    D_80165488 = &D_8016058C[0];
    {
        s32 term1;
        s32 term2;
        term1 = D_801609A8 * 0x320;
        term2 = (D_801609AC * 0x28) + (s32)D_80164B60;
        if (func_8001714C(&D_800ECF7C[0], (void *)(term1 + term2), 0xC) == 0)
            D_80164A50 = 1;
        else
            D_80164A50 = 0;
    }
    D_80164A40 = 1;
}

/** @see decomp.me (100.00%) */
void func_80145FC0(void)
{
    func_800167CC(D_80164B0C);
    func_800167CC(D_80164B10);
    func_800167CC(D_80164B14);
    func_800167CC(D_80164B18);
}

/** @see decomp.me (100.00%) */
void func_80146018(void)
{
    func_800167CC(D_801651A0);
    func_800167CC(D_801651A4);
    func_800167CC(D_801651A8);
    func_800167CC(D_801651AC);
}

/** @see decomp.me (100.00%) */
s32 func_80146070(void)
{
    if (func_800167CC(D_80164B0C) == 1)
    {
        return 0;
    }
    if (func_800167CC(D_80164B10) == 1)
    {
        return 1;
    }
    if (func_800167CC(D_80164B14) == 1)
    {
        return 2;
    }
    if (func_800167CC(D_80164B18) == 1)
    {
        return 3;
    }
    return -1;
}

/** @see decomp.me (100.00%) */
s32 func_80146104(void)
{
    if (func_800167CC(D_801651A0) == 1)
    {
        return 0;
    }
    if (func_800167CC(D_801651A4) == 1)
    {
        return 1;
    }
    if (func_800167CC(D_801651A8) == 1)
    {
        return 2;
    }
    if (func_800167CC(D_801651AC) == 1)
    {
        return 3;
    }
    return -1;
}

void func_80146198(void)
{
    AddheroEntry28 sorted[20];
    s32 out = 0;
    s32 group = 0;
    s32 i;
    do {
        i = 0;
        if (i < D_801609A4) {
            do {
                if (D_801651B0[i] == group &&
                    func_8001714C(D_800ECF7C, &D_80164B60[D_801609A8][i], 0xC) == 0) {
                    func_80016E7C(&D_80164B60[D_801609A8][i], &sorted[out], 0x28);
                    out++;
                }
                i++;
            } while (i < D_801609A4);
        }
        group++;
    } while (group < 8);

    group = 0;
    do {
        i = 0;
        if (i < D_801609A4) {
            do {
                if (D_801651B0[i] == group &&
                    func_8001714C(D_800ECF8C, &D_80164B60[D_801609A8][i], 0xC) == 0) {
                    func_80016E7C(&D_80164B60[D_801609A8][i], &sorted[out], 0x28);
                    out++;
                }
                i++;
            } while (i < D_801609A4);
        }
        group++;
    } while (group < 8);

    i = 0;
    if (D_801609A4 > 0) {
        do {
            if (func_8001714C(D_800ECFC4, &D_80164B60[D_801609A8][i], 8) == 0) {
                func_80016E7C(&D_80164B60[D_801609A8][i], &sorted[out], 0x28);
                out++;
            }
            i++;
        } while (i < D_801609A4);
    }

    if (*(volatile s32 *)&D_801609A4 > 0) {
        i = 0;
        do {
            if (func_8001714C(D_800ECF7C, &D_80164B60[D_801609A8][i], 0xC) != 0 &&
                func_8001714C(D_800ECF8C, &D_80164B60[D_801609A8][i], 0xC) != 0 &&
                func_8001714C(D_800ECFC4, &D_80164B60[D_801609A8][i], 8) != 0) {
                func_80016E7C(&D_80164B60[D_801609A8][i], &sorted[out], 0x28);
                out++;
            }
            i++;
        } while (i < D_801609A4);
    }

    i = 0;
    if (D_801609A4 > 0) {
        do {
            func_80016E7C(&sorted[i], &D_80164B60[D_801609A8][i], 0x28);
            i++;
        } while (i < D_801609A4);
    }
}

s32 func_801465C8(s32 prim, s32 *ot, s32 value, s32 x, s32 y, s32 palette, s32 alignment)
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
    buf[1] = D_801608E0[magnitude / 10000];
    buf[2] = D_801608E0[(magnitude % 10000) / 1000];
    buf[3] = D_801608E0[(magnitude % 1000) / 100];
    buf[4] = D_801608E0[(magnitude % 100) / 10];
    buf[5] = D_801608E0[magnitude % 10];

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
    prim = func_8014686C(prim, ot, (u8 *)&buf[first_digit], x, y, palette, alignment);
    return prim;
}

void func_801467E8(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5)
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
    base = D_801608F8;
    pair[0] = *(u16 *)((u8 *)base + off);
    off = (arg2 - row * 16) * 2;
    pair[1] = *(u16 *)((u8 *)base + off);
    pair[2] = 0;
    func_8014686C(arg0, arg1, pair, arg3, arg4, 0, arg5);
}

s32 func_8014686C(s32 prim, s32 *ot, u8 *text, s32 x, s32 y, s32 palette, s32 alignment)
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
    D_8016D930 = x;
    D_8016D928 = x;
    D_8016D92C = y;

    while (1)
    {
        u32 lead = *cursor;

        if ((u8)lead == 0x20)
        {
            cursor++;
            D_8016D928 += 0x10;
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
        prim = func_80146A3C(prim, ot, code, palette);
    }

    setlen(prim, 1);
    ((GenericGpuPacket *)prim)->word4 = 0xE1000005;
    addPrim(ot, prim);
    return prim + 8;
}

s32 func_80146A3C(s32 prim, s32 *ot, s32 character_code, s32 palette)
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
    entry = D_8016D528;

    while (slot < GLYPH_CACHE_SLOTS)
    {
        if (requested_code == entry->data.code)
        {
            return func_80146C5C((GlyphSprite *)prim, ot, slot, palette);
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

    raster = D_8016D93C;
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
    while ((slot < GLYPH_CACHE_SLOTS) && (D_8016D528[slot].raw != 0))
    {
        slot++;
    }

    if (slot == GLYPH_CACHE_SLOTS)
    {
        return prim;
    }
    D_8016D528[slot].raw = code & 0xFFFF;
    prim = func_80146C5C((GlyphSprite *)prim, ot, slot, palette);

    D_8016D934 = (slot % GLYPH_CACHE_COLUMNS) * 4;
    D_8016D938 = slot & GLYPH_CACHE_ROW_MASK;

    rect.w = 4;
    rect.h = 15;
    rect.x = D_8016D934 + 0x140;
    rect.y = D_8016D938;

    func_80019A34(&rect, D_8016D93C);
    func_80019788(0);

    D_8016D93C += GLYPH_RASTER_BYTES;
    return prim;
}

s32 func_80146C5C(GlyphSprite *sprite, s32 *ot, s32 cache_slot, s32 palette)
{
    u32 ot_tag_high_byte;
    s32 normalized_slot;
    u32 packet_address;
    s32 old_x;
    s32 new_x;
    s32 fits_line;

    D_8016D528[cache_slot].raw |= 0x10000;

    setlen(sprite, 3);
    setcode(sprite, 0x7C);
    sprite->packet.g0 = 0x80;
    sprite->packet.b0 = 0x80;
    sprite->packet.r0 = 0x80;
    normalized_slot = cache_slot;
    sprite->packet.x0 = D_8016D928;
    sprite->packet.y0 = D_8016D92C;

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
    old_x = D_8016D928;
    new_x = old_x + 16;
    fits_line = (old_x + 32) < 0x280;
    D_8016D928 = new_x;

    *ot = ot_tag_high_byte | packet_address;

    if (!fits_line)
    {
        D_8016D928 = D_8016D930;
        D_8016D92C += 16;
    }

    return (s32)sprite;
}

/** @see decomp.me (100.00%) */
void func_80146D64(void)
{
    s32 i;
    s32 *p;

    D_8016D93C = D_80165528;
    i = 0;
    p = (s32 *)D_8016D528;
    do
    {
        *p = (u16)*p;
        i++;
        p++;
    } while (i < 0x100);
}

/** @see decomp.me (100.00%) */
void func_80146DA0(void)
{
    s32 i;
    s32 *p;
    s32 flag;

    i = 0;
    flag = 0x10000;
    p = (s32 *)D_8016D528;
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
void func_80146DE0(void)
{
    s32 i;
    s32 *p;
    u8 *q;

    i = 0xFF;
    p = (s32 *)D_8016D528;
    p += 0xFF;
    do
    {
        *p = 0;
        i--;
        p--;
    } while (i >= 0);

    i = 0;
    q = D_80165528;
    do
    {
        *(u8 *)(i + (s32)q) = 0;
        i++;
    } while (i <= 0x7FFF);
}

void func_80146E30(u8 *out, u8 *in)
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
            pa = D_8015D3B4 + b1 * 2;
            pa += off * 33;
            lead = *in;
            pa += lead * 528;
            *out = *pa;
            out++;
            b1 = in[1];
            off = b1 >> 4;
            b1 &= 0xF;
            pb = D_8015D3B4 + 1 + b1 * 2;
            pb += off * 33;
            lead = *in;
            pb += lead * 528;
            *out = *pb;
            out++;
            in += 2;
        }
        else if ((u8)c >= 0x21)
        {
            lead = *in;
            index = lead - 0x20;
            *out = D_801605A8[(index / 16) * 33 + (index & 0xF) * 2];
            out++;
            lead = *in;
            index = lead - 0x20;
            *out = D_801605A8[(index / 16) * 33 + (index & 0xF) * 2 + 1];
            out++;
            in += 1;
        }
        else
        {
            *out = D_801605A8[0];
            out++;
            *out = D_801605A8[1];
            out++;
            in += 1;
        }
    }
done:
    *out = 0;
}
