cmake_minimum_required(VERSION 4.1)
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
