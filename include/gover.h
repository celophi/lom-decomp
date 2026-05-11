#ifndef _GOVER_H
#define _GOVER_H

#include "common.h"
#include "akao.h"
#include "display.h"
#include "psyq/libgte.h"
#include "psyq/libgpu.h"
#include "psyq/libetc.h"

/**
 * @brief Header for the CLUT (palette) section of a CD image file.
 *
 * @details First structure in the custom binary image format read from CD.
 * Contains the CLUT dimensions and inline palette data, followed at a
 * variable offset by a PixelDataHeader. The @p size field is self-relative:
 * adding its value to its own address yields the start of the PixelDataHeader.
 *
 * Binary layout:
 *   0x00  _pad0[8]   - unknown
 *   0x08  size       - byte distance from &size to the PixelDataHeader
 *   0x0C  _pad1[4]   - unknown
 *   0x10  width      - CLUT entries per row
 *   0x12  height     - CLUT rows (width * height = total palette entries)
 *   0x14  clutData   - inline CLUT pixel data begins here
 */
typedef struct {
    u8     _pad0[8];
    u32    size;
    u8     _pad1[4];
    u16    width;
    u16    height;
    u_long clutData;
} ClutSectionHeader;

/**
 * @brief Header for the pixel data section of a CD image file.
 *
 * @details Located immediately after the inline CLUT data in the binary
 * image buffer. Found at runtime via ClutSectionHeader::size. Contains
 * the pixel data dimensions and inline pixel data.
 *
 * Binary layout:
 *   0x00  _pad0[8]   - unknown
 *   0x08  w          - image width in 16bpp VRAM texels
 *   0x0A  h          - image height in pixels
 *   0x0C  data       - inline pixel data begins here
 */
typedef struct {
    u8     _pad0[8];
    u16    w;
    u16    h;
    u_long data;
} PixelDataHeader;

/**
 * @brief VRAM destination coordinates for a two-section CD image upload.
 *
 * @details Passed to gover_upload_image_to_vram to specify where in VRAM to place
 * the CLUT palette strip and the pixel data independently. Overlaid on a PSX
 * RECT at the call site (x/y map to pixelX/Y; w/h map to clutX/Y).
 */
typedef struct {
    u16 pixelX;
    u16 pixelY;
    u16 clutX;
    u16 clutY;
} VramDstCoords;

typedef struct
{
  u32 unk0;
  u32 unk4;
  u32 unk8;
  u8 unk12[1];
} D_80119F00_t;

/**
 * @brief One half of the Game Over screen's double-buffered frame.
 *
 * @details The Game Over overlay maintains two of these halves back-to-back as
 * one contiguous buffer (total 0x938 bytes). The two halves are flipped each
 * frame in @p gover_run — one is presented while the other is drawn into.
 *
 * The buffer is split across two C symbols for historical layout reasons:
 *
 *   g_goverFrameHeader — anchors @p halves[0] (struct start)
 *   g_goverFrameTail   — equals @c &halves[0].vramRect (i.e. g_goverFrameHeader + 0x90)
 *
 * Field offsets within each half:
 *
 *   0x000  otag         — ordering-table linked-list head (8 entries x 4 bytes)
 *   0x020  disp         — DISPENV configured by SetDefDispEnv
 *   0x034  draw         — DRAWENV configured by SetDefDrawEnv
 *   0x090  vramRect     — VRAM display rect (x, y, w, h)
 *   0x098  primBuf      — scratch space for per-frame GPU primitives
 *   0x498  allocCursor  — next-primitive write pointer (reset to &primBuf each
 *                         frame by gover_run, advanced by gover_build_otag)
 *   0x49C  (size)
 */
typedef struct GoverFrameHalf {
    u8       otag[0x20];
    DISPENV  disp;
    DRAWENV  draw;
    RECT     vramRect;
    u8       primBuf[0x400];
    u8*      allocCursor;
} GoverFrameHalf;


extern void akao_cmd_c0(undefined4 param_1, u_int param_2);
extern void akao_cmd_f0(void);
extern void akao_cmd_f1(void);
extern void akao_cmd_a8(s32);
extern s32 func_800A368C(s32, s32);
extern s32 func_800A380C(void);
extern s32 func_800A39A8(s32, s32, s32, s32);





extern s32 D_8011588C;
extern s32 D_80122988;
extern u32 D_8003EC90;
extern s32 D_8010D018;
extern D_80119F00_t g_audioData;

extern void cdrom_queue_read(s32 resourceIndex, void* dstBuffer);

void gover_show_screen(s32 cdLoadAddr, s32 imageResourceIndex, s32 musicResourceIndex, s32 audioClipIndex);

#endif