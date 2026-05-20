cmake_minimum_required(VERSION 3.16)
project(spacebattle_include LANGUAGES CXX)

target_sources(spacebattle PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/Model/SpaceModel.h
  ${CMAKE_CURRENT_SOURCE_DIR}/Controller/SpaceController.h
  ${CMAKE_CURRENT_SOURCE_DIR}/View/SpaceView.h
  ${CMAKE_CURRENT_SOURCE_DIR}/View/SpriteCrop.h
  ${CMAKE_CURRENT_SOURCE_DIR}/View/EnemyCaptionLetterFactory.h
  ${CMAKE_CURRENT_SOURCE_DIR}/System/SpaceAudio.h
  ${CMAKE_CURRENT_SOURCE_DIR}/System/WordProvider.h
)

