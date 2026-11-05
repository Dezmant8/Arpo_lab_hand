# Лабораторные работы по COM и ACOM

| Работа | Что проверять | Реализация |
|---|---|---|
| 1. Битонная сортировка | [UnitTest](lab1/UnitTest.c), [демонстрация на четырёх потоках](lab1/Demo.c) | [Eco.Lab1](lab1/SourceFiles/CEcoLab1.c), [алгоритм](common/src/BitonicSort.c) |
| 2. Включение и агрегирование | [UnitTest](lab2/UnitTest.c), [состав запусков](lab2/README.md) | **[Eco.Lab2](lab2/SourceFiles/CEcoLab2.c)** агрегирует [Eco.Lab1](lab2/SourceFiles/CEcoLab1.c). [Подключение калькуляторов](lab2/SourceFiles/Composition.c) |

[Клиент](client/UnitTestMain.c) запрашивает у шины одну лабораторную по CID. Остальные интерфейсы он получает через `QueryInterface`. [Скрипт сборки и проверки](scripts/lab.py) раскладывает библиотеки по каталогам и запускает одну и ту же программу для разных сочетаний калькуляторов.

## Запуск на Mac с Apple Silicon

Из корня репозитория:

```sh
python3 scripts/lab.py setup --archive "/путь/к/Eco.Framework_20260918.zip"
git clone https://github.com/peerf-eco/Eco.Education.git .eco/Eco.Education
git -C .eco/Eco.Education checkout --detach 7f1664b0489bcea6d5b211807ea5cb0ce0e1068e
python3 scripts/lab.py test
```

Для одной работы `--lab 1` или `--lab 2`. Первой лабораторной достаточно SDK. Если Eco.Education уже скачан, то просто добавить `--education "/путь/к/Eco.Education"` к команде второй работы.

`demo` запускает пример, `benchmark` сравнивает сортировку с `qsort`. Результаты проверки лежат в `build/labN/dynamic-test/result.json`, замеры — в `build/labN/dynamic-benchmark/benchmark.csv`. Готовые [библиотеки первой](artifacts/macos-arm64/lab1) и [второй](artifacts/macos-arm64/lab2) работ собраны для arm64.
