#include "EcoObject.h"
#include "IEcoInterfaceBus1MemExt.h"
#include "IdEcoMemoryManager1.h"

static uint32_t ECOCALLMETHOD inner_addref(IEcoUnknown* me)
{
    EcoObject* object;

    if (me == 0) {
        return 0;
    }
    object = ECO_OWNER(me, EcoObject, inner);
    if (object->destroying) {
        return 0;
    }
    return ++object->references;
}

static uint32_t ECOCALLMETHOD inner_release(IEcoUnknown* me)
{
    EcoObject* object;
    IEcoSystem1* system;
    IEcoMemoryAllocator1* memory;
    uint32_t remaining;

    if (me == 0) {
        return 0;
    }
    object = ECO_OWNER(me, EcoObject, inner);
    if (object->destroying || object->references == 0) {
        return 0;
    }
    remaining = --object->references;
    if (remaining != 0) {
        return remaining;
    }

    object->destroying = 1;
    object->references = 1;
    system = object->system;
    memory = object->memory;
    object->dispose(object);
    memory->pVTbl->Free(memory, object);
    system->pVTbl->Release(system);
    memory->pVTbl->Release(memory);
    return 0;
}

static int16_t ECOCALLMETHOD inner_query(IEcoUnknown* me,
    const UGUID* iid, void** result)
{
    EcoObject* object;

    if (result == 0) {
        return ERR_ECO_POINTER;
    }
    *result = 0;
    if (me == 0 || iid == 0) {
        return ERR_ECO_POINTER;
    }
    object = ECO_OWNER(me, EcoObject, inner);
    if (object->destroying) {
        return ERR_ECO_FAIL;
    }
    if (IsEqualUGUID(iid, &IID_IEcoUnknown)) {
        *result = &object->inner;
        inner_addref(&object->inner);
        return ERR_ECO_SUCCESS;
    }
    return object->query_views(object, iid, result);
}

static IEcoUnknownVTbl inner_vtable = {
    inner_query, inner_addref, inner_release
};

int16_t eco_check_creation(IEcoUnknown* system, IEcoUnknown* outer,
    const UGUID* iid, void** result, int aggregatable)
{
    if (result == 0) {
        return ERR_ECO_POINTER;
    }
    *result = 0;
    if (system == 0 || iid == 0) {
        return ERR_ECO_POINTER;
    }
    if (outer != 0 && (!aggregatable ||
        !IsEqualUGUID(iid, &IID_IEcoUnknown))) {
        return ERR_ECO_NOAGGREGATION;
    }
    return ERR_ECO_SUCCESS;
}

int16_t eco_object_allocate(IEcoUnknown* system, IEcoUnknown* outer,
    uint32_t size, EcoQueryViews query, EcoDispose dispose,
    EcoObject** result, IEcoInterfaceBus1** bus_result)
{
    IEcoSystem1* runtime = 0;
    IEcoInterfaceBus1* bus = 0;
    IEcoInterfaceBus1MemExt* extension = 0;
    IEcoMemoryAllocator1* memory = 0;
    const UGUID* selected;
    UGUID manager = CID_EcoMemoryManager1;
    EcoObject* object;
    int16_t status;

    *result = 0;
    if (bus_result != 0) {
        *bus_result = 0;
    }
    status = system->pVTbl->QueryInterface(system, &GID_IEcoSystem,
        (void**)&runtime);
    if (status != ERR_ECO_SUCCESS || runtime == 0) {
        status = ERR_ECO_NOSYSTEM;
        goto cleanup;
    }
    status = runtime->pVTbl->QueryInterface(runtime, &IID_IEcoInterfaceBus1,
        (void**)&bus);
    if (status != ERR_ECO_SUCCESS || bus == 0) {
        status = ERR_ECO_NOBUS;
        goto cleanup;
    }
    status = bus->pVTbl->QueryInterface(bus, &IID_IEcoInterfaceBus1MemExt,
        (void**)&extension);
    if (status == ERR_ECO_SUCCESS && extension != 0) {
        selected = extension->pVTbl->get_Manager(extension);
        if (selected == 0) {
            status = ERR_ECO_GET_MEMORY_ALLOCATOR;
            goto cleanup;
        }
        manager = *selected;
    }
    else if (status != ERR_ECO_NOINTERFACE) {
        status = ERR_ECO_GET_MEMORY_ALLOCATOR;
        goto cleanup;
    }
    if (extension != 0) {
        extension->pVTbl->Release(extension);
        extension = 0;
    }
    status = bus->pVTbl->QueryComponent(bus, &manager, 0,
        &IID_IEcoMemoryAllocator1, (void**)&memory);
    if (status != ERR_ECO_SUCCESS || memory == 0) {
        status = ERR_ECO_GET_MEMORY_ALLOCATOR;
        goto cleanup;
    }
    object = (EcoObject*)memory->pVTbl->Alloc(memory, size);
    if (object == 0) {
        status = ERR_ECO_OUTOFMEMORY;
        goto cleanup;
    }
    object->inner.pVTbl = &inner_vtable;
    object->controller = outer != 0 ? outer : &object->inner;
    object->references = 1;
    object->destroying = 0;
    object->system = runtime;
    object->memory = memory;
    object->query_views = query;
    object->dispose = dispose;
    runtime = 0;
    memory = 0;
    *result = object;
    if (bus_result != 0) {
        *bus_result = bus;
        bus = 0;
    }
    status = ERR_ECO_SUCCESS;

cleanup:
    if (extension != 0) {
        extension->pVTbl->Release(extension);
    }
    if (bus != 0) {
        bus->pVTbl->Release(bus);
    }
    if (memory != 0) {
        memory->pVTbl->Release(memory);
    }
    if (runtime != 0) {
        runtime->pVTbl->Release(runtime);
    }
    return status;
}

int16_t eco_object_finish(EcoObject* object, const UGUID* iid, void** result)
{
    int16_t status;

    status = inner_query(&object->inner, iid, result);
    inner_release(&object->inner);
    return status;
}

void eco_object_discard(EcoObject* object)
{
    inner_release(&object->inner);
}

int16_t eco_public_query(EcoObject* object, const UGUID* iid, void** result)
{
    if (result == 0) {
        return ERR_ECO_POINTER;
    }
    *result = 0;
    if (object == 0 || iid == 0) {
        return ERR_ECO_POINTER;
    }
    return object->controller->pVTbl->QueryInterface(object->controller,
        iid, result);
}

uint32_t eco_public_addref(EcoObject* object)
{
    return object->controller->pVTbl->AddRef(object->controller);
}

uint32_t eco_public_release(EcoObject* object)
{
    return object->controller->pVTbl->Release(object->controller);
}

int16_t eco_return_view(EcoObject* object, void* view, void** result)
{
    *result = view;
    eco_public_addref(object);
    return ERR_ECO_SUCCESS;
}
