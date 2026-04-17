#pragma once
#include "Problem.h"

class ProblemManager{
private:
    std::vector<Problem> problems;

public:
    AddProblemResult addProblem(const Problem& p);
    const std::vector<Problem>& getAllProblems() const;
    const Problem* getProblemAt(int index) ;
    SearchResult findByName(const std::string& name) const;
    void deleteAtIndex(int index);
    std::vector<std::string> filterByTag(const std::string& tag) const;
    std::vector<std::string> filterByPlatform(const std::string& platform) const;
    std::vector<std::string> filterByStatus(const Status& status) const;
    bool updateStatusAtIndex(int index, Status newStatus);
    bool updateNotesAtIndex(int index, const std::string& newNotes);
    bool sortByDifficulty();
    bool sortByTime();
    bool descendingSortByRating();
    bool existsProblem(const Problem& p) const;
    void setProblems(const std::vector<Problem>& newProblems);
};