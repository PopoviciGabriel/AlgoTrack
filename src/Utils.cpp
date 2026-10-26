#include "Utils.h"
#include <algorithm>
#include <charconv>
#include <chrono>
#include <iostream>
#include <iomanip>
#include <locale>
#include <sstream>
#include <stdexcept>
#include <system_error>
#include <vector>
#include <cctype>

std::string toLowerCase(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), ::tolower);
    return s;
}

int levenshteinDistance(const std::string& a, const std::string& b) {
    int n = a.size();
    int m = b.size();

    std::vector<std::vector<int>> dp(n+1, std::vector<int>(m+1));
    for(int i=0; i<=n; i++) {
        dp[i][0] = i;
    }
    for(int j=0; j<=m; j++) {
        dp[0][j] = j;
    }

    for(int i=1; i<=n; i++) {
        for(int j=1; j<=m; j++) {
            if(a[i-1] == b[j-1]) {
                dp[i][j] = dp[i-1][j-1];
            }
            else{
                dp[i][j] = 1 + std::min({
                    dp[i-1][j],  // stergere
                    dp[i][j-1],  // inserare
                    dp[i-1][j-1] // inlocuire
                });
            }
        }
    }
    return dp[n][m];
}

int getFuzzyThreshold(const std::string& name) {
    std::string cleaned = trim(name);
    int len = static_cast<int>(cleaned.size());
    return std::min(4, std::max(1, len / 5));
}

std::string trim(const std::string& s) {
    size_t start = 0;
    while (start < s.size() && std::isspace((unsigned char)s[start])) {
        start++;
    }

    size_t end = s.size();
    while (end > start && std::isspace((unsigned char)s[end - 1])) {
        end--;
    }

    return s.substr(start, end - start);
}

bool isBlank(const std::string& s) {
    for (char c : s) {
        if (!std::isspace(static_cast<unsigned char>(c))) {
            return false;
        }
    }
    return true;
}

bool isLeapYear(int year) {
    if (year < 1900 || year > 2100) return false;
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

bool isValidDateFormat(const std::string& s) {
    // --- format check ---
    if (s.size() != 10) return false;
    if (s[2] != '-' || s[5] != '-') return false;

    for (int i = 0; i < (int)s.size(); i++) {
        if (i == 2 || i == 5) continue;
        if (!std::isdigit((unsigned char)s[i])) return false;
    }

    // --- parse ---
    int day = std::stoi(s.substr(0, 2));
    int month = std::stoi(s.substr(3, 2));
    int year = std::stoi(s.substr(6, 4));

    // --- validate month ---
    if (month < 1 || month > 12) return false;

    // --- days per month ---
    int daysInMonth[] = {
        31, // Jan
        28, // Feb
        31, // Mar
        30, // Apr
        31, // May
        30, // Jun
        31, // Jul
        31, // Aug
        30, // Sep
        31, // Oct
        30, // Nov
        31  // Dec
    };

    // leap year fix for Feb
    if (month == 2 && isLeapYear(year)) {
        daysInMonth[1] = 29;
    }

    // --- validate day ---
    if (day < 1 || day > daysInMonth[month - 1]) return false;

    return true;
}

bool isValidIsoDate(const std::string &s)
{
    if (s.size() != 10 || s[4] != '-' || s[7] != '-')
        return false;
    return isValidDateFormat(s.substr(8, 2) + "-" + s.substr(5, 2) + "-" + s.substr(0, 4));
}

static std::string formatIso(std::chrono::year_month_day ymd)
{
    std::ostringstream out;
    out << std::setfill('0')
        << std::setw(4) << static_cast<int>(ymd.year()) << '-'
        << std::setw(2) << static_cast<unsigned>(ymd.month()) << '-'
        << std::setw(2) << static_cast<unsigned>(ymd.day());
    return out.str();
}

std::string todayIsoDate()
{
    const auto today = std::chrono::floor<std::chrono::days>(std::chrono::system_clock::now());
    return formatIso(std::chrono::year_month_day{today});
}

std::string addDaysToIsoDate(const std::string &isoDate, int days)
{
    if (!isValidIsoDate(isoDate))
        throw std::invalid_argument("Date must be YYYY-MM-DD.");
    const int year = std::stoi(isoDate.substr(0, 4));
    const unsigned month = static_cast<unsigned>(std::stoi(isoDate.substr(5, 2)));
    const unsigned day = static_cast<unsigned>(std::stoi(isoDate.substr(8, 2)));
    const std::chrono::year_month_day ymd =
        std::chrono::year{year} / std::chrono::month{month} / std::chrono::day{day};
    const std::chrono::sys_days shifted = std::chrono::sys_days{ymd} + std::chrono::days{days};
    return formatIso(std::chrono::year_month_day{shifted});
}

std::string difficultyToString(Difficulty d) {
    switch (d) {
    case Difficulty::Easy: return "Easy";
    case Difficulty::Medium: return "Medium";
    case Difficulty::Hard: return "Hard";
    }
    return "";
}

std::string difficultyToCSV(Difficulty d) {
    switch (d) {
    case Difficulty::Easy: return "easy";
    case Difficulty::Medium: return "medium";
    case Difficulty::Hard: return "hard";
    }
    return "";
}

Difficulty parseDifficulty(const std::string& input) {
    std::string s = toLowerCase(input);

    if (s == "easy") return Difficulty::Easy;
    if (s == "medium") return Difficulty::Medium;
    if (s == "hard") return Difficulty::Hard;

    throw std::invalid_argument("Invalid difficulty.");
}

std::string statusToString(Status s) {
    switch (s) {
    case Status::Solved: return "Solved";
    case Status::Failed: return "Failed";
    case Status::InProgress: return "In Progress";
    }
    return "";
}

std::string statusToCSV(Status s) {
    switch (s) {
    case Status::Solved: return "solved";
    case Status::Failed: return "failed";
    case Status::InProgress: return "in_progress";
    }
    return "";
}

Status parseStatus(const std::string& input) {
    std::string s = toLowerCase(input);

    if (s == "solved") return Status::Solved;
    if (s == "failed") return Status::Failed;
    if (s == "in_progress" || s == "in progress" || s == "in progres" || s == "in_progres")
        return Status::InProgress;

    throw std::invalid_argument("Invalid status.");
}

std::string escapeCSVField(const std::string& field) {
    bool mustQuote = false;
    std::string result;

    for (char c : field) {
        if (c == '"' || c == ',' || c == '\n') {
            mustQuote = true;
        }

        if (c == '"') {
            result += "\"\""; // ghilimelele se dublează
        } else {
            result += c;
        }
    }

    if (mustQuote) {
        return "\"" + result + "\"";
    }

    return result;
}

std::vector<std::string> parseCSVLine(const std::string& line) {
    std::vector<std::string> fields;
    std::string current;
    bool inQuotes = false;

    for (size_t i = 0; i < line.size(); i++) {
        char c = line[i];

        if (c == '"') {
            if (inQuotes && i + 1 < line.size() && line[i + 1] == '"') {
                current += '"'; // "" -> "
                i++;
            } else {
                inQuotes = !inQuotes; // intrăm/ieșim din quoted field
            }
        }
        else if (c == ',' && !inQuotes) {
            fields.push_back(current);
            current.clear();
        }
        else {
            current += c;
        }
    }

    fields.push_back(current);
    return fields;
}

std::string formatCsvDouble(double value, int decimals)
{
    if (decimals < 0)
        decimals = 0;
    std::ostringstream out;
    out.imbue(std::locale::classic());
    out << std::fixed << std::setprecision(decimals) << value;
    return out.str();
}

double parseCsvDouble(const std::string &text)
{
    std::string normalized = trim(text);
    for (char &c : normalized)
    {
        if (c == ',')
            c = '.';
    }
    if (normalized.empty())
        throw std::runtime_error("Invalid numeric value in CSV.");

    double value = 0.0;
    const char *first = normalized.data();
    const char *last = first + normalized.size();
    const std::from_chars_result parsed = std::from_chars(first, last, value);
    if (parsed.ec != std::errc() || parsed.ptr != last)
        throw std::runtime_error("Invalid numeric value in CSV.");
    return value;
}