#ifndef _GOVER_H
#define _GOVER_H

#include "common.h"
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
 * @details Passed to UploadImageDataToVram to specify where in VRAM to place
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


extern void FUN_8002279c(undefined4 param_1, u_int param_2);
extern void FUN_80022aa8(void);
extern void FUN_80022ac8(void);
extern void func_800224D8(s32);
extern s32 func_800A368C(s32, s32);
extern s32 func_800A380C(void);
extern s32 func_800A39A8(s32, s32, s32, s32);
extern s32 func_801401F0(void);
extern s32 LoadImageFromCd(s32, s16*, s32);

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
u32 UploadImageDataToVram(ClutSectionHeader* header, VramDstCoords* coordinates);

extern s32 func_80140648(s32);
extern s32 D_8011588C;
extern s32 D_80140708;
extern s32 D_80141048;
extern u8 D_801407A0[];
extern s32 D_80140710[];
extern s32 D_80122988;
extern u32 D_8003EC90;
extern s32 D_8010D018;
extern D_80119F00_t D_80119F00;
extern s32 D_80180004;
extern u8 D_80180000[];

#endif