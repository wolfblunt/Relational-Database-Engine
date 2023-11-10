#include "global.h"

/**
 * @brief 
 * SYNTAX :- 
 * <new_relation_name> <- JOIN <table_name1>, <table_name2> ON <column_name1> bin_op <column_name2>
 */

bool syntacticParseJOIN()
{
    logger.log("syntacticParseJOIN");
    if (tokenizedQuery.size() != 9 || tokenizedQuery[2] != "JOIN" || tokenizedQuery[5] != "ON")
    {
        cout << "SYNTAC ERROR" << endl;
        return false;
    }
    parsedQuery.queryType = JOIN;
    parsedQuery.joinResultRelationName = tokenizedQuery[0];
    parsedQuery.joinFirstRelationName = tokenizedQuery[3];
    parsedQuery.joinSecondRelationName = tokenizedQuery[4];
    parsedQuery.joinFirstColumnName = tokenizedQuery[6];
    parsedQuery.joinSecondColumnName = tokenizedQuery[8];
    string binaryOperator = tokenizedQuery[7];
    if (binaryOperator == "<")
        parsedQuery.joinBinaryOperator = LESS_THAN;
    else if (binaryOperator == ">")
        parsedQuery.joinBinaryOperator = GREATER_THAN;
    else if (binaryOperator == ">=" || binaryOperator == "=>")
        parsedQuery.joinBinaryOperator = GEQ;
    else if (binaryOperator == "<=" || binaryOperator == "=<")
        parsedQuery.joinBinaryOperator = LEQ;
    else if (binaryOperator == "==")
        parsedQuery.joinBinaryOperator = EQUAL;
    else if (binaryOperator == "!=")
        parsedQuery.joinBinaryOperator = NOT_EQUAL;
    else
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    return true;
}

bool semanticParseJOIN()
{
    logger.log("semanticParseJOIN");
    if (tableCatalogue.isTable(parsedQuery.joinResultRelationName))
    {
        cout << "SEMANTIC ERROR: Resultant relation already exists" << endl;
        return false;
    }
    if (!tableCatalogue.isTable(parsedQuery.joinFirstRelationName) || !tableCatalogue.isTable(parsedQuery.joinSecondRelationName))
    {
        cout << "SEMANTIC ERROR: Relation doesn't exist" << endl;
        return false;
    }
    if (!tableCatalogue.isColumnFromTable(parsedQuery.joinFirstColumnName, parsedQuery.joinFirstRelationName) || 
        !tableCatalogue.isColumnFromTable(parsedQuery.joinSecondColumnName, parsedQuery.joinSecondRelationName))
    {
        cout << "SEMANTIC ERROR: Column doesn't exist in relation" << endl;
        return false;
    }
    return true;
}


// TT <- JOIN Student, Advisor ON Advisor_id == Adv_Id

void executeJOIN()
{
    logger.log("executeJOIN"); // definitely came here
    
    Table* table1 = tableCatalogue.getTable(parsedQuery.joinFirstRelationName);
    table1->copyTable(parsedQuery.joinFirstRelationName + "_temp"); // copying the pages
    Table* sortedTable1 = new Table();
    sortedTable1->sourceFileName = parsedQuery.joinFirstRelationName + ".csv";
    sortedTable1->tableName = parsedQuery.joinFirstRelationName + "_temp";
    sortedTable1->columns = table1->columns;
    sortedTable1->distinctValuesPerColumnCount = table1->distinctValuesPerColumnCount;
    sortedTable1->columnCount = table1->columnCount;
    sortedTable1->rowCount = table1->rowCount;
    sortedTable1->blockCount = table1->blockCount;
    sortedTable1->maxRowsPerBlock = table1->maxRowsPerBlock;
    sortedTable1->rowsPerBlockCount = table1->rowsPerBlockCount;
    tableCatalogue.insertTable(sortedTable1);
    
    Table* table2 = tableCatalogue.getTable(parsedQuery.joinSecondRelationName);
    table2->copyTable(parsedQuery.joinSecondRelationName + "_temp"); // copying the pages
    Table* sortedTable2 = new Table();
    sortedTable2->sourceFileName = parsedQuery.joinSecondRelationName + ".csv";
    sortedTable2->tableName = parsedQuery.joinSecondRelationName + "_temp";
    sortedTable2->columns = table2->columns;
    sortedTable2->distinctValuesPerColumnCount = table2->distinctValuesPerColumnCount;
    sortedTable2->columnCount = table2->columnCount;
    sortedTable2->rowCount = table2->rowCount;
    sortedTable2->blockCount = table2->blockCount;
    sortedTable2->maxRowsPerBlock = table2->maxRowsPerBlock;
    sortedTable2->rowsPerBlockCount = table2->rowsPerBlockCount;
    tableCatalogue.insertTable(sortedTable2);
    
    int BUFFERSIZE = 10; // in blocks
    sortedTable1->sortTable({parsedQuery.joinFirstColumnName}, {ASC}, BUFFERSIZE - 1);
    sortedTable2->sortTable({parsedQuery.joinSecondColumnName}, {ASC}, BUFFERSIZE - 1);

    int firstColIdx = -1, secondColIdx = -1;
    string firstColName = "", secondColName = "";
    for (int colCounter = 0; colCounter < sortedTable1->columnCount; colCounter++) 
    {
        firstColName = sortedTable1->columns[colCounter];
        if (parsedQuery.joinFirstColumnName == firstColName) 
        {
            firstColIdx = colCounter;
            if (tableCatalogue.isColumnFromTable(parsedQuery.joinSecondRelationName, parsedQuery.joinFirstColumnName))
                firstColName += "_1";
            break;
        }
    }
    for (int colCounter = 0; colCounter < sortedTable2->columnCount; colCounter++) 
    {
        secondColName = sortedTable2->columns[colCounter];
        if (parsedQuery.joinSecondColumnName == secondColName) 
        {
            secondColIdx = colCounter;
            if (tableCatalogue.isColumnFromTable(parsedQuery.joinFirstRelationName, parsedQuery.joinSecondColumnName))
                secondColName += "_2";
            break;
        }
    }
    vector<string> resultTableColumns;
    for (string colName : table1->columns)
    {
        if (colName == parsedQuery.joinFirstColumnName)
            resultTableColumns.push_back(firstColName);
        else 
            resultTableColumns.push_back(colName);
    }
    for (string colName : table2->columns)
    {
        if (colName == parsedQuery.joinSecondColumnName)
            resultTableColumns.push_back(secondColName);
        else 
            resultTableColumns.push_back(colName);
    }

    Table* resultTable = new Table(parsedQuery.joinResultRelationName, resultTableColumns);
    
    for (int block1 = 0; block1 < sortedTable1->blockCount; block1++)
    {
        Page p1 = bufferManager.getPage(sortedTable1->tableName, block1);
        vector<vector<int> > rowsFromTable1 = p1.getRows();
        for (int block2 = 0; block2 < sortedTable2->blockCount; block2++)
        {
            Page p2 = bufferManager.getPage(sortedTable2->tableName, block2);
            vector<vector<int> > rowsFromTable2 = p2.getRows();
            vector<int> resultTableRow;
            int checkPoint = -1; // for a particular pair of {block1, block2}
            if (parsedQuery.joinBinaryOperator == LESS_THAN)
            {
                int rowIdx2 = 0;
                for (int rowIdx1 = 0; rowIdx1 < rowsFromTable1.size(); rowIdx1++)
                {
                    while (rowIdx2 < rowsFromTable2.size() && rowsFromTable1[rowIdx1][firstColIdx] >= rowsFromTable2[rowIdx2][secondColIdx])
                        rowIdx2++;
                    checkPoint = rowIdx2;
                    for (int i = checkPoint; i < rowsFromTable2.size(); i++)
                    {
                        resultTableRow.clear();
                        for (int j = 0; j < rowsFromTable1[rowIdx1].size(); j++)
                            resultTableRow.push_back(rowsFromTable1[rowIdx1][j]);
                        for (int j = 0; j < rowsFromTable2[rowIdx2].size(); j++)
                            resultTableRow.push_back(rowsFromTable2[i][j]);
                        resultTable->writeRow<int>(resultTableRow);
                    }
                }
            }
            else if (parsedQuery.joinBinaryOperator == LEQ)
            {
                int rowIdx2 = 0;
                for (int rowIdx1 = 0; rowIdx1 < rowsFromTable1.size(); rowIdx1++)
                {
                    while (rowIdx2 < rowsFromTable2.size() && rowsFromTable1[rowIdx1][firstColIdx] > rowsFromTable2[rowIdx2][secondColIdx])
                        rowIdx2++;
                    checkPoint = rowIdx2;
                    for (int i = checkPoint; i < rowsFromTable2.size(); i++)
                    {
                        resultTableRow.clear();
                        for (int j = 0; j < rowsFromTable1[rowIdx1].size(); j++)
                            resultTableRow.push_back(rowsFromTable1[rowIdx1][j]);
                        for (int j = 0; j < rowsFromTable2[rowIdx2].size(); j++)
                            resultTableRow.push_back(rowsFromTable2[i][j]);
                        resultTable->writeRow<int>(resultTableRow);
                    }
                }
            }
            else if (parsedQuery.joinBinaryOperator == GREATER_THAN)
            {
                int rowIdx1 = 0;
                for (int rowIdx2 = 0; rowIdx2 < rowsFromTable2.size(); rowIdx2++)
                {
                    while (rowIdx1 < rowsFromTable1.size() && rowsFromTable2[rowIdx2][secondColIdx] >= rowsFromTable1[rowIdx1][firstColIdx])
                        rowIdx1++;
                    checkPoint = rowIdx1;
                    for (int i = checkPoint; i < rowsFromTable1.size(); i++)
                    {
                        resultTableRow.clear();
                        for (int j = 0; j < rowsFromTable1[rowIdx1].size(); j++)
                            resultTableRow.push_back(rowsFromTable1[i][j]);
                        for (int j = 0; j < rowsFromTable2[rowIdx2].size(); j++)
                            resultTableRow.push_back(rowsFromTable2[rowIdx2][j]);
                        resultTable->writeRow<int>(resultTableRow);
                    }
                }
            }
            else if (parsedQuery.joinBinaryOperator == GEQ)
            {
                int rowIdx1 = 0;
                for (int rowIdx2 = 0; rowIdx2 < rowsFromTable2.size(); rowIdx2++)
                {
                    while (rowIdx1 < rowsFromTable1.size() && rowsFromTable2[rowIdx2][secondColIdx] > rowsFromTable1[rowIdx1][firstColIdx])
                        rowIdx1++;
                    checkPoint = rowIdx1;
                    for (int i = checkPoint; i < rowsFromTable1.size(); i++)
                    {
                        resultTableRow.clear();
                        for (int j = 0; j < rowsFromTable1[rowIdx1].size(); j++)
                            resultTableRow.push_back(rowsFromTable1[i][j]);
                        for (int j = 0; j < rowsFromTable2[rowIdx2].size(); j++)
                            resultTableRow.push_back(rowsFromTable2[rowIdx2][j]);
                        resultTable->writeRow<int>(resultTableRow);
                    }
                }
            }
            else // if (parsedQuery.joinBinaryOperator == EQUAL)
            {
                int rowIdx2 = 0;
                for (int rowIdx1 = 0; rowIdx1 < rowsFromTable1.size(); rowIdx1++)
                {
                    while (rowIdx2 < rowsFromTable2.size() && rowsFromTable1[rowIdx1][firstColIdx] >= rowsFromTable2[rowIdx2][secondColIdx])
                        rowIdx2++;
                    checkPoint = rowIdx2;
                    for (int i = 0; i < checkPoint; i++)
                    {
                        resultTableRow.clear();
                        if (rowsFromTable1[rowIdx1][firstColIdx] == rowsFromTable2[i][secondColIdx])
                        {
                            for (int j = 0; j < rowsFromTable1[rowIdx1].size(); j++)
                                resultTableRow.push_back(rowsFromTable1[rowIdx1][j]);
                            for (int j = 0; j < rowsFromTable2[i].size(); j++)
                                resultTableRow.push_back(rowsFromTable2[i][j]);
                            resultTable->writeRow<int>(resultTableRow);    
                        }
                    }
                }
            }
        }
    }
    resultTable->blockify();
    tableCatalogue.deleteTable(sortedTable1->tableName);
    tableCatalogue.deleteTable(sortedTable2->tableName);
    tableCatalogue.insertTable(resultTable);
    return;
    
}
