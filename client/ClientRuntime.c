#include "ClientRuntime.h"
#include "ErrEcoCodes.h"

int16_t client_open(IEcoUnknown* system, ClientRuntime* runtime)
{
    int16_t status;

    runtime->system = 0;
    runtime->bus = 0;
    if (system == 0) {
        return ERR_ECO_POINTER;
    }
    status = system->pVTbl->QueryInterface(system, &GID_IEcoSystem,
        (void**)&runtime->system);
    if (status != ERR_ECO_SUCCESS || runtime->system == 0) {
        client_close(runtime);
        return ERR_ECO_NOSYSTEM;
    }
    status = runtime->system->pVTbl->QueryInterface(runtime->system,
        &IID_IEcoInterfaceBus1, (void**)&runtime->bus);
    if (status != ERR_ECO_SUCCESS || runtime->bus == 0) {
        client_close(runtime);
        return ERR_ECO_NOBUS;
    }
    status = lab_register_components(runtime->bus);
    if (status != ERR_ECO_SUCCESS) {
        client_close(runtime);
    }
    return status;
}

void client_close(ClientRuntime* runtime)
{
    if (runtime->bus != 0) {
        runtime->bus->pVTbl->Release(runtime->bus);
        runtime->bus = 0;
    }
    if (runtime->system != 0) {
        runtime->system->pVTbl->Release(runtime->system);
        runtime->system = 0;
    }
}

int compare_ints(const void* left, const void* right)
{
    int first = *(const int*)left;
    int second = *(const int*)right;

    return (first > second) - (first < second);
}
