#include"global.h"
/**
 * @brief File contains method to process GROUP BY commands.
 * 
 * syntax:
 * <new_table> <- GROUP BY <attribute> ASC|DESC ON <table_name>
 * 
 * 
 */
bool syntacticParseGROUPBY(string resultantRelationName){
    logger.log("syntacticParseGROUPBY");
    if(tokenizedQuery.size() != 13){
        cout << "SYNTAX ERROR\n";
        return false;
    }

    parsedQuery.resultantRelationName = resultantRelationName; // use this parameter for join and group by as well

    parsedQuery.groupByColumnName = tokenizedQuery[4];

    if(tokenizedQuery[5] != "FROM"){
        cout << "SYNTAX ERROR\n";
        return false;
    }

    parsedQuery.groupByRelationName = tokenizedQuery[6];

    if(tokenizedQuery[7] != "HAVING"){
        cout << "SYNTAX ERROR\n";
        return false;
    }

    if(tokenizedQuery[8][3] != '(' || tokenizedQuery[8].back() != ')'){
        cout << "SYNTAX ERROR\n";
        return false;
    }
    else{
        string aggregateFunction;
        for(int i=0; i<3; i++)
            aggregateFunction += tokenizedQuery[8][i];

        if(aggregateFunction == "MIN"){
            parsedQuery.conditionAggregateFunction = MIN;
        }    
        else if(aggregateFunction == "MAX"){
            parsedQuery.conditionAggregateFunction = MAX;
        }     
        else if(aggregateFunction == "SUM"){
            parsedQuery.conditionAggregateFunction = SUM;
        }     
        else if(aggregateFunction == "AVG"){
            parsedQuery.conditionAggregateFunction = AVG;
        }
        else{
            cout << "SYNTAX ERROR\n";
            return false;
        }

        string conditionColumnName;
        for(int i=4; i<(int)tokenizedQuery[8].size() - 1; i++){
            conditionColumnName += tokenizedQuery[8][i];
        }
        parsedQuery.conditionAggregateColumn = conditionColumnName;
    }


    if(tokenizedQuery[9] == "<" || tokenizedQuery[9] == ">" || tokenizedQuery[9] == "<=" || tokenizedQuery[9] == ">=" || tokenizedQuery[9] == "==" || tokenizedQuery[9] == "!="){
        parsedQuery.conditionOperator = tokenizedQuery[9];
    }
    else{
        cout << "SYNTAX ERROR\n";
        return false;
    }
    
    parsedQuery.conditionAttributeValue = stoi(tokenizedQuery[10]);

    if(tokenizedQuery[11] != "RETURN"){
        cout << "SYNTAX ERROR\n";
        return false;
    }

    if(tokenizedQuery[12][3] != '(' || tokenizedQuery[12].back() != ')'){
        cout << "SYNTAX ERROR\n";
        return false;
    }
    else{
        string aggregateFunction;
        for(int i=0; i<3; i++)
            aggregateFunction += tokenizedQuery[12][i];

        if(aggregateFunction == "MIN"){
            parsedQuery.returnAggregateFunction = MIN;
        }    
        else if(aggregateFunction == "MAX"){
            parsedQuery.returnAggregateFunction = MAX;
        }     
        else if(aggregateFunction == "SUM"){
            parsedQuery.returnAggregateFunction = SUM;
        }     
        else if(aggregateFunction == "AVG"){
            parsedQuery.returnAggregateFunction = AVG;
        }
        else{
            cout << "SYNTAX ERROR\n";
            return false;
        }

        string returnColumnName;
        for(int i=4; i<(int)tokenizedQuery[12].size() - 1; i++){
            returnColumnName += tokenizedQuery[12][i];
        }
        parsedQuery.returnAggregateColumn = returnColumnName;
    }

    parsedQuery.queryType = GROUPBY;
    
    return true;
}

bool semanticParseGROUPBY(){
    logger.log("semanticParseGROUPBY");

    if(tableCatalogue.isTable(parsedQuery.resultantRelationName)){
        cout<<"SEMANTIC ERROR: Resultant relation already exists"<<endl;
        return false;
    }

    if(!tableCatalogue.isTable(parsedQuery.groupByRelationName)){
        cout<<"SEMANTIC ERROR: Relation doesn't exist"<<endl;
        return false;
    }

    if(!tableCatalogue.isColumnFromTable(parsedQuery.groupByColumnName, parsedQuery.groupByRelationName)){
        cout<<"SEMANTIC ERROR: Column doesn't exist in relation"<<endl;
        return false;
    }

    if(!tableCatalogue.isColumnFromTable(parsedQuery.conditionAggregateColumn, parsedQuery.groupByRelationName)){
        cout<<"SEMANTIC ERROR: Column doesn't exist in relation"<<endl;
        return false;
    }

    if(!tableCatalogue.isColumnFromTable(parsedQuery.returnAggregateColumn, parsedQuery.groupByRelationName)){
        cout<<"SEMANTIC ERROR: Column doesn't exist in relation"<<endl;
        return false;
    }

    return true;
}

void executeGROUPBY(){
    logger.log("executeGROUPBY");
    Table *table = tableCatalogue.getTable(parsedQuery.groupByRelationName);
    
    // copy the pages
    table->copyTable(parsedQuery.resultantRelationName + "_temp");

    Table *groupSortResultPtr = new Table();
    groupSortResultPtr->sourceFileName = parsedQuery.resultantRelationName + ".csv";
    groupSortResultPtr->tableName = parsedQuery.resultantRelationName + "_temp";
    groupSortResultPtr->columns = table->columns;
    groupSortResultPtr->distinctValuesPerColumnCount = table->distinctValuesPerColumnCount;
    groupSortResultPtr->columnCount = table->columnCount;
    groupSortResultPtr->rowCount = table->rowCount;
    groupSortResultPtr->blockCount = table->blockCount;
    groupSortResultPtr->maxRowsPerBlock = table->maxRowsPerBlock;
    groupSortResultPtr->rowsPerBlockCount = table->rowsPerBlockCount;
    tableCatalogue.insertTable(groupSortResultPtr);


    string resultantColumnName;
    if(parsedQuery.returnAggregateFunction == MAX){
        resultantColumnName += "MAX";
    }
    else if(parsedQuery.returnAggregateFunction == MIN){
        resultantColumnName += "MIN";
    }
    else if(parsedQuery.returnAggregateFunction == AVG){
        resultantColumnName += "AVG";
    }
    else if(parsedQuery.returnAggregateFunction == SUM){
        resultantColumnName += "SUM";
    }

    Table *resultantTable = new Table(parsedQuery.resultantRelationName, {parsedQuery.groupByColumnName, resultantColumnName + parsedQuery.returnAggregateColumn});


    int BUFFERSIZE = 10; // in blocks
    groupSortResultPtr->sortTable({parsedQuery.groupByColumnName}, {ASC}, BUFFERSIZE-1);

    groupSortResultPtr->groupBy(parsedQuery.groupByColumnName, 
                                parsedQuery.conditionAggregateFunction, 
                                parsedQuery.conditionAggregateColumn, 
                                parsedQuery.conditionOperator, 
                                parsedQuery.conditionAttributeValue, 
                                parsedQuery.returnAggregateFunction, 
                                parsedQuery.returnAggregateColumn,
                                resultantTable
                            );

    tableCatalogue.deleteTable(parsedQuery.resultantRelationName + "_temp");
    return;
}