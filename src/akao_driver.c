#include "akao_driver.h"
#include "akao.h"

/* "AKAO" in little-endian */
#define AKAO_MAGIC 0x4F414B41 

/**
 * @brief Relocates an AKAO articulation table by adding the SPU upload base
 *        address into each entry as it is copied.
 *
 * Each articulation entry is 16 bytes wide; the routine streams @p count
 * entries from @p src to @p dst, biasing the first word of every entry by
 * @p spu_base so the in-RAM table can be patched to absolute SPU addresses.
 * The trailing three words of each entry are copied unchanged.
 *
 * @param src       Source articulation table (4 words/entry).
 * @param dst       Destination articulation table (same stride).
 * @param spu_base  SPU base address to add into the first word of each entry.
 * @param count     Number of entries to relocate.
 *
 * @see https://decomp.me/scratch/CJTY6 (100%)
 */
void akao_relocate_articulations(s32* src, s32* dst, s32 spu_base, s32 count)
{
    s32* t0 = dst + 3;
    int new_var;
    s32* v1 = src + 3;
    s32 new_var2;
    char new_var3;
    do
    {
        *dst = (*src) + spu_base;
        new_var2 = v1[-2];
        src += 4;
        count -= 1;
        t0[-2] = new_var2 + spu_base;
        (new_var2 = 4);
        dst += new_var2;
        new_var = -1;
        t0[new_var] = v1[new_var];
        *t0 = *v1;
        v1 += 4;
        t0 += 4;
    } while (count != 0);
}

/**
 * @brief Validates the 'AKAO' magic word at the head of an audio resource.
 *
 * The Square AKAO sound driver tags every bank/sequence with the four-byte
 * little-endian magic 0x4F414B41 ("AKAO"). This function returns 0 only when
 * @p data points at a buffer beginning with that exact magic, by subtracting the
 * magic and letting the result be zero on a match.
 *
 * @param data  Candidate AKAO-tagged buffer (first word is the magic).
 *
 * @return 0 if the magic matches; otherwise *data - AKAO_MAGIC (non-zero delta).
 *
 * @see decomp.me: (100%) https://decomp.me/scratch/scY8u
 */
s32 akao_check_magic(s32* data)
{
    return *data - AKAO_MAGIC;
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
 * @brief Validates an AKAO buffer's magic and forwards it to the driver.
 *
 * Checks that @p sequenceData starts with the 'AKAO' magic via akao_check_magic.
 * On success, reads the bank id (offset 0x18) and SPU upload address
 * (offset 0x10) from the AkaoBankHeader and dispatches the buffer through
 * akao_upload_bank, which primes SpuSetTransferStartAddr and posts the
 * sequence to the audio driver.
 *
 * Called in a tight loop by akao_play_sequence_blocking.
 *
 * @param seq_data            Pointer to an AKAO-tagged sequence buffer.
 * @param wait_for_completion When non-zero, the inner submit blocks until the
 *                            SPU transfer completes.
 *
 * @return 0 if the magic matched and the buffer was submitted; -1 if the
 *         AKAO magic check failed.
 */
s32 akao_submit(AkaoSeqHeader* seq_data, s32 wait_for_completion)
{
    s32 ret = -1;
    AkaoBankHeader* bank_hdr = (AkaoBankHeader*)seq_data;
    if (akao_check_magic((s32*)seq_data) == 0)
    {
        akao_upload_bank(seq_data, wait_for_completion, bank_hdr->bank_id, bank_hdr->spu_dest_addr);
        ret = 0;
        return ret;
    }
    return ret;
}

/**
 * @brief Uploads an AKAO instrument/sample bank to the SPU and patches its
 *        articulation table.
 *
 * Validates the AKAO magic, then:
 *   1. @c SpuSetTransferStartAddr(arg3)  — set the SPU upload base.
 *   2. akao_spu_write of the sample blob (located after the bank's
 *      articulation table at offset 0x40 + articulation_count*0x10, size
 *      sample_size).
 *   3. akao_relocate_articulations to copy the articulation table into the
 *      driver's instrument slot @c g_akao_articulation_slots[arg2*0x10] with each entry's
 *      first word biased by the SPU base.
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
 * @see https://decomp.me/scratch/Awfhy (99.90%)
 */
s32 akao_upload_bank(void* bank, s32 wait_for_completion, s32 bank_id, s32 spu_base)
{
    s32 new_var;
    s32 var_v0;
    AkaoBankHeader* bank_hdr;
    u8* base;

    s32 ret_val;
    akao_spu_wait();
    var_v0 = -1;
    if (akao_check_magic(bank) == 0)
    {
        new_var = bank;
        bank_hdr = (AkaoBankHeader*)bank;

        SpuSetTransferStartAddr(spu_base);
        base = (u8*)bank;
        base = base + 0x40;
        akao_spu_write((s32)(base + (bank_hdr->articulation_count * 0x10)), bank_hdr->sample_size);
        akao_relocate_articulations((s32*)base, (s32*)(g_akao_articulation_slots + (bank_id * 0x10)), spu_base, bank_hdr->articulation_count);
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

/**
 * @brief Zeroes and primes the AKAO driver's runtime state.
 *
 * Touched in akao_driver_init after the SPU is brought up. Clears the music
 * channel state for 0x20 sequence channels (each 0x118 bytes wide, indexed
 * via @c D_80049130) and 0x18 SFX channels (also 0x118-byte stride, in
 * @c g_sfx_channels). Pokes the SPU master/reverb registers
 * (@c 0x1F801D80..1F801DB2, @c 0x1F801DAA control). Calls back into the
 * higher-level @c func_80028E34 / @c SpuSetReverb to install the channel
 * state pointer and reverb mode.
 *
 * @see https://decomp.me/scratch/9R0Vj (96.93%)
 */
void akao_driver_init_state(void)
{
    u16* hw = (u16*)0x1F801DAA;
    u32 t0 = 0x18;
    u8** new_var4;
    u32* new_var3;
    u32* new_var2;
    u32* new_var;
    int new_var7;
    u16 new_var8;
    int new_var5;
    u8* a0 = g_akao_seq_channels;
    u8* a2 = D_80049130;
    int new_var6;
    u8* new_var9;
    u32 a3;
    new_var3 = (u32*)off(D_8003EC30, 4);
    *new_var3 = 0;
    *((u32*)off(D_8003EC30, 0)) = 0;
    *((u32*)off(D_8004D388, 0x14)) = 0;
    *((u32*)off(D_8004D388, 0x10)) = 0;
    *((u32*)off(D_8004D388, 0x0C)) = 0;
    *((u32*)off(D_8004D388, 0x08)) = 0;
    *((u32*)off(D_8004D388, 0x04)) = 0;
    *((u32*)off(D_8004D388, 0x00)) = 0;
    *((u32*)off(g_akao_driver_flags, 0x00)) = 0;
    *((u32*)off(g_akao_driver_flags, 0x04)) = 1;
    new_var = (u32*)off(D_8004F830, 0x00);
    *((u32*)off(D_8004D400, 0x00)) = 0;
    *((u32*)off(a0, 0x04)) = 0;
    *((u32*)off(a0, 0x08)) = 0;
    *((u16*)off(a0, 0x5E)) = 0;
    *((u32*)off(D_8004D400, 0x10)) = 0;
    *((u32*)off(a0, 0x1C)) = 0;
    *((u16*)off(D_8004C2D0, 0x5E)) = 0;
    *((u32*)off(D_8004C2D0, 0x04)) = 0;
    *((u32*)off(a0, 0x50)) = 0x7F0000;
    D_8003EC58 = a2;
    a2 += 0x58;
    g_akao_seq_channel0 = (AkaoChannelState *)a0;
    D_8003EC28 = 0;
    D_8003EC24 = 0;
    D_8003EC70 = 0;
    *((u16*)off(a0, 0x58)) = 0;
    D_8003EC68 = 0x7FFF0000;
    D_8003EC40 = 0;
    D_8003EC74 = 0;
    D_8003EC42 = 0;
    D_8003EC78 = 0;
    D_8003EC64 = 0;
    *((u32*)off(D_8004D400, 0x1C)) = 0;
    *((u32*)off(a0, 0x3C)) = 0;
    *((u32*)off(D_8004D400, 0x20)) = 0;
    a3 = (new_var8 = *hw);
    *((s16*)0x1F801D80) = 0x3FFF;
    *((s16*)0x1F801D82) = 0x3FFF;
    *((s16*)0x1F801DB0) = 0x7FFF;
    *((s16*)0x1F801DB2) = 0x7FFF;
    *((u32*)off(a0, 0x40)) = 0;
    a3 = 0;
    *((u32*)off(D_8004D400, 0x24)) = a3;
    *((u32*)off(a0, 0x44)) = 0;
    *((u16*)off(a0, 0x68)) = 0;
    *((u16*)off(a0, 0x66)) = 0;
    *((u16*)off(a0, 0x64)) = a3;
    *((u16*)off(a0, 0x6C)) = a3;
    *((u32*)off(g_akao_xa_tracker, 0x40)) = 0x7F00;
    *((u32*)off(g_akao_xa_tracker, 0x48)) = a3;
    D_8003EC44 = a3;
    D_8003EC6C = a3;
    D_8003EC7C = a3;
    *((u32*)off(D_8004F830, 0x08)) = a3;
    *((u32*)off(D_8004F830, 0x04)) = a3;
    a3 = *hw;
    *new_var = 0;
    *hw = a3;
    *hw = (*hw) & 0xFFFA;
    *hw = (*hw) | 1;
    a3 = 0;
    hw = a2 - 0x24;
    do
    {
        a3++;
        *((u32*)hw) = 0;
        *((u32*)(a2 + 0xA4)) = t0;
        *((u16*)(a2 + 0x0C)) = a3;
        *((u32*)(a0 = a2 + 0x00)) = a3;
        a2 += 0x118;
    } while ((a3 & 0xFFFF) < 0x20);
    new_var7 = 0x7F00;
    a3 = 0xC;
    new_var5 = 0x8C;
    new_var6 = 1;

    {
        u8* v1 = g_sfx_channels + new_var5;
        do
        {
            u32 tmp = a3 & 0xFFFF;
            a3++;
            *((u32*)(v1 - 0x58)) = 0;
            *((u32*)(v1 + 0x70)) = tmp;
            *((u16*)(v1 - 0x28)) = new_var6;
            *((u32*)(v1 - 0x34)) = 0;
            *((u16*)(v1 + 0x58)) = new_var7;
            *((u16*)(v1 + 0x02)) = 0;
            *((u16*)(v1 - 0x04)) = a3;
            *((u32*)((*(new_var4 = &v1)) - 0x4C)) = 0;
            *((u16*)v1) = 0;
            v1 += 0x118;
        } while ((a3 & 0xFFFF) < 0x18);
    }
    {
        u8* a0_ptr = (u8*)g_akao_seq_channel0;
        u8* v0_ptr = D_8004D400;
        u8* v1_ptr = g_akao_driver_flags;
        a0 = a0_ptr;
        *((u32*)off(a0, 0x18)) = 0;
        *((u32*)off(a0, 0x14)) = 0;
        new_var2 = (u32*)off(v0_ptr, 0x18);
        *((u32*)off(a0, 0x10)) = a3;

        *new_var2 = 1;
        *((u32*)off(v0_ptr, 0x14)) = 0x66A80000;
        *((u32*)off(v0_ptr, 0x0C)) = a3;
        *((u32*)off(v0_ptr, 0x08)) = a3;
        *((u32*)off(v0_ptr, 0x04)) = a3;
        new_var6 = 0x03FFF000;
        *((u32*)off(a0, 0x48)) = new_var6;
        *((u32*)off(a0, 0x4C)) = a3;
        *((u16*)off(a0, 0x5A)) = a3;
        new_var9 = v1_ptr;
        *((u32*)off(new_var9, 0x08)) = (*((u32*)off(new_var9, 0x08))) | 0x80;
    }
    func_80028E34(4, 0x03FFF000, a2, a3);
    SpuSetReverb(1);
}

/**
 * @brief Brings the AKAO sound system online.
 *
 * Boot sequence:
 *   1. @c SpuStart, allocate SPU RAM (@c SpuInitMalloc), set transfer mode.
 *   2. Upload a 64-byte zero-payload primer to SPU (@c &g_akao_spu_zero_primer, size 0x40)
 *      and wait for completion.
 *   3. Run akao_driver_init_state.
 *   4. Disable SPU IRQ; install the AKAO IRQ callback (@c func_8002A134).
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

    do
    {
        /* wait for condition */
    } while (func_80023CA0(0xF2000002, 0x44E8, 0x1000) == 0);

    do
    {
        /* wait for condition */
    } while (func_80023D74(0xF2000002) == 0);

    do
    {
        temp_v0 = func_800167AC(0xF2000002, 2, 0x1000, func_8002A134);
        D_8003EC14 = temp_v0;
    } while (temp_v0 == -1);

    do
    {
        /* wait for completion */
    } while (func_800167DC(D_8003EC14) == 0);
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/z36q3
 */
void func_80023BB8(s32 base)
{
    D_8003EC48 = base;
    base += 0x600;
    D_8003EC50 = base;
    base += 0x300;
    D_8003EC54 = base;
}

/**
 * @brief Tears the AKAO sound system down.
 *
 * Mirrors akao_driver_init in reverse: drains any in-flight SPU upload, stops
 * the per-frame counter, disables and undelivers its event, then clears any
 * lingering SPU IRQs and calls @c SpuQuit.
 *
 * @see https://decomp.me/scratch/VenON (100%)
 */
void akao_driver_shutdown(void)
{
    if (g_akao_spu_xfer_pending == 1)
    {
        akao_spu_write(&g_akao_spu_zero_primer, 0x40);
        akao_spu_wait();
    }
    do
    {

    } while (func_80023DA4(0xF2000002) == 0);
    func_80023C90(0xF2000002, 2);
    do
    {

    } while (func_80023C80(D_8003EC14) == 0);
    do
    {

    } while (func_800167BC(D_8003EC14) == 0);
    func_8002427C(0xFFFFFF);
    SpuQuit();
}