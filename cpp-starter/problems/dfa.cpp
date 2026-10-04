#include <fstream>
#include <iostream>
#include <sstream>

#include "dfa.hpp"

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

// Splits the --check value into words, e.g. "a,ab,abc" -> {"a", "ab", "abc"}
static std::vector<std::string> splitByCommas(const std::string &text) {
    std::stringstream stream(text);
    std::vector<std::string> words;
    std::string word;

    while (std::getline(stream, word, ',')) {
        words.push_back(word);
    }

    return words;
}

// Register the --check flag
void DfaProblem::initialize_parser(cxxopts::Options &options) {
    options.add_options()
        ("check", "Comma-separated words to check against the DFA", cxxopts::value<std::string>());
}

// The DFA problem runs when --check is given
bool DfaProblem::is_chosen_problem(const cxxopts::ParseResult &args) {
    return args.count("check") > 0;
}

// Reads the automaton from the input file
bool DfaProblem::loadAutomaton(const std::string &filename) {
    std::ifstream file(filename);

    if (!file) {
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

        if (parts.size() == 3) {
            transitions[{parts[0], parts[1][0]}] = parts[2];
        }
    }

    return true;
}

// Walks the automaton over the word and checks where it ends up
bool DfaProblem::accepts(const std::string &word) const {
    std::string current = startState;

    for (char symbol : word) {
        auto it = transitions.find({current, symbol});

        // No transition for this symbol (or symbol not in the alphabet): reject
        if (it == transitions.end()) {
            return false;
        }

        current = it->second;
    }

    return finalStates.count(current) > 0;
}

// Run the DFA problem
int DfaProblem::run(const cxxopts::ParseResult &args) {
    std::string inputFilename = args["input"].as<std::string>();
    std::string outputFilename = args["output"].as<std::string>();
    std::string wordsToCheck = args["check"].as<std::string>();

    if (!loadAutomaton(inputFilename)) {
        std::cerr << "Error opening input file: " << inputFilename << std::endl;
        return 1;
    }

    std::ofstream outputFile(outputFilename);

    if (!outputFile) {
        std::cerr << "Error opening output file: " << outputFilename << std::endl;
        return 1;
    }

    std::vector<std::string> words = splitByCommas(wordsToCheck);

    for (size_t i = 0; i < words.size(); i++) {
        if (i > 0) {
            outputFile << '\n';
        }

        outputFile << (accepts(words[i]) ? "IGEN" : "NEM");
    }

    return 0;
}