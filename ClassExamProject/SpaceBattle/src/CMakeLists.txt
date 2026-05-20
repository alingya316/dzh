cmake_minimum_required(VERSION 3.16)
project(spacebattle_src LANGUAGES CXX)

target_sources(spacebattle PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/Model/SpaceModel.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/System/SpaceAudio.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/System/WordProvider.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/View/SpaceView.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/View/EnemyCaptionLetterFactory.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Controller/SpaceController.cpp
)

