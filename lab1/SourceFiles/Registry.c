#include "EcoFactory.h"
#include "LabConstructors.h"
#include "LabRegistry.h"

static EcoFactory factory = ECO_FACTORY("Eco.Lab1 Bitonic sort", lab1_create, 0);

uint32_t lab_factory_count(void)
{
    return 1;
}

IEcoComponentFactory* lab_factory_at(uint32_t index, const UGUID** cid)
{
    if (index != 0 || cid == 0) {
        return 0;
    }
    *cid = &CID_EcoLab1;
    return &factory.iface;
}

int16_t lab_register_components(IEcoInterfaceBus1* bus)
{
    if (bus == 0) {
        return ERR_ECO_POINTER;
    }
    return bus->pVTbl->RegisterComponent(bus, &CID_EcoLab1,
        (IEcoUnknown*)&factory.iface);
}
