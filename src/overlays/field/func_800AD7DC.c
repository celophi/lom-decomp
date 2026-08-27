#include "common.h"

/**
 * @brief Ordered-pair lookup entry; table stride is 5 bytes.
 */
typedef struct
{
    u8 f0;
    u8 f1;
    u8 f2;
    u8 f3;
    u8 f4;
} PairEntry;

extern PairEntry D_800EC55C[];

/**
 * @brief Looks up the result byte for an unordered pair (@p a, @p b).
 *
 * Scans 18 table entries for one whose f0/f2 fields hold @p a and @p b in
 * either order; returns that entry's f4 result, or 0xFF when none match.
 */
s32 func_800AD7DC(s32 a, s32 b)
{
    PairEntry *e = D_800EC55C;
    s32 i;

    i = 0;
    while (i < 18)
    {
        if ((e->f0 == a && e->f2 == b) || (e->f0 == b && e->f2 == a))
        {
            return e->f4;
        }
        i++;
        e++;
    }
    return 0xFF;
}
