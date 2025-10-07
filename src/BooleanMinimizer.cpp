#include "BooleanMinimizer.hpp"

using namespace std;

int Minterm::countOnes() const {
    int count = 0;
    for (char c : term) {
        if (c == '1') count++;
    }
    return count;
}

int Minterm::countDashes() const {
    int count = 0;
    for (char c : term) {
        if (c == '-') count++;
    }
    return count;
}

std::string trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    size_t end = str.find_last_not_of(" \t\r\n");
    return str.substr(start, end - start + 1);
}

bool BooleanMinimizer::readPLA(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open file " << filename << std::endl;
        return false;
    }
    
    std::string line;
    
    while (std::getline(file, line)) {
        line = trim(line);
        
        // 跳過空行和註解
        if (line.empty() || line[0] == '#') continue;
        
        // .i 輸入變數數量
        if (line.find(".i ") == 0) {
            std::string num_str = trim(line.substr(3));
            try {
                numVars = std::stoi(num_str);
            } catch (const std::exception& e) {
                std::cerr << "Error parsing number of variables: " << num_str << std::endl;
                return false;
            }
            continue;
        }
        
        // .o 輸出變數數量
        if (line.find(".o ") == 0) {
            std::string num_str = trim(line.substr(3));
            continue;
        }
        
        // .ilb 輸入變數名稱
        if (line.find(".ilb ") == 0) {
            std::string vars = line.substr(5);
            std::istringstream iss(vars);
            std::string var;
            input_variables.clear();
            while (iss >> var) {
                input_variables.push_back(var);
            }
            continue;
        }
        
        // .ob 輸出變數名稱
        if (line.find(".ob ") == 0) {
            std::string output_name = trim(line.substr(4));
            output_variables = output_name;
            continue;
        }
        
        // .p Product Terms 數量
        if (line.find(".p ") == 0) {
            std::string num_str = trim(line.substr(3));
            continue;
        }
        
        // .e 結束
        if (line == ".e") { 
            break;
        }
        
        // 處理 Product Terms
        size_t space_pos = line.find(' ');
        if (space_pos != std::string::npos) {
            std::string input_part = trim(line.substr(0, space_pos));
            std::string output_part = trim(line.substr(space_pos + 1));
            
            if (output_part == "1"){ 
                minterms.push_back(input_part);
            } else if (output_part == "-"){ 
                dontCares.push_back(input_part);
            }
        }
    }

    file.close();
    return true;
}

vector<int> BooleanMinimizer::expandTerm(const string& term) {
    vector<int> result;
    vector<int> dashPos;
    
    for (int i = 0; i < term.length(); i++) {
        if (term[i] == '-') {
            dashPos.push_back(i);
        }
    }
    
    int numCombinations = 1 << dashPos.size();
    for (int i = 0; i < numCombinations; i++) {
        string expanded = term;
        for (int j = 0; j < dashPos.size(); j++) {
            expanded[dashPos[j]] = (i & (1 << j)) ? '1' : '0';
        }
        
        // 檢查展開的字符串是否只包含 0 和 1
        bool valid = true;
        for (char c : expanded) {
            if (c != '0' && c != '1') {
                valid = false;
                break;
            }
        }
        
        if (valid && !expanded.empty()) {
            try {
                result.push_back(stoi(expanded, nullptr, 2));
            } catch (const std::invalid_argument& e) {
                cerr << "Error converting string to int: " << expanded << endl;
                continue;
            }
        }
    }
    
    return result;
}

bool BooleanMinimizer::canCombine(const string& a, const string& b, int& diffPos) {
    int differences = 0;
    diffPos = -1;
    
    for (int i = 0; i < a.length(); i++) {
        if (a[i] != b[i]) {
            if (a[i] == '-' || b[i] == '-') return false;
            differences++;
            diffPos = i;
            if (differences > 1) return false;
        }
    }
    
    return differences == 1;
}

void BooleanMinimizer::quineMcCluskey() {
    set<int> expandedMinterms;
    map<int, string> mintermMap;
    
    // 展開所有 minterms 和 don't cares
    for (const string& term : minterms) {
        vector<int> expanded = expandTerm(term);
        for (int m : expanded) {
            expandedMinterms.insert(m);
            mintermMap[m] = term;
        }
    }
    
    for (const string& term : dontCares) {
        vector<int> expanded = expandTerm(term);
        for (int m : expanded) {
            expandedMinterms.insert(m);
        }
    }
    
    // 初始化第一組 terms
    vector<Minterm> currentGroup;
    for (int minterm : expandedMinterms) {
        string mintermStr = "";
        for (int i = numVars - 1; i >= 0; i--) {
            mintermStr += ((minterm >> i) & 1) ? '1' : '0';
        }
        set<int> s;
        s.insert(minterm);
        currentGroup.push_back(Minterm(mintermStr, s));
    }
    
    // 重複合併
    while (true) {
        map<int, vector<Minterm>> grouped;
        
        // 用1的數量分組
        for (const auto& minterm : currentGroup) {
            grouped[minterm.countOnes()].push_back(minterm);
        }
        
        vector<Minterm> nextGroup;
        set<string> nextGroupSet;
        
        // 相鄰的組
        for (auto& pair : grouped) {
            int ones = pair.first;
            if (grouped.find(ones + 1) == grouped.end()) continue;
            
            for (auto& term1 : grouped[ones]) {
                for (auto& term2 : grouped[ones + 1]) {
                    int diffPos;
                    if (canCombine(term1.term, term2.term, diffPos)) {
                        string newMintermStr = term1.term;
                        newMintermStr[diffPos] = '-';
                        
                        if (nextGroupSet.find(newMintermStr) == nextGroupSet.end()) {
                            set<int> combinedMinterms = term1.minterms;
                            combinedMinterms.insert(term2.minterms.begin(), 
                                                   term2.minterms.end());
                            nextGroup.push_back(Minterm(newMintermStr, combinedMinterms));
                            nextGroupSet.insert(newMintermStr);
                        }
                        
                        term1.used = true;
                        term2.used = true;
                    }
                }
            }
        }
        
        // 找沒打勾的 minterms 變 PI
        for (auto& term : currentGroup) {
            if (!term.used) {
                bool found = false;
                for (const auto& pi : primeImplicants) {
                    if (pi.term == term.term) {
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    primeImplicants.push_back(term);
                }
            }
        }
        
        if (nextGroup.empty()) break;
        
        currentGroup = nextGroup;
    }
}

bool BooleanMinimizer::covers(const Minterm& pi, int minterm) {
    return pi.minterms.find(minterm) != pi.minterms.end();
}

vector<Minterm> BooleanMinimizer::petrickMethod() {
    // 需要覆蓋的所有 minterms 不含 don't cares
    set<int> requiredMinterms;
    for (const string& term : minterms) {
        vector<int> expanded = expandTerm(term);
        for (int m : expanded) {
            requiredMinterms.insert(m);
        }
    }
    
    
    // 找出EPI
    vector<Minterm> essentialPIs;
    set<int> coveredMinterms;
    
    for (int minterm : requiredMinterms) {
        vector<const Minterm*> coveringPIs;
        
        // 找能包住這個 minterm 的 PI
        for (const auto& pi : primeImplicants) {
            if (covers(pi, minterm)) {
                coveringPIs.push_back(&pi);
            }
        }
        
        // minterm只被一個PI包住 就是必要的
        if (coveringPIs.size() == 1) {
            const Minterm* essentialPI = coveringPIs[0];
            
            // 檢查是否已經加入
            bool alreadyAdded = false;
            for (const auto& epi : essentialPIs) {
                if (epi.term == essentialPI->term) {
                    alreadyAdded = true;
                    break;
                }
            }
            
            if (!alreadyAdded) {
                essentialPIs.push_back(*essentialPI);
                
                // 標記所有被此 PI 包到的 minterms
                for (int m : essentialPI->minterms) {
                    if (requiredMinterms.find(m) != requiredMinterms.end()) {
                        coveredMinterms.insert(m);
                    }
                }
            }
        }
    }
    
    // 找出剩餘未被包進的 minterms
    set<int> remainingMinterms;
    for (int m : requiredMinterms) {
        if (coveredMinterms.find(m) == coveredMinterms.end()) {
            remainingMinterms.insert(m);
        }
    }
    
    vector<Minterm> solution = essentialPIs;
    
    while (!remainingMinterms.empty()) {
        const Minterm* bestPI = nullptr;
        double bestScore = -1;
        int maxCoverage = 0;
        
        // 找到最佳的 PI
        for (const auto& pi : primeImplicants) {
            bool alreadySelected = false;
            for (const auto& selectedPI : solution) {
                if (selectedPI.term == pi.term) {
                    alreadySelected = true;
                    break;
                }
            }
            if (alreadySelected) continue;
            
            int coverage = 0;
            for (int m : remainingMinterms) {
                if (covers(pi, m)) {
                    coverage++;
                }
            }
            
            if (coverage > 0) {
                int literals = numVars - pi.countDashes();
                double score = (double)coverage / literals;
                
                // 優先選包更多 minterms 的
                if (coverage > maxCoverage || 
                    (coverage == maxCoverage && score > bestScore) ||
                    (coverage == maxCoverage && score == bestScore && 
                     (bestPI == nullptr || pi.term < bestPI->term))) {
                    maxCoverage = coverage;
                    bestScore = score;
                    bestPI = &pi;
                }
            }
        }
        
        if (bestPI) {
            solution.push_back(*bestPI);
            
            // 移除被包住的 minterms
            for (int m : bestPI->minterms) {
                remainingMinterms.erase(m);
            }
        } else {
            cout << "Cannot find PI to cover remaining minterms!" << endl;
            break; // 無法找到更多包
        }
    }
    
    return solution;
}

void BooleanMinimizer::writePLA(const string& filename, const vector<Minterm>& solution) {
    ofstream file(filename);
    
    file << ".i " << numVars << endl;
    file << ".o 1" << endl;
    file << ".ilb";
    for (const string& label : input_variables) {
        file << " " << label;
    }
    file << endl;
    file << ".ob f" << endl;
    file << ".p " << solution.size() << endl;
    
    for (const auto& term : solution) {
        file << term.term << " 1" << endl;
    }
    
    file << ".e" << endl;
    file.close();
}

void BooleanMinimizer::minimize(const string& inputFile, const string& outputFile) {
    if (!readPLA(inputFile)) {      
        std::cout << "Can not read the file: " << inputFile << std::endl;
        return;
    }
    
    quineMcCluskey();
    
    vector<Minterm> solution = petrickMethod();
    writePLA(outputFile, solution);
    std::cout << "Done! Output written to: " << outputFile << std::endl;
}
