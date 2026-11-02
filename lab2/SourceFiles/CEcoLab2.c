#include "EcoObject.h"
#include "LabConstructors.h"
#include "IEcoLab2.h"
#include "IEcoSortAndSearch.h"
#include "IEcoCalculatorX.h"
#include "IEcoCalculatorY.h"
#include "CalculatorIds.h"

typedef struct CEcoLab2 {
    EcoObject object;
    IEcoLab2 lab2;
    IEcoCalculatorX calc_x;
    IEcoCalculatorY calc_y;
    IEcoUnknown* inner_lab1;
    IEcoCalculatorX* included_x;
    IEcoCalculatorY* included_y;
} CEcoLab2;

static int16_t ECOCALLMETHOD x_query(IEcoCalculatorX* me,
    const UGUID* iid, void** result)
{
    CEcoLab2* self = me != 0 ? ECO_OWNER(me, CEcoLab2, calc_x) : 0;

    return eco_public_query(self != 0 ? &self->object : 0, iid, result);
}

static uint32_t ECOCALLMETHOD x_addref(IEcoCalculatorX* me)
{
    return me != 0 ? eco_public_addref(&ECO_OWNER(me, CEcoLab2, calc_x)->object) : 0;
}

static uint32_t ECOCALLMETHOD x_release(IEcoCalculatorX* me)
{
    return me != 0 ? eco_public_release(&ECO_OWNER(me, CEcoLab2, calc_x)->object) : 0;
}

static int32_t ECOCALLMETHOD x_add(IEcoCalculatorX* me,
    int16_t left, int16_t right)
{
    CEcoLab2* self;

    if (me == 0) {
        return 0;
    }
    self = ECO_OWNER(me, CEcoLab2, calc_x);
    return self->included_x->pVTbl->Addition(self->included_x, left, right);
}

static int16_t ECOCALLMETHOD x_subtract(IEcoCalculatorX* me,
    int16_t left, int16_t right)
{
    CEcoLab2* self;

    if (me == 0) {
        return 0;
    }
    self = ECO_OWNER(me, CEcoLab2, calc_x);
    return self->included_x->pVTbl->Subtraction(self->included_x, left, right);
}

static IEcoCalculatorXVTbl x_vtable = {
    x_query, x_addref, x_release,
    x_add, x_subtract
};

static int16_t ECOCALLMETHOD y_query(IEcoCalculatorY* me,
    const UGUID* iid, void** result)
{
    CEcoLab2* self = me != 0 ? ECO_OWNER(me, CEcoLab2, calc_y) : 0;

    return eco_public_query(self != 0 ? &self->object : 0, iid, result);
}

static uint32_t ECOCALLMETHOD y_addref(IEcoCalculatorY* me)
{
    return me != 0 ? eco_public_addref(&ECO_OWNER(me, CEcoLab2, calc_y)->object) : 0;
}

static uint32_t ECOCALLMETHOD y_release(IEcoCalculatorY* me)
{
    return me != 0 ? eco_public_release(&ECO_OWNER(me, CEcoLab2, calc_y)->object) : 0;
}

static int32_t ECOCALLMETHOD y_multiply(IEcoCalculatorY* me,
    int16_t left, int16_t right)
{
    CEcoLab2* self;

    if (me == 0) {
        return 0;
    }
    self = ECO_OWNER(me, CEcoLab2, calc_y);
    return self->included_y->pVTbl->Multiplication(self->included_y, left, right);
}

static int16_t ECOCALLMETHOD y_divide(IEcoCalculatorY* me,
    int16_t left, int16_t right)
{
    CEcoLab2* self;

    if (me == 0) {
        return 0;
    }
    self = ECO_OWNER(me, CEcoLab2, calc_y);
    return self->included_y->pVTbl->Division(self->included_y, left, right);
}

static IEcoCalculatorYVTbl y_vtable = {
    y_query, y_addref, y_release,
    y_multiply, y_divide
};

static int16_t ECOCALLMETHOD lab2_query(IEcoLab2* me,
    const UGUID* iid, void** result)
{
    CEcoLab2* self = me != 0 ? ECO_OWNER(me, CEcoLab2, lab2) : 0;

    return eco_public_query(self != 0 ? &self->object : 0, iid, result);
}

static uint32_t ECOCALLMETHOD lab2_addref(IEcoLab2* me)
{
    return me != 0 ? eco_public_addref(&ECO_OWNER(me, CEcoLab2, lab2)->object) : 0;
}

static uint32_t ECOCALLMETHOD lab2_release(IEcoLab2* me)
{
    return me != 0 ? eco_public_release(&ECO_OWNER(me, CEcoLab2, lab2)->object) : 0;
}

static uint32_t ECOCALLMETHOD lab2_version(IEcoLab2* me)
{
    return me != 0 ? 1 : 0;
}

static IEcoLab2VTbl lab2_vtable = {
    lab2_query, lab2_addref, lab2_release, lab2_version
};

static int16_t query_views(EcoObject* object, const UGUID* iid, void** result)
{
    CEcoLab2* self = (CEcoLab2*)object;

    if (IsEqualUGUID(iid, &IID_IEcoLab2)) {
        return eco_return_view(object, &self->lab2, result);
    }
    if (self->included_x != 0 && IsEqualUGUID(iid, &IID_IEcoCalculatorX)) {
        return eco_return_view(object, &self->calc_x, result);
    }
    if (self->included_y != 0 && IsEqualUGUID(iid, &IID_IEcoCalculatorY)) {
        return eco_return_view(object, &self->calc_y, result);
    }
    if (IsEqualUGUID(iid, &IID_IEcoSortAndSearch) ||
        IsEqualUGUID(iid, &IID_IEcoCalculatorX) || IsEqualUGUID(iid, &IID_IEcoCalculatorY)) {
        return self->inner_lab1->pVTbl->QueryInterface(self->inner_lab1, iid, result);
    }
    return ERR_ECO_NOINTERFACE;
}

static void dispose(EcoObject* object)
{
    CEcoLab2* self = (CEcoLab2*)object;
    IEcoUnknown* inner_lab1 = self->inner_lab1;
    IEcoCalculatorX* included_x = self->included_x;
    IEcoCalculatorY* included_y = self->included_y;

    self->inner_lab1 = 0;
    self->included_x = 0;
    self->included_y = 0;
    if (included_y != 0) {
        included_y->pVTbl->Release(included_y);
    }
    if (included_x != 0) {
        included_x->pVTbl->Release(included_x);
    }
    if (inner_lab1 != 0) {
        inner_lab1->pVTbl->Release(inner_lab1);
    }
}

int16_t lab2_create(IEcoUnknown* system, IEcoUnknown* outer,
    const UGUID* iid, void** result)
{
    EcoObject* object;
    CEcoLab2* self;
    IEcoInterfaceBus1* bus;
    int16_t status;

    status = eco_check_creation(system, outer, iid, result, 0);
    if (status != ERR_ECO_SUCCESS) {
        return status;
    }
    status = eco_object_allocate(system, 0, sizeof(CEcoLab2),
        query_views, dispose, &object, &bus);
    if (status != ERR_ECO_SUCCESS) {
        return status;
    }
    self = (CEcoLab2*)object;
    self->lab2.pVTbl = &lab2_vtable;
    self->calc_x.pVTbl = &x_vtable;
    self->calc_y.pVTbl = &y_vtable;
    self->inner_lab1 = 0;
    self->included_x = 0;
    self->included_y = 0;
    status = bus->pVTbl->QueryComponent(bus, &CID_EcoLab1,
        object->controller, &IID_IEcoUnknown, (void**)&self->inner_lab1);
    if (status != ERR_ECO_SUCCESS || self->inner_lab1 == 0) {
        goto failed;
    }
    status = bus->pVTbl->QueryComponent(bus, &CID_EcoCalculatorE, 0,
        &IID_IEcoCalculatorX, (void**)&self->included_x);
    if (status == ERR_ECO_SUCCESS && self->included_x != 0) {
        status = self->included_x->pVTbl->QueryInterface(self->included_x,
            &IID_IEcoCalculatorY, (void**)&self->included_y);
        if (status != ERR_ECO_SUCCESS || self->included_y == 0) {
            goto failed;
        }
    }
    else if (status != ERR_ECO_COMPONENT_NOTFOUND) {
        goto failed;
    }
    bus->pVTbl->Release(bus);
    return eco_object_finish(object, iid, result);

failed:
    bus->pVTbl->Release(bus);
    eco_object_discard(object);
    return status != ERR_ECO_SUCCESS ? status : ERR_ECO_FAIL;
}
