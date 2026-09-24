#include "akao_driver.h"
#include "akao.h"
#include "akao_voice.h"
#include "sdk/libspu.h"

/* Root counter 2 (Psy-Q RCntCNT2) drives the AKAO driver tick. */
#define AKAO_TIMER_COUNTER 0xF2000002
#define AKAO_TIMER_TARGET 0x44E8
/* Psy-Q RCntMdINTR / EvMdINTR: raise an interrupt / deliver by callback. */
#define AKAO_TIMER_MODE_INTR 0x1000
/* Psy-Q EvSpINT: counter interrupt event. */
#define AKAO_TIMER_EVENT_SPEC 2

/* SpuInitMalloc block count, and SPU address/size of the zeroed primer block. */
#define AKAO_SPU_MALLOC_BLOCKS 4
#define AKAO_SPU_PRIMER_ADDR 0x1010
#define AKAO_SPU_PRIMER_SIZE 0x40

/* Defined in akao_driver.c / akao_driver_init_state.c. */
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
 *   4. Disable the SPU IRQ and clear its callback.
 *   5. Configure root counter 2 (SetRCnt + StartRCnt) and open/enable its
 *      event with @c akao_irq_handler as the driver tick callback.
 *
 * @see https://decomp.me/scratch/0YmTg (100%)
 */
void akao_driver_init(void)
{
    s32 event;

    SpuStart();
    SpuInitMalloc(AKAO_SPU_MALLOC_BLOCKS, g_akao_spu_malloc_table);
    SpuSetTransferMode(0);
    SpuSetTransferStartAddr(AKAO_SPU_PRIMER_ADDR);
    akao_spu_write(g_akao_spu_zero_primer, AKAO_SPU_PRIMER_SIZE);
    akao_spu_wait();
    akao_driver_init_state();
    SpuSetIRQ(0);
    SpuSetIRQCallback(0);

    while (SetRCnt(AKAO_TIMER_COUNTER, AKAO_TIMER_TARGET, AKAO_TIMER_MODE_INTR) == 0)
    {
    }

    while (StartRCnt(AKAO_TIMER_COUNTER) == 0)
    {
    }

    do
    {
        event = OpenEvent(AKAO_TIMER_COUNTER, AKAO_TIMER_EVENT_SPEC, AKAO_TIMER_MODE_INTR, akao_irq_handler);
        g_akao_rcnt2_event = event;
    } while (event == -1);

    while (EnableEvent(g_akao_rcnt2_event) == 0)
    {
    }
}

/**
 * @brief Record the resident data regions of a registered AKAO bank.
 * @param base Address of the bank payload after its AKAO header.
 * @see decomp.me (100%) https://decomp.me/scratch/z36q3
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
        akao_spu_write(g_akao_spu_zero_primer, AKAO_SPU_PRIMER_SIZE);
        akao_spu_wait();
    }

    while (StopRCnt(AKAO_TIMER_COUNTER) == 0)
    {
    }

    UnDeliverEvent(AKAO_TIMER_COUNTER, AKAO_TIMER_EVENT_SPEC);

    while (DisableEvent(g_akao_rcnt2_event) == 0)
    {
    }
    while (CloseEvent(g_akao_rcnt2_event) == 0)
    {
    }

    spu_set_key_off(0xFFFFFF);
    SpuQuit();
}
