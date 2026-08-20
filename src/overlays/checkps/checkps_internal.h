#ifndef _CHECKPS_INTERNAL_H
#define _CHECKPS_INTERNAL_H

#include "checkps.h"
#include "akao.h"
#include "display.h"
#include "gpu_packet.h"
#include "main.h"
#include "pad.h"
#include "psyq/libgte.h"
#include "psyq/libgpu.h"
#include "psyq/libapi.h"
#include "psyq/memory.h"
#include "psyq/strings.h"

#define MAX_GLYPH_ENTRIES 256
#define CHECKPS_ORDERING_TABLE_LENGTH 0x1000
#define CHECKPS_PRIMITIVE_BUFFER_SIZE 0x4000
#define CHECKPS_SONG_BUFFER_SIZE 0x4000
#define CHECKPS_RESERVED_BSS_WORDS 32769

/* Bit 16 is cleared at frame start and set when a cache slot is emitted. */
#define GLYPH_USED_FLAG 0x10000

/* CHECKPS renderer, packet, and timing constants. */
#define CHECKPS_FRAME_VSYNC_INTERVAL 2
#define CHECKPS_GPU_TAG_LENGTH_MASK 0xFF000000
#define CHECKPS_GPU_TAG_ADDRESS_MASK 0x00FFFFFF
#define CHECKPS_GPU_DRAW_MODE_COMMAND 0xE1000000
#define CHECKPS_GPU_FILL_RECT_COMMAND 0x02000000
#define CHECKPS_GPU_MASK_BIT_COMMAND 0xE6000002
#define CHECKPS_FADE_TILE_CODE 0x62
#define CHECKPS_FADE_ADDITIVE_DRAW_MODE 0x25
#define CHECKPS_FADE_SUBTRACTIVE_DRAW_MODE 0x45
#define CHECKPS_IMAGE_TPAGE 5
#define CHECKPS_GEOMETRY_SCREEN_DISTANCE 1500
#define CHECKPS_FADE_NEUTRAL 0x100
#define CHECKPS_FADE_ADDITIVE_THRESHOLD (CHECKPS_FADE_NEUTRAL + 1)
#define CHECKPS_DEFAULT_FADE_STEPS 20
#define CHECKPS_IMAGE_DISPLAY_FRAMES 120
#define CHECKPS_IMAGE_CLUT_Y 480
#define CHECKPS_GLYPH_VRAM_X 960
#define CHECKPS_GLYPH_VRAM_WIDTH 64
#define CHECKPS_GLYPH_VRAM_HEIGHT 256
#define CHECKPS_GLYPH_CLUT_Y (VRAM_HEIGHT - 1)
#define CHECKPS_GLYPH_WIDTH 16
#define CHECKPS_GLYPH_BITMAP_ROWS 15
#define CHECKPS_GLYPH_VRAM_WORD_WIDTH 4
#define CHECKPS_GLYPH_RASTER_SLOT_SIZE 0x80
#define CHECKPS_GLYPH_V_COORD_MASK 0xF0
#define CHECKPS_GLYPH_CODE_MASK 0xFFFF
#define CHECKPS_GLYPH_SOURCE_MSB 0x80
#define CHECKPS_GLYPH_PACKET_STRIDE 0x14
#define CHECKPS_TEXT_WRAP_LIMIT (SCREEN_WIDTH * 2)
#define CHECKPS_TEXT_FIRST_PRINTABLE 0x20
#define CHECKPS_TEXT_SPACE 0x20
#define CHECKPS_SJIS_LEAD_BYTE_THRESHOLD 0x80
#define CHECKPS_SJIS_FULLWIDTH_ZERO 0x4F82
#define CHECKPS_SJIS_MINUS 0x5B81
#define CHECKPS_ASCII_TO_SJIS_BIAS 0x7AE1
#define CHECKPS_TEXT_DRAW_MODE_COMMAND 0xE100000F

/* Kanji renderer layout and packet constants. */
#define CHECKPS_KANJI_LINE_HEIGHT 18
#define CHECKPS_KANJI_ADVANCE 17
#define CHECKPS_KANJI_PIXELS_PER_ROW 16
#define CHECKPS_KANJI_GPU_TAG 0x0B000000
#define CHECKPS_KANJI_GPU_LOAD_IMAGE 0xA0000000

/* Hardware-modification warning layout. */
#define CHECKPS_PATTERN_RING_COUNT 16
#define CHECKPS_PATTERN_QUADRANT_COUNT 4
#define CHECKPS_PATTERN_GPU_TAG 0x05000000
#define CHECKPS_PATTERN_GPU_POLY_F4 0x280000FF
#define CHECKPS_WARNING_LINE_COUNT 2
#define CHECKPS_WARNING_TEXT_X 0x50
#define CHECKPS_WARNING_TEXT_Y 0x5C
#define CHECKPS_WARNING_TEXT_WIDTH 16
#define CHECKPS_WARNING_PRIMARY_COLOR 0xFFFF
#define CHECKPS_WARNING_SHADOW_COLOR 0x8000
#define CHECKPS_SPU_CONTROL_REGISTER ((s16*)0x1F801DAA)

/* CHECKPS audio resources and fixed work areas. */
#define CHECKPS_SEQ_RESOURCE_BASE 0x17
#define CHECKPS_AUDIO_BANK_ADDRESS ((AkaoSeqHeader*)0x8013C000)
#define CHECKPS_AUDIO_WORK_ADDRESS ((u8*)0x80180000)
#define CHECKPS_AUDIO_BANK_RESIDENT_STATE 6

/* CD controller polling and state-machine timing. */
/* These mirror CdlDiskError/CdlStatShellOpen; libcd.h is not GCC 2.7.2-clean. */
#define CHECKPS_CD_IRQ_DISK_ERROR 5
#define CHECKPS_CD_STATUS_SHELL_OPEN 0x10
#define CHECKPS_CD_TEST_DELAY_FRAMES 200
#define CHECKPS_CD_PAUSE_DELAY_FRAMES 10
#define CHECKPS_CD_IRQ_ACK_MASK 0x1F
#define CHECKPS_CD_PARAMETER_MODE 0x18
#define CHECKPS_CD_IRQ_STATUS_MASK 7
#define CHECKPS_POINTER_IDENTITY_MAGIC 0x88888889U

/* Controller normalization and key-repeat behavior. */
#define CHECKPS_CONTROLLER_UNAVAILABLE 0xFE
#define CHECKPS_INITIAL_REPEAT_DELAY 15
#define CHECKPS_REPEAT_DELAY 2
#define CHECKPS_DPAD_MASK (PAD_BTN_UP | PAD_BTN_RIGHT | PAD_BTN_DOWN | PAD_BTN_LEFT)
#define CHECKPS_NON_REPEAT_BUTTON_MASK \
    (PAD_BTN_L2 | PAD_BTN_R2 | PAD_BTN_L1 | PAD_BTN_R1 | PAD_BTN_CROSS | PAD_BTN_CIRCLE | PAD_BTN_SELECT | PAD_BTN_L3 | PAD_BTN_START)

typedef enum
{
    CHECKPS_EXIT_NONE = 0,
    CHECKPS_EXIT_IMAGE_TIMEOUT = 2,
} CheckPSExitReason;

typedef struct
{
    s32 red;
    s32 green;
    s32 blue;
    s32 steps;
} FadeColor;

/* First two named bytes of the CD response area. */
typedef struct
{
    u8 status;
    u8 detail;
} CdResponsePrefix;

typedef union
{
    u32 raw;
    struct
    {
        u16 character_code;
        struct
        {
            u16 used_this_frame : 1;
            u16 reserved : 15;
        } flags;
    } data;
} GlyphCacheEntry;

typedef struct
{
    u8 opcode;
    u8 parameter_count;
    u8 response_count;
    u8 irq_code_sum_target;
} CdCommandDescriptor;

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

typedef enum
{
    CHECKPS_CD_POLL_SHELL_OPEN = -2,
    CHECKPS_CD_POLL_DISK_ERROR = -1,
    CHECKPS_CD_POLL_PENDING = 0,
    CHECKPS_CD_POLL_COMPLETE = 1,
} CheckPSCdPollResult;

typedef struct
{
    u16 first_glyph;
    u16 second_glyph;
    s16 terminator;
} EncodedGlyphPair;

/* On-disc blocks inside the embedded TIM-format CHECKPS image. */
typedef struct
{
    s16 x;
    s16 y;
    u16 w;
    u16 h;
} CheckPSTimRect;

typedef struct
{
    u32 size;
    CheckPSTimRect rect;
    u_long data[1];
} CheckPSTimBlock;

typedef struct
{
    u32 magic;
    u32 mode;
    CheckPSTimBlock clut;
} CheckPSTimAsset;

/* Same eight-byte layout as KanjiDrawState, expressed as halfword fields. */
typedef struct
{
    s16 x;
    s16 y;
    s16 width;
    s16 height;
} KanjiDrawStateWords;

typedef struct
{
    union
    {
        struct
        {
            s16 x;
            s16 y;
        } coord;
        s32 packed;
    } position;
    u32 packed_size;
} KanjiDrawState;

typedef SPRT_16 GlyphSpritePacket;

typedef struct
{
    DISPENV disp;
    DRAWENV draw;
    RECT clear_rect;
} CheckPSDisplayBuffer;

typedef struct
{
    u8 reserved_header[0x40];
    u_long ordering_table[CHECKPS_ORDERING_TABLE_LENGTH];
    CheckPSDisplayBuffer display;
    u8 primitive_buffer[CHECKPS_PRIMITIVE_BUFFER_SIZE];
    void* primitive_cursor;
    u8 reserved_tail[0x3C10];
} CheckPSFrame;

struct CheckPSRenderState
{
    CheckPSFrame frames[2];
};

/* Hardware-failure screen data. */
extern u8 g_hardware_pattern_size_table[][2];
extern const u32 g_hardware_modification_warning[15];

/* Embedded/generated CHECKPS assets. */
extern u32 g_embedded_checkps_akao;
extern CheckPSTimAsset g_checkps_image_asset;
extern u16 g_decimal_glyph_table[];
extern u16 g_hex_glyph_table[];

/* CD-ROM integrity-check state machine globals. */
extern s32 g_checkps_state;
extern s32 g_checkps_vsync_timestamp;
extern s32 g_cd_last_track_bcd;
extern u8 g_cd_seek_position_bcd[8];
extern const s32 g_checkps_unused_constant17;
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
extern u8 g_controller_device_type;

void run_checkps_display_loop(CheckPSRenderState* render_state);
void init_checkps_display(CheckPSRenderState* render_state);
void load_embedded_checkps_audio(void);
void load_checkps_song_from_disc(s32 song_index);
void stop_checkps_song(void);
void play_loaded_checkps_song(void);
void play_checkps_sfx(u32 sound_id, u32 volume, u32 pan);
void reset_fade_state(void);
void update_and_draw_fade(CheckPSFrame* frame);
void set_fade_target(s32 red, s32 green, s32 blue, s32 steps);
void update_checkps_input_and_timeout(void);
void draw_checkps_image(CheckPSFrame* frame);
void load_checkps_image(void);
s32 poll_input_device(void);
void process_controller_input(void);
void update_controller_input(void);

void* draw_signed_decimal(void* primitive, u_long* ot_tag, s32 value, s32 x, s32 y, s32 palette, s32 alignment);
void draw_hex_byte(void* primitive, u_long* ot_tag, s32 value, s32 x, s32 y, s32 alignment);
void* draw_cached_text(void* primitive, u_long* ot_tag, const void* text, s32 x, s32 y, s32 palette, s32 alignment);
void* render_cached_glyph(void* primitive, u_long* ot_tag, s32 character_code, s32 palette);
GlyphSpritePacket* emit_glyph_sprite(GlyphSpritePacket* packet, u_long* ot_tag, s32 cache_slot);
void begin_glyph_cache_frame(void);
void evict_unused_glyphs(void);
void reset_glyph_renderer(void);

void draw_kanji_string(const char* text, KanjiDrawState* draw_state, s32 color);
void draw_kanji_glyph(KanjiDrawState* draw_state, u8* bitmap, s32 color);
void draw_hardware_check_pattern(void);

void start_cd_integrity_check(void);
s32 run_cd_integrity_check(s32 single_step);
CheckPSCdPollResult poll_cd_response(CheckPSCdCommandIndex command);
void send_cd_command(CheckPSCdCommandIndex command);
void show_hardware_modification_warning_and_exit(void);

#endif
