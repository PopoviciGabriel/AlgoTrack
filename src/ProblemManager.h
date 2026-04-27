#pragma once

#include <string>
#include <vector>

#include "Problem.h"
#include "Utils.h"
#include "Enums.h"

class ProblemManager{
private:
    std::vector<Problem> problems;

public:
    AddProblemResult addProblem(const Problem& p);
    const std::vector<Problem>& getAllProblems() const;
    const Problem* getProblemAt(int index) const;
    SearchResult findByName(const std::string& name) const;
    bool deleteAtIndex(int index);
    std::vector<Problem> filterByTag(const std::string& tag) const;
    std::vector<Problem> filterByPlatform(const std::string& platform) const;
    std::vector<Problem> filterByStatus(Status status) const;
    bool updateStatusAtIndex(int index, Status newStatus);
    bool updateNotesAtIndex(int index, const std::string& newNotes);
    bool sortByDifficulty();
    bool sortByTime();
    bool descendingSortByRating();
    bool existsProblem(const Problem& p) const;
    void setProblems(std::vector<Problem> newProblems);
};