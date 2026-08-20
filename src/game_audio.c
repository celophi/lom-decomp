#include "game_audio.h"
#include "akao.h"
#include "cdrom.h"

extern s32 g_current_song_handle;
extern u8 g_music_data_buffer;
extern s16 g_akao_song_cmd_arg0;
extern s16 g_akao_song_cmd_neg_arg0;
extern s16 g_akao_song_cmd_arg1;
extern s16 g_akao_song_cmd_arg2;
extern s16 g_akao_song_cmd_arg3;

/**
 * @brief Send command 0xC1 for the active song with parameter 0x12C.
 * @see decomp.me (100%) https://decomp.me/scratch/mXgky
 */
void akao_song_cmd_12c(void)
{
    akao_cmd_c1(g_current_song_handle, 0x12C, 0);
}

/**
 * @brief Load a song and its instrument bank, then start song playback.
 * @param song_index Zero-based song resource index, or 0xFF to do nothing.
 * @see decomp.me (100%) https://decomp.me/scratch/Oy5Dh
 */
void load_and_play_song(s32 song_index)
{
    u32 container_address;
    u32 resource_index;
    s32* section_offsets;
    u8* sequence_destination;
    s32 song_handle;

    if (song_index == 0xFF)
    {
        return;
    }

    resource_index = (song_index + 0x93) & 0xFFFF;
    container_address = 0x80180000;
    cdrom_queue_read(resource_index, container_address);
    cdrom_wait_queue_empty();
    container_address = 0x80180000;
    section_offsets = (s32*)0x80180004;
    sequence_destination = &g_music_data_buffer;

    bcopy((u_char*)((*section_offsets) + container_address), sequence_destination, section_offsets[1] - (*section_offsets));
    akao_upload_bank_blocking((AkaoBankHeader*)(section_offsets[1] + container_address), 1);

    song_handle = akao_play_song((AkaoHeader*)sequence_destination);
    g_current_song_handle = song_handle;
    akao_cmd_c0(song_handle, 0x7f);
}

/**
 * decomp.me link: (100%) https://decomp.me/scratch/DRBPP
 */
void akao_set_song_params(int flags, s16 duration, s16 field_id, s16 sub_id)
{
    g_akao_song_cmd_arg0 = flags;

    if ((flags << 0x10) < 0)
    {
        g_akao_song_cmd_neg_arg0 = flags;
    }

    g_akao_song_cmd_arg1 = duration;
    g_akao_song_cmd_arg2 = field_id;
    g_akao_song_cmd_arg3 = sub_id;
}
