/** @file field_card_clock.c
 * @brief Read and advance the memory-card clock snapshot.
 */

#include "common.h"

/** @brief Frames per second of the field clock. */
#define FRAMES_PER_SECOND 60

/** @brief Frames per minute of the field clock. */
#define FRAMES_PER_MINUTE (FRAMES_PER_SECOND * 60)

/** @brief Frames per hour of the field clock. */
#define FRAMES_PER_HOUR (FRAMES_PER_MINUTE * 60)

/** @brief Decimal value of two packed BCD digits. */
#define BCD_TO_INT(bcd) (((bcd) >> 4) * 10 + ((bcd) & 0xF))

extern s32 D_801227E8;
extern u8 D_80122728[8];
extern s32 D_80122824;

s32 McxCardType(s32);
s32 func_80032174(s32, void*, s32*);
s32 func_80032888(s32, void*);
s32 bcopy(void*, void*, s32);
s32 VSync(s32);

/**
 * @brief Decode the cached card clock and advance its time by elapsed frames.
 *
 * The cached snapshot holds BCD year (two bytes), month, day, a weekday
 * nibble, hour, minute and second. The time of day is advanced by the
 * frames counted since the snapshot was taken.
 *
 * @param clock Eight-byte output: a halfword year, then month, day, weekday, hour, minute and second bytes.
 * @return 1 when a clock snapshot is available, otherwise 0.
 */
s32 func_800AFE14(u8* clock)
{
    s32 frame_offset;
    s32 current_vsync;
    u32 current_time_frames;

    if (D_801227E8 != 0)
    {
        *(s16*)clock = (s16)((D_80122728[0] >> 4) * 1000 + (D_80122728[0] & 0xF) * 100 + (D_80122728[1] >> 4) * 10 + (D_80122728[1] & 0xF));
        clock[2] = (s8)BCD_TO_INT(D_80122728[2]);
        clock[3] = (s8)BCD_TO_INT(D_80122728[3]);
        clock[4] = (s8)(D_80122728[4] & 0xF);
        clock[5] = (u8)BCD_TO_INT(D_80122728[5]);
        clock[6] = (u8)BCD_TO_INT(D_80122728[6]);
        clock[7] = (u8)BCD_TO_INT(D_80122728[7]);

        current_vsync = VSync(-1);
        frame_offset = ((clock[7] * FRAMES_PER_SECOND) + (clock[6] * FRAMES_PER_MINUTE) + (clock[5] * FRAMES_PER_HOUR)) - D_80122824;
        current_time_frames = frame_offset + current_vsync;
        clock[5] = (u8)(current_time_frames / FRAMES_PER_HOUR);
        clock[6] = (u8)((current_time_frames - (clock[5] * FRAMES_PER_HOUR)) / FRAMES_PER_MINUTE);
        clock[7] = (u8)(((current_time_frames - (clock[5] * FRAMES_PER_HOUR)) - (clock[6] * FRAMES_PER_MINUTE)) / FRAMES_PER_SECOND);
        return 1;
    }

    return 0;
}

/**
 * @brief Take a clock snapshot from memory card 0 if none is cached yet.
 *
 * Reads the card clock only when card 0 is present and idle, then records the
 * vertical-sync counter so func_800AFE14 can advance the time later.
 *
 * @see decomp.me (100%)
 */
void func_800B0094(void)
{
    s32 card_type;
    s32 status0;
    s32 status1;
    u8 buf[8];

    if (D_801227E8 == 0)
    {
        card_type = McxCardType(0);
        if (card_type == 1)
        {
            func_80032174(0, &status0, &status1);
            if (status1 == 0 || status1 == 3)
            {
                if (func_80032888(0, buf) == card_type)
                {
                    func_80032174(0, &status0, &status1);
                }
                if (status1 == 0)
                {
                    bcopy(buf, D_80122728, 8);
                    D_80122824 = VSync(-1);
                    D_801227E8 = card_type;
                }
            }
        }
    }
}
