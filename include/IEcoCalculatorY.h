#ifndef ARPO_I_ECO_CALCULATOR_Y_H
#define ARPO_I_ECO_CALCULATOR_Y_H

#include "IEcoBase1.h"
#include "LabIds.h"

typedef struct IEcoCalculatorY IEcoCalculatorY;
typedef struct IEcoCalculatorYVTbl {
    int16_t (ECOCALLMETHOD *QueryInterface)(IEcoCalculatorY* me,
        const UGUID* iid, void** result);
    uint32_t (ECOCALLMETHOD *AddRef)(IEcoCalculatorY* me);
    uint32_t (ECOCALLMETHOD *Release)(IEcoCalculatorY* me);
    int32_t (ECOCALLMETHOD *Multiplication)(IEcoCalculatorY* me,
        int16_t left, int16_t right);
    int16_t (ECOCALLMETHOD *Division)(IEcoCalculatorY* me,
        int16_t left, int16_t right);
} IEcoCalculatorYVTbl;

struct IEcoCalculatorY {
    IEcoCalculatorYVTbl* pVTbl;
};
#endif
