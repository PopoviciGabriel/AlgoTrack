cmake_minimum_required(VERSION 3.15)
project(AlgoTrackQt)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

# Adăugăm fișierul .ui direct în surse
set(PROJECT_SOURCES
    src/main.cpp
    src/MainWindow.cpp
    src/AddProblemDialog.cpp
    src/AddProblemDialog.ui
)

add_executable(AlgoTrackQt ${PROJECT_SOURCES})

target_link_libraries(AlgoTrackQt PRIVATE Qt6::Widgets LibAlgoTrack)
target_include_directories(AlgoTrackQt PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../src)

# Comenzi critice pentru resurse
configure_file(style.qss ${CMAKE_CURRENT_BINARY_DIR}/style.qss COPYONLY)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/data DESTINATION ${CMAKE_CURRENT_BINARY_DIR})