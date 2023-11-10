#include "matrix.h"

/**
 * @brief The MatrixCatalogue acts like an index of matrices existing in the
 * system. Everytime a matrix is added(removed) to(from) the system, it needs to
 * be added(removed) to(from) the MatrixCatalogue. 
 *
 */
class MatrixCatalogue
{

    unordered_map<string, Matrix*> matrices;

    public:
        //unordered_map<string, Matrix*> matrices;
        MatrixCatalogue() {}
        void insertMatrix(Matrix* Matrix);
        void deleteMatrix(string matrixName, bool unload);
        Matrix* getMatrix(string matrixName);
        bool isMatrix(string matrixName);
        //bool isColumnFromMatrix(string columnName, string tableName);
        void print();
        ~MatrixCatalogue();
};
