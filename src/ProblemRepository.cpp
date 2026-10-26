#include "ProblemRepository.h"
#include "Utils.h"

#include <cctype>
#include <filesystem>
#include <fstream>
#include <map>
#include <sstream>

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

namespace
{
bool replaceFile(const std::filesystem::path &temporary, const std::filesystem::path &target)
{
#if defined(_WIN32)
    return MoveFileExW(temporary.c_str(), target.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH) != 0;
#else
    std::error_code error;
    std::filesystem::rename(temporary, target, error);
    return !error;
#endif
}

bool writeAtomically(const std::string &filename, const std::string &contents)
{
    const std::filesystem::path target(filename);
    const std::filesystem::path temporary = target.wstring() + L".tmp";
    {
        std::ofstream file(temporary, std::ios::binary | std::ios::trunc);
        if (!file.is_open())
            return false;
        file << contents;
        file.flush();
        if (!file)
        {
            file.close();
            std::error_code ignored;
            std::filesystem::remove(temporary, ignored);
            return false;
        }
    }
    if (!replaceFile(temporary, target))
    {
        std::error_code ignored;
        std::filesystem::remove(temporary, ignored);
        return false;
    }
    return true;
}

std::string markdownLabel(std::string text)
{
    std::string cleaned;
    for (char c : text)
    {
        if (c == '[' || c == ']' || c == '\n' || c == '\r')
            cleaned.push_back(' ');
        else
            cleaned.push_back(c);
    }
    return cleaned;
}

std::string markdownTarget(const std::string &target)
{
    if (target.rfind("http://", 0) == 0 || target.rfind("https://", 0) == 0)
        return target;
    std::string normalized = target;
    for (char &c : normalized)
    {
        if (c == '\\')
            c = '/';
    }
    if (normalized.size() >= 2 && std::isalpha(static_cast<unsigned char>(normalized[0])) && normalized[1] == ':')
        return "file:///" + normalized;
    return normalized;
}
}

static bool problemExistsInVector(const Problem &p, const std::vector<Problem> &problems)
{
    for (const auto &existing : problems)
    {
        if (toLowerCase(existing.getName()) == toLowerCase(p.getName()) &&
            toLowerCase(existing.getPlatform()) == toLowerCase(p.getPlatform()))
        {
            return true;
        }
    }
    return false;
}

LoadResult ProblemRepository::loadFromFile(const std::string &filename)
{
    LoadResult result;
    std::ifstream file(filename);
    if (!file.is_open())
    {
        result.fileOpened = false;
        return result;
    }

    std::string line;
    while (std::getline(file, line))
    {
        if (line.empty())
            continue;
        try
        {
            Problem p;
            p.fromCSV(line);
            if (problemExistsInVector(p, result.problems))
            {
                result.duplicateCount++;
            }
            else
            {
                result.problems.push_back(p);
                result.loadedCount++;
            }
        }
        catch (...)
        {
            result.invalidCount++;
        }
    }
    return result;
}

bool ProblemRepository::saveToFile(const std::string &filename, const std::vector<Problem> &problems)
{
    std::ostringstream contents;
    for (const auto &p : problems)
        contents << p.toCSV() << "\n";
    return writeAtomically(filename, contents.str());
}

ImportResult ProblemRepository::importFromFile(const std::string &filename, std::vector<Problem> &problems)
{
    ImportResult result;
    std::ifstream file(filename);
    if (!file.is_open())
    {
        result.fileOpened = false;
        return result;
    }

    std::string line;
    while (std::getline(file, line))
    {
        if (line.empty())
            continue;
        try
        {
            Problem p;
            p.fromCSV(line);
            if (problemExistsInVector(p, problems))
            {
                result.duplicateCount++;
            }
            else
            {
                problems.push_back(p);
                result.importedCount++;
            }
        }
        catch (...)
        {
            result.invalidCount++;
        }
    }
    return result;
}

bool ProblemRepository::exportToFile(const std::string &filename, const std::vector<Problem> &problems)
{
    return saveToFile(filename, problems);
}

bool ProblemRepository::exportMarkdown(const std::string &filename, const std::vector<Problem> &problems)
{
    std::map<std::string, std::map<int, std::vector<const Problem *>>> grouped;
    for (const auto &problem : problems)
    {
        for (const auto &tag : problem.getTags())
            grouped[tag][static_cast<int>(problem.getDifficulty())].push_back(&problem);
    }

    std::ostringstream out;
    out << "# AlgoTrack\n\n";
    out << "Practice log grouped by tag and difficulty.\n\n";
    const char *difficultyNames[] = {"Easy", "Medium", "Hard"};

    for (const auto &tagGroup : grouped)
    {
        out << "## " << markdownLabel(tagGroup.first) << "\n\n";
        for (const auto &difficultyGroup : tagGroup.second)
        {
            const int difficulty = difficultyGroup.first;
            const char *difficultyName = (difficulty >= 0 && difficulty <= 2) ? difficultyNames[difficulty] : "Unknown";
            out << "### " << difficultyName << "\n\n";
            for (const Problem *problem : difficultyGroup.second)
            {
                const std::string name = markdownLabel(problem->getName());
                if (!problem->getProblemUrl().empty())
                    out << "- [" << name << "](" << markdownTarget(problem->getProblemUrl()) << ")";
                else
                    out << "- **" << name << "**";
                out << " - " << problem->getPlatform()
                    << " - " << statusToString(problem->getStatus())
                    << " - due " << problem->getDueDate() << "\n";
                if (!problem->getSolutionPath().empty())
                    out << "  - Solution: [" << markdownLabel(problem->getSolutionPath()) << "]("
                        << markdownTarget(problem->getSolutionPath()) << ")\n";
                if (!problem->getTimeComplexity().empty() || !problem->getSpaceComplexity().empty())
                {
                    out << "  - Complexity:";
                    if (!problem->getTimeComplexity().empty())
                        out << " time " << markdownLabel(problem->getTimeComplexity());
                    if (!problem->getSpaceComplexity().empty())
                        out << " space " << markdownLabel(problem->getSpaceComplexity());
                    out << "\n";
                }
                if (!problem->getNotes().empty())
                    out << "  - Notes: " << markdownLabel(problem->getNotes()) << "\n";
            }
            out << "\n";
        }
    }

    if (grouped.empty())
        out << "_No problems yet._\n";
    return writeAtomically(filename, out.str());
}
