#ifndef ALGOTRACK_PROBLEM_H
#define ALGOTRACK_PROBLEM_H

#include <string>
#include <vector>
#include "Enums.h"

class Problem
{
public:
    // Singurul mod de a crea o problemă goală dar VALIDĂ (starea inițială implicită)
    Problem();

    // Constructorul complet care garantează validitatea
    Problem(std::string name, std::string platform, Difficulty difficulty,
            std::vector<std::string> tags, Status status,
            int timeSpent, std::string date, double rating, std::string notes);

    // Business Logic dedicat (Mesaje, nu Settere directi)
    void changeStatus(const Status &newStatus);
    void changeNotes(const std::string &newNotes);

    // Getters (Rămân publici doar cei strict necesari pentru a exporta/afișa datele în UI)
    const std::string &getName() const;
    const std::string &getPlatform() const;
    Difficulty getDifficulty() const;
    const std::vector<std::string> &getTags() const;
    Status getStatus() const;
    int getTimeSpent() const;
    const std::string &getDate() const;
    double getRating() const;
    const std::string &getNotes() const;

    std::string toCSV() const;
    void fromCSV(const std::string &line);

private:
    std::string name;
    std::string platform;
    Difficulty difficulty;
    std::vector<std::string> tags;
    Status status;
    int timeSpent;
    std::string date;
    double rating;
    std::string notes;

    // Setterii devin detalii private de implementare (folosiți doar la construcție/parsare)
    void setName(const std::string &newName);
    void setPlatform(const std::string &newPlatform);
    void setDifficulty(Difficulty newDifficulty);
    void setTags(const std::vector<std::string> &newTags);
    void setStatus(Status newStatus);
    void setTimeSpent(int newTimeSpent);
    void setDate(const std::string &newDate);
    void setRating(double newRating);
    void setNotes(const std::string &newNotes);
};

#endif // ALGOTRACK_PROBLEM_H
