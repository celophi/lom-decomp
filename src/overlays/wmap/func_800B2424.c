#include "wmap_view_effects.h"
#include "wmap_sequence_runtime.h"
#include "common.h"

extern s32 D_801ADAF4;
extern s32 D_801B2F98;
extern s32 D_801B2F9C;
extern void func_800B27F8(void);
extern void func_800B2EC8(void);

/** @brief Set world-map color and state, register two callbacks, and begin a four-tick delay. */
void func_800B2424(void)
{
    func_8006683C(0x903065);
    D_801ADAF4 = 0xD;
    func_8006CAC0(&func_800B2EC8);
    func_8006CAC0(&func_800B27F8);
    D_801B2F9C = 4;
    D_801B2F98 += 1;
}
