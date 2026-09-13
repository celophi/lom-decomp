#include "common.h"

extern s32 D_8012269C;
extern s32 D_8012299C;
extern s32 D_80122994;
extern s32 D_801227C4;
extern s32 D_801227F0;
extern s32 g_gosub_result_count;
extern s32 g_gosub_result_values[];

/**
 * @brief One-time bring-up of the field streaming pipeline (variant 3).
 * @param arg0 Value handed to func_8014024C and stored as the pipeline state base.
 * @see decomp.me (100%) TODO
 */
void func_800AD030(s32 arg0)
{
    if (D_8012269C == 0)
    {
        func_80084240();
        cdrom_stream(0xC, (void *)0x80140000);
        cdrom_wait_queue_empty();
        D_8012299C = arg0 + 1;
        D_801227F0 = 1;
        g_gosub_result_count = 0;
        D_8012269C = 3;
        D_801227C4 = g_gosub_result_values[0];
        func_8014024C((void *)0x80170000, arg0);
    }
}

/**
 * @brief Stream field resource 9 and hand off to func_800C3BB0.
 */
void func_800AD0C8(void)
{
    func_80084240();
    cdrom_stream(9, (void *)0x80140000);
    cdrom_wait_queue_empty();
    func_80140024((void *)0x80150000, 0);
    func_800C3BB0();
    func_80084240();
}

/**
 * @brief Empty stub retained for address/layout parity.
 */
void func_800AD118(void)
{
}

/**
 * @brief One-time bring-up of the field streaming pipeline.
 *
 * On the first call (guard D_8012269C == 0), initialises the subsystem, streams
 * resource 0x11 to 0x80140000, waits for the CD queue to drain, marks the
 * pipeline active, and hands @p arg0 to func_8014011C.
 *
 * @param arg0 Value handed to func_8014011C.
 */
void func_800AD120(s32 arg0)
{
    void func_80084240(void);
    s32 cdrom_stream(s32 resource_index, u32 destination);
    void cdrom_wait_queue_empty(void);
    void func_8014011C(s32 arg0, s32 arg1);

    if (D_8012269C == 0)
    {
        func_80084240();
        cdrom_stream(0x11, 0x80140000);
        cdrom_wait_queue_empty();
        D_80122994 = 1;
        D_8012269C = 4;
        func_8014011C(0x80170000, arg0);
    }
}

/**
 * @brief One-time bring-up of the field streaming pipeline (variant 5).
 *
 * On the first call (guard D_8012269C == 0), initialises the subsystem, streams
 * resource 0x12 to 0x80140000, waits for the CD queue to drain, marks the
 * pipeline active with state 5, and hands @p arg0 to func_8014011C.
 *
 * @param arg0 Value handed to func_8014011C.
 */
void func_800AD194(s32 arg0)
{
    void func_80084240(void);
    s32 cdrom_stream(s32 resource_index, u32 destination);
    void cdrom_wait_queue_empty(void);
    void func_8014011C(s32 arg0, s32 arg1);

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
