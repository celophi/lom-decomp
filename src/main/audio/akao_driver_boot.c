#include "internal/akao_driver.h"
#include "main/audio/akao.h"
#include "internal/akao_voice.h"
#include <libspu.h>
#include <libapi.h>

/* SpuInitMalloc block count, and SPU address/size of the zeroed primer block. */
#define AKAO_SPU_MALLOC_BLOCKS 4
#define AKAO_SPU_PRIMER_ADDR 0x1010
#define AKAO_SPU_PRIMER_SIZE 0x40

/* Resident bank tables: sequence-offset pairs followed by program bank keys. */
#define AKAO_BANK_SEQUENCE_OFFSETS_SIZE 0x600
#define AKAO_BANK_PROGRAM_KEYS_SIZE 0x300

/**
 * @brief Initialize the SPU, driver state and root-counter tick event.
 *
 * Initialize the SPU allocator and upload the zero primer before starting the
 * driver tick. Root counter 2 calls akao_irq_handler at AKAO_TICK_PERIOD intervals.
 *
 * @see decomp.me (100%) https://decomp.me/scratch/0YmTg
 */
void akao_driver_init(void)
{
    s32 event_handle;

    SpuStart();
    SpuInitMalloc(AKAO_SPU_MALLOC_BLOCKS, g_akao_spu_malloc_table);
    SpuSetTransferMode(SPU_TRANSFER_BY_DMA);
    SpuSetTransferStartAddr(AKAO_SPU_PRIMER_ADDR);
    akao_spu_write(g_akao_spu_zero_primer, AKAO_SPU_PRIMER_SIZE);
    akao_spu_wait();
    akao_driver_init_state();
    SpuSetIRQ(SPU_OFF);
    SpuSetIRQCallback(NULL);

    while (SetRCnt(RCntCNT2, AKAO_TICK_PERIOD, RCntMdINTR) == 0);

    while (StartRCnt(RCntCNT2) == 0);

    do
    {
        event_handle = OpenEvent(RCntCNT2, EvSpINT, EvMdINTR, akao_irq_handler);
        g_akao_rcnt2_event = event_handle;
    } while (event_handle == -1);

    while (EnableEvent(g_akao_rcnt2_event) == 0);
}

/**
 * @brief Set the active bank's sequence-offset, bank-key and sequence-data pointers.
 * @param base Bank payload after its AKAO header, beginning with sequence-offset pairs.
 * @see decomp.me (100%) https://decomp.me/scratch/z36q3
 */
void akao_set_bank_data_ptrs(u8* base)
{
    g_akao_bank_prog_base = base;
    base += AKAO_BANK_SEQUENCE_OFFSETS_SIZE;
    g_akao_bank_region_b = base;
    base += AKAO_BANK_PROGRAM_KEYS_SIZE;
    g_akao_bank_region_c = base;
}

/**
 * @brief Stop the driver tick, key off all voices and shut down the SPU.
 *
 * If a transfer is pending, submit the zero primer and wait for completion
 * before stopping root counter 2 and removing its event.
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

    while (StopRCnt(RCntCNT2) == 0);

    UnDeliverEvent(RCntCNT2, EvSpINT);

    while (DisableEvent(g_akao_rcnt2_event) == 0);
    while (CloseEvent(g_akao_rcnt2_event) == 0);

    spu_set_key_off(SPU_ALLCH);
    SpuQuit();
}
