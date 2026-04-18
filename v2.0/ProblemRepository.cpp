#include "ProblemRepository.h"
#include "Utils.h"

#include <fstream>

static bool problemExistsInVector(const Problem& p, const std::vector<Problem>& problems) {
    for (const auto& existing : problems) {
        if (toLowerCase(existing.getName()) == toLowerCase(p.getName()) &&
            toLowerCase(existing.getPlatform()) == toLowerCase(p.getPlatform())) {
            return true;
        }
    }
    return false;
}

std::vector<Problem> ProblemRepository::loadFromFile(const std::string& filename) {
    std::vector<Problem> loadedProblems;

    std::ifstream file(filename);
    if (!file.is_open()) {
        return loadedProblems;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) {
            continue;
        }

        try {
            Problem p;
            p.fromCSV(line);

            if (!problemExistsInVector(p, loadedProblems)) {
                loadedProblems.push_back(p);
            }
        } catch (...) {
        }
    }

    return loadedProblems;
}

bool ProblemRepository::saveToFile(const std::string& filename, const std::vector<Problem>& problems) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        return false;
    }

    for (const auto& p : problems) {
        file << p.toCSV() << "\n";
    }

    return true;
}

ImportResult ProblemRepository::importFromFile(const std::string& filename, std::vector<Problem>& problems) {
    ImportResult result;

    std::ifstream file(filename);
    if (!file.is_open()) {
        result.invalidCount = -1;
        return result;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) {
            continue;
        }

        try {
            Problem p;
            p.fromCSV(line);

            if (problemExistsInVector(p, problems)) {
                result.duplicateCount++;
            } else {
                problems.push_back(p);
                result.importedCount++;
            }
        } catch (...) {
            result.invalidCount++;
        }
    }

    return result;
}

bool ProblemRepository::exportToFile(const std::string& filename, const std::vector<Problem>& problems) {
    return saveToFile(filename, problems);
}