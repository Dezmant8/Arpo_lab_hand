#ifndef ARPO_I_ECO_CALCULATOR_X_H
#define ARPO_I_ECO_CALCULATOR_X_H

#include "IEcoBase1.h"
#include "LabIds.h"

typedef struct IEcoCalculatorX IEcoCalculatorX;
typedef struct IEcoCalculatorXVTbl {
    int16_t (ECOCALLMETHOD *QueryInterface)(IEcoCalculatorX* me,
        const UGUID* iid, void** result);
    uint32_t (ECOCALLMETHOD *AddRef)(IEcoCalculatorX* me);
    uint32_t (ECOCALLMETHOD *Release)(IEcoCalculatorX* me);
    int32_t (ECOCALLMETHOD *Addition)(IEcoCalculatorX* me,
        int16_t left, int16_t right);
    int16_t (ECOCALLMETHOD *Subtraction)(IEcoCalculatorX* me,
        int16_t left, int16_t right);
} IEcoCalculatorXVTbl;

struct IEcoCalculatorX {
    IEcoCalculatorXVTbl* pVTbl;
};
#endif
