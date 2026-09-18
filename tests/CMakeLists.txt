set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Test Widgets Svg Network)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

set(TEST_SOURCES
    test_main.cpp
    ../src/FileOperations.cpp
    ../src/Config.cpp
    ../src/ThemeEngine.cpp
    ../src/EvaSuiteBridge.cpp
    ../src/EvalinkManager.cpp
    ../src/DragDropHelper.cpp
    ../resources/evafile.qrc
)

set(TEST_HEADERS
    ../include/FileOperations.hpp
    ../include/Config.hpp
    ../include/ThemeEngine.hpp
    ../include/EvaSuiteBridge.hpp
    ../include/EvalinkManager.hpp
    ../include/DragDropHelper.hpp
)

add_executable(test_evafile
    ${TEST_SOURCES}
    ${TEST_HEADERS}
)

target_link_libraries(test_evafile PRIVATE
    Qt6::Core
    Qt6::Test
    Qt6::Widgets
    Qt6::Svg
    Qt6::Network
)

add_test(NAME EvaFileTests COMMAND test_evafile)
