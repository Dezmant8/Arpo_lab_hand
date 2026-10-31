#include "EcoObject.h"
#include "CalculatorMath.h"
#include "LabConstructors.h"
#include "IEcoCalculatorX.h"
#include "IEcoCalculatorY.h"

typedef struct CalculatorB {
    EcoObject object;
    IEcoCalculatorX calc_x;
} CalculatorB;

static int16_t ECOCALLMETHOD x_query(IEcoCalculatorX* me,
    const UGUID* iid, void** result)
{
    CalculatorB* self = me != 0 ? ECO_OWNER(me, CalculatorB, calc_x) : 0;

    return eco_public_query(self != 0 ? &self->object : 0, iid, result);
}

static uint32_t ECOCALLMETHOD x_addref(IEcoCalculatorX* me)
{
    return me != 0 ? eco_public_addref(&ECO_OWNER(me, CalculatorB, calc_x)->object) : 0;
}

static uint32_t ECOCALLMETHOD x_release(IEcoCalculatorX* me)
{
    return me != 0 ? eco_public_release(&ECO_OWNER(me, CalculatorB, calc_x)->object) : 0;
}

static int32_t ECOCALLMETHOD x_add(IEcoCalculatorX* me,
    int16_t left, int16_t right)
{
    return me != 0 ? calculator_add(left, right) : 0;
}

static int16_t ECOCALLMETHOD x_subtract(IEcoCalculatorX* me,
    int16_t left, int16_t right)
{
    return me != 0 ? calculator_subtract(left, right) : 0;
}

static IEcoCalculatorXVTbl x_vtable = {
    x_query, x_addref, x_release,
    x_add, x_subtract
};

static int16_t query_views(EcoObject* object, const UGUID* iid, void** result)
{
    CalculatorB* self = (CalculatorB*)object;

    if (IsEqualUGUID(iid, &IID_IEcoCalculatorX)) {
        return eco_return_view(object, &self->calc_x, result);
    }
    return ERR_ECO_NOINTERFACE;
}

static void dispose(EcoObject* object)
{
    (void)object;
}

int16_t calculator_b_create(IEcoUnknown* system, IEcoUnknown* outer,
    const UGUID* iid, void** result, uint32_t configuration)
{
    EcoObject* object;
    CalculatorB* self;
    int16_t status;

    (void)configuration;
    status = eco_check_creation(system, outer, iid, result, 1);
    if (status != ERR_ECO_SUCCESS) {
        return status;
    }
    status = eco_object_allocate(system, outer, sizeof(CalculatorB),
        query_views, dispose, &object, 0);
    if (status != ERR_ECO_SUCCESS) {
        return status;
    }
    self = (CalculatorB*)object;
    self->calc_x.pVTbl = &x_vtable;
    return eco_object_finish(object, iid, result);
}
