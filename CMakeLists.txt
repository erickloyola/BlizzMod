cmake_minimum_required(VERSION 3.20)
project(BlizzMod CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCES
    framework/dllmain.cpp
    framework/helpers.cpp
    framework/il2cpp-init.cpp
    framework/pch-il2cpp.cpp
    framework/version.cpp
    user/main.cpp
    libraries/detours/detours.cpp
    libraries/detours/disasm.cpp
    libraries/detours/modules.cpp
    libraries/imgui/imgui.cpp
    libraries/imgui/imgui_draw.cpp
    libraries/imgui/imgui_impl_dx11.cpp
    libraries/imgui/imgui_impl_win32.cpp
    libraries/imgui/imgui_tables.cpp
    libraries/imgui/imgui_widgets.cpp
    libraries/pipeline/Dx11.cpp
    libraries/pipeline/hooks/HookUtils.cpp
    libraries/pipeline/hooks/PlayerAttributesHooks.cpp
    libraries/pipeline/keybinds.cpp
    libraries/pipeline/settings.cpp
    libraries/pipeline/gui/GUITheme.cpp
    libraries/pipeline/gui/menu.cpp
    libraries/pipeline/gui/tabs/SettingsTAB.cpp
    libraries/pipeline/hooks/DirectX.cpp
    libraries/pipeline/hooks/InitHooks.cpp
    libraries/pipeline/telemetry/TelemetryServer.cpp
    version.def
)

add_library(version SHARED ${SOURCES})

target_include_directories(version PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/appdata
    ${CMAKE_CURRENT_SOURCE_DIR}/framework
    ${CMAKE_CURRENT_SOURCE_DIR}/user
    ${CMAKE_CURRENT_SOURCE_DIR}/libraries
)

target_compile_definitions(version PRIVATE
    _VERSION
    IL2CPPDLL_EXPORTS
    _WINDOWS
    _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
    _USRDLL
    UNICODE
    _UNICODE
)

target_compile_options(version PRIVATE
    /utf-8
    /bigobj
)

target_link_libraries(version PRIVATE
    d3d11
    d3dcompiler
    dxgi
    dxguid
    kernel32
    user32
    gdi32
    ws2_32
)

set_target_properties(version PROPERTIES
    OUTPUT_NAME "version"
    PREFIX ""
)
