#include "Problem.h"
#include "Fsrs.h"
#include "Utils.h"
#include <atomic>
#include <chrono>
#include <random>
#include <sstream>
#include <vector>
#include <string>
#include <stdexcept>

// =========================================================
// IMPLEMENTARE TIPURI PUTERNICE (STRONG TYPES INVARIANTS)
// =========================================================

std::string ProblemName::validate(const std::string &name)
{
    std::string cleaned = trim(name);
    if (isBlank(cleaned))
        throw std::invalid_argument("Name cannot be empty.");
    return cleaned;
}
ProblemName::ProblemName() : m_value("") {}
ProblemName::ProblemName(const std::string &name) : m_value(validate(name)) {}
const std::string &ProblemName::asString() const { return m_value; }

std::string Platform::validate(const std::string &platform)
{
    std::string cleaned = toLowerCase(trim(platform));
    if (isBlank(cleaned))
        throw std::invalid_argument("Platform cannot be empty.");
    return cleaned;
}
Platform::Platform() : m_value("") {}
Platform::Platform(const std::string &platform) : m_value(validate(platform)) {}
const std::string &Platform::asString() const { return m_value; }

std::string Tag::validate(const std::string &tag)
{
    std::string cleaned = toLowerCase(trim(tag));
    if (isBlank(cleaned))
        throw std::invalid_argument("Tag cannot be empty.");
    if (cleaned.find('|') != std::string::npos)
        throw std::invalid_argument("Tag cannot contain '|'.");
    return cleaned;
}
Tag::Tag() : m_value("") {}
Tag::Tag(const std::string &tag) : m_value(validate(tag)) {}
const std::string &Tag::asString() const { return m_value; }

int TimeSpent::validate(int timeSpent)
{
    if (timeSpent < 0)
        throw std::invalid_argument("Time spent cannot be negative.");
    return timeSpent;
}
TimeSpent::TimeSpent() : m_value(0) {}
TimeSpent::TimeSpent(int timeSpent) : m_value(validate(timeSpent)) {}
int TimeSpent::asInt() const { return m_value; }

std::string ProblemDate::validate(const std::string &date)
{
    if (isBlank(date))
        throw std::invalid_argument("Date cannot be empty.");
    if (!isValidDateFormat(date))
        throw std::invalid_argument("Date must be in format DD-MM-YYYY.");
    return date;
}
ProblemDate::ProblemDate() : m_value("") {}
ProblemDate::ProblemDate(const std::string &date) : m_value(validate(date)) {}
const std::string &ProblemDate::asString() const { return m_value; }

double ProblemRating::validate(double rating)
{
    if (rating < 1.0 || rating > 10.0)
        throw std::invalid_argument("Rating must be between 1.0 and 10.0.");
    return rating;
}
ProblemRating::ProblemRating() : m_value(1.0) {}
ProblemRating::ProblemRating(double rating) : m_value(validate(rating)) {}
double ProblemRating::asDouble() const { return m_value; }

namespace
{
std::string generateProblemId()
{
    static std::atomic<uint64_t> sequence{1};
    const auto ticks = static_cast<uint64_t>(
        std::chrono::steady_clock::now().time_since_epoch().count());
    std::random_device device;
    uint64_t id = (ticks << 16) ^ (sequence.fetch_add(1, std::memory_order_relaxed) << 48) ^
                  (static_cast<uint64_t>(device()) << 32);
    if (id == 0)
        id = 1;
    return std::to_string(id);
}
}

void Problem::assignTracking(std::string newId, std::string url, std::string path,
                             std::string due, int interval, double stab, int reps)
{
    problemUrl = trim(url);
    solutionPath = trim(path);
    id = trim(newId);
    if (id.empty())
        id = generateProblemId();
    if (interval < 0)
        throw std::invalid_argument("Interval cannot be negative.");
    if (!(stab > 0.0))
        throw std::invalid_argument("Stability must be positive.");
    if (reps < 0)
        throw std::invalid_argument("Repetition count cannot be negative.");
    intervalDays = interval;
    stability = stab;
    repetitionCount = reps;
    due = trim(due);
    if (due.empty())
        dueDate = todayIsoDate();
    else if (!isValidIsoDate(due))
        throw std::invalid_argument("Due date must be YYYY-MM-DD.");
    else
        dueDate = due;
}

// =========================================================
// IMPLEMENTARE CLASĂ DOMENIU (PROBLEM)
// =========================================================

Problem::Problem()
    : name(),
      platform(),
      difficulty(Difficulty::Easy),
      status(Status::InProgress),
      timeSpent(),
      date(),
      rating(),
      notes(""),
      intervalDays(0),
      stability(1.0),
      repetitionCount(0)
{
    assignTracking("", "", "", "", 0, 1.0, 0);
}

Problem::Problem(std::string name, std::string platform, Difficulty difficulty,
                 std::vector<std::string> tags, Status status,
                 int timeSpent, std::string date, double rating, std::string notes,
                 std::string problemUrl, std::string solutionPath,
                 std::string id, std::string dueDate,
                 int intervalDays, double stability, int repetitionCount,
                 std::string timeComplexity, std::string spaceComplexity)
    : name(name),
      platform(platform),
      difficulty(difficulty),
      status(status),
      timeSpent(timeSpent),
      date(date),
      rating(rating),
      notes(notes),
      timeComplexity(trim(timeComplexity)),
      spaceComplexity(trim(spaceComplexity))
{
    if (tags.empty())
        throw std::invalid_argument("At least one tag is required.");

    for (const auto &rawTag : tags)
    {
        this->tags.push_back(Tag(rawTag));
    }
    m_cachedTags = std::move(tags);
    assignTracking(std::move(id), std::move(problemUrl), std::move(solutionPath),
                   std::move(dueDate), intervalDays, stability, repetitionCount);
}

Problem::Problem(ProblemName name, Platform platform, Difficulty difficulty,
                 std::vector<Tag> tags, Status status,
                 TimeSpent timeSpent, ProblemDate date, ProblemRating rating, std::string notes)
    : name(std::move(name)),
      platform(std::move(platform)),
      difficulty(difficulty),
      tags(std::move(tags)),
      status(status),
      timeSpent(timeSpent),
      date(std::move(date)),
      rating(rating),
      notes(std::move(notes))
{
    if (this->tags.empty())
        throw std::invalid_argument("At least one tag is required.");

    m_cachedTags.clear();
    for (const auto &t : this->tags)
    {
        m_cachedTags.push_back(t.asString());
    }
    assignTracking("", "", "", "", 0, 1.0, 0);
}

void Problem::changeStatus(const Status &newStatus)
{
    status = newStatus;
}

void Problem::changeNotes(const std::string &newNotes)
{
    notes = newNotes;
}

void Problem::adoptId(const std::string &stableId)
{
    const std::string cleaned = trim(stableId);
    if (cleaned.empty())
        throw std::invalid_argument("Id cannot be empty.");
    id = cleaned;
}

void Problem::applyReview(ReviewRating rating, const std::string &todayIso)
{
    const ReviewState next = scheduleReview(
        ReviewState{stability, intervalDays, repetitionCount, dueDate}, rating, todayIso);
    stability = next.stability;
    intervalDays = next.intervalDays;
    repetitionCount = next.repetitionCount;
    dueDate = next.dueDate;
}

void Problem::applySpacedReview(const std::string &todayIso)
{
    if (!isValidIsoDate(todayIso))
        throw std::invalid_argument("Review date must be YYYY-MM-DD.");

    int days = 14;
    if (difficulty == Difficulty::Hard)
        days = 3;
    else if (difficulty == Difficulty::Medium)
        days = 7;

    intervalDays = days;
    dueDate = addDaysToIsoDate(todayIso, days);
    if (repetitionCount < 0)
        repetitionCount = 0;
    ++repetitionCount;
}

const std::string &Problem::getName() const { return name.asString(); }
const std::string &Problem::getPlatform() const { return platform.asString(); }
Difficulty Problem::getDifficulty() const { return difficulty; }
const std::vector<std::string> &Problem::getTags() const { return m_cachedTags; }
Status Problem::getStatus() const { return status; }
int Problem::getTimeSpent() const { return timeSpent.asInt(); }
const std::string &Problem::getDate() const { return date.asString(); }
double Problem::getRating() const { return rating.asDouble(); }
const std::string &Problem::getNotes() const { return notes; }
const std::string &Problem::getId() const { return id; }
const std::string &Problem::getProblemUrl() const { return problemUrl; }
const std::string &Problem::getSolutionPath() const { return solutionPath; }
const std::string &Problem::getTimeComplexity() const { return timeComplexity; }
const std::string &Problem::getSpaceComplexity() const { return spaceComplexity; }
const std::string &Problem::getDueDate() const { return dueDate; }
int Problem::getIntervalDays() const { return intervalDays; }
double Problem::getStability() const { return stability; }
int Problem::getRepetitionCount() const { return repetitionCount; }

std::string Problem::toCSV() const
{
    std::string tagsString;
    for (size_t i = 0; i < tags.size(); i++)
    {
        tagsString += tags[i].asString();
        if (i + 1 < tags.size())
            tagsString += "|";
    }

    return escapeCSVField(name.asString()) + "," +
           escapeCSVField(platform.asString()) + "," +
           escapeCSVField(difficultyToCSV(difficulty)) + "," +
           escapeCSVField(tagsString) + "," +
           escapeCSVField(statusToCSV(status)) + "," +
           escapeCSVField(std::to_string(timeSpent.asInt())) + "," +
           escapeCSVField(date.asString()) + "," +
           escapeCSVField(formatCsvDouble(rating.asDouble(), 1)) + "," +
           escapeCSVField(notes) + "," +
           escapeCSVField(id) + "," +
           escapeCSVField(problemUrl) + "," +
           escapeCSVField(solutionPath) + "," +
           escapeCSVField(dueDate) + "," +
           escapeCSVField(std::to_string(intervalDays)) + "," +
           escapeCSVField(formatCsvDouble(stability, 6)) + "," +
           escapeCSVField(std::to_string(repetitionCount)) + "," +
           escapeCSVField(timeComplexity) + "," +
           escapeCSVField(spaceComplexity);
}

void Problem::fromCSV(const std::string &line)
{
    std::string normalized = line;
    if (!normalized.empty() && normalized.back() == '\r')
        normalized.pop_back();
    std::vector<std::string> fields = parseCSVLine(normalized);
    // 9: formatul inițial. 16: tracking FSRS. 17+: complexitate opțională (și câmpuri viitoare ignorate).
    const bool legacy = fields.size() == 9;
    if (!legacy && fields.size() < 16)
        throw std::runtime_error("Invalid CSV line.");

    ProblemName parsedName(fields[0]);
    Platform parsedPlatform(fields[1]);
    Difficulty parsedDifficulty = parseDifficulty(fields[2]);
    Status parsedStatus = parseStatus(fields[4]);

    std::vector<std::string> rawTags;
    std::stringstream ts(fields[3]);
    std::string temp;
    while (std::getline(ts, temp, '|'))
        rawTags.push_back(temp);

    if (rawTags.empty())
        throw std::invalid_argument("At least one tag is required.");

    std::vector<Tag> parsedTags;
    std::vector<std::string> cachedStrings;
    for (const auto &rt : rawTags)
    {
        Tag t(rt);
        parsedTags.push_back(t);
        cachedStrings.push_back(t.asString());
    }

    int rawTimeSpent;
    double rawRating;
    try
    {
        rawTimeSpent = std::stoi(fields[5]);
        rawRating = parseCsvDouble(fields[7]);
    }
    catch (...)
    {
        throw std::runtime_error("Invalid numeric value in CSV.");
    }

    TimeSpent parsedTimeSpent(rawTimeSpent);
    ProblemRating parsedRating(rawRating);
    ProblemDate parsedDate(fields[6]);

    name = std::move(parsedName);
    platform = std::move(parsedPlatform);
    difficulty = parsedDifficulty;
    tags = std::move(parsedTags);
    m_cachedTags = std::move(cachedStrings);
    status = parsedStatus;
    timeSpent = parsedTimeSpent;
    date = std::move(parsedDate);
    rating = parsedRating;
    notes = fields[8];

    if (legacy)
    {
        timeComplexity.clear();
        spaceComplexity.clear();
        assignTracking("", "", "", todayIsoDate(), 0, 1.0, 0);
        return;
    }

    int parsedInterval = 0;
    double parsedStability = 1.0;
    int parsedReps = 0;
    try
    {
        parsedInterval = std::stoi(fields[13]);
        parsedStability = parseCsvDouble(fields[14]);
        parsedReps = std::stoi(fields[15]);
    }
    catch (...)
    {
        throw std::runtime_error("Invalid schedule value in CSV.");
    }
    assignTracking(fields[9], fields[10], fields[11], fields[12],
                   parsedInterval, parsedStability, parsedReps);
    timeComplexity.clear();
    spaceComplexity.clear();
    if (fields.size() > 16)
        timeComplexity = trim(fields[16]);
    if (fields.size() > 17)
        spaceComplexity = trim(fields[17]);
}

void Problem::setName(const std::string &newName) { name = ProblemName(newName); }
void Problem::setPlatform(const std::string &newPlatform) { platform = Platform(newPlatform); }
void Problem::setDifficulty(Difficulty newDifficulty) { difficulty = newDifficulty; }
void Problem::setTags(const std::vector<std::string> &newTags)
{
    if (newTags.empty())
        throw std::invalid_argument("At least one tag is required.");
    std::vector<Tag> tempTags;
    for (const auto &nt : newTags)
        tempTags.push_back(Tag(nt));
    tags = std::move(tempTags);
    m_cachedTags = newTags;
}
void Problem::setStatus(Status newStatus) { status = newStatus; }
void Problem::setTimeSpent(int newTimeSpent) { timeSpent = TimeSpent(newTimeSpent); }
void Problem::setDate(const std::string &newDate) { date = ProblemDate(newDate); }
void Problem::setRating(double newRating) { rating = ProblemRating(newRating); }
void Problem::setNotes(const std::string &newNotes) { notes = newNotes; }