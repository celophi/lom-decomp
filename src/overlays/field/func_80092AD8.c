#include "common.h"

/**
 * @brief Record fields consumed by the FIELD movement-state update.
 */
typedef struct
{
    u8 pad0[4];
    s32 state_value;
    u8 pad8[0x21 - 8];
    u8 state_flags;
    u8 pad22[0x54 - 0x22];
} FieldStateRecord;

void func_8006C3FC(FieldStateRecord *rec);
void func_80092C24(u8 *rec, s32 arg1);
void func_80097FA0(void *arg0, void *arg1, s32 arg2);

/**
 * @brief Advance selected FIELD movement states and request animation 0x1A.
 * @param entry Record containing the signed state value and state flags.
 * @return Zero while the state is being advanced, or one when it is complete.
 * @note The high state bit selects horizontal displacement direction.
 * @note Local assembly match: 100% with GCC 2.7.2 CDK (83 instructions).
 * @see decomp.me WIP
 */
s32 func_80092AD8(FieldStateRecord *entry)
{
    s32 state_value;
    s32 unused_value;
    s32 timer;
    /**
     * @brief Three-axis displacement stored in scratchpad RAM.
     */
    struct Vector
    {
        s32 x;
        s32 y;
        s32 z;
    } *scratch;

    scratch = (struct Vector *)0x1F800000;
    switch ((u32)(u8)(entry->state_flags & 0x7F) - 8)
    {
        case 0: /* state 8 */
            entry->state_flags = (entry->state_flags & 0x80) | 9;
            func_8006C3FC(entry);
            return 0;
        case 53: /* state 61 */
            state_value = entry->state_value;
            if (state_value < -0xC00)
            {
                entry->state_value = state_value + 0xC00;
                return 0;
            }
            else
            {
                entry->state_value = 0;
                entry->state_flags = (entry->state_flags & 0x80) | 9;
                func_8006C3FC(entry);
                return 0;
            }
        case 64: /* state 72 */
        case 65: /* state 73 */
            state_value = entry->state_value;
            if (state_value < -0xC00)
            {
                if (entry->state_flags & 0x80)
                {
                    scratch->x = 0x200;
                }
                else
                {
                    scratch->x = -0x200;
                }
                scratch->z = 0;
                scratch->y = 0;
                timer = entry->state_value;
                func_80097FA0(entry, scratch, 1);
                timer += 0xC00;
                entry->state_value = timer;
                return 0;
            }
            else if (state_value == 0)
            {
                break;
            }
            else if (state_value < -0xA)
            {
                entry->state_value = -0xA;
                func_80092C24((u8 *)entry, 0x1A);
            }
            else
            {
                entry->state_value = state_value + 1;
            }
            return 0;
        case 1:  /* state 9  */
        case 50: /* state 58 */
        case 51: /* state 59 */
        case 52: /* state 60 */
        case 62: /* state 70 */
        case 63: /* state 71 */
        case 70: /* state 78 */
        case 71: /* state 79 */
            func_80092C24((u8 *)entry, 0x1A);
            return 1;
        default:
            break;
    }
    return 1;
}
