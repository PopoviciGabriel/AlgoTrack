# AlgoTrack

AlgoTrack is a cross-platform desktop application built with **C++17** and **Qt6** designed to help software engineers and students efficiently track their algorithmic problem-solving progress (e.g., LeetCode, Codeforces).

Track problems, analyze patterns, and review statistics in one calm, unified workspace.

## 🏗️ Architecture Highlight

The project is built emphasizing production-ready C++ standards and a strict separation of concerns:

- **Inter-Process Communication (IPC):** The application is split into two independent processes. The GUI (`AlgoTrackQt`) acts purely as a presentation layer and runs the backend business logic (`AlgoTrackCli`) as a child process, communicating via standard I/O streams.
- **Modern C++ Idioms:** The core library utilizes strict **RAII** principles for robust object initialization, ensuring class invariants are established immediately.
- **Pimpl Idiom:** The Qt UI components (like the Add Problem Dialog) employ the Pointer to Implementation (Pimpl) idiom via custom `std::unique_ptr` deleters, completely hiding implementation details from the public headers and drastically reducing recompilation times.

## ✨ Features

- **Problem Tracking:** Log problems with details like Platform, Difficulty, Tags, Time Spent, and Date.
- **Advanced Search & Filter:** Includes both exact string matching and **Levenshtein distance (Fuzzy Matching)** algorithms to find problems even with typos.
- **Dynamic Statistics:** Real-time calculation of solved/failed ratios, average times, average ratings, and most-used algorithmic tags.
- **Data Persistence:** Imports and exports clean `.csv` files.
- **Sorting:** Organize the workspace by Difficulty, Time Spent, or Personal Rating.

## 🚀 Getting Started

### Prerequisites

- **C++17** compatible compiler (GCC, Clang, or MSVC)
- **CMake** (>= 3.15)
- **Qt6** (Widgets module)

### Build Instructions

1. Clone the repository:
   ```bash
   git clone [https://github.com/PopoviciGabriel/AlgoTrack.git](https://github.com/PopoviciGabriel/AlgoTrack.git)
   cd AlgoTrack
   ```
