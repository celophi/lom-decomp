#include "checkps_internal.h"

#include "display.h"
#include "psyq/libapi.h"
#include "psyq/libgte.h"
#include "psyq/libgpu.h"

#define CHECKPS_GPU_FILL_RECT_COMMAND 0x02000000
#define CHECKPS_GPU_MASK_BIT_COMMAND 0xE6000002

#define CHECKPS_WARNING_LINE_COUNT 2
#define CHECKPS_WARNING_TEXT_X 0x50
#define CHECKPS_WARNING_TEXT_Y 0x5C
#define CHECKPS_WARNING_TEXT_WIDTH 16
#define CHECKPS_WARNING_PRIMARY_COLOR 0xFFFF
#define CHECKPS_WARNING_SHADOW_COLOR 0x8000
#define CHECKPS_SPU_CONTROL_REGISTER ((s16*)0x1F801DAA)

/* These mirror CdlDiskError/CdlStatShellOpen; libcd.h is not GCC 2.7.2-clean. */
#define CHECKPS_CD_IRQ_DISK_ERROR 5
#define CHECKPS_CD_STATUS_SHELL_OPEN 0x10
#define CHECKPS_CD_TEST_DELAY_FRAMES 200
#define CHECKPS_CD_PAUSE_DELAY_FRAMES 10
#define CHECKPS_CD_IRQ_ACK_MASK 0x1F
#define CHECKPS_CD_PARAMETER_MODE 0x18
#define CHECKPS_CD_IRQ_STATUS_MASK 7
#define CHECKPS_POINTER_IDENTITY_MAGIC 0x88888889U

/**
 * @brief Named prefix of the CD controller response buffer.
 */
typedef struct
{
    u8 status;
    u8 detail;
} CdResponsePrefix;

/**
 * @brief Opcode and transfer counts for one CD controller command.
 */
typedef struct
{
    u8 opcode;
    u8 parameter_count;
    u8 response_count;
    u8 irq_code_sum_target;
} CdCommandDescriptor;

/**
 * @brief Indices into the CHECKPS CD command descriptor table.
 */
typedef enum
{
    CHECKPS_CD_CMD_NOP = 0,
    CHECKPS_CD_CMD_GET_TN,
    CHECKPS_CD_CMD_GET_TD,
    CHECKPS_CD_CMD_SETLOC,
    CHECKPS_CD_CMD_SEEK_P,
    CHECKPS_CD_CMD_SETMODE,
    CHECKPS_CD_CMD_INIT,
    CHECKPS_CD_CMD_MUTE,
    CHECKPS_CD_CMD_PLAY,
    CHECKPS_CD_CMD_TEST_04,
    CHECKPS_CD_CMD_TEST_05,
    CHECKPS_CD_CMD_PAUSE,
    CHECKPS_CD_CMD_READ_TOC,
    CHECKPS_CD_CMD_GET_ID,
} CheckPSCdCommandIndex;

/**
 * @brief States in the CHECKPS CD integrity-check state machine.
 */
typedef enum
{
    CHECKPS_STATE_IDLE = 0,
    CHECKPS_STATE_START_GET_TN,
    CHECKPS_STATE_WAIT_GET_TN,
    CHECKPS_STATE_WAIT_INIT,
    CHECKPS_STATE_WAIT_GET_TD,
    CHECKPS_STATE_WAIT_READ_TOC,
    CHECKPS_STATE_WAIT_GET_ID,
    CHECKPS_STATE_WAIT_SETLOC,
    CHECKPS_STATE_WAIT_SETMODE,
    CHECKPS_STATE_SEEK_DELAY,
    CHECKPS_STATE_WAIT_SEEK_P,
    CHECKPS_STATE_WAIT_MUTE,
    CHECKPS_STATE_WAIT_PLAY,
    CHECKPS_STATE_WAIT_TEST_04,
    CHECKPS_STATE_TEST_05_DELAY,
    CHECKPS_STATE_WAIT_TEST_05,
    CHECKPS_STATE_WAIT_FAILURE_NOP,
    CHECKPS_STATE_WAIT_RECOVERY_NOP,
    CHECKPS_STATE_WAIT_PAUSE,
    CHECKPS_STATE_PAUSE_DELAY,
} CheckPSState;

/**
 * @brief Outcomes from polling an in-flight CD controller command.
 */
typedef enum
{
    CHECKPS_CD_POLL_SHELL_OPEN = -2,
    CHECKPS_CD_POLL_DISK_ERROR = -1,
    CHECKPS_CD_POLL_PENDING = 0,
    CHECKPS_CD_POLL_COMPLETE = 1,
} CheckPSCdPollResult;

/**
 * @brief Kanji draw state viewed as four consecutive halfwords.
 */
typedef struct
{
    s16 x;
    s16 y;
    s16 width;
    s16 height;
} KanjiDrawStateWords;

extern s32 g_checkps_state;
extern CdCommandDescriptor g_cd_command_table[];
extern u8 g_cd_command_parameters[3];
extern CdResponsePrefix g_cd_response;
extern s32 g_cd_irq_code_sum;
extern u8 g_cd_response_byte2;
extern u8 g_cd_response_payload[2];
extern volatile u8* g_cd_status_register;
extern volatile u8* g_cd_response_register;
extern volatile u8* g_cd_data_register;
extern volatile u8* g_cd_irq_register;

/*
 * GNU as 2.7 pads the standard .text section to a 16-byte boundary.  Keeping
 * this translation unit's code in a custom section avoids synthetic tail
 * bytes; the build renames the section back to .text with objcopy.
 */
#define CHECKPS_GNU_TEXT __attribute__((section(".text.cdrom")))

/*
 * Keep the section attribute on declarations so Splat can discover each
 * function definition normally while GNU as emits this unit into .text.cdrom.
 */
void start_cd_integrity_check(void) CHECKPS_GNU_TEXT;
s32 run_cd_integrity_check(s32 single_step) CHECKPS_GNU_TEXT;
CheckPSCdPollResult poll_cd_response(CheckPSCdCommandIndex command) CHECKPS_GNU_TEXT;
void send_cd_command(CheckPSCdCommandIndex command) CHECKPS_GNU_TEXT;
void show_hardware_modification_warning_and_exit(void) CHECKPS_GNU_TEXT;

/**
 * @brief Initialize the CHECKPS CD integrity state machine.
 */
void start_cd_integrity_check(void)
{
    g_checkps_state = CHECKPS_STATE_START_GET_TN;
}

/**
 * @brief Advance the CHECKPS CD integrity state machine.
 * @param single_step Nonzero to execute one transition; zero to run until idle.
 * @return Current state/result code after processing.
 */
s32 run_cd_integrity_check(s32 single_step)
{
    /* GCC shares this temporary between restart-state stores and VSync samples. */
    s32 restart_state_or_vsync;
    s32 step_result = CHECKPS_STATE_IDLE;
    static void* compiler_label_anchors[] = {&&init_poll_result,  &&get_td_poll_result,  &&setloc_poll_result,  &&setmode_poll_result,  &&seek_p_poll_result,    &&mute_poll_result,   &&play_poll_result,  &&test_04_poll_result,
                                  &&test_05_poll_result, &&failure_nop_poll_result, &&recovery_nop_poll_result, &&pause_poll_result, &&get_id_apply_seek_position, &&test_05_check_response, &&pause_command_error, &&pause_done};

    for (;;)
    {
        switch (g_checkps_state)
        {
        case CHECKPS_STATE_START_GET_TN: /* Start the first CD command in the sequence. */
            send_cd_command(CHECKPS_CD_CMD_GET_TN);
            g_checkps_state = CHECKPS_STATE_WAIT_GET_TN;
            step_result = CHECKPS_STATE_START_GET_TN;
            break;

        case CHECKPS_STATE_WAIT_GET_TN: /* Save the last-track byte returned by GetTN. */
            step_result = poll_cd_response(CHECKPS_CD_CMD_GET_TN);
            switch (step_result)
            {
            case CHECKPS_CD_POLL_SHELL_OPEN:
                send_cd_command(CHECKPS_CD_CMD_NOP);
                g_checkps_state = CHECKPS_STATE_WAIT_RECOVERY_NOP;
                step_result = CHECKPS_STATE_WAIT_GET_TN;
                break;

            case CHECKPS_CD_POLL_PENDING:
                step_result = CHECKPS_STATE_WAIT_GET_TN;
                break;

            case CHECKPS_CD_POLL_COMPLETE:
                g_cd_last_track_bcd = g_cd_response_byte2;
                step_result = CHECKPS_STATE_WAIT_GET_TN;
                send_cd_command(CHECKPS_CD_CMD_INIT);
                g_checkps_state = CHECKPS_STATE_WAIT_INIT;
                break;

            case CHECKPS_CD_POLL_DISK_ERROR:
                send_cd_command(CHECKPS_CD_CMD_GET_TN);
                step_result = CHECKPS_STATE_WAIT_GET_TN;
                break;

            default:
                step_result = CHECKPS_STATE_WAIT_GET_TN;
                break;
            }

            break;

        case CHECKPS_STATE_WAIT_INIT:
            step_result = poll_cd_response(CHECKPS_CD_CMD_INIT);
            if (step_result == CHECKPS_CD_POLL_DISK_ERROR)
            {
                restart_state_or_vsync = 1;
                g_checkps_state = restart_state_or_vsync;
                step_result = CHECKPS_CD_POLL_DISK_ERROR;
            }
            else
            {
            init_poll_result:
                if (step_result < 0)
                {
                    if (step_result == CHECKPS_CD_POLL_SHELL_OPEN)
                    {
                        send_cd_command(CHECKPS_CD_CMD_NOP);
                        g_checkps_state = CHECKPS_STATE_WAIT_RECOVERY_NOP;
                        step_result = CHECKPS_CD_POLL_DISK_ERROR;
                    }
                    else
                    {
                        step_result = CHECKPS_STATE_WAIT_INIT;
                    }
                }
                else
                {
                    if (step_result != CHECKPS_CD_POLL_PENDING)
                    {
                        if (step_result == CHECKPS_CD_POLL_COMPLETE)
                        {
                            g_cd_command_parameters[0] = (g_cd_last_track_bcd >= 2) ? 2 : 0;
                            send_cd_command(CHECKPS_CD_CMD_GET_TD);
                            g_checkps_state = CHECKPS_STATE_WAIT_GET_TD;
                        }
                    }
                    step_result = CHECKPS_STATE_WAIT_INIT;
                }
            }
            break;

        case CHECKPS_STATE_WAIT_GET_TD:
            step_result = poll_cd_response(CHECKPS_CD_CMD_GET_TD);
            if (step_result == CHECKPS_CD_POLL_DISK_ERROR)
            {
                restart_state_or_vsync = 1;
                g_checkps_state = restart_state_or_vsync;
                step_result = CHECKPS_CD_POLL_DISK_ERROR;
            }
            else
            {
            get_td_poll_result:
                if (step_result < 0)
                {
                    if (step_result == CHECKPS_CD_POLL_SHELL_OPEN)
                    {
                        send_cd_command(CHECKPS_CD_CMD_NOP);
                        g_checkps_state = CHECKPS_STATE_WAIT_RECOVERY_NOP;
                        step_result = CHECKPS_CD_POLL_DISK_ERROR;
                    }
                    else
                    {
                        step_result = CHECKPS_STATE_WAIT_SETLOC;
                    }
                }
                else
                {
                    if (step_result != CHECKPS_CD_POLL_PENDING)
                    {
                        if (step_result == CHECKPS_CD_POLL_COMPLETE)
                        {
                            {

                                u8* toc_time_bcd;
                                u8 toc_minutes_bcd;
                                u8 toc_seconds_bcd;
                                u32 midpoint_minutes_value;
                                u32 midpoint_seconds_value;
                                u32 encoded_minutes;
                                s32 toc_minutes;
                                s32 toc_seconds;
                                s32 midpoint_total_seconds;
                                s32 midpoint_minutes;
                                s32 midpoint_seconds;
                                int midpoint_minutes_store_index;
                                int midpoint_minutes_read_index;
                                int encoded_minutes_store_index;
                                int encoded_minutes_read_index;
                                int midpoint_seconds_store_index;
                                int midpoint_seconds_read_index;
                                u32 address_mixer;
                                u32 midpoint_second_tens;
                                u32 encoded_minute_tens;

                                /* These cancelling operations preserve the original pointer
                                   expression emitted by GCC 2.7.2. */
                                address_mixer = ((u32)CHECKPS_POINTER_IDENTITY_MAGIC + (u32)single_step) - (u32)single_step;
                                toc_time_bcd = (u8*)((u32)g_cd_response_payload + (address_mixer ^ (u32)CHECKPS_POINTER_IDENTITY_MAGIC));
                                toc_minutes_bcd = toc_time_bcd[0];
                                toc_seconds_bcd = toc_time_bcd[1];

                                toc_minutes = ((toc_minutes_bcd >> 4) * 10) + (toc_minutes_bcd & 0xF);
                                toc_seconds = (((toc_seconds_bcd >> 4) * 5) * 2) + (toc_seconds_bcd & 0xF);
                                midpoint_total_seconds = ((toc_minutes * 60) + toc_seconds) >> 1;
                                midpoint_minutes = midpoint_total_seconds / 60;
                                midpoint_seconds = midpoint_total_seconds % 60;

                                /* Volatile stores prevent GCC from forwarding these writes;
                                   the following plain reads are also required for the match. */
                                midpoint_minutes_store_index = 0;
                                *(volatile u8*)&g_cd_seek_position_bcd[midpoint_minutes_store_index] = midpoint_minutes;
                                midpoint_minutes_read_index = 0;
                                midpoint_minutes_value = g_cd_seek_position_bcd[midpoint_minutes_read_index];
                                midpoint_seconds_store_index = 1;
                                *(volatile u8*)&g_cd_seek_position_bcd[midpoint_seconds_store_index] = midpoint_seconds;
                                midpoint_seconds_read_index = 1;
                                midpoint_seconds_value = g_cd_seek_position_bcd[midpoint_seconds_read_index];
                                encoded_minutes_store_index = 0;
                                *(volatile u8*)&g_cd_seek_position_bcd[encoded_minutes_store_index] =
                                    ((midpoint_minutes_value / 10) << 4) | (midpoint_minutes_value % 10);
                                encoded_minutes_read_index = 0;
                                encoded_minutes = g_cd_seek_position_bcd[encoded_minutes_read_index];
                                midpoint_second_tens = midpoint_seconds_value / 10;
                                encoded_minute_tens = encoded_minutes / 10;
                                g_cd_seek_position_bcd[1] = (midpoint_second_tens << 4) | (encoded_minutes - encoded_minute_tens * 10);

                                send_cd_command(CHECKPS_CD_CMD_READ_TOC);
                                g_checkps_state = CHECKPS_STATE_WAIT_READ_TOC;
                            }
                        }
                    }
                    step_result = CHECKPS_STATE_WAIT_SETLOC;
                }
            }
            break;

        case CHECKPS_STATE_WAIT_READ_TOC: /* ReadTOC can supply the position bytes later used by Setloc. */
        {
            step_result = poll_cd_response(CHECKPS_CD_CMD_READ_TOC);
            switch (step_result)
            {
            case CHECKPS_CD_POLL_DISK_ERROR:
            {
                /* The post-increment is intentional: it makes both response-byte
                   reads use one base register in the original code shape. */
                u8* response_cursor = (u8*)&g_cd_response;
                u8* response_base = response_cursor;
                if (response_base[0] & 1)
                {
                    if (*++response_cursor & 0x40)
                    {
                        u8 seek_minute_bcd = g_cd_seek_position_bcd[0];
                        u8 seek_second_bcd = g_cd_seek_position_bcd[1];
                        u8* command_params;
                        u32 command_params_address;
                        step_result = CHECKPS_STATE_WAIT_READ_TOC;
                        command_params_address =
                            ((u32)g_cd_command_parameters + (u32)single_step) - (u32)single_step;
                        command_params = (u8*)command_params_address;
                        command_params[2] = 0;
                        *command_params++ = seek_minute_bcd;
                        *command_params = seek_second_bcd;
                        send_cd_command(CHECKPS_CD_CMD_SETLOC);
                        g_checkps_state = CHECKPS_STATE_WAIT_SETLOC;
                    }
                }
                else
                {
                    restart_state_or_vsync = 1;
                    g_checkps_state = restart_state_or_vsync;
                    step_result = CHECKPS_CD_POLL_DISK_ERROR;
                }
            }
            break;

            case CHECKPS_CD_POLL_PENDING:
                step_result = CHECKPS_STATE_WAIT_READ_TOC;
                break;

            case CHECKPS_CD_POLL_COMPLETE:
                send_cd_command(CHECKPS_CD_CMD_GET_ID);
                g_checkps_state = CHECKPS_STATE_WAIT_GET_ID;
                step_result = CHECKPS_STATE_WAIT_READ_TOC;
                break;

            case CHECKPS_CD_POLL_SHELL_OPEN:
                send_cd_command(CHECKPS_CD_CMD_NOP);
                g_checkps_state = CHECKPS_STATE_WAIT_RECOVERY_NOP;
                step_result = CHECKPS_CD_POLL_DISK_ERROR;
                break;

            default:
                step_result = CHECKPS_STATE_WAIT_READ_TOC;
                break;
            }

            break;
        }

        case CHECKPS_STATE_WAIT_GET_ID: /* GetID shares the Setloc parameter setup path. */
            step_result = poll_cd_response(CHECKPS_CD_CMD_GET_ID);
            switch (step_result)
            {
            case CHECKPS_CD_POLL_DISK_ERROR:
                ((s32 (*)(void))show_hardware_modification_warning_and_exit)();
                step_result = CHECKPS_CD_POLL_DISK_ERROR;
                break;

            case CHECKPS_CD_POLL_COMPLETE:
            {
                u8 seek_minute_bcd;
                u8 seek_second_bcd;
                u8* command_params;
                u32 command_params_address;
                step_result = CHECKPS_STATE_WAIT_GET_ID;
            get_id_apply_seek_position:
                seek_minute_bcd = g_cd_seek_position_bcd[0];
                seek_second_bcd = g_cd_seek_position_bcd[1];
                command_params_address =
                    ((u32)g_cd_command_parameters + (u32)single_step) - (u32)single_step;
                command_params = (u8*)command_params_address;
                command_params[2] = 0;
                *command_params++ = seek_minute_bcd;
                *command_params = seek_second_bcd;
                /* This cancels to SETLOC; preserve the expression shape for GCC 2.7.2 register allocation. */
                send_cd_command(3 + ((step_result ^ single_step) ^ step_result ^ single_step));
                g_checkps_state = CHECKPS_STATE_WAIT_SETLOC;
            }
            /* fall through */
            case CHECKPS_CD_POLL_PENDING:
                step_result = CHECKPS_STATE_WAIT_GET_ID;
                break;

            case CHECKPS_CD_POLL_SHELL_OPEN:
                send_cd_command(CHECKPS_CD_CMD_NOP);
                g_checkps_state = CHECKPS_STATE_WAIT_RECOVERY_NOP;
                step_result = CHECKPS_CD_POLL_DISK_ERROR;
                break;

            default:
                step_result = CHECKPS_STATE_WAIT_GET_ID;
                break;
            }

            break;

        case CHECKPS_STATE_WAIT_SETLOC:
            step_result = poll_cd_response(CHECKPS_CD_CMD_SETLOC);
            if (step_result == CHECKPS_CD_POLL_DISK_ERROR)
            {
                restart_state_or_vsync = 1;
                g_checkps_state = restart_state_or_vsync;
                step_result = CHECKPS_CD_POLL_DISK_ERROR;
            }
            else
            {
            setloc_poll_result:
                if (step_result < 0)
                {
                    if (step_result == CHECKPS_CD_POLL_SHELL_OPEN)
                    {
                        send_cd_command(CHECKPS_CD_CMD_NOP);
                        g_checkps_state = CHECKPS_STATE_WAIT_RECOVERY_NOP;
                        step_result = CHECKPS_CD_POLL_DISK_ERROR;
                    }
                    else
                    {
                        step_result = CHECKPS_STATE_WAIT_SETLOC;
                    }
                }
                else
                {
                    if (step_result != CHECKPS_CD_POLL_PENDING)
                    {
                        if (step_result == CHECKPS_CD_POLL_COMPLETE)
                        {
                            g_cd_command_parameters[0] = (u8)step_result;
                            send_cd_command(CHECKPS_CD_CMD_SETMODE);
                            g_checkps_state = CHECKPS_STATE_WAIT_SETMODE;
                        }
                    }
                    step_result = CHECKPS_STATE_WAIT_SETLOC;
                }
            }
            break;

        case CHECKPS_STATE_WAIT_SETMODE:
            step_result = poll_cd_response(CHECKPS_CD_CMD_SETMODE);
            if (step_result == CHECKPS_CD_POLL_DISK_ERROR)
            {
                restart_state_or_vsync = 1;
                g_checkps_state = restart_state_or_vsync;
                step_result = CHECKPS_CD_POLL_DISK_ERROR;
            }
            else
            {
            setmode_poll_result:
                if (step_result < 0)
                {
                    if (step_result == CHECKPS_CD_POLL_SHELL_OPEN)
                    {
                        send_cd_command(CHECKPS_CD_CMD_NOP);
                        g_checkps_state = CHECKPS_STATE_WAIT_RECOVERY_NOP;
                        step_result = CHECKPS_CD_POLL_DISK_ERROR;
                    }
                    else
                    {
                        step_result = CHECKPS_STATE_WAIT_SETMODE;
                    }
                }
                else
                {
                    if (step_result != CHECKPS_CD_POLL_PENDING)
                    {
                        if (step_result == CHECKPS_CD_POLL_COMPLETE)
                        {
                            g_checkps_vsync_timestamp = VSync(-1);
                            g_checkps_state = CHECKPS_STATE_SEEK_DELAY;
                        }
                    }
                    step_result = CHECKPS_STATE_WAIT_SETMODE;
                }
            }
            break;

        case CHECKPS_STATE_SEEK_DELAY: /* Delay three VSync intervals before SeekP. */

            restart_state_or_vsync = VSync(-1);
            if ((g_checkps_vsync_timestamp + 3) < restart_state_or_vsync)
            {
                send_cd_command(CHECKPS_CD_CMD_SEEK_P);
                g_checkps_state = CHECKPS_STATE_WAIT_SEEK_P;
            }
            step_result = CHECKPS_STATE_SEEK_DELAY;
            break;

        case CHECKPS_STATE_WAIT_SEEK_P:
            step_result = poll_cd_response(CHECKPS_CD_CMD_SEEK_P);
            if (step_result == CHECKPS_CD_POLL_DISK_ERROR)
            {
                restart_state_or_vsync = 1;
                g_checkps_state = restart_state_or_vsync;
                step_result = CHECKPS_CD_POLL_DISK_ERROR;
            }
            else
            {
            seek_p_poll_result:
                if (step_result < 0)
                {
                    if (step_result == CHECKPS_CD_POLL_SHELL_OPEN)
                    {
                        send_cd_command(CHECKPS_CD_CMD_NOP);
                        g_checkps_state = CHECKPS_STATE_WAIT_RECOVERY_NOP;
                        step_result = CHECKPS_CD_POLL_DISK_ERROR;
                    }
                    else
                    {
                        step_result = CHECKPS_STATE_WAIT_SEEK_P;
                    }
                }
                else
                {
                    if (step_result != CHECKPS_CD_POLL_PENDING)
                    {
                        if (step_result == CHECKPS_CD_POLL_COMPLETE)
                        {
                            send_cd_command(CHECKPS_CD_CMD_MUTE);
                            g_checkps_state = CHECKPS_STATE_WAIT_MUTE;
                        }
                    }
                    step_result = CHECKPS_STATE_WAIT_SEEK_P;
                }
            }
            break;

        case CHECKPS_STATE_WAIT_MUTE:
            step_result = poll_cd_response(CHECKPS_CD_CMD_MUTE);
            if (step_result == CHECKPS_CD_POLL_DISK_ERROR)
            {
                restart_state_or_vsync = 1;
                g_checkps_state = restart_state_or_vsync;
                step_result = CHECKPS_CD_POLL_DISK_ERROR;
            }
            else
            {
            mute_poll_result:
                if (step_result < 0)
                {
                    if (step_result == CHECKPS_CD_POLL_SHELL_OPEN)
                    {
                        send_cd_command(CHECKPS_CD_CMD_NOP);
                        g_checkps_state = CHECKPS_STATE_WAIT_RECOVERY_NOP;
                        step_result = CHECKPS_CD_POLL_DISK_ERROR;
                    }
                    else
                    {
                        step_result = CHECKPS_STATE_WAIT_MUTE;
                    }
                }
                else
                {
                    if (step_result != CHECKPS_CD_POLL_PENDING)
                    {
                        if (step_result == CHECKPS_CD_POLL_COMPLETE)
                        {
                            send_cd_command(CHECKPS_CD_CMD_PLAY);
                            g_checkps_state = CHECKPS_STATE_WAIT_PLAY;
                        }
                    }
                    step_result = CHECKPS_STATE_WAIT_MUTE;
                }
            }
            break;

        case CHECKPS_STATE_WAIT_PLAY:
            step_result = poll_cd_response(CHECKPS_CD_CMD_PLAY);
            if (step_result == CHECKPS_CD_POLL_DISK_ERROR)
            {
                restart_state_or_vsync = 1;
                g_checkps_state = restart_state_or_vsync;
                step_result = CHECKPS_CD_POLL_DISK_ERROR;
            }
            else
            {
            play_poll_result:
                if (step_result < 0)
                {
                    if (step_result == CHECKPS_CD_POLL_SHELL_OPEN)
                    {
                        send_cd_command(CHECKPS_CD_CMD_NOP);
                        g_checkps_state = CHECKPS_STATE_WAIT_RECOVERY_NOP;
                        step_result = CHECKPS_CD_POLL_DISK_ERROR;
                    }
                    else
                    {
                        step_result = CHECKPS_STATE_WAIT_PLAY;
                    }
                }
                else
                {
                    if (step_result != CHECKPS_CD_POLL_PENDING)
                    {
                        if (step_result == CHECKPS_CD_POLL_COMPLETE)
                        {
                            g_cd_command_parameters[0] = 4;
                            send_cd_command(CHECKPS_CD_CMD_TEST_04);
                            g_checkps_state = CHECKPS_STATE_WAIT_TEST_04;
                        }
                    }
                    step_result = CHECKPS_STATE_WAIT_PLAY;
                }
            }
            break;

        case CHECKPS_STATE_WAIT_TEST_04:
            step_result = poll_cd_response(CHECKPS_CD_CMD_TEST_04);
            if (step_result == CHECKPS_CD_POLL_DISK_ERROR)
            {
                restart_state_or_vsync = 1;
                g_checkps_state = restart_state_or_vsync;
                step_result = CHECKPS_CD_POLL_DISK_ERROR;
            }
            else
            {
            test_04_poll_result:
                if (step_result < 0)
                {
                    if (step_result == CHECKPS_CD_POLL_SHELL_OPEN)
                    {
                        send_cd_command(CHECKPS_CD_CMD_NOP);
                        g_checkps_state = CHECKPS_STATE_WAIT_RECOVERY_NOP;
                        step_result = CHECKPS_CD_POLL_DISK_ERROR;
                    }
                    else
                    {
                        step_result = CHECKPS_STATE_WAIT_TEST_04;
                    }
                }
                else
                {
                    if (step_result != CHECKPS_CD_POLL_PENDING)
                    {
                        if (step_result == CHECKPS_CD_POLL_COMPLETE)
                        {
                            g_checkps_vsync_timestamp = VSync(-1);
                            g_checkps_state = CHECKPS_STATE_TEST_05_DELAY;
                        }
                    }
                    step_result = CHECKPS_STATE_WAIT_TEST_04;
                }
            }
            break;

        case CHECKPS_STATE_TEST_05_DELAY: /* Delay 200 VSync intervals before Test(0x05). */
            restart_state_or_vsync = VSync(-1);
            if ((g_checkps_vsync_timestamp + CHECKPS_CD_TEST_DELAY_FRAMES) < restart_state_or_vsync)
            {
                g_cd_command_parameters[0] = 5;
                send_cd_command(CHECKPS_CD_CMD_TEST_05);
                g_checkps_state = CHECKPS_STATE_WAIT_TEST_05;
            }
            step_result = CHECKPS_STATE_TEST_05_DELAY;
            break;

        case CHECKPS_STATE_WAIT_TEST_05:
            step_result = poll_cd_response(CHECKPS_CD_CMD_TEST_05);
            if (step_result == CHECKPS_CD_POLL_DISK_ERROR)
            {
                restart_state_or_vsync = 1;
                g_checkps_state = restart_state_or_vsync;
                step_result = CHECKPS_CD_POLL_DISK_ERROR;
            }
            else
            {
            test_05_poll_result:
                if (step_result < 0)
                {
                    if (step_result == CHECKPS_CD_POLL_SHELL_OPEN)
                    {
                        send_cd_command(CHECKPS_CD_CMD_NOP);
                        g_checkps_state = CHECKPS_STATE_WAIT_RECOVERY_NOP;
                        step_result = CHECKPS_CD_POLL_DISK_ERROR;
                    }
                    else
                    {
                        step_result = CHECKPS_STATE_WAIT_TEST_05;
                    }
                }
                else
                {
                    if (step_result != CHECKPS_CD_POLL_PENDING)
                    {
                        if (step_result == CHECKPS_CD_POLL_COMPLETE)
                        {
                            step_result = CHECKPS_STATE_WAIT_TEST_05;
                        test_05_check_response:
                            if (g_cd_response_payload[0] != 0)
                            {
                                send_cd_command(CHECKPS_CD_CMD_NOP);
                                g_checkps_state = CHECKPS_STATE_WAIT_FAILURE_NOP;
                            }
                            else
                            {
                                g_checkps_vsync_timestamp = VSync(-1);
                                g_checkps_state = CHECKPS_STATE_PAUSE_DELAY;
                            }
                        }
                    }
                    step_result = CHECKPS_STATE_WAIT_TEST_05;
                }
            }
            break;

        case CHECKPS_STATE_WAIT_FAILURE_NOP:
            step_result = poll_cd_response(CHECKPS_CD_CMD_NOP);
            if (step_result == CHECKPS_CD_POLL_DISK_ERROR)
            {
                restart_state_or_vsync = 1;
                g_checkps_state = restart_state_or_vsync;
                step_result = CHECKPS_CD_POLL_DISK_ERROR;
            }
            else
            {
            failure_nop_poll_result:
                if (step_result < 0)
                {
                    if (step_result == CHECKPS_CD_POLL_SHELL_OPEN)
                    {
                        send_cd_command(CHECKPS_CD_CMD_NOP);
                        g_checkps_state = CHECKPS_STATE_WAIT_RECOVERY_NOP;
                        step_result = CHECKPS_CD_POLL_DISK_ERROR;
                    }
                    else
                    {
                        step_result = CHECKPS_STATE_WAIT_FAILURE_NOP;
                    }
                }
                else
                {
                    if (step_result == CHECKPS_CD_POLL_PENDING)
                    {
                        step_result = CHECKPS_STATE_WAIT_FAILURE_NOP + ((step_result & 1) >> 1);
                        break;
                    }
                    if (step_result != CHECKPS_CD_POLL_COMPLETE)
                    {
                        step_result = CHECKPS_STATE_WAIT_FAILURE_NOP;
                        break;
                    }
                    step_result = CHECKPS_STATE_WAIT_FAILURE_NOP;
                    show_hardware_modification_warning_and_exit();
                    break;
                }
            }
            break;
            step_result = CHECKPS_STATE_WAIT_FAILURE_NOP;
            break;

        case CHECKPS_STATE_WAIT_RECOVERY_NOP:
            step_result = poll_cd_response(CHECKPS_CD_CMD_NOP);
            if (step_result == CHECKPS_CD_POLL_DISK_ERROR)
            {
                restart_state_or_vsync = 1;
                g_checkps_state = restart_state_or_vsync;
                step_result = CHECKPS_CD_POLL_DISK_ERROR;
                break;
            }
            else
            {
            recovery_nop_poll_result:
                if (step_result < 0)
                {
                    if (step_result != CHECKPS_CD_POLL_SHELL_OPEN)
                    {
                        step_result = CHECKPS_STATE_WAIT_FAILURE_NOP;
                        break;
                    }
                }
                else
                {
                    switch (step_result)
                    {
                    case CHECKPS_CD_POLL_COMPLETE:
                        g_checkps_state = (u32)step_result;
                        /* fall through */
                    case CHECKPS_CD_POLL_PENDING:
                    default:
                    recovery_nop_finalize:
                        step_result = CHECKPS_STATE_WAIT_FAILURE_NOP + ((step_result & 1) >> 1);
                        break;
                    }
                    break;
                }
            }
        recovery_nop_reset_command:
            send_cd_command(CHECKPS_CD_CMD_NOP);
            step_result = CHECKPS_STATE_WAIT_FAILURE_NOP;
            break;

        case CHECKPS_STATE_WAIT_PAUSE:
            step_result = poll_cd_response(CHECKPS_CD_CMD_PAUSE);
            if (step_result != CHECKPS_CD_POLL_DISK_ERROR)
            {
            pause_poll_result:
                if (step_result < 0)
                {
                    if (step_result != CHECKPS_CD_POLL_SHELL_OPEN)
                    {
                        step_result = CHECKPS_STATE_WAIT_PAUSE;
                    pause_done:
                        break;
                    }
                    send_cd_command(CHECKPS_CD_CMD_NOP);
                    g_checkps_state = CHECKPS_STATE_WAIT_RECOVERY_NOP;
                    step_result = CHECKPS_CD_POLL_DISK_ERROR;
                    break;
                }
                if (step_result == CHECKPS_CD_POLL_PENDING)
                {
                }
                else if (step_result != CHECKPS_CD_POLL_COMPLETE)
                {
                    step_result = CHECKPS_STATE_WAIT_PAUSE;
                    break;
                }
                if (step_result < 0)
                {
                pause_command_error:
                    send_cd_command(CHECKPS_CD_CMD_NOP);
                    g_checkps_state = CHECKPS_STATE_WAIT_RECOVERY_NOP;
                    step_result = CHECKPS_CD_POLL_DISK_ERROR;
                    break;
                }
            }
            else
            {
                restart_state_or_vsync = 1;
                g_checkps_state = restart_state_or_vsync;
                step_result = CHECKPS_CD_POLL_DISK_ERROR;
                break;
            }
            if (step_result != CHECKPS_CD_POLL_PENDING)
                g_checkps_state = CHECKPS_STATE_IDLE;
        state18_finalize:
            step_result = CHECKPS_STATE_WAIT_PAUSE + ((step_result & 1) >> 1);
            break;

        case CHECKPS_STATE_PAUSE_DELAY: /* Delay ten VSync intervals before Pause. */
            restart_state_or_vsync = VSync(-1);
            if ((g_checkps_vsync_timestamp + CHECKPS_CD_PAUSE_DELAY_FRAMES) < restart_state_or_vsync)
            {
                send_cd_command(CHECKPS_CD_CMD_PAUSE);
                g_checkps_state = CHECKPS_STATE_WAIT_PAUSE;
            }
            step_result = CHECKPS_STATE_PAUSE_DELAY;
            break;

        case CHECKPS_STATE_IDLE: /* Idle. */
            step_result = CHECKPS_STATE_IDLE;
            break;
        }

        if ((single_step == 0) && (step_result != CHECKPS_STATE_IDLE))
        {
            continue;
        }
        return step_result;
    recovery_nop_return:
        if ((single_step == 0) && (step_result != CHECKPS_STATE_IDLE))
        {
            continue;
        }
        return step_result;
    }
}

/**
 * @brief Poll and consume the response for a CHECKPS CD command.
 * @param command Command whose response is expected.
 * @return Poll status describing completion, pending state, or hardware error.
 */
CheckPSCdPollResult poll_cd_response(CheckPSCdCommandIndex command)
{
    u8 irq_code_sum_target;
    u8 irq_sample_a;
    u8 irq_sample_b;
    s32 irq_code;
    s32 irq_code_byte;
    s32 delay_counter;
    int stable_irq;
    s32 response_index;
    irq_code_sum_target = g_cd_command_table[command].irq_code_sum_target;
    *g_cd_status_register = 1;
    irq_sample_a = *g_cd_irq_register;
    irq_sample_b = *g_cd_irq_register;
    if ((stable_irq = irq_sample_a & CHECKPS_CD_IRQ_STATUS_MASK) == (irq_sample_b & CHECKPS_CD_IRQ_STATUS_MASK))
    {
        irq_code = stable_irq;
        irq_code_byte = (unsigned char)irq_code;
        if (irq_code_byte != 0)
        {
            g_cd_irq_code_sum = g_cd_irq_code_sum + irq_code_byte;
            *g_cd_status_register = 1;
            *g_cd_irq_register = CHECKPS_CD_IRQ_STATUS_MASK;
            delay_counter = 0;
            /* Preserve the original four address-zero writes used as a short hardware delay. */
            do
            {
                *((int*)0) = delay_counter;
                delay_counter++;
            } while (delay_counter < 4);
            if (g_cd_irq_code_sum >= (s32)irq_code_sum_target)
            {
                g_cd_irq_code_sum = 0;
                if (irq_code == CHECKPS_CD_IRQ_DISK_ERROR)
                {
                    while (1)
                    {
                        g_cd_response.status = *g_cd_response_register;
                        break;
                    }
                    g_cd_response.detail = *g_cd_response_register;
                    *g_cd_status_register = 1;
                    *g_cd_data_register = CHECKPS_CD_IRQ_ACK_MASK;
                    if (!(g_cd_response.status & CHECKPS_CD_STATUS_SHELL_OPEN))
                    {
                        return CHECKPS_CD_POLL_DISK_ERROR;
                    }

                    return CHECKPS_CD_POLL_SHELL_OPEN;
                }
                else
                {
                    irq_code_byte = 0;
                    response_index = irq_code_byte;
                    if (g_cd_command_table[command].response_count != irq_code_byte)
                    {
                        do
                        {
                            ((u8*)(&g_cd_response))[response_index] = *g_cd_response_register;
                            response_index++;
                        } while (response_index < (s32)g_cd_command_table[command].response_count);
                    }
                    *g_cd_status_register = 1;
                    *g_cd_data_register = CHECKPS_CD_IRQ_ACK_MASK;
                    if (command != CHECKPS_CD_CMD_TEST_05)
                    {
                        response_index = 0;
                        while (1)
                        {
                            if (response_index)
                                return CHECKPS_CD_POLL_SHELL_OPEN;
                            response_index = g_cd_response.status;
                            response_index &= CHECKPS_CD_STATUS_SHELL_OPEN;
                            if (!response_index)
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

/**
 * @brief Write a CHECKPS command and its parameters to the CD controller.
 * @param command Command descriptor index to send.
 */
void send_cd_command(CheckPSCdCommandIndex command)
{
    s32 delay_counter = 0;
    s32* address_zero_delay_sink = 0;
    s32 parameter_index;
    unsigned int descriptor_byte_offset;

    *g_cd_status_register = 1;
    *g_cd_irq_register = CHECKPS_CD_IRQ_STATUS_MASK;

    /* The original performs four writes through address zero between CD-register
       updates. Preserve the sequence because it affects the matched instruction stream. */
    for (delay_counter = 0; delay_counter < 4; delay_counter++) *address_zero_delay_sink = delay_counter;

    *g_cd_status_register = 1;
    *g_cd_data_register = CHECKPS_CD_PARAMETER_MODE;
    *g_cd_status_register = 0;

    /* Keep byte indexing through the field pointer: direct table[command] field
       accesses change GCC 2.7.2 register allocation in this matched function. */
    descriptor_byte_offset = command * sizeof(CdCommandDescriptor);

    parameter_index = 0;
    if ((&g_cd_command_table->parameter_count)[descriptor_byte_offset])
    {
        do
        {
            *g_cd_data_register = g_cd_command_parameters[parameter_index];
            parameter_index++;
        } while (parameter_index < (&g_cd_command_table->parameter_count)[descriptor_byte_offset]);
    }

    *g_cd_status_register = 0;
    *g_cd_response_register = (&g_cd_command_table->opcode)[command * 4];
}

/**
 * @brief Display the hardware-modification warning and terminate execution.
 */
void show_hardware_modification_warning_and_exit(void)
{
    DRAWENV draw_env;
    DISPENV disp_env;
    DR_ENV draw_env_packet;
    u32 gpu_commands[3];
    KanjiDrawStateWords text_state;
    s32 text_color;
    s32 line_index;
    ResetGraph(1);
    StopCallback();
    ResetGraph(5);
    *CHECKPS_SPU_CONTROL_REGISTER = 0;
    SetDefDrawEnv(&draw_env, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    SetDefDispEnv(&disp_env, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    draw_env.isbg = 1;
    SetDrawEnv(&draw_env_packet, &draw_env);
    DrawPrim(&draw_env_packet);
    PutDispEnv(&disp_env);
    gpu_commands[0] = CHECKPS_GPU_FILL_RECT_COMMAND;
    gpu_commands[1] = CHECKPS_GPU_MASK_BIT_COMMAND;
    gpu_commands[2] = 0;
    DrawPrim(gpu_commands);
    text_color = CHECKPS_WARNING_PRIMARY_COLOR;

    text_state.width = CHECKPS_WARNING_TEXT_WIDTH;
    text_state.height = 1;

    for (line_index = 0; line_index < CHECKPS_WARNING_LINE_COUNT; line_index++)
    {
        text_state.x = line_index + CHECKPS_WARNING_TEXT_X;
        text_state.y = line_index + CHECKPS_WARNING_TEXT_Y;
        draw_kanji_string((const char*)&g_hardware_modification_warning, (KanjiDrawState*)&text_state, text_color);
        text_color = CHECKPS_WARNING_SHADOW_COLOR;
    }

    draw_hardware_check_pattern();
    SetDispMask(1);
    exit();
}
