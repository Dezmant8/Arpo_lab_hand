#include "TestSupport.h"

static int has_kind(const ViewKind* kinds, uint32_t count, ViewKind kind)
{
    uint32_t index;

    for (index = 0; index < count; ++index) {
        if (kinds[index] == kind) {
            return 1;
        }
    }
    return 0;
}

static void check_matrix(IEcoUnknown* identity, const ViewKind* kinds,
    uint32_t count)
{
    TestView first;
    TestView second;
    TestView third;
    IEcoUnknown* canonical;
    void* result;
    uint32_t pass;
    uint32_t from;
    uint32_t through;
    uint32_t to;
    int candidate;

    for (pass = 0; pass < 2; ++pass) {
        for (from = 0; from < count; ++from) {
            first.kind = kinds[from];
            first.value = 0;
            REQUIRE(identity->pVTbl->QueryInterface(identity, view_iid(first.kind),
                &first.value) == ERR_ECO_SUCCESS);
            REQUIRE(first.value != 0);
            canonical = 0;
            CHECK(view_query(first, &IID_IEcoUnknown, (void**)&canonical) == 0);
            CHECK(canonical == identity);
            if (canonical != 0) {
                canonical->pVTbl->Release(canonical);
            }
            result = identity;
            CHECK(view_query(first, &IID_UnsupportedTest, &result) == ERR_ECO_NOINTERFACE);
            CHECK(result == 0);
            result = identity;
            CHECK(view_query(first, 0, &result) == ERR_ECO_POINTER);
            CHECK(result == 0);
            CHECK(view_query(first, view_iid(first.kind), 0) == ERR_ECO_POINTER);
            for (candidate = VIEW_UNKNOWN; candidate <= VIEW_LAB2; ++candidate) {
                if (!has_kind(kinds, count, (ViewKind)candidate)) {
                    result = identity;
                    CHECK(view_query(first, view_iid((ViewKind)candidate),
                        &result) == ERR_ECO_NOINTERFACE);
                    CHECK(result == 0);
                }
            }
            exercise_view(first);
            for (through = 0; through < count; ++through) {
                second.kind = kinds[through];
                second.value = 0;
                REQUIRE(view_query(first, view_iid(second.kind), &second.value) == 0);
                REQUIRE(second.value != 0);
                for (to = 0; to < count; ++to) {
                    third.kind = kinds[to];
                    third.value = 0;
                    REQUIRE(view_query(second, view_iid(third.kind), &third.value) == 0);
                    REQUIRE(third.value != 0);
                    canonical = 0;
                    CHECK(view_query(third, &IID_IEcoUnknown, (void**)&canonical) == 0);
                    CHECK(canonical == identity);
                    if (canonical != 0) {
                        canonical->pVTbl->Release(canonical);
                    }
                    view_release(third);
                }
                view_release(second);
            }
            view_release(first);
        }
    }
}

void test_com_matrix(IEcoInterfaceBus1* bus)
{
    IEcoUnknown* identity;
    const UGUID* cid;
    ViewKind kinds[5];
    TestView last;
    uint32_t index;
    uint32_t count;
    uint32_t held;

    for (index = 0; index < lab_factory_count(); ++index) {
        REQUIRE(lab_factory_at(index, &cid) != 0);
        count = component_views(cid, kinds);
        identity = 0;
        REQUIRE(bus->pVTbl->QueryComponent(bus, cid, 0, &IID_IEcoUnknown,
            (void**)&identity) == 0);
        REQUIRE(identity != 0);
        check_matrix(identity, kinds, count);
        identity->pVTbl->Release(identity);

        for (held = 0; held < count; ++held) {
            last.kind = kinds[held];
            last.value = 0;
            REQUIRE(bus->pVTbl->QueryComponent(bus, cid, 0, view_iid(last.kind),
                &last.value) == 0);
            REQUIRE(last.value != 0);
            view_addref(last);
            view_release(last);
            exercise_view(last);
            view_release(last);
        }
    }
}

typedef struct TestOuter {
    IEcoUnknown unknown;
    uint32_t references;
    IEcoUnknown* inner;
    int destroyed;
} TestOuter;

static uint32_t ECOCALLMETHOD outer_addref(IEcoUnknown* me)
{
    return ++((TestOuter*)me)->references;
}

static uint32_t ECOCALLMETHOD outer_release(IEcoUnknown* me)
{
    TestOuter* self = (TestOuter*)me;
    IEcoUnknown* inner;

    CHECK(self->references != 0);
    if (--self->references == 0) {
        inner = self->inner;
        self->inner = 0;
        self->destroyed = 1;
        if (inner != 0) {
            inner->pVTbl->Release(inner);
        }
    }
    return self->references;
}

static int16_t ECOCALLMETHOD outer_query(IEcoUnknown* me,
    const UGUID* iid, void** result)
{
    TestOuter* self = (TestOuter*)me;

    if (result == 0) {
        return ERR_ECO_POINTER;
    }
    *result = 0;
    if (iid == 0) {
        return ERR_ECO_POINTER;
    }
    if (IsEqualUGUID(iid, &IID_IEcoUnknown)) {
        *result = me;
        outer_addref(me);
        return 0;
    }
    if (self->inner == 0) {
        return ERR_ECO_NOINTERFACE;
    }
    return self->inner->pVTbl->QueryInterface(self->inner, iid, result);
}

static IEcoUnknownVTbl outer_vtable = {
    outer_query, outer_addref, outer_release
};

void test_external_aggregation(IEcoInterfaceBus1* bus)
{
    TestOuter outer;
    const UGUID* cid;
    IEcoComponentFactory* factory;
    ViewKind kinds[5];
    TestView last;
    IEcoUnknown* canonical;
    IEcoUnknown* private_unknown;
    uint32_t index;
    uint32_t held;
    uint32_t count;
    void* result;
    int16_t status;

    for (index = 0; index < lab_factory_count(); ++index) {
        factory = lab_factory_at(index, &cid);
        REQUIRE(factory != 0);
        count = component_views(cid, kinds);
        for (held = 1; held < count; ++held) {
            outer.unknown.pVTbl = &outer_vtable;
            outer.references = 1;
            outer.inner = 0;
            outer.destroyed = 0;
            result = &outer;
            status = bus->pVTbl->QueryComponent(bus, cid, &outer.unknown,
                view_iid(kinds[held]), &result);
            CHECK(status == ERR_ECO_NOAGGREGATION ||
                status == ERR_ECO_COMPONENT_NOTFOUND);
            CHECK(result == 0);
            status = bus->pVTbl->QueryComponent(bus, cid, &outer.unknown,
                &IID_IEcoUnknown, (void**)&outer.inner);
            if (!component_aggregatable(cid)) {
                CHECK(status == ERR_ECO_NOAGGREGATION ||
                    status == ERR_ECO_COMPONENT_NOTFOUND);
                CHECK(outer.inner == 0);
                CHECK(outer.references == 1);
                continue;
            }
            REQUIRE(status == 0 && outer.inner != 0);
            CHECK(outer.references == 1);
            private_unknown = 0;
            CHECK(outer.inner->pVTbl->QueryInterface(outer.inner,
                &IID_IEcoUnknown, (void**)&private_unknown) == 0);
            CHECK(private_unknown == outer.inner);
            CHECK(private_unknown != &outer.unknown);
            private_unknown->pVTbl->Release(private_unknown);
            check_matrix(&outer.unknown, kinds, count);
            last.kind = kinds[held];
            last.value = 0;
            REQUIRE(outer.inner->pVTbl->QueryInterface(outer.inner,
                view_iid(last.kind), &last.value) == 0);
            CHECK(outer.references == 2);
            canonical = 0;
            CHECK(view_query(last, &IID_IEcoUnknown, (void**)&canonical) == 0);
            CHECK(canonical == &outer.unknown);
            canonical->pVTbl->Release(canonical);
            outer_release(&outer.unknown);
            CHECK(!outer.destroyed);
            exercise_view(last);
            view_release(last);
            CHECK(outer.destroyed);
            CHECK(outer.references == 0);
            CHECK(outer.inner == 0);
        }
    }
}
