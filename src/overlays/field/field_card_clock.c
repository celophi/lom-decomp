/** @file field_card_clock.c
 * @brief Read the PocketStation clock once and keep it running on the VSync counter.
 */

#include "common.h"
#include "field_calls.h"
#include "sdk/libetc.h"
#include "sdk/memory.h"

/** @brief Frames per second of the field clock. */
#define FRAMES_PER_SECOND 60

/** @brief Frames per minute of the field clock. */
#define FRAMES_PER_MINUTE (FRAMES_PER_SECOND * 60)

/** @brief Frames per hour of the field clock. */
#define FRAMES_PER_HOUR (FRAMES_PER_MINUTE * 60)

/** @brief Tens digit of a packed BCD byte. */
#define BCD_HIGH(bcd) ((bcd) >> 4)

/** @brief Units digit of a packed BCD byte. */
#define BCD_LOW(bcd) ((bcd) & 0xF)

/** @brief Decimal value of two packed BCD digits. */
#define BCD_TO_INT(bcd) (BCD_HIGH(bcd) * 10 + BCD_LOW(bcd))

/** @brief Decimal value of a four-digit BCD year stored as two bytes. */
#define BCD_YEAR_TO_INT(bcd) (BCD_HIGH((bcd)[0]) * 1000 + BCD_LOW((bcd)[0]) * 100 + BCD_HIGH((bcd)[1]) * 10 + BCD_LOW((bcd)[1]))

/** @brief Memory-card port the clock is read from. */
#define CARD_CLOCK_PORT 0

/** @brief func_80032174() mode that waits for the pending command to finish. */
#define MCX_SYNC_WAIT 0

/** @brief McxCardType() result for a PocketStation. */
#define MCX_CARD_POCKETSTATION 1

/** @brief McxSync() result: the last command succeeded. */
#define MCX_RESULT_SUCCESS 0

/** @brief McxSync() result: a new card was inserted. */
#define MCX_RESULT_NEW_CARD 3

/** @brief McxGetTime() return value when the command was issued. */
#define MCX_COMMAND_ISSUED 1

/** @brief Raw McxGetTime() snapshot, all values packed BCD. */
typedef struct
{
    u8 year[2]; /**< Century, then year of the century. */
    u8 month;
    u8 day;
    u8 weekday; /**< Low nibble only. */
    u8 hour;
    u8 minute;
    u8 second;
} FieldCardClockBcd;

/** @brief Decoded card clock returned by field_get_card_clock(). */
typedef struct FieldCardClock
{
    s16 year;
    u8 month;
    u8 day;
    u8 weekday;
    u8 hour;
    u8 minute;
    u8 second;
} FieldCardClock;

s32 McxCardType(s32 port);
s32 func_80032174(s32 mode, s32* command, s32* result);
s32 func_80032888(s32 port, FieldCardClockBcd* time);

/** @brief Clock snapshot read from the PocketStation. */
extern FieldCardClockBcd g_field_card_clock_snapshot;

/** @brief VSync(-1) counter at the time the snapshot was read. */
extern s32 g_field_card_clock_vsync;

/**
 * @brief Decode the cached PocketStation clock and advance it by the frames elapsed since it was read.
 * @param clock Receives the decoded date and the advanced time of day.
 * @return 1 when a clock snapshot is available, otherwise 0.
 */
s32 field_get_card_clock(FieldCardClock* clock)
{
    s32 frame_offset;
    s32 vsync_count;
    u32 time_frames;

    if (g_field_card_clock_valid != 0)
    {
        clock->year = BCD_YEAR_TO_INT(g_field_card_clock_snapshot.year);
        clock->month = BCD_TO_INT(g_field_card_clock_snapshot.month);
        clock->day = BCD_TO_INT(g_field_card_clock_snapshot.day);
        clock->weekday = BCD_LOW(g_field_card_clock_snapshot.weekday);
        clock->hour = BCD_TO_INT(g_field_card_clock_snapshot.hour);
        clock->minute = BCD_TO_INT(g_field_card_clock_snapshot.minute);
        clock->second = BCD_TO_INT(g_field_card_clock_snapshot.second);

        vsync_count = VSync(-1);
        frame_offset = clock->second * FRAMES_PER_SECOND + clock->minute * FRAMES_PER_MINUTE + clock->hour * FRAMES_PER_HOUR - g_field_card_clock_vsync;
        time_frames = frame_offset + vsync_count;
        clock->hour = time_frames / FRAMES_PER_HOUR;
        clock->minute = (time_frames - clock->hour * FRAMES_PER_HOUR) / FRAMES_PER_MINUTE;
        clock->second = (time_frames - clock->hour * FRAMES_PER_HOUR - clock->minute * FRAMES_PER_MINUTE) / FRAMES_PER_SECOND;
        return 1;
    }

    return 0;
}

/**
 * @brief Read the PocketStation clock into the cache if no snapshot was taken yet.
 * @note Also records the VSync counter so field_get_card_clock() can advance the time later.
 */
void field_capture_card_clock(void)
{
    s32 card_type;
    s32 command;
    s32 result;
    FieldCardClockBcd time;

    if (g_field_card_clock_valid == 0)
    {
        card_type = McxCardType(CARD_CLOCK_PORT);
        if (card_type == MCX_CARD_POCKETSTATION)
        {
            func_80032174(MCX_SYNC_WAIT, &command, &result);
            if (result == MCX_RESULT_SUCCESS || result == MCX_RESULT_NEW_CARD)
            {
                if (func_80032888(CARD_CLOCK_PORT, &time) == MCX_COMMAND_ISSUED)
                {
                    func_80032174(MCX_SYNC_WAIT, &command, &result);
                }
                if (result == MCX_RESULT_SUCCESS)
                {
                    bcopy((u8*)&time, (u8*)&g_field_card_clock_snapshot, sizeof(time));
                    g_field_card_clock_vsync = VSync(-1);
                    g_field_card_clock_valid = card_type;
                }
            }
        }
    }
}
