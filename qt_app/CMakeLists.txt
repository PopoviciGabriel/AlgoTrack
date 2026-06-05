cmake_minimum_required(VERSION 3.15)
project(AlgoTrackQt)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# FIX CRITIC PENTRU WINDOWS (MSVC) + QT6 + UTF-8
if(MSVC)
    add_compile_options(/Zc:__cplusplus /permissive- /utf-8)
endif()

find_package(Qt6 REQUIRED COMPONENTS Widgets)

set(PROJECT_SOURCES
    src/main.cpp
    src/MainWindow.cpp
    src/AddProblemDialog.cpp
    src/AddProblemDialog.ui
    resources.qrc
)

# ADAUGAM WIN32 pentru a ascunde consola neagră din spate pe Windows
add_executable(AlgoTrackQt WIN32 ${PROJECT_SOURCES})

target_link_libraries(AlgoTrackQt PRIVATE Qt6::Widgets LibAlgoTrack)
target_include_directories(AlgoTrackQt PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../src)

# Copiem folderul de date. Fișierul style.qss este acum compilat direct prin resources.qrc
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/data DESTINATION ${CMAKE_CURRENT_BINARY_DIR})