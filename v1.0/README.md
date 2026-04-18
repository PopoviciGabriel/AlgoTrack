# AlgoTrack

AlgoTrack is a console application that helps you track your programming problems monitor your progress and analyze how you are doing over time. It is written in C++20.

I built this project for myself to help me keep track of my problems and to get better at writing organized code. I also wanted to practice designing software that's easy to use and understand.

---

## Features

* You can add, update and delete problems

* You can search for problems using a kind of search that finds similar words

* You can filter problems by:

* how hard they are (Easy, Medium, Hard)

* what is happening with them (Solved, Failed In Progress)

* where they are from and what they are about

* You can. Export your data to and from files

* AlgoTrack has a built-in system to track your progress and statistics

* It makes sure that all the information you enter is correct and makes sense

---

## Project Structure

* `Problem` files handle the information about each problem. Make sure it is correct

* `ProblemManager` files do the work of adding, deleting, searching and filtering problems

* `ProblemRepository` files take care of saving and loading your data to and from files

* `Statistics` files. Show you information about your progress

* `Fuzzy` files do the kind of search that finds similar words

* `ConsoleUI` files handle all the interaction between you and the application

* `Utils` files have helpful functions for working with strings, files and dates

---

## Tech Stack

* C++20 is the programming language used

* Object-Oriented Programming is used to make the code organized and easy to understand

* The application is designed to have parts for different tasks

* Files are used to save and load your data

* A custom search function is used to find words

---

## How to Run

### Compile:

```bash

g++ -std=c++20 main.cpp Problem.cpp ProblemManager.cpp ProblemRepository.cpp Statistics.cpp Fuzzy.cpp Utils.cpp ConsoleUI.cpp -o AlgoTrack

```

### Run:

```bash

./AlgoTrack

```

---

## Example Use Case

AlgoTrack helps you to:

* keep track of the problems you have solved from places like LeetCode and Codeforces

* see how many problems you have solved and how hard they were

* quickly find problems using the special search function

* take notes and give ratings to each problem

---

## Future Improvements

* A version with a graphical user interface

* advanced statistics and visualizations

* tests to make sure the application works correctly

* better ways to filter and sort your problems

* support for more file formats like JSON

---

## Why I Built This

I built AlgoTrack to help me track my own progress and to get better at writing C++ code. I wanted to make an application that is useful not just a theoretical exercise.

The main goals of this project were:

* to make the code clean and organized

* to separate tasks into different parts

* to make sure the data is correct and makes sense

* to build something practical and useful

---

## Feedback

I would love to hear your thoughts and suggestions. If you have ideas, for features or ways to improve AlgoTrack please let me know.