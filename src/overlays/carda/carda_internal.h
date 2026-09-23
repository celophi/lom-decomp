#ifndef CARDA_INTERNAL_H
#define CARDA_INTERNAL_H

#include "field_text.h"
#include "common.h"
#include "vector.h"
#include "gpu_packet.h"
#include "sdk/libgte.h"
#include "sdk/libgpu.h"

/**
 * @brief One animated CARDA UI element (a framed window plus its draw callback).
 *
 * Eight of these form the element pool starting at D_80165F80.  attr and attr4
 * are each readable as a whole word or through their bitfields.
 */
typedef struct CardaElement
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
    union
    {
        u32 word;
        struct
        {
            u32 unk4_0 : 1;
            u32 y : 8;
            u32 unk4_9 : 23;
        } f;
    } attr4;
    void *draw_handler;
} CardaElement;

/** @brief Replace the top byte (the element code) of a CardaElement's attr word. */
#define SET_ELEM_CODE(e, c) ((e)->attr.word = ((e)->attr.word & 0x00FFFFFF) | ((u32)(c) << 24))

/** @brief Resolve a glyph pointer from a symbol whose value stores a 16-bit offset. */
#define GLYPH_SYM(sym, off) ((void *)(((u8 *)&(sym) - (off)) + (sym)))

/** @brief Resolve a glyph pointer from a base pointer plus a 16-bit offset field. */
#define GLYPH_OFF(base, off) ((void *)((base) + *(u16 *)((base) + (off))))

/** @brief Memory-card directory entry; layout matches Psy-Q struct DIRENTRY. */
typedef struct CardaDirEntry
{
    /* 0x00 */ char name[20];
    /* 0x14 */ s32 attr;
    /* 0x18 */ s32 size;
    /* 0x1C */ void *next;
    /* 0x20 */ s32 head;
    /* 0x24 */ char system[4];
} CardaDirEntry;

/** @brief Number of directory entries per memory card. */
#define CARDA_ENTRIES_PER_CARD 20

/** @brief Byte size of one card's directory listing in D_80166440. */
#define CARDA_CARD_DIRECTORY_BYTES 0x320

/** @brief Byte size of one directory entry. */
#define CARDA_DIRECTORY_ENTRY_BYTES 0x28

/** @brief Six-byte memory-card path buffer ("bu00:" plus terminator), byte aligned. */
typedef struct
{
    u8 raw[6];
} CardaFileHeaderScratch;

/** @brief 0x20-byte, word-aligned memory-card path scratch buffer. */
typedef struct
{
    s32 unk0;
    s16 unk4;
    u8 pad[0x1A];
} CardaLoadScratch;

/** @brief Eight-byte, word-aligned memory-card path template ("bu00:"). */
typedef union
{
    char text[8];
    u32 align[2];
} CardaCardPathTemplate;

/** @brief Three-byte Shift-JIS separator copied into the save title. */
typedef struct
{
    s8 b0;
    s8 b1;
    s8 b2;
} Char3;

/** @brief Word-aligned buffer initialized from the "bu00:" search path. */
typedef struct EntryHeader7
{
    s32 unk0;
    s16 unk4;
    s8 unk6;
    u8 pad[9];
} EntryHeader7;

/**
 * @brief One 4-byte glyph-cache slot: the cached character code plus per-frame
 *        usage flags, also read as a single word when scanning for a free slot.
 */
typedef union
{
    u32 raw;
    struct
    {
        u16 code;
        u16 flags;
    } data;
} GlyphCacheEntry;

/** @brief 0x14-byte glyph packet: a Psy-Q SPRT_16 plus a trailing padding word. */
typedef struct
{
    SPRT_16 packet;
    u32 padding;
} GlyphSprite;

/* FIELD / main-executable globals used by this overlay. */
extern s32 D_8003EC9C;
extern s32 D_80042FB4;
extern u8 *D_8012271C;
extern s32 D_801227C4;
extern s32 D_80122988;
extern s32 D_801229B0;
extern s32 g_field_card_overlay_mode;
extern s32 g_menu_element_counter;

/* FIELD UI strings and memory-card file names. */
extern u8 D_800EC3D0[];
extern u8 D_800EC3F6[2];
extern u8 D_800EC3FA;
extern char D_800ECF7C[];
extern char D_800ECF8C[];
extern char D_800ECF9C[];
extern char D_800ECFB0[];
extern char D_800ECFC4[];
extern char D_800ECFD0[];

/* CARDA read-only data. */
extern Char3 D_8014008C;
extern Char3 D_80140090;
extern const CardaFileHeaderScratch D_801400C4;
extern const CardaCardPathTemplate D_801401C0;
extern EntryHeader7 D_80140244;

/* CARDA text offset-table entries. */
extern u16 D_8014B038;
extern u16 D_8014B03A;
extern u16 D_8014B03C;
extern u16 D_8014B03E;
extern u16 D_8014B040;
extern u16 D_8014B042;
extern u16 D_8014B044;
extern u16 D_8014B046;
extern u16 D_8014B048;
extern u16 D_8014B04A;
extern u16 D_8014B04C;
extern u16 D_8014B04E;
extern u16 D_8014B050;
extern u16 D_8014B054;
extern u16 D_8014B058;
extern u16 D_8014B05E;
extern u16 D_8014B060;
extern u16 D_8014B064;
extern u16 D_8014B066;
extern u16 D_8014B068;
extern u16 D_8014B06A;
extern u16 D_8014B06C;
extern u16 D_8014B072;
extern u16 D_8014B074;
extern u16 D_8014B076;
extern u16 D_8014B078;
extern u16 D_8014B07A;
extern u16 D_8014B08C;
extern u16 D_8014B090;
extern u16 D_8014B092;
extern u16 D_8014B094;
extern u16 D_8014B09C;
extern u16 D_8014B09E;
extern u16 D_8014B0AC;
extern u16 D_8014B0AE;
extern u16 D_8014B0B0;
extern u16 D_8014B0B4;
extern u16 D_8014B0B6;
extern u16 D_8014B0BA;
extern u16 D_8014B0BE;
extern u16 D_8014B0C2;
extern u16 D_8014B0C6;
extern u16 D_8014B0CA;
extern u16 D_8014B0CC;
extern u16 D_8014B0D2;
extern u16 D_8014B0D4;
extern u16 D_8014B0D6;
extern u16 D_8014B0D8;
extern u16 D_8014B0DA;
extern u16 D_8014B0E6;
extern u16 D_8014B0E8;
extern u16 D_8014B0EC;
extern u16 D_8014B4D4[];
extern u8 D_8014BECC[];
extern u8 D_8014BEE4[];
extern s32 D_8014BF00[];
extern u16 D_8014CA6C[];
extern s32 D_8014CC54[];
extern u8 D_801629D0[];

/* Card-sequence step scripts (D_801663A0 points into these). */
extern u8 D_80165B70[];
extern u8 D_80165B78[];
extern u8 D_80165B7C[];
extern u8 D_80165B84[];
extern u8 D_80165B88[];
extern u8 D_80165B89[];
extern u8 D_80165B90[];
extern u8 D_80165B91[];
extern u8 D_80165BA0[];
extern u8 D_80165BA4[];
extern u8 D_80165BA8[];
extern u8 D_80165BAC[];
extern u8 D_80165BB8[];
extern u8 D_80165BBD[];
extern u8 D_80165BC4[];

/* CARDA state. */
extern u16 D_80165EFC[];
extern u16 D_80165F14[];
extern s32 D_80165F38;
extern s32 D_80165F3C;
extern s32 D_80165F40;
extern u8 D_80165F48[];
extern s32 D_80165F7C;
extern CardaElement D_80165F80[8]; /**< UI element pool. */
extern CardaElement D_80165F8C;
extern s32 D_80165FE0;
extern s32 D_80165FE4;
extern s32 D_80165FE8;
extern s32 D_80165FEC;
extern u8 *D_80165FF0;
extern s32 D_80165FF4;
extern s32 D_80165FF8;
extern s32 D_80165FFC;
extern s32 D_80166000;
extern s32 D_80166004;
extern u8 D_80166008[];
extern s32 D_80166068;
extern s32 D_8016606C;
extern s32 D_80166070;
extern s32 D_80166074;
extern s32 D_80166078;
extern u8 D_80166080[];
extern s32 D_801660A0;
extern s32 D_801660F8;
extern s32 D_801660FC;
extern s32 D_80166100;
extern s32 D_80166104;
extern s32 D_80166108;
extern s32 D_8016610C;
extern s32 D_80166110;
extern s32 D_80166114;
extern s32 D_80166118;
extern u8 D_80166120[];
extern u8 D_80166124[];
extern u8 D_801662A0[];
extern u8 D_8016636F;
extern u8 *D_801663A0;
extern s32 D_801663A4;
extern s32 D_801663A8[];
extern CardaFileHeaderScratch D_801663F8;
extern s32 D_80166438;
extern s32 D_8016643C;
extern CardaDirEntry D_80166440[2][CARDA_ENTRIES_PER_CARD]; /**< Directory listing of both cards. */
extern s32 D_80166A80[];
extern s32 D_80166AD0;
extern s32 D_80166AD4;
extern s32 D_80166AD8;
extern s32 D_80166ADC;
extern s32 D_80166AE0;
extern s32 D_80166AE8[];
extern s32 D_80166B88;
extern s32 D_80166B8C;
extern s32 D_80166B90;
extern s32 D_80166B94;
extern s32 D_80166B98;
extern s32 D_80166B9C;
extern s32 D_80166BA0;
extern u8 D_80166BA8[];
extern GlyphCacheEntry D_80166BE8[];
extern s32 D_80166FE8;
extern s32 D_80166FEC;
extern s32 D_80166FF0;
extern s32 D_80166FF4;
extern s32 D_80166FF8;
extern u8 *D_80166FFC;
extern u8 D_80167000[];

/*
 * External callees.  The ones declared with an empty parameter list were
 * called without a prototype in the original sources and must stay that way.
 */
void field_reset_input_repeat(void);
s32 func_80013F2C();
s32 func_800141EC();
s32 func_800158E0();
s32 func_80016764();
s32 func_800167AC(s32, s32, s32, s32);
void func_800167BC(s32);
s32 func_800167CC(s32);
void func_800167DC(s32);
void func_800167EC(void);
void func_800167FC(void);
s32 func_8001680C(void *, s32);
s32 func_8001681C(s32, void *, s32);
s32 func_8001682C(s32, void *, s32);
s32 func_8001683C(s32);
s32 func_8001684C();
s32 func_8001685C(void *, void *);
s32 func_8001686C(void *);
s32 func_8001687C(s32);
s32 func_80016BCC();
void func_80016E7C();
s32 func_80016F5C();
s32 func_80016F9C(void *, void *);
s32 func_800170BC(void *, void *, ...);
s32 func_8001714C();
s32 func_8001724C(s32);
s32 func_8001725C(s32);
s32 func_8001726C(s32, s32, void *);
s32 func_8001727C(s32, s32, void *);
s32 func_8001729C(s32);
s32 func_800172AC(s32);
s32 func_80017B3C();
void func_80019788(s32);
void func_80019A34(RECT *, void *);
s32 func_8001A5D4();
s32 func_8001C56C();
s32 func_8002054C(s32);
s32 func_80032174(s32, void *, s32 *);
s32 func_80033E7C(s32);
s32 func_800342CC(s32);
s32 func_80034648(s32, s32, s32);
s32 func_80067EB4();
void func_80067F28(void);
void func_80067F5C();
void func_800A3938(s32, s32);
void func_800A55E4(void *buf, s32 arg1);
void func_800A5638(void *buf, s32 arg1);
s32 func_800A88A0();
s32 func_800A8A78();
s32 func_800AD850();
s32 func_800AE76C();
void func_800B0170();
void func_800C1230(s32 slot);

/*
 * CARDA functions.  The ones declared with an empty parameter list are called
 * before their definition with arguments that do not match it (or used to size
 * the outgoing-argument area), so they must not get a prototype here.
 */
s32 func_80140370(s32 arg0);
void func_801403FC(void);
void func_80140830();
s32 func_80140918();
s32 func_80140BAC();
void func_801410E4();
void func_80141164();
void func_801411CC();
void func_80141230();
s32 func_80141250(s32 *ot, s32 prim, s32 x_offset, s32 y_offset);
s32 func_80141B50(s32 *ot, s32 prim, s32 x_offset, s32 y_offset);
s32 func_80141C3C(s32 *ot, s32 prim, s32 arg2, s32 arg3);
s32 func_80141D18(s32 *ot, s32 prim, s32 arg2, s32 arg3);
s32 func_80141DF4(s32 *ot, s32 prim, s32 x_offset, s32 y_offset);
void func_80142508(void *arg0);
s32 func_8014256C(s32 *ot, s32 prim, s32 arg2, s32 arg3);
void func_801425D4();
CardaElement *func_80142614();
void func_80142668();
void func_80142CA4(void);
void func_80142CBC(u8 *arg0, u8 *arg1);
s32 func_80142D40(u8 *arg0);
void func_80142D8C(u8 *arg0, u8 *arg1);
void func_80142E10(void);
u8 *func_80143334(void *arg0);
s32 func_80143380(void);
s32 func_801433C0(u8 *base);
s32 func_80143414(u8 *data);
s32 func_8014344C(s32 *ot, s32 prim, s32 arg2, s32 arg3);
s32 func_8014366C(s32 *ot, s32 prim, s32 arg2, s32 arg3);
s32 func_8014385C(s32 arg0, s32 *arg1);
s32 func_801439B4(s32 *ot, s32 prim, s32 arg2, s32 arg3);
s32 func_80143BD4(s32 *ot, s32 prim, s32 arg2, s32 arg3);
s32 func_80143DF4(s32 *ot, s32 prim, s32 arg2, s32 arg3);
s32 func_80143F90(s32 *ot, s32 prim, s32 arg2, s32 arg3);
s32 func_80144050(s32 *ot, s32 prim, s32 arg2, s32 arg3);
s32 func_801443F0(s32 *ot, s32 prim, s32 arg2, s32 arg3);
void func_801447DC(u32 arg0);
s32 func_80144A24(s32 *ot, s32 prim, s32 x, s32 y);
s32 func_80144CD0();
void func_80144F18();
s32 func_80144F28(s32 prim, s32 *ot, s32 x, s32 y);
s32 func_80145050(s32 *ot, s32 prim, s32 x_offset, s32 y_offset);
void func_80146694(void);
void func_801466F8(void);
s32 func_80146794(s32 prim, s32 *ot, s32 arg2, s32 arg3);
void func_8014697C(s32 arg0);
s32 func_80146AF0(s32 *ot, s32 prim, s32 arg2, s32 arg3);
void func_80146CA4(void);
s32 func_80146E80(s32 *ot, s32 prim, s32 x_offset, s32 y_offset);
s32 func_80146EDC(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
void func_80147100(void);
s8 *func_801471E4(s8 *out, s32 value);
void func_801472F8(s8 *out, s32 value, s32 max_chars);
void func_801473C0(s8 *out, s32 value);
u32 func_801473E4(u8 *s, s32 len);
s32 func_80147490(u8 *text);
s32 func_80147588(void);
s32 func_801477CC(void);
void func_80147C5C(void);
s32 func_80147C94(void);
s32 func_80147DCC(void);
void func_80147E88(void);
s32 func_80147F4C();
void func_80149554(void);
void func_801495E4();
s32 func_80149638(void);
void func_80149690(void);
void func_8014986C(void);
s32 func_8014991C(s32 page);
s32 func_80149A4C(s32 page);
void func_80149DF4(void);
void func_80149FEC(void);
void func_8014A044(void);
s32 func_8014A09C(void);
s32 func_8014A130(void);
void func_8014A1C4(void);
s32 func_8014A65C(s32 prim, s32 *ot, s32 value, s32 x, s32 y, s32 palette, s32 alignment);
void func_8014A87C(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5);
s32 func_8014A900(s32 prim, s32 *ot, u8 *text, s32 x, s32 y, s32 palette, s32 alignment);
s32 func_8014AAD0(s32 prim, s32 *ot, s32 character_code, s32 palette);
s32 func_8014ACF0(GlyphSprite *sprite, s32 *ot, s32 cache_slot, s32 palette);
void func_8014ADF8(void);
void func_8014AE34(void);
void func_8014AE74(void);
void func_8014AEC4(u8 *out, u8 *in);

#endif /* CARDA_INTERNAL_H */
