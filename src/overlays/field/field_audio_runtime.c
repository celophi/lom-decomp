/**
 * @file field_audio_runtime.c
 * @brief FIELD overlay CD-audio / music / SFX runtime subsystem.
 *
 * Consolidated translation unit covering the FIELD AKAO audio helpers: loading
 * SEQ and sound-bank resources from CD-ROM, starting/stopping field music, the
 * SFX-slot dispatchers, and the streaming block-copy/upload state machine.
 *
 * @note Several of these are called cross-overlay (GOVER calls func_800A368C
 *       and func_800A380C while FIELD is resident).
 * @note This file was merged from many per-function sources; shared globals are
 *       viewed with conflicting types across functions, so any symbol with a
 *       type conflict is declared at BLOCK scope inside each user with that
 *       function's original type (GCC 2.7.2 accepts incompatible block-scope
 *       externs and emits identical code).
 */

#include "common.h"
#include "cd_resources.h"
#include "akao.h"
#include "game_state.h"
#include "sdk/memory.h"

/* Scratch buffer the CD-ROM layer decompresses a SEQ resource into. The blob
 * begins with a self-referential offset table; see SEQ_BLOB_OFFSETS. */
#define SEQ_BLOB_BASE 0x80180000

/* Offset table at the head of the SEQ blob. off[0] is the byte offset of the
 * song sequence sub-block, off[1] the byte offset of the instrument bank that
 * follows it. */
#define SEQ_BLOB_OFFSETS 0x80180004

/* Largest music_index this loader accepts; anything above is ignored. */
#define FIELD_SEQ_MAX_INDEX 0x100

/* Fixed CD resource loaded by func_800A3728. TODO: which SEQ this is has not
 * been confirmed; it is not selected through CD_RES_MUSIC_FILE. */
#define FIELD_FIXED_SEQ_RESOURCE 0x92

/* Same scratch blob as SEQ_BLOB_BASE / SEQ_BLOB_OFFSETS, used by the effect
 * bank restore path. */
#define EFFECT_BLOB_BASE 0x80180000
#define EFFECT_BLOB_OFFSETS 0x80180004

/** @brief Header preceding the copied field sound-bank tables. */
typedef struct
{
    /** @brief Byte offset from this header to the current copied table. */
    s32 table_offset;
    /** @brief Unknown field cleared when loading a new bank list. */
    s32 unk_04;
    /** @brief Unknown field cleared when loading a new bank list. */
    s32 unk_08;
} FieldBankCopyHeader;

/*
 * D_8003ECA0 (the AKAO sequence staging area shared with the TITLE overlay) is
 * viewed as `unsigned char` by the field_audio helpers and as `u8[]` by
 * func_800A3FB0, so it is declared at BLOCK scope inside every user with that
 * user's original type; no file-scope copy exists.
 */

/**
 * @brief Per-entry byte-offset table into the D_8003ECA0 staging area.
 * @note Sits 4 bytes past D_8003ECA0 (0x8003ECA4). func_800A3858 forms a
 *       pointer as @c (u8*)&D_8003ECA4 - 4 + D_8003ECA4[index], i.e.
 *       &D_8003ECA0 + D_8003ECA4[index].
 */
extern s32 D_8003ECA4[];

/** @brief Alternate AKAO sequence staging area inside the FIELD overlay. */
extern unsigned char D_80117EF8;

/* Consistent-type shared globals (same declared type in every user). */
extern s32 D_80117EE0;
extern s32 D_80117EE4;
extern s32 D_80117EE8;
extern s32 D_80117EF0;
extern s32 D_80119EF8;
extern s32 D_8011F300;
extern s32 D_8011F320;
extern s32 D_8011F328;
extern u8 D_8011F358[];
extern s32 D_8011588C;
extern s32 D_8011F310;
extern s32 D_8011F314;
extern AkaoHeader *D_8011F304;
extern u8 *D_8010D038;
extern u8 D_800EC398[];

/* External callees (declared here with the signature every user shares). */
void cdrom_queue_read(s32 resource_index, void *dst_buffer);
void cdrom_wait_queue_empty(void);
void akao_cmd_f1(void);
void akao_play_sfx(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
void akao_cmd_21(s32, s32);
s32 func_80022EF8(void *bank_id, s32 arg1, s32 arg2);
void func_80022ED8(void *p, s32 index, s32 one);

/* Forward references to callees defined later in this TU. */
void func_800A39A8(s32 sfx_index, s32 pan, s32 unused, s32 channel_group);
void *func_800A4348(s32 arg0, void *arg1);

/**
 * @brief Restore the shared effect bank when entering FIELD from a state that did not preserve it.
 * @see decomp.me (100.00%)
 */
void field_restore_entry_music(void)
{
    u8 *base;
    u32 *off;

    if (((u32)(g_previousGameState - 2) >= 2U) && (g_previousGameState != 0) &&
        (g_previousGameState != 6) && (g_previousGameState != 7) && (g_previousGameState != 5))
    {
        D_8011F304 = (AkaoHeader *)0x8013C000;
        cdrom_queue_read(CD_RES_SOUND_EFFECT_SET, (void *)EFFECT_BLOB_BASE);
        cdrom_wait_queue_empty();

        base = (u8 *)EFFECT_BLOB_BASE;
        off = (u32 *)EFFECT_BLOB_OFFSETS;
        bcopy(base + off[0], (u8 *)D_8011F304, (s32)(off[1] - off[0]));
        akao_register_bank(D_8011F304);
        akao_upload_bank_blocking((AkaoBankHeader *)(base + off[1]), 1);
    }
}

/**
 * @brief Load an AKAO instrument bank from a fixed CD resource and register it.
 * @param arg0 Base resource index; the read uses arg0 + 0x8E.
 * @return The bank handle returned by akao_register_bank.
 */
s32 func_800A35F4(s32 arg0)
{
    akao_cmd_f1();
    D_8011F304 = (AkaoHeader *) 0x8013C000;
    cdrom_queue_read((arg0 + 0x8E) & 0xFFFF, (void *) 0x8013C000);
    cdrom_wait_queue_empty();
    return akao_register_bank(D_8011F304);
}

/**
 * @brief Load the shared SEQ blob at index 0x16 and upload its bank.
 */
void func_800A3654(void)
{
    cdrom_queue_read(0x16, (void *)SEQ_BLOB_BASE);
    cdrom_wait_queue_empty();
    akao_upload_bank_blocking((AkaoBankHeader *)SEQ_BLOB_BASE, 1);
}

/**
 * @brief Load a field SEQ resource from CD-ROM and submit it for playback.
 *
 * @details Counterpart of TITLE's load_title_seq. Reads resource
 * @c CD_RES_MUSIC_FILE(music_index) into the SEQ_BLOB_BASE scratch
 * buffer, splits the blob via its leading offset table, copies the song
 * sequence to one of two staging areas, then uploads the trailing instrument
 * bank through akao_upload_bank_blocking.
 *
 * @param music_index Music-file index; 0 selects MSC_DATA.DAT. Indices above
 *        FIELD_SEQ_MAX_INDEX are ignored.
 * @param destination_index Selects the staging area for the copied sub-block:
 *        0 picks D_8003ECA0, non-zero picks D_80117EF8.
 *
 * @see decomp.me (100%) TODO
 */
void func_800A368C(s32 music_index, s32 destination_index)
{
    extern unsigned char D_8003ECA0;
    u32* off;
    u8* src;
    s32 count;

    if (music_index < FIELD_SEQ_MAX_INDEX + 1)
    {
        cdrom_queue_read(CD_RES_MUSIC_FILE(music_index), (void*)SEQ_BLOB_BASE);
        cdrom_wait_queue_empty();

        off = (u32*)SEQ_BLOB_OFFSETS;

        count = off[1] - off[0];
        src = (u8*)(off[0] + SEQ_BLOB_BASE);

        if (destination_index != 0)
        {
            bcopy(src, &D_80117EF8, count);
        }
        else
        {
            bcopy(src, &D_8003ECA0, count);
        }

        akao_upload_bank_blocking((AkaoBankHeader*)(off[1] + SEQ_BLOB_BASE), 1);
    }
}

/**
 * @brief Load the fixed field SEQ resource from CD-ROM and play it.
 *
 * @details Variant of func_800A368C with no parameters: the resource index is
 * hardcoded and the destination is always D_8003ECA0. The scratch blob begins
 * with the number of offset entries, and the last entry supplies both the copy
 * length and the byte offset of the AKAO instrument-bank sub-block.
 */
void func_800A3728(void)
{
    extern unsigned char D_8003ECA0;
    u8* dst;
    u32* off_end;
    u32 count;

    cdrom_queue_read(FIELD_FIXED_SEQ_RESOURCE, (void*)SEQ_BLOB_BASE);
    cdrom_wait_queue_empty();

    dst = &D_8003ECA0;
    count = SEQ_BLOB_BASE;
    count = *(u32*)count;
    off_end = (u32*)SEQ_BLOB_OFFSETS + count;

    bcopy((u8*)SEQ_BLOB_BASE, dst, off_end[-1]);
    akao_upload_bank_blocking((AkaoBankHeader*)(off_end[-1] + SEQ_BLOB_BASE), 1);
}

/**
 * @brief Stop the field background music.
 *
 * @details Counterpart of TITLE's stop_title_music and CHECKPS's func_800501AC.
 *
 * @see decomp.me (100%) TODO
 */
void field_stop_song(void)
{
    akao_stop_song(0);
}

/**
 * @brief Stop the field background music using the pending stop modifier.
 *
 * @details Same AKAO command as field_stop_song, but passes the current value
 * of D_8011F310 instead of a hardcoded 0.
 *
 * @see decomp.me (100%) TODO
 */
void func_800A37BC(void)
{
    akao_stop_song(D_8011F310);
}

/**
 * @brief Stop the field background music using the second stop modifier.
 *
 * @details Identical to func_800A37BC except that it passes D_8011F314.
 *
 * @see decomp.me (100%) TODO
 */
void func_800A37E4(void)
{
    akao_stop_song(D_8011F314);
}

/**
 * @brief Start the field background music staged at D_8003ECA0.
 *
 * @details Submits the staged sequence with akao_play_song, records its song
 * handle as the stop modifier for func_800A37BC,
 * applies the current music volume, then issues AKAO commands 0xD4 and 0xD0
 * with 0. Called cross-overlay by GOVER after it stages its own sequence.
 *
 * @see decomp.me (100%) TODO
 */
void func_800A380C(void)
{
    extern unsigned char D_8003ECA0;
    s32 play_result;

    play_result = akao_play_song((AkaoHeader*)&D_8003ECA0);
    D_8011F310 = play_result;
    akao_set_song_volume(play_result, D_8011588C);
    akao_cmd_d4(0);
    akao_cmd_d0(0);
}

/**
 * @brief Start a staged AKAO sequence selected by table index.
 * @param arg0 Index into the D_8003ECA4 byte-offset table.
 * @see decomp.me (100%) TODO
 */
void func_800A3858(s32 arg0)
{
    s32 play_result;
    s32 dummy;

    play_result = akao_cmd_14((u8*)&D_8003ECA4 - 4 + D_8003ECA4[arg0], dummy, 0);
    D_8011F310 = play_result;
    if (play_result == -1)
    {
        D_8011F310 = 0;
    }
    else if (play_result == 0)
    {
        D_8011F310 = 0;
    }
    else
    {
        akao_set_song_volume(0, D_8011588C);
        akao_cmd_d4(0);
    }
    akao_cmd_d0(0);
}

/**
 * @brief Latch the current SFX-group stop modifier from the AKAO driver.
 */
void func_800A38D4(void)
{
    s32 temp_v0;

    temp_v0 = akao_cmd_19_c0((s32) &D_80117EF8, D_8011588C);
    D_8011F314 = temp_v0;
}

/**
 * @brief Issue AKAO command 0xC1 for the stop modifier at the given index.
 * @param arg0 Index into the D_8011F310 stop-modifier pair.
 */
void func_800A3904(s32 arg0)
{
    akao_cmd_c1((&D_8011F310)[arg0]);
}

/**
 * @brief Play a FIELD sound effect at maximum volume.
 * @param sound_id Sound id forwarded to akao_play_sfx's arg0.
 * @param pan Forwarded to akao_play_sfx's arg2.
 */
void func_800A3938(s32 sound_id, s32 pan)
{
    akao_play_sfx(sound_id, 0, pan, 0x7F);
}

/**
 * @brief Play a FIELD sound effect with the pan value doubled.
 * @param arg0 Sound id forwarded to akao_play_sfx's arg0.
 * @param arg1 Pan value; passed as arg1 * 2 to akao_play_sfx's arg2.
 */
void func_800A3960(s32 arg0, s32 arg1)
{
    akao_play_sfx(arg0, 0, arg1 * 2, 0x7F);
}

/**
 * @brief Play a FIELD sound effect in channel group 0.
 * @param sfx_index Sound-effect table index forwarded to func_800A39A8.
 * @param pan Pan value forwarded to func_800A39A8.
 * @param unused Forwarded to func_800A39A8's third argument.
 */
void func_800A3988(s32 sfx_index, s32 pan, s32 unused)
{
    func_800A39A8(sfx_index, pan, unused, 0);
}

/**
 * @brief Play a sound effect from the primary SFX table in a channel group.
 * @param sfx_index Sound-effect table index.
 * @param pan Pan value passed to the sound-effect player.
 * @param arg2 Unused in the body (kept for the shared dispatch signature).
 * @param channel_group Three-channel group selector; clamped to 7.
 * @see decomp.me (100%) TODO
 */
void func_800A39A8(s32 sfx_index, s32 pan, s32 arg2, s32 channel_group)
{
    extern s32 D_80119F00;
    s32 *table;
    s32 base;
    s32 i;
    s32 mask;
    s32 buf;

    if (D_80119F00 != 0)
    {
        if (channel_group >= 8)
        {
            channel_group = 7;
        }
        table = (s32 *)((s32)&D_80119F00 + D_80119F00);
        if ((u32)sfx_index < (u32)(table[0] - 1))
        {
            buf = (s32)table;
            i = 0;
            base = channel_group * 3;
            buf += ((s32 *)buf)[sfx_index + 1];
            for (; i < 3; i++)
            {
                mask = 1 << (base + i);
                if (!akao_is_sfx_playing(mask))
                {
                    akao_play_sfx_from_buffer(buf, mask, pan, 0x7F);
                    break;
                }
            }
        }
    }
}

/**
 * @brief Play a sound effect in the first available channel of a channel group.
 * @param sfx_index Sound-effect table index.
 * @param pan Pan value passed to the sound-effect player.
 * @param arg2 Sound-effect table selector.
 * @param channel_group Three-channel group selector.
 */
void func_800A3A90(s32 sfx_index, s32 pan, s32 arg2, s32 channel_group)
{
    extern s32 D_8011BF00;
    s32 *table;
    s32 base;
    s32 i;
    s32 mask;
    s32 buf;
    u8 *p;

    if (arg2 < 2)
    {
        p = (u8 *)&D_8011BF00;
        table = (s32 *)(p + arg2 * 0x1A00);
        if (table[0] != 0)
        {
            if ((u32)sfx_index < (u32)table[0])
            {
                i = 0;
                base = arg2 * 3;
                buf = (s32)table + table[sfx_index + 1];
                for (; i < 3; i++)
                {
                    mask = 1 << (base + i);
                    if (!akao_is_sfx_playing(mask))
                    {
                        akao_play_sfx_from_buffer(buf, mask, pan, 0x7F);
                        break;
                    }
                }
            }
        }
    }
}

/**
 * @brief Issues three AKAO command-21 voice masks for a clamped channel index.
 *
 * Clamps @p idx to a maximum of 7, then emits akao_cmd_21(0, 1 << bit) for the
 * three consecutive bits starting at idx * 3.
 *
 * @param idx Channel-group index; clamped to a maximum of 7.
 */
void func_800A3B78(s32 idx)
{
    s32 bit;

    if (idx >= 8)
    {
        idx = 7;
    }
    bit = idx * 3;
    akao_cmd_21(0, 1 << bit);
    akao_cmd_21(0, 1 << (bit + 1));
    akao_cmd_21(0, 1 << (bit + 2));
}

/**
 * @brief Load field sound-bank tables and upload their associated audio banks.
 * @param bank_id Resource index; -2 preserves state and -1 clears only the header.
 */
void func_800A3BE8(s32 bank_id)
{
    extern FieldBankCopyHeader D_80119F00;
    FieldBankCopyHeader *header;
    u8 *src;
    u8 *cursor;
    u8 *copy_cursor;
    u8 *sub_block;
    u8 *sub_block_end;
    u8 *flag;
    u8 *header_base;
    s32 *offsets;
    s32 resource_id;
    s32 count;
    s32 i;

    if (bank_id == -2)
    {
        return;
    }

    offsets = (s32 *)&D_80119F00;
    offsets[2] = 0;
    offsets[1] = 0;
    D_80119F00.table_offset = 0;

    resource_id = bank_id + 0x51;
    if (bank_id == -1)
    {
        return;
    }

    resource_id = (u16)resource_id;
    header = (FieldBankCopyHeader *)offsets;
    if (bank_id != 0)
    {
        header = (FieldBankCopyHeader *)offsets;
    }
    cursor = (u8 *)header + sizeof(*header);
    offsets = (s32 *)D_8010D038;

    cdrom_queue_read(resource_id, offsets);
    src = (u8 *)offsets;
    offsets = NULL;
    cdrom_wait_queue_empty();

    i = 0;
    offsets = (s32 *)(src + 4);
    do
    {
        count = *(volatile s32 *)src;
    } while (0);

    if (count <= 0)
    {
        return;
    }

    header_base = (u8 *)header;

    flag = (u8 *)bank_id;
    flag += (s32)D_800EC398;

    do
    {
        header->table_offset = (s32)(((cursor - header_base) >> 2) * 4);

        sub_block = src + (s32)(sub_block = (u8 *)*offsets);
        sub_block_end = sub_block + *(s32 *)(sub_block + (*(s32 *)sub_block) * 4);

        copy_cursor = cursor;
        if (sub_block != sub_block_end)
        {
            do
            {
                *copy_cursor = *sub_block;
                sub_block++;
                copy_cursor++;
            } while (sub_block != sub_block_end);
        }

        cursor = copy_cursor;
        if (*flag == 0)
        {
            akao_upload_bank_blocking((AkaoBankHeader *)sub_block_end, 1);
            return;
        }

        func_80022EF8(sub_block_end, i, 1);
        i++;
        offsets++;
    } while (i < count);
}

/**
 * @brief Load a single sound-bank table and hand it to the streaming uploader.
 * @param arg0 Destination table slot (0 or 1); selects a 0x1A00-byte region.
 * @param arg1 Resource index; -2 preserves state and -1 clears only the header.
 */
void func_800A3D44(s32 arg0, s32 arg1)
{
    extern u8 D_8011BF00[];
    u8 *dst;
    u8 *base;
    u8 *src;
    u8 *end;
    u8 *dst_cursor;

    if (arg1 != -2)
    {
        base = D_8011BF00;
        dst = base + arg0 * 0x1A00;
        *(s32 *)dst = 0;
        if (arg1 != -1)
        {
            arg1 += 0x83;
            src = D_8010D038;
            cdrom_queue_read(arg1 & 0xFFFF, src);
            cdrom_wait_queue_empty();
            end = src + *(s32 *)(src + (*(s32 *)src * 4));
            dst_cursor = dst;
            if (src != end)
            {
                do
                {
                    *dst_cursor++ = *src++;
                } while (src != end);
            }
            func_80022ED8(end, arg0, 1);
        }
    }
}

/**
 * @brief Finds an available SFX slot in the selected three-bit group.
 *
 * @param arg0 Buffer address forwarded to akao_play_sfx_from_buffer.
 * @param arg1 Pan value forwarded to akao_play_sfx_from_buffer.
 * @param arg2 Channel-group selector; group base is arg2 * 3.
 *
 * @note 100% match. The function intentionally has no explicit return: the
 *       original codegen preserves the last status/call result in v0.
 */
s32 func_800A3E10(s32 arg0, s32 arg1, s32 arg2)
{
    s32 base;
    s32 i;
    s32 mask;

    if (arg2 * 3 < 0x18)
    {
        i = 0;
        base = arg2 * 3;
        for (; i < 3; i++)
        {
            mask = 1 << (base + i);
            if (!akao_is_sfx_playing(mask))
            {
                akao_play_sfx_from_buffer(arg0, mask, arg1, 0x7F);
                break;
            }
        }
    }
}

/**
 * @brief Reset the CD streaming state block to idle.
 */
void func_800A3EBC(void)
{
    extern s32 D_80117EEC;
    extern s32 D_8011F308;
    extern s32 D_8011F324;

    D_80117EE0 = 0;
    D_80117EEC = 0;
    D_8011F324 = 0;
    D_8011F328 = 0;
    D_8011F308 = 0;
    D_80117EF0 = 0;
    D_80119EF8 = 0;
    D_80117EE8 = 0;
    D_8011F320 = 0;
    D_8011F300 = 0;
    D_80117EE4 = 0;
}

/**
 * @brief Kick off a guarded CD streaming read when the channel is idle.
 *
 * When the busy flag @c D_80117EE4 is clear, resets the streaming state block
 * and issues a queued CD read for resource index @p arg0 + 0x17, latching the
 * completion callback func_800A4348 and its queue handle in @c D_80117EF0.
 *
 * @param arg0 Base resource index; the read uses arg0 + 0x17.
 *
 * @see decomp.me (100%) TODO
 */
void func_800A3F18(s32 arg0)
{
    extern s32 D_80117EEC;
    extern s32 D_8011F308;
    extern s32 D_8011F324;

    if (D_80117EE4 == 0)
    {
        D_80117EE0 = 0;
        D_80117EEC = 0;
        D_8011F324 = 0;
        D_8011F328 = 0;
        D_8011F308 = 0;
        D_80117EF0 = 0;
        D_80119EF8 = 0;
        D_80117EE8 = 0;
        D_8011F320 = 0;
        D_8011F300 = 0;
        D_80117EE4 = arg0 + 0x17;
        D_80117EF0 = cdrom_queue_read_with_callback((u16)D_80117EE4, &func_800A4348);
    }
}

/**
 * @brief Advance the field music block copy, upload and playback state machine.
 * @note The resident song is assembled at D_8003ECA0; bank data uses 0x801DD000.
 * @note Volatile header reads preserve the original repeated offset loads.
 * @note GCC 2.7.2 CDK matches all 220 instructions (880 bytes).
 */
void func_800A3FB0(void)
{
    s32 akao_cmd_d0(s32);
    s32 akao_cmd_d4(s32);
    s32 akao_get_xfer_state(void);
    s32 akao_reset_xfer_state(void);
    s32 akao_streaming_upload_tick(u8 *source, u32 avail, s32 wait_for_spu);
    void func_800A4320(u8 *, u8 *, s32);
    extern u8 D_8003ECA0[];
    extern u8 *D_80117EEC;
    extern u32 D_8011F308;
    extern u32 D_8011F324;
    s32 resident_bytes;
    s32 upload_bytes;
    s32 remaining_bytes;
    s32 remaining_bytes_second;
    s32 data_offset;
    s32 song_handle;
    s32 resident_remaining;

    if (D_80117EE0 != 0)
    {
        switch (D_8011F308)
        {
        case 1:
            if (D_80119EF8 != 0)
            {
                if (D_80119EF8 < 0x800)
                {
                    bcopy(D_80117EEC, D_80117EE8 + D_8003ECA0, D_80119EF8);
                    bcopy(D_80117EEC + D_80119EF8, (void *)0x801DD000, 0x800 - D_80119EF8);
                    resident_bytes = D_80119EF8;
                    D_8011F300 = 1;
                    D_80119EF8 = 0;
                    D_8011F320 = 0x800 - resident_bytes;
                    D_80117EE8 += resident_bytes;
                }
                else
                {
                    bcopy(D_80117EEC, D_80117EE8 + D_8003ECA0, 0x800);
                    resident_remaining = D_80119EF8 - 0x800;
                    D_80117EE8 += 0x800;
                    D_80119EF8 = resident_remaining;
                    if (resident_remaining == 0)
                    {
                        D_8011F320 = 0;
                        D_8011F300 = 1;
                    }
                }
                goto release_block;
            }
            if (D_8011F300 != 0)
            {
                bcopy(D_80117EEC, (void *)(D_8011F320 + 0x801DD000), 0x800);
                D_8011F320 += 0x800;
                akao_reset_xfer_state();
                akao_streaming_upload_tick((u8*)0x801DD000, 0x800, 1);
                remaining_bytes = D_8011F320 - 0x800;
                D_8011F320 = remaining_bytes;
                func_800A4320((void *)0x801DD000, (void *)0x801DD800, remaining_bytes);
                D_8011F308 = 3;
            }
            else
            {
                bcopy(D_80117EEC, (void *)(D_8011F320 + 0x801DD000), 0x800);
                if ((u32)D_8011F324 < 0x800U)
                {
                    upload_bytes = D_8011F320 + D_8011F324;
                    D_8011F320 = upload_bytes;
                    akao_streaming_upload_tick((u8*)0x801DD000, upload_bytes, 1);
                    D_8011F308 = 4;
                    D_8011F320 = 0;
                }
                else
                {
                    D_8011F320 += 0x800;
                    akao_streaming_upload_tick((u8*)0x801DD000, 0x800, 1);
                    D_8011F308 = 3;
                    remaining_bytes_second = D_8011F320 - 0x800;
                    D_8011F320 = remaining_bytes_second;
                    func_800A4320((void *)0x801DD000, (void *)0x801DD800, remaining_bytes_second);
                }
            }
            D_8011F300 = 0;
            return;
        case 2:
            data_offset = *(volatile s32 *)(D_80117EEC + 4);
            D_80119EF8 = *(s32 *)(D_80117EEC + 8) - *(volatile s32 *)(D_80117EEC + 4);
            bcopy(D_80117EEC + data_offset, D_8003ECA0, 0x800 - data_offset);
            D_80117EE0 = 0;
            D_80117EE8 = 0x800 - *(volatile s32 *)(D_80117EEC + 4);
            D_80119EF8 -= 0x800 - *(volatile s32 *)(D_80117EEC + 4);
            return;
        case 3:
            if (akao_get_xfer_state() == 0)
            {
            release_block:
                D_80117EE0 = 0;
            }
            break;
        case 4:
            if (akao_get_xfer_state() == 0)
            {
                D_80117EE0 = 0;
                song_handle = akao_play_song((AkaoHeader *)D_8003ECA0);
                D_8011F310 = song_handle;
                akao_set_song_volume(song_handle, D_8011588C);
                akao_cmd_d4(0);
                akao_cmd_d0(0);
                D_80117EE4 = 0;
            }
            break;
        case 0:
        default:
            break;
        }
    }
}

/**
 * @brief Copy a run of bytes from one buffer to another.
 * @param dst Destination buffer.
 * @param src Source buffer.
 * @param count Number of bytes to copy.
 */
void func_800A4320(u8 *dst, u8 *src, s32 count)
{
    u8 temp;

    if (count > 0)
    {
        do
        {
            temp = *src;
            src += 1;
            count -= 1;
            *dst = temp;
            dst += 1;
        } while (count != 0);
    }
}

/**
 * @brief Claim the fixed buffer at 0x801DC000 (or 0x801DC800 when bit 11 of arg0 is set) if it is free.
 *
 * Records arg0 and arg1 in D_8011F328 and D_8011F324, sets D_8011F308 to 2
 * when arg0 is 0 and to 1 otherwise, and marks the buffer busy via D_80117EE0.
 *
 * @param arg0 Request word; bit 11 selects the upper buffer half.
 * @param arg1 Stored to D_8011F324.
 * @return The claimed buffer, or NULL when it is already busy.
 * @see decomp.me (100%)
 */
void *func_800A4348(s32 arg0, void *arg1)
{
    extern void *D_80117EEC;
    extern s32 D_8011F308;
    extern void *D_8011F324;
    void *new_var;
    void *ptr;

    if (D_80117EE0 == 0)
    {
        if (arg0 & 0x800)
        {
            new_var = (void *)0x801DC800;
            ptr = new_var;
        }
        else
        {
            ptr = (void *)0x801DC000;
        }
        D_80117EEC = ptr;
        if (arg0 == 0)
        {
            D_8011F308 = 2;
        }
        else
        {
            D_8011F308 = 1;
        }
        D_8011F328 = arg0;
        new_var = arg1;
        D_8011F324 = new_var;
        D_80117EE0 = 1;
        return ptr;
    }
    return (void *)0;
}

/**
 * @brief Zero the 30 bytes of D_8011F358, last byte first.
 */
void func_800A43C0(void)
{
    s32 i = 0x1D;
    u8 *p = &D_8011F358[i];

    for (; i >= 0; i--)
    {
        *p = 0;
        p--;
    }
}
