#ifndef ARPO_CLIENT_RUNTIME_H
#define ARPO_CLIENT_RUNTIME_H

#include "IEcoSystem1.h"
#include "IEcoInterfaceBus1.h"
#include "LabRegistry.h"

typedef struct ClientRuntime {
    IEcoSystem1* system;
    IEcoInterfaceBus1* bus;
} ClientRuntime;

int16_t client_open(IEcoUnknown* system, ClientRuntime* runtime);
void client_close(ClientRuntime* runtime);
int compare_ints(const void* left, const void* right);

#endif
