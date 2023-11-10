#include "global.h"

BufferManager::BufferManager()
{
    logger.log("BufferManager::BufferManager");
}

/**
 * @brief Function called to read a page from the buffer manager. If the page is
 * not present in the pool, the page is read and then inserted into the pool.
 *
 * @param tableName 
 * @param pageIndex 
 * @return Page 
 */
Page BufferManager::getPage(string tableName, int pageIndex)
{
    logger.log("BufferManager::getPage");
    string pageName = "../data/temp/"+tableName + "_Page" + to_string(pageIndex);
    if (this->inPool(pageName))
        return this->getFromPool(pageName);
    else
        return this->insertIntoPool(tableName, pageIndex);
}

Page BufferManager::getPageNotInCatalogue(string tableName, int pageIndex, int columnCount, int pageRowCount)
{
    logger.log("BufferManager::getPageNotInCatalogue");
    string pageName = "../data/temp/"+tableName + "_Page" + to_string(pageIndex);
    

    vector<vector<int>> rows(pageRowCount, vector<int>(columnCount, 0));

    ifstream fin(pageName, ios::in);
    
    int number;
    for (uint rowCounter = 0; rowCounter < pageRowCount; rowCounter++)
    {
        for (int columnCounter = 0; columnCounter < columnCount; columnCounter++)
        {
            fin >> number;
            rows[rowCounter][columnCounter] = number;
        }
    }
    fin.close();

    Page p(tableName, pageIndex, rows, pageRowCount);
    return p;
}

/**
 * @brief Checks to see if a page exists in the pool
 *
 * @param pageName 
 * @return true 
 * @return false 
 */
bool BufferManager::inPool(string pageName)
{
    logger.log("BufferManager::inPool");
    for (auto page : this->pages)
    {
        if (pageName == page.first.pageName)
            return true;
    }
    return false;
}

/**
 * @brief If the page is present in the pool, then this function returns the
 * page. Note that this function will fail if the page is not present in the
 * pool.
 *
 * @param pageName 
 * @return Page 
 */
Page BufferManager::getFromPool(string pageName)
{
    logger.log("BufferManager::getFromPool");
    for (auto page : this->pages)
        if (pageName == page.first.pageName)
            return page.first;
}

/**
 * @brief Inserts page indicated by tableName and pageIndex into pool. If the
 * pool is full, the pool ejects the oldest inserted page from the pool and adds
 * the current page at the end. It naturally follows a queue data structure. 
 *
 * @param tableName 
 * @param pageIndex 
 * @return Page 
 */
Page BufferManager::insertIntoPool(string tableName, int pageIndex)
{
    logger.log("BufferManager::insertIntoPool");
    Page page(tableName, pageIndex);
    BLOCKS_READ++;
    if (this->pages.size() >= BLOCK_COUNT){
        Page frontPage = pages.begin()->first;
        if(pages.begin()->second == 1){
            //dirty bit is one so we need to write to disk
            writePage(frontPage.getTableName(), frontPage.getPageIndex(), frontPage.getRows(), frontPage.getRowCount());
        }
        pages.pop_front();
    }
    pages.push_back({page, 0});
    return page;
}


/**
 * @brief write to the cache but not to disk. This saves disk accesses.
 */
void BufferManager::writeToCache(string tableName, int pageIndex, vector<vector<int> > &rows, int rowCount)
{
    logger.log("BufferManager::writeToCache");
    string pageName = "../data/temp/"+tableName + "_Page" + to_string(pageIndex);
    Page newPage(tableName, pageIndex, rows, rowCount);
    if(!inPool(pageName))
        insertIntoPool(tableName, pageIndex);

    for (auto &page : this->pages){
        if (pageName == page.first.pageName){
            page.first = newPage;
            page.second = 1;
            return;
        }
    }
    assert(1 == 0);
}


/**
 * @brief Scan through the cache and write the page to disk if required.
 */
void BufferManager::writeCacheToDisk(){
    for (auto &page : this->pages){
        if(page.second == 1){
            //dirty bit is one so we need to write to disk
            writePage(page.first.getTableName(), page.first.getPageIndex(), page.first.getRows(), page.first.getRowCount());
        }
    }
}

/**
 * @brief The buffer manager is also responsible for writing pages. This is
 * called when new tables are created using assignment statements.
 *
 * @param tableName 
 * @param pageIndex 
 * @param rows 
 * @param rowCount 
 */
void BufferManager::writePage(string tableName, int pageIndex, vector<vector<int> > rows, int rowCount)
{
    logger.log("BufferManager::writePage");
    Page page(tableName, pageIndex, rows, rowCount);
    BLOCKS_WRITTEN++;
    page.writePage();
}

/**
 * @brief Deletes file names fileName
 *
 * @param fileName 
 */
void BufferManager::deleteFile(string fileName)
{
    
    if (remove(fileName.c_str()))
        logger.log("BufferManager::deleteFile: Err");
    else 
        logger.log("BufferManager::deleteFile: Success");
}

/**
 * @brief Overloaded function that calls deleteFile(fileName) by constructing
 * the fileName from the tableName and pageIndex.
 *
 * @param tableName 
 * @param pageIndex 
 */
void BufferManager::deleteFile(string tableName, int pageIndex)
{
    logger.log("BufferManager::deleteFile");
    string fileName = "../data/temp/" + tableName + "_Page" + to_string(pageIndex);
    this->deleteFile(fileName);
}

void BufferManager::clearCache(){
    pages.clear();
}


