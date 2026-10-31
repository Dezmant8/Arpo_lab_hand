#include <string.h>
#include "FakeEnvironment.h"
#include "TestSupport.h"

static uint32_t requests_for(FakeEnvironment* environment,
    const UGUID* cid, int aggregated)
{
    uint32_t index;
    uint32_t count = 0;

    for (index = 0; index < environment->recorded_queries; ++index) {
        if (IsEqualUGUID(cid, &environment->queries[index].cid) &&
            (environment->queries[index].outer != 0) == aggregated) {
            ++count;
        }
    }
    return count;
}

static void check_routes(FakeEnvironment* environment, const UGUID* cid)
{
#if ARPO_LAB == 1
    (void)environment;
    (void)cid;
#else
    if (IsEqualUGUID(cid, &CID_Lab1IncludedA)) {
        CHECK(requests_for(environment, &CID_EcoCalculatorA, 0) == 1);
        CHECK(requests_for(environment, &CID_EcoCalculatorD, 0) == 1);
    }
    if (IsEqualUGUID(cid, &CID_Lab1AggregatedB) || IsEqualUGUID(cid, &CID_EcoLab1)) {
        CHECK(requests_for(environment, &CID_EcoCalculatorB, 1) == 1);
        CHECK(requests_for(environment, &CID_EcoCalculatorD, 0) == 1);
    }
    if (IsEqualUGUID(cid, &CID_Lab1IncludedC)) {
        CHECK(requests_for(environment, &CID_EcoCalculatorC, 0) == 1);
    }
    if (IsEqualUGUID(cid, &CID_Lab1IncludedD)) {
        CHECK(requests_for(environment, &CID_EcoCalculatorD, 0) == 1);
    }
    if (IsEqualUGUID(cid, &CID_Lab1IncludedE)) {
        CHECK(requests_for(environment, &CID_EcoCalculatorE, 0) == 1);
    }
    if (IsEqualUGUID(cid, &CID_EcoLab2)) {
        CHECK(requests_for(environment, &CID_Lab1AggregatedB, 1) == 1);
        CHECK(requests_for(environment, &CID_EcoCalculatorE, 0) == 1);
        CHECK(requests_for(environment, &CID_EcoCalculatorB, 1) == 2);
    }
#endif
}

static void constructor_failures(uint32_t factory_index)
{
    FakeEnvironment environment;
    IEcoComponentFactory* factory;
    const UGUID* cid;
    IEcoUnknown* object;
    uint32_t allocations;
    uint32_t queries;
    uint32_t step;
    uint32_t mode;
    int16_t status;
    void* result;
    ViewKind kinds[5];

    factory = lab_factory_at(factory_index, &cid);
    REQUIRE(factory != 0);
    fake_open(&environment);
    REQUIRE(lab_register_components(&environment.bus) == 0);
    component_views(cid, kinds);
    result = &environment;
    CHECK(factory->pVTbl->Alloc(factory, &environment.unknown,
        &environment.unknown, view_iid(kinds[1]), &result) == ERR_ECO_NOAGGREGATION);
    CHECK(result == 0);
    if (!component_aggregatable(cid)) {
        result = &environment;
        CHECK(factory->pVTbl->Alloc(factory, &environment.unknown,
            &environment.unknown, &IID_IEcoUnknown, &result) == ERR_ECO_NOAGGREGATION);
        CHECK(result == 0);
    }
    object = 0;
    REQUIRE(factory->pVTbl->Alloc(factory, &environment.unknown, 0,
        &IID_IEcoUnknown, (void**)&object) == 0);
    REQUIRE(object != 0);
    allocations = environment.allocation_calls;
    queries = environment.query_calls;
    check_routes(&environment, cid);
    object->pVTbl->Release(object);
    fake_close(&environment);

    for (mode = 0; mode < 3; ++mode) {
        for (step = 1; step <= (mode == 0 ? allocations : queries); ++step) {
            fake_open(&environment);
            REQUIRE(lab_register_components(&environment.bus) == 0);
            environment.fail_allocation = mode == 0 ? step : 0;
            environment.fail_query = mode == 1 ? step : 0;
            environment.null_query = mode == 2 ? step : 0;
            result = &environment;
            status = factory->pVTbl->Alloc(factory, &environment.unknown,
                0, &IID_IEcoUnknown, &result);
            CHECK(status != ERR_ECO_SUCCESS);
            CHECK(result == 0);
            if (status == 0 && result != 0) {
                ((IEcoUnknown*)result)->pVTbl->Release(result);
            }
            fake_close(&environment);
        }
    }

    fake_open(&environment);
    REQUIRE(lab_register_components(&environment.bus) == 0);
    result = &environment;
    CHECK(factory->pVTbl->Alloc(factory, 0, 0, &IID_IEcoUnknown,
        &result) == ERR_ECO_POINTER);
    CHECK(result == 0);
    result = &environment;
    CHECK(factory->pVTbl->Alloc(factory, &environment.unknown, 0, 0,
        &result) == ERR_ECO_POINTER);
    CHECK(result == 0);
    CHECK(factory->pVTbl->Alloc(factory, &environment.unknown, 0,
        &IID_IEcoUnknown, 0) == ERR_ECO_POINTER);
    result = &environment;
    CHECK(factory->pVTbl->Alloc(factory, &environment.unknown, 0,
        &IID_UnsupportedTest, &result) == ERR_ECO_NOINTERFACE);
    CHECK(result == 0);
    fake_close(&environment);
}

static void lifetime_through_each_view(uint32_t factory_index)
{
    FakeEnvironment environment;
    const UGUID* cid;
    IEcoComponentFactory* factory;
    IEcoUnknown* object;
    ViewKind kinds[5];
    TestView held;
    uint32_t index;
    uint32_t count;

    factory = lab_factory_at(factory_index, &cid);
    REQUIRE(factory != 0);
    count = component_views(cid, kinds);
    for (index = 0; index < count; ++index) {
        fake_open(&environment);
        REQUIRE(lab_register_components(&environment.bus) == 0);
        object = 0;
        REQUIRE(factory->pVTbl->Alloc(factory, &environment.unknown, 0,
            &IID_IEcoUnknown, (void**)&object) == 0);
        held.kind = kinds[index];
        held.value = 0;
        REQUIRE(object->pVTbl->QueryInterface(object, view_iid(held.kind),
            &held.value) == 0);
        object->pVTbl->Release(object);
        CHECK(environment.live_blocks != 0);
        exercise_view(held);
        view_release(held);
        CHECK(environment.live_blocks == 0);
        fake_close(&environment);
    }
}

static void runtime_failures(void)
{
    FakeEnvironment environment;
    const UGUID* cid;
    IEcoComponentFactory* factory = lab_factory_at(0, &cid);
    IEcoUnknown* object;
    int mode;
    int16_t status;

    for (mode = 0; mode < 5; ++mode) {
        fake_open(&environment);
        REQUIRE(lab_register_components(&environment.bus) == 0);
        environment.no_system = mode == 0;
        environment.no_bus = mode == 1;
        environment.broken_extension = mode == 2;
        environment.null_manager = mode == 3;
        environment.no_extension = mode == 4;
        object = 0;
        status = factory->pVTbl->Alloc(factory, &environment.unknown, 0,
            &IID_IEcoUnknown, (void**)&object);
        CHECK((status == 0) == (mode == 4));
        if (object != 0) {
            object->pVTbl->Release(object);
        }
        fake_close(&environment);
    }
}

static void sort_allocation_failures(void)
{
    FakeEnvironment environment;
    IEcoSortAndSearch* sort;
    int values[] = {5, -1, 3, 2, 0};
    int original[5];
    uint32_t step;
    uint32_t objects;

    memcpy(original, values, sizeof(values));
    for (step = 1; step <= 2; ++step) {
        fake_open(&environment);
        REQUIRE(lab_register_components(&environment.bus) == 0);
        sort = 0;
        REQUIRE(environment.bus.pVTbl->QueryComponent(&environment.bus,
            &CID_EcoLab1, 0, &IID_IEcoSortAndSearch, (void**)&sort) == 0);
        objects = environment.live_blocks;
        environment.fail_allocation = environment.allocation_calls + step;
        sort->pVTbl->Sort(sort, values, 5, sizeof(values[0]), compare_ints);
        CHECK(sort->pVTbl->GetStatus(sort, 0) == ERR_ECO_OUTOFMEMORY);
        CHECK(memcmp(values, original, sizeof(values)) == 0);
        CHECK(environment.live_blocks == objects);
        environment.fail_allocation = 0;
        sort->pVTbl->Sort(sort, values, 5, sizeof(values[0]), compare_ints);
        CHECK(sort->pVTbl->GetStatus(sort, 0) == 0);
        CHECK(values[0] == -1 && values[4] == 5);
        sort->pVTbl->Release(sort);
        fake_close(&environment);
        memcpy(values, original, sizeof(values));
    }
}

void test_failures_and_lifetime(void)
{
    FakeEnvironment environment;
    IEcoUnknown* object;
    uint32_t index;
    int16_t status;

    for (index = 0; index < lab_factory_count(); ++index) {
        constructor_failures(index);
        lifetime_through_each_view(index);
    }
    runtime_failures();
    sort_allocation_failures();
    for (index = 1; index <= lab_factory_count(); ++index) {
        fake_open(&environment);
        environment.fail_register = index;
        status = lab_register_components(&environment.bus);
        CHECK(status != 0);
        CHECK(environment.entry_count == 0);
        fake_close(&environment);
    }
    fake_open(&environment);
    REQUIRE(lab_register_components(&environment.bus) == 0);
    test_external_aggregation(&environment.bus);
    fake_close(&environment);

#if ARPO_LAB == 2
    fake_open(&environment);
    REQUIRE(lab_register_components(&environment.bus) == 0);
    environment.missing_cid = &CID_EcoCalculatorB;
    object = 0;
    CHECK(environment.bus.pVTbl->QueryComponent(&environment.bus,
        &CID_EcoCalculatorD, 0, &IID_IEcoUnknown, (void**)&object) == 0);
    CHECK(object != 0);
    CHECK(requests_for(&environment, &CID_EcoCalculatorA, 0) == 1);
    if (object != 0) {
        object->pVTbl->Release(object);
    }
    fake_close(&environment);
#else
    (void)object;
    (void)requests_for;
#endif
}
