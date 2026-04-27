#pragma once
#include "Problem.h"

#include <vector>
#include <unordered_map>
#include <string>

struct StatisticsSummary {
    int totalProblems = 0;

    int solved = 0;
    int failed = 0;
    int inProgress = 0;

    int easy = 0;
    int medium = 0;
    int hard = 0;

    int totalTime = 0;
    double averageTime = 0.0;

    std::unordered_map<std::string, int> platformUsage;
    std::unordered_map<std::string, int> tagUsage;

    std::string mostUsedTag = "";
    int mostUsedTagCount = 0;

    double averageRating = 0.0;
    int longestProblemTime = 0;
};

StatisticsSummary calculateStatistics(const std::vector<Problem>& problems);


