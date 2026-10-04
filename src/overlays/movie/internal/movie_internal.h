#ifndef LOM_MOVIE_INTERNAL_H
#define LOM_MOVIE_INTERNAL_H

#include "overlays/movie/movie.h"
#include "main/cdrom.h"
#include "main/audio/akao_cmd.h"
#include "overlays/movie/movie_state.h"
#include "common/pad.h"
#include "main/controller.h"
#include "sdk/libgte.h"
#include "sdk/libgpu.h"
#include "sdk/libpress.h"
#include "sdk/libcd.h"

/** @brief MovieState::end_state sentinel values. */
typedef enum
{
    END_STATE_RUNNING,
    END_STATE_NEAR_END,
    END_STATE_DONE
} MovieEndState;

/** @brief MDEC output pipeline states. */
typedef enum
{
    MDEC_STATE_IDLE,
    MDEC_STATE_ACTIVE,
    MDEC_STATE_CHAINED
} MovieMdecState;

/** @brief GPU transfer mode using DrawSync and LoadImage. */
#define MOVIE_GPU_MODE_STANDARD 0

/** @brief VLC decode and XA audio state used by @ref movie_update. */
#define STANDARD_VLC_DECODE_SIZE 4096
#define ALTERNATE_VLC_DECODE_SIZE 5802
#define STANDARD_VLC_RETRY_COUNT 3
#define ALTERNATE_VLC_RETRY_COUNT 1
#define AUDIO_STREAM_STATE_IDLE 0
#define AUDIO_STREAM_STATE_SECTOR_READY 1
#define AUDIO_STREAM_STATE_PRIMED 2
#define AKAO_COMMAND_SELECTOR_9E 3
#define AKAO_XA_POSITION_UNAVAILABLE (-1)
#define AUDIO_SECTORS_PER_XA_FRAME 2

/** @brief GPU and CD status values used by the MDEC output callback. */
#define CD_READY_CALLBACK_PENDING 1
#define DRAW_SYNC_MODE_POLL 1
#define DRAW_SYNC_DEFER_THRESHOLD 2
#define BREAK_DRAW_FAILURE ((u_long*)-1)

/** @brief Sector types in the movie stream header, after the CD sector subheader. */
#define SECTOR_TYPE_VIDEO 0x8001
#define SECTOR_TYPE_AUDIO 1

/** @brief MovieState::continuation_type values; selects which ring a continuation sector goes into. */
#define CONTINUATION_VIDEO 0
#define CONTINUATION_AUDIO 1

/** @brief CD sector geometry consumed by @ref cd_sector_callback. */
#define CD_SECTOR_BYTES 2048
#define CD_HEADER_WORDS 8
#define CD_HEADER_BYTES (CD_HEADER_WORDS * sizeof(u32))
#define CD_PAYLOAD_BYTES (CD_SECTOR_BYTES - CD_HEADER_BYTES)
#define CD_PAYLOAD_WORDS (CD_PAYLOAD_BYTES / sizeof(u32))

/**
 * @brief Header layout shared by video- and audio-ring entries.
 *
 * 12 bytes total. The same layout is the leading prefix of every 32-byte
 * movie header read by @ref cd_sector_callback. Used for both
 * video (video_table_base, 32-byte stride) and audio (audio_data_base,
 * 2048-byte stride) ring entries.
 */
typedef struct
{
    u16 unknown;          /**< First halfword of the stream header. */
    u16 sector_type;      /**< Video or audio sector type. */
    u16 chunk_sector_idx; /**< Sector position within a multi-sector frame. */
    u16 sector_count;     /**< Sectors comprising this frame. */
    u32 frame_number;
} SectorEntry;

/**
 * @brief One video-ring table entry: 32 bytes copied as 8 u32 words by
 *        @ref cd_sector_callback.
 *
 * The first 12 bytes are the SectorEntry header; the remaining 20 bytes hold
 * sector metadata. The actual VLC payload lives in a parallel buffer
 * (video_data_base, 2016-byte stride).
 */
typedef union VideoSectorEntry
{
    SectorEntry header;
    u32 words[CD_HEADER_WORDS];
} VideoSectorEntry;

/**
 * @brief One PSX CD sector (2048 bytes) of audio ring data.
 *
 * The 32-byte movie header is followed by 2016 bytes of AKAO stream data.
 */
typedef struct AudioSector
{
    VideoSectorEntry header_block;
    u8 payload[CD_PAYLOAD_BYTES];
} AudioSector;

/**
 * @brief One slot of the video VLC payload buffer: 2016 bytes of raw
 *        bitstream data.
 *
 * video_data_base is a parallel array of these, indexed by the same
 * read/write indices as video_table_base.
 */
typedef union VideoVlcPayload
{
    u8 data[CD_PAYLOAD_BYTES];
    u_long words[CD_PAYLOAD_WORDS];
} VideoVlcPayload;

void movie_mdec_out_callback(void);
u8* cd_sector_callback(s32 bytes_transferred, u32 bytes_remaining);
s32 get_next_audio_entry(AudioSector** out_entry);
void draw_sync_callback(void);
s32 get_next_video_entry(VideoVlcPayload** out_vlc_data, VideoSectorEntry** out_entry_header);
void advance_audio_read(void);
void advance_video_read(void);

#endif
