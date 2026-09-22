#include "common.h"
#include "vector.h"

typedef struct
{
    u16 id;
    u16 count;
    s32 value;
} ShopEntry;

extern u8 D_800EC3E2[];
extern u8 D_800EC3F2[];
extern u8 D_800EC3F4[];
extern void *D_8012271C;
extern s32 D_80142D08;
extern s32 D_80142D0C;
extern s32 D_80142D10;
extern s32 D_80145244;
extern s32 D_80145250;
extern s32 D_80145CDC;

extern s32 func_800A88A0(s32 prim, s32 *ot, void *text, s32 color, s32 x, s32 y, s32 mode);
extern s32 func_800A8A78(s32 *ot, s32 prim, s32 value, s32 color, Vec2s *pos, s32 mode);
extern void func_800A8B90(void *dst, s32 value, s32 arg2);
extern void func_80142130(u8 *dst, u8 *src);
extern void func_80142200(u8 *dst, u8 *src);

#define SEL_ENTRY ((D_80145CDC * 8) + D_80145250)
#define SEL_REC ((u8 *)((((*(u16 *)SEL_ENTRY) & 0x7FFF) << 6) + D_80145244))

/**
 * @brief Draw the detail rows for the currently selected shop entry.
 *
 * Reads the selected entry (index @ref D_80145CDC into the @ref D_80145250
 * table). Entries with the high bit set are items: their record (base
 * @ref D_80145244) selects one of three detail layouts from bits 8-9 of
 * field 0x14. Non-item entries take a shorter fallback layout. Glyph strings
 * are assembled from the D_80142Dxx / D_800EC3xx tables and emitted through
 * the shared text primitives.
 *
 * @param ot   Ordering-table pointer used for emitted primitives.
 * @param prim Primitive-buffer write cursor.
 * @param xoff Horizontal layout offset (subtracted from every x).
 * @param yoff Vertical layout offset (subtracted from every row y).
 * @return Advanced primitive-buffer write cursor.
 *
 * @note Two instruction-order differences remain: the initial archive-table
 *       address setup and the separator append destination setup.
 * @see decomp.me (99.09%) TODO: no scratch link yet
 */
s32 func_801419D4(s32 *ot, s32 prim, s32 xoff, s32 yoff)
{
    Vec2s pos;
    u8 sp28buf[0x30];
    u8 sp58buf[0x30];
    u8 sp88buf[0x30];
    u16 *entry;
    u16 id;
    s32 *archive;
    u8 *name_buffer;

    entry = (u16 *)SEL_ENTRY;
    id = *entry;
    if (id != 0xFFFF)
    {
        name_buffer = sp28buf;
        if (id & 0x8000)
        {
            u8 *tbl_c = (u8 *)&D_80142D0C;
            s32 base_c = (s32)tbl_c - 8;
            u8 *firstrec = (u8 *)(((id & 0x7FFF) << 6) + D_80145244);
            u8 *glyph = D_800EC3E2;
            u8 *s3base = glyph - 0x1E;
            u32 disc14;

            func_80142200(name_buffer,
                (u8 *)(D_80142D0C + (*(u16 *)(((*(u16 *)(firstrec + 0x16) & 0x3F) * 2) + D_80142D0C + base_c) + base_c)));
            func_80142130(sp28buf,
                (u8 *)(D_800EC3E2[0] + ((glyph[1] << 8) + (s32)s3base)));

            disc14 = *(u32 *)(SEL_REC + 0x14);
            switch ((disc14 >> 8) & 3)
            {
            case 0:
            {
                s32 tblptr = *(s32 *)(base_c + 0xC);
                s32 x0;
                s32 y0;
                s32 temp_s4;

                func_80142130(sp28buf,
                    (u8 *)(tblptr + (*(u16 *)(((disc14 >> 9) & 0x7E) + tblptr + base_c) + base_c)));
                y0 = 0x12 - yoff;
                prim = func_800A88A0(prim, ot,
                    (void *)(s3base[0x2A] + ((s3base[0x2B] << 8) + (s32)s3base)), 4, 0x10 - xoff, y0, 0);
                x0 = 0x70 - xoff;
                pos.x = x0;
                pos.y = y0;
                prim = func_800A8A78(ot, prim, *(u16 *)(SEL_REC + 0x24), 4, &pos, 1);
                temp_s4 = *(u16 *)(SEL_REC + 0x24) - *(u16 *)((u8 *)D_8012271C + 0x664);
                func_80142200(sp58buf,
                    (u8 *)(s3base[0x3E] + ((s3base[0x3F] << 8) + (s32)s3base)));
                if (temp_s4 >= 0)
                {
                    func_80142130(sp58buf,
                        (u8 *)(s3base[0x16] + ((s3base[0x17] << 8) + (s32)s3base)));
                }
                func_800A8B90(sp88buf, temp_s4, 0);
                func_80142130(sp58buf, sp88buf);
                func_80142130(sp58buf,
                    (u8 *)(s3base[0x40] + ((s3base[0x41] << 8) + (s32)s3base)));
                pos.x = x0;
                pos.y = y0;
                prim = func_800A88A0(prim, ot, sp58buf, 4, pos.x, pos.y, 0);
                break;
            }
            case 1:
            {
                s32 tblptr = *(s32 *)(base_c + 0xC);
                s32 y0;
                s32 temp_a1;
                u8 *rec;

                func_80142130(sp28buf,
                    (u8 *)(tblptr + (*(u16 *)((u8 *)&D_80142D0C + (((disc14 >> 9) & 0x7E) + tblptr) + 0xE) + base_c)));
                y0 = 0x12 - yoff;
                temp_a1 = func_800A88A0(prim, ot,
                    (void *)(s3base[0x2C] + ((s3base[0x2D] << 8) + (s32)s3base)), 4, 0x10 - xoff, y0, 0);
                pos.x = 0x74 - xoff;
                pos.y = y0;
                rec = SEL_REC;
                prim = func_800A8A78(ot, temp_a1,
                    *(u16 *)(rec + 0x24) + *(u16 *)(rec + 0x26) + *(u16 *)(rec + 0x28) + *(u16 *)(rec + 0x2A),
                    4, &pos, 0);
                break;
            }
            default:
            {
                s32 tbl_e;
                s32 temp_a1;
                s32 var_a0;
                s32 tblptr;
                u8 *rec;
                u8 *text;

                archive = &D_80142D10;
                tbl_e = (s32)archive;
                {
                    u16 offset = *(u16 *)(tbl_e - (-(((*(u32 *)(SEL_REC + 0x14) >> 9) & 0x7E) + D_80142D10)) + 0x22);
                    tbl_e -= 0xC;
                    text = (u8 *)(D_80142D10 + (offset + tbl_e));
                }
                func_80142130(sp28buf, text);
                temp_a1 = func_800A88A0(prim, ot,
                    (void *)((D_800EC3F2[1] << 8) + (((u8 *)&D_800EC3F2 - 0x2E) + D_800EC3F2[0])), 4, 0x10 - xoff, (0x12 - yoff), 0);
                pos.x = 0x42 - xoff;
                pos.y = (0x12 - yoff);
                var_a0 = func_800A8A78(ot, temp_a1, *(u8 *)(SEL_REC + 0x26), 4, &pos, 0);
                tblptr = *(s32 *)(tbl_e + 0x10);
                rec = SEL_REC;
                text = (u8 *)(tblptr + (*(u16 *)((*(u8 *)(rec + 0x25) * 2) + ((*(u8 *)(rec + 0x24) * 0x1C) + tblptr) + tbl_e) + tbl_e));
                prim = func_800A88A0(var_a0, ot, text, 4, 0x11C - xoff, (0x12 - yoff), 1);
                break;
            }
            }
            prim = func_800A88A0(prim, ot, sp28buf, 4, 0x96 - xoff, 2 - yoff, 2);
        }
        else
        {
            u8 *tbl_d = (u8 *)&D_80142D08;
            s32 base_d = (s32)tbl_d - 4;

            s32 temp_a1;

            temp_a1 = func_800A88A0(
                func_800A88A0(prim, ot,
                    (void *)(D_80142D08 + (*(u16 *)((*entry * 2) + D_80142D08 + base_d) + base_d)),
                    4, 0x96 - xoff, 2 - yoff, 2),
                ot,
                (void *)((D_800EC3F4[1] << 8) + (((u8 *)&D_800EC3F4 - 0x30) + D_800EC3F4[0])),
                4, 0x10 - xoff, 0x12 - yoff, 0);
            {
            u8 *inventory = D_8012271C;
            pos.x = 0x70 - xoff;
            pos.y = 0x12 - yoff;
            prim = func_800A8A78(ot, temp_a1,
                *(u8 *)(inventory + *(u16 *)SEL_ENTRY + 0x25E0), 4, &pos, 0);
            }
        }
    }
    return prim;
}
