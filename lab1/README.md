# Лабораторная 1 — битонная сортировка

Основная реализация — [CEcoLab1.c](SourceFiles/CEcoLab1.c) вместе с общим [BitonicSort.c](../common/src/BitonicSort.c). Для его сборки  `--lab 1`.

| Часть работы | Где проверять |
|---|---|
| Универсальный контракт qsort и управление исполнителем | [IEcoSortAndSearch.h](../include/IEcoSortAndSearch.h) |
| Сеть сравнения индексов и атомарное применение результата | [BitonicSort.c](../common/src/BitonicSort.c) |
| Интерфейс объекта, QI, AddRef, Release | [CEcoLab1.c](SourceFiles/CEcoLab1.c), [общая реализация времени жизни](../common/src/EcoObject.c) |
| Фабрика и регистрация | [EcoFactory.c](../common/src/EcoFactory.c), [Registry.c](SourceFiles/Registry.c) |
| Четыре потока и барьер каждой стадии | [ParallelExecutor.c](../client/ParallelExecutor.c) |
| Рабочий пример | [Demo.c](Demo.c) |
| Эталон, граничные случаи, стабильность и типы | [TestSorting.c](../tests/TestSorting.c) |
| COM и отказы | [TestCom.c](../tests/TestCom.c), [TestFailures.c](../tests/TestFailures.c) |
| Измерение времени | [Benchmark.c](../client/Benchmark.c) |

```bash
python3 scripts/lab.py demo --lab 1
python3 scripts/lab.py test --lab 1
python3 scripts/lab.py benchmark --lab 1
```

Базовый Lab1 предоставляет только `IEcoUnknown` и `IEcoSortAndSearch`, явно отклоняет внешнее агрегирование. Поддержка агрегирования добавлена в [версии Lab1 для второй работы](../lab2/SourceFiles/CEcoLab1.c), не изменяя IID или ABI сортировки.

Сортируются индексы исходных элементов. Дополнение до степени двойки выполняется служебными индексами; исходные данные не дополняются фиктивными значениями. Для равных ключей используется исходная позиция, поэтому порядок стабилен. На ошибке выделения памяти или исполнения вход остаётся неизменным. Сложность — O(N log² N) сравнений и O(4N + n·width) байт временной памяти, где N — ближайшая сверху степень двойки.


