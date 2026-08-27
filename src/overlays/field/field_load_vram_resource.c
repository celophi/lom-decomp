#include "common.h"

extern u8 *D_8010D038;
void cdrom_queue_read(s32 resource_index, void *dst_buffer);
void cdrom_wait_queue_empty(void);
s32 func_80086374(s16 *rect, u8 *data, s32 mode);

/**
 * @brief Loads a VRAM resource from disc and uploads it via func_80086374.
 *
 * Queues a CD read of resource @p id (masked to 16 bits) into the shared field
 * CD buffer @c D_8010D038, waits for the queue to drain, then hands the loaded
 * blob to func_80086374 to upload into VRAM using @p rect and @p arg2.
 *
 * @param id Resource index; masked to 16 bits for the CD queue.
 * @param rect Destination rectangle forwarded to func_80086374.
 * @param arg2 Upload mode forwarded to func_80086374.
 */
void field_load_vram_resource(s32 id, s16 *rect, s32 arg2)
{
    u8 *buf = D_8010D038;

    cdrom_queue_read(id & 0xFFFF, buf);
    cdrom_wait_queue_empty();
    func_80086374(rect, buf, arg2);
}
