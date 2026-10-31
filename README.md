# Лабораторные работы по COM и ACOM

 **Битонная сортировка**, **включение и агрегирование компонентов**. В репозитории находятся компоненты, консольные клиенты и независимые тесты. 

## Что и где

**Лабораторная 1, тема 64:** основной компонент — [`lab1/SourceFiles/CEcoLab1.c`](lab1/SourceFiles/CEcoLab1.c), алгоритм — [`common/src/BitonicSort.c`](common/src/BitonicSort.c). Он сортирует произвольные элементы , поддерживает неполные степени двойки, стабильность и параллельное исполнение стадий.

**Лабораторная 2, :** основной внешний компонент — [`lab2/SourceFiles/CEcoLab2.c`](lab2/SourceFiles/CEcoLab2.c). Он агрегирует расширенный [`Eco.Lab1`](lab2/SourceFiles/CEcoLab1.c) и включает калькулятор E. Пять конфигураций Lab1 показывают включение и агрегирование **всех A–E**. [Описания лабораторной 2](lab2/README.md).

| Части | Прямые ссылки |
|---|---|
| Алгоритм и параллельные стадии | [BitonicSort.c](common/src/BitonicSort.c), [ParallelExecutor.c](client/ParallelExecutor.c) |
| Публичные интерфейсы и идентификаторы | [IEcoSortAndSearch.h](include/IEcoSortAndSearch.h), [IEcoLab2.h](include/IEcoLab2.h), [X](include/IEcoCalculatorX.h), [Y](include/IEcoCalculatorY.h), [IID/CID](include/LabIds.h) |
| Unknown, счётчики, создание и уничтожение | [EcoObject.h](common/include/EcoObject.h), [EcoObject.c](common/src/EcoObject.c) |
| Фабрика и регистрация в настоящей шине Eco | [EcoFactory.c](common/src/EcoFactory.c), [реестр Lab1](lab1/SourceFiles/Registry.c), [реестр Lab2](lab2/SourceFiles/Registry.c) |
| Композиция второй работы | [Composition.c](lab2/SourceFiles/Composition.c), [A](lab2/Calculators/CalculatorA.c), [B](lab2/Calculators/CalculatorB.c), [C](lab2/Calculators/CalculatorC.c), [D](lab2/Calculators/CalculatorD.c), [E](lab2/Calculators/CalculatorE.c) |
| Консольные клиенты через EcoMain | [Lab1](lab1/Demo.c), [Lab2](lab2/Demo.c), [подключение SDK](client/ClientRuntime.c) |
| Независимые проверки | [COM и агрегирование](tests/TestCom.c), [сортировка и эталон qsort](tests/TestSorting.c), [отказы и время жизни](tests/TestFailures.c), [тестовая среда](tests/FakeEnvironment.c), [точка входа](tests/TestMain.c) |
| Сборки  | [libEcoLab1.a](artifacts/macos-arm64/libEcoLab1.a), [libEcoLab2.a](artifacts/macos-arm64/libEcoLab2.a) |

## Подготовка и запуск

Нужен SDK не входящий в репозиторий

```bash
python3 scripts/lab.py setup --archive "/путь/к/Eco.Framework_20260918.zip"
python3 scripts/lab.py demo
python3 scripts/lab.py test
python3 scripts/lab.py benchmark
```

Для запуска одной работы `--lab 1` или `--lab 2`. Результаты сохраняются в `build/`.

##  Результаты

| Проверка | Лабораторная 1 | Лабораторная 2 |
|---|---:|---:|
| Выполнено проверок утверждений | 12 096 | 62 844 |
| Ошибки утверждений | 0 | 0 |
| Сценарии сравнения сортировки с эталоном | 1 680 | 1 680 |
| Измерения времени с проверкой результата | 336 | 336 |



Сборки Lab1 и Lab2 используют отдельные реестры и имеют общие имена экспортируемых функций. Вторая уже содержит расширенный Lab1 и тот же общий алгоритм.

