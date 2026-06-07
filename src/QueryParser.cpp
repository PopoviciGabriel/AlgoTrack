#include "QueryParser.h"
#include "Utils.h"
#include <sstream>

std::vector<FilterRule> QueryParser::parse(const std::string &query)
{
    std::vector<FilterRule> rules;
    std::stringstream ss(query);
    std::string token;

    while (ss >> token)
    {
        std::string lowerToken = toLowerCase(token);

        if (lowerToken.find("platform:") == 0)
        {
            rules.push_back({FilterRule::Type::PlatformEquals, lowerToken.substr(9)});
        }
        else if (lowerToken.find("tag:") == 0)
        {
            rules.push_back({FilterRule::Type::TagContains, lowerToken.substr(4)});
        }
        else if (lowerToken.find("diff:") == 0)
        {
            rules.push_back({FilterRule::Type::DifficultyEquals, lowerToken.substr(5)});
        }
        else if (lowerToken.find("status:") == 0)
        {
            std::string st = lowerToken.substr(7);
            if (st == "in-progress" || st == "in_progress")
                st = "inprogress";
            rules.push_back({FilterRule::Type::StatusEquals, st});
        }
        else
        {
            rules.push_back({FilterRule::Type::NameMatches, lowerToken});
        }
    }
    return rules;
}

bool QueryParser::matches(const Problem &p, const std::vector<FilterRule> &rules)
{
    for (const auto &rule : rules)
    {
        switch (rule.type)
        {
        case FilterRule::Type::NameMatches:
        {
            if (toLowerCase(p.getName()).find(rule.value) == std::string::npos)
                return false;
            break;
        }
        case FilterRule::Type::PlatformEquals:
        {
            if (toLowerCase(p.getPlatform()) != rule.value)
                return false;
            break;
        }
        case FilterRule::Type::TagContains:
        {
            bool hasTag = false;
            for (const auto &tag : p.getTags())
            {
                if (toLowerCase(tag) == rule.value)
                {
                    hasTag = true;
                    break;
                }
            }
            if (!hasTag)
                return false;
            break;
        }
        case FilterRule::Type::DifficultyEquals:
        {
            std::string diffStr;
            if (p.getDifficulty() == Difficulty::Easy)
                diffStr = "easy";
            else if (p.getDifficulty() == Difficulty::Medium)
                diffStr = "medium";
            else
                diffStr = "hard";
            if (diffStr != rule.value)
                return false;
            break;
        }
        case FilterRule::Type::StatusEquals:
        {
            std::string statStr;
            if (p.getStatus() == Status::Solved)
                statStr = "solved";
            else if (p.getStatus() == Status::Failed)
                statStr = "failed";
            else
                statStr = "inprogress";
            if (statStr != rule.value)
                return false;
            break;
        }
        }
    }
    return true;
}