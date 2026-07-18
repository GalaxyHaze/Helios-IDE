cmake_minimum_required(VERSION 3.21)
project(Helios)
include(CTest)

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

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network Test)

if(APPLE)
    # Qt6 may hardcode linking to the deprecated AGL framework, which is missing in newer macOS SDKs.
    # We can create a dummy AGL framework to satisfy the linker.
    execute_process(COMMAND mkdir -p ${CMAKE_BINARY_DIR}/dummy_frameworks/AGL.framework)
    execute_process(COMMAND xcrun clang -dynamiclib -o ${CMAKE_BINARY_DIR}/dummy_frameworks/AGL.framework/AGL -x c /dev/null)
    link_directories(BEFORE ${CMAKE_BINARY_DIR}/dummy_frameworks)
endif()

# Define o executavel principal
add_executable(Helios
    main.cpp
    editor/core/TomlSettingsStore.h
    editor/core/TomlSettingsStore.cpp
    editor/core/ThemeManager.h
    editor/core/ThemeManager.cpp
    editor/core/AppearanceController.h
    editor/core/AppearanceController.cpp
    editor/core/TranslationManager.h
    editor/core/TranslationManager.cpp
    editor/panels/WelcomeWidget.h
    editor/panels/WelcomeWidget.cpp
    editor/widgets/ProjectTreeModel.h
    editor/widgets/ProjectTreeModel.cpp
    editor/panels/OutlinePanel.h
    editor/panels/OutlinePanel.cpp
    editor/core/MainWindow.h
    editor/core/MainWindow.cpp
    editor/editor/Code.h
    editor/editor/Code.cpp
    editor/editor/Syntax.h
    editor/editor/Syntax.cpp
    editor/editor/VimMotionController.h
    editor/editor/VimMotionController.cpp
    editor/editor/LspClient.h
    editor/editor/LspClient.cpp
    editor/core/ZithToolchainManager.h
    editor/core/ZithToolchainManager.cpp
    editor/editor/LspCompletionModel.h
    editor/editor/LspCompletionModel.cpp
    editor/panels/DiagnosticsPanel.h
    editor/panels/DiagnosticsPanel.cpp
    editor/panels/ReferencesPanel.h
    editor/panels/ReferencesPanel.cpp
    editor/widgets/ActivityBar.h
    editor/widgets/ActivityBar.cpp
    editor/widgets/BreadcrumbsBar.h
    editor/widgets/BreadcrumbsBar.cpp
    editor/panels/FileTreePanel.h
    editor/panels/FileTreePanel.cpp
    editor/core/ContextManager.h
    editor/core/ContextManager.cpp
    editor/panels/CompilerPanel.h
    editor/panels/CompilerPanel.cpp
    editor/core/SnippetManager.h
    editor/core/SnippetManager.cpp
    editor/widgets/FindReplaceBar.h
    editor/widgets/FindReplaceBar.cpp
    editor/panels/SearchPanel.h
    editor/panels/SearchPanel.cpp
    editor/panels/GitPanel.h
    editor/panels/GitPanel.cpp
    editor/panels/SettingsPanel.h
    editor/panels/SettingsPanel.cpp
    editor/panels/PreferencesDialog.h
    editor/panels/PreferencesDialog.cpp
    editor/panels/ShortcutsDialog.h
    editor/panels/ShortcutsDialog.cpp
    editor/panels/LspManagerDialog.h
    editor/panels/LspManagerDialog.cpp
    editor/panels/VimHelpDialog.h
    editor/panels/VimHelpDialog.cpp
    editor/assets/resources.qrc
)

target_link_libraries(Helios PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Network
    Qt6::Widgets
)

# Copiar arquivos de temas e traducoes para o diretorio de build
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/themes" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/i18n" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")

# Testes automatizados
add_executable(test_helios
    tests/test_helios.cpp
    editor/core/TomlSettingsStore.h
    editor/core/TomlSettingsStore.cpp
    editor/core/ThemeManager.h
    editor/core/ThemeManager.cpp
    editor/core/AppearanceController.h
    editor/core/AppearanceController.cpp
    editor/editor/Syntax.h
    editor/editor/Syntax.cpp
    editor/editor/VimMotionController.h
    editor/editor/VimMotionController.cpp
    editor/core/TranslationManager.h
    editor/core/TranslationManager.cpp
    editor/panels/SettingsPanel.h
    editor/panels/SettingsPanel.cpp
    editor/widgets/ProjectTreeModel.h
    editor/widgets/ProjectTreeModel.cpp
)
target_link_libraries(test_helios PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Gui
    Qt6::Test
)
add_test(NAME HeliosTests COMMAND test_helios)
set_tests_properties(HeliosTests PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

#Optmizations, clang, please use Clang to compile this
if(CMAKE_BUILD_TYPE MATCHES "Release|RelWithDebInfo|MinSizeRel")
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        message(STATUS "Helios: Aplicando otimizacoes agressivas para o compilador Clang.")

        target_compile_options(Helios PRIVATE -O3 -march=native)

        include(CheckIPOSupported)
        check_ipo_supported(RESULT ipo_supported OUTPUT ipo_error)

        if(ipo_supported)
            set_target_properties(Helios PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
            
            target_compile_options(Helios PRIVATE -flto=thin)
            target_link_options(Helios PRIVATE -flto=thin)
            message(STATUS "Helios: ThinLTO ativo com sucesso!")
        else()
            message(WARNING "Helios: LTO nao suportado neste ambiente: ${ipo_error}")
        endif()
    endif()
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(Helios PRIVATE HELIOS_THEME_TIMING)
endif()
