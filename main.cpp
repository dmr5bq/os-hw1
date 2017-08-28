#include <iostream>
#include <vector>

using namespace std;

void stringToTokens(string& inputStr, string* arrPtr);
int numberOfTokens(string& inputStr);
string getLine();

static string PIPE_CHAR = "|";

int main() {


    while (1) {
        string lineStr = getLine();

        vector<int> commandIndices;
        int tokenCount = numberOfTokens(lineStr);

        string tokenArr[tokenCount];
        stringToTokens(lineStr, tokenArr);

        if (tokenArr->length() > 0) {
            commandIndices.push_back(0);
        }
        for (int i = 0; i < tokenCount + 1; i += 1) {
            if (tokenArr[i] == PIPE_CHAR) {
                if (i != tokenCount - 1) {
                    commandIndices.push_back(i + 1);
                }
                else {
                    // ERROR
                }
            }
        }

        for (int i = 0; i < commandIndices.size(); i++)
            cout << commandIndices[i] << endl;


        // find all pipe operators
        // execute first token and those following pipes

//        read a line of input
//        parse the line
//        for each command in the line {
//                pid = fork();
//                if (pid == 0) {
//                    do redirection stuff
//                        execve ( command, args , ...);
//                    oops, why did exec fail?
//                } else {
//                    store pid somewhere
//                }
//        }
//        for each command in the line {
//                waitpid (stored pid, &status);
//                check return code placed in status;
//        }
        if (lineStr == "exit" || lineStr == "quit") {
            break;
        }
    }
    return 0;
}

// 1. String parsing

string getLine() {
    string inputBuffer;
    cout << "> ";
    getline(cin, inputBuffer);
    return inputBuffer;
}


int numberOfTokens(string& inputStr) {
    char current;
    int count = 1;
    int i = 0;
    string whitespace = " \t";

    // find first and last non-whitespace characters
    size_t beginPosition = inputStr.find_first_not_of(whitespace);
    size_t endPosition = inputStr.find_last_not_of(whitespace);

    i = (int) beginPosition;

    // ensure that there is at least one token; if not, return count is 0
    if (inputStr.length() == 0 || beginPosition == string::npos || endPosition == string::npos) {
        return 0;
    }

    // iterate over string
    while (i < endPosition) {
        current = inputStr[i];

        // check if char is a space or tab
        if (current == whitespace[0] || current == whitespace[1]) {
            // ensure that, if multiple WS characters, the token is not double-counted
            if (i > 0 && inputStr[i-1] != whitespace[0] && inputStr[i-1] != whitespace[1])
            count += 1;
        }
        i += 1;
    }
    return count;
}


void stringToTokens(string &inputStr, string* arrPtr) {
    int i = 0;
    int currIndex = 0;
    string whitespace = " \t";

    // find first and last non-whitespace characters
    size_t beginPosition = inputStr.find_first_not_of(whitespace);
    size_t endPosition = inputStr.find_last_not_of(whitespace);

    i = (int) beginPosition;

    // iterate over string
    while (i < endPosition) {
        // increment i until a non-WS char is found
        while ((inputStr[i] == whitespace[0] || inputStr[i] == whitespace[1])) {
            i += 1;
        }

        // set beginning of token to current pos -- first non-WS char
        int tokenBeginIndex = i;

        // increment i until a WS char is found
        while (inputStr[i] != whitespace[0] && inputStr[i] != whitespace[1]) {
            i += 1;
        }
        // set token end index to current pos -- first WS char
        int tokenEndIndex = i;

        string token = inputStr.substr(tokenBeginIndex, tokenEndIndex - tokenBeginIndex);

        // save the token into the token array
        arrPtr[currIndex] = token;

        // increment the pointer index
        currIndex += 1;
    }
}

// 2. Interpretation of commands

// 3. Link command names to short notation