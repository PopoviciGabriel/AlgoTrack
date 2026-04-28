# AlgoTrack Qt

AlgoTrack Qt is the desktop version of AlgoTrack that you can use on your computer. It was built using C++17 and Qt 6.

AlgoTrack Qt uses the main parts as the console version. These include:

- problem management

- saving data to CSV files

- search

- statistics

- sorting

## Requirements

To use AlgoTrack Qt you need to have the following things:

- Qt 6

- CMake 3.16 or a newer version

- a C++17 compiler

- Ninja is recommended, but not required

If you are using Windows with MSYS2 UCRT64 you can install the required packages with this command:

pacman -S mingw-w64-ucrt-x86_64-qt6-base mingw-w64-ucrt-x86_64-qt6-tools mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-ninja

## Build

To build AlgoTrack Qt follow these steps from the repository root:

$env:Path = "C:\msys64\ucrt64\bin;$env:Path"

Then go to the qt_app folder.

Use this command to create the build files: cmake -S. -B build -G Ninja

Then use this command to build AlgoTrack Qt: cmake --build build

## Run

You can run AlgoTrack Qt by opening the file.\build\AlgoTrackQt.exe

## Data Storage

AlgoTrack Qt automatically saves your problems to a file called data/problems.csv. This file is located in the folder as the AlgoTrack Qt program.

When you start AlgoTrack Qt again it will load the problems from this file automatically.

## Windows Release

The public version of AlgoTrack for Windows is available in a ZIP file called AlgoTrack for Windows.zip.

Inside this ZIP file you will find the AlgoTrack Qt desktop app in a folder called AlgoTrack-qt-windows.

To run it follow these steps:

1. Extract the ZIP file.

2. Open the AlgoTrack-qt-windows folder.

3. Run the file AlgoTrackQt.exe.

Make sure to keep all the files and folders inside AlgoTrack-qt-windows

Do not move the AlgoTrackQt.exe file outside its folder because it needs the included Qt runtime files to work

## Manual Deployment

If you want to deploy AlgoTrack Qt, on Windows you need to build it first.

Use these commands to build AlgoTrack Qt:

$env:Path = "C:\msys64\ucrt64\bin;$env:Path"

Then go to the qt_app folder.

Use this command to create the build files: cmake -S. -B build -G Ninja

Then use this command to build AlgoTrack Qt: cmake --build build

After building AlgoTrack Qt you need to deploy the required Qt files.

Use this command to do this: windeployqt.\build\AlgoTrackQt.exe

The Qt release folder should contain these files:

AlgoTrackQt.exe

Qt6Core.dll

Qt6Gui.dll

Qt6Widgets.dll

Qt6Network.dll

D3Dcompiler_47.dll

And these folders:

platforms

styles

imageformats

generic

networkinformation

tls

translations

data

Do not include these files and folders:

source

build

CMakeFiles

CMakeCache.txt