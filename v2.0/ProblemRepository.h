#pragma once

#include <string>
#include <vector>
#include "Problem.h"

struct ImportResult {
    bool fileOpened = true;
    int importedCount = 0;
    int duplicateCount = 0;
    int invalidCount = 0;
};

class ProblemRepository {
public:
    static std::vector<Problem> loadFromFile(const std::string& filename);
    static bool saveToFile(const std::string& filename, const std::vector<Problem>& problems);

    static ImportResult importFromFile(const std::string& filename, std::vector<Problem>& problems);
    static bool exportToFile(const std::string& filename, const std::vector<Problem>& problems);
};