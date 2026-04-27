#include "Statistics.h"

#include <numeric>

StatisticsSummary calculateStatistics(const std::vector<Problem>& problems) {
    StatisticsSummary summary;

    summary.totalProblems = static_cast<int>(problems.size());

    for (const auto& p : problems) {
        // status
        if (p.getStatus() == Status::Solved) {
            summary.solved++;
        } else if (p.getStatus() == Status::Failed) {
            summary.failed++;
        } else if (p.getStatus() == Status::InProgress) {
            summary.inProgress++;
        }

        // difficulty
        if (p.getDifficulty() == Difficulty::Easy) {
            summary.easy++;
        } else if (p.getDifficulty() == Difficulty::Medium) {
            summary.medium++;
        } else if (p.getDifficulty() == Difficulty::Hard) {
            summary.hard++;
        }

        // time
        summary.totalTime += p.getTimeSpent();

        if (p.getTimeSpent() > summary.longestProblemTime) {
            summary.longestProblemTime = p.getTimeSpent();
        }

        // rating
        summary.averageRating += p.getRating();

        // platform
        summary.platformUsage[p.getPlatform()]++;

        // tags
        for (const auto& tag : p.getTags()) {
            summary.tagUsage[tag]++;
        }
    }

    if (summary.totalProblems > 0) {
        summary.averageTime =
            static_cast<double>(summary.totalTime) / summary.totalProblems;

        summary.averageRating =
            summary.averageRating / summary.totalProblems;
    }

    for (const auto& tag : summary.tagUsage) {
        if (tag.second > summary.mostUsedTagCount) {
            summary.mostUsedTag = tag.first;
            summary.mostUsedTagCount = tag.second;
        }
    }

    return summary;
}