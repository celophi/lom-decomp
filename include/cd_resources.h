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

    /* SOUND/EFFECT.SET */
    CD_RES_SOUND_EFFECT_SET = 21,
} CdResourceId;

#endif
