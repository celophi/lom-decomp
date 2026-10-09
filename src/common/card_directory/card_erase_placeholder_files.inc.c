#include "common/card_directory.h"
#include "common/card_menu.h"

/**
 * @brief Remove both placeholder save filenames from the active card.
 */
inline void card_erase_placeholder_files(void)
{
    CardFilePath card_path;

    strcpy(card_path.text, CARD_DEVICE_PREFIX);
    card_path.device.characters.slot += g_card_slot;
    strcat(card_path.text, g_lom_save_dummy_filename);
    erase(card_path.text);

    strcpy(card_path.text, CARD_DEVICE_PREFIX);
    card_path.device.characters.slot += g_card_slot;
    strcat(card_path.text, g_lom_pocketstation_dummy_filename);
    erase(card_path.text);
}
