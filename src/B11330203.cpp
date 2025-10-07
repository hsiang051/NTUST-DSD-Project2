#include <iostream>
#include "BooleanMinimizer.hpp"

using namespace std;

int main(int argc, char* argv[]) {
    if (argc != 3) {
        cerr << "Usage: " << argv[0] << " PLA_IN_FILE PLA_OUT_FILE" << endl;
        return 1;
    }
    
    BooleanMinimizer minimizer;
    minimizer.minimize(argv[1], argv[2]);

    return 0;
}