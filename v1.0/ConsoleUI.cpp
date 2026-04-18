#include "ConsoleUI.h"
#include "Problem.h"
#include "Statistics.h"
#include "Utils.h"
#include <iostream>
#include <limits>
#include <vector>
#include <string>
#include <iomanip>
#include <unordered_map>

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
        int howManyTags;
        std::cout << "How many tags: ";

        while (!(std::cin >> howManyTags) || howManyTags < 1) {
            std::cout << "Invalid entry. Re-enter how many tags you have: ";
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

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

    std::vector<std::string> tags = p.getTags();
    std::cout << "Tags        : ";
    for (size_t i = 0; i < (p.getTags()).size(); i++) {
        std::cout << tags[i] << (i + 1 < tags.size() ? ", " : "");
    }
    std::cout << "\n";

    std::cout << "Notes       : " << p.getNotes() << "\n";

    std::cout << "=============================\n";
}


void showStatisticsUI(const ProblemManager& manager) {
    const auto& problems = manager.getAllProblems();

    std::cout << "====== STATISTICS ======\n";

    // Total problems
    std::cout << "Total problems: " << getTotalProblems(problems) << "\n";
    std::cout << "\n";

    // Status

        std::cout << "Status:\n";
        std::vector<int> statuses = getStatuses(problems);
        std::cout << "Solved: " << statuses[0] << "\n";
        std::cout << "Failed: " << statuses[1] << "\n";
        std::cout << "In progress: " << statuses[2] << "\n";

    std::cout << "\n";

    // Difficulty

        std::cout << "Difficulty:\n";
        std::vector<int> difficulties = getDifficulties(problems);
        std::cout << "Easy: " << difficulties[0] << "\n";
        std::cout << "Medium: " << difficulties[1] << "\n";
        std::cout << "Hard: " << difficulties[2] << "\n";

    std::cout << "\n";

    // Total time spent
    std::cout << "Total time spent: " << totalTimeSpent(problems) << "\n";

    // Average time per problem
    std::cout  << "Average time per problem: " << averageTimeSpent(problems) << "\n";
    std::cout << "\n";

    // Platform usage

        std::cout << "Platform usage:\n";
        std::unordered_map<std::string, int> platformUsage = platformsUsage(problems);
        for (const auto& platform : platformUsage) {
            std::cout << platform.first << ": " << platform.second << "\n";
        }
        std::cout << "\n";


    // Most used tags
    const std::unordered_map<std::string, int> mostUsedTagsV = mostUsedTags(problems);
    std::cout << "Most used tags:\n";
    for (const auto& tag : mostUsedTagsV) {
        std::cout << tag.first << " -> " << tag.second << "\n";
    }
    std::cout << "\n";


    // Most used tag
    auto tag = mostUsedTag(mostUsedTagsV);
    if (tag.second == 0) {
        std::cout << "Most used tag: none\n";
    } else {
        std::cout << "Most used tag: " << tag.first << " - " << tag.second << "\n";
        std::cout << "\n";
    }

    // Average rating
    std::cout << "Average rating: " << averageRating(problems) << "\n";
    std::cout << "\n";

    // Longest problem time
    std::cout << "Longest problem time: " << longestProblem(problems) << "\n";
    std::cout << "\n";
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