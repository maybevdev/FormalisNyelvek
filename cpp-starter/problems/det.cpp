#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>

#include "det.hpp"

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

// Joins the parts with spaces, e.g. {"s0", "s1"} -> "s0 s1"
static std::string joinWithSpaces(const std::vector<std::string> &parts) {
    std::string result;

    for (size_t i = 0; i < parts.size(); i++) {
        if (i > 0) {
            result += " ";
        }

        result += parts[i];
    }

    return result;
}

// Checks if the list contains the given element
static bool contains(const std::vector<std::string> &list, const std::string &element) {
    return std::find(list.begin(), list.end(), element) != list.end();
}

// Register the --det flag
void DetProblem::initialize_parser(cxxopts::Options &options) {
    options.add_options()
        ("det", "Convert the automaton into a deterministic one");
}

// The determinization problem runs when --det is given
bool DetProblem::is_chosen_problem(const cxxopts::ParseResult &args) {
    return args.count("det") > 0;
}

// Reads the automaton from the input file and checks the transitions
bool DetProblem::loadAutomaton(const std::string &filename) {
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

    // Line 4: final states
    std::getline(file, line);
    for (const std::string &state : splitBySpaces(line)) {
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

        // The same symbol can lead to more states, so we collect them in a set
        transitions[{parts[0], parts[1]}].insert(parts[2]);
    }

    return true;
}

// Subset construction: every new state is a set of old states.
// Returns the lines of the output file.
std::vector<std::string> DetProblem::determinize() const {
    // Transitions must be listed in alphabetical order of the symbols
    std::vector<std::string> symbols = alphabet;
    std::sort(symbols.begin(), symbols.end());

    // newStates[i] = the old states that make up s<i>
    std::vector<std::set<std::string>> newStates;
    newStates.push_back({startState});

    std::vector<std::string> transitionLines;

    // newStates grows while we go through it, new sets are added at the end
    for (size_t i = 0; i < newStates.size(); i++) {
        for (const std::string &symbol : symbols) {
            // Collect every state reachable from s<i> with this symbol
            std::set<std::string> next;

            for (const std::string &state : newStates[i]) {
                auto it = transitions.find({state, symbol});

                if (it != transitions.end()) {
                    next.insert(it->second.begin(), it->second.end());
                }
            }

            // No transition: we don't list this edge
            if (next.empty()) {
                continue;
            }

            // Find the set among the new states, or add it if it is not there yet
            auto found = std::find(newStates.begin(), newStates.end(), next);
            size_t nextIndex = found - newStates.begin();

            if (found == newStates.end()) {
                newStates.push_back(next);
            }

            transitionLines.push_back("s" + std::to_string(i) + " " + symbol + " s" + std::to_string(nextIndex));
        }
    }

    // A new state is final if it contains at least one old final state
    std::vector<std::string> stateNames;
    std::vector<std::string> finalNames;

    for (size_t i = 0; i < newStates.size(); i++) {
        std::string name = "s" + std::to_string(i);
        stateNames.push_back(name);

        for (const std::string &state : newStates[i]) {
            if (finalStates.count(state) > 0) {
                finalNames.push_back(name);
                break;
            }
        }
    }

    // Build the output in the same format as the input
    std::vector<std::string> lines;
    lines.push_back(joinWithSpaces(stateNames));
    lines.push_back(joinWithSpaces(alphabet));
    lines.push_back("s0");
    lines.push_back(joinWithSpaces(finalNames));

    for (const std::string &transitionLine : transitionLines) {
        lines.push_back(transitionLine);
    }

    return lines;
}

// Run the determinization problem
int DetProblem::run(const cxxopts::ParseResult &args) {
    std::string inputFilename = args["input"].as<std::string>();
    std::string outputFilename = args["output"].as<std::string>();

    if (!loadAutomaton(inputFilename)) {
        return 1;
    }

    std::ofstream outputFile(outputFilename);

    if (!outputFile) {
        std::cerr << "Error opening output file: " << outputFilename << std::endl;
        return 1;
    }

    std::vector<std::string> lines = determinize();

    for (size_t i = 0; i < lines.size(); i++) {
        if (i > 0) {
            outputFile << '\n';
        }

        outputFile << lines[i];
    }

    return 0;
}