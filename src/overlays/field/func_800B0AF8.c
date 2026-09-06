#include "common.h"

extern u8 *D_80122B74;
extern u8 *D_80122B78;

void func_800B0BDC(void);
s32 *func_800C1EC8(s32 *src, s32 *dest, s32 n);
void func_800B0C10(void);
void func_800B0C54(void);
void func_800B0D3C(void);
void func_800B0E80(void);
void func_800B0EFC(void);
extern s32 func_800BD414(s32 arg0, s32 arg1);

/**
 * @brief Initializes the field menu/audio subsystem and flags queued song requests.
 */
void func_800B0AF8(void)
{
    func_800B0BDC();
    func_800C1EC8(0, (s32 *)(D_80122B78 + 0x400), 0xB04);
    func_800B0C10();
    func_800B0C54();
    func_800B0D3C();
    func_800B0E80();
    func_800B0EFC();

    if (D_80122B74[0x840] != 0)
    {
        if (func_800BD414(0, 0x2F08) == 0xFF)
        {
            akao_set_song_params(0x8001, 0x320, 1, 0);
        }
    }

    if (D_80122B74[0xA90] != 0)
    {
        if (func_800BD414(0, 0x2F00) == 0xFF)
        {
            akao_set_song_params(0x8001, 0x320, 2, 0);
        }
    }
}
