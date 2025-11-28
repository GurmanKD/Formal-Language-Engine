#include <iostream>
#include <vector>
#include <string>
#include <set>
#include <map>
#include <queue>
#include <stack>
#include <algorithm>
#include <iomanip>
using namespace std;

class AutomatonState {
public:
    vector<int> epsilonMoves;
    map<char, vector<int>> symbolMoves;
};

class NFASegment {
public:
    int startState;
    int endState;

    NFASegment(int start = -1, int end = -1)
        : startState(start), endState(end) {}
};

int main() {
    cout << "Regex to NFA to DFA Converter\n";
    return 0;
}
