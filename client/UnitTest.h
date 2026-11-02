#ifndef ARPO_UNIT_TEST_H
#define ARPO_UNIT_TEST_H

#include "IEcoSortAndSearch.h"

int test_sort(IEcoSortAndSearch* sort);
#if ARPO_LAB == 2
int test_x(IEcoSortAndSearch* sort);
int test_y(IEcoSortAndSearch* sort);
int test_identity(IEcoUnknown* lab, IEcoSortAndSearch* sort);
#endif

#endif
