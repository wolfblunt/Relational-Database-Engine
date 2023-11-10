#include "global.h"

void MatrixCatalogue::insertMatrix(Matrix* matrix)
{
    logger.log("MatrixCatalogue::~insertMatrix"); 
    this->matrices[matrix->matrixName] = matrix;
}

void MatrixCatalogue::deleteMatrix(string matrixName, bool unload)
{
    logger.log("MatrixCatalogue::deleteMatrix");
    if (unload) 
        this->matrices[matrixName]->unload();
    //delete this->tables[tableName];
    this->matrices.erase(matrixName);
}

Matrix* MatrixCatalogue::getMatrix(string matrixName)
{
    logger.log("MatrixCatalogue::getMatrix"); 
    if (this->matrices.find(matrixName) == this->matrices.end())
        return NULL;
    Matrix *matrix = this->matrices[matrixName];
    return matrix;
}

bool MatrixCatalogue::isMatrix(string matrixName)
{
    logger.log("MatrixCatalogue::isMatrix"); 
    if(this->matrices.count(matrixName))
        return true;
    return false;
}

void MatrixCatalogue::print()
{
    logger.log("MatrixCatalogue::print"); 
    cout << "\n\nMATRICES" << endl;
    int rowCount = 0;
    for (auto rel : this->matrices)
    {
        cout << rel.first << endl;
        rowCount++;
    }
    printRowCount(rowCount);
}

MatrixCatalogue::~MatrixCatalogue()
{
    logger.log("MatrixCatalogue::~MatrixCatalogue"); 
    for(auto table: this->matrices)
    {
        table.second->unload();
        delete table.second;
    }
}
