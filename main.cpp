#include <iostream>
#include <limits>
#include "ProblemManager.h"
#include "Utils.h"
#include "ConsoleUI.h"
#include "ProblemRepository.h"

void showMenu();
void handleFilterMenu(ProblemManager& manager);
void handleUpdateMenu(ProblemManager& manager);
void handleSortMenu(ProblemManager& manager);
void handleImportExportMenu(ProblemManager& manager);

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
    std::cout << "Choose option: ";
}

void handleFilterMenu(ProblemManager& manager) {
    std::cout << "   1. Filter by tag\n";
    std::cout << "   2. Filter by platform\n";
    std::cout << "   3. Filter by status\n";
    std::cout << "   Choose option: ";

    int choice;
    std::cin >> choice;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    switch (choice) {
        case 1: {
            std::string tag;
            std::cout << "Enter the tag: ";
            std::getline(std::cin >> std::ws, tag);
            std::vector<std::string> res = manager.filterByTag(tag);

            std::cout << "Problems with tag '" << tag << "':\n";

            if (res.empty()) {
                std::cout << " - None -\n";
            } else {
                for (const auto& name : res) {
                    std::cout << " - " << name << "\n";
                }
            }

            break;
        }
        case 2: {
            std::string platform;
            std::cout << "Enter the platform: ";
            std::getline(std::cin >> std::ws, platform);
            std::vector<std::string> res = manager.filterByPlatform(platform);
            
            std::cout << "Problems with platform '" << platform << "':\n";

            if (res.empty()) {
                std::cout << " - None -\n";
            } else {
                for (const auto& name : res) {
                    std::cout << " - " << name << "\n";
                }
            }

            break;
        }
        case 3: {
            Status status;
            std::cout << "Enter the status: ";
            status = readStatus();
            std::vector<std::string> res = manager.filterByStatus(status);
        
            std::cout << "Problems with status '" << statusToString(status) << "':\n";

            if (res.empty()) {
                std::cout << " - None -\n";
            } else {
                for (const auto& name : res) {
                    std::cout << " - " << name << "\n";
                }
            }

            break;
        }
        default:
            std::cout << " - Invalid option -\n";
    }
}

void handleUpdateMenu(ProblemManager& manager) {
    std::cout << "   1. Update status\n";
    std::cout << "   2. Update notes\n";
    std::cout << "Choose option: ";

    int choice;
    std::cin >> choice;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    switch (choice) {
        case 1: {
            std::string name;
            Status newStatus;
            std::cout << "Enter the name of problem: ";
            std::getline(std::cin, name);
            std::cout << "Enter the new status: ";
            newStatus = readStatus();
            
            SearchResult searchResult = manager.findByName(name);
                if (searchResult.exactFound) {
                    if (manager.updateStatusAtIndex(searchResult.index, newStatus)) {
                    std::cout << "--- The problem was successfully updated ---\n";
                    ProblemRepository::saveToFile("problems.csv", manager.getAllProblems());
                } else {
                    std::cout << "--- Update failed ---\n";
                }
                }
                else if (searchResult.fuzzyFound) {
                    std::cout << "Found closest match: " << searchResult.closestMatch << "\n";
                    std::cout << "Update it? (y/n): ";

                    char ans;
                    std::cin >> ans;
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

                    if (ans == 'y' || ans == 'Y') {
                        if (manager.updateStatusAtIndex(searchResult.index, newStatus)) {
                        std::cout << "--- The problem was successfully updated ---\n";
                        ProblemRepository::saveToFile("problems.csv", manager.getAllProblems());
                    } else {
                        std::cout << "--- Update failed ---\n";
                    }
                    } else {
                        std::cout << "--- Problem update canceled ---\n";
                    }
                }
                else {
                    std::cout << "--- Problem not found ---\n";
                }
            
            break;
        }
        case 2: {
            std::string name, newNotes;
            std::cout << "Enter the name of problem: ";
            std::getline(std::cin, name);
            std::cout << "Enter the new notes: ";
            std::getline(std::cin >> std::ws, newNotes);
            
            SearchResult searchResult = manager.findByName(name);
                if (searchResult.exactFound) {
                    if (manager.updateNotesAtIndex(searchResult.index, newNotes)) {
                        std::cout << "--- The problem was successfully updated ---\n";
                        ProblemRepository::saveToFile("problems.csv", manager.getAllProblems());
                } else {
                    std::cout << "--- Update failed ---\n";
                }
                }
                else if (searchResult.fuzzyFound) {
                    std::cout << "Found closest match: " << searchResult.closestMatch << "\n";
                    std::cout << "Update it? (y/n): ";

                    char ans;
                    std::cin >> ans;
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

                    if (ans == 'y' || ans == 'Y') {
                        if (manager.updateNotesAtIndex(searchResult.index, newNotes)) {
                        std::cout << "--- The problem was successfully updated ---\n";
                        ProblemRepository::saveToFile("problems.csv", manager.getAllProblems());
                    } else {
                        std::cout << "--- Update failed ---\n";
                    }
                    } else {
                        std::cout << "--- Problem update canceled ---\n";
                    }
                }
                else {
                    std::cout << "--- Problem not found ---\n";
                }

            break;
        }
        default:
            std::cout << " - Invalid option -\n";
    }
}

void handleSortMenu(ProblemManager& manager) {
    std::cout << "   1. Sort by difficulty\n";
    std::cout << "   2. Sort by time\n";
    std::cout << "   3. Descending sort by rating\n";
    std::cout << "Choose option: ";

    int choice;
    std::cin >> choice;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    switch (choice) {
        case 1:
            if(manager.sortByDifficulty()) {
                std::cout << "--- Sorted by difficulty ---\n";
                ProblemRepository::saveToFile("problems.csv", manager.getAllProblems());
            } else {
                std::cout << "--- Sorting failed ---\n";
            }
            break;
        case 2:
            if(manager.sortByTime()) {
                std::cout << "--- Sorted by time ---\n";
                ProblemRepository::saveToFile("problems.csv", manager.getAllProblems());
            } else {
                std::cout << "--- Sorting failed ---\n";
            }
            break;
        case 3:
            if(manager.descendingSortByRating()) {
                std::cout << "--- Sorted by rating ---\n";
                ProblemRepository::saveToFile("problems.csv", manager.getAllProblems());
            } else {
                std::cout << "--- Sorting failed ---\n";
            }
            break;
        default:
            std::cout << " - Invalid option -\n";
    }
}

void handleImportExportMenu(ProblemManager& manager) {
    std::cout << "   1. Import from file\n";
    std::cout << "   2. Export to file\n";
    std::cout << "Choose option: ";

    int choice;
    std::cin >> choice;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    switch (choice) {
        case 1: {
            std::string fileName;
            std::cout << "Enter the file name: ";
            std::getline(std::cin, fileName);

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

                if (!ProblemRepository::saveToFile("problems.csv", manager.getAllProblems())) {
                    std::cout << "Error saving problems.csv!\n";
                }
            }

            break;
        }

        case 2: {
            std::string fileName;
            std::cout << "Enter the file name: ";
            std::getline(std::cin, fileName);

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

int main() {
    ProblemManager manager;
    manager.setProblems(ProblemRepository::loadFromFile("problems.csv"));

    int choice;

    while (true) {
        showMenu();
        std::cin >> choice;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        if (choice == 0) {
            std::cout << "Exit...\n";
            break;
        }

        switch (choice) {
            case 1:
                {
                    Problem p = readProblemFromConsole();
                    AddProblemResult result = manager.addProblem(p);

                    if (result == AddProblemResult::Duplicate) {
                        std::cout << "This problem already exists in the collection.\n";
                    } else {
                        std::cout << "Problem added successfully.\n";
                        ProblemRepository::saveToFile("problems.csv", manager.getAllProblems());
                    }
                    break;
                }
            case 2:
                {
                    const auto& all = manager.getAllProblems();

                    if (all.empty()) {
                        std::cout << "There are no problems.\n";
                    } else {
                        for (size_t i = 0; i < all.size(); i++) {
                            std::cout << "--- Problem Nr " << i + 1 << " ---\n";
                            displayProblem(all[i]);
                        }
                    }
                    break;
                }
            case 3: {
                std::string name;
                std::cout << "Name: ";
                std::getline(std::cin, name);
                SearchResult searchResult = manager.findByName(name);
                if (searchResult.exactFound) {
                    const Problem* p = manager.getProblemAt(searchResult.index);
                    if (p) {
                        std::cout << "--- Problem found ---\n";
                        displayProblem(*p);
                    }
                }
                else if (searchResult.fuzzyFound) {
                    std::cout << "Found closest match: " << searchResult.closestMatch << "\n";
                    std::cout << "Show the problem? (y/n): ";

                    char ans;
                    std::cin >> ans;
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

                    if (ans == 'y' || ans == 'Y') {
                        const Problem* p = manager.getProblemAt(searchResult.index);

                        if(p) {
                            std::cout << "--- Problem found ---\n";
                            displayProblem(*p);
                        }
                    } else {
                        std::cout << "--- Problem display canceled! ---\n";
                    }
                }
                else {
                    std::cout << "Problem not found!\n";
                }

                break;
            }
            case 4: {
                std::string name;
                std::cout << "Problem name: ";
                std::getline(std::cin, name);
                SearchResult searchResult = manager.findByName(name);
                if (searchResult.exactFound) {
                    manager.deleteAtIndex(searchResult.index);
                    std::cout << "--- Problem successfully deleted ---\n";
                    ProblemRepository::saveToFile("problems.csv", manager.getAllProblems());
                }
                else if (searchResult.fuzzyFound) {
                    std::cout << "Found closest match: " << searchResult.closestMatch << "\n";
                    std::cout << "Delete the problem? (y/n): ";

                    char ans;
                    std::cin >> ans;
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

                    if (ans == 'y' || ans == 'Y') {
                        manager.deleteAtIndex(searchResult.index);
                        std::cout << "--- Problem successfully deleted ---\n";
                        ProblemRepository::saveToFile("problems.csv", manager.getAllProblems());
                    } else {
                        std::cout << "--- Problem delete canceled ---\n";
                    }
                }
                else {
                    std::cout << "--- Problem not found ---\n";
                }

                break;
            }
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

    return 0;
}