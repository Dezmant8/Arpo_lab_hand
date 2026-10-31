#include <stdio.h>
#include "TestSupport.h"

int16_t ECOCALLMETHOD EcoMain(IEcoUnknown* system)
{
    ClientRuntime runtime;
    unsigned long previous;
    int16_t status;

    status = client_open(system, &runtime);
    if (status != 0) {
        fprintf(stderr, "Cannot start Eco runtime: %d\n", (int)status);
        return status;
    }
    previous = test_checks;
    test_com_matrix(runtime.bus);
    printf("COM_MATRIX checks=%lu\n", test_checks - previous);
    previous = test_checks;
    test_external_aggregation(runtime.bus);
    printf("REAL_AGGREGATION checks=%lu\n", test_checks - previous);
    previous = test_checks;
    test_sorting(runtime.bus);
    printf("SORTING checks=%lu\n", test_checks - previous);
    previous = test_checks;
    test_failures_and_lifetime();
    printf("FAILURES_AND_LIFETIME checks=%lu\n", test_checks - previous);
    client_close(&runtime);
    printf("LAB%d_TEST checks=%lu failures=%lu\n",
        ARPO_LAB, test_checks, test_failures);
    return test_failures == 0 ? ERR_ECO_SUCCESS : ERR_ECO_FAIL;
}
