#ifndef BOOLEAN_MINIMIZER_HPP
#define BOOLEAN_MINIMIZER_HPP

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <set>
#include <map>
#include <algorithm>
#include <bitset>
#include <sstream>
#include <climits>

struct Minterm {
    std::string term;      // 例如: "10-1"
    std::set<int> minterms; // 原始 minterm 編號
    bool used;
    
    Minterm(std::string t, std::set<int> m) : term(t), minterms(m), used(false) {}
    
    int countOnes() const;
    int countDashes() const;
};

class BooleanMinimizer {
private:
    int numVars;
    std::vector<std::string> input_variables;
    std::string output_variables;
    std::vector<std::string> minterms;
    std::vector<std::string> dontCares;
    std::vector<Minterm> primeImplicants;
    
public:
    bool readPLA(const std::string& filename);
    std::vector<int> expandTerm(const std::string& term);
    bool canCombine(const std::string& a, const std::string& b, int& diffPos);
    void quineMcCluskey();
    bool covers(const Minterm& pi, int minterm);
    std::vector<Minterm> petrickMethod();
    void writePLA(const std::string& filename, const std::vector<Minterm>& solution);
    void minimize(const std::string& inputFile, const std::string& outputFile);
};

#endif // BOOLEAN_MINIMIZER_HPP