#include <iostream>
#include <vector>
#include <regex>
#include <unistd.h>
#include <fstream>
#include <fcntl.h>

using namespace std;

/**
 * Author: Dominic Ritchey
 * Email: dmr5bq@virginia.edu
 * Date: 18 September 2017
 * Course: CS 4414 - Operating Systems
 * Assignment: Machine Problem 1 - Shell
 *
 * Included file(s):
 *      msh.cpp
 *      Makefile
 *
 * In order to compile:
 *      $ make
 *
 * Description:
 *      This project is a simple shell as per the project requirements
 *      for CS 4414 at the University of Virginia. All relevant implementation
 *      details can be found in the attached writeup.
 * **/

bool stringToTokens(string& inputStr, string* arrPtr);
int getNumberOfTokens(string& inputStr);
string getLine();
bool fillIndicesOfCommandTokens(vector<int>* commandIndices, string* tokenArr, int tokenCount);
bool isValidToken(string token);

const int    READ_END =     0;      // pipe read-end integer
const int    WRITE_END =    1;      // pipe write-end integer
const string PIPE_CHAR =    "|";    // pipe character token
const string EXIT_CMD =     "exit"; // pre-defined exit command
const int    MAX_LINE_LENGTH = 100; // set length for lines
const int    CHILD_PID =    0;      // pid assigned to a child by fork()
const int    CWD_BUFFER_LENGTH = 100; // max length of buffer used for getting CWD
const string STD_FILE_SEP = "/";      // file separator in Unix
const string STD_INPUT_REDIR_CHAR = "<"; // file input redirection char in Unix
const string STD_OUTPUT_REDIR_CHAR = ">";// file output redirection char in Unix

/* Error messages used below */
const string ERR_MSG_PIPE_ERROR =   "Invalid syntax. Pipe operator must be preceded and followed by a token group.";
const string ERR_MSG_LENGTH =       "Lines of input to the shell must be less than 100 characters in length.";
const string ERR_MSG_REGEX =        "Tokens must be pipes or consist of characters A–Z, a–z, 0–9, dash, dot, >, <, forward slash, and underscore";
const string ERR_MSG_SYS_ERROR =    "Something went wrong when creating a new process.";
const string ERR_MSG_CMD_GRP =      "Not a valid command group";
const string ERR_MSG_IO_DESIGNATED = "A file redirection has already been specified";


int main(int argc, char** argv) {
    ifstream infile;

    if (argc > 1) {
        infile.open(argv[1]);
    }

    loopStart:
    do {
        vector<int>     commandIndices;
        int             numberOfTokens;
        bool            pipeError;
        bool            tokenError;
        vector<pid_t>   pidVec;
        const string    pattern("[A-Za-z0-9-|/._ \t<>]*");
        const regex     re(pattern);
        int             numberOfCommands;
        string          lineStr;
        bool            outputDesignatedFlag = false;
        bool            inputDesignatedFlag = false;

        /* Retrieve a line from a file if run with a file name argument */
        if (argc > 1) {
            char c;
            lineStr = "";
            while ( infile.get(c)  && c != '\n')
                lineStr += c;
        }/* Exit command detected - exit shell */
        else {
            cout << "> ";
            lineStr = getLine();
        }

        /* Exit command detected - exit shell */
        if (lineStr == EXIT_CMD) {
            break;
        }

        /* Line is too long - get next line */
        if (lineStr.length() >= MAX_LINE_LENGTH) {
            cerr << ERR_MSG_LENGTH << endl;
            goto loopStart;
        }

        /* Invalid characters - get next line */
        if (!regex_match(lineStr, re)) {
            cerr << ERR_MSG_REGEX << endl;
            goto loopStart;
        }

        if (argc > 1 && infile.eof())
            exit(0);

        // find the number of tokens
        numberOfTokens = getNumberOfTokens(lineStr);

        // allocate the token array
        string tokenArr[numberOfTokens];

        // fill the token array from the input string
        tokenError = stringToTokens(lineStr, tokenArr);

        // fill the command indices into the vector
        pipeError = fillIndicesOfCommandTokens(&commandIndices, tokenArr, numberOfTokens);

        // if there is a misplaced pipe, cerr the pipe error
        if (pipeError) {
            cerr << ERR_MSG_PIPE_ERROR << endl;
            goto loopStart;
        }

        if (tokenError) {
            cerr << ERR_MSG_REGEX << endl;
            goto loopStart;
        }

        numberOfCommands = (int) commandIndices.size();
        int pipes[numberOfCommands][2];

        for (int commandNumber = 0; commandNumber < numberOfCommands; commandNumber++) {
            vector<string> optionsVector;
            int     numberOfOptions;
            pid_t   pid;
            int     fd[2];
            string iFileName;
            string oFileName;

            const char * commandName = tokenArr[commandIndices[commandNumber]].c_str();
            int  indexOfCommand = commandIndices[commandNumber];

            char cwd[CWD_BUFFER_LENGTH];
            getcwd(cwd, CWD_BUFFER_LENGTH);

            string absPath = cwd;
            absPath += STD_FILE_SEP;
            absPath += commandName;

            string path = ((string) commandName).at(0) == STD_FILE_SEP.at(0) ?
                          commandName : absPath;

            // grab all of the options following commands and put them into a vector
            for (int i = indexOfCommand;
                 tokenArr[i] != PIPE_CHAR && i < numberOfTokens;
                 i += 1) {

                if (tokenArr[i] == STD_INPUT_REDIR_CHAR) {
                    if (commandNumber != 0) {
                        cerr << ERR_MSG_CMD_GRP << endl;
                        goto loopStart;
                    }
                    else if (inputDesignatedFlag) {
                        cerr << ERR_MSG_IO_DESIGNATED << endl;
                        goto loopStart;
                    }
                    else if (i + 1 < numberOfTokens) {
                         if (tokenArr[i + 1] != PIPE_CHAR) {
                            iFileName = tokenArr[i + 1];
                            inputDesignatedFlag = true;
                        } else {
                            cerr << ERR_MSG_CMD_GRP << endl;
                            goto loopStart;
                        }
                        i++;
                    }
                }
                else if (tokenArr[i] == STD_OUTPUT_REDIR_CHAR){
                    if (commandNumber != numberOfCommands - 1) {
                        cerr << ERR_MSG_CMD_GRP << endl;
                        goto loopStart;
                    }
                    else if (outputDesignatedFlag) {
                        cerr << ERR_MSG_IO_DESIGNATED << endl;
                        goto loopStart;
                    }
                    else if (i + 1 < numberOfTokens) {
                        if (tokenArr[i + 1] != PIPE_CHAR) {
                            oFileName = tokenArr[i + 1];
                            outputDesignatedFlag = true;
                        } else {
                            cerr << ERR_MSG_CMD_GRP << endl;
                            goto loopStart;
                        }
                        i++;
                    }
                }
                else {
                    optionsVector.push_back(tokenArr[i]);
                }
            }


            numberOfOptions = (int) optionsVector.size();

            // allocate array to be passed to argv
            const char* argv[numberOfOptions + 1];

            argv[0] = commandName;
            argv[numberOfOptions] =  NULL;

            // convert vector to an array
            for (int i = 1 ; i < numberOfOptions; i += 1) {
                argv[i] = optionsVector[i].c_str();
            }


            pipe(fd);

            pipes[commandNumber][READ_END] = fd[READ_END];
            pipes[commandNumber][WRITE_END] = fd[WRITE_END];

            // start system calls
            pid = fork();

            if (pid == CHILD_PID) {
                if (commandNumber == 0) {
                    if (iFileName.length() > 0) {
                        string absInPath = cwd;
                        absInPath += STD_FILE_SEP;
                        absInPath += iFileName;

                        string inPath = ((string) iFileName).at(0) == STD_FILE_SEP.at(0) ?
                                      iFileName : absInPath;

                        int inFD = open(inPath.c_str(), O_RDONLY);

                        if (inFD == -1) {
                            cerr << strerror(errno) << endl;
                            goto loopStart;
                        }

                        int dupErr = dup2(inFD, STDIN_FILENO);
                        if (dupErr == -1) {
                            cerr << strerror(errno) << endl;
                            goto loopStart;
                        }
                    }
                }
                if (commandNumber > 0) {

                    int newInFD = pipes[commandNumber-1][READ_END];
                    dup2(newInFD, STDIN_FILENO);
                    close(pipes[commandNumber-1][WRITE_END]);
                }
                if (commandNumber < numberOfCommands - 1) {

                    int newOutFD = pipes[commandNumber][WRITE_END];
                    dup2(newOutFD, STDOUT_FILENO);
                    close(pipes[commandNumber][READ_END]);
                }
                if (commandNumber == numberOfCommands - 1) {
                    if (oFileName.length() > 0) {
                        string absOutPath = cwd;
                        absOutPath += STD_FILE_SEP;
                        absOutPath += oFileName;

                        string outPath = ((string) oFileName).at(0) == STD_FILE_SEP.at(0) ?
                                        oFileName : absOutPath;

                        int outFD = open(outPath.c_str(), O_WRONLY|O_CREAT);
                        int dupErr = dup2(outFD, STDOUT_FILENO);
                        if (dupErr == -1) {
                            cerr << strerror(errno) << endl;
                        }
                    }
                }

                int execError = execve(path.c_str(), (char * const *) argv, NULL);

                if (execError == -1) {
                    cerr  << strerror(errno) << endl;
                }

                exit(0);

            }
            else if (pid > 0){
                pidVec.push_back(pid);
            }
            else {
                // print an error
                cerr << ERR_MSG_SYS_ERROR << endl;
                continue;
            }
        }



        for (int i = 0; i < numberOfCommands; i++) {
            close(pipes[i][READ_END]);
            close(pipes[i][WRITE_END]);
        }

        int statuses[numberOfCommands];

        for (int i = 0; i < numberOfCommands; i += 1) {
            waitpid(pidVec[i], &statuses[i], (int) NULL);
        }

        for (int i = 0; i < numberOfCommands; i++) {
            cout << "Process " << tokenArr[commandIndices[i]] << " exited with status code " << statuses[i] << endl;
        }

    } while (true);
    if (argc > 1)
        infile.close();
    exit(0);
}


string getPath(string commandName) {
    // child process
    // get the cwd for file redirects
    char cwd[CWD_BUFFER_LENGTH];
    getcwd(cwd, CWD_BUFFER_LENGTH);

    string absPath = cwd;
    absPath += STD_FILE_SEP;
    absPath += commandName;

    string path = ((string) commandName).at(0) == STD_FILE_SEP.at(0) ?
                  commandName : absPath;

    return path;
}

// 1. String parsing

string getLine() {
    string inputBuffer;
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


bool stringToTokens(string &inputStr, string* arrPtr) {
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
        return true;
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

        if (isValidToken(token))
            // save the token into the token array
            arrPtr[currIndex] = token;
        else
            return true;

        // increment the pointer index
        currIndex += 1;
    }
    return false;
}

bool isValidToken(string token) {
    const string    pattern1("[A-Za-z0-9-._<>/]*");
    const string    pattern2("[|]?");
    const regex     re1(pattern1);
    const regex     re2(pattern2);

    return regex_match(token, re1) || regex_match(token, re2) ;
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
