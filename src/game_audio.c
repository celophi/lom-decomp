#include "game_audio.h"
#include "akao.h"
#include "cdrom.h"
#include "cd_resources.h"
#include "sdk/memory.h"

#define SONG_FADE_OUT_TICKS 300
#define SONG_INDEX_NONE 0xFF
#define SONG_SEQUENCE_BUFFER_SIZE 1024
#define SONG_LOAD_BUFFER ((void*)0x80180000)

/** @brief Resident song sequence with its AKAO header. */
typedef union
{
    AkaoHeader header;
    u8 bytes[SONG_SEQUENCE_BUFFER_SIZE];
} SongSequence;

extern s32 g_current_song_handle;
extern SongSequence g_music_sequence;
extern s16 g_last_error_status;
extern s16 g_game_diagnostic_code;
extern s16 g_game_diagnostic_arg0;
extern s16 g_game_diagnostic_arg1;

/**
 * @brief Fade the current song to silence over 300 audio ticks.
 * @see decomp.me (100%) https://decomp.me/scratch/mXgky
 */
void fade_out_current_song(void)
{
    akao_cmd_c1(g_current_song_handle, SONG_FADE_OUT_TICKS, 0);
}

/**
 * @brief Load a SND_SOTO song and its instrument bank, then start playback.
 * @param song_index Song resource index, or SONG_INDEX_NONE to leave playback unchanged.
 * @see decomp.me (100%) https://decomp.me/scratch/Oy5Dh
 */
void load_and_play_song(s32 song_index)
{
    AkaoContainerHeader* container;
    u8* container_data;
    u32 resource_index;
    u32* section_offsets;
    SongSequence* sequence;
    void* bank_data;
    s32 song_handle;

    if (song_index == SONG_INDEX_NONE)
    {
        return;
    }

    resource_index = (song_index + CD_RES_SND_SOTO_SET) & 0xFFFF;
    container = SONG_LOAD_BUFFER;
    cdrom_queue_read(resource_index, container);
    cdrom_wait_queue_empty();
    container_data = SONG_LOAD_BUFFER;
    section_offsets = container->section_offsets;
    sequence = &g_music_sequence;

    bcopy(&container_data[section_offsets[0]], sequence->bytes, section_offsets[1] - section_offsets[0]);
    bank_data = &container_data[section_offsets[1]];
    akao_upload_bank_blocking(bank_data, TRUE);

    song_handle = akao_play_song(&sequence->header);
    g_current_song_handle = song_handle;
    akao_set_song_volume(song_handle, AKAO_VOLUME_MAX);
}

/**
 * @brief Record a diagnostic and retain the last error status.
 * @param status Diagnostic status; a negative 16-bit value marks an error.
 * @param code Diagnostic code.
 * @param arg0 First diagnostic value.
 * @param arg1 Second diagnostic value.
 * @see decomp.me (100%) https://decomp.me/scratch/DRBPP
 */
void record_game_diagnostic(s32 status, s32 code, s32 arg0, s32 arg1)
{
    g_game_diagnostic_status = status;

    if ((s16)status < 0)
    {
        g_last_error_status = status;
    }

    g_game_diagnostic_code = code;
    g_game_diagnostic_arg0 = arg0;
    g_game_diagnostic_arg1 = arg1;
}
