#include "card_callbacks.h"
#include "saved_game.h"

#if defined(VERSION_JP)
#define CARD_RESOURCE_HEADER_SIZE 768
#define CARD_REWARD_COUNT 50
#define CARD_LAND_MASK_COUNT 20
#define CARD_FIRST_MANA_LAND 7
#define CARD_LAST_LAND 31
#define CARD_MAX_MANA_LEVEL 6

/** @brief Initial game state passed to the pet-transfer resource. */
typedef struct
{
    s32 growth_delta;
    u32 reward_count;
    u8 rewards[CARD_REWARD_COUNT];
    u8 reserved[2];
    u8 egg_species;
    u8 level;
    u8 early_land_count;
    u8 land_count;
    s32 land_bits;
    u8 mana_masks[CARD_LAND_MASK_COUNT];
    PetRecord pet;
} CardPetTransfer;

/** @brief Resource header followed by its pet-transfer state. */
typedef struct
{
    u8 header[CARD_RESOURCE_HEADER_SIZE];
    CardPetTransfer transfer;
} CardPetResource;

#endif

/**
 * @brief Initialize the pet-transfer resource for the Japanese release.
 * @param resource Loaded CARD resource containing the transfer state.
 * @param pet Pet to copy into the resource.
 * @see decomp.me (100%) https://decomp.me/scratch/LO4aD
 * @note The US release leaves the resource unchanged.
 */
void card_resource_noop_hook(u8* resource, PetRecord* pet)
{
#if defined(VERSION_JP)
    CardPetTransfer* transfer;
    u8* destination;
    u8* source;
    u8* mana_masks;
    u8* mana;
    s32 i;
    u32 land_id;
    s32 land_bit;
    s32 land_bits;
    u8 early_land_count;
    u8 land_count;
    u8 enabled;
    FieldLandRecord* land;
    u32 mana_mask;

    transfer = &((CardPetResource*)resource)->transfer;
    transfer->egg_species = pet->egg_species;
    transfer->level = pet->progress.level;
    transfer->growth_delta = 0;
    transfer->reward_count = 0;
    destination = transfer->rewards;
    for (i = CARD_REWARD_COUNT - 1; i != -1; i--)
    {
        *destination++ = 0;
    }
    destination = (u8*)&transfer->pet;
    source = (u8*)pet;
    for (i = sizeof(PetRecord) - 1; i != -1; i--)
    {
        *destination++ = *source++;
    }
    mana_masks = transfer->mana_masks;
    land_bit = 1;
    land_bits = 0;
    land_id = 1;
    early_land_count = 0;
    land_count = 0;
    do
    {
        if (land_id == 14 || land_id == 20 || land_id == 22 || land_id == 28 || land_id == 29)
        {
            continue;
        }
        enabled = 0;
        if (land_id == 24)
        {
            land = &g_saved_game.layout.lands[33];
            if (land->flags & FIELD_LAND_FLAG_04)
            {
                enabled = 1;
            }
            else
            {
                land = &g_saved_game.layout.lands[24];
                if (land->flags & FIELD_LAND_FLAG_02)
                {
                    enabled = 1;
                }
            }
        }
        else
        {
            if (land_id == 6)
            {
                land = &g_saved_game.layout.lands[32];
            }
            else
            {
                land = &g_saved_game.layout.lands[land_id];
            }
            if (land->flags & FIELD_LAND_FLAG_02)
            {
                enabled = 1;
            }
        }
        if (enabled)
        {
            land_bits |= land_bit;
            land_count++;
            if (land_id < CARD_FIRST_MANA_LAND)
            {
                early_land_count++;
            }
        }
        land_bit <<= 1;
        if (land_id >= CARD_FIRST_MANA_LAND)
        {
            mana_mask = 0;
            if (enabled)
            {
                mana = land->levels;
                for (i = sizeof(land->levels) - 1; i != -1; i--)
                {
                    if (*mana++ == CARD_MAX_MANA_LEVEL)
                    {
                        mana_mask |= 1U << (sizeof(land->levels) / sizeof(land->levels[0]));
                    }
                    mana_mask >>= 1;
                }
            }
            *mana_masks++ = mana_mask;
        }
    } while (land_id++ < CARD_LAST_LAND);
    transfer->early_land_count = early_land_count;
    transfer->land_count = land_count;
    transfer->land_bits = land_bits;
#endif
}
