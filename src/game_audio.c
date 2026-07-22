#include "game_audio.h"

/**
 * decomp.me link (100%) https://decomp.me/scratch/mXgky
 */
void akao_song_cmd_12c(void)
{
    akao_cmd_c1(g_current_song_handle, 0x12C, 0);
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/Oy5Dh
 */
void load_and_play_song(s32 song_index)
{
    u32 cd_dest;
    u32 res_index;
    s32* seq_header;
    u8* seq_dest;
    s32 song_handle;

    if (song_index == 0xFF)
    {
        return;
    }

    res_index = (song_index + 0x93) & 0xFFFF;
    cd_dest = 0x80180000;
    cdrom_queue_read(res_index, cd_dest);
    cdrom_wait_queue_empty();
    cd_dest = 0x80180000;
    seq_header = (s32*)0x80180004;
    seq_dest = &g_music_data_buffer;

    bcopy((u_char*)((*seq_header) + cd_dest), seq_dest, seq_header[1] - (*seq_header));
    akao_play_sequence_blocking((AkaoSeqHeader*)(seq_header[1] + cd_dest), 1);

    song_handle = akao_play_song(seq_dest);
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
