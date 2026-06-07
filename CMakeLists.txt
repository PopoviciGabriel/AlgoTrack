cmake_minimum_required(VERSION 3.15)
project(AlgoTrack)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_options(/Zc:__cplusplus /permissive-)
endif()

add_library(LibAlgoTrack STATIC
    src/Problem.cpp
    src/ProblemManager.cpp
    src/ProblemRepository.cpp
    src/Statistics.cpp
    src/Fuzzy.cpp
    src/Utils.cpp
    src/QueryParser.cpp
)

target_include_directories(LibAlgoTrack PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

add_executable(AlgoTrackCli
    src/main.cpp
)

target_link_libraries(AlgoTrackCli PRIVATE LibAlgoTrack)

add_subdirectory(qt_app)