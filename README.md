# AlgoTrack

> A modern C++ console application for tracking and analyzing solved problems.

## Features

* Fuzzy search (Levenshtein distance)

* CSV persistence (import/export support)

* Statistics dashboard

* Total problems

* Status distribution (Solved / Failed / In Progress)

* Difficulty breakdown

* time & rating

* Most used tags

* Tag & platform filtering

* Update system (status & notes)

* Sorting

* By difficulty

* By time spent

* By rating (descending)

* logic/UI separation

* Built with C++ (C++17)

## Project Structure

```

AlgoTrack/

│

├── src/

│   ├── Problem.*            # core entity

│   ├── ProblemManager.*    # business logic

│   ├── ProblemRepository.*# file handling (CSV)

│   ├── Statistics.*       # analytics engine

│   ├── Fuzzy.*            # fuzzy search (Levenshtein)

│   ├── Utils.*            # helpers

│   ├── ConsoleUI.*        # user interface

│   └── main.cpp

│

├── data/

│   └── problems.example.csv

│

├── CMakeLists.txt

└── README.md

```

## Build & Run

### Requirements

* CMake ≥ 3.16

* C++17 compiler (g++, clang, MSVC)

### Build

```bash

mkdir build

cd build

cmake..

cmake --build.

```

### Run

```bash

./AlgoTrack

```

Windows (PowerShell):

```bash

.\AlgoTrack.exe

```

## CSV Format

Each problem is stored as:

```

name,platform,difficulty,tags,status,timeSpent,date,rating notes

```

### Example:

```

two sum,easy,arrays|hashmap,solved,10,25-03-2026,5.0,very easy

dijkstra graph,atcoder hard graphs|dijkstra solved,60,26-03-2026,9.0,important problem

```

Notes:

* Tags are separated by `|`

* Date format: `DD-MM-YYYY`

* Rating: `1.0. 10.0`

## Design Highlights

* Strong separation of concerns

* Logic ≠ UI ≠ persistence

* Input validation (exceptions)

* Custom CSV parser (handles quotes & commas)

* Fuzzy matching system for UX

* Modular architecture (easy to extend)

## Example Use Cases

* Track solved problems from platforms like:

* LeetCode

* Codeforces

* AtCoder

* Analyze your progress over time

* Identify areas (difficulty / tags)

## Future Improvements

* [ ] GUI version (Qt / ImGui)

* [ ] JSON support

* [ ] Charts / visualization

* [ ] Cloud sync

* [ ] Tag-based recommendations

## Author

Built as a project to improve:

* Modern C++

* Clean architecture

* Real-world application design

## If you like it

Give it a star, on GitHub. It helps a lot!