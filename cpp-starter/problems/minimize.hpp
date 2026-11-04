//
// Created by Lenovo on 2026. 11. 04..
//

#ifndef PROJECT_MINIMIZE_HPP
#define PROJECT_MINIMIZE_HPP

#include <map>
#include <ostream>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "../problem.hpp"

class MinimizeProblem : public Problem {
public:
    void initialize_parser(cxxopts::Options &options) override;
    bool is_chosen_problem(const cxxopts::ParseResult &args) override;
    int run(const cxxopts::ParseResult &args) override;

private:
    std::vector<std::string> states;
    std::vector<std::string> alphabet;
    std::string startState;
    std::set<std::string> finalStates;

    // (current state, symbol) -> next state
    std::map<std::pair<std::string, std::string>, std::string> transitions;

    // original state -> name of the merged state, e.g. "q3" -> "s0"
    std::map<std::string, std::string> newNames;

    bool loadAutomaton(const std::string &filename);
    std::set<std::string> findReachableStates() const;
    void minimize();
    void writeAutomaton(std::ostream &out) const;
};


#endif //PROJECT_MINIMIZE_HPP
