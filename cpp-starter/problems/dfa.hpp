#ifndef PROBLEMS_DFA_H
#define PROBLEMS_DFA_H

#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "../problem.hpp"

class DfaProblem : public Problem {
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
    std::map<std::pair<std::string, char>, std::string> transitions;

    bool loadAutomaton(const std::string &filename);
    bool accepts(const std::string &word) const;
};

#endif // PROBLEMS_DFA_H