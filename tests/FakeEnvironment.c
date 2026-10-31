#include <stdlib.h>
#include <string.h>
#include "FakeEnvironment.h"
#include "TestSupport.h"
#include "IdEcoMemoryManager1.h"

typedef union AllocationHeader {
    struct {
        FakeEnvironment* owner;
        uint32_t size;
    } value;
    double alignment[4];
} AllocationHeader;

static int16_t query_environment(FakeEnvironment* self,
    const UGUID* iid, void** result)
{
    if (result == 0) {
        return ERR_ECO_POINTER;
    }
    *result = 0;
    if (iid == 0) {
        return ERR_ECO_POINTER;
    }
    if (IsEqualUGUID(iid, &IID_IEcoUnknown)) {
        *result = &self->unknown;
    }
    else if (IsEqualUGUID(iid, &GID_IEcoSystem) && !self->no_system) {
        *result = &self->system;
    }
    else if (IsEqualUGUID(iid, &IID_IEcoInterfaceBus1) && !self->no_bus) {
        *result = &self->bus;
    }
    else if (IsEqualUGUID(iid, &IID_IEcoMemoryAllocator1)) {
        *result = &self->memory;
    }
    else if (IsEqualUGUID(iid, &IID_IEcoInterfaceBus1MemExt) && !self->no_extension) {
        if (self->broken_extension) {
            return ERR_ECO_FAIL;
        }
        *result = &self->extension;
    }
    else {
        return ERR_ECO_NOINTERFACE;
    }
    ++self->references;
    return ERR_ECO_SUCCESS;
}

static int16_t ECOCALLMETHOD unknown_query(IEcoUnknown* me,
    const UGUID* iid, void** result)
{
    return query_environment(ECO_OWNER(me, FakeEnvironment, unknown), iid, result);
}

static uint32_t ECOCALLMETHOD unknown_addref(IEcoUnknown* me)
{
    return ++ECO_OWNER(me, FakeEnvironment, unknown)->references;
}

static uint32_t ECOCALLMETHOD unknown_release(IEcoUnknown* me)
{
    FakeEnvironment* self = ECO_OWNER(me, FakeEnvironment, unknown);

    CHECK(self->references > 1);
    return --self->references;
}

static int16_t ECOCALLMETHOD system_query(IEcoSystem1* me,
    const UGUID* iid, void** result)
{
    return query_environment(ECO_OWNER(me, FakeEnvironment, system), iid, result);
}

static uint32_t ECOCALLMETHOD system_addref(IEcoSystem1* me)
{
    return ++ECO_OWNER(me, FakeEnvironment, system)->references;
}

static uint32_t ECOCALLMETHOD system_release(IEcoSystem1* me)
{
    FakeEnvironment* self = ECO_OWNER(me, FakeEnvironment, system);

    CHECK(self->references > 1);
    return --self->references;
}

static int16_t ECOCALLMETHOD bus_query(IEcoInterfaceBus1* me,
    const UGUID* iid, void** result)
{
    return query_environment(ECO_OWNER(me, FakeEnvironment, bus), iid, result);
}

static uint32_t ECOCALLMETHOD bus_addref(IEcoInterfaceBus1* me)
{
    return ++ECO_OWNER(me, FakeEnvironment, bus)->references;
}

static uint32_t ECOCALLMETHOD bus_release(IEcoInterfaceBus1* me)
{
    FakeEnvironment* self = ECO_OWNER(me, FakeEnvironment, bus);

    CHECK(self->references > 1);
    return --self->references;
}

static int16_t ECOCALLMETHOD extension_query(IEcoInterfaceBus1MemExt* me,
    const UGUID* iid, void** result)
{
    return query_environment(ECO_OWNER(me, FakeEnvironment, extension), iid, result);
}

static uint32_t ECOCALLMETHOD extension_addref(IEcoInterfaceBus1MemExt* me)
{
    return ++ECO_OWNER(me, FakeEnvironment, extension)->references;
}

static uint32_t ECOCALLMETHOD extension_release(IEcoInterfaceBus1MemExt* me)
{
    FakeEnvironment* self = ECO_OWNER(me, FakeEnvironment, extension);

    memset(&self->manager, 0, sizeof(self->manager));
    CHECK(self->references > 1);
    return --self->references;
}

static int16_t ECOCALLMETHOD memory_query(IEcoMemoryAllocator1* me,
    const UGUID* iid, void** result)
{
    return query_environment(ECO_OWNER(me, FakeEnvironment, memory), iid, result);
}

static uint32_t ECOCALLMETHOD memory_addref(IEcoMemoryAllocator1* me)
{
    return ++ECO_OWNER(me, FakeEnvironment, memory)->references;
}

static uint32_t ECOCALLMETHOD memory_release(IEcoMemoryAllocator1* me)
{
    FakeEnvironment* self = ECO_OWNER(me, FakeEnvironment, memory);

    CHECK(self->references > 1);
    return --self->references;
}

static void ECOCALLMETHOD system_set_data(IEcoSystem1* me, void* value)
{
    ECO_OWNER(me, FakeEnvironment, system)->data = value;
}

static void* ECOCALLMETHOD system_get_data(IEcoSystem1* me)
{
    return ECO_OWNER(me, FakeEnvironment, system)->data;
}

static int16_t ECOCALLMETHOD bus_init(IEcoInterfaceBus1* me)
{
    (void)me;
    return ERR_ECO_SUCCESS;
}

static int16_t ECOCALLMETHOD bus_init_with(IEcoInterfaceBus1* me,
    void* heap, uint32_t size)
{
    (void)me;
    (void)heap;
    (void)size;
    return ERR_ECO_NOTIMPL;
}

static int16_t ECOCALLMETHOD bus_register(IEcoInterfaceBus1* me,
    const UGUID* cid, IEcoUnknown* factory)
{
    FakeEnvironment* self = ECO_OWNER(me, FakeEnvironment, bus);
    IEcoComponentFactory* owned = 0;
    int16_t status;

    ++self->register_calls;
    if (self->register_calls == self->fail_register) {
        return ERR_ECO_FAIL;
    }
    if (self->entry_count == 16 || cid == 0 || factory == 0) {
        return ERR_ECO_INVALIDARG;
    }
    status = factory->pVTbl->QueryInterface(factory, &IID_IEcoComponentFactory,
        (void**)&owned);
    if (status != ERR_ECO_SUCCESS) {
        return status;
    }
    self->entries[self->entry_count].cid = *cid;
    self->entries[self->entry_count].factory = owned;
    ++self->entry_count;
    return ERR_ECO_SUCCESS;
}

static int16_t ECOCALLMETHOD bus_unregister(IEcoInterfaceBus1* me,
    const UGUID* cid)
{
    FakeEnvironment* self = ECO_OWNER(me, FakeEnvironment, bus);
    uint32_t index;

    for (index = 0; index < self->entry_count; ++index) {
        if (IsEqualUGUID(cid, &self->entries[index].cid)) {
            self->entries[index].factory->pVTbl->Release(self->entries[index].factory);
            --self->entry_count;
            self->entries[index] = self->entries[self->entry_count];
            return ERR_ECO_SUCCESS;
        }
    }
    return ERR_ECO_COMPONENT_NOTFOUND;
}

static int16_t ECOCALLMETHOD bus_component(IEcoInterfaceBus1* me,
    const UGUID* cid, IEcoUnknown* outer, const UGUID* iid, void** result)
{
    FakeEnvironment* self = ECO_OWNER(me, FakeEnvironment, bus);
    IEcoComponentFactory* factory;
    uint32_t index;
    int16_t status;

    if (result == 0) {
        return ERR_ECO_POINTER;
    }
    *result = 0;
    if (cid == 0 || iid == 0) {
        return ERR_ECO_POINTER;
    }
    ++self->query_calls;
    if (self->recorded_queries < 64) {
        self->queries[self->recorded_queries].cid = *cid;
        self->queries[self->recorded_queries].outer = outer;
        ++self->recorded_queries;
    }
    if (self->query_calls == self->fail_query) {
        return ERR_ECO_FAIL;
    }
    if (self->query_calls == self->null_query) {
        return ERR_ECO_SUCCESS;
    }
    if (self->missing_cid != 0 && IsEqualUGUID(cid, self->missing_cid)) {
        return ERR_ECO_COMPONENT_NOTFOUND;
    }
    if (IsEqualUGUID(cid, &CID_EcoMemoryManager1)) {
        return query_environment(self, iid, result);
    }
    for (index = 0; index < self->entry_count; ++index) {
        if (IsEqualUGUID(cid, &self->entries[index].cid)) {
            factory = self->entries[index].factory;
            status = factory->pVTbl->Alloc(factory, &self->unknown, outer, iid, result);
            if (status == ERR_ECO_SUCCESS) {
                CHECK(*result != 0);
                CHECK(factory->pVTbl->Init(factory, &self->unknown, *result) == 0);
            }
            return status;
        }
    }
    return ERR_ECO_COMPONENT_NOTFOUND;
}

static const UGUID* ECOCALLMETHOD extension_get_manager(IEcoInterfaceBus1MemExt* me)
{
    FakeEnvironment* self = ECO_OWNER(me, FakeEnvironment, extension);

    self->manager = CID_EcoMemoryManager1;
    return self->null_manager ? 0 : &self->manager;
}

static int16_t ECOCALLMETHOD extension_set_manager(IEcoInterfaceBus1MemExt* me,
    const UGUID* cid)
{
    (void)me;
    (void)cid;
    return ERR_ECO_NOTIMPL;
}

static int16_t ECOCALLMETHOD extension_expand(IEcoInterfaceBus1MemExt* me, bool_t expand)
{
    (void)me;
    (void)expand;
    return ERR_ECO_NOTIMPL;
}

static void* ECOCALLMETHOD memory_allocate(IEcoMemoryAllocator1* me, uint32_t size)
{
    FakeEnvironment* self = ECO_OWNER(me, FakeEnvironment, memory);
    AllocationHeader* block;
    unsigned char* bytes;

    ++self->allocation_calls;
    if (self->allocation_calls == self->fail_allocation) {
        return 0;
    }
    if (size > 16UL * 1024 * 1024) {
        return 0;
    }
    block = (AllocationHeader*)malloc(sizeof(*block) + size + 1);
    if (block == 0) {
        return 0;
    }
    block->value.owner = self;
    block->value.size = size;
    bytes = (unsigned char*)(block + 1);
    memset(bytes, 0xA5, size);
    bytes[size] = 0x7B;
    ++self->live_blocks;
    return bytes;
}

static void ECOCALLMETHOD memory_free(IEcoMemoryAllocator1* me, void* pointer)
{
    FakeEnvironment* self = ECO_OWNER(me, FakeEnvironment, memory);
    AllocationHeader* block;

    if (pointer == 0) {
        return;
    }
    block = (AllocationHeader*)pointer - 1;
    CHECK(block->value.owner == self);
    CHECK(((unsigned char*)pointer)[block->value.size] == 0x7B);
    CHECK(self->live_blocks != 0);
    --self->live_blocks;
    memset(pointer, 0xDD, block->value.size);
    free(block);
}

static void* ECOCALLMETHOD memory_reallocate(IEcoMemoryAllocator1* me,
    void* pointer, uint32_t size)
{
    AllocationHeader* old;
    void* replacement = memory_allocate(me, size);
    uint32_t copied;

    if (replacement != 0 && pointer != 0) {
        old = (AllocationHeader*)pointer - 1;
        copied = size < old->value.size ? size : old->value.size;
        memcpy(replacement, pointer, copied);
        memory_free(me, pointer);
    }
    return replacement;
}

static void* ECOCALLMETHOD memory_copy(IEcoMemoryAllocator1* me,
    void* destination, void* source, uint32_t size)
{
    (void)me;
    return memcpy(destination, source, size);
}

static void* ECOCALLMETHOD memory_fill(IEcoMemoryAllocator1* me,
    void* destination, char_t value, uint32_t size)
{
    (void)me;
    return memset(destination, value, size);
}

static int16_t ECOCALLMETHOD memory_compare(IEcoMemoryAllocator1* me,
    void* left, void* right, uint32_t size)
{
    int difference;

    (void)me;
    difference = memcmp(left, right, size);
    return (int16_t)((difference > 0) - (difference < 0));
}

static uint32_t ECOCALLMETHOD memory_size(IEcoMemoryAllocator1* me, void* pointer)
{
    (void)me;
    return pointer != 0 ? ((AllocationHeader*)pointer - 1)->value.size : 0;
}

static IEcoUnknownVTbl unknown_vtable = {
    unknown_query, unknown_addref, unknown_release
};
static IEcoSystem1VTbl system_vtable = {
    system_query, system_addref, system_release, system_set_data, system_get_data
};
static IEcoInterfaceBus1VTbl bus_vtable = {
    bus_query, bus_addref, bus_release, bus_init, bus_init_with,
    bus_register, bus_unregister, bus_component
};
static IEcoInterfaceBus1MemExtVTbl extension_vtable = {
    extension_query, extension_addref, extension_release,
    extension_set_manager, extension_get_manager, extension_expand
};
static IEcoMemoryAllocator1VTbl memory_vtable = {
    memory_query, memory_addref, memory_release,
    memory_allocate, memory_free, memory_reallocate,
    memory_copy, memory_fill, memory_compare, memory_size
};

void fake_open(FakeEnvironment* self)
{
    memset(self, 0, sizeof(*self));
    self->unknown.pVTbl = &unknown_vtable;
    self->system.pVTbl = &system_vtable;
    self->bus.pVTbl = &bus_vtable;
    self->extension.pVTbl = &extension_vtable;
    self->memory.pVTbl = &memory_vtable;
    self->references = 1;
    self->manager = CID_EcoMemoryManager1;
}

void fake_close(FakeEnvironment* self)
{
    while (self->entry_count != 0) {
        --self->entry_count;
        self->entries[self->entry_count].factory->pVTbl->Release(
            self->entries[self->entry_count].factory);
    }
    CHECK(self->references == 1);
    CHECK(self->live_blocks == 0);
}
