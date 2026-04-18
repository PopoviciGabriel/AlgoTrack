#include "Problem.h"
#include "Utils.h"
#include <iostream>
#include <limits>
#include <sstream>
#include <vector>
#include <string>
#include <iomanip>

Problem::Problem() {
    name = "";
    platform = "";
    difficulty = Difficulty::Easy;
    status = Status::InProgress;
    timeSpent = 0;
    date = "";
    rating = 1.0;
    notes = "";
}

Problem::Problem(std::string name, std::string platform, Difficulty difficulty,
                 std::vector<std::string> tags, Status status,
                 int timeSpent, std::string date, double rating, std::string notes) {
    setName(name);
    setPlatform(platform);
    setDifficulty(difficulty);
    setTags(tags);
    setStatus(status);
    setTimeSpent(timeSpent);
    setDate(date);
    setRating(rating);
    setNotes(notes);
}

void Problem::changeStatus(const Status& newStatus) {
    setStatus(newStatus);
}

void Problem::changeNotes(const std::string& newNotes) {
    setNotes(newNotes);
}


// ---------------
// --- Setters ---
// ---------------

void Problem::setName(const std::string& newName) {
    std::string cleaned = trim(newName);

    if (isBlank(cleaned)) {
        throw std::invalid_argument("Name cannot be empty.");
    }

    name = cleaned;
}

void Problem::setPlatform(const std::string& newPlatform) {
    std::string cleaned = toLowerCase(trim(newPlatform));

    if (isBlank(cleaned)) {
        throw std::invalid_argument("Platform cannot be empty.");
    }

    platform = cleaned;
}

void Problem::setDifficulty(Difficulty newDifficulty) {
    difficulty = newDifficulty;
}

void Problem::setTags(const std::vector<std::string>& newTags) {
    if (newTags.empty()) {
        throw std::invalid_argument("At least one tag is required.");
    }

    std::vector<std::string> cleanedTags;

    for (const auto& tag : newTags) {
        std::string cleaned = toLowerCase(trim(tag));

        if (isBlank(cleaned)) {
            throw std::invalid_argument("Tag cannot be empty.");
        }

        if (cleaned.find('|') != std::string::npos) {
            throw std::invalid_argument("Tag cannot contain '|'.");
        }

        cleanedTags.push_back(cleaned);
    }

    tags = cleanedTags;
}

void Problem::setStatus(Status newStatus) {
    status = newStatus;
}

void Problem::setTimeSpent(int newTimeSpent) {
    if (newTimeSpent < 0) {
        throw std::invalid_argument("Time spent cannot be negative.");
    }
    timeSpent = newTimeSpent;
}

void Problem::setDate(const std::string& newDate) {
    if(isBlank(newDate)) {
        throw std::invalid_argument("Date cannot be empty.");
    }
    else if(!isValidDateFormat(newDate)) {
        throw std::invalid_argument("Date must be in format DD-MM-YYYY.");
    }
    date = newDate;
}

void Problem::setRating(double newRating) {
    if (newRating < 1.0 || newRating > 10.0) {
        throw std::invalid_argument("Rating must be between 1.0 and 10.0.");
    }
    rating = newRating;
}

void Problem::setNotes(const std::string& newNotes) {
    notes = newNotes;
}

// ---------------
// --- Getters ---
// ---------------

const std::string& Problem::getName() const {
    return name;
}

const std::string& Problem::getPlatform() const {
    return platform;
}

Difficulty Problem::getDifficulty() const {
    return difficulty;
}

const std::vector<std::string>& Problem::getTags() const {
    return tags;
}

Status Problem::getStatus() const {
    return status;
}

int Problem::getTimeSpent() const {
    return timeSpent;
}

const std::string& Problem::getDate() const {
    return date;
}

double Problem::getRating() const {
    return rating;
}

const std::string& Problem::getNotes() const {
    return notes;
}

std::string Problem::toCSV() const {
    std::string tagsString;
    for (int i = 0; i < (int)tags.size(); i++) {
        tagsString += tags[i];
        if (i + 1 < (int)tags.size()) {
            tagsString += "|";
        }
    }

    return escapeCSVField(name) + "," +
           escapeCSVField(platform) + "," +
           escapeCSVField(difficultyToCSV(difficulty)) + "," +
           escapeCSVField(tagsString) + "," +
           escapeCSVField(statusToCSV(status)) + "," +
           escapeCSVField(std::to_string(timeSpent)) + "," +
           escapeCSVField(date) + "," +
           escapeCSVField(std::to_string(rating)) + "," +
           escapeCSVField(notes);
}

void Problem::fromCSV(const std::string& line) {
    std::vector<std::string> fields = parseCSVLine(line);

    if (fields.size() != 9) {
        throw std::runtime_error("Invalid CSV line.");
    }

    std::string parsedName = fields[0];
    std::string parsedPlatform = fields[1];
    std::string tagsString = fields[3];
    std::string timeStr = fields[5];
    std::string parsedDate = fields[6];
    std::string ratingStr = fields[7];
    std::string parsedNotes = fields[8];

    Difficulty parsedDifficulty = parseDifficulty(fields[2]);
    Status parsedStatus = parseStatus(fields[4]);

    // Numbers
    int parsedTimeSpent;
    double parsedRating;

    try {
        parsedTimeSpent = std::stoi(timeStr);
        parsedRating = std::stod(ratingStr);
    } catch (...) {
        throw std::runtime_error("Invalid numeric value in CSV.");
    }

    // Tags
    std::vector<std::string> parsedTags;
    std::stringstream ts(tagsString);
    std::string temp;

    while (std::getline(ts, temp, '|')) {
        parsedTags.push_back(temp);
    }

    // Aplicăm setters (IMPORTANT)
    setName(parsedName);
    setPlatform(parsedPlatform);
    setDifficulty(parsedDifficulty);
    setTags(parsedTags);
    setStatus(parsedStatus);
    setTimeSpent(parsedTimeSpent);
    setDate(parsedDate);
    setRating(parsedRating);
    setNotes(parsedNotes);
}