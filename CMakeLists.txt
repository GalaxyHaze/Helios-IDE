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

set(HELIOS_SHARED_SOURCES
    editor/core/AppearanceController.h
    editor/core/AppearanceController.cpp
    editor/core/ApplicationThemeController.h
    editor/core/ApplicationThemeController.cpp
    editor/core/ApplicationStyle.h
    editor/core/ApplicationStyle.cpp
    editor/core/ContextManager.h
    editor/core/ContextManager.cpp
    editor/core/ContextWorkspaceController.h
    editor/core/ContextWorkspaceController.cpp
    editor/core/ContextNavigationController.h
    editor/core/ContextNavigationController.cpp
    editor/core/WorkspaceRootController.h
    editor/core/WorkspaceRootController.cpp
    editor/core/WorkspaceRootInteractionController.h
    editor/core/WorkspaceRootInteractionController.cpp
    editor/core/SidebarController.h
    editor/core/SidebarController.cpp
    editor/core/WindowLayoutPersistence.h
    editor/core/WindowLayoutPersistence.cpp
    editor/core/WindowLayoutController.h
    editor/core/WindowLayoutController.cpp
    editor/core/FileIcons.h
    editor/core/FileIcons.cpp
    editor/core/GitCommandRunner.h
    editor/core/GitCommandRunner.cpp
    editor/core/GitRepositorySession.h
    editor/core/GitRepositorySession.cpp
    editor/core/GitStatusParser.h
    editor/core/GitStatusParser.cpp
    editor/core/LanguageIdentity.h
    editor/core/LanguageIdentity.cpp
    editor/core/LanguageServiceFeedbackController.h
    editor/core/LanguageServiceFeedbackController.cpp
    editor/core/LspEventSource.h
    editor/core/LspClientEventSource.h
    editor/core/LspClientEventSource.cpp
    editor/core/LspSettingsPersistence.h
    editor/core/LspSettingsPersistence.cpp
    editor/core/LanguageServiceWorkspaceController.h
    editor/core/LanguageServiceWorkspaceController.cpp
    editor/core/LspDocumentCoordinator.h
    editor/core/LspDocumentCoordinator.cpp
    editor/core/LspEditorLifecycleController.h
    editor/core/LspEditorLifecycleController.cpp
    editor/core/LspEditorResultRouter.h
    editor/core/LspEditorResultRouter.cpp
    editor/core/LspReferencesRouter.h
    editor/core/LspReferencesRouter.cpp
    editor/core/LspCodeActionRouter.h
    editor/core/LspCodeActionRouter.cpp
    editor/core/LspRuntimePresentationController.h
    editor/core/LspRuntimePresentationController.cpp
    editor/core/LspRuntimePresentationState.h
    editor/core/LspRuntimePresentationModel.h
    editor/core/LspRuntimePresentationModel.cpp
    editor/core/LspLogPresenter.h
    editor/core/LspLogPresenter.cpp
    editor/core/LspRuntimeController.h
    editor/core/LspRuntimeController.cpp
    editor/core/LspRuntimeEventController.h
    editor/core/LspRuntimeEventController.cpp
    editor/core/LocationNavigator.h
    editor/core/LocationNavigator.cpp
    editor/core/EditorChromeController.h
    editor/core/EditorChromeController.cpp
    editor/core/EditorWorkspacePresentationController.h
    editor/core/EditorWorkspacePresentationController.cpp
    editor/core/StatusBarController.h
    editor/core/StatusBarController.cpp
    editor/core/ShellCommandSurface.h
    editor/core/ShellCommandSurface.cpp
    editor/core/ShellTranslationController.h
    editor/core/ShellTranslationController.cpp
    editor/core/ShellDialogController.h
    editor/core/ShellDialogController.cpp
    editor/core/ShellCommand.h
    editor/core/LspCompletionRouter.h
    editor/core/LspCompletionRouter.cpp
    editor/core/LspRestartPolicy.h
    editor/core/LspRestartPolicy.cpp
    editor/core/MainWindow.h
    editor/core/MainWindow.cpp
    editor/core/RunOutputCollector.h
    editor/core/RunOutputCollector.cpp
    editor/core/SnippetManager.h
    editor/core/SnippetManager.cpp
    editor/core/ShortcutCatalog.h
    editor/core/ShortcutCatalog.cpp
    editor/core/ShortcutTreePresenter.h
    editor/core/ShortcutTreePresenter.cpp
    editor/core/ThemeManager.h
    editor/core/ThemeManager.cpp
    editor/core/ThemeDefinition.h
    editor/core/ThemeDefinitionParser.h
    editor/core/ThemeDefinitionParser.cpp
    editor/core/TomlSettingsStore.h
    editor/core/TomlSettingsStore.cpp
    editor/core/TomlSettingsSnapshot.h
    editor/core/TomlSettingsCodec.h
    editor/core/TomlSettingsCodec.cpp
    editor/core/TranslationManager.h
    editor/core/TranslationManager.cpp
    editor/core/WorkspaceEdit.h
    editor/core/WorkspaceEdit.cpp
    editor/core/WorkspaceEditApplier.h
    editor/core/WorkspaceEditApplier.cpp
    editor/core/WorkspaceSearch.h
    editor/core/WorkspaceSearch.cpp
    editor/core/WorkspaceSearchController.h
    editor/core/WorkspaceSearchController.cpp
    editor/core/WorkspaceReplaceController.h
    editor/core/WorkspaceReplaceController.cpp
    editor/core/WorkspaceNavigationController.h
    editor/core/WorkspaceNavigationController.cpp
    editor/core/WorkspaceCommandController.h
    editor/core/WorkspaceCommandController.cpp
    editor/core/WorkspaceCommandResultDecoder.h
    editor/core/WorkspaceCommandResultDecoder.cpp
    editor/core/WorkspaceTaskOutputController.h
    editor/core/WorkspaceTaskOutputController.cpp
    editor/core/WorkspaceCommandState.h
    editor/core/WorkspaceCommandAvailability.h
    editor/core/WorkspaceCommandAvailabilityController.h
    editor/core/WorkspaceCommandAvailabilityController.cpp
    editor/core/WorkspacePanelPresentationController.h
    editor/core/WorkspacePanelPresentationController.cpp
    editor/core/ClangdLifecycleCoordinator.h
    editor/core/ClangdLifecycleCoordinator.cpp
    editor/core/ClangdExecutableResolver.h
    editor/core/ClangdExecutableResolver.cpp
    editor/core/EditorSyntaxController.h
    editor/core/EditorSyntaxController.cpp
    editor/core/EditorFileController.h
    editor/core/EditorFileController.cpp
    editor/core/EditorSessionController.h
    editor/core/EditorSessionController.cpp
    editor/core/EditorInteractionController.h
    editor/core/EditorInteractionController.cpp
    editor/core/EditorTabCloseController.h
    editor/core/EditorTabCloseController.cpp
    editor/core/EditorLspActionController.h
    editor/core/EditorLspActionController.cpp
    editor/core/ZithToolchainManager.h
    editor/core/ZithToolchainManager.cpp
    editor/core/ZithRuntimeAssetDownloader.h
    editor/core/ZithRuntimeAssetDownloader.cpp
    editor/core/ZithReleaseCatalog.h
    editor/core/ZithReleaseCatalog.cpp
    editor/core/ZithRuntimeInstaller.h
    editor/core/ZithRuntimeInstaller.cpp
    editor/core/ZithRuntimeOverrideResolver.h
    editor/core/ZithRuntimeOverrideResolver.cpp
    editor/core/ZithRuntimeCatalog.h
    editor/core/ZithRuntimeCatalog.cpp
    editor/core/ZithRuntimeState.h
    editor/core/ZithRuntimeState.cpp
    editor/core/ZithRuntimeLifecycleCoordinator.h
    editor/core/ZithRuntimeLifecycleCoordinator.cpp
    editor/editor/CHighlighter.h
    editor/editor/CHighlighter.cpp
    editor/editor/Code.h
    editor/editor/Code.cpp
    editor/editor/EditorAppearanceController.h
    editor/editor/EditorAppearanceController.cpp
    editor/editor/EditorDecorationController.h
    editor/editor/EditorDecorationController.cpp
    editor/editor/EditorContextMenuController.h
    editor/editor/EditorContextMenuController.cpp
    editor/editor/BracketMatcher.h
    editor/editor/BracketMatcher.cpp
    editor/editor/EditorDiagnosticHighlighter.h
    editor/editor/EditorDiagnosticHighlighter.cpp
    editor/editor/EditorTextEditApplier.h
    editor/editor/EditorTextEditApplier.cpp
    editor/editor/EditorCompletionInsertionPolicy.h
    editor/editor/EditorCompletionInsertionPolicy.cpp
    editor/editor/EditorTypingPolicy.h
    editor/editor/EditorTypingPolicy.cpp
    editor/editor/EditorDeletionPolicy.h
    editor/editor/EditorDeletionPolicy.cpp
    editor/editor/EditorLanguageFeatureController.h
    editor/editor/EditorLanguageFeatureController.cpp
    editor/editor/EditorLanguageFeedbackPresenter.h
    editor/editor/EditorLanguageFeedbackPresenter.cpp
    editor/editor/EditorLanguageRequestContext.h
    editor/editor/EditorDocumentSyncController.h
    editor/editor/EditorDocumentSyncController.cpp
    editor/editor/LspClient.h
    editor/editor/LspClient.cpp
    editor/editor/LspProcessTransport.h
    editor/editor/LspProcessTransport.cpp
    editor/editor/LspTypes.h
    editor/editor/LspTypes.cpp
    editor/editor/LspResultDecoder.h
    editor/editor/LspResultDecoder.cpp
    editor/editor/LspProtocolCodec.h
    editor/editor/LspProtocolCodec.cpp
    editor/editor/LspRequestTracker.h
    editor/editor/LspRequestTracker.cpp
    editor/editor/LspRequestSender.h
    editor/editor/LspRequestSender.cpp
    editor/editor/LspServerMessageDispatcher.h
    editor/editor/LspServerMessageDispatcher.cpp
    editor/editor/LspInitializationBuilder.h
    editor/editor/LspInitializationBuilder.cpp
    editor/editor/LspDocumentSync.h
    editor/editor/LspDocumentSync.cpp
    editor/editor/LspDocumentProtocol.h
    editor/editor/LspDocumentProtocol.cpp
    editor/editor/LspFeatureRequestRouter.h
    editor/editor/LspFeatureRequestRouter.cpp
    editor/editor/LspShutdownEscalation.h
    editor/editor/LspShutdownEscalation.cpp
    editor/editor/LspSessionLifecycle.h
    editor/editor/LspSessionLifecycle.cpp
    editor/editor/LspDocumentRegistry.h
    editor/editor/LspDocumentRegistry.cpp
    editor/editor/LspCompletionModel.h
    editor/editor/LspCompletionModel.cpp
    editor/editor/Syntax.h
    editor/editor/Syntax.cpp
    editor/editor/VimMotionController.h
    editor/editor/VimMotionController.cpp
    editor/editor/VimCommandSession.h
    editor/editor/VimCommandSession.cpp
    editor/editor/VimMotionResolver.h
    editor/editor/VimMotionResolver.cpp
    editor/editor/VimCharacterSearch.h
    editor/editor/VimCharacterSearch.cpp
    editor/editor/VimPendingOperationSession.h
    editor/editor/VimPendingOperationSession.cpp
    editor/editor/VimSearchSession.h
    editor/editor/VimSearchSession.cpp
    editor/editor/VimOperation.h
    editor/editor/VimDocumentOperations.h
    editor/editor/VimDocumentOperations.cpp
    editor/panels/BottomPanel.h
    editor/panels/BottomPanel.cpp
    editor/panels/CompilerPanel.h
    editor/panels/CompilerPanel.cpp
    editor/panels/DiagnosticsPanel.h
    editor/panels/DiagnosticsPanel.cpp
    editor/panels/FileTreePanel.h
    editor/panels/FileTreePanel.cpp
    editor/panels/GitPanel.h
    editor/panels/GitPanel.cpp
    editor/panels/GitStatusListPresenter.h
    editor/panels/GitStatusListPresenter.cpp
    editor/panels/GitPanelPresentationModel.h
    editor/panels/GitPanelPresentationModel.cpp
    editor/panels/LspManagerDialog.h
    editor/panels/LspManagerDialog.cpp
    editor/panels/OutlinePanel.h
    editor/panels/OutlinePanel.cpp
    editor/panels/PreferencesDialog.h
    editor/panels/PreferencesDialog.cpp
    editor/panels/ReferencesPanel.h
    editor/panels/ReferencesPanel.cpp
    editor/panels/SearchPanel.h
    editor/panels/SearchPanel.cpp
    editor/panels/SettingsPanel.h
    editor/panels/SettingsPanel.cpp
    editor/panels/ShortcutsDialog.h
    editor/panels/ShortcutsDialog.cpp
    editor/panels/GettingStartedDialog.h
    editor/panels/GettingStartedDialog.cpp
    editor/panels/VimHelpDialog.h
    editor/panels/VimHelpDialog.cpp
    editor/panels/WelcomeWidget.h
    editor/panels/WelcomeWidget.cpp
    editor/widgets/ActivityBar.h
    editor/widgets/ActivityBar.cpp
    editor/widgets/BreadcrumbsBar.h
    editor/widgets/BreadcrumbsBar.cpp
    editor/widgets/FindReplaceBar.h
    editor/widgets/FindReplaceBar.cpp
    editor/widgets/ProjectTreeModel.h
    editor/widgets/ProjectTreeModel.cpp
    editor/assets/resources.qrc
)

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
    ${HELIOS_SHARED_SOURCES}
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
    ${HELIOS_SHARED_SOURCES}
)
target_link_libraries(test_helios PRIVATE
    Qt6::Core
    Qt6::Network
    Qt6::Widgets
    Qt6::Gui
    Qt6::Test
)
target_compile_definitions(test_helios PRIVATE HELIOS_UNIT_TESTING)
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
