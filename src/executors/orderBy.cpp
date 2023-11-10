#include"global.h"
/**
 * @brief File contains method to process ORDER BY commands.
 * 
 * syntax:
 * <new_table> <- ORDER BY <attribute> ASC|DESC ON <table_name>
 * 
 * 
 */
bool syntacticParseORDERBY(string resultantRelationName){
    logger.log("syntacticParseORDERBY");
    if(tokenizedQuery.size() != 8){
        cout << "SYNTAX ERROR\n";
        return false;
    }

    parsedQuery.resultantRelationName = resultantRelationName; // use this parameter for join and group by as well

    parsedQuery.orderByColumnName = tokenizedQuery[4];

    if(tokenizedQuery[5] != "ASC" && tokenizedQuery[5] != "DESC"){
        cout << "SYNTAX ERROR\n";
        return false;
    }
    else{
        if(tokenizedQuery[5] == "ASC")
            parsedQuery.orderByStrategy = ASC;
        else
            parsedQuery.orderByStrategy = DESC;
    }

    if(tokenizedQuery[6] != "ON"){
        cout << "SYNTAX ERROR\n";
        return false;
    }

    parsedQuery.queryType = ORDERBY;
    parsedQuery.orderByRelationName = tokenizedQuery.back();
    
    return true;
}

bool semanticParseORDERBY(){
    logger.log("semanticParseORDERBY");

    if(tableCatalogue.isTable(parsedQuery.resultantRelationName)){
        cout<<"SEMANTIC ERROR: Resultant relation already exists"<<endl;
        return false;
    }

    if(!tableCatalogue.isTable(parsedQuery.orderByRelationName)){
        cout<<"SEMANTIC ERROR: Relation doesn't exist"<<endl;
        return false;
    }

    if(!tableCatalogue.isColumnFromTable(parsedQuery.orderByColumnName, parsedQuery.orderByRelationName)){
        cout<<"SEMANTIC ERROR: Column doesn't exist in relation"<<endl;
        return false;
    }

    return true;
}

void executeORDERBY(){
    logger.log("executeORDERBY");
    Table *table = tableCatalogue.getTable(parsedQuery.orderByRelationName);
    
    // copy the pages
    table->copyTable(parsedQuery.resultantRelationName);

    Table *sortResultPtr = new Table();
    sortResultPtr->sourceFileName = parsedQuery.resultantRelationName + ".csv";
    sortResultPtr->tableName = parsedQuery.resultantRelationName;
    sortResultPtr->columns = table->columns;
    sortResultPtr->distinctValuesPerColumnCount = table->distinctValuesPerColumnCount;
    sortResultPtr->columnCount = table->columnCount;
    sortResultPtr->rowCount = table->rowCount;
    sortResultPtr->blockCount = table->blockCount;
    sortResultPtr->maxRowsPerBlock = table->maxRowsPerBlock;
    sortResultPtr->rowsPerBlockCount = table->rowsPerBlockCount;
    tableCatalogue.insertTable(sortResultPtr);

    int BUFFERSIZE = 10; // in blocks
    sortResultPtr->sortTable({parsedQuery.orderByColumnName}, {parsedQuery.orderByStrategy}, BUFFERSIZE-1);
	
    return;
}