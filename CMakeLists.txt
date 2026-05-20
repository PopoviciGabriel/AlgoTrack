cmake_minimum_required(VERSION 3.15)
project(AlgoTrack)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# FIX CRITIC PENTRU WINDOWS (MSVC) + QT6: Forțează raportarea corectă a standardului C++17
if(MSVC)
    add_compile_options(/Zc:__cplusplus)
endif()

# 1. Definim librăria de bază (Business Logic)
# Adăugăm toate fișierele din src/ care reprezintă logica aplicației, FĂRĂ main.cpp
add_library(LibAlgoTrack STATIC
    src/Problem.cpp
    src/ProblemManager.cpp
    src/ProblemRepository.cpp
    src/Statistics.cpp
    src/Fuzzy.cpp
    src/Utils.cpp
)

# Permitem includerea ușoară a header-elor din interiorul proiectului
target_include_directories(LibAlgoTrack PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

# 2. Definim executabilul de tip CLI (Linie de comandă automatizată)
add_executable(AlgoTrackCli
    src/main.cpp
)

# Legăm executabilul CLI de librăria noastră de logică
target_link_libraries(AlgoTrackCli PRIVATE LibAlgoTrack)

add_subdirectory(qt_app)