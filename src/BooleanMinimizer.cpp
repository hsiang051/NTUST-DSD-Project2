#include "BooleanMinimizer.hpp"
#include <climits>

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
            
            // 只保留輸出為1的
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
    vector<int> dashPositions;
    
    for (int i = 0; i < term.length(); i++) {
        if (term[i] == '-') {
            dashPositions.push_back(i);
        }
    }
    
    int numCombinations = 1 << dashPositions.size();
    for (int i = 0; i < numCombinations; i++) {
        string expanded = term;
        for (int j = 0; j < dashPositions.size(); j++) {
            expanded[dashPositions[j]] = (i & (1 << j)) ? '1' : '0';
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
    set<int> allMinterms;
    map<int, string> mintermMap;
    
    // 展開所有 minterms 和 don't cares
    for (const string& term : minterms) {
        vector<int> expanded = expandTerm(term);
        for (int m : expanded) {
            allMinterms.insert(m);
            mintermMap[m] = term;
        }
    }
    
    for (const string& term : dontCares) {
        vector<int> expanded = expandTerm(term);
        for (int m : expanded) {
            allMinterms.insert(m);
        }
    }
    
    // 初始化第一組 terms
    vector<Minterm> currentGroup;
    for (int m : allMinterms) {
        string binary = "";
        for (int i = numVars - 1; i >= 0; i--) {
            binary += ((m >> i) & 1) ? '1' : '0';
        }
        set<int> s;
        s.insert(m);
        currentGroup.push_back(Minterm(binary, s));
    }
    
    vector<Minterm> allTerms = currentGroup;
    
    // 反覆合併
    while (true) {
        map<int, vector<Minterm>> grouped;
        
        // 按照 1 的個數分組
        for (const auto& term : currentGroup) {
            grouped[term.countOnes()].push_back(term);
        }
        
        vector<Minterm> nextGroup;
        set<string> nextGroupSet;
        
        // 嘗試合併相鄰組
        for (auto& pair : grouped) {
            int ones = pair.first;
            if (grouped.find(ones + 1) == grouped.end()) continue;
            
            for (auto& term1 : grouped[ones]) {
                for (auto& term2 : grouped[ones + 1]) {
                    int diffPos;
                    if (canCombine(term1.term, term2.term, diffPos)) {
                        string newTerm = term1.term;
                        newTerm[diffPos] = '-';
                        
                        if (nextGroupSet.find(newTerm) == nextGroupSet.end()) {
                            set<int> combinedMinterms = term1.minterms;
                            combinedMinterms.insert(term2.minterms.begin(), 
                                                   term2.minterms.end());
                            nextGroup.push_back(Minterm(newTerm, combinedMinterms));
                            nextGroupSet.insert(newTerm);
                        }
                        
                        term1.used = true;
                        term2.used = true;
                    }
                }
            }
        }
        
        // 收集未使用的 terms (這些是 prime implicants)
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
        allTerms.insert(allTerms.end(), nextGroup.begin(), nextGroup.end());
    }
}

bool BooleanMinimizer::covers(const Minterm& pi, int minterm) {
    return pi.minterms.find(minterm) != pi.minterms.end();
}

vector<Minterm> BooleanMinimizer::petrickMethod() {
    // 首先找出我們需要覆蓋的所有 minterms (不包括 don't cares)
    set<int> requiredMinterms;
    for (const string& term : minterms) {
        vector<int> expanded = expandTerm(term);
        for (int m : expanded) {
            requiredMinterms.insert(m);
        }
    }
    
    cout << "Required minterms to cover: ";
    for (int m : requiredMinterms) {
        cout << m << " ";
    }
    cout << endl;
    
    // 找出必要的 prime implicants (essential prime implicants)
    vector<Minterm> essentialPIs;
    set<int> coveredMinterms;
    
    for (int minterm : requiredMinterms) {
        vector<const Minterm*> coveringPIs;
        
        // 找到所有能覆蓋此 minterm 的 prime implicants
        for (const auto& pi : primeImplicants) {
            if (covers(pi, minterm)) {
                coveringPIs.push_back(&pi);
            }
        }
        
        // 如果只有一個 PI 能覆蓋此 minterm，它就是必要的
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
                cout << "Essential PI found: " << essentialPI->term << endl;
                
                // 標記所有被此 PI 覆蓋的 minterms
                for (int m : essentialPI->minterms) {
                    if (requiredMinterms.find(m) != requiredMinterms.end()) {
                        coveredMinterms.insert(m);
                    }
                }
            }
        }
    }
    
    // 找出剩餘未覆蓋的 minterms
    set<int> remainingMinterms;
    for (int m : requiredMinterms) {
        if (coveredMinterms.find(m) == coveredMinterms.end()) {
            remainingMinterms.insert(m);
        }
    }
    
    cout << "Remaining minterms after essential PIs: ";
    for (int m : remainingMinterms) {
        cout << m << " ";
    }
    cout << endl;
    
    // 使用改進的貪心演算法選擇剩餘的 PIs
    vector<Minterm> solution = essentialPIs;
    
    while (!remainingMinterms.empty()) {
        const Minterm* bestPI = nullptr;
        double bestScore = -1;
        int maxCoverage = 0;
        
        // 找到最佳的 PI
        for (const auto& pi : primeImplicants) {
            // 檢查是否已經在解決方案中
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
                double score = (double)coverage / literals; // 覆蓋數量除以 literal 數量
                
                // 優先選擇覆蓋更多 minterms 的，然後考慮效率，最後考慮字典序
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
            cout << "Selected PI: " << bestPI->term << " (covers " << maxCoverage 
                 << " remaining minterms, " << (numVars - bestPI->countDashes()) 
                 << " literals)" << endl;
            
            // 移除被覆蓋的 minterms
            for (int m : bestPI->minterms) {
                remainingMinterms.erase(m);
            }
        } else {
            cout << "Warning: Cannot find PI to cover remaining minterms!" << endl;
            break; // 無法找到更多覆蓋
        }
    }
    
    cout << "Final solution with " << solution.size() << " terms:" << endl;
    for (const auto& pi : solution) {
        cout << "  " << pi.term << endl;
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
    
    // 簡單按字典序排序
    vector<Minterm> sortedSolution = solution;
    sort(sortedSolution.begin(), sortedSolution.end(), [](const Minterm& a, const Minterm& b) {
        return a.term < b.term;
    });
    
    for (const auto& term : sortedSolution) {
        file << term.term << " 1" << endl;
    }
    
    file << ".e" << endl;
    file.close();
}

void BooleanMinimizer::minimize(const string& inputFile, const string& outputFile) {
    if (!readPLA(inputFile)) {      
        std::cerr << "Failed to read PLA file: " << inputFile << std::endl;
        return;
    }
    
    // 輸出原始 minterms 和 don't cares
    cout << "Original minterms: ";
    for (const string& m : minterms) {
        cout << m << " ";
    }
    cout << endl;
    
    cout << "Don't cares: ";
    for (const string& dc : dontCares) {
        cout << dc << " ";
    }
    cout << endl;
    
    quineMcCluskey();
    
    // 輸出所有找到的 prime implicants 用於調試
    cout << "Found " << primeImplicants.size() << " prime implicants:" << endl;
    for (const auto& pi : primeImplicants) {
        cout << "  " << pi.term << " covers: ";
        for (int m : pi.minterms) {
            cout << m << " ";
        }
        cout << endl;
    }
    
    vector<Minterm> solution = petrickMethod();
    writePLA(outputFile, solution);
    
    // 計算 literals
    int totalLiterals = 0;
    for (const auto& term : solution) {
        totalLiterals += (numVars - term.countDashes());
    }
    
    cout << "Total number of terms: " << solution.size() << endl;
    cout << "Total number of literals: " << totalLiterals << endl;
}
