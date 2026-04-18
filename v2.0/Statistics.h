#pragma once
#include "Problem.h"

#include <vector>
#include <unordered_map>
#include <string>

int getTotalProblems(const std::vector<Problem>& problems);
std::vector<int> getStatuses(const std::vector<Problem>& problems);
std::vector<int> getDifficulties(const std::vector<Problem>& problems);
int totalTimeSpent(const std::vector<Problem>& problems);
double averageTimeSpent(const std::vector<Problem>& problems);
std::unordered_map<std::string, int> platformsUsage(const std::vector<Problem>& problems);
std::unordered_map<std::string, int> mostUsedTags(const std::vector<Problem>& problems);
std::pair<std::string, int> mostUsedTag(const std::unordered_map<std::string, int>& mostUsedTags);
double averageRating(const std::vector<Problem>& problems);
int longestProblem(const std::vector<Problem>& problems);
