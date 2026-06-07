#ifndef ALGOTRACK_PROBLEM_MANAGER_H
#define ALGOTRACK_PROBLEM_MANAGER_H

#include <vector>
#include <string>
#include "Problem.h"
#include "Enums.h"

enum class AddProblemResult
{
    Success,
    Duplicate
};

struct SearchResult
{
    bool exactFound = false;
    bool fuzzyFound = false;
    int index = -1;
    int distance = -1;
    std::string closestMatch;
};

class ProblemManager
{
public:
    AddProblemResult addProblem(const Problem &p);
    const std::vector<Problem> &getAllProblems() const;
    const Problem *getProblemAt(int index) const;
    SearchResult findByName(const std::string &name) const;

    // Motorul nou de căutare avansată
    std::vector<Problem> searchAdvanced(const std::string &query) const;

    bool deleteAtIndex(int index);

    std::vector<Problem> filterByTag(const std::string &tag) const;
    std::vector<Problem> filterByPlatform(const std::string &platform) const;
    std::vector<Problem> filterByStatus(Status status) const;

    bool updateStatusAtIndex(int index, Status newStatus);
    bool updateNotesAtIndex(int index, const std::string &newNotes);

    bool sortByDifficulty();
    bool sortByTime();
    bool descendingSortByRating();

    void setProblems(std::vector<Problem> newProblems);

private:
    std::vector<Problem> problems;
    bool existsProblem(const Problem &p) const;
};

#endif // ALGOTRACK_PROBLEM_MANAGER_H