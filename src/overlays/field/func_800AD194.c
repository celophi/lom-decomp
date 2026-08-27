#include "common.h"

extern s32 D_8012269C;
extern s32 D_80122994;
void func_80084240(void);
s32 cdrom_stream(s32 resource_index, u32 destination);
void cdrom_wait_queue_empty(void);
void func_8014011C(s32 arg0, s32 arg1);

/**
 * @brief One-time bring-up of the field streaming pipeline (variant 5).
 *
 * On the first call (guard D_8012269C == 0), initialises the subsystem, streams
 * resource 0x12 to 0x80140000, waits for the CD queue to drain, marks the
 * pipeline active with state 5, and hands @p arg0 to func_8014011C.
 */
void func_800AD194(s32 arg0)
{
    if (D_8012269C == 0)
    {
        func_80084240();
        cdrom_stream(0x12, 0x80140000);
        cdrom_wait_queue_empty();
        D_80122994 = 1;
        D_8012269C = 5;
        func_8014011C(0x80170000, arg0);
    }
}
