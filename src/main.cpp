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

class NFABuilder {
public:
    // List of all NFA states created
    vector<AutomatonState> stateList;

    // Create a new state and return its index
    int createState() {
        stateList.push_back(AutomatonState());
        return static_cast<int>(stateList.size()) - 1;
    }

    // Build NFA from postfix regular expression using Thompson's construction
    NFASegment constructFromPostfix(const string& postfix) {
        stack<NFASegment> fragments;

        for (char token : postfix) {

            // 1) Literal symbol: create start ->(symbol)-> end
            if (!isRegexOperator(token)) {
                int start = createState();
                int end   = createState();

                stateList[start].symbolMoves[token].push_back(end);
                fragments.push(NFASegment(start, end));
            }

            // 2) Concatenation: A.B  => connect A.end ε→ B.start
            else if (token == '.') {
                NFASegment second = fragments.top(); fragments.pop();
                NFASegment first  = fragments.top(); fragments.pop();

                stateList[first.endState].epsilonMoves.push_back(second.startState);
                fragments.push(NFASegment(first.startState, second.endState));
            }

            // 3) Union: A|B  => newStart ε→ A.start, B.start; A.end,B.end ε→ newEnd
            else if (token == '|') {
                NFASegment second = fragments.top(); fragments.pop();
                NFASegment first  = fragments.top(); fragments.pop();

                int newStart = createState();
                int newEnd   = createState();

                stateList[newStart].epsilonMoves.push_back(first.startState);
                stateList[newStart].epsilonMoves.push_back(second.startState);

                stateList[first.endState].epsilonMoves.push_back(newEnd);
                stateList[second.endState].epsilonMoves.push_back(newEnd);

                fragments.push(NFASegment(newStart, newEnd));
            }

            // 4) Kleene star: A*  => zero or more repetitions
            // newStart ε→ A.start, newEnd
            // A.end ε→ A.start, newEnd
            else if (token == '*') {
                NFASegment frag = fragments.top(); fragments.pop();

                int newStart = createState();
                int newEnd   = createState();

                stateList[newStart].epsilonMoves.push_back(frag.startState);
                stateList[newStart].epsilonMoves.push_back(newEnd);

                stateList[frag.endState].epsilonMoves.push_back(frag.startState);
                stateList[frag.endState].epsilonMoves.push_back(newEnd);

                fragments.push(NFASegment(newStart, newEnd));
            }

            // 5) One-or-more: A+  => at least one repetition
            // newStart ε→ A.start
            // A.end ε→ A.start, newEnd
            else if (token == '+') {
                NFASegment frag = fragments.top(); fragments.pop();

                int newStart = createState();
                int newEnd   = createState();

                stateList[newStart].epsilonMoves.push_back(frag.startState);

                stateList[frag.endState].epsilonMoves.push_back(frag.startState);
                stateList[frag.endState].epsilonMoves.push_back(newEnd);

                fragments.push(NFASegment(newStart, newEnd));
            }

            // 6) Optional: A?  => zero or one occurrence
            // newStart ε→ A.start, newEnd
            // A.end ε→ newEnd
            else if (token == '?') {
                NFASegment frag = fragments.top(); fragments.pop();

                int newStart = createState();
                int newEnd   = createState();

                stateList[newStart].epsilonMoves.push_back(frag.startState);
                stateList[newStart].epsilonMoves.push_back(newEnd);

                stateList[frag.endState].epsilonMoves.push_back(newEnd);

                fragments.push(NFASegment(newStart, newEnd));
            }
        }

        // Final NFA fragment on stack represents the whole regex
        return fragments.top();
    }
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
