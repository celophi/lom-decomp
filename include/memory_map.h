#ifndef MEMORY_MAP_H
#define MEMORY_MAP_H

/**
 * @file memory_map.h
 * @brief Fixed memory regions the game addresses directly, rather than through linked symbols.
 *
 * The game keeps its load buffers, overlay work areas and shared state blocks
 * at fixed RAM addresses, and uses the scratchpad as fast work memory. The
 * original code reaches them through constant addresses, and the compiled
 * code depends on it: a linker symbol in their place changes register
 * allocation. Every such address is defined here once; the typed views in
 * other headers and files are built from these names. A build that cannot
 * place memory at the original addresses only has to change the definitions
 * in this file.
 *
 * Values are plain integers, so each view casts to its own type.
 */

/* ---- Scratchpad ---------------------------------------------------------- */

/** @brief The 1 KiB on-chip scratchpad RAM; routines lay out their own work data in it. */
#define SCRATCHPAD_ADDRESS 0x1F800000
/** @brief Size of the scratchpad in bytes. */
#define SCRATCHPAD_SIZE 0x400
/**
 * @brief Address @p offset bytes into the scratchpad.
 * @param offset Byte offset, below SCRATCHPAD_SIZE.
 */
#define SCRATCHPAD_AT(offset) (SCRATCHPAD_ADDRESS + (offset))

/* ---- Main RAM, from low to high ------------------------------------------ */

/** @brief Pointer to the song on the primary sequencer channel (the word g_akao_seq_channel0 names). */
#if defined(VERSION_JP)
#define AKAO_PRIMARY_SONG_ADDRESS 0x8003EDCC
#else
#define AKAO_PRIMARY_SONG_ADDRESS 0x8003EC5C
#endif

/**
 * @brief Upper part of the primary overlay slot.
 * @note FIELD keeps the data page holding g_field_object_states here; CHECKPS gets it as render buffers.
 */
#define PRIMARY_OVERLAY_UPPER_ADDRESS 0x80100000

/** @brief Instrument bank area just below the secondary overlay slot (TITLE, FIELD, CHECKPS). */
#define SOUND_BANK_ADDRESS 0x8013C000

/**
 * @brief Secondary overlay slot: MENU, GOLEM, GNAME, SHOP, MOVIE and the other sub-overlays load here.
 * @note While no sub-overlay is loaded, FIELD uses the slot as its CD buffer and actor heap; the
 *       space after a loaded sub-overlay holds the work buffers passed to its entry point.
 */
#define SECONDARY_OVERLAY_ADDRESS 0x80140000
/**
 * @brief Address @p offset bytes into the secondary overlay slot.
 * @param offset Byte offset, below 0x40000 (the load buffer follows).
 */
#define SECONDARY_OVERLAY_AT(offset) (SECONDARY_OVERLAY_ADDRESS + (offset))

/** @brief Buffer that CD resources are read into before they are unpacked or copied elsewhere. */
#define LOAD_BUFFER_ADDRESS 0x80180000
/**
 * @brief Address @p offset bytes into the load buffer.
 * @param offset Byte offset.
 */
#define LOAD_BUFFER_AT(offset) (LOAD_BUFFER_ADDRESS + (offset))

/** @brief Staging area for streamed CD sectors. */
#define CD_STREAM_STAGING_ADDRESS 0x801DA000
/**
 * @brief Address @p offset bytes into the CD stream staging area.
 * @param offset Byte offset.
 */
#define CD_STREAM_STAGING_AT(offset) (CD_STREAM_STAGING_ADDRESS + (offset))

/** @brief Sector buffer of the CD and audio streams. */
#define CD_STREAM_BUFFER_ADDRESS 0x801DC000
/**
 * @brief Address @p offset bytes into the CD stream buffer.
 * @param offset Byte offset.
 */
#define CD_STREAM_BUFFER_AT(offset) (CD_STREAM_BUFFER_ADDRESS + (offset))

/** @brief FIELD's 4bpp text cache (256 by 96 pixels). */
#define TEXT_CACHE_ADDRESS 0x801DE000

/** @brief FIELD's collision node lists: blocking nodes, then touched nodes at +0x100. */
#define COLLISION_LISTS_ADDRESS 0x801E1000
/**
 * @brief Address @p offset bytes into the collision node lists.
 * @param offset Byte offset.
 */
#define COLLISION_LISTS_AT(offset) (COLLISION_LISTS_ADDRESS + (offset))

/** @brief The field font, loaded by the main executable and read by FIELD's text drawer. */
#define FONT_ADDRESS 0x801E1200
/**
 * @brief Address @p offset bytes into the loaded font.
 * @param offset Byte offset.
 */
#define FONT_AT(offset) (FONT_ADDRESS + (offset))

/* ---- Resident state page (0x801ED000) ------------------------------------ */

/** @brief Page of fixed state blocks shared by the main executable and the overlays. */
#define RESIDENT_STATE_ADDRESS 0x801ED000
/** @brief FIELD's workspace: allocator state and the text system. */
#define FIELD_WORKSPACE_ADDRESS RESIDENT_STATE_ADDRESS
/** @brief Header block of the loaded field map (its size words). */
#define MAP_BOUNDS_ADDRESS (RESIDENT_STATE_ADDRESS + 0x400)
/** @brief Pending text-window configuration. */
#define TEXT_CONFIG_ADDRESS (RESIDENT_STATE_ADDRESS + 0x408)
/** @brief Scene state kept across overlays. */
#define SCENE_STATE_ADDRESS (RESIDENT_STATE_ADDRESS + 0x480)
/** @brief Movie playback and streaming state. */
#define MOVIE_STATE_ADDRESS (RESIDENT_STATE_ADDRESS + 0x500)
/** @brief State of the two controller ports. */
#define CONTROLLER_STATE_ADDRESS (RESIDENT_STATE_ADDRESS + 0x600)
/** @brief CD system state. */
#define CD_SYSTEM_ADDRESS (RESIDENT_STATE_ADDRESS + 0x800)
/** @brief CD resource table entries. */
#define CD_RESOURCE_ENTRIES_ADDRESS (RESIDENT_STATE_ADDRESS + 0x998)

/** @brief Initial stack top handed to the BIOS. */
#define BIOS_STACK_ADDRESS 0x801FFF00

#endif
