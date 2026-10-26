cmake_minimum_required(VERSION 3.15)
project(AlgoTrackQt)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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

add_executable(AlgoTrackQt WIN32 ${PROJECT_SOURCES})

target_link_libraries(AlgoTrackQt PRIVATE Qt6::Widgets LibAlgoTrack)

# --- Automatizare copiere Dependinte & Date pentru Windows/MSVC ---
add_custom_command(TARGET AlgoTrackQt POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "Sincronizare AlgoTrackCli si date pentru AlgoTrackQt..."
    # 1. Copiaza executabilul CLI backend langa AlgoTrackQt
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:AlgoTrackCli>"
        "$<TARGET_FILE_DIR:AlgoTrackQt>/$<TARGET_FILE_NAME:AlgoTrackCli>"
    # 2. Copiaza folderul de date cu problems.csv langa executabil
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/data"
        "$<TARGET_FILE_DIR:AlgoTrackQt>/data"
    COMMENT "Copiere automata CLI si data/problems.csv in folderul de iesire"
)

target_include_directories(AlgoTrackQt PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../src)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/data DESTINATION ${CMAKE_CURRENT_BINARY_DIR})