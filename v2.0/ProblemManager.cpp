#include "ProblemManager.h"
#include "Statistics.h"
#include "Utils.h"
#include "Fuzzy.h"
#include <iostream>
#include <map>
#include <algorithm>
#include <fstream>
#include <limits>

AddProblemResult ProblemManager::addProblem(const Problem& p) {
    if (existsProblem(p)) {
        return AddProblemResult::Duplicate;
    }

    problems.push_back(p);
    return AddProblemResult::Success;
}

const std::vector<Problem>& ProblemManager::getAllProblems() const {
    return problems;
}

const Problem* ProblemManager::getProblemAt(int index) {
    if (index < 0 || index >= (int)problems.size())
        return nullptr;
    return &problems[index];
}

SearchResult ProblemManager::findByName(const std::string& name) const {
    auto [index, distance, closestMatch] = fuzzy(name, problems);

    if (index != -1 && distance == 0) {
        return {true, false, index, distance, closestMatch};
    }

    int threshold = getFuzzyThreshold(name);
    if(index != -1 && distance <= threshold) 
        return {false, true, index, distance, closestMatch};
    else
        return {};
}

void ProblemManager::deleteAtIndex(int index) {
    problems.erase(problems.begin() + index);
}

std::vector<std::string> ProblemManager::filterByTag(const std::string& tag) const {
    std::vector<std::string> res;
    std::string searchedTag = toLowerCase(tag);

    for (const auto& p : problems) {
        for (const auto& t : p.getTags()) {
            if (toLowerCase(t) == searchedTag) {
                res.push_back(p.getName());
                break;
            }
        }
    }
    return res;
}

std::vector<std::string> ProblemManager::filterByPlatform(const std::string& platform) const {
    std::vector<std::string> res;
    std::string searchedPlatform = toLowerCase(platform);

    for (const auto& p : problems) {
        if (searchedPlatform == toLowerCase(p.getPlatform())) {
                res.push_back(p.getName());
        }
    }
    return res;
}

std::vector<std::string> ProblemManager::filterByStatus(const Status& status) const {
    std::vector<std::string> res;

    for (const auto& p : problems) {
        if (status == p.getStatus()) 
            res.push_back(p.getName());
    }
    return res;
}

bool ProblemManager::updateStatusAtIndex(int index, Status newStatus) {
    if (index < 0 || index >= (int)problems.size())
        return false;

    problems[index].changeStatus(newStatus);
    return true;
}

bool ProblemManager::updateNotesAtIndex(int index, const std::string& newNotes) {
    if (index < 0 || index >= (int)problems.size())
        return false;

    problems[index].changeNotes(newNotes);
    return true;
}

bool ProblemManager::sortByDifficulty() {
    if((int)problems.size() == 0) return false;

    std::sort(problems.begin(), problems.end(),
        [](const Problem& a, const Problem& b) {
            return static_cast<int>(a.getDifficulty()) < static_cast<int>(b.getDifficulty());
    });

    return true;
}

bool ProblemManager::sortByTime() {
    if((int)problems.size() == 0) return false;

    std::sort(problems.begin(), problems.end(),
        [](const Problem& p1, const Problem& p2) {
            return p1.getTimeSpent() < p2.getTimeSpent();
        });

    return true;
}

bool ProblemManager::descendingSortByRating() {
    if((int)problems.size() == 0) return false;

    std::sort(problems.begin(), problems.end(),
        [](const Problem& p1, const Problem& p2){
            return p1.getRating() > p2.getRating();
    });

    return true;
}

bool ProblemManager::existsProblem(const Problem& p) const {
    for (const auto& existing : problems) {
        if (toLowerCase(existing.getName()) == toLowerCase(p.getName()) &&
            toLowerCase(existing.getPlatform()) == toLowerCase(p.getPlatform())) {
            return true;
        }
    }
    return false;
}

void ProblemManager::setProblems(const std::vector<Problem>& newProblems) {
    problems = std::move(newProblems);
}
