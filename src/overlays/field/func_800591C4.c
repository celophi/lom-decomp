#include "common.h"

void func_800584DC(s32, void *, s32);

/**
 * @brief Walk a singly-linked list rooted at arg1+0x8 and call func_800584DC
 *        on each node, passing arg0 and arg2 as the first and third arguments.
 * @param arg0 Context pointer forwarded to func_800584DC as the first argument.
 * @param arg1 Pointer to a struct whose field at offset 0x8 is the list head.
 * @param arg2 Value forwarded to func_800584DC as the third argument.
 * @see decomp.me (100%) TODO
 */
void func_800591C4(s32 arg0, void *arg1, s32 arg2) {
    void **var_s0;

    var_s0 = *(void ***)((u8 *)arg1 + 0x8);
    if (var_s0 != NULL) {
        do {
            func_800584DC(arg0, var_s0, arg2);
            var_s0 = *var_s0;
        } while (var_s0 != NULL);
    }
}
