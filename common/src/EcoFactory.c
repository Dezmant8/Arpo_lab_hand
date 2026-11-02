#include "EcoObject.h"
#include "LabConstructors.h"

#if ARPO_SERVER == 1
#define create_instance lab1_create
#elif ARPO_SERVER == 2
#define create_instance lab2_create
#else
#error Select the server to build
#endif

typedef struct ModuleFactory {
    IEcoComponentFactory iface;
    uint32_t references;
} ModuleFactory;

static uint32_t ECOCALLMETHOD factory_addref(IEcoComponentFactory* me)
{
    ModuleFactory* factory = (ModuleFactory*)me;

    return factory != 0 ? ++factory->references : 0;
}

static uint32_t ECOCALLMETHOD factory_release(IEcoComponentFactory* me)
{
    ModuleFactory* factory = (ModuleFactory*)me;

    if (factory == 0) return 0;
    if (factory->references != 0) --factory->references;
    return factory->references;
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
    if (result == 0) {
        return ERR_ECO_POINTER;
    }
    *result = 0;
    if (me == 0 || system == 0 || iid == 0) {
        return ERR_ECO_POINTER;
    }
    return create_instance(system, outer, iid, result);
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
    return me != 0 ? "Eco.Lab" : 0;
}

static char_t* ECOCALLMETHOD factory_version(IEcoComponentFactory* me)
{
    return me != 0 ? "1.0.0" : 0;
}

static char_t* ECOCALLMETHOD factory_manufacturer(IEcoComponentFactory* me)
{
    return me != 0 ? "ARPO" : 0;
}

static IEcoComponentFactoryVTbl factory_vtable = {
    factory_query, factory_addref, factory_release,
    factory_allocate, factory_initialize,
    factory_name, factory_version, factory_manufacturer
};

ECO_EXPORT IEcoComponentFactory* ECOCALLMETHOD GetIEcoComponentFactoryPtr(void)
{
    static ModuleFactory factory = {{&factory_vtable}, 1};

    return &factory.iface;
}
