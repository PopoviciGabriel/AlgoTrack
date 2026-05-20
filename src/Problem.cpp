#include "Problem.h"
#include "Utils.h"
#include <sstream>
#include <vector>
#include <string>
#include <stdexcept>

namespace
{
    // Funcții pure de validare ascunse în acest fișier de translație.
    // Ele primesc valorile brute și returnează valoarea curățată sau aruncă eroare.
    std::string validateName(std::string name)
    {
        std::string cleaned = trim(name);
        if (isBlank(cleaned))
            throw std::invalid_argument("Name cannot be empty.");
        return cleaned;
    }

    std::string validatePlatform(std::string platform)
    {
        std::string cleaned = toLowerCase(trim(platform));
        if (isBlank(cleaned))
            throw std::invalid_argument("Platform cannot be empty.");
        return cleaned;
    }

    std::vector<std::string> validateTags(const std::vector<std::string> &tags)
    {
        if (tags.empty())
            throw std::invalid_argument("At least one tag is required.");
        std::vector<std::string> cleanedTags;
        for (const auto &tag : tags)
        {
            std::string cleaned = toLowerCase(trim(tag));
            if (isBlank(cleaned))
                throw std::invalid_argument("Tag cannot be empty.");
            if (cleaned.find('|') != std::string::npos)
                throw std::invalid_argument("Tag cannot contain '|'.");
            cleanedTags.push_back(cleaned);
        }
        return cleanedTags;
    }

    int validateTimeSpent(int timeSpent)
    {
        if (timeSpent < 0)
            throw std::invalid_argument("Time spent cannot be negative.");
        return timeSpent;
    }

    std::string validateDate(std::string date)
    {
        if (isBlank(date))
            throw std::invalid_argument("Date cannot be empty.");
        if (!isValidDateFormat(date))
            throw std::invalid_argument("Date must be in format DD-MM-YYYY.");
        return date;
    }

    double validateRating(double rating)
    {
        if (rating < 1.0 || rating > 10.0)
            throw std::invalid_argument("Rating must be between 1.0 and 10.0.");
        return rating;
    }
}

Problem::Problem()
    : name(""),
      platform(""),
      difficulty(Difficulty::Easy),
      status(Status::InProgress),
      timeSpent(0),
      date(""),
      rating(1.0),
      notes("")
{
}

// Constructorul perfect RAII: Când ajungem la {}, obiectul este 100% valid.
Problem::Problem(std::string name, std::string platform, Difficulty difficulty,
                 std::vector<std::string> tags, Status status,
                 int timeSpent, std::string date, double rating, std::string notes)
    : name(validateName(name)),
      platform(validatePlatform(platform)),
      difficulty(difficulty),
      tags(validateTags(tags)),
      status(status),
      timeSpent(validateTimeSpent(timeSpent)),
      date(validateDate(date)),
      rating(validateRating(rating)),
      notes(notes)
{
}

void Problem::changeStatus(const Status &newStatus)
{
    status = newStatus;
}

void Problem::changeNotes(const std::string &newNotes)
{
    notes = newNotes;
}

const std::string &Problem::getName() const { return name; }
const std::string &Problem::getPlatform() const { return platform; }
Difficulty Problem::getDifficulty() const { return difficulty; }
const std::vector<std::string> &Problem::getTags() const { return tags; }
Status Problem::getStatus() const { return status; }
int Problem::getTimeSpent() const { return timeSpent; }
const std::string &Problem::getDate() const { return date; }
double Problem::getRating() const { return rating; }
const std::string &Problem::getNotes() const { return notes; }

std::string Problem::toCSV() const
{
    std::string tagsString;
    for (int i = 0; i < (int)tags.size(); i++)
    {
        tagsString += tags[i];
        if (i + 1 < (int)tags.size())
            tagsString += "|";
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

void Problem::fromCSV(const std::string &line)
{
    std::vector<std::string> fields = parseCSVLine(line);
    if (fields.size() != 9)
        throw std::runtime_error("Invalid CSV line.");

    // Extragem, validăm și suprascriem invarianții
    std::string parsedName = validateName(fields[0]);
    std::string parsedPlatform = validatePlatform(fields[1]);
    Difficulty parsedDifficulty = parseDifficulty(fields[2]);
    Status parsedStatus = parseStatus(fields[4]);

    std::vector<std::string> rawTags;
    std::stringstream ts(fields[3]);
    std::string temp;
    while (std::getline(ts, temp, '|'))
        rawTags.push_back(temp);
    std::vector<std::string> parsedTags = validateTags(rawTags);

    int parsedTimeSpent;
    double parsedRating;
    try
    {
        parsedTimeSpent = std::stoi(fields[5]);
        parsedRating = std::stod(fields[7]);
    }
    catch (...)
    {
        throw std::runtime_error("Invalid numeric value in CSV.");
    }

    parsedTimeSpent = validateTimeSpent(parsedTimeSpent);
    parsedRating = validateRating(parsedRating);
    std::string parsedDate = validateDate(fields[6]);

    // Odată ce totul e valid, asamblăm memoria
    name = parsedName;
    platform = parsedPlatform;
    difficulty = parsedDifficulty;
    tags = parsedTags;
    status = parsedStatus;
    timeSpent = parsedTimeSpent;
    date = parsedDate;
    rating = parsedRating;
    notes = fields[8];
}