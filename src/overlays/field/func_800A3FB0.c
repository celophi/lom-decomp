#include "common.h"
#include "akao.h"
#include "sdk/memory.h"

s32 akao_cmd_d0(s32);
s32 akao_cmd_d4(s32);
s32 akao_get_xfer_state(void);
s32 akao_reset_xfer_state(void);
s32 akao_streaming_upload_tick(s32, u32, s32);
void func_800A4320(u8 *, u8 *, s32); /* extern */
extern u8 D_8003ECA0[];
extern s32 D_8011588C;
extern s32 D_80117EE0;
extern s32 D_80117EE4;
extern s32 D_80117EE8;
extern u8 *D_80117EEC;
extern s32 D_80119EF8;
extern s32 D_8011F300;
extern u32 D_8011F308;
extern s32 D_8011F310;
extern s32 D_8011F320;
extern u32 D_8011F324;

/**
 * @brief Advance the field music block copy, upload and playback state machine.
 * @note The resident song is assembled at D_8003ECA0; bank data uses 0x801DD000.
 * @note Volatile header reads preserve the original repeated offset loads.
 * @note GCC 2.7.2 CDK matches all 220 instructions (880 bytes).
 */
void func_800A3FB0(void)
{
    s32 resident_bytes;
    s32 upload_bytes;
    s32 remaining_bytes;
    s32 remaining_bytes_second;
    s32 data_offset;
    s32 song_handle;
    s32 resident_remaining;

    if (D_80117EE0 != 0)
    {
        switch (D_8011F308)
        {
        case 1:
            if (D_80119EF8 != 0)
            {
                if (D_80119EF8 < 0x800)
                {
                    bcopy(D_80117EEC, D_80117EE8 + D_8003ECA0, D_80119EF8);
                    bcopy(D_80117EEC + D_80119EF8, (void *)0x801DD000, 0x800 - D_80119EF8);
                    resident_bytes = D_80119EF8;
                    D_8011F300 = 1;
                    D_80119EF8 = 0;
                    D_8011F320 = 0x800 - resident_bytes;
                    D_80117EE8 += resident_bytes;
                }
                else
                {
                    bcopy(D_80117EEC, D_80117EE8 + D_8003ECA0, 0x800);
                    resident_remaining = D_80119EF8 - 0x800;
                    D_80117EE8 += 0x800;
                    D_80119EF8 = resident_remaining;
                    if (resident_remaining == 0)
                    {
                        D_8011F320 = 0;
                        D_8011F300 = 1;
                    }
                }
                goto release_block;
            }
            if (D_8011F300 != 0)
            {
                bcopy(D_80117EEC, (void *)(D_8011F320 + 0x801DD000), 0x800);
                D_8011F320 += 0x800;
                akao_reset_xfer_state();
                akao_streaming_upload_tick(0x801DD000, 0x800, 1);
                remaining_bytes = D_8011F320 - 0x800;
                D_8011F320 = remaining_bytes;
                func_800A4320((void *)0x801DD000, (void *)0x801DD800, remaining_bytes);
                D_8011F308 = 3;
            }
            else
            {
                bcopy(D_80117EEC, (void *)(D_8011F320 + 0x801DD000), 0x800);
                if ((u32)D_8011F324 < 0x800U)
                {
                    upload_bytes = D_8011F320 + D_8011F324;
                    D_8011F320 = upload_bytes;
                    akao_streaming_upload_tick(0x801DD000, upload_bytes, 1);
                    D_8011F308 = 4;
                    D_8011F320 = 0;
                }
                else
                {
                    D_8011F320 += 0x800;
                    akao_streaming_upload_tick(0x801DD000, 0x800, 1);
                    D_8011F308 = 3;
                    remaining_bytes_second = D_8011F320 - 0x800;
                    D_8011F320 = remaining_bytes_second;
                    func_800A4320((void *)0x801DD000, (void *)0x801DD800, remaining_bytes_second);
                }
            }
            D_8011F300 = 0;
            return;
        case 2:
            data_offset = *(volatile s32 *)(D_80117EEC + 4);
            D_80119EF8 = *(s32 *)(D_80117EEC + 8) - *(volatile s32 *)(D_80117EEC + 4);
            bcopy(D_80117EEC + data_offset, D_8003ECA0, 0x800 - data_offset);
            D_80117EE0 = 0;
            D_80117EE8 = 0x800 - *(volatile s32 *)(D_80117EEC + 4);
            D_80119EF8 -= 0x800 - *(volatile s32 *)(D_80117EEC + 4);
            return;
        case 3:
            if (akao_get_xfer_state() == 0)
            {
            release_block:
                D_80117EE0 = 0;
            }
            break;
        case 4:
            if (akao_get_xfer_state() == 0)
            {
                D_80117EE0 = 0;
                song_handle = akao_play_song((AkaoHeader *)D_8003ECA0);
                D_8011F310 = song_handle;
                akao_set_song_volume(song_handle, D_8011588C);
                akao_cmd_d4(0);
                akao_cmd_d0(0);
                D_80117EE4 = 0;
            }
            break;
        case 0:
        default:
            break;
        }
    }
}
