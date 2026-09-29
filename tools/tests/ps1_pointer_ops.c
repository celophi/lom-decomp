/** @file ps1_pointer_ops.c
 * @brief Exercise pointer storage and original game code against mapped RAM.
 */
#include "akao_driver.h"
#include "field_animation.h"
#include "field_script.h"

#define CHECK(condition)                                                                                                                                       \
    do                                                                                                                                                         \
    {                                                                                                                                                          \
        if (!(condition))                                                                                                                                      \
        {                                                                                                                                                      \
            return __LINE__;                                                                                                                                   \
        }                                                                                                                                                      \
    } while (0)

FieldScriptContextPtr g_field_script;

/** @brief Step past the opcode when the real branch handler reads a zero delta. */
void field_script_op_00(void)
{
    g_field_script->pc++;
}

void akao_seq_op_set_pitch_jitter_depth(AkaoChannelState* channel);

/** @brief Test handler reached through a stored code address. */
typedef s32 (*TestCalcOp)(s32 left, s32 right);
/** @brief Stored address of a TestCalcOp. */
typedef PS1_CODE(TestCalcOp) TestCalcOpSlot;

/** @brief PS1 address the test resolver maps to test_calc_add. */
#define TEST_CALC_ADD_ADDRESS 0x800BFD18U

static u32 g_test_resolved_address;

/**
 * @brief Add two values; the target of the test slot.
 * @param left First value.
 * @param right Second value.
 * @return The sum.
 */
static s32 test_calc_add(s32 left, s32 right)
{
    return left + right;
}

/**
 * @brief Test resolver: map the one known address, and record what was asked.
 * @param address PS1 code address read from a slot.
 * @return The host function, or null for an unknown address.
 */
Ps1CodeFunc ps1_resolve_code(u32 address)
{
    g_test_resolved_address = address;
    return address == TEST_CALC_ADD_ADDRESS ? (Ps1CodeFunc)test_calc_add : 0;
}

/**
 * @brief Test implicit widening, storage, arithmetic, tables and game consumers.
 * @param storage Writable mapped RAM whose address has bit 31 set.
 * @return Zero on success, otherwise the failed check's line number.
 */
int ps1_test_pointer_ops(void* storage)
{
    u8* bytes = storage;
    u8_ptr cursor;
    u8_ptr table[3];
    u8_ptr* slot;
    u16_ptr halfwords;
    FieldImageReq* images = (FieldImageReq*)(bytes + 0x100);
    FieldScriptRecordState* frame = (FieldScriptRecordState*)(bytes + 0x200);
    AkaoChannelState* channel = (AkaoChannelState*)(bytes + 0x300);
    AkaoCommandParam command;
    TestCalcOpSlot code_table[2] = {{0x80010000U}, {TEST_CALC_ADD_ADDRESS}};
    __UINTPTR_TYPE__ address = (__UINTPTR_TYPE__)bytes;

    CHECK(address >= 0x80000000U && address < 0xFFFFF000U);
    CHECK(sizeof(cursor) == 4 && sizeof(table) == 12);
    cursor = bytes;
    CHECK((__UINTPTR_TYPE__)cursor == address);
    bytes[0] = 17;
    bytes[1] = 29;
    CHECK(*cursor++ == 17);
    CHECK(cursor == bytes + 1 && cursor[0] == 29);
    cursor += 3;
    CHECK(cursor - bytes == 4);
    halfwords = (u16*)bytes;
    halfwords++;
    CHECK((u8*)halfwords == bytes + 2);

    table[0] = bytes;
    table[1] = bytes + 1;
    table[2] = 0;
    slot = table;
    CHECK(*slot++ == bytes);
    CHECK(**slot == 29);
    CHECK((u8*)slot - (u8*)table == 4);
    CHECK(table[2] == 0);

    images[0].next = &images[1];
    images[0].rect.x = 123;
    images[1].rect.x = 456;
    images[0].data = (u_long*)bytes;
    CHECK(images[0].next->rect.x == 456);
    CHECK(images[0].rect.x == 123);
    CHECK((u8*)images[0].data == bytes);
    command.buffer = bytes;
    CHECK((u32)command.value == (u32)address);
    cursor = (u8_ptr)-1;
    CHECK((__UINTPTR_TYPE__)cursor == 0xFFFFFFFFU);

    g_field_script = (FieldScriptContext*)frame;
    g_field_script->active_record = 0;
    frame->flags = 0xA5A55A5A;
    frame->wait.word = 0x12345678;
    frame->pc = bytes;
    bytes[1] = 6;
    bytes[2] = 0;
    field_script_branch(1);
    CHECK(frame->pc == bytes + 6);
    bytes[7] = 0xFC;
    bytes[8] = 0xFF;
    field_script_branch(1);
    CHECK(frame->pc == bytes + 2);
    bytes[3] = 0;
    bytes[4] = 0;
    field_script_branch(1);
    CHECK(frame->pc == bytes + 3);
    CHECK(frame->flags == 0xA5A55A5A && frame->wait.word == 0x12345678);

    CHECK(sizeof(code_table) == 8);
    CHECK(PS1_CALL(code_table[1])(20, 22) == 42);
    CHECK(g_test_resolved_address == TEST_CALC_ADD_ADDRESS);

    channel->seq_cursor = bytes;
    channel->loop_cursor[0] = bytes + 8;
    channel->return_cursor = bytes + 16;
    akao_seq_op_set_pitch_jitter_depth(channel);
    CHECK(channel->pitch_scale == 17);
    CHECK(channel->seq_cursor == bytes + 1);
    CHECK(channel->loop_cursor[0] == bytes + 8 && channel->return_cursor == bytes + 16);
    return 0;
}
