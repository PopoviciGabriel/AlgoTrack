#include "Fsrs.h"
#include "Utils.h"

#include <cmath>
#include <stdexcept>

ReviewState scheduleReview(const ReviewState &current, ReviewRating rating, const std::string &todayIso)
{
    if (!isValidIsoDate(todayIso))
        throw std::invalid_argument("Review date must be YYYY-MM-DD.");

    ReviewState next = current;
    if (!(next.stability > 0.0))
        next.stability = 1.0;
    if (next.intervalDays < 0)
        next.intervalDays = 0;
    if (next.repetitionCount < 0)
        next.repetitionCount = 0;

    if (rating == ReviewRating::Again)
    {
        next.stability = std::max(1.0, next.stability * 0.5);
        next.intervalDays = 1;
        next.repetitionCount = 0;
    }
    else
    {
        next.repetitionCount += 1;
        if (next.repetitionCount == 1)
        {
            if (rating == ReviewRating::Hard)
                next.intervalDays = 1;
            else if (rating == ReviewRating::Good)
                next.intervalDays = 3;
            else
                next.intervalDays = 7;
            next.stability = std::max(next.stability, static_cast<double>(next.intervalDays));
        }
        else
        {
            double growth = 2.0;
            if (rating == ReviewRating::Hard)
                growth = 1.2;
            else if (rating == ReviewRating::Easy)
                growth = 2.5;
            next.stability = std::max(1.0, next.stability * growth);
            next.intervalDays = std::max(1, static_cast<int>(std::lround(next.stability)));
        }
    }

    if (next.intervalDays > 36500)
        next.intervalDays = 36500;
    next.dueDate = addDaysToIsoDate(todayIso, next.intervalDays);
    return next;
}
