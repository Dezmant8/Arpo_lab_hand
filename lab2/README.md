# Лабораторная 2 — включение и агрегирование

**[Eco.Lab2](SourceFiles/CEcoLab2.c), создаваемый по `CID_EcoLab2`.** Он агрегирует расширенный [Eco.Lab1](SourceFiles/CEcoLab1.c) и включает. Сортировка возвращается как настоящий интерфейс внутреннего Lab1; арифметика доступна через собственные X/Y внешнего объекта. Переходы из любого публичного интерфейса ведут к одному Unknown Lab2.

Пять дополнительных конфигураций Lab1 необходимы для демонстрации всех компонентов A–E.

| CID / конфигурация | реализация |
|---|---|
| `CID_Lab1IncludedA` | Включение [A](Calculators/CalculatorA.c) для X и [D](Calculators/CalculatorD.c) для Y |
| `CID_Lab1AggregatedB` | Агрегирование [B](Calculators/CalculatorB.c) для X и включение D для Y; также конфигурация `CID_EcoLab1` |
| `CID_Lab1IncludedC` | Включение одного [C](Calculators/CalculatorC.c), X и Y получаются у одного экземпляра |
| `CID_Lab1IncludedD` | Включение одного D, который сам включает B для X и реализует Y |
| `CID_Lab1IncludedE` | Включение одного [E](Calculators/CalculatorE.c), который сам агрегирует B для X и реализует Y |
| `CID_EcoLab2` | Агрегирование Lab1 в конфигурации B/D, включение одного E для X/Y |



1. [Composition.c](SourceFiles/Composition.c): создание зависимостей и владение ссылками.
2.  [CEcoLab1.c](SourceFiles/CEcoLab1.c) и [CEcoLab2.c](SourceFiles/CEcoLab2.c): выдача интерфейсов и передача управляющего Unknown.
3.  [EcoObject.c](../common/src/EcoObject.c): неделегируемый Unknown, делегирование публичных методов, транзакционное создание, освобождение.
4.  [EcoFactory.c](../common/src/EcoFactory.c) и [Registry.c](SourceFiles/Registry.c): фабрики A–E, профили Lab1, Lab2 и откат регистрации.
5.  [Demo.c](Demo.c): примеры арифметики, сортировка, получение X через Sort и Y через X, сравнение Unknown.
6. запускаются тесты [TestCom.c](../tests/TestCom.c), [TestFailures.c](../tests/TestFailures.c) и [TestSorting.c](../tests/TestSorting.c) через общий [TestMain.c](../tests/TestMain.c).

```bash
python3 scripts/lab.py demo --lab 2
python3 scripts/lab.py test --lab 2
python3 scripts/lab.py benchmark --lab 2
```

Публичные контракты: [X](../include/IEcoCalculatorX.h), [Y](../include/IEcoCalculatorY.h), [Sort](../include/IEcoSortAndSearch.h), [Lab2](../include/IEcoLab2.h); идентификаторы — [LabIds.h](../include/LabIds.h). `IEcoLab2::GetVersion()` возвращает **1** — версию контракта.


