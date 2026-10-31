#include "EcoObject.h"
#include "SortState.h"
#include "Lab1Composition.h"

typedef struct CEcoLab1 {
    EcoObject object;
    IEcoSortAndSearch sort;
    SortState state;
    IEcoCalculatorX calc_x;
    IEcoCalculatorY calc_y;
    Lab1Parts parts;
} CEcoLab1;

static int16_t ECOCALLMETHOD sort_query(IEcoSortAndSearch* me,
    const UGUID* iid, void** result)
{
    CEcoLab1* self = me != 0 ? ECO_OWNER(me, CEcoLab1, sort) : 0;

    return eco_public_query(self != 0 ? &self->object : 0, iid, result);
}

static uint32_t ECOCALLMETHOD sort_addref(IEcoSortAndSearch* me)
{
    return me != 0 ? eco_public_addref(&ECO_OWNER(me, CEcoLab1, sort)->object) : 0;
}

static uint32_t ECOCALLMETHOD sort_release(IEcoSortAndSearch* me)
{
    return me != 0 ? eco_public_release(&ECO_OWNER(me, CEcoLab1, sort)->object) : 0;
}

static void ECOCALLMETHOD sort_run(IEcoSortAndSearch* me, void* base,
    size_t count, size_t width, EcoCompare compare)
{
    CEcoLab1* self;

    if (me == 0) {
        return;
    }
    self = ECO_OWNER(me, CEcoLab1, sort);
    sort_state_run(&self->state, self->object.memory,
        base, count, width, compare);
}

static int16_t ECOCALLMETHOD sort_set_executor(IEcoSortAndSearch* me,
    uint32_t lanes, EcoSortExecute execute, void* context)
{
    CEcoLab1* self;

    if (me == 0) {
        return ERR_ECO_POINTER;
    }
    self = ECO_OWNER(me, CEcoLab1, sort);
    return sort_state_set_executor(&self->state, lanes, execute, context);
}

static int16_t ECOCALLMETHOD sort_status(IEcoSortAndSearch* me,
    EcoSortStats* statistics)
{
    CEcoLab1* self;

    if (me == 0) {
        return ERR_ECO_POINTER;
    }
    self = ECO_OWNER(me, CEcoLab1, sort);
    return sort_state_status(&self->state, statistics);
}

static IEcoSortAndSearchVTbl sort_vtable = {
    sort_query, sort_addref, sort_release,
    sort_run, sort_set_executor, sort_status
};

static int16_t ECOCALLMETHOD x_query(IEcoCalculatorX* me,
    const UGUID* iid, void** result)
{
    CEcoLab1* self = me != 0 ? ECO_OWNER(me, CEcoLab1, calc_x) : 0;

    return eco_public_query(self != 0 ? &self->object : 0, iid, result);
}

static uint32_t ECOCALLMETHOD x_addref(IEcoCalculatorX* me)
{
    return me != 0 ? eco_public_addref(&ECO_OWNER(me, CEcoLab1, calc_x)->object) : 0;
}

static uint32_t ECOCALLMETHOD x_release(IEcoCalculatorX* me)
{
    return me != 0 ? eco_public_release(&ECO_OWNER(me, CEcoLab1, calc_x)->object) : 0;
}

static int32_t ECOCALLMETHOD x_add(IEcoCalculatorX* me,
    int16_t left, int16_t right)
{
    CEcoLab1* self;

    if (me == 0) {
        return 0;
    }
    self = ECO_OWNER(me, CEcoLab1, calc_x);
    return self->parts.included_x->pVTbl->Addition(self->parts.included_x, left, right);
}

static int16_t ECOCALLMETHOD x_subtract(IEcoCalculatorX* me,
    int16_t left, int16_t right)
{
    CEcoLab1* self;

    if (me == 0) {
        return 0;
    }
    self = ECO_OWNER(me, CEcoLab1, calc_x);
    return self->parts.included_x->pVTbl->Subtraction(self->parts.included_x, left, right);
}

static IEcoCalculatorXVTbl x_vtable = {
    x_query, x_addref, x_release,
    x_add, x_subtract
};

static int16_t ECOCALLMETHOD y_query(IEcoCalculatorY* me,
    const UGUID* iid, void** result)
{
    CEcoLab1* self = me != 0 ? ECO_OWNER(me, CEcoLab1, calc_y) : 0;

    return eco_public_query(self != 0 ? &self->object : 0, iid, result);
}

static uint32_t ECOCALLMETHOD y_addref(IEcoCalculatorY* me)
{
    return me != 0 ? eco_public_addref(&ECO_OWNER(me, CEcoLab1, calc_y)->object) : 0;
}

static uint32_t ECOCALLMETHOD y_release(IEcoCalculatorY* me)
{
    return me != 0 ? eco_public_release(&ECO_OWNER(me, CEcoLab1, calc_y)->object) : 0;
}

static int32_t ECOCALLMETHOD y_multiply(IEcoCalculatorY* me,
    int16_t left, int16_t right)
{
    CEcoLab1* self;

    if (me == 0) {
        return 0;
    }
    self = ECO_OWNER(me, CEcoLab1, calc_y);
    return self->parts.included_y->pVTbl->Multiplication(self->parts.included_y, left, right);
}

static int16_t ECOCALLMETHOD y_divide(IEcoCalculatorY* me,
    int16_t left, int16_t right)
{
    CEcoLab1* self;

    if (me == 0) {
        return 0;
    }
    self = ECO_OWNER(me, CEcoLab1, calc_y);
    return self->parts.included_y->pVTbl->Division(self->parts.included_y, left, right);
}

static IEcoCalculatorYVTbl y_vtable = {
    y_query, y_addref, y_release,
    y_multiply, y_divide
};

static int16_t query_views(EcoObject* object, const UGUID* iid, void** result)
{
    CEcoLab1* self = (CEcoLab1*)object;

    if (IsEqualUGUID(iid, &IID_IEcoSortAndSearch)) {
        return eco_return_view(object, &self->sort, result);
    }
    if (IsEqualUGUID(iid, &IID_IEcoCalculatorX)) {
        if (self->parts.inner_b != 0) {
            return self->parts.inner_b->pVTbl->QueryInterface(
                self->parts.inner_b, iid, result);
        }
        return eco_return_view(object, &self->calc_x, result);
    }
    if (IsEqualUGUID(iid, &IID_IEcoCalculatorY)) {
        return eco_return_view(object, &self->calc_y, result);
    }
    return ERR_ECO_NOINTERFACE;
}

static void dispose(EcoObject* object)
{
    CEcoLab1* self = (CEcoLab1*)object;

    lab1_parts_close(&self->parts);
}

int16_t lab1_create(IEcoUnknown* system, IEcoUnknown* outer,
    const UGUID* iid, void** result, uint32_t configuration)
{
    EcoObject* object;
    CEcoLab1* self;
    IEcoInterfaceBus1* bus;
    int16_t status;

    status = eco_check_creation(system, outer, iid, result, 1);
    if (status != ERR_ECO_SUCCESS) {
        return status;
    }
    if (configuration > LAB1_INCLUDE_E) {
        return ERR_ECO_INVALIDARG;
    }
    status = eco_object_allocate(system, outer, sizeof(CEcoLab1),
        query_views, dispose, &object, &bus);
    if (status != ERR_ECO_SUCCESS) {
        return status;
    }
    self = (CEcoLab1*)object;
    self->sort.pVTbl = &sort_vtable;
    self->calc_x.pVTbl = &x_vtable;
    self->calc_y.pVTbl = &y_vtable;
    sort_state_initialize(&self->state);
    status = lab1_parts_open(&self->parts, bus, object->controller,
        (Lab1Composition)configuration);
    bus->pVTbl->Release(bus);
    if (status == ERR_ECO_SUCCESS && (self->parts.included_y == 0 ||
        (self->parts.included_x == 0 && self->parts.inner_b == 0))) {
        status = ERR_ECO_FAIL;
    }
    if (status != ERR_ECO_SUCCESS) {
        eco_object_discard(object);
        return status;
    }
    return eco_object_finish(object, iid, result);
}
