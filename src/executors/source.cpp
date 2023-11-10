#include "global.h"
using namespace std;

/**
 * @brief 
 * SYNTAX: SOURCE filename
 */
bool syntacticParseSOURCE()
{
    logger.log("syntacticParseSOURCE");
    if (tokenizedQuery.size() != 2)
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    parsedQuery.queryType = SOURCE;
    parsedQuery.sourceFileName = tokenizedQuery[1];
    return true;
}

bool semanticParseSOURCE()
{
    logger.log("semanticParseSOURCE");
    if (!isQueryFile(parsedQuery.sourceFileName))
    {
        cout << "SEMANTIC ERROR: File doesn't exist" << endl;
        return false;
    }
    return true;
}


void doCommand1()
{
    logger.log("doCommand");
    if (syntacticParse() && semanticParse())
        executeCommand();
    return;
}

void executeSOURCE()
{
    logger.log("executeSOURCE");

    regex delim("[^\\s,]+");
    string command;

    fstream new_file;
    string fileName = "../data/" + parsedQuery.sourceFileName + ".ra";
    logger.log(fileName);
    new_file.open(fileName, ios::in); 
    
    tokenizedQuery.clear();
    parsedQuery.clear();
    logger.log("\nReading New Command: ");
    if (new_file.is_open()) { 
        string sa;
        // logger.log("Open File");
        while (getline(new_file, command)) { 
            tokenizedQuery.clear();
            parsedQuery.clear();
            // logger.log("command");
            auto words_begin = std::sregex_iterator(command.begin(), command.end(), delim);
            auto words_end = std::sregex_iterator();
            for (std::sregex_iterator i = words_begin; i != words_end; ++i)
                tokenizedQuery.emplace_back((*i).str());

            if (tokenizedQuery.size() == 1 && tokenizedQuery.front() == "QUIT")
            {
                break;
            }

            if (tokenizedQuery.empty())
            {
                continue;
            }

            if (tokenizedQuery.size() == 1)
            {
                cout << "SYNTAX ERROR" << endl;
                continue;
            }

            doCommand1(); 
        }
    }
    return;
}