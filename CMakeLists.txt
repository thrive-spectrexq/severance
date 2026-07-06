cmake_minimum_required(VERSION 3.28)
project(severance VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

include(cmake/Dependencies.cmake)

add_executable(severance
    src/main.cpp
    src/core/logging/Logger.cpp
    src/core/events/EventBus.cpp
    src/core/application/Application.cpp
    src/gui/windows/MainWindow.cpp
    src/gui/dashboard/DashboardView.cpp
    src/gui/perimeter_grid/PerimeterGridView.cpp
    src/gui/process_view/ProcessView.cpp
    src/gui/process_view/ProcessTreeModel.cpp
    src/gui/process_view/ProcessDetailPanel.cpp
    src/gui/theme/Theme.cpp
    src/gui/terminal/TerminalOverlay.cpp
    src/gui/search/SearchOverlay.cpp
    src/gui/network_view/NetworkView.cpp
    src/gui/network_view/NetworkDetailPanel.cpp
    src/gui/security_view/SecurityView.cpp
    src/gui/widgets/ToastNotification.cpp
    src/gui/file_view/FileView.cpp
    src/gui/file_view/FileDetailPanel.cpp
    src/gui/timeline/TimelineView.cpp
    src/gui/timeline/TimelineDetailPanel.cpp
    src/gui/timeline/TimelineHistogram.cpp
    src/gui/isolation_view/IsolationView.cpp
    src/gui/session_view/SessionView.cpp
    src/gui/ai_panel/AiPanel.cpp
    src/gui/board_comms/BoardCommsView.cpp
    src/gui/optics_and_design/OpticsDesignView.cpp
    src/gui/graphs/ResourceGraph.cpp
    src/gui/graphs/OpenGLResourceGraph.cpp
    src/gui/widgets/StatusIndicator.cpp
    src/gui/widgets/DonutChartWidget.cpp
    src/gui/widgets/HorizontalBarChartWidget.cpp
    src/gui/widgets/NumberGridWidget.cpp
    src/gui/widgets/NeuralTopologyWidget.cpp
    src/gui/windows/SettingsWindow.cpp
    src/utils/ThreadPool.cpp
    src/core/concurrency/ThreadPool.cpp
    src/utils/UUID.cpp
    src/utils/Config.cpp
    src/core/process/ProcessManager.cpp
    src/core/metrics/WindowsMetricsProvider.cpp
    src/core/metrics/BaselineManager.cpp
    src/core/network/NetworkManager.cpp
    src/core/filesystem/FileMonitor.cpp
    src/core/filesystem/EtwMonitor.cpp
    src/core/correlation/CorrelationEngine.cpp
    src/core/plugins/PluginManager.cpp
    src/core/plugins/PluginLoader.cpp
    src/core/sandbox/SandboxManager.cpp
    src/core/store/EventStore.cpp
    src/core/ai/AiEngine.cpp
    src/core/workspace/WorkspaceManager.cpp
    src/core/notifications/NotificationManager.cpp
    src/core/notifications/RuleEngine.cpp
    src/core/session/SessionManager.cpp
    src/core/security/FimManager.cpp
    src/core/security/ActiveResponse.cpp
    src/core/security/EventLogManager.cpp
    src/core/security/RootkitScanner.cpp
    src/platform/windows/sev_platform_win32.c
)

target_include_directories(severance PRIVATE src src/core src/gui src/utils ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(severance PRIVATE
    spdlog::spdlog
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Sql
    Qt6::Network
    Qt6::OpenGL
    Qt6::OpenGLWidgets
    psapi
)

enable_testing()

# --- Shared source files used by tests ---
set(TEST_COMMON_SOURCES
    src/core/logging/Logger.cpp
    src/core/events/EventBus.cpp
    src/utils/ThreadPool.cpp
    src/core/concurrency/ThreadPool.cpp
    src/utils/UUID.cpp
    src/utils/Config.cpp
    src/core/process/ProcessManager.cpp
    src/core/process/ProcessTree.cpp
    src/core/network/NetworkManager.cpp
    src/core/filesystem/FileMonitor.cpp
    src/core/plugins/PluginManager.cpp
    src/core/plugins/PluginLoader.cpp
    src/core/sandbox/SandboxManager.cpp
    src/core/notifications/NotificationManager.cpp
)

# --- Test executable ---
add_executable(severance_tests
    tests/unit/test_setup.cpp
    tests/unit/test_core_utils.cpp
    tests/unit/test_process_network.cpp
    tests/unit/test_system_components.cpp
    ${TEST_COMMON_SOURCES}
)
target_include_directories(severance_tests PRIVATE src src/core src/gui src/utils ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(severance_tests PRIVATE
    spdlog::spdlog
    Catch2::Catch2WithMain
    Qt6::Core
    Qt6::Network
)

# Register individual test suites for clearer CI output
add_test(NAME CoreUtils COMMAND severance_tests "[EventBus],[Logger],[ThreadPool],[UUID],[Config]")
add_test(NAME ProcessNetwork COMMAND severance_tests "[ProcessManager],[ProcessTree],[NetworkManager]")
add_test(NAME SystemComponents COMMAND severance_tests "[FileMonitor],[PluginManager],[SandboxManager]")

# Plugins
add_subdirectory(plugins)
