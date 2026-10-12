#ifndef _CARD_CALLBACKS_H
#define _CARD_CALLBACKS_H

#include "common.h"
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

/** @brief Game state exchanged with the PocketStation pet-transfer program; CARDA reads the rewards and growth back. */
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

/** @brief Pet-transfer file: the PocketStation program header followed by the transfer state. */
typedef struct
{
    u8 header[CARD_RESOURCE_HEADER_SIZE];
    CardPetTransfer transfer;
} CardPetResource;

void card_prepare_pet_transfer(u8* resource, struct PetRecord* pet);

#endif
