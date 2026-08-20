#ifndef _CD_RESOURCES_H
#define _CD_RESOURCES_H

/**
 * Named indices into the SKCDPOSE.DAT resource table loaded by cdrom_load_resource_table.
 * Each entry maps to a CdResourceEntry (CdlLOC + dataSize) describing a file's
 * location and size on the disc. Pass these to cdrom_queue_read, cdrom_queue_seek,
 * cdrom_get_resource_size, cdrom_stream, etc.
 */
typedef enum CdResourceId
{
    /* SYSTEM.CNF */
    CD_RES_SYSTEM_CNF = 0,

    /* SLUS_010.13 */
    CD_RES_SLUS_010_13 = 1,

    /* BIN/FIELD.BIN */
    CD_RES_FIELD_BIN = 2,

    /* BIN/WMAP.BIN */
    CD_RES_WMAP_BIN = 3,

    /* BIN/TITLE.BIN */
    CD_RES_TITLE_BIN = 4,

    /* BIN/GNAME.BIN */
    CD_RES_GNAME_BIN = 5,

    /* BIN/MENU.BIN */
    CD_RES_MENU_BIN = 6,

    /* BIN/SHOP.BIN */
    CD_RES_SHOP_BIN = 7,

    /* BIN/ZUKAN.BIN */
    CD_RES_ZUKAN_BIN = 8,

    /* BIN/GOLEM.BIN */
    CD_RES_GOLEM_BIN = 9,

    /* BIN/GOVER.BIN */
    CD_RES_GOVER_BIN = 10,

    /* BIN/MOVIE.BIN */
    CD_RES_MOVIE_BIN = 11,

    /* BIN/CARDA.BIN */
    CD_RES_CARDA_BIN = 12,

    /* BIN/GOSUB.BIN */
    CD_RES_GOSUB_BIN = 13,

    /* BIN/WSEL.BIN */
    CD_RES_WSEL_BIN = 14,

    /* BIN/CHECKPS.BIN */
    CD_RES_CHECKPS_BIN = 15,

    /* BIN/CLOAD.BIN */
    CD_RES_CLOAD_BIN = 16,

    /* BIN/NIKI.BIN */
    CD_RES_NIKI_BIN = 17,

    /* BIN/ADDHERO.BIN */
    CD_RES_ADDHERO_BIN = 18,

    /* SOUND/EFFECT.SET */
    CD_RES_SOUND_EFFECT_SET = 21,

    /* SOUND/MSC_DATA.DAT */
    CD_RES_MSC_DATA = 23,

    /* SOUND/MUSIC002.SET */
    CD_RES_MUSIC002_SET = 24,
} CdResourceId;

/**
 * @brief Convert a music-file index to its 16-bit CD resource index.
 * @note Index 0 selects MSC_DATA.DAT; index 1 selects MUSIC002.SET.
 */
#define CD_RES_MUSIC_FILE(file_index) (((file_index) + CD_RES_MSC_DATA) & 0xFFFF)

#endif
