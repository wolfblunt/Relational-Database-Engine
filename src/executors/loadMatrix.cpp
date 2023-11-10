#include "global.h"
// #include "../matrix.h"
// #include "../matrix/matrixCatalogue.cpp"

/**
 * @brief
 * SYNTAX: LOAD MATRIX matrix_name
 */
bool syntacticParseLOADMATRIX()
{
	logger.log("syntacticParseLOADMATRIX");
	if (tokenizedQuery.size() != 3)
	{
		cout << "SYNTAX ERROR" << endl;
		return false;
	}
	parsedQuery.queryType = LOAD_MATRIX;
	parsedQuery.loadRelationName = tokenizedQuery[2];
	return true;
}

bool semanticParseLOADMATRIX()
{
	logger.log("semanticParseLOADMATRIX");
	if (matrixCatalogue.isMatrix(parsedQuery.loadRelationName))
	{
		cout << "SEMANTIC ERROR: Matrix already exists" << endl;
		return false;
	}
	if (!isFileExists(parsedQuery.loadRelationName))
	{
		cout << "SEMANTIC ERROR: Data file doesn't exist" << endl;
		return false;
	}
	return true;
}

void executeLOADMATRIX()
{
	logger.log("executeLOADMATRIX");
	Matrix *matrix = new Matrix(parsedQuery.loadRelationName);
	if (matrix->load())
	{
		matrixCatalogue.insertMatrix(matrix);
		cout << "Loaded Matrix." << " Row Count: " << matrix->rowCount <<  " Column Count: " << matrix->columnCount << endl;
	}
	return;
}
