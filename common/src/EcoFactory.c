#include "EcoFactory.h"

static uint32_t ECOCALLMETHOD factory_addref(IEcoComponentFactory* me)
{
    EcoFactory* factory;

    if (me == 0) {
        return 0;
    }
    factory = ECO_OWNER(me, EcoFactory, iface);
    return ++factory->references;
}

static uint32_t ECOCALLMETHOD factory_release(IEcoComponentFactory* me)
{
    EcoFactory* factory;

    if (me == 0) {
        return 0;
    }
    factory = ECO_OWNER(me, EcoFactory, iface);
    if (factory->references == 0) {
        return 0;
    }
    return --factory->references;
}

static int16_t ECOCALLMETHOD factory_query(IEcoComponentFactory* me,
    const UGUID* iid, void** result)
{
    if (result == 0) {
        return ERR_ECO_POINTER;
    }
    *result = 0;
    if (me == 0 || iid == 0) {
        return ERR_ECO_POINTER;
    }
    if (!IsEqualUGUID(iid, &IID_IEcoUnknown) &&
        !IsEqualUGUID(iid, &IID_IEcoComponentFactory)) {
        return ERR_ECO_NOINTERFACE;
    }
    *result = me;
    factory_addref(me);
    return ERR_ECO_SUCCESS;
}

static int16_t ECOCALLMETHOD factory_allocate(IEcoComponentFactory* me,
    IEcoUnknown* system, IEcoUnknown* outer, const UGUID* iid, void** result)
{
    EcoFactory* factory;

    if (result == 0) {
        return ERR_ECO_POINTER;
    }
    *result = 0;
    if (me == 0 || system == 0 || iid == 0) {
        return ERR_ECO_POINTER;
    }
    factory = ECO_OWNER(me, EcoFactory, iface);
    return factory->create(system, outer, iid, result, factory->configuration);
}

static int16_t ECOCALLMETHOD factory_initialize(IEcoComponentFactory* me,
    IEcoUnknown* system, void* instance)
{
    if (me == 0 || system == 0 || instance == 0) {
        return ERR_ECO_POINTER;
    }
    return ERR_ECO_SUCCESS;
}

static char_t* ECOCALLMETHOD factory_name(IEcoComponentFactory* me)
{
    return me != 0 ? ECO_OWNER(me, EcoFactory, iface)->name : 0;
}

static char_t* ECOCALLMETHOD factory_version(IEcoComponentFactory* me)
{
    return me != 0 ? "1.0.0" : 0;
}

static char_t* ECOCALLMETHOD factory_manufacturer(IEcoComponentFactory* me)
{
    return me != 0 ? "ARPO" : 0;
}

IEcoComponentFactoryVTbl eco_factory_vtable = {
    factory_query, factory_addref, factory_release,
    factory_allocate, factory_initialize,
    factory_name, factory_version, factory_manufacturer
};
