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
set(default_build_type "Release")
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

# Define o executável principal
add_executable(Helios
    main.cpp
    editor/MainWindow.h
    editor/MainWindow.cpp
    editor/Code.h
    editor/Code.cpp
    editor/Syntax.h
    editor/Syntax.cpp
    editor/LspClient.h
    editor/LspClient.cpp
    editor/ZithToolchainManager.h
    editor/ZithToolchainManager.cpp
    editor/LspCompletionModel.h
    editor/LspCompletionModel.cpp
    editor/DiagnosticsPanel.h
    editor/DiagnosticsPanel.cpp
    editor/ActivityBar.h
    editor/ActivityBar.cpp
    editor/BreadcrumbsBar.h
    editor/BreadcrumbsBar.cpp
    editor/FileTreePanel.h
    editor/FileTreePanel.cpp
    editor/ContextManager.h
    editor/ContextManager.cpp
    editor/CompilerPanel.h
    editor/CompilerPanel.cpp
    editor/SnippetManager.h
    editor/SnippetManager.cpp
    editor/FindReplaceBar.h
    editor/FindReplaceBar.cpp
    editor/SearchPanel.h
    editor/SearchPanel.cpp
    editor/GitPanel.h
    editor/GitPanel.cpp
    editor/SettingsPanel.h
    editor/SettingsPanel.cpp
    editor/resources.qrc
)

target_link_libraries(Helios PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Network
    Qt6::Widgets
)

#Optmizations, clang, please use Clang to compile this
if(CMAKE_BUILD_TYPE MATCHES "Release|RelWithDebInfo|MinSizeRel")
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        message(STATUS "Helios: Aplicando otimizações agressivas para o compilador Clang.")

        target_compile_options(Helios PRIVATE -O3 -march=native)

        include(CheckIPOSupported)
        check_ipo_supported(RESULT ipo_supported OUTPUT ipo_error)

        if(ipo_supported)
            set_target_properties(Helios PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
            
            target_compile_options(Helios PRIVATE -flto=thin)
            target_link_options(Helios PRIVATE -flto=thin)
            message(STATUS "Helios: ThinLTO ativo com sucesso!")
        else()
            message(WARNING "Helios: LTO não suportado neste ambiente: ${ipo_error}")
        endif()
    endif()
endif()