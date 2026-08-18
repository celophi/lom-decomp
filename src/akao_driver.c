#include "akao_driver.h"
#include "akao.h"
#include "akao_voice.h"
#include "psyq/libspu.h"

/* "AKAO" in little-endian */
#define AKAO_MAGIC 0x4F414B41

/**
 * @brief Relocates an AKAO articulation table by adding the SPU upload base
 *        address into each entry as it is copied.
 *
 * Streams @p count @ref AkaoArticulation entries from @p src to @p dst,
 * biasing @c sample_addr and @c loop_addr by @p spu_base so the in-RAM
 * table holds absolute SPU addresses. The @c adsr and @c pitch_misc fields
 * are copied verbatim.
 *
 * @param src       Source articulation table.
 * @param dst       Destination articulation table.
 * @param spu_base  SPU base address added to @c sample_addr and @c loop_addr.
 * @param count     Number of entries to relocate (must be > 0; do-while shape).
 *
 * @see AkaoArticulation
 * @see https://decomp.me/scratch/CJTY6 (100%)
 */
void akao_relocate_articulations(AkaoArticulation* src, AkaoArticulation* dst, s32 spu_base, s32 count)
{
    do
    {
        dst->sample_addr = src->sample_addr + spu_base;
        dst->loop_addr = src->loop_addr + spu_base;
        dst->adsr.word = src->adsr.word;
        dst->pitch_misc.word = src->pitch_misc.word;
        src++;
        dst++;
    } while (--count != 0);
}

/**
 * @brief Validates the 'AKAO' magic word at the head of an audio resource.
 *
 * The Square AKAO sound driver tags every bank/sequence with the four-byte
 * little-endian magic 0x4F414B41 ("AKAO"). Returns 0 iff @p hdr begins with
 * that magic, by subtracting and letting the result be zero on a match.
 *
 * @param hdr  Candidate AKAO-tagged buffer.
 *
 * @return 0 if the magic matches; otherwise (hdr->magic - AKAO_MAGIC).
 *
 * @see decomp.me: (100%) https://decomp.me/scratch/scY8u
 */
s32 akao_check_magic(AkaoSeqHeader* hdr)
{
    return hdr->magic - AKAO_MAGIC;
}

/**
 * @brief SPU transfer-completion callback.
 *
 * Installed via @c SpuSetTransferCallback by akao_spu_arm_xfer; the SPU
 * library invokes this when a write/read transfer finishes. Tears down the
 * callback (passes NULL back to libspu) and clears the in-flight flag so
 * akao_spu_wait can release.
 *
 * @see https://decomp.me/scratch/qI6jZ (100%)
 */
void akao_spu_xfer_done_cb(void)
{
    SpuSetTransferCallback(0);
    g_akao_spu_xfer_pending = 0;
}

/**
 * @brief Arms the SPU for an asynchronous transfer.
 *
 * Sets the in-flight flag (@c g_akao_spu_xfer_pending = 1) and registers
 * akao_spu_xfer_done_cb as the libspu transfer-callback. Used as the prelude
 * to either akao_spu_write or akao_spu_read.
 *
 * @see https://decomp.me/scratch/oy7T9 (100%)
 */
void akao_spu_arm_xfer(void)
{
    g_akao_spu_xfer_pending = 1;
    SpuSetTransferCallback(&akao_spu_xfer_done_cb);
}

/**
 * @brief Begins an asynchronous SPU write (SpuWrite + done-callback hookup).
 *
 * Inlined equivalent of akao_spu_arm_xfer immediately followed by
 * @c SpuWrite(arg0, arg1). Caller pairs this with akao_spu_wait when it
 * needs synchronous completion.
 *
 * @param src_addr   Source address in main RAM.
 * @param byte_count Number of bytes to upload.
 *
 * @see https://decomp.me/scratch/D2YiT (100%)
 */
void akao_spu_write(s32 src_addr, s32 byte_count)
{
    g_akao_spu_xfer_pending = 1;
    SpuSetTransferCallback(&akao_spu_xfer_done_cb);
    SpuWrite(src_addr, byte_count);
}

/**
 * @brief Begins an asynchronous SPU read (SpuRead + done-callback hookup).
 *
 * Mirror of akao_spu_write but invokes @c SpuRead. Currently unused inside
 * akao_spu.c; kept here because it is part of the AKAO SPU helper set and is
 * referenced via the .ld linker script.
 *
 * @param dst_addr   Destination address in main RAM.
 * @param byte_count Number of bytes to read back from the SPU.
 *
 * @see https://decomp.me/scratch/lLOqn (100%)
 */
void akao_spu_read(s32 dst_addr, s32 byte_count)
{
    akao_spu_arm_xfer();
    SpuRead(dst_addr, byte_count);
}

/**
 * @brief Spin-waits for an in-flight SPU transfer to complete.
 *
 * Polls the volatile in-flight flag @c g_akao_spu_xfer_pending until akao_spu_xfer_done_cb
 * clears it. Used wherever the AKAO upload paths need to synchronize before
 * issuing the next SPU operation.
 *
 * @see https://decomp.me/scratch/fqPPO (100%)
 */
void akao_spu_wait(void)
{
    while ((*((volatile s32*)(&g_akao_spu_xfer_pending))) == 1);
}

/**
 * @brief Validates an AKAO bank's magic and forwards it to the driver.
 *
 * Checks that @p bank starts with the 'AKAO' magic via akao_check_magic.
 * On success, reads @c bank_id and @c spu_dest_addr from the bank header and
 * dispatches the buffer through akao_upload_bank, which primes
 * SpuSetTransferStartAddr and posts the sample/articulation upload to the
 * audio driver.
 *
 * Called in a tight loop by akao_play_sequence_blocking.
 *
 * @param bank                Pointer to an AKAO-tagged bank buffer.
 * @param wait_for_completion When non-zero, the inner submit blocks until the
 *                            SPU transfer completes.
 *
 * @return 0 if the magic matched and the buffer was submitted; -1 if the
 *         AKAO magic check failed.
 *
 * @see decomp.me (100%) https://decomp.me/scratch/B0eQd
 */
s32 akao_submit(AkaoBankHeader* bank, s32 wait_for_completion)
{
    if (akao_check_magic(&bank->header) == 0)
    {
        akao_upload_bank(bank, wait_for_completion, bank->bank_id, bank->spu_dest_addr);
        return 0;
    }
    return -1;
}

/**
 * @brief Uploads an AKAO instrument/sample bank to the SPU and patches its
 *        articulation table.
 *
 * Validates the AKAO magic, then:
 *   1. @c SpuSetTransferStartAddr(spu_base) - set the SPU upload base.
 *   2. akao_spu_write of the sample blob (located after the bank's
 *      articulation table at offset 0x40 + articulation_count * sizeof(AkaoArticulation),
 *      size sample_size).
 *   3. akao_relocate_articulations copies @c articulation_count AkaoArticulation
 *      entries into the driver's instrument slot at byte offset
 *      @c bank_id*0x10 within @c g_akao_articulation_slots (i.e. conceptually
 *      @c ((AkaoArticulation*)g_akao_articulation_slots)[bank_id]), biasing
 *      @c sample_addr and @c loop_addr by @p spu_base on the way in.
 * On magic-mismatch, latches @c g_akao_spu_xfer_pending = -1 and returns -1.
 *
 * @param bank                Pointer to an AkaoBankHeader buffer in main RAM.
 * @param wait_for_completion When non-zero, akao_spu_wait blocks until the
 *                            SPU DMA finishes.
 * @param bank_id             Instrument-table slot index (= @c bank->bank_id).
 * @param spu_base            SPU upload base address in bytes
 *                            (= @c bank->spu_dest_addr).
 *
 * @return 0 on success, -1 on AKAO magic mismatch.
 *
 * @see https://decomp.me/scratch/Awfhy (100%)
 */
s32 akao_upload_bank(void* bank, s32 wait_for_completion, s32 bank_id, s32 spu_base)
{
    u8* base;
    s32 new_var;
    s32 var_v0;
    s32 ret_val;
    AkaoBankHeader* bank_hdr;
    s32 hdr_copy;

    akao_spu_wait();
    var_v0 = -1;
    if ((hdr_copy = akao_check_magic((AkaoSeqHeader*)bank)) == 0)
    {
        hdr_copy = bank;
        bank_hdr = (AkaoBankHeader*)hdr_copy;

        SpuSetTransferStartAddr(spu_base);
        bank = (u8*)bank + 0x40;
        base = (u8*)bank;
        akao_spu_write((s32)(base + (bank_hdr->articulation_count * 0x10)), bank_hdr->sample_size);
        akao_relocate_articulations((AkaoArticulation*)base,
                                    (AkaoArticulation*)(g_akao_articulation_slots + (bank_id * 0x10)), spu_base,
                                    bank_hdr->articulation_count);
        var_v0 = 0;
        if (wait_for_completion != 0)
        {
            akao_spu_wait();
        }
        ret_val = 0;
    }
    else
    {
        g_akao_spu_xfer_pending = -1;
        ret_val = -1;
    }
    new_var = ret_val;
    return new_var;
}

