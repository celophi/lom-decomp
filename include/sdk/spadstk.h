#ifndef SDK_SPADSTK_H
#define SDK_SPADSTK_H

/**
 * @brief Save the current stack pointer and enter a scratchpad stack.
 * @param save_slot Word-aligned address above the stack for the saved pointer.
 * @note Pair with ResetSpadStack in the same function. Do not access
 *       the caller's stack locals while the scratch stack is active.
 */
#define SetSpadStack(save_slot) \
    __asm__ volatile ( \
        "addu $8,%0,$0\n\t" \
        "sw $29,0($8)\n\t" \
        "addiu $8,$8,-4\n\t" \
        "addu $29,$8,$0" \
        : : "r" (save_slot) : "$8", "memory")

/** @brief Leave the scratch stack and restore the saved caller stack pointer. */
#define ResetSpadStack() \
    __asm__ volatile ( \
        "addiu $29,$29,4\n\t" \
        "lw $29,0($29)" \
        : : : "$29", "memory")

#endif
