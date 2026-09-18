cmake_minimum_required(VERSION 3.20)
project(evafile VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Qt6 Configuration
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS
    Core
    Gui
    Widgets
    Svg
    SvgWidgets
    Multimedia
    Network
)

find_package(PkgConfig REQUIRED)
pkg_check_modules(POPPLER_QT6 poppler-qt6)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${POPPLER_QT6_INCLUDE_DIRS}
)

# Source files
set(SOURCES
    src/main.cpp
    src/MainWindow.cpp
    src/ThemeEngine.cpp
    src/Config.cpp
    src/BreadcrumbBar.cpp
    src/TabBar.cpp
    src/Sidebar.cpp
    src/FileView.cpp
    src/SplitManager.cpp
    src/PreviewDrawer.cpp
    src/FileOperations.cpp
    src/EvaSuiteBridge.cpp
    src/EvalinkManager.cpp
    src/EvalinkDrawer.cpp
    src/EvalinkDialog.cpp
    src/DragDropHelper.cpp
    resources/evafile.qrc
)

# Headers (for moc)
set(HEADERS
    include/MainWindow.hpp
    include/ThemeEngine.hpp
    include/Config.hpp
    include/BreadcrumbBar.hpp
    include/TabBar.hpp
    include/Sidebar.hpp
    include/FileView.hpp
    include/SplitManager.hpp
    include/PreviewDrawer.hpp
    include/FileOperations.hpp
    include/EvaSuiteBridge.hpp
    include/EvalinkManager.hpp
    include/EvalinkDrawer.hpp
    include/EvalinkDialog.hpp
    include/DragDropHelper.hpp
)

add_executable(evafile ${SOURCES} ${HEADERS})

target_link_libraries(evafile PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Svg
    Qt6::SvgWidgets
    Qt6::Multimedia
    Qt6::Network
    ${POPPLER_QT6_LIBRARIES}
)

if(POPPLER_QT6_FOUND)
    target_compile_definitions(evafile PRIVATE HAVE_POPPLER_QT6=1)
endif()

# Installation
install(TARGETS evafile RUNTIME DESTINATION bin)
install(FILES resources/evafile.desktop DESTINATION share/applications)
install(FILES resources/icons/evafile.svg DESTINATION share/icons/hicolor/scalable/apps)

# Testing
enable_testing()
add_subdirectory(tests)
