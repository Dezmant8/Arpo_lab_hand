#ifndef ARPO_I_ECO_LAB2_H
#define ARPO_I_ECO_LAB2_H

#include "IEcoBase1.h"
#include "LabIds.h"

typedef struct IEcoLab2 IEcoLab2;
typedef struct IEcoLab2VTbl {
    int16_t (ECOCALLMETHOD *QueryInterface)(IEcoLab2* me,
        const UGUID* iid, void** result);
    uint32_t (ECOCALLMETHOD *AddRef)(IEcoLab2* me);
    uint32_t (ECOCALLMETHOD *Release)(IEcoLab2* me);
    uint32_t (ECOCALLMETHOD *GetVersion)(IEcoLab2* me);
} IEcoLab2VTbl;

struct IEcoLab2 {
    IEcoLab2VTbl* pVTbl;
};
#endif
