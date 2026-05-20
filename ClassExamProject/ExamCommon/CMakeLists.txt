cmake_minimum_required(VERSION 3.16)
project(exam_common LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 COMPONENTS Widgets Multimedia Core QUIET)
if(Qt6_FOUND)
    set(QT_LIBS Qt6::Widgets Qt6::Multimedia Qt6::Core)
else()
    find_package(Qt5 REQUIRED COMPONENTS Widgets Multimedia Core)
    set(QT_LIBS Qt5::Widgets Qt5::Multimedia Qt5::Core)
endif()

add_library(exam_common STATIC
    src/AudioBase.cpp
)

target_include_directories(exam_common PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_link_libraries(exam_common PUBLIC ${QT_LIBS})

if(MSVC)
    target_compile_options(exam_common PUBLIC /utf-8)
endif()
