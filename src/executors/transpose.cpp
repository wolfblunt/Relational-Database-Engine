#include "global.h"

/**
 * @brief
 * SYNTAX: TRANSPOSE matrix_name
 */


bool syntacticParseTRANSPOSE()
{
	logger.log("syntacticParseTRANSPOSE");
	if (tokenizedQuery.size() != 3)
	{
		cout << "SYNTAX ERROR" << endl;
		return false;
	}
	parsedQuery.queryType = TRANSPOSE;
	parsedQuery.transposeRelationName = tokenizedQuery[2];
	return true;
}

bool semanticParseTRANSPOSE()
{
	logger.log("semanticParseTRANSPOSE");
	if (!matrixCatalogue.isMatrix(parsedQuery.transposeRelationName))
	{
		cout << "SEMANTIC ERROR: Matrix does not exists" << endl;
		return false;
	}
	return true;
}

void executeTRANSPOSE()
{
	logger.log("executeTRANSPOSE");
	string matrixName = parsedQuery.transposeRelationName;
    Matrix* matrix = matrixCatalogue.getMatrix(parsedQuery.transposeRelationName);
	matrix->transpose();

	cout << "Number of blocks read: " << BLOCKS_READ << "\n";
	cout << "Number of blocks written: " << BLOCKS_WRITTEN << "\n";
	cout << "Number of blocks accessed: " << BLOCKS_READ + BLOCKS_WRITTEN << "\n\n";
	return;
}