#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cctype>
#include <sstream>
#include <set>
#include <algorithm>
#include <cmath>
#include <unordered_map>
#include <memory>
#include <map>

using namespace std;

std::vector<std::string> input_variables;
std::vector<std::string> product_terms;
std::set<std::string> minterms; // 儲存展開後的 minterms

struct BDDNode {
    std::string variable;
    std::shared_ptr<BDDNode> elseEdge;  // 0的邊
    std::shared_ptr<BDDNode> thenEdge;  // 1的邊
    std::string path;
    bool is_terminal;
    BDDNode(const std::string& _variable, std::shared_ptr<BDDNode> _elseEdge, std::shared_ptr<BDDNode> _thenEdge,std::string _path, bool _is_terminal)
        : variable(_variable), elseEdge(_elseEdge), thenEdge(_thenEdge), path(_path), is_terminal(_is_terminal) {}
};

std::vector<std::shared_ptr<BDDNode>> nodes; // 儲存所有 BDD 節點
static std::shared_ptr<BDDNode> ZERO = std::make_shared<BDDNode>("0",nullptr, nullptr,"", true);
static std::shared_ptr<BDDNode> ONE  = std::make_shared<BDDNode>("1", nullptr, nullptr,"", true);
std::unordered_map<std::shared_ptr<BDDNode>, std::shared_ptr<BDDNode>> reduction_cache;

std::string trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    size_t end = str.find_last_not_of(" \t\r\n");
    return str.substr(start, end - start + 1);
}

bool readPLA(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open file " << filename << std::endl;
        return false;
    }
    
    std::string line;
    
    std::cout << "=== 開始讀取 PLA 檔案 ===" << std::endl;
    
    while (std::getline(file, line)) {
        line = trim(line);
        
        // 跳過空行和註解
        if (line.empty() || line[0] == '#') continue;
        
        // .i 輸入變數數量
        if (line.find(".i ") == 0) {
            std::string num_str = line.substr(3);
            continue;
        }
        
        // .o 輸出變數數量
        if (line.find(".o ") == 0) {
            std::string num_str = line.substr(3);
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
            std::string output_name = line.substr(4);
            continue;
        }
        
        // .p Product Terms 數量
        if (line.find(".p ") == 0) {
            std::string num_str = line.substr(3);
            continue;
        }
        
        // .e 結束
        if (line == ".e") { 
            std::cout << "=== 結束解析 ===" << std::endl;
            break;
        }
        
        // 處理 Product Terms
        size_t space_pos = line.find(' ');
        if (space_pos != std::string::npos) {
            std::string input_part = trim(line.substr(0, space_pos));
            std::string output_part = trim(line.substr(space_pos + 1));
            
            
            // 只保留輸出為1的
            if (output_part == "1") product_terms.push_back(input_part);
        } else {
            std::cout << "發現無法解析的Product Term: 沒有找到空格" << std::endl;
        }
    }
    
    file.close();
    return true;
}

// 展開單一Product Terms為所有對應的 minterms
void expandProductTermRecursive(const std::string& term) {
    if(term.find('-') == std::string::npos) {
        minterms.insert(term);
        return;
    }
    expandProductTermRecursive(term.substr(0, term.find('-')) + '0' + term.substr(term.find('-') + 1));
    expandProductTermRecursive(term.substr(0, term.find('-')) + '1' + term.substr(term.find('-') + 1));
}

// 展開所有Product Terms
void expandAllProductTerms() {
    minterms.clear();
    
    for (const std::string& term : product_terms) {
        expandProductTermRecursive(term);
    }
}


std::shared_ptr<BDDNode> buildBDDTreeRecursive(const std::vector<std::string>& variables, size_t depth = 0, std::string path = "") {
    if (depth == variables.size()) {
        if (minterms.find(path) != minterms.end()) {
            return ONE;
        }
        return ZERO;
    }

    auto node = std::make_shared<BDDNode>(variables[depth], nullptr, nullptr, path, false);
    node->elseEdge = buildBDDTreeRecursive(variables, depth + 1, path + "0");
    node->thenEdge = buildBDDTreeRecursive(variables, depth + 1, path + "1"); 
    nodes.push_back(node); 
    return node;
}

// ROBDD 化簡函數
std::shared_ptr<BDDNode> reduceToROBDD(std::shared_ptr<BDDNode> node) {
    if (!node) return nullptr;
    
    // 終端節點不需要化簡
    if (node->is_terminal) return node;
    
    // 檢查快取，避免重複化簡
    auto cache_it = reduction_cache.find(node);
    if (cache_it != reduction_cache.end()) {
        return cache_it->second;
    }
    
    auto reduced_else = reduceToROBDD(node->elseEdge);
    auto reduced_then = reduceToROBDD(node->thenEdge);
    
    // 兩邊指向同一個節點，回傳其中一個(else)
    if (reduced_else == reduced_then) {
        reduction_cache[node] = reduced_else;
        return reduced_else;
    }
    
    // 檢查是否已經有相同變數和相同子節點的節點存在
    static std::map<std::tuple<std::string, std::shared_ptr<BDDNode>, std::shared_ptr<BDDNode>>, std::shared_ptr<BDDNode>> unique_table;
    
    auto key = std::make_tuple(node->variable, reduced_else, reduced_then);
    auto unique_it = unique_table.find(key);
    
    if (unique_it != unique_table.end()) {
        // 找到，返回已存在的節點
        reduction_cache[node] = unique_it->second;
        return unique_it->second;
    }
    
    // 建立新的 化簡後的節點
    auto reduced_node = std::make_shared<BDDNode>(node->variable, reduced_else, reduced_then, node->path, false);
    
    unique_table[key] = reduced_node;
    reduction_cache[node] = reduced_node;
    
    return reduced_node;
}

void exportBDDToDot(const std::shared_ptr<BDDNode>& node, std::ostringstream& dotFile, std::vector<std::shared_ptr<BDDNode>>& visited, std::map<std::string, std::vector<std::shared_ptr<BDDNode>>>& ranks) {
    if (!node) return;
    for (auto& n : visited) if (n == node) return;
    visited.push_back(node);

    // 將節點加入對應的rank
    if(node->variable != "0" && node->variable != "1")
        ranks[node->variable].push_back(node);

    // 節點
    if (node->is_terminal) {
        dotFile << "  \"" << node.get() << "\" [label=\"" << node->variable << "\", shape=box]\n";
    } else {
        dotFile << "  \"" << node.get() << "\" [label=\"" << node->variable << "\"]\n";
    }
    // elseEdge (0)
    if (node->elseEdge) {
        dotFile << "  \"" << node.get() << "\" -> \"" << node->elseEdge.get() << "\" [label=\"0\", style=dotted]\n";
        exportBDDToDot(node->elseEdge, dotFile, visited, ranks);
    }
    // thenEdge (1)
    if (node->thenEdge) {
        dotFile << "  \"" << node.get() << "\" -> \"" << node->thenEdge.get() << "\" [label=\"1\", style=solid]\n";
        exportBDDToDot(node->thenEdge, dotFile, visited, ranks);
    }
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cout << "使用方法: " << argv[0] << " <input.pla> <output.dot>" << std::endl;
        std::cout << "範例: " << argv[0] << " input.pla output.dot" << std::endl;
        return 1;
    }
    
    if (!readPLA(argv[1])) {
        return 1;
    }
    
    expandAllProductTerms();

    std::cout << "=== 正在建立BDD Tree ===" << std::endl;
    auto root = buildBDDTreeRecursive(input_variables, 0, "");
    
    std::cout << "=== 正在進行化簡 ===" << std::endl;
    root = reduceToROBDD(root);

    // 輸出 dot 檔
    std::ofstream dotFile(argv[2]);
    std::ostringstream dotContent;
    dotFile << "digraph BDD {\n";
    std::vector<std::shared_ptr<BDDNode>> visited;
    std::map<std::string, std::vector<std::shared_ptr<BDDNode>>> ranks;
    
    std::cout << "=== 正在轉換成DOT ===" << std::endl;
    exportBDDToDot(root, dotContent, visited, ranks);
    
    // 輸出 rank 設定
    std::cout << "=== 正在處理RANK ===" << std::endl;
    for (const auto& [variable, nodeList] : ranks) {
        dotFile << "  {rank=same ";
        for (const auto& node : nodeList) {
            dotFile << "\"" << node.get() << "\" ";
        }
        dotFile << "}\n";
    }

    dotFile << dotContent.str();
    
    dotFile << "}\n";
    dotFile.close();
    std::cout << "=== DOT檔輸出完成 ===" << std::endl;

    return 0;
}