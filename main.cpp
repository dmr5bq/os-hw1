#include <iostream>
#include <vector>
#include <unistd.h>


using namespace std;

void stringToTokens(string& inputStr, string* arrPtr);
int numberOfTokens(string& inputStr);
string getLine();
bool fillIndicesOfCommandTokens(vector<int>* commandIndices, string* tokenArr, int tokenCount);


static string PIPE_CHAR = "|";

static string ERR_MSG_PIPE_ERROR = "Invalid syntax. Pipe operator must be preceded and followed by a token group.";

int main() {

    while (1) {
        vector<int> commandIndices;
        int tokenCount;
        bool pipeError;
        vector<pid_t> pidVec;

        // read in a line
        string lineStr = getLine();

        // find the number of tokens
        tokenCount = numberOfTokens(lineStr);

        // allocate the token array
        string tokenArr[tokenCount];

        // fill the token array from the input string
        stringToTokens(lineStr, tokenArr);

        // fill the command indices into the vector
        pipeError = fillIndicesOfCommandTokens(&commandIndices, tokenArr, tokenCount);

        // if there is a misplaced pipe, cerr the pipe error
        if (pipeError) {
            cerr << ERR_MSG_PIPE_ERROR << endl;
        }

        // execute first token and those following pipes

        for (int commandNumber = 0; commandNumber < commandIndices.size(); commandNumber += 1) {
           pid_t pid = fork();
            string commandName = tokenArr[commandIndices[commandNumber]];

//            vector<string> optionsVec;
//            int tokenInd = commandIndices[commandNumber];
//            while (tokenInd < tokenArr->length() && tokenArr[tokenInd] != PIPE_CHAR) {
//                cout << tokenArr[tokenInd] << endl;
//                optionsVec.push_back(tokenArr[tokenInd]);
//                tokenInd++;
//            }



            if (pid == 0) {

                // redirection
                // execve
                //execve(commandName.c_str(), optionsArr, NULL);
            } else {
                vector<string> optionsVec;
                int tokenInd = commandIndices[commandNumber];
                while (tokenInd < tokenArr->length() && tokenArr[tokenInd] != PIPE_CHAR) {
                    cout << tokenArr[tokenInd] << endl;
                    optionsVec.push_back(tokenArr[tokenInd]);
                    tokenInd++;
                }

                pidVec.push_back(pid);
            }
        }
        for (int commandNumber = 0; commandNumber < commandIndices.size(); commandNumber += 1) {
            int status;
            waitpid(pidVec[commandNumber], &status, WNOHANG);
        }

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
        if (lineStr == "exit") {
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

    if (beginPosition == endPosition && inputStr[beginPosition] != whitespace[0]
                                     && inputStr[beginPosition] != whitespace[1]) {
        arrPtr[0] = inputStr[beginPosition];
        return;
    }

    // iterate over string
    while (i < endPosition) {
        // increment i until a non-WS char is found
        while ((inputStr[i] == whitespace[0] || inputStr[i] == whitespace[1])) {
            i += 1;
        }

        // set beginning of token to current pos -- first non-WS char
        int tokenBeginIndex = i;

        // increment i until a WS char is found
        while (inputStr[i] != whitespace[0] && inputStr[i] != whitespace[1] && i <= endPosition) {
            i += 1;
        }
        // set token end index to current pos -- first WS char
        int tokenEndIndex = i;

        string token = inputStr.substr((unsigned long) tokenBeginIndex, (unsigned long) (tokenEndIndex - tokenBeginIndex));

        // save the token into the token array
        arrPtr[currIndex] = token;

        // increment the pointer index
        currIndex += 1;
    }
}

bool fillIndicesOfCommandTokens(vector<int>* commandIndices, string* tokenArr, int tokenCount) {
    // the first token must be a command
    if (tokenArr->length() > 0) {
        commandIndices->push_back(0);
    }
    for (int i = 0; i < tokenCount + 1; i += 1) {
        if (tokenArr[i] == PIPE_CHAR) {
            // ensure the pipe is not the last token
            if (i != tokenCount - 1 && i != 0) {
                // add the index to the command indices
                commandIndices->push_back(i + 1);
            }
            else {
                // true if error
                return true;
            }
        }
    }
    // if no error, false
    return false;
}

// 2. Interpretation of commands

// 3. Link command names to short notation