#pragma once

#include "Problem.h"
#include "ProblemManager.h"
#include "Enums.h"

#include <string>

Problem readProblemFromConsole();
void displayProblem(const Problem& p);
void showStatisticsUI(const ProblemManager& manager);

Difficulty readDifficulty();
Status readStatus();

int readInt(const std::string& prompt);

void runConsoleApp();