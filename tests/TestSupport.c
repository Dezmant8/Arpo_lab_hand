#include <stdio.h>
#include "TestSupport.h"

unsigned long test_checks = 0;
unsigned long test_failures = 0;
const UGUID IID_UnsupportedTest = {0x01, 0x10,
    {0xFF, 0xFF, 0xFF, 0xFF, 0x88, 0x88, 0x77, 0x77,
     0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0x12, 0x34, 0x56}};

int test_check(int condition, const char* expression, const char* file, int line)
{
    ++test_checks;
    if (!condition) {
        ++test_failures;
        fprintf(stderr, "FAIL %s:%d %s\n", file, line, expression);
    }
    return condition;
}

const UGUID* view_iid(ViewKind kind)
{
    switch (kind) {
    case VIEW_UNKNOWN: return &IID_IEcoUnknown;
    case VIEW_SORT: return &IID_IEcoSortAndSearch;
    case VIEW_X: return &IID_IEcoCalculatorX;
    case VIEW_Y: return &IID_IEcoCalculatorY;
    case VIEW_LAB2: return &IID_IEcoLab2;
    }
    return 0;
}

int16_t view_query(TestView view, const UGUID* iid, void** result)
{
    switch (view.kind) {
    case VIEW_UNKNOWN:
        return ((IEcoUnknown*)view.value)->pVTbl->QueryInterface(view.value, iid, result);
    case VIEW_SORT:
        return ((IEcoSortAndSearch*)view.value)->pVTbl->QueryInterface(view.value, iid, result);
    case VIEW_X:
        return ((IEcoCalculatorX*)view.value)->pVTbl->QueryInterface(view.value, iid, result);
    case VIEW_Y:
        return ((IEcoCalculatorY*)view.value)->pVTbl->QueryInterface(view.value, iid, result);
    case VIEW_LAB2:
        return ((IEcoLab2*)view.value)->pVTbl->QueryInterface(view.value, iid, result);
    }
    return ERR_ECO_NOINTERFACE;
}

uint32_t view_addref(TestView view)
{
    switch (view.kind) {
    case VIEW_UNKNOWN: return ((IEcoUnknown*)view.value)->pVTbl->AddRef(view.value);
    case VIEW_SORT: return ((IEcoSortAndSearch*)view.value)->pVTbl->AddRef(view.value);
    case VIEW_X: return ((IEcoCalculatorX*)view.value)->pVTbl->AddRef(view.value);
    case VIEW_Y: return ((IEcoCalculatorY*)view.value)->pVTbl->AddRef(view.value);
    case VIEW_LAB2: return ((IEcoLab2*)view.value)->pVTbl->AddRef(view.value);
    }
    return 0;
}

uint32_t view_release(TestView view)
{
    switch (view.kind) {
    case VIEW_UNKNOWN: return ((IEcoUnknown*)view.value)->pVTbl->Release(view.value);
    case VIEW_SORT: return ((IEcoSortAndSearch*)view.value)->pVTbl->Release(view.value);
    case VIEW_X: return ((IEcoCalculatorX*)view.value)->pVTbl->Release(view.value);
    case VIEW_Y: return ((IEcoCalculatorY*)view.value)->pVTbl->Release(view.value);
    case VIEW_LAB2: return ((IEcoLab2*)view.value)->pVTbl->Release(view.value);
    }
    return 0;
}

void exercise_view(TestView view)
{
    IEcoSortAndSearch* sort;
    IEcoCalculatorX* calc_x;
    IEcoCalculatorY* calc_y;
    IEcoUnknown* unknown = 0;
    int values[] = {3, -8, 3, 1, 0};

    switch (view.kind) {
    case VIEW_UNKNOWN:
        CHECK(view_query(view, &IID_IEcoUnknown, (void**)&unknown) == ERR_ECO_SUCCESS);
        if (unknown != 0) {
            CHECK(unknown == view.value);
            unknown->pVTbl->Release(unknown);
        }
        break;
    case VIEW_SORT:
        sort = (IEcoSortAndSearch*)view.value;
        sort->pVTbl->Sort(sort, values, 5, sizeof(values[0]), compare_ints);
        CHECK(sort->pVTbl->GetStatus(sort, 0) == ERR_ECO_SUCCESS);
        CHECK(values[0] == -8 && values[1] == 0 && values[2] == 1 &&
            values[3] == 3 && values[4] == 3);
        break;
    case VIEW_X:
        calc_x = (IEcoCalculatorX*)view.value;
        CHECK(calc_x->pVTbl->Addition(calc_x, 3, 5) == 8);
        CHECK(calc_x->pVTbl->Addition(calc_x, 32767, 32767) == 65534L);
        CHECK(calc_x->pVTbl->Subtraction(calc_x, 3, 5) == -2);
        CHECK(calc_x->pVTbl->Subtraction(calc_x, -32768, 0) == -32768L);
        CHECK(calc_x->pVTbl->Subtraction(calc_x, 32767, -1) == 0);
        break;
    case VIEW_Y:
        calc_y = (IEcoCalculatorY*)view.value;
        CHECK(calc_y->pVTbl->Multiplication(calc_y, 3, 5) == 15);
        CHECK(calc_y->pVTbl->Multiplication(calc_y, -32768, -32768) == 1073741824L);
        CHECK(calc_y->pVTbl->Division(calc_y, 15, 3) == 5);
        CHECK(calc_y->pVTbl->Division(calc_y, -7, 2) == -3);
        CHECK(calc_y->pVTbl->Division(calc_y, 7, -2) == -3);
        CHECK(calc_y->pVTbl->Division(calc_y, -7, -2) == 3);
        CHECK(calc_y->pVTbl->Division(calc_y, 1, 0) == 0);
        CHECK(calc_y->pVTbl->Division(calc_y, -32768, -1) == 0);
        break;
    case VIEW_LAB2:
        CHECK(((IEcoLab2*)view.value)->pVTbl->GetVersion(view.value) == 1);
        break;
    }
}

uint32_t component_views(const UGUID* cid, ViewKind* kinds)
{
    uint32_t count = 0;

    kinds[count++] = VIEW_UNKNOWN;
#if ARPO_LAB == 1
    (void)cid;
    kinds[count++] = VIEW_SORT;
#else
    if (IsEqualUGUID(cid, &CID_EcoCalculatorA) ||
        IsEqualUGUID(cid, &CID_EcoCalculatorB)) {
        kinds[count++] = VIEW_X;
    }
    else if (IsEqualUGUID(cid, &CID_EcoCalculatorC) ||
        IsEqualUGUID(cid, &CID_EcoCalculatorD) ||
        IsEqualUGUID(cid, &CID_EcoCalculatorE)) {
        kinds[count++] = VIEW_X;
        kinds[count++] = VIEW_Y;
    }
    else {
        kinds[count++] = VIEW_SORT;
        kinds[count++] = VIEW_X;
        kinds[count++] = VIEW_Y;
        if (IsEqualUGUID(cid, &CID_EcoLab2)) {
            kinds[count++] = VIEW_LAB2;
        }
    }
#endif
    return count;
}

int component_aggregatable(const UGUID* cid)
{
#if ARPO_LAB == 1
    (void)cid;
    return 0;
#else
    return IsEqualUGUID(cid, &CID_EcoCalculatorB) ||
        IsEqualUGUID(cid, &CID_EcoLab1) ||
        IsEqualUGUID(cid, &CID_Lab1IncludedA) ||
        IsEqualUGUID(cid, &CID_Lab1AggregatedB) ||
        IsEqualUGUID(cid, &CID_Lab1IncludedC) ||
        IsEqualUGUID(cid, &CID_Lab1IncludedD) ||
        IsEqualUGUID(cid, &CID_Lab1IncludedE);
#endif
}
