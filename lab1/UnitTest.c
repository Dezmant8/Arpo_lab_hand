#include "ClientRuntime.h"
#include "UnitTest.h"

int test_sort(IEcoSortAndSearch* sort)
{
    int values[] = {3, -1, 3, 0, 2};
    sort->pVTbl->Sort(sort, values, 5, sizeof(values[0]), compare_ints);
    return sort->pVTbl->GetStatus(sort, 0) == 0 &&
        values[0] == -1 && values[1] == 0 && values[2] == 2 &&
        values[3] == 3 && values[4] == 3;
}
