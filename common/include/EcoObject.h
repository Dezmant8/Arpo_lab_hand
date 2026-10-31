#ifndef ARPO_ECO_OBJECT_H
#define ARPO_ECO_OBJECT_H

#include <stddef.h>
#include "IEcoSystem1.h"
#include "IEcoInterfaceBus1.h"
#include "IEcoMemoryAllocator1.h"
#include "ErrEcoCodes.h"

#define ECO_OWNER(pointer, type, member) \
    ((type*)((char*)(pointer) - offsetof(type, member)))

typedef struct EcoObject EcoObject;
typedef int16_t (*EcoQueryViews)(EcoObject* object, const UGUID* iid,
    void** result);
typedef void (*EcoDispose)(EcoObject* object);

struct EcoObject {
    IEcoUnknown inner;
    IEcoUnknown* controller;
    uint32_t references;
    int destroying;
    IEcoSystem1* system;
    IEcoMemoryAllocator1* memory;
    EcoQueryViews query_views;
    EcoDispose dispose;
};

int16_t eco_check_creation(IEcoUnknown* system, IEcoUnknown* outer,
    const UGUID* iid, void** result, int aggregatable);
int16_t eco_object_allocate(IEcoUnknown* system, IEcoUnknown* outer,
    uint32_t size, EcoQueryViews query, EcoDispose dispose,
    EcoObject** result, IEcoInterfaceBus1** bus);
int16_t eco_object_finish(EcoObject* object, const UGUID* iid, void** result);
void eco_object_discard(EcoObject* object);
int16_t eco_public_query(EcoObject* object, const UGUID* iid, void** result);
uint32_t eco_public_addref(EcoObject* object);
uint32_t eco_public_release(EcoObject* object);
int16_t eco_return_view(EcoObject* object, void* view, void** result);

#endif
