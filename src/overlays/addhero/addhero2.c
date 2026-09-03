#include "common.h"
#include "gpu_packet.h"
#include "sdk/libgte.h"
#include "sdk/libgpu.h"

/* ------------------------------------------------------------------ */
/* Shared types                                                       */
/* ------------------------------------------------------------------ */

typedef struct
{
    u8 data[0x28];
} AddheroEntry28;

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

extern u8 D_80160574[];
extern u8 D_8016057C[];
extern u8 D_8016058C[];
extern u8 D_801605A8[];
extern u8 D_8015D3B4[];
extern u8 D_801609F0[];
extern u8 D_80164B20[];
extern u8 D_80165208[];
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
/* Functions                                                          */
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
        D_80165488 = D_80160574;
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
        if (func_8001681C(D_8016548C, D_80165208,
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
            D_80165488 = D_80160574;
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
