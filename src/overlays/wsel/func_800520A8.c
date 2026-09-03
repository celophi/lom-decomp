#include "common.h"

extern s32 D_800CA8D8;
extern u8 D_80052608[];
extern u8 D_8007CC2C[];
extern u8 D_8008E650[];
extern u8 D_8009023C[];
extern u8 D_80098E80[];
extern u8 D_800A90A4[];
extern u8 D_800B92C8[];
extern u8 D_800C130C[];

extern void func_80052154(void);
extern void func_80052510(void);
extern void func_800521D0(void*, s32);

/**
 * @brief WSEL resource-table init: reset state, run the sub-inits, and register
 *        the eight resource blobs by slot index.
 * @see (100%)
 */
void func_800520A8(void)
{
    D_800CA8D8 = 0;
    func_80052154();
    func_80052510();
    func_800521D0(D_80052608, 0);
    func_800521D0(D_8007CC2C, 1);
    func_800521D0(D_8008E650, 2);
    func_800521D0(D_8009023C, 3);
    func_800521D0(D_80098E80, 4);
    func_800521D0(D_800A90A4, 5);
    func_800521D0(D_800B92C8, 6);
    func_800521D0(D_800C130C, 7);
}
