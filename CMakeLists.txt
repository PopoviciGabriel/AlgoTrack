cmake_minimum_required(VERSION 3.16)

project(AlgoTrack)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(AlgoTrack
    src/main.cpp
    src/Problem.cpp
    src/ProblemManager.cpp
    src/ProblemRepository.cpp
    src/Statistics.cpp
    src/Fuzzy.cpp
    src/Utils.cpp
    src/ConsoleUI.cpp
)