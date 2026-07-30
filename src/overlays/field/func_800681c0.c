#include "common.h"

void field_open_return_to_title_prompt(void);

/**
 * @brief Open the return-to-title confirmation prompt if arg0 is zero.
 * @param arg0 When 0, opens the prompt; otherwise returns immediately.
 * @see decomp.me (100%) TODO
 */
void func_800681C0(s32 arg0) {
    if (arg0 == 0) {
        field_open_return_to_title_prompt();
    }
}
