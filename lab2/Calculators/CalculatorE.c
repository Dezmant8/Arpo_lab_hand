#include "EcoObject.h"
#include "CalculatorMath.h"
#include "LabConstructors.h"
#include "IEcoCalculatorX.h"
#include "IEcoCalculatorY.h"

typedef struct CalculatorE {
    EcoObject object;
    IEcoCalculatorY calc_y;
    IEcoUnknown* inner_b;
} CalculatorE;

static int16_t ECOCALLMETHOD y_query(IEcoCalculatorY* me,
    const UGUID* iid, void** result)
{
    CalculatorE* self = me != 0 ? ECO_OWNER(me, CalculatorE, calc_y) : 0;

    return eco_public_query(self != 0 ? &self->object : 0, iid, result);
}

static uint32_t ECOCALLMETHOD y_addref(IEcoCalculatorY* me)
{
    return me != 0 ? eco_public_addref(&ECO_OWNER(me, CalculatorE, calc_y)->object) : 0;
}

static uint32_t ECOCALLMETHOD y_release(IEcoCalculatorY* me)
{
    return me != 0 ? eco_public_release(&ECO_OWNER(me, CalculatorE, calc_y)->object) : 0;
}

static int32_t ECOCALLMETHOD y_multiply(IEcoCalculatorY* me,
    int16_t left, int16_t right)
{
    return me != 0 ? calculator_multiply(left, right) : 0;
}

static int16_t ECOCALLMETHOD y_divide(IEcoCalculatorY* me,
    int16_t left, int16_t right)
{
    return me != 0 ? calculator_divide(left, right) : 0;
}

static IEcoCalculatorYVTbl y_vtable = {
    y_query, y_addref, y_release,
    y_multiply, y_divide
};

static int16_t query_views(EcoObject* object, const UGUID* iid, void** result)
{
    CalculatorE* self = (CalculatorE*)object;

    if (IsEqualUGUID(iid, &IID_IEcoCalculatorX)) {
        return self->inner_b->pVTbl->QueryInterface(self->inner_b, iid, result);
    }
    if (IsEqualUGUID(iid, &IID_IEcoCalculatorY)) {
        return eco_return_view(object, &self->calc_y, result);
    }
    return ERR_ECO_NOINTERFACE;
}

static void dispose(EcoObject* object)
{
    CalculatorE* self = (CalculatorE*)object;
    IEcoUnknown* dependency = self->inner_b;

    self->inner_b = 0;
    if (dependency != 0) {
        dependency->pVTbl->Release(dependency);
    }
}

int16_t calculator_e_create(IEcoUnknown* system, IEcoUnknown* outer,
    const UGUID* iid, void** result, uint32_t configuration)
{
    EcoObject* object;
    CalculatorE* self;
    IEcoInterfaceBus1* bus;
    int16_t status;

    (void)configuration;
    status = eco_check_creation(system, outer, iid, result, 0);
    if (status != ERR_ECO_SUCCESS) {
        return status;
    }
    status = eco_object_allocate(system, outer, sizeof(CalculatorE),
        query_views, dispose, &object, &bus);
    if (status != ERR_ECO_SUCCESS) {
        return status;
    }
    self = (CalculatorE*)object;
    self->calc_y.pVTbl = &y_vtable;
    self->inner_b = 0;
    status = bus->pVTbl->QueryComponent(bus, &CID_EcoCalculatorB,
        object->controller, &IID_IEcoUnknown, (void**)&self->inner_b);
    bus->pVTbl->Release(bus);
    if (status != ERR_ECO_SUCCESS || self->inner_b == 0) {
        eco_object_discard(object);
        return status != ERR_ECO_SUCCESS ? status : ERR_ECO_FAIL;
    }
    return eco_object_finish(object, iid, result);
}
