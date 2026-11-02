#include "Lab1Composition.h"
#include "CalculatorIds.h"
#include "ErrEcoCodes.h"

int16_t lab1_parts_open(Lab1Parts* parts, IEcoInterfaceBus1* bus,
    IEcoUnknown* controller)
{
    int16_t status;

    parts->inner_b = 0;
    parts->included_x = 0;
    parts->included_y = 0;
    status = bus->pVTbl->QueryComponent(bus, &CID_EcoCalculatorB, controller,
        &IID_IEcoUnknown, (void**)&parts->inner_b);
    if (status == ERR_ECO_COMPONENT_NOTFOUND) {
        status = bus->pVTbl->QueryComponent(bus, &CID_EcoCalculatorA, 0,
            &IID_IEcoCalculatorX, (void**)&parts->included_x);
    }
    if (status != ERR_ECO_SUCCESS) {
        return status;
    }

    status = bus->pVTbl->QueryComponent(bus, &CID_EcoCalculatorD, 0,
        &IID_IEcoCalculatorY, (void**)&parts->included_y);
    if (status == ERR_ECO_COMPONENT_NOTFOUND) {
        status = bus->pVTbl->QueryComponent(bus, &CID_EcoCalculatorE, 0,
            &IID_IEcoCalculatorY, (void**)&parts->included_y);
    }
    if (status == ERR_ECO_COMPONENT_NOTFOUND) {
        if (parts->included_x != 0) {
            return parts->included_x->pVTbl->QueryInterface(parts->included_x,
                &IID_IEcoCalculatorY, (void**)&parts->included_y);
        }
        return bus->pVTbl->QueryComponent(bus, &CID_EcoCalculatorA, 0,
            &IID_IEcoCalculatorY, (void**)&parts->included_y);
    }
    return status;
}

void lab1_parts_close(Lab1Parts* parts)
{
    IEcoCalculatorX* included_x = parts->included_x;
    IEcoCalculatorY* included_y = parts->included_y;
    IEcoUnknown* inner_b = parts->inner_b;

    parts->included_x = 0;
    parts->included_y = 0;
    parts->inner_b = 0;
    if (included_y != 0) {
        included_y->pVTbl->Release(included_y);
    }
    if (included_x != 0) {
        included_x->pVTbl->Release(included_x);
    }
    if (inner_b != 0) {
        inner_b->pVTbl->Release(inner_b);
    }
}
