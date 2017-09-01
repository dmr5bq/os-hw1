#include <iostream>
#include <vector>
#include <regex>
#include <unistd.h>


using namespace std;

void stringToTokens(string& inputStr, string* arrPtr);
int getNumberOfTokens(string& inputStr);
string getLine();
bool fillIndicesOfCommandTokens(vector<int>* commandIndices, string* tokenArr, int tokenCount);


static const string PIPE_CHAR = "|";
static const string EXIT_CMD = "exit";
static const int    MAX_LINE_LENGTH = 100;
static const int    CHILD_PID = 0;
static const int    CWD_BUFFER_LENGTH = 100;
static const string STD_FILE_SEP = "/";

static const string ERR_MSG_PIPE_ERROR = "Invalid syntax. Pipe operator must be preceded and followed by a token group.";
static const string ERR_MSG_LENGTH = "Lines of input to the shell must be less than 100 characters in length.";
static const string ERR_MSG_REGEX = "Lines must consist of characters A–Z, a–z, 0–9, dash, dot, forward slash, and underscore";
static const string ERR_MSG_SYS_ERROR = "Something went wrong when creating a new process.";

int main() {

    while (true) {
        vector<int>     commandIndices;
        int             numberOfTokens;
        bool            pipeError;
        vector<pid_t>   pidVec;
        const string    pattern("[A-Za-z0-9-|/._ \t<>]*");
        const regex     re(pattern);
        int             numberOfCommands;

        // read in a line
        string lineStr = getLine();

        if (lineStr == EXIT_CMD) {
            break;
        }

        // catch too-long case
        if (lineStr.length() >= MAX_LINE_LENGTH) {
            cerr << ERR_MSG_LENGTH << endl;
            continue;
        }

        // catch invalid character case
        if (!regex_match(lineStr, re)) {
            cerr << ERR_MSG_REGEX << endl;
            continue;
        }

        // find the number of tokens
        numberOfTokens = getNumberOfTokens(lineStr);

        // allocate the token array
        string tokenArr[numberOfTokens];

        // fill the token array from the input string
        stringToTokens(lineStr, tokenArr);

        // fill the command indices into the vector
        pipeError = fillIndicesOfCommandTokens(&commandIndices, tokenArr, numberOfTokens);

        // if there is a misplaced pipe, cerr the pipe error
        if (pipeError) {
            cerr << ERR_MSG_PIPE_ERROR << endl;
            continue;
        }

        numberOfCommands = (int) commandIndices.size();

        for (int commandNumber = 0; commandNumber < numberOfCommands; commandNumber++) {
            vector<string> optionsVector;
            int     numberOfOptions;
            pid_t   pid;

            string   commandName = tokenArr[commandIndices[commandNumber]];
            int      indexOfCommand = commandIndices[commandNumber];

            // grab all of the options following commands and put them into a vector
            for (int i = indexOfCommand + 1;
                 tokenArr[i] != PIPE_CHAR && i < numberOfTokens;
                 i += 1) {
                optionsVector.push_back(tokenArr[i]);
            }

            numberOfOptions = (int) optionsVector.size();

            // allocate array to be passed to argv
            string optionsArray[numberOfOptions + 1];

            // set argv[0] = commandName
            optionsArray[0] = commandName;

            // convert vector to an aray
            for (int i = 1 ; i < numberOfOptions; i += 1) {
                optionsArray[i] = optionsVector[i];
            }

            // start system calls
            pid = fork();

            if (pid == CHILD_PID) {
                // child process
                // get the cwd for file redirects
                char cwd[CWD_BUFFER_LENGTH];
                getcwd(cwd, CWD_BUFFER_LENGTH);

                string path = commandName.at(0) == STD_FILE_SEP.at(0) ?
                              commandName : cwd + STD_FILE_SEP + commandName;


                execve(path, optionsArray, )
            }
            else if (pid > 0){
                // parent process
            }
            else {
                // print an error
                cerr << ERR_MSG_SYS_ERROR << endl;
                continue;
            }

        }

        // execute first token and those following pipes

//        for (int commandNumber = 0; commandNumber < commandIndices.size(); commandNumber += 1) {
//           pid_t pid = fork();
//            string commandName = tokenArr[commandIndices[commandNumber]];
//
//            if (pid == 0) {
//
//                // redirection
//                // execve
//                //execve(commandName.c_str(), optionsArr, NULL);
//            } else {
//                vector<string> optionsVec;
//                int tokenInd = commandIndices[commandNumber];
//                while (tokenInd < tokenArr->length() && tokenArr[tokenInd] != PIPE_CHAR) {
//                    cout << tokenArr[tokenInd] << endl;
//                    optionsVec.push_back(tokenArr[tokenInd]);
//                    tokenInd++;
//                }
//
//                pidVec.push_back(pid);
//            }
//        }
//        for (int commandNumber = 0; commandNumber < commandIndices.size(); commandNumber += 1) {
//            int status;
//            waitpid(pidVec[commandNumber], &status, WNOHANG);
//        }

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


int getNumberOfTokens(string& inputStr) {
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