#ifndef ARPO_FAKE_ENVIRONMENT_H
#define ARPO_FAKE_ENVIRONMENT_H

#include "EcoObject.h"
#include "IEcoInterfaceBus1MemExt.h"

typedef struct FakeEntry {
    UGUID cid;
    IEcoComponentFactory* factory;
} FakeEntry;

typedef struct FakeQuery {
    UGUID cid;
    IEcoUnknown* outer;
} FakeQuery;

typedef struct FakeEnvironment {
    IEcoUnknown unknown;
    IEcoSystem1 system;
    IEcoInterfaceBus1 bus;
    IEcoInterfaceBus1MemExt extension;
    IEcoMemoryAllocator1 memory;
    uint32_t references;
    uint32_t live_blocks;
    uint32_t allocation_calls;
    uint32_t query_calls;
    uint32_t fail_allocation;
    uint32_t fail_query;
    uint32_t null_query;
    uint32_t register_calls;
    uint32_t fail_register;
    int no_system;
    int no_bus;
    int no_extension;
    int broken_extension;
    int null_manager;
    const UGUID* missing_cid;
    UGUID manager;
    FakeEntry entries[16];
    uint32_t entry_count;
    FakeQuery queries[64];
    uint32_t recorded_queries;
    void* data;
} FakeEnvironment;

void fake_open(FakeEnvironment* environment);
void fake_close(FakeEnvironment* environment);

#endif
