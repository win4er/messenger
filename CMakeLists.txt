cmake_minimum_required(VERSION 3.16)
project(P2P_Chat VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BUILD_TESTS "Build tests" OFF)
option(ENABLE_DEBUG "Enable debug output" ON)

if(ENABLE_DEBUG)
    add_compile_definitions(P2P_DEBUG)
    set(CMAKE_BUILD_TYPE Debug)
else()
    set(CMAKE_BUILD_TYPE Release)
endif()

# Флаги для GCC/Clang
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(
        -Wall
        -Wextra
        -Wpedantic
        -Wshadow
        -Wnon-virtual-dtor
        -Wold-style-cast
        -Wcast-align
        -Wunused
        -Woverloaded-virtual
        -Wconversion
        -Wsign-conversion
        -Wnull-dereference
        -Wdouble-promotion
        -Wformat=2
        -Werror=return-type
        -Werror=uninitialized
    )
    
    if(ENABLE_DEBUG)
        add_compile_options(-g -O0 -fsanitize=address,undefined)
        add_link_options(-fsanitize=address,undefined)
    else()
        add_compile_options(-O2 -DNDEBUG)
    endif()
endif()

# Поиск необходимых библиотек
find_package(Threads REQUIRED)

# Дополнительные библиотеки (пока не используются, но готовимся)
# find_package(Sodium)     # Для криптографии
# find_package(Curses)     # Для TUI
# find_package(FTXUI)      # Альтернативный TUI

add_executable(p2p_chat
    src/peer.cpp
    src/main.cpp
)

target_link_libraries(p2p_chat
    PRIVATE
    Threads::Threads
    # ${SODIUM_LIBRARIES}     # Потом добавим
    # ${CURSES_LIBRARIES}     # Потом добавим
)

target_include_directories(p2p_chat
    PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

set_target_properties(p2p_chat PROPERTIES
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)


if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

message(STATUS "P2P Chat Configuration:")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Debug mode: ${ENABLE_DEBUG}")
message(STATUS "  Build tests: ${BUILD_TESTS}")
