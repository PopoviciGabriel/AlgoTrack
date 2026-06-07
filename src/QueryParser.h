#ifndef ALGOTRACK_QUERY_PARSER_H
#define ALGOTRACK_QUERY_PARSER_H

#include <string>
#include <vector>
#include "Problem.h"

struct FilterRule
{
    enum class Type
    {
        NameMatches,
        PlatformEquals,
        TagContains,
        DifficultyEquals,
        StatusEquals
    } type;
    std::string value;
};

class QueryParser
{
public:
    static std::vector<FilterRule> parse(const std::string &query);
    static bool matches(const Problem &p, const std::vector<FilterRule> &rules);
};

#endif // ALGOTRACK_QUERY_PARSER_H