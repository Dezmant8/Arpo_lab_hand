# Битонная сортировка

Начните с [UnitTest.c](UnitTest.c). Входной массив содержит отрицательное число и два одинаковых ключа. Его длина пять проверяет случай, когда число элементов не является степенью двойки.

[Интерфейс](../include/IEcoSortAndSearch.h) принимает внешний массив, размер элемента и компаратор. [Eco.Lab1](SourceFiles/CEcoLab1.c) вызывает [битонную сортировку](../common/src/BitonicSort.c). [Demo.c](Demo.c) запускает её на четырёх потоках, [Benchmark.c](../client/Benchmark.c) измеряет время относительно `qsort`.

```sh
python3 scripts/lab.py test --lab 1
python3 scripts/lab.py demo --lab 1
python3 scripts/lab.py benchmark --lab 1
```

[Расширенная версия для второй работы](../lab2/SourceFiles/CEcoLab1.c) сохраняет тот же CID и интерфейс сортировки и поддерживает внешнее агрегирование.
