#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include "ProblemManager.h"
#include "ProblemRepository.h"
#include "Statistics.h"
#include "Utils.h"

constexpr const char *DATA_FILE = "data/problems.csv";

void autoSave(const ProblemManager &manager)
{
    ProblemRepository::saveToFile(DATA_FILE, manager.getAllProblems());
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
    else
    {
        std::cout << "ERROR: Nu s-a putut deschide fisierul de date. Aplicatia va porni goala." << std::endl;
    }

    std::cout << "READY" << std::endl;

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
        {
            std::cout << "OK: BYE" << std::endl;
            break;
        }
        else if (command == "LIST")
        {
            const auto &all = manager.getAllProblems();
            std::cout << "START_LIST" << std::endl;
            for (const auto &p : all)
            {
                std::cout << p.toCSV() << std::endl;
            }
            std::cout << "END_LIST" << std::endl;
        }
        else if (command == "ADD")
        {
            std::string csvData;
            std::getline(ss, csvData);
            csvData = trim(csvData);

            try
            {
                Problem p;
                p.fromCSV(csvData);

                if (manager.addProblem(p) == AddProblemResult::Duplicate)
                {
                    std::cout << "ERROR: DUPLICATE" << std::endl;
                }
                else
                {
                    autoSave(manager);
                    std::cout << "SUCCESS: ADDED" << std::endl;
                }
            }
            catch (const std::exception &e)
            {
                std::cout << "ERROR: " << e.what() << std::endl;
            }
        }
        else if (command == "SEARCH")
        {
            std::string name;
            std::getline(ss, name);
            name = trim(name);

            SearchResult res = manager.findByName(name);
            if (res.exactFound)
            {
                std::cout << "FOUND_EXACT|" << manager.getProblemAt(res.index)->getName() << "|" << manager.getProblemAt(res.index)->toCSV() << std::endl;
            }
            else if (res.fuzzyFound)
            {
                // CORECTAT: Trimitem linia CSV completă prin toCSV() și la Fuzzy Match, nu doar numele!
                std::cout << "FOUND_FUZZY|" << manager.getProblemAt(res.index)->getName() << "|" << manager.getProblemAt(res.index)->toCSV() << std::endl;
            }
            else
            {
                std::cout << "NOT_FOUND" << std::endl;
            }
        }
        else if (command == "DELETE_INDEX")
        {
            int index;
            if (ss >> index)
            {
                if (manager.deleteAtIndex(index))
                {
                    autoSave(manager);
                    std::cout << "SUCCESS: DELETED" << std::endl;
                }
                else
                {
                    std::cout << "ERROR: INVALID_INDEX" << std::endl;
                }
            }
        }
        else if (command == "UPDATE_STATUS")
        {
            int index;
            std::string statusStr;
            if (ss >> index)
            {
                std::getline(ss, statusStr);
                statusStr = trim(statusStr);
                try
                {
                    Status newStatus = parseStatus(statusStr);
                    if (manager.updateStatusAtIndex(index, newStatus))
                    {
                        autoSave(manager);
                        std::cout << "SUCCESS: STATUS_UPDATED" << std::endl;
                    }
                    else
                    {
                        std::cout << "ERROR: INVALID_INDEX" << std::endl;
                    }
                }
                catch (...)
                {
                    std::cout << "ERROR: INVALID_STATUS" << std::endl;
                }
            }
        }
        else if (command == "UPDATE_NOTES")
        {
            int index;
            std::string notes;
            if (ss >> index)
            {
                std::getline(ss, notes);
                notes = trim(notes);
                if (manager.updateNotesAtIndex(index, notes))
                {
                    autoSave(manager);
                    std::cout << "SUCCESS: NOTES_UPDATED" << std::endl;
                }
                else
                {
                    std::cout << "ERROR: INVALID_INDEX" << std::endl;
                }
            }
        }
        else if (command == "STATS")
        {
            StatisticsSummary stats = calculateStatistics(manager.getAllProblems());
            std::cout << "START_STATS" << std::endl;
            std::cout << "total:" << stats.totalProblems << std::endl;
            std::cout << "solved:" << stats.solved << std::endl;
            std::cout << "failed:" << stats.failed << std::endl;
            std::cout << "progress:" << stats.inProgress << std::endl;
            std::cout << "avg_time:" << stats.averageTime << std::endl;
            std::cout << "avg_rating:" << stats.averageRating << std::endl;
            std::cout << "most_used_tag:" << (stats.mostUsedTagCount > 0 ? stats.mostUsedTag : "none") << std::endl;
            std::cout << "END_STATS" << std::endl;
        }
        else if (command == "SORT_DIFFICULTY")
        {
            manager.sortByDifficulty();
            autoSave(manager);
            std::cout << "SUCCESS: SORTED" << std::endl;
        }
        else if (command == "SORT_TIME")
        {
            manager.sortByTime();
            autoSave(manager);
            std::cout << "SUCCESS: SORTED" << std::endl;
        }
        else if (command == "SORT_RATING")
        {
            manager.descendingSortByRating();
            autoSave(manager);
            std::cout << "SUCCESS: SORTED" << std::endl;
        }
        else if (command == "OPEN")
        {
            std::string path;
            std::getline(ss, path);
            path = trim(path);

            LoadResult res = ProblemRepository::loadFromFile(path);
            if (res.fileOpened)
            {
                manager.setProblems(res.problems);
                std::cout << "SUCCESS: OPENED" << std::endl;
            }
            else
            {
                std::cout << "ERROR: Cannot open file" << std::endl;
            }
        }
        else if (command == "SAVE")
        {
            autoSave(manager);
            std::cout << "SUCCESS: SAVED" << std::endl;
        }
        else if (command == "SAVE_AS")
        {
            std::string path;
            std::getline(ss, path);
            path = trim(path);

            if (ProblemRepository::saveToFile(path, manager.getAllProblems()))
            {
                std::cout << "SUCCESS: SAVED_AS" << std::endl;
            }
            else
            {
                std::cout << "ERROR: Cannot save to file" << std::endl;
            }
        }
        else if (command == "IMPORT")
        {
            std::string path;
            std::getline(ss, path);
            path = trim(path);

            std::vector<Problem> tempProblems = manager.getAllProblems();
            ImportResult res = ProblemRepository::importFromFile(path, tempProblems);
            if (res.fileOpened)
            {
                manager.setProblems(tempProblems);
                autoSave(manager);
                std::cout << "SUCCESS: IMPORTED" << std::endl;
            }
            else
            {
                std::cout << "ERROR: Import failed" << std::endl;
            }
        }
        else if (command == "EXPORT")
        {
            std::string path;
            std::getline(ss, path);
            path = trim(path);

            if (ProblemRepository::exportToFile(path, manager.getAllProblems()))
            {
                std::cout << "SUCCESS: EXPORTED" << std::endl;
            }
            else
            {
                std::cout << "ERROR: Export failed" << std::endl;
            }
        }
        else
        {
            std::cout << "ERROR: UNKNOWN_COMMAND" << std::endl;
        }
    }

    return 0;
}