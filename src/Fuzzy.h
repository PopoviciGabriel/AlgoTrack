#ifndef ALGOTRACK_FUZZY_H
#define ALGOTRACK_FUZZY_H

#include <string>
#include <vector>
#include <tuple>
#include "Problem.h"

std::tuple<int, int, std::string> fuzzy(const std::string &name, const std::vector<Problem> &problems);

#endif // ALGOTRACK_FUZZY_H
