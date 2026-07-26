/**
 * @file field_audio.c
 * @brief FIELD overlay AKAO audio helpers: loading SEQ resources from CD-ROM
 *        and starting/stopping field music.
 *
 * @note Several of these are called cross-overlay (GOVER calls func_800A368C
 *       and func_800A380C while FIELD is resident).
 */

#include "common.h"
#include "cd.h"
#include "akao.h"
#include "psyq/memory.h"

/* Scratch buffer the CD-ROM layer decompresses a SEQ resource into. The blob
 * begins with a self-referential offset table; see SEQ_BLOB_OFFSETS. */
#define SEQ_BLOB_BASE 0x80180000

/* Offset table at the head of the SEQ blob. off[0] is the byte offset of the
 * instrument/sample sub-block, off[1] the byte offset of the AKAO sequence
 * sub-block that follows it. */
#define SEQ_BLOB_OFFSETS 0x80180004

/* Resource index of the first field SEQ; music_index is added to this to
 * select a variant. Same base TITLE uses for its own SEQ loads. */
#define FIELD_SEQ_RESOURCE_BASE 0x17

/* Largest music_index this loader accepts; anything above is ignored. */
#define FIELD_SEQ_MAX_INDEX 0x100

/* Fixed CD resource loaded by func_800A3728. TODO: which SEQ this is has not
 * been confirmed; it is not part of the FIELD_SEQ_RESOURCE_BASE range. */
#define FIELD_FIXED_SEQ_RESOURCE 0x92

void akao_play_sequence_blocking(AkaoSeqHeader* sequence_data, s32 wait_for_completion);

/** @brief AKAO sequence staging area shared with the TITLE overlay. */
extern unsigned char D_8003ECA0;

/** @brief Alternate AKAO sequence staging area inside the FIELD overlay. */
extern unsigned char D_80117EF8;

/**
 * @brief First word of the SEQ scratch blob: the number of entries in the
 *        offset table that follows it at SEQ_BLOB_OFFSETS.
 * @note Aliases SEQ_BLOB_BASE. It is declared as a symbol rather than spelled
 *       as a literal because the original code emits it %hi/%lo, unlike the
 *       raw 0x80180000 constants used for the buffer base itself.
 */
extern u32 D_80180000;

/**
 * @brief Stop-modifier passed to akao_stop_song by func_800A37BC.
 * @note Written by func_800A380C (below) with the value akao_play_song leaves
 *       in the return register, and again by func_800A3858 (still asm).
 *       akao_stop_song's parameter is believed to be a fade-out duration or
 *       flag; its exact meaning is still unknown.
 */
extern s32 D_8011F310;

/**
 * @brief Second stop-modifier, passed to akao_stop_song by func_800A37E4.
 * @note Adjacent to D_8011F310 and written by func_800A38D4 (still asm). The
 *       pair look like two independently-tracked stop modifiers rather than
 *       one value, since each has its own writer and its own stop wrapper.
 */
extern s32 D_8011F314;

/** @brief Current field music volume. Aliased by GOVER as g_akao_music_volume. */
extern s32 D_8011588C;

/**
 * @brief Load a field SEQ resource from CD-ROM and submit it for playback.
 *
 * @details Counterpart of TITLE's load_title_seq. Reads resource
 * (FIELD_SEQ_RESOURCE_BASE + @p music_index) into the SEQ_BLOB_BASE scratch
 * buffer, splits the blob via its leading offset table, copies the
 * instrument/sample sub-block to one of two staging areas, then hands the
 * trailing sequence sub-block to akao_play_sequence_blocking.
 *
 * @param music_index Offset added to FIELD_SEQ_RESOURCE_BASE to select the
 *        SEQ variant. Indices above FIELD_SEQ_MAX_INDEX are ignored and the
 *        function returns without touching the CD-ROM.
 * @param destination_index Selects the staging area for the copied sub-block:
 *        0 picks D_8003ECA0, non-zero picks D_80117EF8.
 *
 * @see decomp.me (100%) TODO
 */
void func_800A368C(s32 music_index, s32 destination_index)
{
    u32* off;
    u8* src;
    s32 count;

    if (music_index < FIELD_SEQ_MAX_INDEX + 1)
    {
        cdrom_queue_read((music_index + FIELD_SEQ_RESOURCE_BASE) & 0xFFFF, (void*)SEQ_BLOB_BASE);
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

        akao_play_sequence_blocking((AkaoSeqHeader*)(off[1] + SEQ_BLOB_BASE), 1);
    }
}

/**
 * @brief Load the fixed field SEQ resource from CD-ROM and play it.
 *
 * @details Variant of func_800A368C with no parameters: the resource index is
 * hardcoded and the destination is always D_8003ECA0. It also locates its
 * sub-block differently -- rather than using the first two offset-table
 * entries, it reads the blob's leading entry count and takes the LAST offset,
 * off_end[-1], using it both as the copy length and as the byte offset of the
 * AKAO sequence sub-block.
 *
 * @note Unlike func_800A368C, the copy source is the blob base itself rather
 *       than base + off[0], so the copied region includes the header.
 *
 * @note NOT YET MATCHED. The single remaining defect is that the target emits
 *       @c "lui s0,%hi(D_80180000); lw s0,%lo(D_80180000)(s0)" (one register)
 *       where this C produces @c "lui v0,%hi(...); lw s0,%lo(...)(v0)" (two),
 *       which also displaces the SEQ_BLOB_OFFSETS constant from v0 to v1. All
 *       5 differing rows are that one cause; instruction count, frame layout
 *       and emit order are already exact. Binding @c dst before the @c off_end
 *       statement is required (+1 exact row); binding it after is worth 0.
 *       See working/func_800A3728/status.md for the 22 probe variants already
 *       measured inert, and do not re-probe those classes.
 *
 * @see decomp.me (98.97%) TODO
 */
void func_800A3728(void)
{
    u8* dst;
    u32* off_end;

    cdrom_queue_read(FIELD_FIXED_SEQ_RESOURCE, (void*)SEQ_BLOB_BASE);
    cdrom_wait_queue_empty();

    dst = &D_8003ECA0;
    off_end = (u32*)SEQ_BLOB_OFFSETS + D_80180000;

    bcopy((u8*)SEQ_BLOB_BASE, dst, off_end[-1]);
    akao_play_sequence_blocking((AkaoSeqHeader*)(off_end[-1] + SEQ_BLOB_BASE), 1);
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
 * @details Submits the staged sequence with akao_play_song, records the value
 * it leaves in the return register as the stop modifier for func_800A37BC,
 * applies the current music volume, then issues AKAO commands 0xD4 and 0xD0
 * with 0. Called cross-overlay by GOVER after it stages its own sequence.
 *
 * @note akao_play_song is declared void in src/akao_cmd.c, yet this caller
 *       consumes its return register, so the original very likely declared it
 *       as returning a value. It is left implicitly declared here to preserve
 *       that. What the returned value actually means is unknown; it is only
 *       ever fed back to akao_stop_song and akao_cmd_c0.
 *
 * @see decomp.me (100%) TODO
 */
void func_800A380C(void)
{
    s32 play_result;

    play_result = akao_play_song(&D_8003ECA0);
    D_8011F310 = play_result;
    akao_cmd_c0(play_result, D_8011588C);
    akao_cmd_d4(0);
    akao_cmd_d0(0);
}
