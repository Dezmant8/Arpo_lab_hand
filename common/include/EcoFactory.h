#ifndef ARPO_ECO_FACTORY_H
#define ARPO_ECO_FACTORY_H

#include "EcoObject.h"

typedef int16_t (*EcoConstructor)(IEcoUnknown* system, IEcoUnknown* outer,
    const UGUID* iid, void** result, uint32_t configuration);

typedef struct EcoFactory {
    IEcoComponentFactory iface;
    uint32_t references;
    char_t* name;
    EcoConstructor create;
    uint32_t configuration;
} EcoFactory;

extern IEcoComponentFactoryVTbl eco_factory_vtable;

#define ECO_FACTORY(name, create, configuration) \
    {{&eco_factory_vtable}, 1, name, create, configuration}

#endif
