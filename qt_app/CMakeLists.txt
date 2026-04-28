cmake_minimum_required(VERSION 3.16)

project(AlgoTrackQt VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

qt_standard_project_setup()

set(CORE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../src")

qt_add_executable(AlgoTrackQt
    src/main.cpp
    src/MainWindow.h
    src/MainWindow.cpp
    src/AddProblemDialog.h
    src/AddProblemDialog.cpp
    ${CORE_DIR}/Problem.h
    ${CORE_DIR}/Problem.cpp
    ${CORE_DIR}/ProblemManager.h
    ${CORE_DIR}/ProblemManager.cpp
    ${CORE_DIR}/ProblemRepository.h
    ${CORE_DIR}/ProblemRepository.cpp
    ${CORE_DIR}/Statistics.h
    ${CORE_DIR}/Statistics.cpp
    ${CORE_DIR}/Fuzzy.h
    ${CORE_DIR}/Fuzzy.cpp
    ${CORE_DIR}/Utils.h
    ${CORE_DIR}/Utils.cpp
    ${CORE_DIR}/Enums.h
)

target_include_directories(AlgoTrackQt PRIVATE
    src
    ${CORE_DIR}
)

target_link_libraries(AlgoTrackQt PRIVATE Qt6::Widgets)

set_target_properties(AlgoTrackQt PROPERTIES
    WIN32_EXECUTABLE ON
)
