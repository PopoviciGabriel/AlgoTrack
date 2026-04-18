#include "Statistics.h"

#include <numeric>

int getTotalProblems(const std::vector<Problem>& problems) {
    return problems.size();
}

std::vector<int> getStatuses(const std::vector<Problem>& problems) {
    std::vector<int> statuses(3, 0);
    // 0 - "solved"
    // 1 - "failed"
    // 2 - "in_progress"

    for (const auto& p : problems) {
        if (p.getStatus() == Status::Solved) {
            statuses[0]++;
        }
        else if (p.getStatus() == Status::Failed) {
            statuses[1]++;
        }
        else if (p.getStatus() == Status::InProgress) {
            statuses[2]++;
        }
    }

    return statuses;
}

std::vector<int> getDifficulties(const std::vector<Problem>& problems) {
    std::vector<int> difficulties(3, 0);
    // 0 - "easy"
    // 1 - "medium"
    // 2 - "hard"
    for (const auto& p : problems) {
        if (p.getDifficulty() == Difficulty::Easy) {
            difficulties[0]++;
        }
        else if (p.getDifficulty() == Difficulty::Medium) {
            difficulties[1]++;
        }
        else if (p.getDifficulty() == Difficulty::Hard) {
            difficulties[2]++;
        }
    }

    return difficulties;
}

int totalTimeSpent(const std::vector<Problem>& problems) {
    int total = std::accumulate(
        problems.begin(),
        problems.end(),
        0,
        [](int sum, const Problem& p) {
            return sum + p.getTimeSpent();
        }
    );

    return total;
}

double averageTimeSpent(const std::vector<Problem>& problems) {
    const int totalProblems = getTotalProblems(problems);
    if (totalProblems == 0) return 0.0;

    const int totalTime = totalTimeSpent(problems);
    return static_cast<double>(totalTime) / totalProblems;
}

std::unordered_map<std::string, int> platformsUsage(const std::vector<Problem>& problems) {
    std::unordered_map<std::string, int> platforms;

    for (const auto& p : problems) {
            platforms[p.getPlatform()]++;
    }
    return platforms;
}

std::unordered_map<std::string, int> mostUsedTags(const std::vector<Problem>& problems) {
    std::unordered_map<std::string, int> tags;

    for (const auto& p : problems) {
        std::vector<std::string> Ptags = p.getTags();

        for (const auto& tag : Ptags) {
            tags[tag]++;
        }
    }

    return tags;
}

std::pair<std::string, int> mostUsedTag(const std::unordered_map<std::string, int>& mostUsedTags) {
    std::string name;
    int count = 0;

    for (const auto& t : mostUsedTags) {
        if (t.second > count) {
            name = t.first;
            count = t.second;
        }
    }

    return {name, count};
}

double averageRating(const std::vector<Problem>& problems) {
    if (problems.empty()) return 0.0;

    double total = 0;
    for (const auto& p : problems) {
        total += p.getRating();
    }

    return total / problems.size();
}

int longestProblem(const std::vector<Problem>& problems) {
    int ans = 0;
    for (const auto& p : problems) {
        int time = p.getTimeSpent();
        if (time > ans) {
            ans = time;
        }
    }

    return ans;
}
