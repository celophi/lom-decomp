#ifndef _CARD_CALLBACKS_H
#define _CARD_CALLBACKS_H

#include "common.h"

struct PetRecord;

void card_resource_noop_hook(u8* resource, struct PetRecord* pet);

#endif
