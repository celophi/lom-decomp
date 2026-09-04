#include "common.h"
extern s32 D_80157D50;
extern s32 D_80157D54;
extern s32 D_80157D58;
extern s32 D_80157D60;
extern s32 D_80157D64;
void func_80141C08(u8 *arg0)
{
    s32 saved;
    volatile s32 pad[2];
    if (D_80157D64 != 0) {
        saved = *(s32 *)(arg0 + 0x4040);
        switch (D_80157D64) {
        case 1: {
            s32 *counter = &D_80157D54;
            if (++*counter == 8) {
                func_80142B3C();
                D_80157D64 = 2;
                *counter = 0;
                D_80157D50 = D_80157D60;
                func_801424D0(0x100, 0x100, 0x100, 6);
            }
            break;
        }
        case 2:
            if (++D_80157D54 == 8) D_80157D64 = 0;
            break;
        case 3: {
            s32 *counter = &D_80157D54;
            if (++*counter == 8) {
                func_80142B3C();
                D_80157D64 = 4;
                *counter = 0;
                D_80157D50 = D_80157D60;
                func_801424D0(0x100, 0x100, 0x100, 6);
            }
            break;
        }
        case 4:
            if (++D_80157D54 == 8) {
                D_80157D64 = 0;
                D_80157D54 = 0;
            }
            break;
        case 5:
            if (++D_80157D54 == 8) {
                D_80157D64 = 2;
                D_80157D54 = 0;
                D_80157D58 = 1;
                func_801424D0(0x100, 0x100, 0x100, 6);
            }
            break;
        case 6: {
            s32 *counter = &D_80157D54;
            if (++*counter == 8) {
                func_80142B3C();
                D_80157D64 = 2;
                D_80157D58 = 0;
                *counter = 0;
                D_80157D50 = D_80157D60;
                func_801424D0(0x100, 0x100, 0x100, 6);
            }
            break;
        }
        }
        *(s32 *)(arg0 + 0x4040) = saved;
    }
}
