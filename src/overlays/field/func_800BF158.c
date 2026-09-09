#include "common.h"

extern u8 *func_800C1E40(s32);
extern u8 *D_80123FC0;
/** @brief Partial shared field state with the selector nibble at offset 0x58. */
typedef struct
{
    u8 bytes[0x58];
    unsigned int flags : 4;
    unsigned int other_flags : 28;
} FieldRecordState;
extern FieldRecordState *D_80123FC4;

/**
 * @brief Copy the selected resource record into the shared field state.
 * @note The final loop selects the last entry below 16, or retains 15.
 * @note GCC 2.7.2 CDK: 97.696075%, 84/102 exact instructions; register and
 * epilogue scheduling differences remain. The low-nibble bitfield is required
 * to reproduce the repeated byte load and masked word update.
 */
void func_800BF158(void)
{
    s32 i;
    u8 *record;

    record = func_800C1E40(4);
    D_80123FC0 = record;
    i = 0;
    *(u16 *)(D_80123FC4->bytes + 0x3A) = *(u16 *)(record + (D_80123FC4->bytes[6] * 20) + 0x186);
    do
    {
        D_80123FC4->bytes[i + 0x3C] = *(D_80123FC0 + (i + D_80123FC4->bytes[6] * 20) + 0x188);
        i++;
    } while (i < 4);
    i = 0;
    do
    {
        D_80123FC4->bytes[i + 0x40] = *(D_80123FC0 + (i + D_80123FC4->bytes[6] * 20) + 0x18C);
        i++;
    } while (i < 4);
    i = 0;
    do
    {
        D_80123FC4->bytes[i * 2 + 0xC] = *(D_80123FC0 + (i + D_80123FC4->bytes[6] * 20) + 0x190);
        D_80123FC4->bytes[i + 0x44] = 0;
        i++;
    } while (i < 8);
    i = 1;
    D_80123FC4->flags = 0xF;
    do
    {
        if (D_80123FC4->bytes[i + 0x28] < 0x10U)
        {
            D_80123FC4->flags = D_80123FC4->bytes[i + 0x28];
        }
        i++;
    } while (i < 5);
}
