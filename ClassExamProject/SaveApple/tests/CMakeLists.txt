find_package(GTest CONFIG REQUIRED)

add_executable(saveapple_nonui_tests
    test_fruit.cpp
    test_gamedata.cpp
    test_gameconfig.cpp
    ../src/Model/Fruit.cpp
    ../src/Model/GameData.cpp
    ../src/Config/GameConfig.cpp
)

target_include_directories(saveapple_nonui_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

target_link_libraries(saveapple_nonui_tests
    exam_common
    GTest::gtest
    GTest::gtest_main
    ${QT_LIBS}
)

add_executable(saveapple_cli_smoke
    cli_smoke.cpp
    ../src/Model/Fruit.cpp
    ../src/Model/GameData.cpp
    ../src/Config/GameConfig.cpp
)

target_include_directories(saveapple_cli_smoke PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

if (Qt6_FOUND)
    target_link_libraries(saveapple_cli_smoke exam_common Qt6::Core)
else()
    target_link_libraries(saveapple_cli_smoke exam_common Qt5::Core)
endif()

