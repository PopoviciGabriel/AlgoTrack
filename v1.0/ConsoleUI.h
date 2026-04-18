#pragma once
#include "Problem.h"
#include "ProblemManager.h"
#include <iostream>

Problem readProblemFromConsole();
void displayProblem(const Problem& p);
void showStatisticsUI(const ProblemManager& manager);
Difficulty readDifficulty();
Status readStatus();