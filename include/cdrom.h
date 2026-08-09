#ifndef CDROM_H
#define CDROM_H

#include "common.h"

typedef u8* (*CdCommandCallback)(s32 bytes_transferred, u32 bytes_remaining);
typedef u8* (*CdStreamGetBufferCallback)(s32 chunk_index, s32* capacity);
typedef void (*CdStreamChunkDoneCallback)(s32 chunk_index);

void cdrom_init(void);
void cdrom_stop(void);
s32 cdrom_stream(s32 resource_index, u32 destination);
void cdrom_stream_chunked(u16 resource_index, CdStreamGetBufferCallback get_buffer,
                          CdStreamChunkDoneCallback chunk_done);
s32 cdrom_queue_command(u8 command, u16 resource_index, void* dst_buffer,
                        CdCommandCallback callback);
u32 cdrom_process_state(void);
void cdrom_verify_recovery(void);
void cdrom_wait_queue_empty(void);
void cdrom_set_audio_volume(u8 volume, s32 stereo_channel);
void cdrom_load_resource_table(s32 lba, s32 data_size_bytes);
void cdrom_reset(void);
void cdrom_queue_read(s32 resource_index, void* dst_buffer);
s32 cdrom_can_queue_resource(s32 resource_index);
void cdrom_queue_read_with_callback(s32 resource_index, CdCommandCallback callback);
void cdrom_queue_seek(s32 resource_index);
s32 cdrom_get_resource_size(s32 resource_index);
s32 cdrom_get_error_status(void);

#endif
