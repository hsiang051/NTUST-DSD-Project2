
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <fstream>

struct BDDNode {
    std::string variable;
    std::shared_ptr<BDDNode> elseEdge;  // 0的邊
    std::shared_ptr<BDDNode> thenEdge;  // 1的邊
    bool is_terminal;
    BDDNode(const std::string& _variable, std::shared_ptr<BDDNode> _elseEdge, std::shared_ptr<BDDNode> _thenEdge, bool _is_terminal)
        : variable(_variable), elseEdge(_elseEdge), thenEdge(_thenEdge), is_terminal(_is_terminal) {}
};

std::vector<std::shared_ptr<BDDNode>> nodes;
static std::shared_ptr<BDDNode> ZERO = std::make_shared<BDDNode>("0",nullptr, nullptr, true);
static std::shared_ptr<BDDNode> ONE  = std::make_shared<BDDNode>("1", nullptr, nullptr, true);

//build a bdd tree by input variables
std::shared_ptr<BDDNode>buildTreeRecursively(const std::vector<std::string>& variables, size_t depth = 0)
{
    if (depth == variables.size()) {
        return ZERO;
    }

    auto node = std::make_shared<BDDNode>(variables[depth], nullptr, nullptr, false);
    node->elseEdge = buildTreeRecursively(variables, depth + 1);
    node->thenEdge = buildTreeRecursively(variables, depth + 1); 
    nodes.push_back(node); 
    return node;
}
// 將 BDD tree 輸出為 dot 檔
void exportBDDToDot(const std::shared_ptr<BDDNode>& node, std::ofstream& dotFile, std::vector<std::shared_ptr<BDDNode>>& visited) {
    if (!node) return;
    for (auto& n : visited) if (n == node) return;
    visited.push_back(node);

    // 節點
    if (node->is_terminal) {
        dotFile << "  \"" << node.get() << "\" [label=\"" << node->variable << "\", shape=box]\n";
    } else {
        dotFile << "  \"" << node.get() << "\" [label=\"" << node->variable << "\"]\n";
    }
    // elseEdge (0)
    if (node->elseEdge) {
        dotFile << "  \"" << node.get() << "\" -> \"" << node->elseEdge.get() << "\" [label=\"0\", style=dotted]\n";
        exportBDDToDot(node->elseEdge, dotFile, visited);
    }
    // thenEdge (1)
    if (node->thenEdge) {
        dotFile << "  \"" << node.get() << "\" -> \"" << node->thenEdge.get() << "\" [label=\"1\", style=solid]\n";
        exportBDDToDot(node->thenEdge, dotFile, visited);
    }
}

int main() {
    nodes.clear();
    std::vector<std::string> variables = {"a", "b", "c"}; 
    auto root = buildTreeRecursively(variables, 0);

    // 輸出 dot 檔
    std::ofstream dotFile("bdd.dot");
    dotFile << "digraph BDD {\n";
    std::vector<std::shared_ptr<BDDNode>> visited;
    exportBDDToDot(root, dotFile, visited);
    dotFile << "}\n";
    dotFile.close();

    return 0;
}