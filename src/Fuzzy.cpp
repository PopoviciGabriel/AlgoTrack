#include "Fuzzy.h"
#include "Utils.h"

std::tuple<int, int, std::string> fuzzy(const std::string& name, const std::vector<Problem>& problems) {
    std::string searchedName = toLowerCase(name);

    int bestDistance = 1e9;
    int bestIndex = -1;
    std::string closestMatch;

    for (int i = 0; i < (int)problems.size(); i++) {
        std::string problemName = toLowerCase(problems[i].getName());
        int dist = levenshteinDistance(problemName, searchedName);

        if (dist < bestDistance) {
            bestDistance = dist;
            bestIndex = i;
            closestMatch = problems[i].getName();
        }
    }

    return {bestIndex, bestDistance, closestMatch};
}