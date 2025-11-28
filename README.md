# ThompsonSubsetEngine

C++ implementation of Thompson's construction and subset construction
to convert regular expressions into NFA and DFA and interactively test strings.

## Features

- Parses regular expressions with:
  - Union (`|`)
  - Concatenation (implicit, made explicit as `.`)
  - Kleene star (`*`)
  - One-or-more (`+`)
  - Optional (`?`)
- Builds an NFA using Thompson's construction
- Converts NFA to DFA using subset construction
- Prints readable NFA and DFA adjacency matrices
- Simulates the DFA step-by-step to accept/reject input strings

## Build & Run

```bash
g++ -std=c++17 -O2 -o regex_automata src/main.cpp
./regex_automata
```
1. You will be prompted to:
Enter a regular expression, e.g.:
```bash 
(a|b)*abb
```
2. See:
  - Preprocessed regex with explicit concatenation
  - Postfix form
  - NFA adjacency matrix
  - DFA adjacency matrix
3. Enter test strings like:
```bash 
abb
abbab
```
and see whether they are ACCEPTED or REJECTED with a step-by-step trace.
#### Theory (High-level)
- Thompson's Construction
  Builds an NFA from a regular expression by composing small NFA fragments
  for literals and operators (|, ., *, +, ?) using ε-transitions.
- Subset Construction
  Converts an NFA to an equivalent DFA by treating each DFA state as a set
  of NFA states and computing transitions via move + ε-closure.
