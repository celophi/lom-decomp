#include "common.h"

/** @brief Partial PrimSprt20 layout used by func_800ADCD0. */
typedef struct
{
    u32 tag;
    u32 rgbc;
    s16 x0;
    s16 y0;
    u8 u0;
    u8 v0;
    u16 clut;
    s16 w;
    s16 h;
} PrimSprt20;

/** @brief Partial SizeRec layout used by func_800ADCD0. */
typedef struct
{
    u16 unk0;
    u16 unk2;
    s16 unk4;
    s16 unk6;
} SizeRec;

/** @brief Partial UVRec layout used by func_800ADCD0. */
typedef struct
{
    u8 unk0;
    u8 unk1;
    u8 unk2;
    u8 unk3;
    s16 unk4;
    s16 unk6;
} UVRec;

extern s32 g_menu_element_counter;

/**
 * @brief Tile a rectangle with sprite primitives and link them into an ordering table.
 * @param arg0 Next free primitive-buffer address.
 * @param arg1 Ordering-table entry receiving the primitive chain.
 * @param arg2 Destination rectangle position and dimensions.
 * @param arg3 Texture origin and maximum tile dimensions.
 * @return The first free buffer address after the emitted primitives.
 * @note WIP: allocation and instruction-order differences remain.
 */
void *func_800ADCD0(void *arg0, u32 *arg1, SizeRec *arg2, UVRec *arg3)
{
    s32 rows_h;
    s32 cols_w;
    s32 y_accum;
    s32 x_accum;
    s32 seg_h;
    s32 seg_w;
    u32 tmp;
    s32 tag_len;
    s32 code_byte;
    s16 clut;
    s32 rgbc_const;
    s32 mask_lo;
    s32 mask_hi;

    if (arg2->unk4 > 0)
    {
        if (arg2->unk6 > 0)
        {
            y_accum = 0;
            rows_h = arg2->unk6;
            rgbc_const = 0x808080;
            tag_len = 4;
            code_byte = 0x64;
            mask_lo = 0xFFFFFF;
            do
            {
                x_accum = 0;
                seg_h = (arg3->unk6 < rows_h) ? arg3->unk6 : rows_h;
                cols_w = arg2->unk4;
                mask_hi = 0xFF000000;
                do
                {
                    seg_w = (arg3->unk4 < cols_w) ? arg3->unk4 : cols_w;

                    ((PrimSprt20 *)arg0)->rgbc = rgbc_const;
                    ((u8 *)arg0)[3] = tag_len;
                    ((u8 *)arg0)[7] = code_byte;
                    ((PrimSprt20 *)arg0)->x0 = (s16)(arg2->unk0 + x_accum);
                    ((PrimSprt20 *)arg0)->y0 = (s16)(arg2->unk2 + y_accum);
                    ((PrimSprt20 *)arg0)->u0 = arg3->unk0;
                    ((PrimSprt20 *)arg0)->v0 = arg3->unk2;
                    ((PrimSprt20 *)arg0)->w = seg_w;
                    ((PrimSprt20 *)arg0)->h = seg_h;
                    clut = 0x7CD0;
                    if (g_menu_element_counter != 0)
                    {
                        clut = 0x7D10;
                    }
                    ((PrimSprt20 *)arg0)->clut = clut;

                    ((PrimSprt20 *)arg0)->tag = (((PrimSprt20 *)arg0)->tag & mask_hi) | (*arg1 & mask_lo);
                    tmp = (u32)arg0 & mask_lo;
                    arg0 = (u8 *)arg0 + 0x14;
                    *arg1 = (*arg1 & mask_hi) | tmp;

                    x_accum += seg_w;
                    cols_w -= seg_w;
                } while (cols_w != 0);

                rows_h -= seg_h;
                y_accum += seg_h;
            } while (rows_h != 0);
        }
        return arg0;
    }
    return arg0;
}
