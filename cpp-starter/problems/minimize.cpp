//
// Created by Lenovo on 2026. 11. 04..
//

#include "minimize.hpp"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>

// Splits a line into its space-separated parts, e.g. "q0 q1 q2" -> {"q0", "q1", "q2"}
static std::vector<std::string> splitBySpaces(const std::string &line) {
    std::istringstream stream(line);
    std::vector<std::string> parts;
    std::string part;

    while (stream >> part) {
        parts.push_back(part);
    }

    return parts;
}

// Joins the parts with spaces, e.g. {"s0", "s1", "s2"} -> "s0 s1 s2"
static std::string joinBySpaces(const std::vector<std::string> &parts) {
    std::string result;

    for (size_t i = 0; i < parts.size(); i++) {
        if (i > 0) {
            result += ' ';
        }

        result += parts[i];
    }

    return result;
}

// Checks if the list contains the element
static bool contains(const std::vector<std::string> &list, const std::string &element) {
    return std::find(list.begin(), list.end(), element) != list.end();
}

// Changes the first letter of a state to 's', e.g. "q1" -> "s1"
static std::string renameState(const std::string &state) {
    return "s" + state.substr(1);
}

// Register the --mini flag
void MinimizeProblem::initialize_parser(cxxopts::Options &options) {
    options.add_options()
        ("mini", "Minimize the DFA from the input file");
}

// The minimization problem runs when --mini is given
bool MinimizeProblem::is_chosen_problem(const cxxopts::ParseResult &args) {
    return args.count("mini") > 0;
}

// Reads the automaton from the input file and checks that it is valid
bool MinimizeProblem::loadAutomaton(const std::string &filename) {
    std::ifstream file(filename);

    if (!file) {
        std::cerr << "Error opening input file: " << filename << std::endl;
        return false;
    }

    std::string line;

    // Line 1: states
    std::getline(file, line);
    states = splitBySpaces(line);

    // Line 2: alphabet
    std::getline(file, line);
    alphabet = splitBySpaces(line);

    // Line 3: start state
    std::getline(file, line);
    std::vector<std::string> start = splitBySpaces(line);
    if (!start.empty()) {
        startState = start[0];
    }

    if (!contains(states, startState)) {
        std::cerr << "Invalid start state: " << startState << std::endl;
        return false;
    }

    // Line 4: final states
    std::getline(file, line);
    for (const std::string &state : splitBySpaces(line)) {
        if (!contains(states, state)) {
            std::cerr << "Invalid final state: " << state << std::endl;
            return false;
        }

        finalStates.insert(state);
    }

    // Remaining lines: one transition each, "from symbol to"
    while (std::getline(file, line)) {
        std::vector<std::string> parts = splitBySpaces(line);

        // Skip empty lines
        if (parts.empty()) {
            continue;
        }

        if (parts.size() != 3) {
            std::cerr << "Invalid transition: " << line << std::endl;
            return false;
        }

        if (!contains(states, parts[0]) || !contains(states, parts[2])) {
            std::cerr << "Unknown state in transition: " << line << std::endl;
            return false;
        }

        if (!contains(alphabet, parts[1])) {
            std::cerr << "Unknown symbol in transition: " << line << std::endl;
            return false;
        }

        // A DFA can only have one next state for a (state, symbol) pair
        auto it = transitions.find({parts[0], parts[1]});
        if (it != transitions.end() && it->second != parts[2]) {
            std::cerr << "The automaton is not deterministic: " << line << std::endl;
            return false;
        }

        transitions[{parts[0], parts[1]}] = parts[2];
    }

    return true;
}

// Collects the states that can be reached from the start state
std::set<std::string> MinimizeProblem::findReachableStates() const {
    std::set<std::string> reachable;
    std::vector<std::string> toVisit;

    reachable.insert(startState);
    toVisit.push_back(startState);

    while (!toVisit.empty()) {
        std::string current = toVisit.back();
        toVisit.pop_back();

        for (const std::string &symbol : alphabet) {
            auto it = transitions.find({current, symbol});

            if (it != transitions.end() && reachable.count(it->second) == 0) {
                reachable.insert(it->second);
                toVisit.push_back(it->second);
            }
        }
    }

    return reachable;
}

// Merges the equivalent states, the result is saved in newNames
void MinimizeProblem::minimize() {
    // Unreachable states are dropped
    std::set<std::string> reachable = findReachableStates();

    // Step 1: two groups, non-final states (0) and final states (1)
    std::map<std::string, int> group;
    for (const std::string &state : reachable) {
        group[state] = finalStates.count(state) > 0 ? 1 : 0;
    }

    // Step 2: split the groups until nothing changes.
    // Two states stay together only if they are in the same group now
    // and for every symbol they go into the same group.
    size_t groupCount = 0;

    while (true) {
        // signature = {own group, group after symbol 1, group after symbol 2, ...}
        std::map<std::vector<int>, int> signatureToGroup;
        std::map<std::string, int> newGroup;

        for (const std::string &state : reachable) {
            std::vector<int> signature;
            signature.push_back(group[state]);

            for (const std::string &symbol : alphabet) {
                auto it = transitions.find({state, symbol});

                // -1 means there is no transition with this symbol
                if (it == transitions.end()) {
                    signature.push_back(-1);
                } else {
                    signature.push_back(group[it->second]);
                }
            }

            // Same signature -> same new group
            if (signatureToGroup.count(signature) == 0) {
                int newId = static_cast<int>(signatureToGroup.size());
                signatureToGroup[signature] = newId;
            }

            newGroup[state] = signatureToGroup[signature];
        }

        group = newGroup;

        // No group was split, we are done
        if (signatureToGroup.size() == groupCount) {
            break;
        }

        groupCount = signatureToGroup.size();
    }

    // Step 3: every group gets the name of its smallest state.
    // The set is sorted, so the first state we meet from a group is the smallest one.
    std::map<int, std::string> groupName;
    for (const std::string &state : reachable) {
        if (groupName.count(group[state]) == 0) {
            groupName[group[state]] = renameState(state);
        }
    }

    for (const std::string &state : reachable) {
        newNames[state] = groupName[group[state]];
    }
}

// Writes the minimized automaton in the same format as the input
void MinimizeProblem::writeAutomaton(std::ostream &out) const {
    // std::set keeps everything sorted and removes the duplicates
    std::set<std::string> stateSet;
    for (const auto &entry : newNames) {
        stateSet.insert(entry.second);
    }

    std::set<std::string> finalSet;
    for (const std::string &state : finalStates) {
        if (newNames.count(state) > 0) {
            finalSet.insert(newNames.at(state));
        }
    }

    // {from, symbol, to}, sorted by from, then symbol, then to
    std::set<std::vector<std::string>> newTransitions;
    for (const auto &entry : transitions) {
        const std::string &from = entry.first.first;
        const std::string &symbol = entry.first.second;
        const std::string &to = entry.second;

        // Transitions of unreachable states are dropped
        if (newNames.count(from) > 0) {
            newTransitions.insert({newNames.at(from), symbol, newNames.at(to)});
        }
    }

    out << joinBySpaces(std::vector<std::string>(stateSet.begin(), stateSet.end())) << '\n';
    out << joinBySpaces(alphabet) << '\n';
    out << newNames.at(startState) << '\n';
    out << joinBySpaces(std::vector<std::string>(finalSet.begin(), finalSet.end()));

    for (const std::vector<std::string> &transition : newTransitions) {
        out << '\n' << transition[0] << ' ' << transition[1] << ' ' << transition[2];
    }
}

// Run the minimization problem
int MinimizeProblem::run(const cxxopts::ParseResult &args) {
    std::string inputFilename = args["input"].as<std::string>();
    std::string outputFilename = args["output"].as<std::string>();

    if (!loadAutomaton(inputFilename)) {
        return 1;
    }

    minimize();

    std::ofstream outputFile(outputFilename);

    if (!outputFile) {
        std::cerr << "Error opening output file: " << outputFilename << std::endl;
        return 1;
    }

    writeAutomaton(outputFile);
    return 0;
}