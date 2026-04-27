#include "ConsoleUI.h"
#include "Problem.h"
#include "Statistics.h"
#include "Utils.h"
#include "ProblemRepository.h"
#include <iostream>
#include <limits>
#include <vector>
#include <string>
#include <iomanip>

Problem readProblemFromConsole() {
    Problem p;
    
    // --- Name ---
    
    while (true) {
        std::string inputName;
        std::cout << "Name: ";
        std::getline(std::cin >> std::ws, inputName);

        try {
            p.setName(inputName);
            break;
        } catch (const std::exception& e) {
            std::cout << e.what() << "\n";
        }
    }
    
    // --- Platform ---
    
    while (true) {
        std::string inputPlatform;
        std::cout << "Platform: ";
        std::getline(std::cin >> std::ws, inputPlatform);

        try {
            p.setPlatform(inputPlatform);
            break;
        } catch (const std::exception& e) {
            std::cout << e.what() << "\n";
        }
    }

    // --- Difficulty ---
    
    std::cout << "Difficulty (Easy / Medium / Hard): ";
    p.setDifficulty(readDifficulty());

    // --- Tags ---

    while (true) {
        int howManyTags = readInt("How many tags: ");

        while (howManyTags < 1) {
            std::cout << "You need at least one tag.\n";
            howManyTags = readInt("How many tags: ");
        }

        std::vector<std::string> inputTags;
        for (int i = 0; i < howManyTags; i++) {
            std::string tag;
            std::cout << "Tag " << i + 1 << ": ";
            std::getline(std::cin, tag);
            inputTags.push_back(toLowerCase(tag));
        }

        try {
            p.setTags(inputTags);
            break;
        } catch (const std::exception& e) {
            std::cout << e.what() << "\n";
        }
    }

    // --- Status ---
    
    std::cout << "Status (Solved / Failed / In Progress): ";
    p.setStatus(readStatus());

    // --- Time Spent ---
    
    while (true) {
        int time;
        std::cout << "Time spent: ";

        if (!(std::cin >> time)) {
            std::cout << "Invalid input. Re-enter time spent.\n";
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            continue;
        }

        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        try {
            p.setTimeSpent(time);
            break;
        } catch (const std::exception& e) {
            std::cout << e.what() << "\n";
        }
    }

    // --- Rating ---
    
    while (true) {
        double inputRating;
        std::cout << "Rating (1.00 - 10.00): ";

        if (!(std::cin >> inputRating)) {
            std::cout << "Invalid input. Re-enter rating.\n";
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            continue;
        }

        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        try {
            p.setRating(inputRating);
            break;
        } catch (const std::exception& e) {
            std::cout << e.what() << "\n";
        }
    }

    // --- Date ---

    while (true) {
        std::string inputDate;
        std::cout << "Date (DD-MM-YYYY): ";
        std::getline(std::cin >> std::ws, inputDate);

        try {
            p.setDate(inputDate);
            break;
        } catch (const std::exception& e) {
            std::cout << e.what() << "\n";
        }
    }

    // --- Notes ---
    std::string inputNotes;
    std::cout << "Notes: ";
    std::getline(std::cin >> std::ws, inputNotes);
    p.setNotes(inputNotes);
    
    return p;
}
void displayProblem(const Problem& p) {
    std::cout << p.getName() << "\n\n";

    std::cout << "Platform    : " << p.getPlatform() << "\n";
    std::cout << "Difficulty  : " << difficultyToString(p.getDifficulty()) << "\n";
    std::cout << "Status      : " << statusToString(p.getStatus()) << "\n";
    std::cout << "Time Spent  : " << p.getTimeSpent() << "\n";
    std::cout << "Date        : " << p.getDate() << "\n";
    std::cout << "Rating      : " << std::fixed << std::setprecision(2) << p.getRating() << "\n";

    const auto& tags = p.getTags();
    std::cout << "Tags        : ";
    for (size_t i = 0; i < tags.size(); i++) {
        std::cout << tags[i] << (i + 1 < tags.size() ? ", " : "");
    }
    std::cout << "\n";

    std::cout << "Notes       : " << p.getNotes() << "\n";

    std::cout << "=============================\n";
}


void showStatisticsUI(const ProblemManager& manager) {
    StatisticsSummary stats = calculateStatistics(manager.getAllProblems());

    std::cout << "====== STATISTICS ======\n\n";

    std::cout << "Total problems: " << stats.totalProblems << "\n\n";

    std::cout << "Status:\n";
    std::cout << "Solved: " << stats.solved << "\n";
    std::cout << "Failed: " << stats.failed << "\n";
    std::cout << "In progress: " << stats.inProgress << "\n\n";

    std::cout << "Difficulty:\n";
    std::cout << "Easy: " << stats.easy << "\n";
    std::cout << "Medium: " << stats.medium << "\n";
    std::cout << "Hard: " << stats.hard << "\n\n";

    std::cout << "Total time spent: " << stats.totalTime << "\n";
    std::cout << "Average time per problem: " << stats.averageTime << "\n\n";

    std::cout << "Platform usage:\n";
    for (const auto& platform : stats.platformUsage) {
        std::cout << platform.first << ": " << platform.second << "\n";
    }
    std::cout << "\n";

    std::cout << "Most used tags:\n";
    for (const auto& tag : stats.tagUsage) {
        std::cout << tag.first << " -> " << tag.second << "\n";
    }
    std::cout << "\n";

    if (stats.mostUsedTagCount == 0) {
        std::cout << "Most used tag: none\n";
    } else {
        std::cout << "Most used tag: "
                  << stats.mostUsedTag << " - "
                  << stats.mostUsedTagCount << "\n";
    }

    std::cout << "\n";

    std::cout << "Average rating: " << stats.averageRating << "\n";
    std::cout << "Longest problem time: " << stats.longestProblemTime << "\n\n";
}

Difficulty readDifficulty() {
    std::string s;
    while (true) {
        std::getline(std::cin, s);
        s = toLowerCase(s);

        try {
            return parseDifficulty(s);
        } catch (const std::exception&) {
            std::cout << "Invalid difficulty. Try again: ";
        }
    }
}

Status readStatus() {
    std::string s;
    
    while(true) {
        std::getline(std::cin, s);
        s = toLowerCase(s);

        try {
            return parseStatus(s);
        } catch (const std::exception&) {
            std::cout << "Invalid status. Try again: ";
        }
    }
}

int readInt(const std::string& prompt) {
    int value;

    while (true) {
        std::cout << prompt;

        if (std::cin >> value) {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            return value;
        }

        std::cout << "Invalid input. Try again.\n";
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
}

namespace {
    constexpr const char* DATA_FILE = "data/problems.csv";

    bool saveCurrentData(const ProblemManager& manager) {
        return ProblemRepository::saveToFile(DATA_FILE, manager.getAllProblems());
    }

    void showMenu() {
        std::cout << "\n========== PROBLEM MANAGER ==========\n";
        std::cout << "1. Add problem\n";
        std::cout << "2. Show all problems\n";
        std::cout << "3. Search by name\n";
        std::cout << "4. Delete by name\n";
        std::cout << "5. Filter\n";
        std::cout << "6. Update\n";
        std::cout << "7. Sort\n";
        std::cout << "8. Show statistics\n";
        std::cout << "9. Import/Export\n";
        std::cout << "0. Exit\n";
    }

    void handleFilterMenu(ProblemManager& manager) {
        std::cout << "   1. Filter by tag\n";
        std::cout << "   2. Filter by platform\n";
        std::cout << "   3. Filter by status\n";

        int choice = readInt("Choose option: ");

        switch (choice) {
            case 1: {
                std::string tag;
                std::cout << "Enter the tag: ";
                std::getline(std::cin >> std::ws, tag);

                std::vector<Problem> res = manager.filterByTag(tag);

                std::cout << "Problems with tag '" << tag << "':\n";

                if (res.empty()) {
                    std::cout << " - None -\n";
                } else {
                    for (const auto& p : res) {
                        displayProblem(p);
                    }
                }

                break;
            }

            case 2: {
                std::string platform;
                std::cout << "Enter the platform: ";
                std::getline(std::cin >> std::ws, platform);

                std::vector<Problem> res = manager.filterByPlatform(platform);

                std::cout << "Problems with platform '" << platform << "':\n";

                if (res.empty()) {
                    std::cout << " - None -\n";
                } else {
                    for (const auto& p : res) {
                        displayProblem(p);
                    }
                }

                break;
            }

            case 3: {
                std::cout << "Enter the status: ";
                Status status = readStatus();

                std::vector<Problem> res = manager.filterByStatus(status);

                std::cout << "Problems with status '" << statusToString(status) << "':\n";

                if (res.empty()) {
                    std::cout << " - None -\n";
                } else {
                    for (const auto& p : res) {
                        displayProblem(p);
                    }
                }

                break;
            }

            default:
                std::cout << " - Invalid option -\n";
        }
    }

    void handleUpdateStatus(ProblemManager& manager) {
        std::string name;

        std::cout << "Enter the name of problem: ";
        std::getline(std::cin >> std::ws, name);

        std::cout << "Enter the new status: ";
        Status newStatus = readStatus();

        SearchResult searchResult = manager.findByName(name);

        if (searchResult.exactFound) {
            if (manager.updateStatusAtIndex(searchResult.index, newStatus)) {
                std::cout << "--- The problem was successfully updated ---\n";

                if (!saveCurrentData(manager)) {
                    std::cout << "Error saving file.\n";
                }
            } else {
                std::cout << "--- Update failed ---\n";
            }
        } else if (searchResult.fuzzyFound) {
            std::cout << "Found closest match: " << searchResult.closestMatch << "\n";
            std::cout << "Update it? (y/n): ";

            char ans;
            std::cin >> ans;
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

            if (ans == 'y' || ans == 'Y') {
                if (manager.updateStatusAtIndex(searchResult.index, newStatus)) {
                    std::cout << "--- The problem was successfully updated ---\n";

                    if (!saveCurrentData(manager)) {
                        std::cout << "Error saving file.\n";
                    }
                } else {
                    std::cout << "--- Update failed ---\n";
                }
            } else {
                std::cout << "--- Problem update canceled ---\n";
            }
        } else {
            std::cout << "--- Problem not found ---\n";
        }
    }

    void handleUpdateNotes(ProblemManager& manager) {
        std::string name;
        std::string newNotes;

        std::cout << "Enter the name of problem: ";
        std::getline(std::cin >> std::ws, name);

        std::cout << "Enter the new notes: ";
        std::getline(std::cin >> std::ws, newNotes);

        SearchResult searchResult = manager.findByName(name);

        if (searchResult.exactFound) {
            if (manager.updateNotesAtIndex(searchResult.index, newNotes)) {
                std::cout << "--- The problem was successfully updated ---\n";

                if (!saveCurrentData(manager)) {
                    std::cout << "Error saving file.\n";
                }
            } else {
                std::cout << "--- Update failed ---\n";
            }
        } else if (searchResult.fuzzyFound) {
            std::cout << "Found closest match: " << searchResult.closestMatch << "\n";
            std::cout << "Update it? (y/n): ";

            char ans;
            std::cin >> ans;
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

            if (ans == 'y' || ans == 'Y') {
                if (manager.updateNotesAtIndex(searchResult.index, newNotes)) {
                    std::cout << "--- The problem was successfully updated ---\n";

                    if (!saveCurrentData(manager)) {
                        std::cout << "Error saving file.\n";
                    }
                } else {
                    std::cout << "--- Update failed ---\n";
                }
            } else {
                std::cout << "--- Problem update canceled ---\n";
            }
        } else {
            std::cout << "--- Problem not found ---\n";
        }
    }

    void handleUpdateMenu(ProblemManager& manager) {
        std::cout << "   1. Update status\n";
        std::cout << "   2. Update notes\n";

        int choice = readInt("Choose option: ");

        switch (choice) {
            case 1:
                handleUpdateStatus(manager);
                break;

            case 2:
                handleUpdateNotes(manager);
                break;

            default:
                std::cout << " - Invalid option -\n";
        }
    }

    void handleSortMenu(ProblemManager& manager) {
        std::cout << "   1. Sort by difficulty\n";
        std::cout << "   2. Sort by time\n";
        std::cout << "   3. Descending sort by rating\n";

        int choice = readInt("Choose option: ");

        bool sorted = false;

        switch (choice) {
            case 1:
                sorted = manager.sortByDifficulty();
                if (sorted) std::cout << "--- Sorted by difficulty ---\n";
                break;

            case 2:
                sorted = manager.sortByTime();
                if (sorted) std::cout << "--- Sorted by time ---\n";
                break;

            case 3:
                sorted = manager.descendingSortByRating();
                if (sorted) std::cout << "--- Sorted by rating ---\n";
                break;

            default:
                std::cout << " - Invalid option -\n";
                return;
        }

        if (!sorted) {
            std::cout << "--- Sorting failed ---\n";
            return;
        }

        if (!saveCurrentData(manager)) {
            std::cout << "Error saving file.\n";
        }
    }

    void handleImportExportMenu(ProblemManager& manager) {
        std::cout << "   1. Import from file\n";
        std::cout << "   2. Export to file\n";

        int choice = readInt("Choose option: ");

        switch (choice) {
            case 1: {
                std::string fileName;

                std::cout << "Enter the file name: ";
                std::getline(std::cin >> std::ws, fileName);

                std::vector<Problem> tempProblems = manager.getAllProblems();
                ImportResult result = ProblemRepository::importFromFile(fileName, tempProblems);

                if (!result.fileOpened) {
                    std::cout << "Error opening file!\n";
                } else {
                    manager.setProblems(tempProblems);

                    std::cout << "Import completed from " << fileName << "\n";
                    std::cout << "Imported: " << result.importedCount << "\n";
                    std::cout << "Duplicates skipped: " << result.duplicateCount << "\n";
                    std::cout << "Invalid lines skipped: " << result.invalidCount << "\n";

                    if (!saveCurrentData(manager)) {
                        std::cout << "Error saving file.\n";
                    }
                }

                break;
            }

            case 2: {
                std::string fileName;

                std::cout << "Enter the file name: ";
                std::getline(std::cin >> std::ws, fileName);

                if (ProblemRepository::exportToFile(fileName, manager.getAllProblems())) {
                    std::cout << "Export completed to " << fileName << "\n";
                } else {
                    std::cout << "Error opening file!\n";
                }

                break;
            }

            default:
                std::cout << " - Invalid option -\n";
        }
    }

    void handleSearchByName(const ProblemManager& manager) {
        std::string name;

        std::cout << "Name: ";
        std::getline(std::cin >> std::ws, name);

        SearchResult searchResult = manager.findByName(name);

        if (searchResult.exactFound) {
            const Problem* p = manager.getProblemAt(searchResult.index);

            if (p) {
                std::cout << "--- Problem found ---\n";
                displayProblem(*p);
            }
        } else if (searchResult.fuzzyFound) {
            std::cout << "Found closest match: " << searchResult.closestMatch << "\n";
            std::cout << "Show the problem? (y/n): ";

            char ans;
            std::cin >> ans;
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

            if (ans == 'y' || ans == 'Y') {
                const Problem* p = manager.getProblemAt(searchResult.index);

                if (p) {
                    std::cout << "--- Problem found ---\n";
                    displayProblem(*p);
                }
            } else {
                std::cout << "--- Problem display canceled! ---\n";
            }
        } else {
            std::cout << "Problem not found!\n";
        }
    }

    void handleDeleteByName(ProblemManager& manager) {
        std::string name;

        std::cout << "Problem name: ";
        std::getline(std::cin >> std::ws, name);

        SearchResult searchResult = manager.findByName(name);

        if (searchResult.exactFound) {
            if (manager.deleteAtIndex(searchResult.index)) {
                std::cout << "--- Problem successfully deleted ---\n";

                if (!saveCurrentData(manager)) {
                    std::cout << "Error saving file.\n";
                }
            } else {
                std::cout << "--- Invalid index ---\n";
            }
        } else if (searchResult.fuzzyFound) {
            std::cout << "Found closest match: " << searchResult.closestMatch << "\n";
            std::cout << "Delete the problem? (y/n): ";

            char ans;
            std::cin >> ans;
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

            if (ans == 'y' || ans == 'Y') {
                if (manager.deleteAtIndex(searchResult.index)) {
                    std::cout << "--- Problem successfully deleted ---\n";

                    if (!saveCurrentData(manager)) {
                        std::cout << "Error saving file.\n";
                    }
                } else {
                    std::cout << "--- Invalid index ---\n";
                }
            } else {
                std::cout << "--- Problem delete canceled ---\n";
            }
        } else {
            std::cout << "--- Problem not found ---\n";
        }
    }

    void handleShowAllProblems(const ProblemManager& manager) {
        const auto& all = manager.getAllProblems();

        if (all.empty()) {
            std::cout << "There are no problems.\n";
            return;
        }

        for (size_t i = 0; i < all.size(); i++) {
            std::cout << "--- Problem Nr " << i + 1 << " ---\n";
            displayProblem(all[i]);
        }
    }

    void loadInitialData(ProblemManager& manager) {
        LoadResult loadResult = ProblemRepository::loadFromFile(DATA_FILE);

        if (loadResult.fileOpened) {
            manager.setProblems(loadResult.problems);

            std::cout << "Loaded problems: " << loadResult.loadedCount << "\n";

            if (loadResult.duplicateCount > 0) {
                std::cout << "Duplicates skipped: " << loadResult.duplicateCount << "\n";
            }

            if (loadResult.invalidCount > 0) {
                std::cout << "Invalid lines skipped: " << loadResult.invalidCount << "\n";
            }
        } else {
            std::cout << "No existing data file found. Starting fresh.\n";
        }
    }
}

void runConsoleApp() {
    ProblemManager manager;
    loadInitialData(manager);

    while (true) {
        showMenu();

        int choice = readInt("Choose option: ");

        if (choice == 0) {
            std::cout << "Exit...\n";
            break;
        }

        switch (choice) {
            case 1: {
                Problem p = readProblemFromConsole();
                AddProblemResult result = manager.addProblem(p);

                if (result == AddProblemResult::Duplicate) {
                    std::cout << "This problem already exists in the collection.\n";
                } else {
                    std::cout << "Problem added successfully.\n";

                    if (!saveCurrentData(manager)) {
                        std::cout << "Error saving file.\n";
                    }
                }

                break;
            }

            case 2:
                handleShowAllProblems(manager);
                break;

            case 3:
                handleSearchByName(manager);
                break;

            case 4:
                handleDeleteByName(manager);
                break;

            case 5:
                handleFilterMenu(manager);
                break;

            case 6:
                handleUpdateMenu(manager);
                break;

            case 7:
                handleSortMenu(manager);
                break;

            case 8:
                showStatisticsUI(manager);
                break;

            case 9:
                handleImportExportMenu(manager);
                break;

            default:
                std::cout << " - Invalid option -\n";
        }
    }
}