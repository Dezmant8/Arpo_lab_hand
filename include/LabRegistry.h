#ifndef ARPO_LAB_REGISTRY_H
#define ARPO_LAB_REGISTRY_H

#include "IEcoInterfaceBus1.h"
#include "LabIds.h"

int16_t lab_register_components(IEcoInterfaceBus1* bus);
uint32_t lab_factory_count(void);
IEcoComponentFactory* lab_factory_at(uint32_t index, const UGUID** cid);

#endif
