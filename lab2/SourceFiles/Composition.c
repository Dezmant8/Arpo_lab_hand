#include "Lab1Composition.h"
#include "ErrEcoCodes.h"

static int16_t include_xy(Lab1Parts* parts, IEcoInterfaceBus1* bus,
    const UGUID* cid)
{
    int16_t status;

    status = bus->pVTbl->QueryComponent(bus, cid, 0,
        &IID_IEcoCalculatorX, (void**)&parts->included_x);
    if (status != ERR_ECO_SUCCESS || parts->included_x == 0) {
        return status != ERR_ECO_SUCCESS ? status : ERR_ECO_FAIL;
    }
    return parts->included_x->pVTbl->QueryInterface(parts->included_x,
        &IID_IEcoCalculatorY, (void**)&parts->included_y);
}

int16_t lab1_parts_open(Lab1Parts* parts, IEcoInterfaceBus1* bus,
    IEcoUnknown* controller, Lab1Composition composition)
{
    int16_t status;

    parts->included_x = 0;
    parts->included_y = 0;
    parts->inner_b = 0;
    switch (composition) {
    case LAB1_INCLUDE_A_D:
        status = bus->pVTbl->QueryComponent(bus, &CID_EcoCalculatorA, 0,
            &IID_IEcoCalculatorX, (void**)&parts->included_x);
        break;
    case LAB1_AGGREGATE_B_INCLUDE_D:
        status = bus->pVTbl->QueryComponent(bus, &CID_EcoCalculatorB, controller,
            &IID_IEcoUnknown, (void**)&parts->inner_b);
        break;
    case LAB1_INCLUDE_C:
        return include_xy(parts, bus, &CID_EcoCalculatorC);
    case LAB1_INCLUDE_D:
        return include_xy(parts, bus, &CID_EcoCalculatorD);
    case LAB1_INCLUDE_E:
        return include_xy(parts, bus, &CID_EcoCalculatorE);
    default:
        return ERR_ECO_INVALIDARG;
    }
    if (status != ERR_ECO_SUCCESS) {
        return status;
    }
    return bus->pVTbl->QueryComponent(bus, &CID_EcoCalculatorD, 0,
        &IID_IEcoCalculatorY, (void**)&parts->included_y);
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
