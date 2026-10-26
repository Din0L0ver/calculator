# Calculator

Консольный калькулятор на C++17 с динамически подключаемыми плагинами.

## Возможности

- операции `+`, `-`, `*`, `/` и круглые скобки;
- правильный приоритет операций;
- загрузка функций из библиотек в папке `plugins`;
- добавление новых функций без пересборки `calc.exe`;
- обработка ошибок выражений и загрузки плагинов.

В проекте есть три плагина:

| Исходный файл | Файл после сборки на Windows | Операция |
| --- | --- | --- |
| `plugins/Sin.cpp` | `funcsin.dll` | `sin(x)` в градусах |
| `plugins/Power.cpp` | `funcdeg.dll` | `x^y` |
| `plugins/Ln.cpp` | `funcln.dll` | `ln(x)` |

## Сборка

Требуются Visual Studio 2022 и CMake 3.16 или новее.

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DBUILD_TESTING=OFF
cmake --build build --config Release
```

## Запуск

Запускать программу нужно из папки `build/bin`, потому что каталог `plugins` ищется относительно текущей папки.

```powershell
cd build/bin
.\calc.exe "16 + 4 * (3 - 1)"
.\calc.exe "2^4 + sin(90)"
.\calc.exe "ln(-100)"
```

Результат:

```text
24
17
Error: ln: ln requires a positive argument
