# vtui_graphics — AGENTS.md

## Build

```powershell
cmake -S . -B build -G "Ninja" -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

- Requires: CMake >= 3.10, Ninja, MSVC (Windows only).
- C++20 required (`CMAKE_CXX_STANDARD 20`).
- Already-built artifacts (`.ninja` files, `libvtui_core.a`, `main.exe`) are in `build/` — just run `cmake --build build` after edits.
- No `.gitignore` — build artifacts are tracked.

## Targets

| Target | Type | Description |
|--------|------|-------------|
| `vtui_core` | static library | Core rendering library |
| `main` | executable | Example app (links vtui_core) |

Source: `CMakeLists.txt:7-28`

## Project layout

- `include/vtui/` — public headers: `buffer.hpp`, `renderer.hpp`, `types.hpp`, `result.hpp`, `event.hpp`
- `src/core/` — core rendering (`buffer.cpp`, `renderer.cpp`)
- `src/platform/` — platform abstraction layer (`pal.hpp`, `win32.cpp`)
- `src/util/` — UTF-8 utilities, wcwidth, transliteration (`utf8.cpp`, `utf8.hpp`, `wcwidth.cpp`, `wcwidth.hpp`, `translit.cpp`, `translit.hpp`)
- `src/compositor/`, `src/input/`, `src/widgets/` — empty directories (planned, not yet implemented)
- `example/` — single `main.cpp` consuming the library

## Code style

- Google style (`.clang-format`): 2-space indent, 80 col limit, left-aligned pointers/references, Attach braces.
- Run `clang-format -i <file>` before committing.

## Key architectural facts

- **Rust-style error handling**: `Result<T>` / `Result<void>` with `.unwrap()`, `.expect()`, `.map()`, `.and_then()` — defined in `include/vtui/result.hpp`.
- **Double-buffered screen**: `ScreenBuffer` uses front/back buffers with per-cell dirty tracking (`is_dirty`, `mark_dirty`). Draw to `at()`/`set_cell()`, flush with `present()`.
- **Renderer**: emits ANSI escape sequences to stdout, batching output in a 4096-byte internal buffer. Only emits diffs (skips cells unchanged since last `present()`).
- **Platform abstraction**: `pal.h` declares `vtui_pal_init/shutdown`, `vtui_pal_write_input/output`, `vtui_pal_poll_raw_event`. Currently only win32 implementation exists.
- **Event system**: `KeyEvent`, `MouseEvent`, `ResizeEvent` types in `event.hpp` — but `input/` and `compositor/` are stubs; event polling is not wired up yet.

## Quirks & gotchas

- `src/` is a private include directory (`target_include_directories(vtui_core PRIVATE src)`) — internal headers like `platform/pal.h` use `#include "platform/pal.h"` from any source file.
- Buffer sizing constants are in `include/vtui/renderer.hpp`: `BufSize` (4096), `FlushThreshold` (64).
- Codepoints are `char32_t`. UTF-8 encode/decode helpers in `src/util/utf8.hpp`.
- Colors: 16 named ANSI colors + `Default` (255), stored in `Color` enum (`include/vtui/types.hpp`).
- No tests, no CI, no formatter/linter scripts beyond `.clang-format`.
