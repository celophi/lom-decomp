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

/**
 * @brief Builds the per-frame GPU primitive list for the Game Over screen.
 *
 * @details Called every frame from gover_run. Advances @p g_fadeLevel by
 * @p g_fadeStep and inserts four primitives into the ordering table at
 * @p pOtBuf in back-to-front order:
 *
 *   1. DR_TPAGE  — selects texture page 0xA7 (8bpp, VRAM X=448)
 *   2. SPRT      — right half of the image (64x224) at screen X=256
 *   3. DR_TPAGE  — selects texture page 0xA5 (8bpp, VRAM X=320)
 *   4. SPRT      — left half of the image (256x224) at screen X=0
 *
 * Both sprites share the CLUT at VRAM (0, GOVER_CLUT_Y) and are color-modulated by
 * @p g_fadeLevel (0 = black, 0x80 = full brightness), producing the fade-in
 * and fade-out effect. Primitives are allocated from the buffer region starting
 * at @p pOtBuf+0x98, with the allocation cursor stored at @p pOtBuf+0x498.
 *
 * @param pOtBuf  Pointer to the double-buffered OTag buffer. Serves as both
 *                the OTag[0] linked-list head and the container for the
 *                primitive allocation cursor at offset 0x498.
 *
 * @see decomp.me (97.86%) https://decomp.me/scratch/q3LKi
 */
void gover_build_otag(void* pOtBuf);

extern void gover_load_image_from_cd(s32 cdResourceIndex, VramDstCoords* coordinates, u32 ramBuffer);

/**
 * @brief Uploads a CLUT palette and pixel data from a CD image buffer into VRAM.
 *
 * @details Performs two LoadImage calls against a custom binary image format
 * read from CD. The first uploads the CLUT palette as a single-row strip at
 * the coordinates given by @p coordinates->clutX/Y. The second uploads the
 * pixel data at @p coordinates->pixelX/Y using dimensions from the
 * PixelDataHeader, which is located at a variable byte offset inside the
 * ClutSectionHeader (stored in ClutSectionHeader::size, measured from the
 * address of that field itself).
 *
 * @param header      Pointer to the ClutSectionHeader at the start of the
 *                    CD image buffer.
 * @param coordinates VRAM destination coordinates for both the CLUT and pixel
 *                    data sections.
 * @return            Pixel data width rounded up to the nearest 64-texel
 *                    texture page boundary (see ALIGN64).
 *
 * @see decomp.me (100%) https://decomp.me/scratch/BEM7D
 */
u32 gover_upload_image_to_vram(ClutSectionHeader* header, VramDstCoords* coordinates);

extern void gover_load_audio_clip(s32 audioClipIndex);
extern s32 D_8011588C;
extern s32 D_80122988;
extern u32 D_8003EC90;
extern s32 D_8010D018;
extern D_80119F00_t g_audioData;

extern void cdrom_queue_read(s32 resourceIndex, void* dstBuffer);

#endif