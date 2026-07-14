cmake_minimum_required(VERSION 3.21)
project(Helios)

# Ninja is a single-configuration generator. Default to an optimized build while
# preserving an explicit choice such as -DCMAKE_BUILD_TYPE=Debug.
if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS Debug Release RelWithDebInfo MinSizeRel)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network)

if(APPLE)
    # Qt6 may hardcode linking to the deprecated AGL framework, which is missing in newer macOS SDKs.
    # We can create a dummy AGL framework to satisfy the linker.
    execute_process(COMMAND mkdir -p ${CMAKE_BINARY_DIR}/dummy_frameworks/AGL.framework)
    execute_process(COMMAND xcrun clang -dynamiclib -o ${CMAKE_BINARY_DIR}/dummy_frameworks/AGL.framework/AGL -x c /dev/null)
    link_directories(BEFORE ${CMAKE_BINARY_DIR}/dummy_frameworks)
endif()

add_executable(Helios
    main.cpp
    editor/MainWindow.cpp
    editor/Code.cpp
    editor/Syntax.cpp
    editor/LspClient.cpp
    editor/ZithToolchainManager.cpp
    editor/LspCompletionModel.cpp
    editor/DiagnosticsPanel.cpp
    editor/ActivityBar.cpp
    editor/BreadcrumbsBar.cpp
    editor/FileTreePanel.cpp
    editor/ContextManager.cpp
    editor/CompilerPanel.cpp
    editor/SnippetManager.cpp
    editor/FindReplaceBar.cpp
    editor/SearchPanel.cpp
    editor/GitPanel.cpp
    editor/SettingsPanel.cpp
    editor/resources.qrc
)

target_link_libraries(Helios PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Network
    Qt6::Widgets
)
