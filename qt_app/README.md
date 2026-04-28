# AlgoTrack Qt

AlgoTrack Qt is the desktop version of AlgoTrack that you can use on your computer. It was built using C++17 and Qt 6.

AlgoTrack Qt uses the core logic as the console version of AlgoTrack. This means it can do things like:

- manage problems

- save data to a CSV file

- do fuzzy searches

- show statistics

- sort things

## Requirements

To use AlgoTrack Qt you need to have:

- Qt 6

- CMake 3.16 or a newer version

- a C++17 compiler

- it is recommended that you use Ninja

If you are using Windows with MSYS2 UCRT64 you can install the required packages with this command:

pacman -S mingw-w64-ucrt-x86_64-qt6-base mingw-w64-ucrt-x86_64-qt6-tools mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-ninja

## Build

To build AlgoTrack Qt do the following from the repository root:

$env:Path = "C:\msys64\ucrt64\bin;$env:Path"

then go to the qt_app folder

cd qt_app

then run these commands:

cmake -S. -B build -G Ninja

cmake --build build

## Run

To run AlgoTrack Qt run the AlgoTrackQt.exe file that you find in the build folder:

.\build\AlgoTrackQt.exe

## Data Storage

The AlgoTrack Qt app will automatically save your problems to a file called problems.csv. This file is stored in the folder as the AlgoTrack Qt executable. When you start the app again it will load the problems, from this file automatically.

## Windows Release

To make a Windows release of AlgoTrack Qt first build the app:

$env:Path = "C:\msys64\ucrt64\bin;$env:Path"

cd qt_app

cmake -S. -B build -G Ninja

cmake --build build

Then you need to deploy the required Qt files:

windeployqt.\build\AlgoTrackQt.exe

The release ZIP file should contain these files:

AlgoTrackQt.exe

Qt6Core.dll

Qt6Gui.dll

Qt6Widgets.dll

Qt6Network.dll

D3Dcompiler_47.dll

and these folders:

platforms

styles

imageformats

generic

networkinformation

tls

translations

data

Do not include these files and folders in the release ZIP:

source

build

CMakeFiles

CMakeCache.txt

## Usage

To use AlgoTrack Qt download the Qt release ZIP file extract it then run the AlgoTrackQt.exe file.