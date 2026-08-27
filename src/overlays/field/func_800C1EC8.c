#include "common.h"

/**
 * @brief Word-granular copy or zero-fill of a buffer.
 *
 * Rounds the byte length @p n up toward zero for negative values, converts it
 * to a word count (n >> 2), then either copies that many words from @p src to
 * @p dest (when @p src is non-NULL) or zero-fills @p dest for that many words
 * (when @p src is NULL). Returns @p dest advanced past the last word written.
 *
 * @param src Source word pointer, or NULL to zero-fill instead of copy.
 * @param dest Destination word pointer.
 * @param n Byte length; converted to a word count internally.
 * @return @p dest advanced past the last word written (unchanged when the word
 *         count is zero).
 * @note 97.5% match (23/24 rows). The lone residue is at +0x34: the copy loop's
 *       exit emits `j` to the shared epilogue where the target emits its own
 *       `jr ra`. This is a gcc 2.7.2 make_return_insns decision (the exit jump
 *       targets the label before the return rather than end_of_function_label)
 *       that no source shape, the [EXIT-03] wrapper form, or the permuter
 *       (114k iterations) has been able to reproduce.
 */
s32 *func_800C1EC8(s32 *src, s32 *dest, s32 n)
{
    s32 word;

    if (n < 0)
    {
        n += 3;
    }
    n >>= 2;
    if (src != NULL)
    {
        if (n != 0)
        {
            do
            {
                word = *src;
                src++;
                n--;
                *dest = word;
                dest++;
            } while (n != 0);
            return dest;
        }
        return dest;
    }
    if (n != 0)
    {
        do
        {
            *dest = 0;
            n--;
            dest++;
        } while (n != 0);
    }
    return dest;
}
