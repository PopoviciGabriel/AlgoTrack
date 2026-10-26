#ifndef ALGOTRACK_FSRS_H
#define ALGOTRACK_FSRS_H

#include <string>
#include "Enums.h"

struct ReviewState
{
    double stability = 1.0;
    int intervalDays = 0;
    int repetitionCount = 0;
    std::string dueDate;
};

// Minimal SM-2 / FSRS schedule. Again always returns the card tomorrow.
ReviewState scheduleReview(const ReviewState &current, ReviewRating rating, const std::string &todayIso);

#endif // ALGOTRACK_FSRS_H
