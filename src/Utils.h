#ifndef ALGOTRACK_UTILS_H
#define ALGOTRACK_UTILS_H

#include <string>
#include <vector>
#include "Enums.h"

std::string toLowerCase(std::string s);
int levenshteinDistance(const std::string &a, const std::string &b);
int getFuzzyThreshold(const std::string &name);
std::string trim(const std::string &s);
bool isBlank(const std::string &s);
bool isLeapYear(int year);
bool isValidDateFormat(const std::string &s);
bool isValidIsoDate(const std::string &s);
std::string todayIsoDate();
std::string addDaysToIsoDate(const std::string &isoDate, int days);

std::string difficultyToString(Difficulty d);
std::string difficultyToCSV(Difficulty d);
Difficulty parseDifficulty(const std::string &input);

std::string statusToString(Status s);
std::string statusToCSV(Status s);
Status parseStatus(const std::string &input);

std::string escapeCSVField(const std::string &field);
std::vector<std::string> parseCSVLine(const std::string &line);

// Punct zecimal, independent de locale-ul sistemului (ex. virgula din ro_RO).
std::string formatCsvDouble(double value, int decimals);
double parseCsvDouble(const std::string &text);

#endif // ALGOTRACK_UTILS_H
