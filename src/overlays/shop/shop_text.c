#include "shop_text.h"

/** @brief Glyph codes 0x19-0x1F start a two-byte sequence. */
#define SHOP_TEXT_IS_WIDE(c) ((c) >= 0x19 && (c) <= 0x1F)

static s32 shop_text_length(u8* text);

/**
 * @brief Append one text string to another.
 * @param dst Terminated string to extend; must have room for @p src.
 * @param src Terminated string to append.
 */
void shop_text_append(u8* dst, u8* src)
{
    s32 dst_length;
    s32 src_length;
    s32 i;

    dst_length = shop_text_length(dst);
    src_length = shop_text_length(src);
    for (i = 0; i < src_length; i++)
    {
        dst[dst_length + i] = src[i];
    }
    dst[dst_length + i] = 0;
}

/**
 * @brief Measure a text string in bytes, counting each two-byte glyph sequence whole.
 * @param text Terminated string.
 * @return Length of @p text excluding the terminator.
 */
static s32 shop_text_length(u8* text)
{
    u8 c;
    s32 length;

    c = *text;
    length = 0;
    while (c != 0)
    {
        if (SHOP_TEXT_IS_WIDE(c))
        {
            text += 2;
            length += 2;
        }
        else
        {
            text += 1;
            length += 1;
        }
        c = *text;
    }
    return length;
}

/**
 * @brief Copy a text string.
 * @param dst Destination buffer; must have room for @p src.
 * @param src Terminated string to copy.
 */
void shop_text_copy(u8* dst, u8* src)
{
    u8* p;
    s32 length;
    s32 i;

    p = src;
    length = 0;
    while (*p != 0)
    {
        if (SHOP_TEXT_IS_WIDE(*p))
        {
            p += 2;
            length += 2;
        }
        else
        {
            p += 1;
            length += 1;
        }
    }
    for (i = 0; i < length; i++)
    {
        dst[i] = src[i];
    }
    dst[i] = 0;
}
