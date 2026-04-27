#pragma once
#include <string>

enum class Difficulty { Easy, Medium, Hard };
enum class Status { Solved, Failed, InProgress };
enum class AddProblemResult { Success, Duplicate };

struct SearchResult{
    bool exactFound = false;
    bool fuzzyFound = false;
    int index = -1;
    int distance = -1;
    std::string closestMatch;
};