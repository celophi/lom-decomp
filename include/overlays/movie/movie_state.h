#ifndef MOVIE_STATE_H
#define MOVIE_STATE_H

#include "common.h"
#include <libgte.h>
#include <libgpu.h>

/** @brief Saved callback represented as either an SDK return value or handler. */
typedef union
{
    u32 address;
    void (*handler)(void);
} MovieCallback;

/** @brief Rectangle layout within MovieState. */
#define MOVIE_DISPLAY_RECT_COUNT 2
#define MDEC_OUTPUT_RECT_INDEX MOVIE_DISPLAY_RECT_COUNT
#define MOVIE_RECT_COUNT (MDEC_OUTPUT_RECT_INDEX + 1)

/**
 * @brief Shared movie buffers, decoder callbacks, and playback state.
 *
 * The block lives at a fixed address and is shared between the main loop
 * and the interrupt-driven callbacks (CD sector, MDEC output, DrawSync).
 * Completed frames become visible to the consumer when the sector callback
 * advances the write index and last-frame marker. GPU and MDEC callbacks
 * communicate pending work and completed output through the handshake flags.
 */
typedef struct
{
    union VideoSectorEntry* video_table_base;
    union VideoVlcPayload* video_data_base;
    struct AudioSector* audio_data_base;
    void* vlc_table;
    void* vlc_input_buf[2];
    u_long* mdec_output_buf[2];

    RECT rects[MOVIE_RECT_COUNT];

    MovieCallback dec_dct_out_callback;
    MovieCallback draw_sync_callback;

    u8 pad_40[4];

    u32 resource_index;
    u32 current_frame;
    u32 total_frames;
    s32 video_ring_capacity;
    s32 audio_ring_capacity;

    volatile s32 video_write_idx;
    s32 video_read_idx;
    volatile s32 video_wrap_idx; /**< End of the older contiguous ring segment. */
    volatile s32 audio_write_idx;
    s32 audio_read_idx;
    volatile s32 audio_wrap_idx; /**< End of the older contiguous ring segment. */
    s32 audio_buffered_count;
    volatile u32 frame_number;
    u32 continuation_type;

    u16 chunk_sector_idx;
    u16 sectors_remaining;
    volatile u32 last_video_frame;
    u32 last_consumed_video_frame;

    volatile u32 last_audio_frame;
    u32 last_consumed_audio_frame;

    u8 gpu_mode;
    u8 use_cd_audio;
    u8 audio_stream_state;
    u8 input_buf_idx;
    u8 vlc_retry_count;
    u8 mdec_retry_pending;
    u8 video_service_busy;
    volatile u8 draw_sync_target;
    volatile u8 chunk_idx;
    volatile u8 out_buf_idx;
    volatile u8 pending_vram_upload;
    volatile u8 pending_mdec_decode;
    volatile s8 mdec_busy;
    volatile u8 frame_ready;
    volatile u8 end_of_stream;
    volatile u8 end_state;
} MovieState;

#define MOVIE_STATE ((MovieState*)MOVIE_STATE_ADDRESS)

extern u8 g_gpu_mode;
extern u8 g_movie_use_cd_audio;

/** @brief Foreground video servicing excludes the DrawSync callback. */
extern u8 g_movie_video_service_busy;
/** @brief A decoded frame is waiting for the MDEC input buffer. */
extern u8 g_movie_mdec_retry_pending;
/** @brief Audio ring priming state shared with the sector callback. */
extern u8 g_movie_audio_stream_state;

#endif
