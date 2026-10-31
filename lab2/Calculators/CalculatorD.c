#include "EcoObject.h"
#include "CalculatorMath.h"
#include "LabConstructors.h"
#include "IEcoCalculatorX.h"
#include "IEcoCalculatorY.h"

typedef struct CalculatorD {
    EcoObject object;
    IEcoCalculatorX calc_x;
    IEcoCalculatorY calc_y;
    IEcoCalculatorX* included_x;
} CalculatorD;

static int16_t ECOCALLMETHOD x_query(IEcoCalculatorX* me,
    const UGUID* iid, void** result)
{
    CalculatorD* self = me != 0 ? ECO_OWNER(me, CalculatorD, calc_x) : 0;

    return eco_public_query(self != 0 ? &self->object : 0, iid, result);
}

static uint32_t ECOCALLMETHOD x_addref(IEcoCalculatorX* me)
{
    return me != 0 ? eco_public_addref(&ECO_OWNER(me, CalculatorD, calc_x)->object) : 0;
}

static uint32_t ECOCALLMETHOD x_release(IEcoCalculatorX* me)
{
    return me != 0 ? eco_public_release(&ECO_OWNER(me, CalculatorD, calc_x)->object) : 0;
}

static int32_t ECOCALLMETHOD x_add(IEcoCalculatorX* me,
    int16_t left, int16_t right)
{
    CalculatorD* self;

    if (me == 0) {
        return 0;
    }
    self = ECO_OWNER(me, CalculatorD, calc_x);
    return self->included_x->pVTbl->Addition(self->included_x, left, right);
}

static int16_t ECOCALLMETHOD x_subtract(IEcoCalculatorX* me,
    int16_t left, int16_t right)
{
    CalculatorD* self;

    if (me == 0) {
        return 0;
    }
    self = ECO_OWNER(me, CalculatorD, calc_x);
    return self->included_x->pVTbl->Subtraction(self->included_x, left, right);
}

static IEcoCalculatorXVTbl x_vtable = {
    x_query, x_addref, x_release,
    x_add, x_subtract
};

static int16_t ECOCALLMETHOD y_query(IEcoCalculatorY* me,
    const UGUID* iid, void** result)
{
    CalculatorD* self = me != 0 ? ECO_OWNER(me, CalculatorD, calc_y) : 0;

    return eco_public_query(self != 0 ? &self->object : 0, iid, result);
}

static uint32_t ECOCALLMETHOD y_addref(IEcoCalculatorY* me)
{
    return me != 0 ? eco_public_addref(&ECO_OWNER(me, CalculatorD, calc_y)->object) : 0;
}

static uint32_t ECOCALLMETHOD y_release(IEcoCalculatorY* me)
{
    return me != 0 ? eco_public_release(&ECO_OWNER(me, CalculatorD, calc_y)->object) : 0;
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
    CalculatorD* self = (CalculatorD*)object;

    if (IsEqualUGUID(iid, &IID_IEcoCalculatorX)) {
        return eco_return_view(object, &self->calc_x, result);
    }
    if (IsEqualUGUID(iid, &IID_IEcoCalculatorY)) {
        return eco_return_view(object, &self->calc_y, result);
    }
    return ERR_ECO_NOINTERFACE;
}

static void dispose(EcoObject* object)
{
    CalculatorD* self = (CalculatorD*)object;
    IEcoCalculatorX* dependency = self->included_x;

    self->included_x = 0;
    if (dependency != 0) {
        dependency->pVTbl->Release(dependency);
    }
}

int16_t calculator_d_create(IEcoUnknown* system, IEcoUnknown* outer,
    const UGUID* iid, void** result, uint32_t configuration)
{
    EcoObject* object;
    CalculatorD* self;
    IEcoInterfaceBus1* bus;
    int16_t status;

    (void)configuration;
    status = eco_check_creation(system, outer, iid, result, 0);
    if (status != ERR_ECO_SUCCESS) {
        return status;
    }
    status = eco_object_allocate(system, outer, sizeof(CalculatorD),
        query_views, dispose, &object, &bus);
    if (status != ERR_ECO_SUCCESS) {
        return status;
    }
    self = (CalculatorD*)object;
    self->calc_x.pVTbl = &x_vtable;
    self->calc_y.pVTbl = &y_vtable;
    self->included_x = 0;
    status = bus->pVTbl->QueryComponent(bus, &CID_EcoCalculatorB, 0,
        &IID_IEcoCalculatorX, (void**)&self->included_x);
    if (status == ERR_ECO_COMPONENT_NOTFOUND) {
        status = bus->pVTbl->QueryComponent(bus, &CID_EcoCalculatorA, 0,
            &IID_IEcoCalculatorX, (void**)&self->included_x);
    }
    bus->pVTbl->Release(bus);
    if (status != ERR_ECO_SUCCESS || self->included_x == 0) {
        eco_object_discard(object);
        return status != ERR_ECO_SUCCESS ? status : ERR_ECO_FAIL;
    }
    return eco_object_finish(object, iid, result);
}
