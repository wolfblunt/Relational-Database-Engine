#include "global.h"

/**
 * @brief
 * SYNTAX: COMPUTE matrix_name
 */


bool syntacticParseCOMPUTE()
{
	logger.log("syntacticParseCOMPUTE");
	if (tokenizedQuery.size() != 2)
	{
		cout << "SYNTAX ERROR" << endl;
		return false;
	}
	parsedQuery.queryType = COMPUTE;
	parsedQuery.computeRelationName = tokenizedQuery[1];
	return true;
}

bool semanticParseCOMPUTE()
{
	logger.log("semanticParseCOMPUTE");
	if (!matrixCatalogue.isMatrix(parsedQuery.computeRelationName))
	{
		cout << "SEMANTIC ERROR: Matrix does not exists" << endl;
		return false;
	}
	return true;
}

void executeCOMPUTE()
{
	logger.log("executeCOMPUTE");
    Matrix *matrix = matrixCatalogue.getMatrix(parsedQuery.computeRelationName);

	// copy the pages
	matrix->copyMatrix(parsedQuery.computeRelationName + "_RESULT");

	// copy the metadata and change sourcefilename and matrix name
	Matrix *computeResultPtr = new Matrix();
	computeResultPtr->sourceFileName = parsedQuery.computeRelationName + "_RESULT" + ".csv";
	computeResultPtr->matrixName = parsedQuery.computeRelationName + "_RESULT";
	computeResultPtr->columnCount = matrix->columnCount;
    computeResultPtr->rowCount = matrix->rowCount;
    computeResultPtr->blockCount = matrix->blockCount;
    computeResultPtr->maxRowsPerBlock = matrix->maxRowsPerBlock;
    computeResultPtr->rowsPerBlockCount = matrix->rowsPerBlockCount;
	computeResultPtr->maxElementsPerBlock = matrix->maxElementsPerBlock;
	matrixCatalogue.insertMatrix(computeResultPtr);

	computeResultPtr->compute();

	cout << "Number of blocks read: " << BLOCKS_READ << "\n";
	cout << "Number of blocks written: " << BLOCKS_WRITTEN << "\n";
	cout << "Number of blocks accessed: " << BLOCKS_READ + BLOCKS_WRITTEN << "\n\n";
	return;
}