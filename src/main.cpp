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

string insertConcatenation(const string& regex) {
    string result;

    auto isLiteral = [](char ch) {
        return ch != '|' && ch != '*' && ch != '+' &&
               ch != '?' && ch != '(' && ch != ')' && ch != '.';
    };

    for (size_t i = 0; i < regex.length(); i++) {
        result += regex[i];

        if (i + 1 < regex.length()) {
            char curr = regex[i];
            char next = regex[i + 1];

            bool needConcat =
                (isLiteral(curr) || curr == ')' || curr == '*' || curr == '+' || curr == '?') &&
                (isLiteral(next) || next == '(');

            if (needConcat)
                result += '.';
        }
    }

    return result;
}

int getOperatorPrecedence(char op) {
    if (op == '*' || op == '+' || op == '?') return 3;
    if (op == '.') return 2;
    if (op == '|') return 1;
    return 0;
}

bool isRegexOperator(char ch) {
    return ch == '|' || ch == '.' || ch == '*' || ch == '+' || ch == '?';
}

string convertToPostfix(const string& infix) {
    string output;
    stack<char> operators;

    for (char ch : infix) {

        // If operand (literal), add directly to output
        if (!isRegexOperator(ch) && ch != '(' && ch != ')') {
            output += ch;
        }

        // Left parenthesis
        else if (ch == '(') {
            operators.push(ch);
        }

        // Right parenthesis: pop until '('
        else if (ch == ')') {
            while (!operators.empty() && operators.top() != '(') {
                output += operators.top();
                operators.pop();
            }
            operators.pop(); // Remove '('
        }

        // Operator encountered
        else {
            // Unary operators (*, +, ?) → right associative
            if (ch == '*' || ch == '+' || ch == '?') {
                while (!operators.empty() &&
                       getOperatorPrecedence(operators.top()) >
                       getOperatorPrecedence(ch)) {
                    output += operators.top();
                    operators.pop();
                }
            }

            // Binary operators (|, .) → left associative
            else {
                while (!operators.empty() &&
                       getOperatorPrecedence(operators.top()) >=
                       getOperatorPrecedence(ch)) {
                    output += operators.top();
                    operators.pop();
                }
            }

            operators.push(ch);
        }
    }

    // Pop remaining operators
    while (!operators.empty()) {
        output += operators.top();
        operators.pop();
    }

    return output;
}

int main() {
    cout << "Regex to NFA to DFA Converter\n";
    return 0;
}
