#include "checkps.h"

/*
 * GNU as 2.7 pads the standard .text section to a 16-byte boundary.  Keeping
 * this translation unit's code in a custom section avoids synthetic tail
 * bytes; the build renames the section back to .text with objcopy.
 */
#define CHECKPS_GNU_TEXT __attribute__((section(".text.code7")))
#define CHECKPS_CD_IRQ_DISK_ERROR 5
#define CHECKPS_CD_STATUS_SHELL_OPEN 0x10

/*
 * Keep the section attribute on declarations so Splat can discover each
 * function definition normally while GNU as emits this unit into .text.code7.
 */
void StartCdIntegrityCheck(void) CHECKPS_GNU_TEXT;
s32 RunCdIntegrityCheck(s32 singleStep) CHECKPS_GNU_TEXT;
CheckPSCdPollResult PollCdResponse(CheckPSCdCommandIndex command) CHECKPS_GNU_TEXT;
void SendCdCommand(CheckPSCdCommandIndex command) CHECKPS_GNU_TEXT;
void ShowHardwareModificationWarningAndExit(void) CHECKPS_GNU_TEXT;

/** Initialize the CHECKPS CD integrity state machine. */
void StartCdIntegrityCheck(void)
{
    g_checkpsState = CHECKPS_STATE_START_GET_TN;
}

/**
 * Drive the CHECKPS CD integrity command state machine.
 *
 * A nonzero singleStep executes one transition. Zero keeps processing until
 * the state machine becomes idle. The explicit gotos and label-address array
 * are compiler-shaping constructs required by GCC 2.7.2; rewriting them as
 * structured loops changes register allocation and code generation.
 */
s32 RunCdIntegrityCheck(s32 singleStep)
{
    /* GCC shares this temporary between restart-state stores and VSync samples. */
    s32 restartStateOrVsync;
    s32 stepResult = CHECKPS_STATE_IDLE;
    static void* compilerLabelAnchors[] = {&&init_poll_result,  &&get_td_poll_result,  &&setloc_poll_result,  &&setmode_poll_result,  &&seek_p_poll_result,    &&mute_poll_result,   &&play_poll_result,  &&test_04_poll_result,
                                  &&test_05_poll_result, &&failure_nop_poll_result, &&recovery_nop_poll_result, &&pause_poll_result, &&get_id_apply_seek_position, &&test_05_check_response, &&pause_command_error, &&pause_done};

    for (;;)
    {
        switch (g_checkpsState)
        {
        case CHECKPS_STATE_START_GET_TN: /* Start the first CD command in the sequence. */
            SendCdCommand(CHECKPS_CD_CMD_GET_TN);
            g_checkpsState = CHECKPS_STATE_WAIT_GET_TN;
            stepResult = CHECKPS_STATE_START_GET_TN;
            break;

        case CHECKPS_STATE_WAIT_GET_TN: /* Save the last-track byte returned by GetTN. */
            stepResult = PollCdResponse(CHECKPS_CD_CMD_GET_TN);
            switch (stepResult)
            {
            case CHECKPS_CD_POLL_SHELL_OPEN:
                SendCdCommand(CHECKPS_CD_CMD_NOP);
                g_checkpsState = CHECKPS_STATE_WAIT_RECOVERY_NOP;
                stepResult = CHECKPS_STATE_WAIT_GET_TN;
                break;

            case CHECKPS_CD_POLL_PENDING:
                stepResult = CHECKPS_STATE_WAIT_GET_TN;
                break;

            case CHECKPS_CD_POLL_COMPLETE:
                g_cdLastTrackBcd = g_cdResponseByte2;
                stepResult = CHECKPS_STATE_WAIT_GET_TN;
                SendCdCommand(CHECKPS_CD_CMD_INIT);
                g_checkpsState = CHECKPS_STATE_WAIT_INIT;
                break;

            case CHECKPS_CD_POLL_DISK_ERROR:
                SendCdCommand(CHECKPS_CD_CMD_GET_TN);
                stepResult = CHECKPS_STATE_WAIT_GET_TN;
                break;

            default:
                stepResult = CHECKPS_STATE_WAIT_GET_TN;
                break;
            }

            break;

        case CHECKPS_STATE_WAIT_INIT:
            stepResult = PollCdResponse(CHECKPS_CD_CMD_INIT);
            if (stepResult == CHECKPS_CD_POLL_DISK_ERROR)
            {
                restartStateOrVsync = 1;
                g_checkpsState = restartStateOrVsync;
                stepResult = CHECKPS_CD_POLL_DISK_ERROR;
            }
            else
            {
            init_poll_result:
                if (stepResult < 0)
                {
                    if (stepResult == CHECKPS_CD_POLL_SHELL_OPEN)
                    {
                        SendCdCommand(CHECKPS_CD_CMD_NOP);
                        g_checkpsState = CHECKPS_STATE_WAIT_RECOVERY_NOP;
                        stepResult = CHECKPS_CD_POLL_DISK_ERROR;
                    }
                    else
                    {
                        stepResult = CHECKPS_STATE_WAIT_INIT;
                    }
                }
                else
                {
                    if (stepResult != CHECKPS_CD_POLL_PENDING)
                    {
                        if (stepResult == CHECKPS_CD_POLL_COMPLETE)
                        {
                            g_cdCommandParameters[0] = (g_cdLastTrackBcd >= 2) ? 2 : 0;
                            SendCdCommand(CHECKPS_CD_CMD_GET_TD);
                            g_checkpsState = CHECKPS_STATE_WAIT_GET_TD;
                        }
                    }
                    stepResult = CHECKPS_STATE_WAIT_INIT;
                }
            }
            break;

        case CHECKPS_STATE_WAIT_GET_TD:
            stepResult = PollCdResponse(CHECKPS_CD_CMD_GET_TD);
            if (stepResult == CHECKPS_CD_POLL_DISK_ERROR)
            {
                restartStateOrVsync = 1;
                g_checkpsState = restartStateOrVsync;
                stepResult = CHECKPS_CD_POLL_DISK_ERROR;
            }
            else
            {
            get_td_poll_result:
                if (stepResult < 0)
                {
                    if (stepResult == CHECKPS_CD_POLL_SHELL_OPEN)
                    {
                        SendCdCommand(CHECKPS_CD_CMD_NOP);
                        g_checkpsState = CHECKPS_STATE_WAIT_RECOVERY_NOP;
                        stepResult = CHECKPS_CD_POLL_DISK_ERROR;
                    }
                    else
                    {
                        stepResult = CHECKPS_STATE_WAIT_SETLOC;
                    }
                }
                else
                {
                    if (stepResult != CHECKPS_CD_POLL_PENDING)
                    {
                        if (stepResult == CHECKPS_CD_POLL_COMPLETE)
                        {
                            {

                                u8* tocTimeBcd;
                                u8 tocMinutesBcd;
                                u8 tocSecondsBcd;
                                u32 midpointMinutesValue;
                                u32 midpointSecondsValue;
                                u32 encodedMinutes;
                                s32 tocMinutes;
                                s32 tocSeconds;
                                s32 midpointTotalSeconds;
                                s32 midpointMinutes;
                                s32 midpointSeconds;
                                int midpointMinutesStoreIndex;
                                int midpointMinutesReadIndex;
                                int encodedMinutesStoreIndex;
                                int encodedMinutesReadIndex;
                                int midpointSecondsStoreIndex;
                                int midpointSecondsReadIndex;
                                u32 addressMixer;
                                u32 midpointSecondTens;
                                u32 encodedMinuteTens;

                                /* These cancelling operations preserve the original pointer
                                   expression emitted by GCC 2.7.2. */
                                addressMixer = ((u32)0x88888889U + (u32)singleStep) - (u32)singleStep;
                                tocTimeBcd = (u8*)((u32)g_cdResponsePayload + (addressMixer ^ (u32)0x88888889U));
                                tocMinutesBcd = tocTimeBcd[0];
                                tocSecondsBcd = tocTimeBcd[1];

                                tocMinutes = ((tocMinutesBcd >> 4) * 10) + (tocMinutesBcd & 0xF);
                                tocSeconds = (((tocSecondsBcd >> 4) * 5) * 2) + (tocSecondsBcd & 0xF);
                                midpointTotalSeconds = ((tocMinutes * 60) + tocSeconds) >> 1;
                                midpointMinutes = midpointTotalSeconds / 60;
                                midpointSeconds = midpointTotalSeconds % 60;

                                /* Volatile stores prevent GCC from forwarding these writes;
                                   the following plain reads are also required for the match. */
                                midpointMinutesStoreIndex = 0;
                                *(volatile u8*)&g_cdSeekPositionBcd[midpointMinutesStoreIndex] = midpointMinutes;
                                midpointMinutesReadIndex = 0;
                                midpointMinutesValue = g_cdSeekPositionBcd[midpointMinutesReadIndex];
                                midpointSecondsStoreIndex = 1;
                                *(volatile u8*)&g_cdSeekPositionBcd[midpointSecondsStoreIndex] = midpointSeconds;
                                midpointSecondsReadIndex = 1;
                                midpointSecondsValue = g_cdSeekPositionBcd[midpointSecondsReadIndex];
                                encodedMinutesStoreIndex = 0;
                                *(volatile u8*)&g_cdSeekPositionBcd[encodedMinutesStoreIndex] =
                                    ((midpointMinutesValue / 10) << 4) | (midpointMinutesValue % 10);
                                encodedMinutesReadIndex = 0;
                                encodedMinutes = g_cdSeekPositionBcd[encodedMinutesReadIndex];
                                midpointSecondTens = midpointSecondsValue / 10;
                                encodedMinuteTens = encodedMinutes / 10;
                                g_cdSeekPositionBcd[1] = (midpointSecondTens << 4) | (encodedMinutes - encodedMinuteTens * 10);

                                SendCdCommand(CHECKPS_CD_CMD_READ_TOC);
                                g_checkpsState = CHECKPS_STATE_WAIT_READ_TOC;
                            }
                        }
                    }
                    stepResult = CHECKPS_STATE_WAIT_SETLOC;
                }
            }
            break;

        case CHECKPS_STATE_WAIT_READ_TOC: /* ReadTOC can supply the position bytes later used by Setloc. */
        {
            stepResult = PollCdResponse(CHECKPS_CD_CMD_READ_TOC);
            switch (stepResult)
            {
            case CHECKPS_CD_POLL_DISK_ERROR:
            {
                /* The post-increment is intentional: it makes both response-byte
                   reads use one base register in the original code shape. */
                u8* responseCursor = (u8*)&g_cdResponse;
                u8* responseBase = responseCursor;
                if (responseBase[0] & 1)
                {
                    if (*++responseCursor & 0x40)
                    {
                        u8 seekMinuteBcd = g_cdSeekPositionBcd[0];
                        u8 seekSecondBcd = g_cdSeekPositionBcd[1];
                        u8* commandParams;
                        u32 commandParamsAddress;
                        stepResult = CHECKPS_STATE_WAIT_READ_TOC;
                        commandParamsAddress =
                            ((u32)g_cdCommandParameters + (u32)singleStep) - (u32)singleStep;
                        commandParams = (u8*)commandParamsAddress;
                        commandParams[2] = 0;
                        *commandParams++ = seekMinuteBcd;
                        *commandParams = seekSecondBcd;
                        SendCdCommand(CHECKPS_CD_CMD_SETLOC);
                        g_checkpsState = CHECKPS_STATE_WAIT_SETLOC;
                    }
                }
                else
                {
                    restartStateOrVsync = 1;
                    g_checkpsState = restartStateOrVsync;
                    stepResult = CHECKPS_CD_POLL_DISK_ERROR;
                }
            }
            break;

            case CHECKPS_CD_POLL_PENDING:
                stepResult = CHECKPS_STATE_WAIT_READ_TOC;
                break;

            case CHECKPS_CD_POLL_COMPLETE:
                SendCdCommand(CHECKPS_CD_CMD_GET_ID);
                g_checkpsState = CHECKPS_STATE_WAIT_GET_ID;
                stepResult = CHECKPS_STATE_WAIT_READ_TOC;
                break;

            case CHECKPS_CD_POLL_SHELL_OPEN:
                SendCdCommand(CHECKPS_CD_CMD_NOP);
                g_checkpsState = CHECKPS_STATE_WAIT_RECOVERY_NOP;
                stepResult = CHECKPS_CD_POLL_DISK_ERROR;
                break;

            default:
                stepResult = CHECKPS_STATE_WAIT_READ_TOC;
                break;
            }

            break;
        }

        case CHECKPS_STATE_WAIT_GET_ID: /* GetID shares the Setloc parameter setup path. */
            stepResult = PollCdResponse(CHECKPS_CD_CMD_GET_ID);
            switch (stepResult)
            {
            case CHECKPS_CD_POLL_DISK_ERROR:
                ((s32 (*)(void))ShowHardwareModificationWarningAndExit)();
                stepResult = CHECKPS_CD_POLL_DISK_ERROR;
                break;

            case CHECKPS_CD_POLL_COMPLETE:
            {
                u8 seekMinuteBcd;
                u8 seekSecondBcd;
                u8* commandParams;
                u32 commandParamsAddress;
                stepResult = CHECKPS_STATE_WAIT_GET_ID;
            get_id_apply_seek_position:
                seekMinuteBcd = g_cdSeekPositionBcd[0];
                seekSecondBcd = g_cdSeekPositionBcd[1];
                commandParamsAddress =
                    ((u32)g_cdCommandParameters + (u32)singleStep) - (u32)singleStep;
                commandParams = (u8*)commandParamsAddress;
                commandParams[2] = 0;
                *commandParams++ = seekMinuteBcd;
                *commandParams = seekSecondBcd;
                /* This cancels to SETLOC; preserve the expression shape for GCC 2.7.2 register allocation. */
                SendCdCommand(3 + ((stepResult ^ singleStep) ^ stepResult ^ singleStep));
                g_checkpsState = CHECKPS_STATE_WAIT_SETLOC;
            }
            /* fall through */
            case CHECKPS_CD_POLL_PENDING:
                stepResult = CHECKPS_STATE_WAIT_GET_ID;
                break;

            case CHECKPS_CD_POLL_SHELL_OPEN:
                SendCdCommand(CHECKPS_CD_CMD_NOP);
                g_checkpsState = CHECKPS_STATE_WAIT_RECOVERY_NOP;
                stepResult = CHECKPS_CD_POLL_DISK_ERROR;
                break;

            default:
                stepResult = CHECKPS_STATE_WAIT_GET_ID;
                break;
            }

            break;

        case CHECKPS_STATE_WAIT_SETLOC:
            stepResult = PollCdResponse(CHECKPS_CD_CMD_SETLOC);
            if (stepResult == CHECKPS_CD_POLL_DISK_ERROR)
            {
                restartStateOrVsync = 1;
                g_checkpsState = restartStateOrVsync;
                stepResult = CHECKPS_CD_POLL_DISK_ERROR;
            }
            else
            {
            setloc_poll_result:
                if (stepResult < 0)
                {
                    if (stepResult == CHECKPS_CD_POLL_SHELL_OPEN)
                    {
                        SendCdCommand(CHECKPS_CD_CMD_NOP);
                        g_checkpsState = CHECKPS_STATE_WAIT_RECOVERY_NOP;
                        stepResult = CHECKPS_CD_POLL_DISK_ERROR;
                    }
                    else
                    {
                        stepResult = CHECKPS_STATE_WAIT_SETLOC;
                    }
                }
                else
                {
                    if (stepResult != CHECKPS_CD_POLL_PENDING)
                    {
                        if (stepResult == CHECKPS_CD_POLL_COMPLETE)
                        {
                            g_cdCommandParameters[0] = (u8)stepResult;
                            SendCdCommand(CHECKPS_CD_CMD_SETMODE);
                            g_checkpsState = CHECKPS_STATE_WAIT_SETMODE;
                        }
                    }
                    stepResult = CHECKPS_STATE_WAIT_SETLOC;
                }
            }
            break;

        case CHECKPS_STATE_WAIT_SETMODE:
            stepResult = PollCdResponse(CHECKPS_CD_CMD_SETMODE);
            if (stepResult == CHECKPS_CD_POLL_DISK_ERROR)
            {
                restartStateOrVsync = 1;
                g_checkpsState = restartStateOrVsync;
                stepResult = CHECKPS_CD_POLL_DISK_ERROR;
            }
            else
            {
            setmode_poll_result:
                if (stepResult < 0)
                {
                    if (stepResult == CHECKPS_CD_POLL_SHELL_OPEN)
                    {
                        SendCdCommand(CHECKPS_CD_CMD_NOP);
                        g_checkpsState = CHECKPS_STATE_WAIT_RECOVERY_NOP;
                        stepResult = CHECKPS_CD_POLL_DISK_ERROR;
                    }
                    else
                    {
                        stepResult = CHECKPS_STATE_WAIT_SETMODE;
                    }
                }
                else
                {
                    if (stepResult != CHECKPS_CD_POLL_PENDING)
                    {
                        if (stepResult == CHECKPS_CD_POLL_COMPLETE)
                        {
                            g_checkpsVsyncTimestamp = VSync(-1);
                            g_checkpsState = CHECKPS_STATE_SEEK_DELAY;
                        }
                    }
                    stepResult = CHECKPS_STATE_WAIT_SETMODE;
                }
            }
            break;

        case CHECKPS_STATE_SEEK_DELAY: /* Delay three VSync intervals before SeekP. */

            restartStateOrVsync = VSync(-1);
            if ((g_checkpsVsyncTimestamp + 3) < restartStateOrVsync)
            {
                SendCdCommand(CHECKPS_CD_CMD_SEEK_P);
                g_checkpsState = CHECKPS_STATE_WAIT_SEEK_P;
            }
            stepResult = CHECKPS_STATE_SEEK_DELAY;
            break;

        case CHECKPS_STATE_WAIT_SEEK_P:
            stepResult = PollCdResponse(CHECKPS_CD_CMD_SEEK_P);
            if (stepResult == CHECKPS_CD_POLL_DISK_ERROR)
            {
                restartStateOrVsync = 1;
                g_checkpsState = restartStateOrVsync;
                stepResult = CHECKPS_CD_POLL_DISK_ERROR;
            }
            else
            {
            seek_p_poll_result:
                if (stepResult < 0)
                {
                    if (stepResult == CHECKPS_CD_POLL_SHELL_OPEN)
                    {
                        SendCdCommand(CHECKPS_CD_CMD_NOP);
                        g_checkpsState = CHECKPS_STATE_WAIT_RECOVERY_NOP;
                        stepResult = CHECKPS_CD_POLL_DISK_ERROR;
                    }
                    else
                    {
                        stepResult = CHECKPS_STATE_WAIT_SEEK_P;
                    }
                }
                else
                {
                    if (stepResult != CHECKPS_CD_POLL_PENDING)
                    {
                        if (stepResult == CHECKPS_CD_POLL_COMPLETE)
                        {
                            SendCdCommand(CHECKPS_CD_CMD_MUTE);
                            g_checkpsState = CHECKPS_STATE_WAIT_MUTE;
                        }
                    }
                    stepResult = CHECKPS_STATE_WAIT_SEEK_P;
                }
            }
            break;

        case CHECKPS_STATE_WAIT_MUTE:
            stepResult = PollCdResponse(CHECKPS_CD_CMD_MUTE);
            if (stepResult == CHECKPS_CD_POLL_DISK_ERROR)
            {
                restartStateOrVsync = 1;
                g_checkpsState = restartStateOrVsync;
                stepResult = CHECKPS_CD_POLL_DISK_ERROR;
            }
            else
            {
            mute_poll_result:
                if (stepResult < 0)
                {
                    if (stepResult == CHECKPS_CD_POLL_SHELL_OPEN)
                    {
                        SendCdCommand(CHECKPS_CD_CMD_NOP);
                        g_checkpsState = CHECKPS_STATE_WAIT_RECOVERY_NOP;
                        stepResult = CHECKPS_CD_POLL_DISK_ERROR;
                    }
                    else
                    {
                        stepResult = CHECKPS_STATE_WAIT_MUTE;
                    }
                }
                else
                {
                    if (stepResult != CHECKPS_CD_POLL_PENDING)
                    {
                        if (stepResult == CHECKPS_CD_POLL_COMPLETE)
                        {
                            SendCdCommand(CHECKPS_CD_CMD_PLAY);
                            g_checkpsState = CHECKPS_STATE_WAIT_PLAY;
                        }
                    }
                    stepResult = CHECKPS_STATE_WAIT_MUTE;
                }
            }
            break;

        case CHECKPS_STATE_WAIT_PLAY:
            stepResult = PollCdResponse(CHECKPS_CD_CMD_PLAY);
            if (stepResult == CHECKPS_CD_POLL_DISK_ERROR)
            {
                restartStateOrVsync = 1;
                g_checkpsState = restartStateOrVsync;
                stepResult = CHECKPS_CD_POLL_DISK_ERROR;
            }
            else
            {
            play_poll_result:
                if (stepResult < 0)
                {
                    if (stepResult == CHECKPS_CD_POLL_SHELL_OPEN)
                    {
                        SendCdCommand(CHECKPS_CD_CMD_NOP);
                        g_checkpsState = CHECKPS_STATE_WAIT_RECOVERY_NOP;
                        stepResult = CHECKPS_CD_POLL_DISK_ERROR;
                    }
                    else
                    {
                        stepResult = CHECKPS_STATE_WAIT_PLAY;
                    }
                }
                else
                {
                    if (stepResult != CHECKPS_CD_POLL_PENDING)
                    {
                        if (stepResult == CHECKPS_CD_POLL_COMPLETE)
                        {
                            g_cdCommandParameters[0] = 4;
                            SendCdCommand(CHECKPS_CD_CMD_TEST_04);
                            g_checkpsState = CHECKPS_STATE_WAIT_TEST_04;
                        }
                    }
                    stepResult = CHECKPS_STATE_WAIT_PLAY;
                }
            }
            break;

        case CHECKPS_STATE_WAIT_TEST_04:
            stepResult = PollCdResponse(CHECKPS_CD_CMD_TEST_04);
            if (stepResult == CHECKPS_CD_POLL_DISK_ERROR)
            {
                restartStateOrVsync = 1;
                g_checkpsState = restartStateOrVsync;
                stepResult = CHECKPS_CD_POLL_DISK_ERROR;
            }
            else
            {
            test_04_poll_result:
                if (stepResult < 0)
                {
                    if (stepResult == CHECKPS_CD_POLL_SHELL_OPEN)
                    {
                        SendCdCommand(CHECKPS_CD_CMD_NOP);
                        g_checkpsState = CHECKPS_STATE_WAIT_RECOVERY_NOP;
                        stepResult = CHECKPS_CD_POLL_DISK_ERROR;
                    }
                    else
                    {
                        stepResult = CHECKPS_STATE_WAIT_TEST_04;
                    }
                }
                else
                {
                    if (stepResult != CHECKPS_CD_POLL_PENDING)
                    {
                        if (stepResult == CHECKPS_CD_POLL_COMPLETE)
                        {
                            g_checkpsVsyncTimestamp = VSync(-1);
                            g_checkpsState = CHECKPS_STATE_TEST_05_DELAY;
                        }
                    }
                    stepResult = CHECKPS_STATE_WAIT_TEST_04;
                }
            }
            break;

        case CHECKPS_STATE_TEST_05_DELAY: /* Delay 200 VSync intervals before Test(0x05). */
            restartStateOrVsync = VSync(-1);
            if ((g_checkpsVsyncTimestamp + 0xC8) < restartStateOrVsync)
            {
                g_cdCommandParameters[0] = 5;
                SendCdCommand(CHECKPS_CD_CMD_TEST_05);
                g_checkpsState = CHECKPS_STATE_WAIT_TEST_05;
            }
            stepResult = CHECKPS_STATE_TEST_05_DELAY;
            break;

        case CHECKPS_STATE_WAIT_TEST_05:
            stepResult = PollCdResponse(CHECKPS_CD_CMD_TEST_05);
            if (stepResult == CHECKPS_CD_POLL_DISK_ERROR)
            {
                restartStateOrVsync = 1;
                g_checkpsState = restartStateOrVsync;
                stepResult = CHECKPS_CD_POLL_DISK_ERROR;
            }
            else
            {
            test_05_poll_result:
                if (stepResult < 0)
                {
                    if (stepResult == CHECKPS_CD_POLL_SHELL_OPEN)
                    {
                        SendCdCommand(CHECKPS_CD_CMD_NOP);
                        g_checkpsState = CHECKPS_STATE_WAIT_RECOVERY_NOP;
                        stepResult = CHECKPS_CD_POLL_DISK_ERROR;
                    }
                    else
                    {
                        stepResult = CHECKPS_STATE_WAIT_TEST_05;
                    }
                }
                else
                {
                    if (stepResult != CHECKPS_CD_POLL_PENDING)
                    {
                        if (stepResult == CHECKPS_CD_POLL_COMPLETE)
                        {
                            stepResult = CHECKPS_STATE_WAIT_TEST_05;
                        test_05_check_response:
                            if (g_cdResponsePayload[0] != 0)
                            {
                                SendCdCommand(CHECKPS_CD_CMD_NOP);
                                g_checkpsState = CHECKPS_STATE_WAIT_FAILURE_NOP;
                            }
                            else
                            {
                                g_checkpsVsyncTimestamp = VSync(-1);
                                g_checkpsState = CHECKPS_STATE_PAUSE_DELAY;
                            }
                        }
                    }
                    stepResult = CHECKPS_STATE_WAIT_TEST_05;
                }
            }
            break;

        case CHECKPS_STATE_WAIT_FAILURE_NOP:
            stepResult = PollCdResponse(CHECKPS_CD_CMD_NOP);
            if (stepResult == CHECKPS_CD_POLL_DISK_ERROR)
            {
                restartStateOrVsync = 1;
                g_checkpsState = restartStateOrVsync;
                stepResult = CHECKPS_CD_POLL_DISK_ERROR;
            }
            else
            {
            failure_nop_poll_result:
                if (stepResult < 0)
                {
                    if (stepResult == CHECKPS_CD_POLL_SHELL_OPEN)
                    {
                        SendCdCommand(CHECKPS_CD_CMD_NOP);
                        g_checkpsState = CHECKPS_STATE_WAIT_RECOVERY_NOP;
                        stepResult = CHECKPS_CD_POLL_DISK_ERROR;
                    }
                    else
                    {
                        stepResult = CHECKPS_STATE_WAIT_FAILURE_NOP;
                    }
                }
                else
                {
                    if (stepResult == CHECKPS_CD_POLL_PENDING)
                    {
                        stepResult = 16 + ((stepResult & 1) >> 1);
                        break;
                    }
                    if (stepResult != CHECKPS_CD_POLL_COMPLETE)
                    {
                        stepResult = CHECKPS_STATE_WAIT_FAILURE_NOP;
                        break;
                    }
                    stepResult = CHECKPS_STATE_WAIT_FAILURE_NOP;
                    ShowHardwareModificationWarningAndExit();
                    break;
                }
            }
            break;
            stepResult = CHECKPS_STATE_WAIT_FAILURE_NOP;
            break;

        case CHECKPS_STATE_WAIT_RECOVERY_NOP:
            stepResult = PollCdResponse(CHECKPS_CD_CMD_NOP);
            if (stepResult == CHECKPS_CD_POLL_DISK_ERROR)
            {
                restartStateOrVsync = 1;
                g_checkpsState = restartStateOrVsync;
                stepResult = CHECKPS_CD_POLL_DISK_ERROR;
                break;
            }
            else
            {
            recovery_nop_poll_result:
                if (stepResult < 0)
                {
                    if (stepResult != CHECKPS_CD_POLL_SHELL_OPEN)
                    {
                        stepResult = CHECKPS_STATE_WAIT_FAILURE_NOP;
                        break;
                    }
                }
                else
                {
                    switch (stepResult)
                    {
                    case CHECKPS_CD_POLL_COMPLETE:
                        g_checkpsState = (u32)stepResult;
                        /* fall through */
                    case CHECKPS_CD_POLL_PENDING:
                    default:
                    recovery_nop_finalize:
                        stepResult = 16 + ((stepResult & 1) >> 1);
                        break;
                    }
                    break;
                }
            }
        recovery_nop_reset_command:
            SendCdCommand(CHECKPS_CD_CMD_NOP);
            stepResult = CHECKPS_STATE_WAIT_FAILURE_NOP;
            break;

        case CHECKPS_STATE_WAIT_PAUSE:
            stepResult = PollCdResponse(CHECKPS_CD_CMD_PAUSE);
            if (stepResult != CHECKPS_CD_POLL_DISK_ERROR)
            {
            pause_poll_result:
                if (stepResult < 0)
                {
                    if (stepResult != CHECKPS_CD_POLL_SHELL_OPEN)
                    {
                        stepResult = CHECKPS_STATE_WAIT_PAUSE;
                    pause_done:
                        break;
                    }
                    SendCdCommand(CHECKPS_CD_CMD_NOP);
                    g_checkpsState = CHECKPS_STATE_WAIT_RECOVERY_NOP;
                    stepResult = CHECKPS_CD_POLL_DISK_ERROR;
                    break;
                }
                if (stepResult == CHECKPS_CD_POLL_PENDING)
                {
                }
                else if (stepResult != CHECKPS_CD_POLL_COMPLETE)
                {
                    stepResult = CHECKPS_STATE_WAIT_PAUSE;
                    break;
                }
                if (stepResult < 0)
                {
                pause_command_error:
                    SendCdCommand(CHECKPS_CD_CMD_NOP);
                    g_checkpsState = CHECKPS_STATE_WAIT_RECOVERY_NOP;
                    stepResult = CHECKPS_CD_POLL_DISK_ERROR;
                    break;
                }
            }
            else
            {
                restartStateOrVsync = 1;
                g_checkpsState = restartStateOrVsync;
                stepResult = CHECKPS_CD_POLL_DISK_ERROR;
                break;
            }
            if (stepResult != CHECKPS_CD_POLL_PENDING)
                g_checkpsState = CHECKPS_STATE_IDLE;
        state18_finalize:
            stepResult = 18 + ((stepResult & 1) >> 1);
            break;

        case CHECKPS_STATE_PAUSE_DELAY: /* Delay ten VSync intervals before Pause. */
            restartStateOrVsync = VSync(-1);
            if ((g_checkpsVsyncTimestamp + 10) < restartStateOrVsync)
            {
                SendCdCommand(CHECKPS_CD_CMD_PAUSE);
                g_checkpsState = CHECKPS_STATE_WAIT_PAUSE;
            }
            stepResult = CHECKPS_STATE_PAUSE_DELAY;
            break;

        case CHECKPS_STATE_IDLE: /* Idle. */
            stepResult = CHECKPS_STATE_IDLE;
            break;
        }

        if ((singleStep == 0) && (stepResult != CHECKPS_STATE_IDLE))
        {
            continue;
        }
        return stepResult;
    recovery_nop_return:
        if ((singleStep == 0) && (stepResult != CHECKPS_STATE_IDLE))
        {
            continue;
        }
        return stepResult;
    }
}

/** Poll and consume the response for a CHECKPS CD command. */
CheckPSCdPollResult PollCdResponse(CheckPSCdCommandIndex command)
{
    u8 irqCodeSumTarget;
    u8 irqSampleA;
    u8 irqSampleB;
    s32 irqCode;
    s32 irqCodeByte;
    s32 delayCounter;
    int stableIrq;
    s32 responseIndex;
    irqCodeSumTarget = g_cdCommandTable[command].irqCodeSumTarget;
    *g_cdStatusRegister = 1;
    irqSampleA = *g_cdIrqRegister;
    irqSampleB = *g_cdIrqRegister;
    if ((stableIrq = irqSampleA & 7) == (irqSampleB & 7))
    {
        irqCode = stableIrq;
        irqCodeByte = (unsigned char)irqCode;
        if (irqCodeByte != 0)
        {
            g_cdIrqCodeSum = g_cdIrqCodeSum + irqCodeByte;
            *g_cdStatusRegister = 1;
            *g_cdIrqRegister = 7;
            delayCounter = 0;
            /* Preserve the original four address-zero writes used as a short hardware delay. */
            do
            {
                *((int*)0) = delayCounter;
                delayCounter++;
            } while (delayCounter < 4);
            if (g_cdIrqCodeSum >= (s32)irqCodeSumTarget)
            {
                g_cdIrqCodeSum = 0;
                if (irqCode == CHECKPS_CD_IRQ_DISK_ERROR)
                {
                    while (1)
                    {
                        g_cdResponse.status = *g_cdResponseRegister;
                        break;
                    }
                    g_cdResponse.detail = *g_cdResponseRegister;
                    *g_cdStatusRegister = 1;
                    *g_cdDataRegister = 0x1F;
                    if (!(g_cdResponse.status & CHECKPS_CD_STATUS_SHELL_OPEN))
                    {
                        return CHECKPS_CD_POLL_DISK_ERROR;
                    }

                    return CHECKPS_CD_POLL_SHELL_OPEN;
                }
                else
                {
                    irqCodeByte = 0;
                    responseIndex = irqCodeByte;
                    if (g_cdCommandTable[command].responseCount != irqCodeByte)
                    {
                        do
                        {
                            ((u8*)(&g_cdResponse))[responseIndex] = *g_cdResponseRegister;
                            responseIndex++;
                        } while (responseIndex < (s32)g_cdCommandTable[command].responseCount);
                    }
                    *g_cdStatusRegister = 1;
                    *g_cdDataRegister = 0x1F;
                    if (command != CHECKPS_CD_CMD_TEST_05)
                    {
                        responseIndex = 0;
                        while (1)
                        {
                            if (responseIndex)
                                return CHECKPS_CD_POLL_SHELL_OPEN;
                            responseIndex = g_cdResponse.status;
                            responseIndex &= 0x10;
                            if (!responseIndex)
                                break;
                        }
                    }
                    return CHECKPS_CD_POLL_COMPLETE;
                }
            }
            return CHECKPS_CD_POLL_PENDING;
        }
    }
    return CHECKPS_CD_POLL_PENDING;
}

/** Write one CHECKPS command descriptor and its parameters to the CD registers. */
void SendCdCommand(CheckPSCdCommandIndex command)
{
    s32 delayCounter = 0;
    s32* addressZeroDelaySink = 0;
    s32 parameterIndex;
    unsigned int descriptorByteOffset;

    *g_cdStatusRegister = 1;
    *g_cdIrqRegister = 7;

    /* The original performs four writes through address zero between CD-register
       updates. Preserve the sequence because it affects the matched instruction stream. */
    for (delayCounter = 0; delayCounter < 4; delayCounter++) *addressZeroDelaySink = delayCounter;

    *g_cdStatusRegister = 1;
    *g_cdDataRegister = 0x18;
    *g_cdStatusRegister = 0;

    /* Keep byte indexing through the field pointer: direct table[command] field
       accesses change GCC 2.7.2 register allocation in this matched function. */
    descriptorByteOffset = command * sizeof(CdCommandDescriptor);

    parameterIndex = 0;
    if ((&g_cdCommandTable->parameterCount)[descriptorByteOffset])
    {
        do
        {
            *g_cdDataRegister = g_cdCommandParameters[parameterIndex];
            parameterIndex++;
        } while (parameterIndex < (&g_cdCommandTable->parameterCount)[descriptorByteOffset]);
    }

    *g_cdStatusRegister = 0;
    *g_cdResponseRegister = (&g_cdCommandTable->opcode)[command * 4];
}

/** Reset the GPU, display the hardware-modification warning, and terminate. */
void ShowHardwareModificationWarningAndExit(void)
{
    DRAWENV drawEnv;
    DISPENV dispEnv;
    DR_ENV drawEnvPacket;
    u32 gpuCommands[3];
    KanjiDrawStateWords textState;
    s32 textColor;
    s32 lineIndex;
    ResetGraph(1);
    StopCallback();
    ResetGraph(5);
    *((s16*)0x1F801DAA) = 0; /* SPU control register. */
    SetDefDrawEnv(&drawEnv, 0, 0, 0x140, 0xF0);
    SetDefDispEnv(&dispEnv, 0, 0, 0x140, 0xF0);
    drawEnv.isbg = 1;
    SetDrawEnv(&drawEnvPacket, &drawEnv);
    DrawPrim(&drawEnvPacket);
    PutDispEnv(&dispEnv);
    gpuCommands[0] = 0x02000000;
    gpuCommands[1] = 0xE6000002;
    gpuCommands[2] = 0;
    DrawPrim(gpuCommands);
    textColor = 0xFFFF;

    textState.width = 0x10;
    textState.height = 1;

    for (lineIndex = 0; lineIndex < 2; lineIndex++)
    {
        textState.x = lineIndex + 0x50;
        textState.y = lineIndex + 0x5C;
        DrawKanjiString((const char*)&g_hardwareModificationWarning, (KanjiDrawState*)&textState, textColor);
        textColor = 0x8000;
    }

    DrawHardwareCheckPattern();
    SetDispMask(1);
    exit();
}
