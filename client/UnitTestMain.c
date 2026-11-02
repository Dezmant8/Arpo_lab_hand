#include <stdio.h>
#include "ClientRuntime.h"
#include "UnitTest.h"
#include "ErrEcoCodes.h"

static int check_component(IEcoUnknown* lab)
{
    IEcoSortAndSearch* sort = 0;
    int correct;

    if (lab->pVTbl->QueryInterface(lab, &IID_IEcoSortAndSearch, (void**)&sort) != 0 || sort == 0)
        return 0;
    correct = test_sort(sort);
#if ARPO_LAB == 2
    correct = correct && test_x(sort) && test_y(sort) && test_identity(lab, sort);
#endif
    sort->pVTbl->Release(sort);
    return correct;
}

int16_t ECOCALLMETHOD EcoMain(IEcoUnknown* system)
{
    ClientRuntime runtime;
    IEcoUnknown* component = 0;
    int16_t status = client_open(system, &runtime);

    if (status != 0) return status;
    status = runtime.bus->pVTbl->QueryComponent(runtime.bus, &TEST_CLASS, 0,
        &IID_IEcoUnknown, (void**)&component);
    if (status != 0 || component == 0) {
        if (status == 0) status = ERR_ECO_FAIL;
        printf("QueryComponent failed: %d\n", (int)status);
    }
    else {
        status = check_component(component) ? ERR_ECO_SUCCESS : ERR_ECO_FAIL;
        component->pVTbl->Release(component);
    }
    client_close(&runtime);
    return status;
}
