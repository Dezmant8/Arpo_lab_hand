#ifndef ARPO_LAB_CONSTRUCTORS_H
#define ARPO_LAB_CONSTRUCTORS_H

#include "IEcoBase1.h"

int16_t lab1_create(IEcoUnknown* system, IEcoUnknown* outer,
    const UGUID* iid, void** result);
int16_t lab2_create(IEcoUnknown* system, IEcoUnknown* outer,
    const UGUID* iid, void** result);
#endif
