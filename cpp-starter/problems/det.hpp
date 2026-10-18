//
// Created by Lenovo on 2026. 10. 18..
//

#ifndef PROJECT_DET_HPP
#define PROJECT_DET_HPP

#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "../problem.hpp"

class DetProblem : public Problem {
public:
    void initialize_parser(cxxopts::Options &options) override;
    bool is_chosen_problem(const cxxopts::ParseResult &args) override;
    int run(const cxxopts::ParseResult &args) override;

private:
    std::vector<std::string> states;
    std::vector<std::string> alphabet;
    std::string startState;
    std::set<std::string> finalStates;

    // (current state, symbol) -> all next states (there can be more than one)
    std::map<std::pair<std::string, std::string>, std::set<std::string>> transitions;

    bool loadAutomaton(const std::string &filename);
    std::vector<std::string> determinize() const;
};



#endif //PROJECT_DET_HPP
