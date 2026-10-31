#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ParallelExecutor.h"
#include "TestSupport.h"

static unsigned long sorted_cases = 0;
static uint32_t random_state = 0x51C0FFEEUL;

static uint32_t next_random(void)
{
    random_state = random_state * 1664525UL + 1013904223UL;
    return random_state;
}

static int compare_descending(const void* left, const void* right)
{
    return compare_ints(right, left);
}

static int compare_extreme(const void* left, const void* right)
{
    int order = compare_ints(left, right);

    return order < 0 ? INT_MIN : order > 0 ? INT_MAX : 0;
}

static void check_statistics(const EcoSortStats* statistics, size_t count,
    size_t width)
{
    uint32_t padded = count != 0 ? 1 : 0;
    uint32_t logarithm = 0;
    uint32_t stages;

    while (padded < count) {
        padded *= 2;
        ++logarithm;
    }
    stages = logarithm * (logarithm + 1) / 2;
    CHECK(statistics->padded_count == padded);
    CHECK(statistics->stages == stages);
    CHECK(statistics->network_pairs == (uint64_t)(padded / 2) * stages);
    CHECK(statistics->scratch_bytes == (count > 1 ?
        (uint64_t)padded * sizeof(uint32_t) + count * width : 0));
}

static void integer_case(IEcoSortAndSearch* sort, size_t count, unsigned pattern)
{
    int* actual;
    int* expected;
    size_t index;
    EcoCompare compare = pattern == 5 ? compare_descending : compare_extreme;
    EcoSortStats statistics;

    actual = (int*)malloc((count + 1) * sizeof(int));
    expected = (int*)malloc((count + 1) * sizeof(int));
    REQUIRE(actual != 0 && expected != 0);
    for (index = 0; index < count; ++index) {
        switch (pattern) {
        case 0: actual[index] = (int)(next_random() % 20001) - 10000; break;
        case 1: actual[index] = (int)index; break;
        case 2: actual[index] = (int)(count - index); break;
        case 3: actual[index] = 42; break;
        case 4: actual[index] = index % 2 ? INT_MIN : INT_MAX; break;
        default: actual[index] = (int)(next_random() % 11) - 5; break;
        }
    }
    actual[count] = 0x123456;
    memcpy(expected, actual, (count + 1) * sizeof(int));
    qsort(expected, count, sizeof(int), compare);
    sort->pVTbl->Sort(sort, actual, count, sizeof(int), compare);
    CHECK(sort->pVTbl->GetStatus(sort, &statistics) == ERR_ECO_SUCCESS);
    CHECK(memcmp(actual, expected, (count + 1) * sizeof(int)) == 0);
    check_statistics(&statistics, count, sizeof(int));
    ++sorted_cases;
    free(expected);
    free(actual);
}

typedef struct Record {
    int key;
    uint32_t position;
    unsigned char payload[37];
} Record;

static int compare_records(const void* left, const void* right)
{
    const Record* first = (const Record*)left;
    const Record* second = (const Record*)right;

    return (first->key > second->key) - (first->key < second->key);
}

static int compare_records_reference(const void* left, const void* right)
{
    const Record* first = (const Record*)left;
    const Record* second = (const Record*)right;
    int order = compare_records(left, right);

    if (order != 0) {
        return order;
    }
    return (first->position > second->position) - (first->position < second->position);
}

static int compare_bytes(const void* left, const void* right)
{
    return memcmp(left, right, 13);
}

static void generic_cases(IEcoSortAndSearch* sort)
{
    Record actual[257];
    Record expected[257];
    unsigned char bytes[131][13];
    unsigned char reference_bytes[131][13];
    size_t index;
    size_t byte;

    memset(actual, 0, sizeof(actual));
    for (index = 0; index < 257; ++index) {
        actual[index].key = (int)(next_random() % 19) - 9;
        actual[index].position = (uint32_t)index;
        for (byte = 0; byte < 37; ++byte) {
            actual[index].payload[byte] = (unsigned char)next_random();
        }
    }
    memcpy(expected, actual, sizeof(actual));
    qsort(expected, 257, sizeof(Record), compare_records_reference);
    sort->pVTbl->Sort(sort, actual, 257, sizeof(Record), compare_records);
    CHECK(sort->pVTbl->GetStatus(sort, 0) == 0);
    CHECK(memcmp(actual, expected, sizeof(actual)) == 0);
    ++sorted_cases;
    for (index = 0; index < 131; ++index) {
        for (byte = 0; byte < 13; ++byte) {
            bytes[index][byte] = (unsigned char)next_random();
        }
    }
    memcpy(reference_bytes, bytes, sizeof(bytes));
    qsort(reference_bytes, 131, 13, compare_bytes);
    sort->pVTbl->Sort(sort, bytes, 131, 13, compare_bytes);
    CHECK(sort->pVTbl->GetStatus(sort, 0) == 0);
    CHECK(memcmp(bytes, reference_bytes, sizeof(bytes)) == 0);
    ++sorted_cases;
}

static int compare_long(const void* left, const void* right)
{
    long first = *(const long*)left;
    long second = *(const long*)right;

    return (first > second) - (first < second);
}

static int compare_float(const void* left, const void* right)
{
    float first = *(const float*)left;
    float second = *(const float*)right;

    return (first > second) - (first < second);
}

static int compare_double(const void* left, const void* right)
{
    double first = *(const double*)left;
    double second = *(const double*)right;

    if (first != first) {
        return second != second ? 0 : 1;
    }
    if (second != second) {
        return -1;
    }
    return (first > second) - (first < second);
}

static int compare_long_double(const void* left, const void* right)
{
    long double first = *(const long double*)left;
    long double second = *(const long double*)right;

    return (first > second) - (first < second);
}

static void primitive_case(IEcoSortAndSearch* sort, void* values,
    size_t count, size_t width, EcoCompare compare)
{
    void* expected = malloc(count * width);

    REQUIRE(expected != 0);
    memcpy(expected, values, count * width);
    qsort(expected, count, width, compare);
    sort->pVTbl->Sort(sort, values, count, width, compare);
    CHECK(sort->pVTbl->GetStatus(sort, 0) == 0);
    CHECK(memcmp(values, expected, count * width) == 0);
    free(expected);
    ++sorted_cases;
}

static void primitive_cases(IEcoSortAndSearch* sort)
{
    long longs[] = {LONG_MAX, -1, LONG_MIN, 0, 100, 100};
    float floats[] = {1.25F, -100.5F, 0.0F, 3.0F, 1.25F};
    double doubles[] = {1.0e100, -1.0e100, 0.0, 0.5, -0.25, 9.0};
    long double extended[] = {1.25L, -3.5L, 0.0L, 1.25L, 100.0L};

    doubles[5] = strtod("nan", 0);
    CHECK(doubles[5] != doubles[5]);
    primitive_case(sort, longs, 6, sizeof(long), compare_long);
    primitive_case(sort, floats, 5, sizeof(float), compare_float);
    primitive_case(sort, doubles, 6, sizeof(double), compare_double);
    primitive_case(sort, extended, 5, sizeof(long double), compare_long_double);
}

typedef struct SerialExecutor {
    uint32_t calls;
    uint32_t fail_on;
} SerialExecutor;

static int16_t ECOCALLMETHOD serial_execute(void* context,
    EcoSortJob job, void* job_context, uint32_t lanes)
{
    SerialExecutor* executor = (SerialExecutor*)context;
    uint32_t lane;

    ++executor->calls;
    for (lane = 0; lane < lanes; ++lane) {
        job(job_context, lane, lanes);
        if (executor->calls == executor->fail_on && lane == 0) {
            return ERR_ECO_FAIL;
        }
    }
    return 0;
}

static void boundary_cases(IEcoSortAndSearch* sort)
{
    int values[] = {3, 1, 2};
    int original[3];
    SerialExecutor executor;

    CHECK(sort->pVTbl->SetExecutor(sort, 1, 0, 0) == 0);
    CHECK(sort->pVTbl->SetExecutor(sort, 0, 0, 0) == ERR_ECO_INVALIDARG);
    CHECK(sort->pVTbl->SetExecutor(sort, 65, 0, 0) == ERR_ECO_INVALIDARG);
    CHECK(sort->pVTbl->SetExecutor(sort, 2, 0, 0) == ERR_ECO_INVALIDARG);
    sort->pVTbl->Sort(sort, 0, 0, 0, 0);
    CHECK(sort->pVTbl->GetStatus(sort, 0) == 0);
    memcpy(original, values, sizeof(values));
    sort->pVTbl->Sort(sort, 0, 1, sizeof(int), compare_ints);
    CHECK(sort->pVTbl->GetStatus(sort, 0) == ERR_ECO_INVALIDARG);
    sort->pVTbl->Sort(sort, values, 3, 0, compare_ints);
    CHECK(sort->pVTbl->GetStatus(sort, 0) == ERR_ECO_INVALIDARG);
    sort->pVTbl->Sort(sort, values, 3, sizeof(int), 0);
    CHECK(sort->pVTbl->GetStatus(sort, 0) == ERR_ECO_INVALIDARG);
    sort->pVTbl->Sort(sort, values, (size_t)-1, sizeof(int), compare_ints);
    CHECK(sort->pVTbl->GetStatus(sort, 0) == ERR_ECO_INVALIDARG);
    sort->pVTbl->Sort(sort, values, 3, (size_t)-1, compare_ints);
    CHECK(sort->pVTbl->GetStatus(sort, 0) == ERR_ECO_INVALIDARG);
    sort->pVTbl->Sort(sort, values, (size_t)1073741824UL, 1, compare_ints);
    CHECK(sort->pVTbl->GetStatus(sort, 0) == ERR_ECO_INVALIDARG);
    CHECK(memcmp(values, original, sizeof(values)) == 0);
    executor.calls = 0;
    executor.fail_on = 2;
    CHECK(sort->pVTbl->SetExecutor(sort, 4, serial_execute, &executor) == 0);
    sort->pVTbl->Sort(sort, values, 3, sizeof(int), compare_ints);
    CHECK(sort->pVTbl->GetStatus(sort, 0) == ERR_ECO_FAIL);
    CHECK(executor.calls == 2);
    CHECK(memcmp(values, original, sizeof(values)) == 0);
    executor.calls = 0;
    executor.fail_on = 0;
    CHECK(sort->pVTbl->SetExecutor(sort, 64, serial_execute, &executor) == 0);
    sort->pVTbl->Sort(sort, values, 3, sizeof(int), compare_ints);
    CHECK(sort->pVTbl->GetStatus(sort, 0) == 0);
    CHECK(values[0] == 1 && values[1] == 2 && values[2] == 3);
    CHECK(executor.calls == 3);
    CHECK(sort->pVTbl->SetExecutor(sort, 1, 0, 0) == 0);
}

void test_sorting(IEcoInterfaceBus1* bus)
{
    const size_t extra_sizes[] = {255, 256, 257, 511, 512, 513, 1023, 1025, 4097};
    IEcoSortAndSearch* sort = 0;
    ParallelPool pool;
    size_t count;
    size_t index;
    unsigned pattern;
    unsigned parallel;

    REQUIRE(bus->pVTbl->QueryComponent(bus, &CID_EcoLab1, 0,
        &IID_IEcoSortAndSearch, (void**)&sort) == 0);
    REQUIRE(sort != 0);
    REQUIRE(parallel_pool_open(&pool, 4) == 0);
    for (parallel = 0; parallel < 2; ++parallel) {
        CHECK(sort->pVTbl->SetExecutor(sort, parallel ? 4 : 1,
            parallel ? parallel_pool_execute : 0, parallel ? &pool : 0) == 0);
        for (pattern = 0; pattern < 6; ++pattern) {
            for (count = 0; count <= 129; ++count) {
                integer_case(sort, count, pattern);
            }
            for (index = 0; index < sizeof(extra_sizes) / sizeof(extra_sizes[0]); ++index) {
                integer_case(sort, extra_sizes[index], pattern);
            }
        }
        generic_cases(sort);
        primitive_cases(sort);
    }
    CHECK(sort->pVTbl->SetExecutor(sort, 1, 0, 0) == 0);
    parallel_pool_close(&pool);
    boundary_cases(sort);
    sort->pVTbl->Release(sort);
    printf("SORT_CASES=%lu\n", sorted_cases);
}
