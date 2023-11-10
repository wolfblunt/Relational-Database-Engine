#include "global.h"


/**
 * @brief Construct a new Matrix:: Matrix object
 *
 */
Matrix::Matrix()
{
    logger.log("Matrix::Matrix");
}

/**
 * @brief The Matrix class holds all information related to a loaded matrix. It
 * also implements methods that interact with the parsers, executors, cursors
 * and the buffer manager. There are typically 2 ways a matrix object gets
 * created through the course of the workflow - the first is by using the LOAD
 * command and the second is to use assignment statements (SELECT, PROJECT,
 * JOIN, SORT, CROSS and DISTINCT). 
 *
 */


Matrix::Matrix(string matrixName)
{
    logger.log("Matrix::Matrix");
    this->sourceFileName = "../data/" + matrixName + ".csv";
    this->matrixName = matrixName;
}

bool Matrix::load()
{
    logger.log("Matrix::load");
    fstream fin(this->sourceFileName, ios::in);
    string line;
    if (getline(fin, line))
    {
        fin.close();
        if (this->extractMetadata(line))
            if (this->blockify())
                return true;
    }
    fin.close();
    return false;
}




bool Matrix::extractMetadata(string firstLine)
{
    logger.log("Matrix::extractMetadata");
    string word;
    stringstream s(firstLine);
    while (getline(s, word, ','))
    {
        word.erase(std::remove_if(word.begin(), word.end(), ::isspace), word.end());
        this->columnCount++;
        this->rowCount++;
    }
    this->maxRowsPerBlock = max((uint)1, (uint)((BLOCK_SIZE * 1000) / (sizeof(int) * this->columnCount)));
    this->maxElementsPerBlock = (uint)(BLOCK_SIZE * 1000) / sizeof(int);
    return true;
}


/**
 * @brief This function splits all the rows and stores them in multiple files of
 * one block size. 
 *
 * @return true if successfully blockified
 * @return false otherwise
 */
bool Matrix::blockify()
{
    logger.log("Matrix::blockify");
    ifstream fin(this->sourceFileName, ios::in);
    string line, word;
    vector<int> row(this->columnCount, 0);
    vector<vector<int> > rowsInPage(this->maxRowsPerBlock, row);
    int pageCounter = 0;

    if(this->maxRowsPerBlock > 1)
    {
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
            if (pageCounter == this->maxRowsPerBlock)
            {
                bufferManager.writePage(this->matrixName, this->blockCount, rowsInPage, pageCounter);
                this->blockCount++;
                this->rowsPerBlockCount.emplace_back(pageCounter);
                pageCounter = 0;
            }
        }
        if (pageCounter)
        {
            bufferManager.writePage(this->matrixName, this->blockCount, rowsInPage, pageCounter);
            this->blockCount++;
            this->rowsPerBlockCount.emplace_back(pageCounter);
            pageCounter = 0;
        }

        if (this->rowCount == 0)
            return false;
    }
    else
    {
        while (getline(fin, line))
        {
            stringstream s(line);
            int columnCounter = 0;
            while(columnCounter != this->columnCount)
            {
                int curElementsCounter = 0;
                vector<int> row;
                for (; columnCounter < this->columnCount && curElementsCounter < this->maxElementsPerBlock; columnCounter++, curElementsCounter++)
                {
                    if (!getline(s, word, ','))
                        return false;
                    row.push_back(stoi(word));
                }
                bufferManager.writePage(this->matrixName, this->blockCount, {row}, 1);
                this->blockCount++;
                this->rowsPerBlockCount.emplace_back(1);
            }
        }
    }
    return true;
}


/**
 * @brief Function changes the current name of the matrix to the specified 
 * new name
 * 
 */
void Matrix::renameMatrix(string oldName, string newName)
{
    logger.log("Matrix::renameMatrix");
    for (int i = 0; i < this->blockCount; i++)
    {
        string oldFileName = "../data/temp/" + oldName + "_Page" + to_string(i);
        string newFileName = "../data/temp/" + newName + "_Page" + to_string(i);
        rename(oldFileName.c_str(), newFileName.c_str());
    }
    Matrix* matrix = matrixCatalogue.getMatrix(oldName);
    matrix->sourceFileName = "../data/" + newName + ".csv";
    matrix->matrixName = newName;
    matrixCatalogue.insertMatrix(matrix);
    matrixCatalogue.deleteMatrix(oldName, 0);
}



/**
 * @brief Function prints the 1st 20 rows of the matrix. If the matrix contains
 * more rows than 20, exactly 20 rows are printed, else all the rows are printed.
 *
 */
void Matrix::printMatrix()
{
    logger.log("Matrix::printMatrix");
    uint count = min((long long)PRINT_COUNT, this->rowCount);
    // directly start printing from the 1st row onwards
    Cursor cursor(this->matrixName, 0);
    vector<int> row;
    if(this->columnCount <= this->maxElementsPerBlock)
    {
        for (int rowCounter = 0; rowCounter < count; rowCounter++)
        {
            row = cursor.getNextMatrix();
            this->writePrintRow(row, cout);
        }
    }
    else
    {
        int numBlocksInOneRow = this->columnCount / this->maxElementsPerBlock + (this->columnCount % this->maxElementsPerBlock != 0);
        for (int rowCounter = 0; rowCounter < count; rowCounter++)
        {
            for(int j = 0; j < numBlocksInOneRow; j++)
            {
                vector<int> temprow = cursor.getNextMatrix();
                for(int x: temprow)
                    row.push_back(x);
            }
            this->writePrintRow(row, cout);
            row.clear();
        }
    }
    printRowCount(this->rowCount);
}



/**
 * @brief called when EXPORT command is invoked to move source file to "data"
 * folder.
 *
 */
void Matrix::makePermanent()
{
    logger.log("Matrix::makePermanent");
    if(!this->isPermanent())
        bufferManager.deleteFile(this->sourceFileName);
    string newSourceFile = "../data/" + this->matrixName + ".csv";
    ofstream fout(newSourceFile, ios::out);
    Cursor cursor(this->matrixName, 0);
    vector<int> row;
    uint count = min((long long)PRINT_COUNT, this->rowCount);
    if(this->columnCount <= this->maxElementsPerBlock)
    {
        for (int rowCounter = 0; rowCounter < count; rowCounter++)
        {
            row = cursor.getNextMatrix();
            this->writePrintRow(row, fout);
        }
    }
    else
    {
        int numBlocksInOneRow = this->columnCount / this->maxElementsPerBlock + (this->columnCount % this->maxElementsPerBlock != 0);
        for (int rowCounter = 0; rowCounter < count; rowCounter++)
        {
            for(int j = 0; j < numBlocksInOneRow; j++)
            {
                vector<int> temprow = cursor.getNextMatrix();
                for(int x: temprow)
                    row.push_back(x);
            }
            this->writePrintRow(row, fout);
            row.clear();
        }
    }
    fout.close();
}



/**
 * @brief Function to check if matrix is already exported
 *
 * @return true if exported
 * @return false otherwise
 */
bool Matrix::isPermanent()
{
    logger.log("Matrix::isPermanent");
    if (this->sourceFileName == "../data/" + this->matrixName + ".csv")
    {
        return true;
    }
    return false;
}


/**
 * @brief The unload function removes the matrix from the database by deleting
 * all temporary files created as part of this matrix
 *
 */
void Matrix::unload()
{
    logger.log("Matrix::~unload");
    for (int pageCounter = 0; pageCounter < this->blockCount; pageCounter++)
        bufferManager.deleteFile(this->matrixName, pageCounter);
    if (!isPermanent())
        bufferManager.deleteFile(this->sourceFileName);
}


/**
 * @brief This function returns one row of the matrix using the cursor object. It
 * returns an empty row is all rows have been read.
 *
 * @param cursor 
 * @return vector<int> 
 */
void Matrix::getNextPage(Cursor *cursor)
{
    logger.log("Matrix::getNextMatrix");

        if (cursor->pageIndex < this->blockCount - 1)
        {
            cursor->nextPage(cursor->pageIndex+1);
        }
}

/**
 * @brief Get index of the page in which matrix[i][j] lies
 *
 */
int getPageIndex(int row, int col, Matrix* matrix){
    if(matrix->maxRowsPerBlock == 1)
        return row * (matrix->columnCount / matrix->maxElementsPerBlock + (matrix->columnCount % matrix->maxElementsPerBlock != 0)) + col/matrix->maxElementsPerBlock;
    else
        return row / (matrix->maxRowsPerBlock);
}


/**
 * @brief Get the row index inside a page in which matrix[i][j] lies
 *
 */
int getRowIndex(int row, int col, Matrix* matrix){
    return row%(matrix->maxRowsPerBlock);
}


/**
 * @brief Get the column index inside a page in which matrix[i][j] lies
 *
 */
int getColumnIndex(int row, int col, Matrix* matrix){
    if(matrix->maxRowsPerBlock == 1){
        return col % matrix->maxElementsPerBlock;
    }
    else
        return col;
}


/**
 * @brief Get the table inside a page
 *
 */
vector<vector<int>> getPageValues(Page &page){
    vector<vector<int>> pageValues;
    vector<int> temp;
    int tempindex = 0;
    temp = page.getRow(tempindex);
    while(temp.size() != 0){
        pageValues.push_back(temp);
        tempindex++;
        temp = page.getRow(tempindex);
    }
    return pageValues;
}


/**
 * @brief Copy the pages of a matrix. The destination matrix name must be different from source matrix name
 *
 */
void Matrix::copyMatrix(string destMatrixName){
    logger.log("Matrix:copyMatrix");
    if (this->matrixName == destMatrixName){
        logger.log("Copy Failed");
        return;
    }
    for(int i=0; i<this->blockCount; i++){
        Page p = bufferManager.getPage(this->matrixName, i);
        vector<vector<int>> pageValues = getPageValues(p);
        bufferManager.writePage(destMatrixName, i, pageValues, pageValues.size());
    }
}

/**
 * @brief Swap matrix[i][j] and matrix[j][i]
 *
 */
void swapPosition(int row, int col, Matrix* matrix){
    int pageIndex1 = getPageIndex(row, col, matrix);
    int pageIndex2 = getPageIndex(col, row, matrix);

    // logger.log(to_string(row) + " " + to_string(col));
    // logger.log("PAGE INDICES "  + to_string(pageIndex1) + " " + to_string(pageIndex2));

    Page page1 = bufferManager.getPage(matrix->matrixName, pageIndex1);
    Page page2 = bufferManager.getPage(matrix->matrixName, pageIndex2);

    int rowIndex1 = getRowIndex(row, col, matrix);
    int rowIndex2 = getRowIndex(col, row, matrix);

    int columnIndex1 = getColumnIndex(row, col, matrix);
    int columnIndex2 = getColumnIndex(col, row, matrix);
    // logger.log(to_string(rowIndex1) + " " + to_string(columnIndex1));
    // logger.log(to_string(rowIndex2) + " " + to_string(columnIndex2));

    vector<vector<int>> page1Values = getPageValues(page1);
    vector<vector<int>> page2Values = getPageValues(page2);

    int val1 = page1Values[rowIndex1][columnIndex1];
    int val2 = page2Values[rowIndex2][columnIndex2];

    // logger.log(to_string(val1) + " " + to_string(val2));
    // logger.log(to_string(page1Values[0].size()) + " " + to_string(page2Values[0].size()));

    if(pageIndex1 == pageIndex2){
        page1Values[rowIndex1][columnIndex1] = val2;
        page1Values[rowIndex2][columnIndex2] = val1;
        bufferManager.writeToCache(matrix->matrixName, pageIndex1, page1Values, page1Values.size());
        return;
    }

    page1Values[rowIndex1][columnIndex1] = val2;
    page2Values[rowIndex2][columnIndex2] = val1;

    bufferManager.writeToCache(matrix->matrixName, pageIndex1, page1Values, page1Values.size());
    bufferManager.writeToCache(matrix->matrixName, pageIndex2, page2Values, page2Values.size());
}


/**
 * @brief Check if matrix[i][j] == matrix[j][i]
 *
 */
bool isEqual(int row, int col, Matrix* matrix){
    int pageIndex1 = getPageIndex(row, col, matrix);
    int pageIndex2 = getPageIndex(col, row, matrix);

    Page page1 = bufferManager.getPage(matrix->matrixName, pageIndex1);
    Page page2 = bufferManager.getPage(matrix->matrixName, pageIndex2);

    int rowIndex1 = getRowIndex(row, col, matrix);
    int rowIndex2 = getRowIndex(col, row, matrix);

    int columnIndex1 = getColumnIndex(row, col, matrix);
    int columnIndex2 = getColumnIndex(col, row, matrix);

    vector<vector<int>> page1Values = getPageValues(page1);
    vector<vector<int>> page2Values = getPageValues(page2);

    int val1 = page1Values[rowIndex1][columnIndex1];
    int val2 = page2Values[rowIndex2][columnIndex2];

    return (val1 == val2);
}


/**
 * @brief For matrix[i][j] and matrix[j][i] do the compute operation
 *
 */
void computeCell(int row, int col, Matrix* matrix){
    int pageIndex1 = getPageIndex(row, col, matrix);
    int pageIndex2 = getPageIndex(col, row, matrix);

    Page page1 = bufferManager.getPage(matrix->matrixName, pageIndex1);
    Page page2 = bufferManager.getPage(matrix->matrixName, pageIndex2);

    int rowIndex1 = getRowIndex(row, col, matrix);
    int rowIndex2 = getRowIndex(col, row, matrix);

    int columnIndex1 = getColumnIndex(row, col, matrix);
    int columnIndex2 = getColumnIndex(col, row, matrix);

    vector<vector<int>> page1Values = getPageValues(page1);
    vector<vector<int>> page2Values = getPageValues(page2);

    int val1 = page1Values[rowIndex1][columnIndex1];
    int val2 = page2Values[rowIndex2][columnIndex2];

    if(pageIndex1 == pageIndex2){
        page1Values[rowIndex1][columnIndex1] = val1 - val2;
        page1Values[rowIndex2][columnIndex2] = val2 - val1;
        bufferManager.writeToCache(matrix->matrixName, pageIndex1, page1Values, page1Values.size());
        return;
    }

    page1Values[rowIndex1][columnIndex1] = val1 - val2;
    page2Values[rowIndex2][columnIndex2] = val2 - val1;

    bufferManager.writeToCache(matrix->matrixName, pageIndex1, page1Values, page1Values.size());
    bufferManager.writeToCache(matrix->matrixName, pageIndex2, page2Values, page2Values.size());
}

/**
 * @brief Check if a matrix is Symettric
 *
 */
bool Matrix::isSymmetric(){
    int dimension = this->columnCount;
    bool symmetric = true;
    for(int i=0; i<dimension; i++){
        for(int j=i; j<dimension; j++){
            symmetric &= isEqual(i, j, this);
        }
    }
    bufferManager.writeCacheToDisk();
    return symmetric;
}

/**
 * @brief Compute transpose(A) for a matrix
 *
 */
void Matrix::transpose(){
    int dimension = this->columnCount;
    for(int i=0; i<dimension; i++){
        for(int j=i+1; j<dimension; j++){
            swapPosition(i, j, this);
        }
    }
    bufferManager.writeCacheToDisk();
}


/**
 * @brief Compute A - transpose(A) for a matrix
 *
 */
void Matrix::compute(){
    int dimension = this->columnCount;
    for(int i=0; i<dimension; i++){
        for(int j=i; j<dimension; j++){
            computeCell(i, j, this);
        }
    }
    bufferManager.writeCacheToDisk();
}
