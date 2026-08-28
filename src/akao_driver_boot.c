#include "akao_driver.h"
#include "akao.h"
#include "akao_voice.h"
#include "sdk/libspu.h"

/* Defined in akao_driver.c / akao_driver_init_state.c. */
void akao_spu_write(s32 src_addr, s32 byte_count);
void akao_spu_wait(void);
void akao_driver_init_state(void);

/**
 * @brief Brings the AKAO sound system online.
 *
 * Boot sequence:
 *   1. @c SpuStart, allocate SPU RAM (@c SpuInitMalloc), set transfer mode.
 *   2. Upload a 64-byte zero-payload primer to SPU (@c &g_akao_spu_zero_primer, size 0x40)
 *      and wait for completion.
 *   3. Run akao_driver_init_state.
 *   4. Disable SPU IRQ; install the AKAO IRQ callback (@c akao_irq_handler).
 *   5. Configure the per-frame counter (@c F2000002 / SetRCnt + StartRCnt)
 *      and open/enable its event so akao_xa_advance_frame ticks.
 *
 * @see https://decomp.me/scratch/0YmTg (100%)
 */
void akao_driver_init(void)
{
    s32 temp_v0;

    SpuStart();
    SpuInitMalloc(4, &g_akao_spu_malloc_table);
    SpuSetTransferMode(0);
    SpuSetTransferStartAddr(0x1010);
    akao_spu_write(&g_akao_spu_zero_primer, 0x40);
    akao_spu_wait();
    akao_driver_init_state();
    SpuSetIRQ(0);
    SpuSetIRQCallback(0);

    while (SetRCnt(0xF2000002, 0x44E8, 0x1000) == 0);

    while (StartRCnt(0xF2000002) == 0);

    do
    {
        temp_v0 = OpenEvent(0xF2000002, 2, 0x1000, akao_irq_handler);
        g_akao_rcnt2_event = temp_v0;
    } while (temp_v0 == -1);

    while (EnableEvent(g_akao_rcnt2_event) == 0);
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/z36q3
 */
void akao_set_bank_data_ptrs(s32 base)
{
    g_akao_bank_prog_base = base;
    base += 0x600;
    g_akao_bank_region_b = base;
    base += 0x300;
    g_akao_bank_region_c = base;
}

/**
 * @brief Tears the AKAO sound system down.
 *
 * Mirrors akao_driver_init in reverse: drains any in-flight SPU upload, stops
 * the per-frame counter, disables and undelivers its event, then clears any
 * lingering SPU IRQs and calls @c SpuQuit.
 *
 * @see https://decomp.me/scratch/1FglZ (100%)
 */
void akao_driver_shutdown(void)
{
    if (g_akao_spu_xfer_pending == 1)
    {
        akao_spu_write(&g_akao_spu_zero_primer, 0x40);
        akao_spu_wait();
    }

    while (StopRCnt(0xF2000002) == 0);

    UnDeliverEvent(0xF2000002, 2);

    while (DisableEvent(g_akao_rcnt2_event) == 0);
    while (CloseEvent(g_akao_rcnt2_event) == 0);

    spu_set_key_off(0xFFFFFF);
    SpuQuit();
}
