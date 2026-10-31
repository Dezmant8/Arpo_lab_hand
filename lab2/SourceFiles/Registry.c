#include "EcoFactory.h"
#include "LabConstructors.h"
#include "LabRegistry.h"

static EcoFactory factory_a = ECO_FACTORY("Calculator A", calculator_a_create, 0);
static EcoFactory factory_b = ECO_FACTORY("Calculator B", calculator_b_create, 0);
static EcoFactory factory_c = ECO_FACTORY("Calculator C", calculator_c_create, 0);
static EcoFactory factory_d = ECO_FACTORY("Calculator D", calculator_d_create, 0);
static EcoFactory factory_e = ECO_FACTORY("Calculator E", calculator_e_create, 0);
static EcoFactory factory_ad = ECO_FACTORY("Lab1 includes A and D", lab1_create,
    LAB1_INCLUDE_A_D);
static EcoFactory factory_bd = ECO_FACTORY("Lab1 aggregates B and includes D",
    lab1_create, LAB1_AGGREGATE_B_INCLUDE_D);
static EcoFactory factory_ic = ECO_FACTORY("Lab1 includes C", lab1_create,
    LAB1_INCLUDE_C);
static EcoFactory factory_id = ECO_FACTORY("Lab1 includes D", lab1_create,
    LAB1_INCLUDE_D);
static EcoFactory factory_ie = ECO_FACTORY("Lab1 includes E", lab1_create,
    LAB1_INCLUDE_E);
static EcoFactory factory_lab2 = ECO_FACTORY("Eco.Lab2", lab2_create, 0);

typedef struct RegisteredComponent {
    const UGUID* cid;
    EcoFactory* factory;
} RegisteredComponent;

static const RegisteredComponent components[] = {
    {&CID_EcoCalculatorA, &factory_a},
    {&CID_EcoCalculatorB, &factory_b},
    {&CID_EcoCalculatorC, &factory_c},
    {&CID_EcoCalculatorD, &factory_d},
    {&CID_EcoCalculatorE, &factory_e},
    {&CID_Lab1IncludedA, &factory_ad},
    {&CID_Lab1AggregatedB, &factory_bd},
    {&CID_Lab1IncludedC, &factory_ic},
    {&CID_Lab1IncludedD, &factory_id},
    {&CID_Lab1IncludedE, &factory_ie},
    {&CID_EcoLab2, &factory_lab2},
    {&CID_EcoLab1, &factory_bd}
};

uint32_t lab_factory_count(void)
{
    return (uint32_t)(sizeof(components) / sizeof(components[0]));
}

IEcoComponentFactory* lab_factory_at(uint32_t index, const UGUID** cid)
{
    if (index >= lab_factory_count() || cid == 0) {
        return 0;
    }
    *cid = components[index].cid;
    return &components[index].factory->iface;
}

int16_t lab_register_components(IEcoInterfaceBus1* bus)
{
    uint32_t index;
    int16_t status;

    if (bus == 0) {
        return ERR_ECO_POINTER;
    }
    for (index = 0; index < lab_factory_count(); ++index) {
        status = bus->pVTbl->RegisterComponent(bus, components[index].cid,
            (IEcoUnknown*)&components[index].factory->iface);
        if (status != ERR_ECO_SUCCESS) {
            while (index != 0) {
                --index;
                bus->pVTbl->UnRegisterComponent(bus, components[index].cid);
            }
            return status;
        }
    }
    return ERR_ECO_SUCCESS;
}
