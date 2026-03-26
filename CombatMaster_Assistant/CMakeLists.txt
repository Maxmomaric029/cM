cmake_minimum_required(VERSION 3.15)
project(Nexus_External CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── Source files ──────────────────────────────────────────────────────────────
# Explicitly list to avoid pulling in dllmain.cpp or hooks
file(GLOB_RECURSE SOURCES
    "src/main.cpp"
    "src/memory/*.cpp"
    "src/ui/*.cpp"
    "src/visuals/*.cpp"
    "src/targeting/*.cpp"
    "src/overlay/*.cpp"
    "src/utils/*.cpp"
)
file(GLOB_RECURSE HEADERS
    "src/*.h"
    "src/**/*.h"
)

# ── Build as standalone EXE (not a DLL) ──────────────────────────────────────
add_executable(${PROJECT_NAME} WIN32 ${SOURCES} ${HEADERS})

# ── nlohmann/json (header-only) ───────────────────────────────────────────────
# Expects include/nlohmann/json.hpp to exist (same as before)

# ── Include directories ───────────────────────────────────────────────────────
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/imgui
    ${CMAKE_SOURCE_DIR}/include/imgui/backends
)

# ── ImGui sources ─────────────────────────────────────────────────────────────
target_sources(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/include/imgui/imgui.cpp
    ${CMAKE_SOURCE_DIR}/include/imgui/imgui_demo.cpp
    ${CMAKE_SOURCE_DIR}/include/imgui/imgui_draw.cpp
    ${CMAKE_SOURCE_DIR}/include/imgui/imgui_tables.cpp
    ${CMAKE_SOURCE_DIR}/include/imgui/imgui_widgets.cpp
    ${CMAKE_SOURCE_DIR}/include/imgui/backends/imgui_impl_win32.cpp
    ${CMAKE_SOURCE_DIR}/include/imgui/backends/imgui_impl_dx11.cpp
)

# ── Link libraries ────────────────────────────────────────────────────────────
# No MinHook needed — we are not hooking anything
target_link_libraries(${PROJECT_NAME} PRIVATE
    d3d11
    dxgi
    dwmapi
    psapi       # For EnumProcessModules
)

# ── Post-build: copy resources + font dir ─────────────────────────────────────
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/resources
        $<TARGET_FILE_DIR:${PROJECT_NAME}>
    COMMAND ${CMAKE_COMMAND} -E make_directory
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/fonts
)
