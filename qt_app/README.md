# AlgoTrack Qt

AlgoTrack Qt is a desktop version of AlgoTrack. It is built with C++17 and Qt 6.

It uses the core logic as the console version for:

* managing problems

* saving data to CSV files

* searching with fuzzy logic

* showing statistics

* sorting

## Requirements

To run AlgoTrack Qt you need:

* Qt 6

* CMake 3.16 or higher

* A C++17 compiler

* Ninja is recommended

If you're on Windows, with MSYS2 UCRT64 run:

pacman -S mingw-w64-ucrt-x86_64-qt6-base mingw-w64-ucrt-x86_64-qt6-tools mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-ninja

## Build

To build AlgoTrack Qt follow these steps:

1. Open a terminal. Navigate to the repository root.

2. Run:

$env:Path = "C:\msys64\ucrt64\bin;$env:Path"

3. Change directory to qt_app:

cd qt_app

4. Run CMake:

cmake -S. -B build -G Ninja

5. Build the project:

cmake --build build

## Run

To run AlgoTrack Qt execute:

.\build\AlgoTrackQt.exe

## Data Storage

AlgoTrack Qt saves problems to a CSV file called:

data/problems.csv

This file is located in the folder as the AlgoTrackQt.exe.

When you start AlgoTrack Qt again it loads the file automatically.

## Windows Release

To create a Windows release follow these steps:

1. Build the app first:

$env:Path = "C:\msys64\ucrt64\bin;$env:Path"

cd qt_app

cmake -S. -B build -G Ninja

cmake --build build

2. Deploy the Qt files:

windeployqt.\build\AlgoTrackQt.exe

A release ZIP should contain these files:

* AlgoTrackQt.exe

* Qt6Core.dll

* Qt6Gui.dll

* Qt6Widgets.dll

* Qt6Network.dll

* D3Dcompiler_47.dll

* platforms/

* styles/

* imageformats/

* generic/

* networkinformation/

* tls/

* translations/

* data/

Do not include these folders:

* source/

* build/

* CMakeFiles/

* CMakeCache.txt

## Usage

To use AlgoTrack Qt follow these steps:

1. Download the Qt release ZIP.

2. Extract the ZIP.

3. Run AlgoTrackQt.exe.