#include "common/overlay_audio.h"
#include "main/audio/akao.h"
#include "main/cd_resources.h"
#include "main/cdrom.h"
#include <memory.h>

/**
 * @brief Load a music file from CD, keep its sequence in g_overlay_song_buffer and upload its instrument bank.
 * @param song_index Music-file index; 0 selects MSC_DATA.DAT.
 */
void load_song_from_disc(s32 song_index)
{
    u32* offsets;
    u8* base;

    cdrom_queue_read(CD_RES_MUSIC_FILE(song_index), LOAD_BUFFER_AKAO_CONTAINER);
    cdrom_wait_queue_empty();

    offsets = LOAD_BUFFER_AKAO_CONTAINER->section_offsets;
    base = (u8*)LOAD_BUFFER_AKAO_CONTAINER;

    bcopy(base + offsets[AKAO_CONTAINER_RESIDENT_SECTION], g_overlay_song_buffer,
          (s32)(offsets[AKAO_CONTAINER_BANK_SECTION] - offsets[AKAO_CONTAINER_RESIDENT_SECTION]));
    akao_upload_bank_blocking((AkaoBankHeader*)(base + offsets[AKAO_CONTAINER_BANK_SECTION]), 1);
}
