#include "EcoObject.h"
#include "SortState.h"
#include "LabConstructors.h"

typedef struct CEcoLab1 {
    EcoObject object;
    IEcoSortAndSearch sort;
    SortState state;
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

static int16_t query_views(EcoObject* object, const UGUID* iid, void** result)
{
    CEcoLab1* self = (CEcoLab1*)object;

    if (IsEqualUGUID(iid, &IID_IEcoSortAndSearch)) {
        return eco_return_view(object, &self->sort, result);
    }
    return ERR_ECO_NOINTERFACE;
}

static void dispose(EcoObject* object)
{
    (void)object;
}

int16_t lab1_create(IEcoUnknown* system, IEcoUnknown* outer,
    const UGUID* iid, void** result)
{
    EcoObject* object;
    CEcoLab1* self;
    int16_t status;

    status = eco_check_creation(system, outer, iid, result, 0);
    if (status != ERR_ECO_SUCCESS) {
        return status;
    }
    status = eco_object_allocate(system, 0, sizeof(CEcoLab1),
        query_views, dispose, &object, 0);
    if (status != ERR_ECO_SUCCESS) {
        return status;
    }
    self = (CEcoLab1*)object;
    self->sort.pVTbl = &sort_vtable;
    sort_state_initialize(&self->state);
    return eco_object_finish(object, iid, result);
}
