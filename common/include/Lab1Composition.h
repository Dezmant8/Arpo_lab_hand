#ifndef ARPO_LAB1_COMPOSITION_H
#define ARPO_LAB1_COMPOSITION_H

#include "IEcoInterfaceBus1.h"
#include "IEcoCalculatorX.h"
#include "IEcoCalculatorY.h"

typedef struct Lab1Parts {
    IEcoCalculatorX* included_x;
    IEcoCalculatorY* included_y;
    IEcoUnknown* inner_b;
} Lab1Parts;

int16_t lab1_parts_open(Lab1Parts* parts, IEcoInterfaceBus1* bus,
    IEcoUnknown* controller);
void lab1_parts_close(Lab1Parts* parts);

#endif
