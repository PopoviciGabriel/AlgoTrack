#ifndef ALGOTRACK_PROBLEM_H
#define ALGOTRACK_PROBLEM_H

#include <string>
#include <vector>
#include "Enums.h"

class ProblemName
{
    std::string m_value;
    static std::string validate(const std::string& name);

public:
    ProblemName();
    explicit ProblemName(const std::string& name);
    const std::string& asString() const;
};

class Platform
{
    std::string m_value;
    static std::string validate(const std::string& platform);

public:
    Platform();
    explicit Platform(const std::string& platform);
    const std::string& asString() const;
};

class Tag
{
    std::string m_value;
    static std::string validate(const std::string& tag);

public:
    Tag();
    explicit Tag(const std::string& tag);
    const std::string& asString() const;
};

class TimeSpent
{
    int m_value;
    static int validate(int timeSpent);

public:
    TimeSpent();
    explicit TimeSpent(int timeSpent);
    int asInt() const;
};

class ProblemDate
{
    std::string m_value;
    static std::string validate(const std::string& date);

public:
    ProblemDate();
    explicit ProblemDate(const std::string& date);
    const std::string& asString() const;
};

class ProblemRating
{
    double m_value;
    static double validate(double rating);

public:
    ProblemRating();
    explicit ProblemRating(double rating);
    double asDouble() const;
};

class Problem
{
    ProblemName name;
    Platform platform;
    Difficulty difficulty;
    std::vector<Tag> tags;
    Status status;
    TimeSpent timeSpent;
    ProblemDate date;
    ProblemRating rating;
    std::string notes;
    std::vector<std::string> m_cachedTags;

    void setName(const std::string &newName);
    void setPlatform(const std::string &newPlatform);
    void setDifficulty(Difficulty newDifficulty);
    void setTags(const std::vector<std::string> &newTags);
    void setStatus(Status newStatus);
    void setTimeSpent(int newTimeSpent);
    void setDate(const std::string &newDate);
    void setRating(double newRating);
    void setNotes(const std::string &newNotes);

public:
    Problem();
    
    // Interfață compatibilă cu primitivele la granița dialogului/sistemului
    Problem(std::string name, std::string platform, Difficulty difficulty,
            std::vector<std::string> tags, Status status,
            int timeSpent, std::string date, double rating, std::string notes);

    // Constructor cu tipuri puternice pure
    Problem(ProblemName name, Platform platform, Difficulty difficulty,
            std::vector<Tag> tags, Status status,
            TimeSpent timeSpent, ProblemDate date, ProblemRating rating, std::string notes);

    void changeStatus(const Status &newStatus);
    void changeNotes(const std::string &newNotes);

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
};

#endif // ALGOTRACK_PROBLEM_H