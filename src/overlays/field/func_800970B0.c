#include "common.h"

extern u8 D_8010AED0[];
extern u8 *D_8010D038;
extern void cdrom_queue_read(s32 resource_index, void *dst_buffer);
extern void cdrom_wait_queue_empty(void);

/**
 * @brief Loads resource 0x5DD and copies its packed 11x24x32-byte payload.
 *
 * The resource data begins one byte into D_8010D038. Rows are copied into
 * D_8010AED0 using a 0x300-byte outer stride and a 0x20-byte row stride.
 *
 * @note gcc272_cdk, 100% match.
 */
void func_800970B0(void)
{
    s32 i, j, k;
    u8 *src, *row, *dst;
    u8 value;

    cdrom_queue_read(0x5DD, D_8010D038);
    cdrom_wait_queue_empty();
    src = D_8010D038 + 1;
    for (i = 0; i < 11; i++) {
        for (j = 0; j < 24; j++) {
            k = 0;
            row = (i * 0x300 + j * 0x20) + D_8010AED0;
            do {
                dst = row + k;
                k++;
                value = *src;
                *dst = value;
                src++;
            } while (k < 32);
        }
    }
}
