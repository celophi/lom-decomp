#include "common.h"

/**
 * @brief Overlay header word for GOSUB.BIN.
 * @details Every overlay begins with a 4-byte module number at its load
 * address; compiled code and data follow at offset 4. Nothing at runtime reads
 * it. This translation unit holds only this word and is always linked first so
 * the word lands at the load address without disturbing the alignment of the
 * rodata that follows. See docs/OVERLAY_HEADER_WORD.md.
 */
const s32 g_gosub_overlay_id = 11;
