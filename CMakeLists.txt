cmake_minimum_required(VERSION 3.10)
project(vtui LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(vtui_core
    # Платформа
    src/platform/win32.cpp

    # Утилиты
    src/util/utf8.cpp

    # C++ ядро
    src/core/buffer.cpp
    src/core/renderer.cpp
    src/core/error.cpp
)

target_include_directories(vtui_core PUBLIC include)
target_include_directories(vtui_core PRIVATE src)

target_compile_features(vtui_core PUBLIC cxx_std_20)

add_executable(main
    example/main.cpp
)

target_link_libraries(main PRIVATE vtui_core)
target_include_directories(main PRIVATE src)