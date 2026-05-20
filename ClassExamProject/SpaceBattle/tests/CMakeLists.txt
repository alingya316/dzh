find_package(GTest CONFIG REQUIRED)
find_package(Qt6 COMPONENTS Core QUIET)
if(Qt6_FOUND)
  set(QT_CORE Qt6::Core)
else()
  find_package(Qt5 REQUIRED COMPONENTS Core)
  set(QT_CORE Qt5::Core)
endif()

add_executable(spacebattle_tests
    test_spacemodel.cpp
    ../src/Model/SpaceModel.cpp
)

target_include_directories(spacebattle_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

target_link_libraries(spacebattle_tests
    exam_common
    GTest::gtest
    GTest::gtest_main
    ${QT_CORE}
)

if(MSVC)
    target_compile_options(spacebattle_tests PRIVATE /utf-8)
endif()

if(MSVC)
    target_compile_options(spacebattle_tests PRIVATE /utf-8)
endif()
