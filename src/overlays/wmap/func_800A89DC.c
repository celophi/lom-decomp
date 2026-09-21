#include "common.h"
#include "cdrom.h"

extern void (*D_800D6D5C[])(void);
extern s32 D_8013B294;
extern s32 D_80182E3C;

/**
 * @brief Wait for pending reads and dispatch a resource set when its index changes.
 * @param index Requested resource-set index; invalid indices dispatch set zero.
 */
void func_800A89DC(s32 index)
{
    cdrom_wait_queue_empty();
    if (index == D_80182E3C)
    {
        return;
    }
    D_80182E3C = index;
    if (index >= 36 || index < 0)
    {
        index = 0;
        D_8013B294 = 1;
    }
    D_800D6D5C[index]();
}
