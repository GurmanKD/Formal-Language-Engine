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

set<int> computeEpsilonClosure(const vector<AutomatonState>& nfa,
                               const set<int>& initialStates) {
    set<int> closure = initialStates;
    stack<int> workStack;

    // Initialize stack with the starting states
    for (int state : initialStates) {
        workStack.push(state);
    }

    // DFS/BFS over epsilon transitions
    while (!workStack.empty()) {
        int current = workStack.top();
        workStack.pop();

        for (int target : nfa[current].epsilonMoves) {
            if (closure.find(target) == closure.end()) {
                closure.insert(target);
                workStack.push(target);
            }
        }
    }

    return closure;
}

set<int> computeMove(const vector<AutomatonState>& nfa,
                     const set<int>& states,
                     char symbol) {
    set<int> result;

    for (int state : states) {
        auto it = nfa[state].symbolMoves.find(symbol);
        if (it != nfa[state].symbolMoves.end()) {
            // Insert all target states for this symbol
            result.insert(it->second.begin(), it->second.end());
        }
    }

    return result;
}

string stateSetToString(const set<int>& states) {
    string key;
    for (int s : states) {
        key += to_string(s);
        key += ",";  // delimiter
    }
    return key;
}

void printNFAMatrix(const vector<AutomatonState>& nfa,
                    const set<char>& alphabet,
                    int start,
                    int accept) {
    cout << "\n========== NFA ADJACENCY MATRIX ==========\n";
    cout << "Start State: " << start << " | Accept State: " << accept << "\n\n";

    int n = static_cast<int>(nfa.size());
    vector<char> symbols(alphabet.begin(), alphabet.end());

    // Header
    cout << setw(8) << "State";
    for (char c : symbols) {
        cout << setw(6) << c;
    }
    cout << setw(8) << "epsilon\n";

    cout << string(8 + static_cast<int>(symbols.size()) * 6 + 8, '-') << "\n";

    // Rows
    for (int i = 0; i < n; i++) {
        cout << setw(8) << i;

        // For each symbol, print first destination or '-'
        for (char c : symbols) {
            auto it = nfa[i].symbolMoves.find(c);
            if (it != nfa[i].symbolMoves.end() && !it->second.empty()) {
                cout << setw(6) << it->second[0];
            } else {
                cout << setw(6) << "-";
            }
        }

        // Epsilon moves
        if (!nfa[i].epsilonMoves.empty()) {
            cout << setw(8) << "{";
            for (size_t j = 0; j < nfa[i].epsilonMoves.size(); j++) {
                cout << nfa[i].epsilonMoves[j];
                if (j + 1 < nfa[i].epsilonMoves.size()) cout << ",";
            }
            cout << "}";
        } else {
            cout << setw(8) << "-";
        }

        cout << "\n";
    }
}

void printDFAMatrix(const vector<set<int>>& dfaStates,
                    const vector<map<char, int>>& dfaTrans,
                    const set<char>& alphabet,
                    const set<int>& acceptStates) {
    cout << "\n========== DFA ADJACENCY MATRIX ==========\n";
    cout << "Start State: 0 | Accept States: {";
    for (int acc : acceptStates) {
        cout << acc << " ";
    }
    cout << "}\n\n";

    vector<char> symbols(alphabet.begin(), alphabet.end());

    // Header
    cout << setw(10) << "State";
    for (char c : symbols) {
        cout << setw(8) << c;
    }
    cout << "\n";

    cout << string(10 + static_cast<int>(symbols.size()) * 8, '-') << "\n";

    // Rows
    for (size_t i = 0; i < dfaStates.size(); i++) {
        cout << setw(10) << i;

        for (char c : symbols) {
            auto it = dfaTrans[i].find(c);
            if (it != dfaTrans[i].end()) {
                cout << setw(8) << it->second;
            } else {
                cout << setw(8) << "-";
            }
        }

        cout << "\n";
    }
}

bool testStringOnDFA(const string& testStr,
                     const vector<map<char, int>>& dfaTrans,
                     const set<int>& acceptStates) {
    cout << "\n========== STRING MATCHING: \"" << testStr << "\" ==========\n";

    int currentState = 0;
    cout << "Step-by-step execution:\n";
    cout << "Initial State: " << currentState << "\n";

    for (size_t i = 0; i < testStr.length(); i++) {
        char symbol = testStr[i];

        auto it = dfaTrans[currentState].find(symbol);
        if (it == dfaTrans[currentState].end()) {
            cout << "Step " << (i + 1) << ": Read '" << symbol
                 << "' from State " << currentState
                 << " -> STUCK (No transition)\n";
            cout << "Result: REJECTED\n";
            return false;
        }

        int nextState = it->second;
        cout << "Step " << (i + 1) << ": Read '" << symbol
             << "' from State " << currentState
             << " -> State " << nextState << "\n";

        currentState = nextState;
    }

    bool accepted = (acceptStates.count(currentState) > 0);
    cout << "Final State: " << currentState << "\n";
    cout << "Result: " << (accepted ? "ACCEPTED" : "REJECTED")
         << " (Final state is " << (accepted ? "" : "NOT ")
         << "an accept state)\n";

    return accepted;
}

int main() {
    // Fancy banner
    cout <<
        "╔═══════════════════════════════════════════════════════╗\n";
    cout <<
        "║ REGEX TO NFA TO DFA CONVERTER WITH STRING MATCHING   ║\n";
    cout <<
        "║ Using Thompson's & Subset Construction               ║\n";
    cout <<
        "╚═══════════════════════════════════════════════════════╝\n\n";

    // 1. Read regex
    cout << "Enter regular expression: ";
    string regex;
    getline(cin, regex);

    // Remove spaces (optional)
    regex.erase(remove(regex.begin(), regex.end(), ' '), regex.end());

    // 2. Preprocess and convert to postfix
    cout << "\n========== PREPROCESSING ==========\n";
    string withConcat = insertConcatenation(regex);
    cout << "Original Regex: " << regex << "\n";
    cout << "With Concatenation: " << withConcat << "\n";

    string postfix = convertToPostfix(withConcat);
    cout << "Postfix Notation: " << postfix << "\n";

    // 3. NFA construction (Thompson's)
    cout << "\n========== NFA CONSTRUCTION (Thompson's) ==========\n";

    NFABuilder builder;
    NFASegment nfaResult = builder.constructFromPostfix(postfix);

    int nfaStart  = nfaResult.startState;
    int nfaAccept = nfaResult.endState;

    vector<AutomatonState> nfa = builder.stateList;

    // Build alphabet (all non-operator symbols in postfix)
    set<char> alphabet;
    for (char ch : postfix) {
        if (!isRegexOperator(ch) && ch != '(' && ch != ')') {
            alphabet.insert(ch);
        }
    }

    cout << "Total NFA States: " << nfa.size() << "\n";
    cout << "Start State: " << nfaStart << "\n";
    cout << "Accept State: " << nfaAccept << "\n";
    cout << "Alphabet: {";
    for (char ch : alphabet) cout << ch << " ";
    cout << "}\n";

    printNFAMatrix(nfa, alphabet, nfaStart, nfaAccept);

    // 4. DFA construction (Subset Construction)
    cout << "\n========== DFA CONSTRUCTION (Subset Construction) ==========\n";

    map<string, int> stateMapping;              // "set of NFA states" -> DFA state id
    vector<set<int>> dfaStateList;              // DFA states as sets of NFA states
    vector<map<char, int>> dfaTransitions;      // transitions for each DFA state
    queue<int> processingQueue;
    set<int> dfaAcceptStates;                   // we'll fill this later

    // Initial DFA state = ε-closure({nfaStart})
    set<int> initialSet = { nfaStart };
    set<int> startClosure = computeEpsilonClosure(nfa, initialSet);

    stateMapping[stateSetToString(startClosure)] = 0;
    dfaStateList.push_back(startClosure);
    dfaTransitions.push_back(map<char, int>());
    processingQueue.push(0);

    // BFS over DFA states
    while (!processingQueue.empty()) {
        int currentDFA = processingQueue.front();
        processingQueue.pop();

        const set<int>& currentStates = dfaStateList[currentDFA];

        for (char symbol : alphabet) {
            // Move on symbol from NFA states in this DFA state
            set<int> afterMove = computeMove(nfa, currentStates, symbol);
            if (afterMove.empty()) continue;

            // ε-closure of the move result
            set<int> closure = computeEpsilonClosure(nfa, afterMove);
            string key = stateSetToString(closure);

            // If this set not seen before, create new DFA state
            if (stateMapping.find(key) == stateMapping.end()) {
                int newStateID = static_cast<int>(dfaStateList.size());
                stateMapping[key] = newStateID;
                dfaStateList.push_back(closure);
                dfaTransitions.push_back(map<char, int>());
                processingQueue.push(newStateID);
            }

            int targetDFA = stateMapping[key];
            dfaTransitions[currentDFA][symbol] = targetDFA;
        }
    }

    // Determine DFA accept states (any DFA state containing nfaAccept)
    for (size_t i = 0; i < dfaStateList.size(); i++) {
        if (dfaStateList[i].count(nfaAccept)) {
            dfaAcceptStates.insert(static_cast<int>(i));
        }
    }

    cout << "Total DFA States: " << dfaStateList.size() << "\n";
    cout << "Start State: 0\n";
    cout << "Accept States: {";
    for (int acc : dfaAcceptStates) cout << acc << " ";
    cout << "}\n";

    printDFAMatrix(dfaStateList, dfaTransitions, alphabet, dfaAcceptStates);

    // 5. String testing
    cout << "\n========== STRING TESTING ==========\n";
    cout << "Enter number of test strings: ";
    int numTests;
    cin >> numTests;
    cin.ignore(); // clear newline

    for (int i = 0; i < numTests; i++) {
        cout << "\nTest " << (i + 1) << " - Enter string: ";
        string testStr;
        getline(cin, testStr);

        testStringOnDFA(testStr, dfaTransitions, dfaAcceptStates);
    }

    cout <<
        "\n╔═══════════════════════════════════════════════════════╗\n";
    cout <<
        "║ PROCESSING COMPLETE                                  ║\n";
    cout <<
        "╚═══════════════════════════════════════════════════════╝\n";
    cout << "\n========== STRING TESTING ==========\n";
    cout << "Enter number of test strings: ";

    int numTests;
    cin >> numTests;
    cin.ignore(); // flush newline

    for (int i = 0; i < numTests; i++) {
        cout << "\nTest " << (i + 1) << " - Enter string: ";
        string testStr;
        getline(cin, testStr);

        testStringOnDFA(testStr, dfaTransitions, dfaAcceptStates);
    }
    return 0;
}
