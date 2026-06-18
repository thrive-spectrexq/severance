cmake_minimum_required(VERSION 3.28)
project(severance VERSION 0.1.0 LANGUAGES CXX)

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
    src/gui/process_view/ProcessView.cpp
    src/gui/process_view/ProcessTreeModel.cpp
    src/gui/theme/Theme.cpp
    src/gui/search/SearchOverlay.cpp
    src/gui/network_view/NetworkView.cpp
    src/gui/file_view/FileView.cpp
    src/utils/ThreadPool.cpp
    src/utils/UUID.cpp
    src/utils/Config.cpp
    src/core/process/ProcessManager.cpp
    src/core/metrics/WindowsMetricsProvider.cpp
    src/core/network/NetworkManager.cpp
    src/core/filesystem/FileMonitor.cpp
    src/core/filesystem/EtwMonitor.cpp
    src/core/plugins/PluginManager.cpp
    src/core/plugins/PluginLoader.cpp
    src/core/sandbox/SandboxManager.cpp
    src/core/store/EventStore.cpp
)

target_include_directories(severance PRIVATE src src/core src/gui src/utils)
target_link_libraries(severance PRIVATE
    spdlog::spdlog
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Sql
)

enable_testing()
add_executable(severance_tests
    tests/unit/dummy_test.cpp
    src/core/logging/Logger.cpp
    src/core/events/EventBus.cpp
    src/utils/ThreadPool.cpp
    src/utils/UUID.cpp
    src/utils/Config.cpp
    src/core/process/ProcessManager.cpp
    src/core/network/NetworkManager.cpp
    src/core/filesystem/FileMonitor.cpp
    src/core/plugins/PluginManager.cpp
    src/core/plugins/PluginLoader.cpp
    src/core/sandbox/SandboxManager.cpp
)
target_include_directories(severance_tests PRIVATE src src/core src/gui src/utils)
target_link_libraries(severance_tests PRIVATE
    spdlog::spdlog
    Catch2::Catch2WithMain
)
add_test(NAME dummy_test COMMAND severance_tests)
