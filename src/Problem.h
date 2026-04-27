#pragma once
#include "Enums.h"
#include <string>
#include <vector>

class Problem {
private:
    std::string name, platform;
    Difficulty difficulty;
    std::vector<std::string> tags;
    Status status;
    int timeSpent;
    std::string date;
    double rating;
    std::string notes;

public:
    Problem();
    Problem(std::string name, std::string platform, Difficulty difficulty,
            std::vector<std::string> tags, Status status,
            int timeSpent, std::string date, double rating, std::string notes);

    std::string toCSV() const;
    void fromCSV(const std::string& line);

    void changeStatus(const Status& newStatus);
    void changeNotes(const std::string& newNotes);

    // --- Setters ---
    void setName(const std::string& newName);
    void setPlatform(const std::string& newPlatform);
    void setDifficulty(Difficulty newDifficulty);
    void setTags(const std::vector<std::string>& newTags);
    void setStatus(Status newStatus);
    void setTimeSpent(int newTimeSpent);
    void setDate(const std::string& newDate);
    void setRating(double newRating);
    void setNotes(const std::string& newNotes);

    // --- Getters ---
    const std::string& getName() const;
    const std::string& getPlatform() const;
    Difficulty getDifficulty() const;
    const std::vector<std::string>& getTags() const;
    Status getStatus() const;
    int getTimeSpent() const;
    const std::string& getDate() const;
    double getRating() const;
    const std::string& getNotes() const;
};