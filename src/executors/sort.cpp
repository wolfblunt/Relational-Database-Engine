#include"global.h"
/**
 * @brief File contains method to process SORT commands.
 * 
 * syntax:
 * SORT <table_name> BY <column_name1, column_name2,..., column_namek> IN <ASC|DESC, ASC|DESC,..., ASC|DESC>
 * 
 * sorting_order = ASC | DESC 
 */
bool syntacticParseSORT(){
    logger.log("syntacticParseSORT");
    for(int i=0; i<tokenizedQuery.size(); i++){
        cout << tokenizedQuery[i] << " ";
    }
    cout << endl;

    parsedQuery.queryType = SORT;
    if(tokenizedQuery.size() < 6)
    {
        cout<<"SIZE SYNTAX ERROR"<<endl;
        return false;
    }
    parsedQuery.sortRelationName = tokenizedQuery[1];
    if(tokenizedQuery[2] != "BY")
    {
        cout<<"'BY' SYNTAX ERROR"<<endl;
        return false;
    }
    int i = 3;
    while (i<tokenizedQuery.size() && tokenizedQuery[i]!="IN")
    {
        parsedQuery.sortColumnName.push_back(tokenizedQuery[i]);
        i++;
    }
    if(i == tokenizedQuery.size())
    {
        cout<<"'NOT IN' SYNTAX ERROR"<<endl;
        return false;
    }
    i++;
    while (i<tokenizedQuery.size())
    {
        if(tokenizedQuery[i] == "ASC")
        {
            parsedQuery.sortingStrategy.push_back(ASC);
        }
        else if(tokenizedQuery[i] == "DESC")
        {
            parsedQuery.sortingStrategy.push_back(DESC);
        }
        else
        {
            cout<<"STRATEGY SYNTAX ERROR"<<endl;
            return false;
        }
        i++;
    }
    logger.log("sortingStrategy Size : "+to_string(parsedQuery.sortingStrategy.size()));
    logger.log("sortColumnName Size : "+to_string(parsedQuery.sortColumnName.size()));
    if(parsedQuery.sortingStrategy.size() != parsedQuery.sortColumnName.size())
    {
        cout<<"SIZE NOT MATCH SYNTAX ERROR"<<endl;
        return false;
    }
    
    return true;
}

bool semanticParseSORT(){
    logger.log("semanticParseSORT");

    if(!tableCatalogue.isTable(parsedQuery.sortRelationName)){
        cout<<"SEMANTIC ERROR: Relation doesn't exist"<<endl;
        return false;
    }

    for(int i=0; i<parsedQuery.sortColumnName.size(); i++){
        if(!tableCatalogue.isColumnFromTable(parsedQuery.sortColumnName[i], parsedQuery.sortRelationName)){
            cout<<"SEMANTIC ERROR: Column doesn't exist in relation"<<endl;
            return false;
        }
    }
    
    return true;
}

void executeSORT(){
    logger.log("executeSORT");
    Table* table = tableCatalogue.getTable(parsedQuery.sortRelationName);
    int BUFFERSIZE = 10; // in blocks
    table->sortTable(parsedQuery.sortColumnName, parsedQuery.sortingStrategy, BUFFERSIZE-1);
    table->makePermanent();
    return;
}