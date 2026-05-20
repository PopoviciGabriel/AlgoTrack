#ifndef ALGOTRACK_STATISTICS_H
#define ALGOTRACK_STATISTICS_H

#include <vector>
#include <string>
#include <map>
#include "Problem.h"

struct StatisticsSummary
{
    int totalProblems = 0;
    int solved = 0;
    int failed = 0;
    int inProgress = 0;
    int easy = 0;
    int medium = 0;
    int hard = 0;
    int totalTime = 0;
    double averageTime = 0.0;
    double averageRating = 0.0;
    int longestProblemTime = 0;
    std::string mostUsedTag;
    int mostUsedTagCount = 0;
    std::map<std::string, int> platformUsage;
    std::map<std::string, int> tagUsage;
};

StatisticsSummary calculateStatistics(const std::vector<Problem> &problems);

#endif // ALGOTRACK_STATISTICS_H
