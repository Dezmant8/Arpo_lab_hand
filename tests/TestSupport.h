#ifndef ARPO_TEST_SUPPORT_H
#define ARPO_TEST_SUPPORT_H

#include "ClientRuntime.h"
#include "IEcoSortAndSearch.h"
#include "IEcoCalculatorX.h"
#include "IEcoCalculatorY.h"
#include "IEcoLab2.h"
#include "ErrEcoCodes.h"

typedef enum ViewKind {
    VIEW_UNKNOWN, VIEW_SORT, VIEW_X, VIEW_Y, VIEW_LAB2
} ViewKind;

typedef struct TestView {
    ViewKind kind;
    void* value;
} TestView;

extern unsigned long test_checks;
extern unsigned long test_failures;
extern const UGUID IID_UnsupportedTest;

int test_check(int condition, const char* expression, const char* file, int line);
#define CHECK(expression) test_check((expression), #expression, __FILE__, __LINE__)
#define REQUIRE(expression) do { \
    if (!CHECK(expression)) { return; } \
} while (0)

const UGUID* view_iid(ViewKind kind);
int16_t view_query(TestView view, const UGUID* iid, void** result);
uint32_t view_addref(TestView view);
uint32_t view_release(TestView view);
void exercise_view(TestView view);
uint32_t component_views(const UGUID* cid, ViewKind* kinds);
int component_aggregatable(const UGUID* cid);
void test_com_matrix(IEcoInterfaceBus1* bus);
void test_sorting(IEcoInterfaceBus1* bus);
void test_failures_and_lifetime(void);
void test_external_aggregation(IEcoInterfaceBus1* bus);

#endif
