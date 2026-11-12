//
// Created by Lenovo on 2026. 11. 12..
//

#ifndef FORMALISNYELVEK_REGEXES_HPP
#define FORMALISNYELVEK_REGEXES_HPP


#include <string>
#include <vector>

#include "../problem.hpp"

class RegexProblem : public Problem {
public:
    void initialize_parser(cxxopts::Options &options) override;
    bool is_chosen_problem(const cxxopts::ParseResult &args) override;
    int run(const cxxopts::ParseResult &args) override;

private:
    // The lines of the log file
    std::vector<std::string> lines;

    bool loadLines(const std::string &filename);

    // Every task returns the lines of the output file
    std::vector<std::string> countMatchingLines() const;     // --rgx 1
    std::vector<std::string> findErrorJavaFiles() const;     // --rgx 2
    std::vector<std::string> findProcessIds() const;         // --rgx 3
    std::vector<std::string> findMostErrors() const;         // --rgx 4
    std::vector<std::string> countMainThreadErrors() const;  // --rgx 5
};

#endif //FORMALISNYELVEK_REGEXES_HPP
