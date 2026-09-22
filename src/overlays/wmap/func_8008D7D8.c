#include "wmap_sequence_runtime.h"
#include "common.h"

extern s32 func_8006683C(s32);
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern s32 D_801B2A10;
extern s32 D_801B2A14;
extern void func_8008DBFC(void);
extern void func_8008E718(void);

/** @brief Register two callbacks around a color and state update and begin a two-tick delay. */
void func_8008D7D8(void)
{
    func_8006CAC0(&func_8008E718);
    func_8006683C(0x501040);
    D_801ADAF4 = 4;
    D_801ADAE0 = 1;
    func_8006CAC0(&func_8008DBFC);
    D_801B2A14 = 2;
    D_801B2A10 += 1;
}
