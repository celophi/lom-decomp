#include "main/card_callbacks.h"
#include "common/saved_game.h"

/** @brief Bytes of resource header in front of the transfer state. */
#define CARD_RESOURCE_HEADER_SIZE 768
/** @brief Reward slots in the transfer state. */
#define CARD_REWARD_COUNT 50
/** @brief First land that records its maxed spirits; lower lands are only counted. */
#define CARD_FIRST_SPIRIT_LAND 7
/** @brief Last land the transfer records. */
#define CARD_LAST_LAND 31
/** @brief Lands CARD_FIRST_SPIRIT_LAND to CARD_LAST_LAND, less the five unused ids. */
#define CARD_SPIRIT_LAND_COUNT 20

/** @brief Initial game state passed to the pet-transfer resource. */
typedef struct
{
    s32 growth_delta;
    u32 reward_count;
    u8 rewards[CARD_REWARD_COUNT];
    u8 reserved[2];
    u8 egg_species;
    u8 level;
    /** @brief Enabled lands below CARD_FIRST_SPIRIT_LAND. */
    u8 early_land_count;
    /** @brief Enabled lands in all. */
    u8 land_count;
    /** @brief One bit per recorded land, in land order; set when the land is enabled. */
    s32 land_bits;
    /** @brief Per land from CARD_FIRST_SPIRIT_LAND: bit n set when spirit n is at FIELD_LAND_SPIRIT_MAX. */
    u8 maxed_spirits[CARD_SPIRIT_LAND_COUNT];
    PetRecord pet;
} CardPetTransfer;

/** @brief Resource header followed by its pet-transfer state. */
typedef struct
{
    u8 header[CARD_RESOURCE_HEADER_SIZE];
    CardPetTransfer transfer;
} CardPetResource;

/**
 * @brief Fill the PocketStation pet-transfer resource: the pet record, cleared
 *        rewards, and which lands are enabled and have maxed spirits.
 * @param resource Loaded CARD resource containing the transfer state.
 * @param pet Pet to copy into the resource.
 * @see decomp.me (100%) https://decomp.me/scratch/LO4aD
 * @note The US release leaves the resource unchanged.
 */
void card_prepare_pet_transfer(u8* resource, PetRecord* pet)
{
#if defined(VERSION_JP)
    CardPetTransfer* transfer;
    u8* destination;
    u8* source;
    u8* maxed_spirits;
    u8* level;
    s32 i;
    u32 land_id;
    s32 land_bit;
    s32 land_bits;
    u8 early_land_count;
    u8 land_count;
    u8 enabled;
    FieldLandRecord* land;
    u32 spirit_mask;

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

    maxed_spirits = transfer->maxed_spirits;
    land_bit = 1;
    land_bits = 0;
    land_id = 1;
    early_land_count = 0;
    land_count = 0;
    do
    {
        if (land_id == FIELD_LAND_UNUSED_14 || land_id == FIELD_LAND_UNUSED_20 || land_id == FIELD_LAND_UNUSED_22 || land_id == FIELD_LAND_UNUSED_28 ||
            land_id == FIELD_LAND_UNUSED_29)
        {
            continue;
        }

        enabled = 0;
        if (land_id == FIELD_LAND_LUCEMIA)
        {
            land = &g_saved_game.layout.lands[FIELD_LAND_LUCEMIA_REPLACEMENT];
            if (land->flags & FIELD_LAND_FLAG_04)
            {
                enabled = 1;
            }
            else
            {
                land = &g_saved_game.layout.lands[FIELD_LAND_LUCEMIA];
                if (land->flags & FIELD_LAND_FLAG_02)
                {
                    enabled = 1;
                }
            }
        }
        else
        {
            if (land_id == FIELD_LAND_UNNAMED_06)
            {
                land = &g_saved_game.layout.lands[FIELD_LAND_ORCHARDS];
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
            if (land_id < CARD_FIRST_SPIRIT_LAND)
            {
                early_land_count++;
            }
        }
        land_bit <<= 1;

        if (land_id >= CARD_FIRST_SPIRIT_LAND)
        {
            /* Each spirit's bit enters at the top and shifts down, so spirit n ends in bit n. */
            spirit_mask = 0;
            if (enabled)
            {
                level = land->levels;
                for (i = FIELD_LAND_SPIRIT_COUNT - 1; i != -1; i--)
                {
                    if (*level++ == FIELD_LAND_SPIRIT_MAX)
                    {
                        spirit_mask |= 1 << FIELD_LAND_SPIRIT_COUNT;
                    }
                    spirit_mask >>= 1;
                }
            }
            *maxed_spirits++ = spirit_mask;
        }
    } while (land_id++ < CARD_LAST_LAND);

    transfer->early_land_count = early_land_count;
    transfer->land_count = land_count;
    transfer->land_bits = land_bits;
#endif
}
