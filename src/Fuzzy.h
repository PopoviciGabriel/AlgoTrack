#pragma once
#include <tuple>
#include <string>
#include <vector>
#include "Problem.h"

std::tuple<int, int, std::string> fuzzy(const std::string& name, const std::vector<Problem>& problems);