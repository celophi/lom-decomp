#include "main.h"

/** @brief Header fields of the pad context written by func_800A8D10. */
typedef struct
{
    u8  pad00[0x18];
    u32 unk18;   /* 0x18 */
    s16 unk1C;   /* 0x1C */
    s8  unk1E;   /* 0x1E */
    u8  pad1F;
    u32 unk20;   /* 0x20 */
    s16 unk24;   /* 0x24 */
    s8  unk26;   /* 0x26 */
    s8  unk27;   /* 0x27 */
    u8  pad28[0xCF - 0x28];
    s8  unkCF;   /* 0xCF */
} PadCtxHeader;

#define PAD_HEADER ((PadCtxHeader *)g_pad_ctx)
#define PAD_BYTES ((u8 *)g_pad_ctx)

#define IS_DBCS_LEAD_BYTE(byte) (((byte) >= 0x19) && ((byte) <= 0x1F))
#define NAME_GLYPH_SIZE_SINGLE 1
#define NAME_GLYPH_SIZE_DOUBLE 2

void field_text_reset_scratch(void);
void func_80063194(void);
void func_800A92CC(void *arg0);
void func_800A939C(void *arg0);
void akao_stop_sfx_by_id(s32 id);

extern s32 D_801227F8[];
extern s32 D_80122908;
extern u8 D_80122910[];
extern s32 D_80122984;
extern u8 D_800FE3A0[];
extern u8 D_801226E0[];
extern u8 D_801227D0;
extern u8 D_801228D0[];
extern u8 D_801228E0[];

/**
 * @brief Point the pad context at the menu layout buffer.
 */
void func_800A8CFC(void)
{
    g_pad_ctx = (PadContext *)g_menuLayoutBuffer;
}

/**
 * @brief Store the field entry parameters into the pad context header.
 * @param arg0 Stored to unk24.
 * @param arg1 Stored to unk26.
 * @param arg2 Stored to unk27.
 * @param arg3 Low 25 bits stored into unk18.
 * @param arg4 Stored to unk1C.
 * @param arg5 Stored to unk1E.
 */
void func_800A8D10(s16 arg0, s8 arg1, s8 arg2, s32 arg3, s32 arg4, s32 arg5)
{
    PAD_HEADER->unk24 = arg0;
    PAD_HEADER->unk26 = arg1;
    PAD_HEADER->unk27 = arg2;
    PAD_HEADER->unk18 = (PAD_HEADER->unk18 & 0xFE000000) | (arg3 & 0x01FFFFFF);
    PAD_HEADER->unk1C = (s16)arg4;
    PAD_HEADER->unk1E = (s8)arg5;
    PAD_HEADER->unk20 = (PAD_HEADER->unk20 & 0xFFFC0000) | g_music_track_index;
    /* The target reads only the low byte of the s32 g_save_slot_index. */
    PAD_HEADER->unkCF = *(u8 *)&g_save_slot_index;
}

/**
 * @brief Append a (word, byte) pair to the ten-entry list at D_801227F8 / D_80122910.
 * @param arg0 Word stored at the next free index.
 * @param arg1 Byte stored at the same index.
 */
void func_800A8D8C(s32 arg0, u8 arg1)
{
    s32 index = D_80122908;

    if (index < 10)
    {
        D_801227F8[index] = arg0;
        D_80122910[index] = arg1;
        D_80122908++;
    }
}

/**
 * @brief Byte length of a NUL-terminated name, counting DBCS lead bytes 0x19-0x1F as two bytes.
 * @param arg0 Name string.
 * @return Byte count excluding the terminator.
 */
s32 func_800A8DDC(u8 *arg0)
{
    s32 count;
    u8 c;

    count = 0;
    c = *arg0;
    if (c != 0)
    {
        do
        {
            if ((u32)(c - 0x19) < 7)
            {
                arg0 += 2;
                count += 2;
            }
            else
            {
                arg0 += 1;
                count += 1;
            }
            c = *arg0;
        } while (c != 0);
    }
    return count;
}

/**
 * @brief Copy a NUL-terminated name, honouring DBCS glyph widths.
 * @param dest Destination buffer; receives the terminator.
 * @param src Source name.
 */
void func_800A8E28(u8 *dest, u8 *src)
{
    volatile u8 *p;
    s32 len;
    s32 i;

    p = (volatile u8 *)src;
    len = 0;
    while (*p != 0)
    {
        if ((u32)(*p - 0x19) < 7)
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
        dest[i] = src[i];
    }
    dest[i] = 0;
}

/**
 * @brief Append a NUL-terminated name onto another, honouring DBCS glyph widths.
 * @param destination Existing name; the source is appended after its last glyph.
 * @param source Name to append.
 */
void func_800A8EAC(u8 *destination, const u8 *source)
{
    const u8 *scan_cursor;
    s32 destination_byte_count;
    s32 source_byte_count;
    s32 append_offset;
    s32 byte_index;

    scan_cursor = destination;
    destination_byte_count = 0;
    while (*scan_cursor)
    {
        if (IS_DBCS_LEAD_BYTE(*scan_cursor))
        {
            scan_cursor += NAME_GLYPH_SIZE_DOUBLE;
            destination_byte_count += NAME_GLYPH_SIZE_DOUBLE;
        }
        else
        {
            scan_cursor += NAME_GLYPH_SIZE_SINGLE;
            destination_byte_count += NAME_GLYPH_SIZE_SINGLE;
        }
    }

    scan_cursor = source;
    source_byte_count = 0;
    append_offset = destination_byte_count;
    while (*scan_cursor)
    {
        if (IS_DBCS_LEAD_BYTE(*scan_cursor))
        {
            scan_cursor += NAME_GLYPH_SIZE_DOUBLE;
            source_byte_count += NAME_GLYPH_SIZE_DOUBLE;
        }
        else
        {
            scan_cursor += NAME_GLYPH_SIZE_SINGLE;
            source_byte_count += NAME_GLYPH_SIZE_SINGLE;
        }
    }

    for (byte_index = 0; byte_index < source_byte_count; byte_index++)
    {
        destination[byte_index + append_offset] = source[byte_index];
    }
    destination[byte_index + append_offset] = 0;
}

/**
 * @brief Copy a fixed 0x40-byte record from src to dst.
 * @param dst Destination buffer.
 * @param src Source buffer.
 */
void func_800A8F8C(u8 *dst, u8 *src)
{
    u32 i;

    i = 0;
    do
    {
        i++;
        *dst++ = *src++;
    } while (i < 0x40);
}

/**
 * @brief Compact the active field-object table, then clear the tail.
 *
 * Walks the 0x64 fixed-size (0x40-byte) records starting at @c g_pad_ctx +
 * 0xCE0: each live record is moved down to the next free slot (via bcopy) and
 * its old copy zeroed, closing gaps. Any slots left between the compacted end
 * and @c g_pad_ctx + 0x25E0 are then zeroed.
 *
 * @see decomp.me (100%) TODO
 */
void func_800A8FB4(void)
{
    s32 var_s2;
    u8 *var_s0;
    u8 *var_s1;

    var_s2 = 0;
    var_s0 = PAD_BYTES + 0xCE0;
    var_s1 = var_s0;
    do
    {
        if (*var_s1 != 0)
        {
            if (var_s1 != var_s0)
            {
                bcopy(var_s1, var_s0, 0x40);
                *var_s1 = 0;
            }
            var_s0 += 0x40;
        }
        var_s2 += 1;
        var_s1 += 0x40;
    } while (var_s2 < 0x64);
    while ((u32)var_s0 < (u32)(PAD_BYTES + 0x25E0))
    {
        *var_s0 = 0;
        var_s0 += 0x40;
    }
}

/**
 * @brief Find the first free inventory record in g_pad_ctx's item table.
 * @return Pointer to the first record whose first byte is 0, or NULL if all
 *         0x64 records are occupied.
 */
u8 *func_800A9060(void)
{
    s32 count;
    u8* rec;

    rec = PAD_BYTES + 0xCE0;
    for (count = 0; count < 0x64; count++)
    {
        if (*rec == 0)
        {
            return rec;
        }
        rec += 0x40;
    }
    return NULL;
}

/**
 * @brief Reset text scratch, run one of two handlers on arg0 by D_80122984, then call func_80063194.
 * @param arg0 Forwarded to func_800A92CC or func_800A939C.
 */
void func_800A909C(void *arg0)
{
    field_text_reset_scratch();

    if (D_80122984)
    {
        func_800A92CC(arg0);
    }
    else
    {
        func_800A939C(arg0);
    }

    func_80063194();
}

/**
 * @brief Restores per-part bytes saved for each active field actor.
 *
 * Stops sound effect 0x7E, then walks the active actor-index table and copies
 * the saved bytes back to offsets 0x2E and 0x33 of each 0x48-byte part record.
 */
void func_800A90F8(void)
{
    s32 i;
    u8 *rec;

    akao_stop_sfx_by_id(0x7E);
    for (i = 0; i < D_801227D0; i++)
    {
        rec = D_800FE3A0 + D_801226E0[i] * 0x48;
        rec[0x2E] = D_801228D0[i];
        rec[0x33] = D_801228E0[i];
    }
}
