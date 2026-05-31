# vtui_graphics

**vtui_graphics** — лёгкая C++20 библиотека для построения текстового интерфейса (TUI) с полной
поддержкой Unicode (графемные кластеры, CJK, эмодзи, ZWJ-последовательности).

Цель — растрово-точный вывод произвольного Unicode-текста в терминал через ANSI escape-последовательности,
без ASCII-fallback'ов и без внешних зависимостей (Harfbuzz, ICU и т.д.). Терминал берёт на себя
финальное рендеринг графем — библиотека лишь правильно формирует кластеры.

## Статус

Активная разработка ядра. Работает:
- Двойная буферизация с грязным трекингом (dirty-флаги)
- Инкрементальный рендеринг (только изменённые ячейки)
- Графемные кластеры: base + до 7 extension codepoints inline в Cell
- Автоматическое определение ширины (1/2) для CJK и восточноазиатских символов
- Полноценная обработка trail-ячеек (skip при рендеринге)
- Win32-бэкенд (UTF-8, alternate screen, мышь)
- Rust-подобный `Result<T>` без исключений

Не реализовано:
- `src/input/`, `src/compositor/`, `src/widgets/` — стабы
- Событийный ввод не подключён
- Нет тестов

## Требования

- CMake >= 3.10
- Ninja
- MSVC (Windows)
- C++20

## Сборка

```powershell
cmake -S . -B build -G "Ninja" -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

## Архитектура

```
include/vtui/          Публичные заголовки
  buffer.hpp           ScreenBuffer (double buffer, dirty flags, set_cell/at/fill)
  renderer.hpp         Renderer (ANSI escape emission, batch flush)
  types.hpp            Cell, Coord, Size, Color, MaxClusterExt=7
  result.hpp           Result<T>, Result<void>
  event.hpp            KeyEvent, MouseEvent, ResizeEvent (стабы)

src/
  core/                Реализация буфера и рендерера (buffer.cpp, renderer.cpp)
  platform/            Абстракция платформы (pal.hpp, win32.cpp)
  util/                UTF-8 encode/decode, wcwidth, grapheme cluster_encode
  input/               Стаб (будущий ввод)
  compositor/          Стаб (будущий композитор)
  widgets/             Стаб (будущие виджеты)

example/main.cpp       Точка входа (заглушка, возвращает 0)
```

## Ключевые решения

- **Без ASCII-fallback**: терминал обязан быть UTF-8. Весь код транслитерации удалён.
- **Inline-кластеры**: `Cell` хранит `base + ext[7]` на стеке, без кучи и сентинелей.
- **Ширина авто-вычисляется**: `compute_width()` в `set_cell()` / `fill()` / `clear()`.
- **Trail-флаги**: ячейки, «занятые» широким символом, помечаются `trail` и пропускаются рендерером.
- **Bulk sync**: после `present()` фронт-буфер синхронизируется одним `std::copy` (было: per-cell
  присваивание в трёх местах).
- **Управляющие символы** (C0 без NUL, C1, DEL) молча дропаются рендерером.

Подробнее: `AGENTS.md`
