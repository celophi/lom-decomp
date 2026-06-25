#include "common.h"

void func_800683C8(void);

/**
 * @brief Call func_800683C8 if arg0 is zero.
 * @param arg0 When 0, triggers the call; otherwise returns immediately.
 * @see decomp.me (100%) TODO
 */
void func_800681C0(s32 arg0) {
    if (arg0 == 0) {
        func_800683C8();
    }
}
