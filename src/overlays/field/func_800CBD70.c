#include "common.h"

/** @brief Partial RefClassView layout used by func_800CBD70. */
typedef struct
{
    u8 pad[0x29D8];
    u8 value;
} RefClassView;

/** @brief Partial OutEntry layout used by func_800CBD70. */
typedef struct
{
    u16 flag;
    u16 value;
} OutEntry;

extern u8 D_800459AE;
extern s32 D_80122C00;
extern u8 D_800F2098[];
extern u8 D_800F2180[];
extern u8 g_menuLayoutBuffer[];

/**
 * @brief Validate menu entries against their reference classes and fill result pairs.
 * @param arg0 Output array of flag/value pairs, one per menu-layout entry.
 * @return The global status byte D_800459AE.
 * @note WIP: GCC hoists the reference-class load that the target repeats.
 */
u8 func_800CBD70(void *arg0)
{
    OutEntry *out;
    u8 *buf;
    s32 i;
    s32 word;
    s32 type;
    u8 ref_class;
    s32 table_val;

    out = (OutEntry *)arg0;
    i = 0;
    if (g_menuLayoutBuffer[0x29D6] != 0)
    {
        buf = g_menuLayoutBuffer;
        do
        {
            word = *(s32 *)(buf + 0x29DC + i * 4);
            ref_class = buf[D_80122C00 + 0x29D8];
            type = word & 3;
            if ((type != ref_class && type != 3) ||
                (((table_val = *(s32 *)(D_800F2098 + (word & 0xFC))) != 0) &&
                 ((buf[ref_class * 332 + 0x2B50] & 0xF) != table_val)))
            {
                out[i].flag = 1;
                out[i].value = 0xF;
            }
            else
            {
                out[i].flag = 0;
                out[i].value = *(u16 *)(D_800F2180 + (*(u8 *)(buf + 0x29DC + i * 4) & 0xFC));
            }
            i++;
        } while (i < buf[0x29D6]);
    }
    return D_800459AE;
}
