#include "global.h"

/**
 * @brief Construct a new Table:: Table object
 *
 */
Table::Table()
{
    logger.log("Table::Table");
}

/**
 * @brief Construct a new Table:: Table object used in the case where the data
 * file is available and LOAD command has been called. This command should be
 * followed by calling the load function;
 *
 * @param tableName 
 */
Table::Table(string tableName)
{
    logger.log("Table::Table");
    this->sourceFileName = "../data/" + tableName + ".csv";
    this->tableName = tableName;
}

/**
 * @brief Construct a new Table:: Table object used when an assignment command
 * is encountered. To create the table object both the table name and the
 * columns the table holds should be specified.
 *
 * @param tableName 
 * @param columns 
 */
Table::Table(string tableName, vector<string> columns)
{
    logger.log("Table::Table");
    this->sourceFileName = "../data/temp/" + tableName + ".csv";
    this->tableName = tableName;
    this->columns = columns;
    this->columnCount = columns.size();
    this->maxRowsPerBlock = (uint)((BLOCK_SIZE * 1000) / (sizeof(int) * columnCount));
    this->writeRow<string>(columns);
}

/**
 * @brief The load function is used when the LOAD command is encountered. It
 * reads data from the source file, splits it into blocks and updates table
 * statistics.
 *
 * @return true if the table has been successfully loaded 
 * @return false if an error occurred 
 */
bool Table::load()
{
    logger.log("Table::load");
    fstream fin(this->sourceFileName, ios::in);
    string line;
    if (getline(fin, line))
    {
        fin.close();
        if (this->extractColumnNames(line))
            if (this->blockify())
                return true;
    }
    fin.close();
    return false;
}

/**
 * @brief Function extracts column names from the header line of the .csv data
 * file. 
 *
 * @param line 
 * @return true if column names successfully extracted (i.e. no column name
 * repeats)
 * @return false otherwise
 */
bool Table::extractColumnNames(string firstLine)
{
    logger.log("Table::extractColumnNames");
    unordered_set<string> columnNames;
    string word;
    stringstream s(firstLine);
    while (getline(s, word, ','))
    {
        word.erase(std::remove_if(word.begin(), word.end(), ::isspace), word.end());
        if (columnNames.count(word))
            return false;
        columnNames.insert(word);
        this->columns.emplace_back(word);
    }
    this->columnCount = this->columns.size();
    this->maxRowsPerBlock = (uint)((BLOCK_SIZE * 1000) / (sizeof(int) * this->columnCount));
    return true;
}

/**
 * @brief This function splits all the rows and stores them in multiple files of
 * one block size. 
 *
 * @return true if successfully blockified
 * @return false otherwise
 */
bool Table::blockify()
{
    logger.log("Table::blockify");
    ifstream fin(this->sourceFileName, ios::in);
    string line, word;
    vector<int> row(this->columnCount, 0);
    vector<vector<int> > rowsInPage(this->maxRowsPerBlock, row);
    int pageCounter = 0;
    unordered_set<int> dummy;
    dummy.clear();
    this->distinctValuesInColumns.assign(this->columnCount, dummy);
    this->distinctValuesPerColumnCount.assign(this->columnCount, 0);
    getline(fin, line);
    while (getline(fin, line))
    {
        stringstream s(line);
        for (int columnCounter = 0; columnCounter < this->columnCount; columnCounter++)
        {
            if (!getline(s, word, ','))
                return false;
            row[columnCounter] = stoi(word);
            rowsInPage[pageCounter][columnCounter] = row[columnCounter];
        }
        pageCounter++;
        this->updateStatistics(row);
        if (pageCounter == this->maxRowsPerBlock)
        {
            bufferManager.writePage(this->tableName, this->blockCount, rowsInPage, pageCounter);
            this->blockCount++;
            this->rowsPerBlockCount.emplace_back(pageCounter);
            pageCounter = 0;
        }
    }
    if (pageCounter && rowsInPage.size())
    {
        bufferManager.writePage(this->tableName, this->blockCount, rowsInPage, pageCounter);
        this->blockCount++;
        this->rowsPerBlockCount.emplace_back(pageCounter);
        pageCounter = 0;
    }

    if (this->rowCount == 0)
        return false;
    this->distinctValuesInColumns.clear();
    return true;
}

/**
 * @brief Given a row of values, this function will update the statistics it
 * stores i.e. it updates the number of rows that are present in the column and
 * the number of distinct values present in each column. These statistics are to
 * be used during optimisation.
 *
 * @param row 
 */
void Table::updateStatistics(vector<int> row)
{
    this->rowCount++;
    for (int columnCounter = 0; columnCounter < this->columnCount; columnCounter++)
    {
        if (!this->distinctValuesInColumns[columnCounter].count(row[columnCounter]))
        {
            this->distinctValuesInColumns[columnCounter].insert(row[columnCounter]);
            this->distinctValuesPerColumnCount[columnCounter]++;
        }
    }
}

/**
 * @brief Checks if the given column is present in this table.
 *
 * @param columnName 
 * @return true 
 * @return false 
 */
bool Table::isColumn(string columnName)
{
    logger.log("Table::isColumn");
    for (auto col : this->columns)
    {
        if (col == columnName)
        {
            return true;
        }
    }
    return false;
}

/**
 * @brief Renames the column indicated by fromColumnName to toColumnName. It is
 * assumed that checks such as the existence of fromColumnName and the non prior
 * existence of toColumnName are done.
 *
 * @param fromColumnName 
 * @param toColumnName 
 */
void Table::renameColumn(string fromColumnName, string toColumnName)
{
    logger.log("Table::renameColumn");
    for (int columnCounter = 0; columnCounter < this->columnCount; columnCounter++)
    {
        if (columns[columnCounter] == fromColumnName)
        {
            columns[columnCounter] = toColumnName;
            break;
        }
    }
    return;
}

/**
 * @brief Function prints the first few rows of the table. If the table contains
 * more rows than PRINT_COUNT, exactly PRINT_COUNT rows are printed, else all
 * the rows are printed.
 *
 */
void Table::print()
{
    logger.log("Table::print");
    uint count = min((long long)PRINT_COUNT, this->rowCount);

    //print headings
    this->writeRow(this->columns, cout);
    Cursor cursor(this->tableName, 0);
    vector<int> row;
    for (int rowCounter = 0; rowCounter < count; rowCounter++)
    {
        row = cursor.getNext();
        this->writeRow(row, cout);
    }
    printRowCount(this->rowCount);
}



/**
 * @brief This function returns one row of the table using the cursor object. It
 * returns an empty row is all rows have been read.
 *
 * @param cursor 
 * @return vector<int> 
 */
void Table::getNextPage(Cursor *cursor)
{
    logger.log("Table::getNext");

        if (cursor->pageIndex < this->blockCount - 1)
        {
            cursor->nextPage(cursor->pageIndex+1);
        }
}



/**
 * @brief called when EXPORT command is invoked to move source file to "data"
 * folder.
 *
 */
void Table::makePermanent()
{
    logger.log("Table::makePermanent");
    if(!this->isPermanent())
        bufferManager.deleteFile(this->sourceFileName);
    string newSourceFile = "../data/" + this->tableName + ".csv";
    ofstream fout(newSourceFile, ios::out);

    //print headings
    this->writeRow(this->columns, fout);

    Cursor cursor(this->tableName, 0);
    vector<int> row;
    for (int rowCounter = 0; rowCounter < this->rowCount; rowCounter++)
    {
        row = cursor.getNext();
        this->writeRow(row, fout);
    }
    fout.close();
}

/**
 * @brief Function to check if table is already exported
 *
 * @return true if exported
 * @return false otherwise
 */
bool Table::isPermanent()
{
    logger.log("Table::isPermanent");
    if (this->sourceFileName == "../data/" + this->tableName + ".csv")
    return true;
    return false;
}

/**
 * @brief The unload function removes the table from the database by deleting
 * all temporary files created as part of this table
 *
 */
void Table::unload(){
    logger.log("Table::~unload");
    logger.log(to_string(this->blockCount));
    for (int pageCounter = 0; pageCounter < this->blockCount; pageCounter++)
        bufferManager.deleteFile(this->tableName, pageCounter);
    if (!isPermanent())
        bufferManager.deleteFile(this->sourceFileName);
}


class CompareRows {
    vector<int> paramIndices;
    vector<SortingStrategy> strat;
public:
    CompareRows(vector<int> &p, vector<SortingStrategy> &s){
        paramIndices = p;
        strat = s;
    }

    bool operator() (const vector<int> &first, const vector<int> &second) {

        for(int i=0; i<paramIndices.size(); i++){
            int ind = paramIndices[i];
            if(first[ind] != second[ind]){
                if(strat[i] == ASC)
                    return first[ind] < second[ind];
                else
                    return second[ind] < first[ind];
            }
        }
        return false;
    }
};

class CompareRowsAlongWithPartitionNumber {
    vector<int> paramIndices;
    vector<SortingStrategy> strat;
public:
    CompareRowsAlongWithPartitionNumber(vector<int> &p, vector<SortingStrategy> &s){
        paramIndices = p;
        strat = s;
    }

    bool operator() (const pair<vector<int>, int> &one, const pair<vector<int>, int> &two) {
        //the int is the block number
        for(int i=0; i<paramIndices.size(); i++){
            int ind = paramIndices[i];
            if(one.first[ind] != two.first[ind]){
                if(strat[i] == ASC){
                    return one.first[ind] > two.first[ind];
                }
                else {
                    return two.first[ind] > one.first[ind];
                }
            }
        }
        return false;
    }
};





void Table::internalSort(int startPage, int endPage, vector<int> &columnIndices, vector<SortingStrategy> &sortingStrats){
    vector<vector<int>> b;
    for(int i=startPage; i<=endPage; i++){
        Page p = bufferManager.getPage(this->tableName, i);
        for(int j=0; j<p.getRowCount(); j++){
            b.push_back(p.getRow(j));
        }
    }

    sort(b.begin(), b.end(), CompareRows(columnIndices, sortingStrats));

    int pageIndex = startPage;
    vector<vector<int>> temp;
    for(int i=0; i<b.size(); i++){
        temp.push_back(b[i]); // Can it be done without temp?
        if(temp.size() == maxRowsPerBlock){
            bufferManager.writeToCache(this->tableName, pageIndex, temp, temp.size());
            temp.clear();
            pageIndex++;
        }
    }

    if(temp.size() != 0){
        bufferManager.writeToCache(this->tableName, pageIndex, temp, temp.size());
    }

    bufferManager.writeCacheToDisk();
}

void printpq( priority_queue<pair<vector<int>, int>, vector<pair<vector<int>, int>>, CompareRowsAlongWithPartitionNumber> pq){
    while(pq.size() != 0){
        pair<vector<int>, int> p = pq.top();
        pq.pop();
        for(int i=0; i<p.first.size(); i++){
            cout << p.first[i] << " ";
        }
        cout << ",     " << p.second << endl;
    }
    cout << endl;
}

void Table::mergePartitions(int startPage, int endPage, vector<int> &columnIndices, vector<SortingStrategy> &sortingStrats, vector<int> &partitionStartPoints){
    int numberPartitions = partitionStartPoints.size();
    vector<int> currentRows(numberPartitions); // size of this vector is equal to merge factor
    vector<int> currentPageIndices = partitionStartPoints; // size of this vector is equal to merge factor
    CompareRowsAlongWithPartitionNumber cmp(columnIndices, sortingStrats);
    priority_queue<pair<vector<int>, int>, vector<pair<vector<int>, int>>, CompareRowsAlongWithPartitionNumber> pq(cmp); // size of this priority queue is equal to merge factor
    vector<Page> pageList; // this used 9 out of 10 blocks


    for(int i=0; i<numberPartitions; i++){
        Page page = bufferManager.getPage(this->tableName, currentPageIndices[i]);
        pageList.push_back(page);
        pq.push({page.getRow(0), i});
    }

    vector<vector<int>> writePageValues; // this uses the last 1 block
    int writePageIndex = startPage;

    while(pq.size() != 0){
        pair<vector<int>, int> pv = pq.top();
        pq.pop();
        int partitionNumber = pv.second;

        writePageValues.push_back(pv.first);
        if(writePageValues.size() == this->maxRowsPerBlock){
            bufferManager.writePage(this->tableName + "_sorted", writePageIndex, writePageValues, writePageValues.size());
            writePageIndex++;
            writePageValues.clear();
        }

        currentRows[partitionNumber]++;
        // cout << "CURRENTROWS: ";
        // for(int i=0; i< numberPartitions; i++){
        //     cout << currentRows[i] << " ";
        // }
        // cout << endl;
        // cout << "PARTITIONSTARTPOINTS: ";
        // for(int i=0; i< numberPartitions; i++){
        //     cout << partitionStartPoints[i] << " ";
        // }
        // cout << endl;
        // cout << "CURRENTPAGEINDICES: ";
        // for(int i=0; i< numberPartitions; i++){
        //     cout << currentPageIndices[i] << " ";
        // }
        // cout << endl;
        if(currentRows[partitionNumber] == this->rowsPerBlockCount[currentPageIndices[partitionNumber]]){
            // change the block number of a partition
            currentPageIndices[partitionNumber]++;
            if(partitionNumber == numberPartitions-1 && currentPageIndices[partitionNumber] <= endPage){
                Page page = bufferManager.getPage(this->tableName, currentPageIndices[partitionNumber]);
                pageList[partitionNumber] = page;
                pq.push({page.getRow(0), partitionNumber});
                currentRows[partitionNumber] = 0;
            }
            else if(partitionNumber < numberPartitions-1 && currentPageIndices[partitionNumber] < partitionStartPoints[partitionNumber+1]){
                Page page = bufferManager.getPage(this->tableName, currentPageIndices[partitionNumber]);
                pageList[partitionNumber] = page;
                pq.push({page.getRow(0), partitionNumber});
                currentRows[partitionNumber] = 0;
            }
        }
        else{
            pq.push({pageList[partitionNumber].getRow(currentRows[partitionNumber]), partitionNumber});
        }
    }

    if(writePageValues.size() != 0){
        bufferManager.writePage(this->tableName + "_sorted", writePageIndex, writePageValues, writePageValues.size());
        writePageValues.clear();
    }


    for(int i=startPage; i<=endPage; i++){
        Page p = bufferManager.getPageNotInCatalogue(this->tableName + "_sorted", i, this->columnCount, this->rowsPerBlockCount[i]);
        vector<vector<int>> values = p.getRows();
        bufferManager.writePage(this->tableName, i, values, values.size());
        bufferManager.deleteFile(this->tableName + "_sorted", i);
    }

    // for out of place sorting, first create a copy of the table and then sort the copy in place (similar to what is done in matrix)
}

void Table::sortUtil(int startPage, int endPage, vector<int> &columnIndices, vector<SortingStrategy> &sortingStrats, int mergeFactor){

    if(endPage - startPage + 1 <= mergeFactor){
        internalSort(startPage, endPage, columnIndices, sortingStrats);
        return;
    }

    vector<int> partitionStartPoints;
    int partitionStart = startPage;
    while(partitionStart <= endPage){
        partitionStartPoints.push_back(partitionStart);
        int partitionSize = (endPage - startPage + 1) / mergeFactor + ((endPage - startPage + 1) % mergeFactor != 0); // basically the ceil function
        int partitionEnd = min(endPage, partitionStart + partitionSize - 1);
        sortUtil(partitionStart, partitionEnd, columnIndices, sortingStrats, mergeFactor);
        partitionStart += partitionSize;
    }

    mergePartitions(startPage, endPage, columnIndices, sortingStrats, partitionStartPoints);
}


void Table::sortTable(vector<string> columns, vector<SortingStrategy> orders, int mergeFactor){
    logger.log("Table::sort");

    vector<int> columnIndices;
    int cur = 0;
    for(int i=0; i<this->columns.size(); i++){
        if(this->columns[i] == columns[cur]){
            columnIndices.push_back(i);
            cur++;
        }
    }
    sortUtil(0, this->blockCount - 1, columnIndices, orders, mergeFactor);
}

void Table::copyTable(string destTableName){
    logger.log("Table:copyTable");
    if (this->tableName == destTableName){
        logger.log("Copy Failed");
        return;
    }
    for(int i=0; i<this->blockCount; i++){
        Page p = bufferManager.getPage(this->tableName, i);
        vector<vector<int>> pageValues = p.getRows();
        bufferManager.writePage(destTableName, i, pageValues, pageValues.size());
    }
}



bool checkGroupCondition(AggregateStrategy havingStrategy, int havingThreshold, int aggregateValueHaving, int count, string binOp){
    if(havingStrategy == AVG)
        aggregateValueHaving /= count;

    if(binOp == "<"){
        return (aggregateValueHaving < havingThreshold);
    }
    else if(binOp == ">"){
        return (aggregateValueHaving > havingThreshold);
    }
    else if(binOp == "<="){
        return (aggregateValueHaving <= havingThreshold);
    }
    else if(binOp == ">="){
        return (aggregateValueHaving >= havingThreshold);
    }
    else if(binOp == "=="){
        return (aggregateValueHaving == havingThreshold);
    }
    else if(binOp == "!="){
        return (aggregateValueHaving != havingThreshold);
    }
    return false;
}

int updateAggregateValue(AggregateStrategy strategy, int aggregateValue, int newValue){
    if(strategy == MIN){
        return min(aggregateValue, newValue);
    }
    else if(strategy == MAX){
        return max(aggregateValue, newValue);
    }
    else if(strategy == SUM){
        return aggregateValue + newValue;
    }
    else if(strategy == AVG){
        return aggregateValue + newValue; // divinding by count later
    }
    return 0;
}

// TT <- GROUP BY Stud_age FROM crossed HAVING AVG(Stud_Id) <= 20 RETURN AVG(Advisor_id)

void Table::groupBy(string groupingColumn, AggregateStrategy havingStrategy, string havingColumn, string binOp, int havingThreshold, AggregateStrategy returnStrategy, string returnColumn, Table *resultantTable){
    logger.log("Table:groupBy");

    int groupingColumnIndex;
    for(int i=0; i<this->columns.size(); i++){
        if(this->columns[i] == groupingColumn){
            groupingColumnIndex = i;
        }
    }

    int havingColumnIndex;
    for(int i=0; i<this->columns.size(); i++){
        if(this->columns[i] == havingColumn){
            havingColumnIndex = i;
        }
    }

    int returnColumnIndex;
    for(int i=0; i<this->columns.size(); i++){
        if(this->columns[i] == returnColumn){
            returnColumnIndex = i;
        }
    }

    int prevGroupingColumnValue = -1;
    int aggregateValueHaving = -1;
    int aggregateValueReturn = -1;
    int count = 0;

    for(int curPage=0; curPage<this->blockCount; curPage++){
        Page p = bufferManager.getPage(this->tableName, curPage);
        vector<vector<int>> values = p.getRows();
        for(int i=0; i<this->rowsPerBlockCount[curPage]; i++){
            // cout << prevGroupingColumnValue << " " << aggregateValueHaving << " " << aggregateValueReturn << " " << count << " " << curPage << endl;
            if(prevGroupingColumnValue == -1 || prevGroupingColumnValue == values[i][groupingColumnIndex]){
                if(aggregateValueHaving == -1)
                    aggregateValueHaving = values[i][havingColumnIndex];
                else
                    aggregateValueHaving = updateAggregateValue(havingStrategy, aggregateValueHaving, values[i][havingColumnIndex]);
                
                if(aggregateValueReturn == -1)
                    aggregateValueReturn = values[i][returnColumnIndex];
                else
                    aggregateValueReturn = updateAggregateValue(returnStrategy, aggregateValueReturn, values[i][returnColumnIndex]);

                count++;
            }
            else{
                bool includeGroup = checkGroupCondition(havingStrategy, havingThreshold, aggregateValueHaving, count, binOp);
                if(includeGroup){
                    if(havingStrategy == AVG)
                        aggregateValueHaving /= count;
                    if(returnStrategy == AVG)
                        aggregateValueReturn /= count;
                    resultantTable->writeRow<int>({prevGroupingColumnValue, aggregateValueReturn});
                }
                count = 1;
                aggregateValueHaving = values[i][havingColumnIndex];
                aggregateValueReturn = values[i][returnColumnIndex];
            }
            prevGroupingColumnValue = values[i][groupingColumnIndex];
        }
    }

    if(count != 1){
        bool includeGroup = checkGroupCondition(havingStrategy, havingThreshold, aggregateValueHaving, count, binOp);
        if(includeGroup){
            if(havingStrategy == AVG)
                aggregateValueHaving /= count;
            if(returnStrategy == AVG)
                aggregateValueReturn /= count;
            resultantTable->writeRow<int>({prevGroupingColumnValue, aggregateValueReturn});
        }
    }

    resultantTable->blockify();
    tableCatalogue.insertTable(resultantTable);
    bufferManager.clearCache();
}



/**
 * @brief Function that returns a cursor that reads rows from this table
 * 
 * @return Cursor 
 */
Cursor Table::getCursor()
{
    logger.log("Table::getCursor");
    Cursor cursor(this->tableName, 0);
    return cursor;
}


/**
 * @brief Function that returns the index of column indicated by columnName
 * 
 * @param columnName 
 * @return int 
 */
int Table::getColumnIndex(string columnName)
{
    logger.log("Table::getColumnIndex");
    for (int columnCounter = 0; columnCounter < this->columnCount; columnCounter++)
    {
        if (this->columns[columnCounter] == columnName)
            return columnCounter;
    }
}
