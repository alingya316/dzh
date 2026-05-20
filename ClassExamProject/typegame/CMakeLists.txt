cmake_minimum_required(VERSION 3.16)
project(gamelauncher LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 COMPONENTS Widgets Core QUIET)
if(Qt6_FOUND)
  set(QT_LIBS Qt6::Widgets Qt6::Core)
else()
  find_package(Qt5 REQUIRED COMPONENTS Widgets Core)
  set(QT_LIBS Qt5::Widgets Qt5::Core)
endif()

set(SA ${CMAKE_CURRENT_SOURCE_DIR}/../SaveApple)
set(SB ${CMAKE_CURRENT_SOURCE_DIR}/../SpaceBattle)

add_executable(typegame
    main.cpp
    GameLauncher.cpp
    SpaceTestDriver.cpp
    AppleTestDriver.cpp
    gui_mode.cpp
    cli_mode.cpp
    resources/resources.qrc
    ${SB}/resources/spacebattle_resources.qrc
    ${SA}/resources/saveapple_resources.qrc
)

target_include_directories(typegame PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${SA}/src
    ${SA}/include
    ${SB}/include
    ${SB}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/../ExamCommon/include
)

target_link_libraries(typegame PRIVATE
    saveapple
    spacebattle
    ${QT_LIBS}
)

if(MSVC)
  target_compile_options(typegame PRIVATE /utf-8)
endif()
