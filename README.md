# AlgoTrack

AlgoTrack is a C++ program that helps you keep track of the programming problems you have solved. It also helps you see how you are doing and saves your problem history in CSV files.

The project has two versions:

- Console version

- Qt desktop version

The console version is simple and works in the terminal. The Qt version has an interface that you can use on your desktop.

## Download

To get AlgoTrack go to the GitHub Releases page. Download the file called AlgoTrack for Windows.zip. When you open this file you will see two folders:

AlgoTrack-qt-windows/

AlgoTrack-console-windows/

Most people will want to use the Qt desktop version.

## Features

Here are some things AlgoTrack can do:

- Add and manage the problems you have solved

- Save your progress in CSV files

- Import and export CSV files

- Search for problems using a kind of search called fuzzy search

- See statistics about your progress

- Track the status of your problems

- See how difficult your problems are

- Track how much time you spend on problems and your rating

- See which tags you use the most

- Filter your problems by tag and platform

- Update the status and notes of your problems

- Sort your problems by difficulty time spent and rating

- Keep the different parts of the program separate

## Requirements

To build AlgoTrack from source you need:

- CMake 3.16 or newer

- A C++17 compiler

- Qt 6 for the desktop GUI version

- Ninja is recommended on Windows

## Build Console Version

To build the console version do this:

From the repository root run these commands:

cmake -S. -B build

cmake --build build

Then you can run AlgoTrack on Windows by typing:

.\build\AlgoTrack.exe

## Build Qt Desktop Version

First install Qt 6. On Windows with MSYS2 UCRT64 you can do this:

pacman -S mingw-w64-ucrt-x86_64-base mingw-w64-ucrt-x86_64-qt6-tools mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-ninja

Then build AlgoTrack:

$env:Path = "C:\msys64\ucrt64\bin;$env:Path"

cd qt_app

cmake -S. -B build -G Ninja

cmake --build build

Now you can run AlgoTrack by typing:

.\build\AlgoTrackQt.exe

## CSV Format

Each problem is saved in a CSV file like this:

name,platform,difficulty,tags,status,timeSpent,date,rating notes

For example:

two sum,easy,arrays|hashmap,solved,10,25-03-2026,5.0,very easy

dijkstra graph,atcoder,hard graphs|dijkstra,in_progress,60,26-03-2026,9.0,need to revisit

Some notes:

- Tags are separated by |

- The date format is DD-MM-YYYY

- The rating is between 1.0 and 10.0

## Qt Desktop App

The Qt version has:

- A table that shows your problems

- A dialog to add problems

- Automatic CSV saving

- Fuzzy search

- A statistics panel

- Controls to sort your problems

- CSV import and export

The Qt app saves your data automatically to a file called data/problems.csv in the same folder, as the executable.

## Release Files

On GitHub Releases you can download a Windows ZIP file called AlgoTrack for Windows.zip. When you open this file you will see two folders:

AlgoTrack-qt-windows/

AlgoTrack-console-windows/

The Qt folder has the desktop app and all the Qt runtime files. The console folder has the version.

The source repository should not have:

-.exe files

-.dll files

- build folders

- release ZIP/RAR files

## Author

AlgoTrack was created by Gabriel Popovici as a C++ project. He wanted to make a program that has an architecture, persistence and a desktop application.