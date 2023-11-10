#include "global.h"

/**
 * @brief
 * SYNTAX: RENAME MATRIX <matrix_currentname> <matrix_newname>
 */

bool syntacticParseRENAMEMATRIX()
{
    logger.log("syntacticParseRENAMEMATRIX");
    if (tokenizedQuery.size() != 4)
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    parsedQuery.queryType = RENAME_MATRIX;
    parsedQuery.oldMatrixName = tokenizedQuery[2];
    parsedQuery.newMatrixName = tokenizedQuery[3];
    return true;
}

bool semanticParseRENAMEMATRIX()
{
    logger.log("semanticParseRENAMEMATRIX");
    if (!matrixCatalogue.isMatrix(parsedQuery.oldMatrixName))
    {
        cout << "SEMANTIC ERROR: Matrix doesn't exist" << endl;
        return false;
    }
    if (matrixCatalogue.isMatrix(parsedQuery.newMatrixName))
    {
        cout << "SEMANTIC ERROR: Matrix with name already exists" << endl;
        return false;
    }
    return true;
}

void executeRENAMEMATRIX()
{
    logger.log("executeRENAMEMATRIX");
    Matrix* matrix = matrixCatalogue.getMatrix(parsedQuery.oldMatrixName);
    matrix->renameMatrix(parsedQuery.oldMatrixName, parsedQuery.newMatrixName);
}
