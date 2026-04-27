#pragma once
#include <string>
#include <vector>
#include "Enums.h"

std::string toLowerCase(std::string s);
int getFuzzyThreshold(const std::string& name);
int levenshteinDistance(const std::string& a, const std::string& b);
std::string trim(const std::string& s);

// --- For setters
bool isBlank(const std::string& s);
bool isLeapYear(int year);
bool isValidDateFormat(const std::string& s);

// --- Difficulty ---
std::string difficultyToString(Difficulty d);   // UI
std::string difficultyToCSV(Difficulty d);      // fișier
Difficulty parseDifficulty(const std::string& s); // text -> enum

// --- Status ---
std::string statusToString(Status s);   // UI
std::string statusToCSV(Status s);      // fișier
Status parseStatus(const std::string& s); // text -> enum

std::string escapeCSVField(const std::string& field);
std::vector<std::string> parseCSVLine(const std::string& line);
