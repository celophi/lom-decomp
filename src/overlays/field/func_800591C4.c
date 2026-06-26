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

/**
 * @brief Walk a run-length-encoded count table to locate the record covering a
 *        given linear index, returning that record and the cumulative count
 *        consumed before it.
 *
 * The first byte of @p data holds a 7-bit count (high bit ignored). If @p index
 * is below that count the table does not reach @p index, so @p out is left 0 and
 * @p data is returned unchanged. Otherwise the leading count is committed to
 * @p out, the 0x18-byte header is skipped, and the function steps through the
 * following 4-byte records, accumulating each record's 7-bit count, until the
 * running total would exceed @p index. The pointer to that record is returned
 * and @p out holds the cumulative count of all preceding records.
 *
 * @param data  Pointer to the count table (RLE header followed by 4-byte records).
 * @param index Linear index to resolve; signed compare against each running total.
 * @param out   Receives the cumulative count consumed before the returned record.
 * @return Pointer to the record whose range contains @p index.
 *
 * @note @p out is taken as volatile and an otherwise-dead read of byte 7 of the
 *       header is preserved; both are required to reproduce the original codegen
 *       (the reload of *out and the stray load). The do/while(0) wrapper and the
 *       $v1 register pin on the dead load are likewise required to match GCC's
 *       basic-block ordering and register allocation - removing any of them
 *       drops the match.
 * @see decomp.me (100%) TODO
 */
u8 *func_80059224(u8 *data, s32 index, volatile s8 *out) {
    u8 count;
    u8 byte;
    u8 acc;
    u8 cand;
    register u8 unused asm("$3");

    *out = 0;
    count = *data & 0x7F;
    do
    {
        if (index >= count)
        {
            *out = count;
            unused = *(volatile u8 *)(data + 7);
            data += 0x18;
            byte = *data;
            acc = *out;
            cand = acc + (byte & 0x7F);
            while (index >= (u8)cand)
            {
                data += 4;
                cand = acc + (byte & 0x7F);
                *out = cand;
                byte = *data;
                acc = cand;
                cand = cand + (byte & 0x7F);
            }
        }
        return data;
    } while (0);
}
