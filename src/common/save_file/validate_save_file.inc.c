/*
 * Shared save-file function; see include/save_file.h. Included by each
 * overlay that has it, at the point where it sits in that overlay's binary.
 */
#include "save_file.h"

/**
 * @brief Check a save file's stored checksum and its "ANA" magic.
 * @param file Save file to check.
 * @return 1 when the checksum and magic both match, otherwise 0.
 */
s32 validate_save_file(SaveFile* file)
{
    if (file->checksum == compute_save_checksum(file))
    {
        if (file->magic == SAVE_FILE_MAGIC)
        {
            return 1;
        }
    }
    return 0;
}
