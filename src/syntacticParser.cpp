#include "global.h"

bool syntacticParse()
{
    logger.log("syntacticParse");
    string possibleQueryType = tokenizedQuery[0];
    // logger.log("tokenizedQuery : "+tokenizedQuery[0]);
    if (tokenizedQuery.size() < 2)
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }

    if (possibleQueryType == "CLEAR")
        return syntacticParseCLEAR();
    else if (possibleQueryType == "INDEX")
        return syntacticParseINDEX();
    else if (possibleQueryType == "LIST")
        return syntacticParseLIST();
    else if (possibleQueryType == "LOAD")
    {
        if (tokenizedQuery.size() == 3 && tokenizedQuery[1] == "MATRIX")
            return syntacticParseLOADMATRIX();
        return syntacticParseLOAD();
    }
    else if (possibleQueryType == "PRINT")
    {
        if (tokenizedQuery.size() == 3 && tokenizedQuery[1] == "MATRIX")
            return syntacticParsePRINTMATRIX();
        return syntacticParsePRINT();
    }
    else if (possibleQueryType == "RENAME")
    {
        if (tokenizedQuery.size() == 4 && tokenizedQuery[1] == "MATRIX")
            return syntacticParseRENAMEMATRIX();
        return syntacticParseRENAME();
    }
    else if(possibleQueryType == "EXPORT")
    {
        if (tokenizedQuery.size() == 3 && tokenizedQuery[1] == "MATRIX")
            return syntacticParseEXPORTMATRIX();
        return syntacticParseEXPORT();
    }
    else if(possibleQueryType == "SOURCE")
        return syntacticParseSOURCE();
    else if(possibleQueryType == "TRANSPOSE" && tokenizedQuery[1] == "MATRIX"){
        return syntacticParseTRANSPOSE();
    }
    else if(possibleQueryType == "CHECKSYMMETRY"){
        return syntacticParseCHECKSYMMETRY();
    }
    else if(possibleQueryType == "COMPUTE"){
        return syntacticParseCOMPUTE();
    }
    else if (possibleQueryType == "SORT")
            return syntacticParseSORT();
    else
    {
        string resultantRelationName = possibleQueryType;
        if (tokenizedQuery[1] != "<-" || tokenizedQuery.size() < 3)
        {
            cout << "SYNTAX ERROR" << endl;
            return false;
        }
        possibleQueryType = tokenizedQuery[2];
        if (possibleQueryType == "PROJECT")
            return syntacticParsePROJECTION();
        else if (possibleQueryType == "SELECT")
            return syntacticParseSELECTION();
        else if (possibleQueryType == "JOIN")
            return syntacticParseJOIN();
        else if (possibleQueryType == "CROSS")
            return syntacticParseCROSS();
        else if (possibleQueryType == "DISTINCT")
            return syntacticParseDISTINCT();
        else if (possibleQueryType == "ORDER" && tokenizedQuery.size() >= 4 && tokenizedQuery[3] == "BY" )
            return syntacticParseORDERBY(resultantRelationName);
        else if (possibleQueryType == "GROUP" && tokenizedQuery.size() >= 4 && tokenizedQuery[3] == "BY" )
            return syntacticParseGROUPBY(resultantRelationName);
        else
        {
            cout << "SYNTAX ERROR" << endl;
            return false;
        }
    }
    return false;
}

ParsedQuery::ParsedQuery()
{
}

void ParsedQuery::clear()
{
    logger.log("ParseQuery::clear");
    this->queryType = UNDETERMINED;

    this->clearRelationName = "";

    this->crossResultRelationName = "";
    this->crossFirstRelationName = "";
    this->crossSecondRelationName = "";

    this->distinctResultRelationName = "";
    this->distinctRelationName = "";

    this->exportRelationName = "";

    this->indexingStrategy = NOTHING;
    this->indexColumnName = "";
    this->indexRelationName = "";

    this->joinBinaryOperator = NO_BINOP_CLAUSE;
    this->joinResultRelationName = "";
    this->joinFirstRelationName = "";
    this->joinSecondRelationName = "";
    this->joinFirstColumnName = "";
    this->joinSecondColumnName = "";

    this->loadRelationName = "";
    
    this->printRelationName = "";

    this->projectionResultRelationName = "";
    this->projectionColumnList.clear();
    this->projectionRelationName = "";

    this->renameFromColumnName = "";
    this->renameToColumnName = "";
    this->renameRelationName = "";

    this->selectType = NO_SELECT_CLAUSE;
    this->selectionBinaryOperator = NO_BINOP_CLAUSE;
    this->selectionResultRelationName = "";
    this->selectionRelationName = "";
    this->selectionFirstColumnName = "";
    this->selectionSecondColumnName = "";
    this->selectionIntLiteral = 0;

    this->sortingStrategy.clear();
    this->sortResultRelationName = "";
    this->sortColumnName.clear();
    this->sortRelationName = "";

    this->sourceFileName = "";
}

/**
 * @brief Checks to see if source file exists. Called when LOAD command is
 * invoked.
 *
 * @param tableName 
 * @return true 
 * @return false 
 */
bool isFileExists(string tableName)
{
    string fileName = "../data/" + tableName + ".csv";
    struct stat buffer;
    return (stat(fileName.c_str(), &buffer) == 0);
}

/**
 * @brief Checks to see if source file exists. Called when SOURCE command is
 * invoked.
 *
 * @param tableName 
 * @return true 
 * @return false 
 */
bool isQueryFile(string fileName){
    fileName = "../data/" + fileName + ".ra";
    struct stat buffer;
    return (stat(fileName.c_str(), &buffer) == 0);
}
