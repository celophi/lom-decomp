#include "internal/akao_driver.h"
#include "main/audio/akao.h"
#include "internal/akao_voice.h"
#include "sdk/libspu.h"
#include "sdk/libapi.h"

/* SpuInitMalloc block count, and SPU address/size of the zeroed primer block. */
#define AKAO_SPU_MALLOC_BLOCKS 4
#define AKAO_SPU_PRIMER_ADDR 0x1010
#define AKAO_SPU_PRIMER_SIZE 0x40

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
 * @see decomp.me (100%) https://decomp.me/scratch/0YmTg
 */
void akao_driver_init(void)
{
    s32 event;

    SpuStart();
    SpuInitMalloc(AKAO_SPU_MALLOC_BLOCKS, g_akao_spu_malloc_table);
    SpuSetTransferMode(SPU_TRANSFER_BY_DMA);
    SpuSetTransferStartAddr(AKAO_SPU_PRIMER_ADDR);
    akao_spu_write(g_akao_spu_zero_primer, AKAO_SPU_PRIMER_SIZE);
    akao_spu_wait();
    akao_driver_init_state();
    SpuSetIRQ(SPU_OFF);
    SpuSetIRQCallback(NULL);

    while (SetRCnt(RCntCNT2, AKAO_TICK_PERIOD, RCntMdINTR) == 0)
    {
    }

    while (StartRCnt(RCntCNT2) == 0)
    {
    }

    do
    {
        event = OpenEvent(RCntCNT2, EvSpINT, EvMdINTR, akao_irq_handler);
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
void akao_set_bank_data_ptrs(u8* base)
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
 * @see decomp.me (100%) https://decomp.me/scratch/1FglZ
 */
void akao_driver_shutdown(void)
{
    if (g_akao_spu_xfer_pending == 1)
    {
        akao_spu_write(g_akao_spu_zero_primer, AKAO_SPU_PRIMER_SIZE);
        akao_spu_wait();
    }

    while (StopRCnt(RCntCNT2) == 0)
    {
    }

    UnDeliverEvent(RCntCNT2, EvSpINT);

    while (DisableEvent(g_akao_rcnt2_event) == 0)
    {
    }
    while (CloseEvent(g_akao_rcnt2_event) == 0)
    {
    }

    spu_set_key_off(SPU_ALLCH);
    SpuQuit();
}
