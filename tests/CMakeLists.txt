set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Test Widgets Svg)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

set(TEST_SOURCES
    test_main.cpp
    ../src/FileOperations.cpp
    ../src/Config.cpp
    ../src/ThemeEngine.cpp
    ../src/EvaSuiteBridge.cpp
    ../resources/evafile.qrc
)

set(TEST_HEADERS
    ../include/FileOperations.hpp
    ../include/Config.hpp
    ../include/ThemeEngine.hpp
    ../include/EvaSuiteBridge.hpp
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
)

add_test(NAME EvaFileTests COMMAND test_evafile)
