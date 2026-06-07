#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include "ProblemManager.h"
#include "ProblemRepository.h"
#include "Utils.h"

constexpr const char *DATA_FILE = "data/problems.csv";

void autoSave(const ProblemManager &manager)
{
    ProblemRepository::saveToFile(DATA_FILE, manager.getAllProblems());
}

void printStats(const ProblemManager &manager)
{
    std::cout << "START_STATS\n";
    const auto &problems = manager.getAllProblems();
    std::cout << "total:" << problems.size() << "\n";

    int solved = 0, failed = 0, inprog = 0;
    int totalTime = 0;
    double ratingSum = 0;

    int easy = 0, medium = 0, hard = 0;
    std::map<std::string, int> dateCounts;

    for (const auto &p : problems)
    {
        if (p.getStatus() == Status::Solved)
            solved++;
        else if (p.getStatus() == Status::Failed)
            failed++;
        else
            inprog++;

        if (p.getDifficulty() == Difficulty::Easy)
            easy++;
        else if (p.getDifficulty() == Difficulty::Medium)
            medium++;
        else
            hard++;

        dateCounts[p.getDate()]++;
        totalTime += p.getTimeSpent();
        ratingSum += p.getRating();
    }

    std::cout << "solved:" << solved << "\n";
    std::cout << "failed:" << failed << "\n";
    std::cout << "progress:" << inprog << "\n";
    std::cout << "total_time:" << totalTime << "\n";
    if (!problems.empty())
    {
        std::cout << "avg_rating:" << (ratingSum / problems.size()) << "\n";
        std::cout << "avg_time:" << (totalTime / problems.size()) << "\n";
    }

    std::cout << "diff_easy:" << easy << "\n";
    std::cout << "diff_medium:" << medium << "\n";
    std::cout << "diff_hard:" << hard << "\n";

    for (auto const &[dateStr, count] : dateCounts)
    {
        std::cout << "date_freq:" << dateStr << "|" << count << "\n";
    }
    std::cout << "END_STATS\n";
}

int main()
{
    ProblemManager manager;

    LoadResult loadResult = ProblemRepository::loadFromFile(DATA_FILE);
    if (!loadResult.fileOpened)
    {
        loadResult = ProblemRepository::loadFromFile("qt_app/data/problems.csv");
    }
    if (loadResult.fileOpened)
    {
        manager.setProblems(loadResult.problems);
    }

    std::cout << "READY\n";

    std::string line;
    while (std::getline(std::cin, line))
    {
        line = trim(line);
        if (line.empty())
            continue;

        std::stringstream ss(line);
        std::string command;
        ss >> command;

        if (command == "EXIT")
            break;
        else if (command == "LIST")
        {
            std::cout << "START_LIST\n";
            for (const auto &p : manager.getAllProblems())
                std::cout << p.toCSV() << "\n";
            std::cout << "END_LIST\n";
        }
        else if (command == "STATS")
            printStats(manager);
        else if (command == "ADD")
        {
            std::string csv;
            std::getline(ss, csv);
            try
            {
                Problem p;
                p.fromCSV(trim(csv));
                if (manager.addProblem(p) == AddProblemResult::Success)
                {
                    autoSave(manager);
                    std::cout << "SUCCESS: ADDED\n";
                }
                else
                    std::cout << "ERROR: Duplicate problem\n";
            }
            catch (...)
            {
                std::cout << "ERROR: Invalid CSV format\n";
            }
        }
        else if (command == "DELETE_INDEX")
        {
            int idx;
            if (ss >> idx && manager.deleteAtIndex(idx))
            {
                autoSave(manager);
                std::cout << "SUCCESS: DELETED\n";
            }
            else
                std::cout << "ERROR: Invalid index\n";
        }
        else if (command == "SEARCH")
        {
            std::string query;
            std::getline(ss, query);
            std::vector<Problem> results = manager.searchAdvanced(trim(query));
            if (results.empty())
                std::cout << "NOT_FOUND\n";
            else
            {
                std::cout << "START_LIST\n";
                for (const auto &p : results)
                    std::cout << p.toCSV() << "\n";
                std::cout << "END_LIST\n";
            }
        }
        else if (command == "SORT_DIFFICULTY")
        {
            manager.sortByDifficulty();
            std::cout << "SUCCESS\n";
        }
        else if (command == "SORT_TIME")
        {
            manager.sortByTime();
            std::cout << "SUCCESS\n";
        }
        else if (command == "SORT_RATING")
        {
            manager.descendingSortByRating();
            std::cout << "SUCCESS\n";
        }
        else if (command == "OPEN")
        {
            std::string path;
            std::getline(ss, path);
            LoadResult lr = ProblemRepository::loadFromFile(trim(path));
            if (lr.fileOpened)
            {
                manager.setProblems(lr.problems);
                autoSave(manager);
                std::cout << "SUCCESS: OPENED\n";
            }
            else
                std::cout << "ERROR: Cannot open file\n";
        }
        else if (command == "SAVE")
        {
            if (ProblemRepository::saveToFile(DATA_FILE, manager.getAllProblems()))
                std::cout << "SUCCESS: SAVED\n";
            else
                std::cout << "ERROR: Cannot save file\n";
        }
        else if (command == "SAVE_AS")
        {
            std::string path;
            std::getline(ss, path);
            if (ProblemRepository::saveToFile(trim(path), manager.getAllProblems()))
                std::cout << "SUCCESS: SAVED_AS\n";
            else
                std::cout << "ERROR: Cannot save to file\n";
        }
        else if (command == "IMPORT")
        {
            std::string path;
            std::getline(ss, path);
            std::vector<Problem> tempProblems = manager.getAllProblems();
            if (ProblemRepository::importFromFile(trim(path), tempProblems).fileOpened)
            {
                manager.setProblems(tempProblems);
                autoSave(manager);
                std::cout << "SUCCESS: IMPORTED\n";
            }
            else
                std::cout << "ERROR: Import failed\n";
        }
        else if (command == "EXPORT")
        {
            std::string path;
            std::getline(ss, path);
            if (ProblemRepository::exportToFile(trim(path), manager.getAllProblems()))
                std::cout << "SUCCESS: EXPORTED\n";
            else
                std::cout << "ERROR: Export failed\n";
        }
        else
            std::cout << "ERROR: UNKNOWN_COMMAND\n";
    }
    return 0;
}