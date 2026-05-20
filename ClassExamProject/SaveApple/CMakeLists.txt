cmake_minimum_required(VERSION 3.16)
project(saveapple LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC OFF)
set(CMAKE_AUTORCC ON)

find_package(Qt6 COMPONENTS Widgets Multimedia QUIET)
if(Qt6_FOUND)
    set(QT_LIBS Qt6::Widgets Qt6::Multimedia)
else()
    find_package(Qt5 REQUIRED COMPONENTS Widgets Multimedia)
    set(QT_LIBS Qt5::Widgets Qt5::Multimedia)
endif()

add_library(saveapple STATIC)

target_include_directories(saveapple PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_sources(saveapple PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/resources/saveapple_resources.qrc
)

add_subdirectory(src)
add_subdirectory(include)
add_subdirectory(resources)
add_subdirectory(tests)

target_link_libraries(saveapple PRIVATE exam_common ${QT_LIBS})

if(MSVC)
    target_compile_options(saveapple PRIVATE /utf-8)
endif()
