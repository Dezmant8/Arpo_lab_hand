#ifndef ARPO_LAB_CONSTRUCTORS_H
#define ARPO_LAB_CONSTRUCTORS_H

#include "IEcoBase1.h"

typedef enum Lab1Composition {
    LAB1_INCLUDE_A_D,
    LAB1_AGGREGATE_B_INCLUDE_D,
    LAB1_INCLUDE_C,
    LAB1_INCLUDE_D,
    LAB1_INCLUDE_E
} Lab1Composition;

int16_t lab1_create(IEcoUnknown* system, IEcoUnknown* outer,
    const UGUID* iid, void** result, uint32_t configuration);
int16_t lab2_create(IEcoUnknown* system, IEcoUnknown* outer,
    const UGUID* iid, void** result, uint32_t configuration);
int16_t calculator_a_create(IEcoUnknown* system, IEcoUnknown* outer,
    const UGUID* iid, void** result, uint32_t configuration);
int16_t calculator_b_create(IEcoUnknown* system, IEcoUnknown* outer,
    const UGUID* iid, void** result, uint32_t configuration);
int16_t calculator_c_create(IEcoUnknown* system, IEcoUnknown* outer,
    const UGUID* iid, void** result, uint32_t configuration);
int16_t calculator_d_create(IEcoUnknown* system, IEcoUnknown* outer,
    const UGUID* iid, void** result, uint32_t configuration);
int16_t calculator_e_create(IEcoUnknown* system, IEcoUnknown* outer,
    const UGUID* iid, void** result, uint32_t configuration);

#endif
