# AlgoTrack

AlgoTrack is a C++ application for tracking solved programming problems, analyzing progress, and storing problem history in CSV files.

The project includes two versions:

- Console version

- Qt desktop version

The console version is lightweight and runs in the terminal.

The Qt version provides a desktop graphical interface.

## Download

Go to the GitHub Releases page and download one of the Windows builds:

- AlgoTrack Qt Desktop App: recommended for most users

- AlgoTrack Console App: terminal version

After downloading a release ZIP, extract it and run the executable inside.

## Features

- Add and manage solved problems

- CSV persistence

- Import and export CSV files

- Fuzzy search using Levenshtein distance

- Statistics dashboard

- Status distribution

- Difficulty breakdown

- Time and rating tracking

- Most used tags

- Tag and platform filtering

- Update status and notes

- Sorting by difficulty, time spent, and rating

- Clean separation between core logic, persistence, and UI

## Project Structure

AlgoTrack/

src/

Problem.*             core entity

ProblemManager.*      business logic

ProblemRepository.*   CSV file handling

Statistics.*          analytics engine

Fuzzy.*               fuzzy search

Utils.*               helper functions

ConsoleUI.*           console interface

main.cpp              console entry point

qt_app/

src/

main.cpp

MainWindow.*

AddProblemDialog.*

CMakeLists.txt

README.md

data/

problems.example.csv

docs/

HowToRun.txt

CMakeLists.txt

README.md

## Requirements

For building from source:

- CMake 3.16 or newer

- C++17 compiler

- Qt 6 for the desktop GUI version

- Ninja recommended on Windows

## Build Console Version

From the repository root:

cmake -S. -B build

cmake --build build

Run on Windows:

.\build\AlgoTrack.exe

## Build Qt Desktop Version

Install Qt 6 first.

On Windows with MSYS2 UCRT64:

pacman -S mingw-w64-ucrt-x86_64-qt6-base mingw-w64-ucrt-x86_64-qt6-tools mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-ninja

Then build:

$env:Path = "C:\msys64\ucrt64\bin;$env:Path"

cd qt_app

cmake -S. -B build -G Ninja

cmake --build build

Run:

.\build\AlgoTrackQt.exe

## CSV Format

Each problem is stored as:

name,platform,difficulty,tags,status,timeSpent,date,rating,notes

Example:

two sum,leetcode,easy,arrays|hashmap,solved,10,25-03-2026,5.0,very easy

dijkstra graph,atcoder,hard,graphs|dijkstra,in_progress,60,26-03-2026,9.0,need to revisit

Notes:

- Tags are separated by |

- Date format is DD-MM-YYYY

- Rating is between 1.0 and 10.0

## Qt Desktop App

The Qt version includes:

- table-based problem view

- add problem dialog

- automatic CSV saving

- fuzzy search

- statistics panel

- sorting controls

- CSV import and export

The Qt app saves data automatically to:

data/problems.csv

inside the same folder as the executable.

## Release Files

GitHub Releases should include two downloadable files:

AlgoTrack-qt-v1.0-windows.zip

AlgoTrack-terminal-v1.0-windows.zip

The Qt release ZIP should include the executable and required Qt runtime files.

The source repository should not include:

- .exe files

- .dll files

- build folders

- release ZIP/RAR files

## Author

Created by Gabriel Popovici as a C++ project focused on clean architecture, persistence, and desktop application development.