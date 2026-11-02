#include "UnitTest.h"
#include "IEcoCalculatorX.h"
#include "IEcoCalculatorY.h"

int test_x(IEcoSortAndSearch* sort)
{
    IEcoCalculatorX* x = 0;
    int correct;
    if (sort->pVTbl->QueryInterface(sort, &IID_IEcoCalculatorX, (void**)&x) != 0 || x == 0)
        return 0;
    correct = x->pVTbl->Addition(x, 3, 5) == 8 && x->pVTbl->Subtraction(x, 3, 5) == -2;
    x->pVTbl->Release(x);
    return correct;
}

int test_y(IEcoSortAndSearch* sort)
{
    IEcoCalculatorY* y = 0;
    int correct;
    if (sort->pVTbl->QueryInterface(sort, &IID_IEcoCalculatorY, (void**)&y) != 0 || y == 0)
        return 0;
    correct = y->pVTbl->Multiplication(y, 3, 5) == 15 && y->pVTbl->Division(y, 15, 3) == 5;
    y->pVTbl->Release(y);
    return correct;
}

int test_identity(IEcoUnknown* lab, IEcoSortAndSearch* sort)
{
    IEcoUnknown* owner = 0;
    int same;
    if (sort->pVTbl->QueryInterface(sort, &IID_IEcoUnknown, (void**)&owner) != 0 || owner == 0)
        return 0;
    same = owner == lab;
    owner->pVTbl->Release(owner);
    return same;
}
