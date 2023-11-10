#include "global.h"

/**
 * @brief
 * SYNTAX: CHECKSYMMETRY matrix_name
 */


bool syntacticParseCHECKSYMMETRY()
{
	logger.log("syntacticParseCHECKSYMMETRY");
	if (tokenizedQuery.size() != 2)
	{
		cout << "SYNTAX ERROR" << endl;
		return false;
	}
	parsedQuery.queryType = CHECKSYMMETRY;
	parsedQuery.checksymmetryRelationName = tokenizedQuery[1];
	return true;
}

bool semanticParseCHECKSYMMETRY()
{
	logger.log("semanticParseCHECKSYMMETRY");
	if (!matrixCatalogue.isMatrix(parsedQuery.checksymmetryRelationName))
	{
		cout << "SEMANTIC ERROR: Matrix does not exists" << endl;
		return false;
	}
	return true;
}

void executeCHECKSYMMETRY()
{
	logger.log("executeCHECKSYMMETRY");
    string matrixName = parsedQuery.checksymmetryRelationName;
    Matrix* matrix = matrixCatalogue.getMatrix(parsedQuery.checksymmetryRelationName);
	bool symmetric = matrix->isSymmetric();
	if(symmetric)
		cout << "TRUE\n\n";
	else
		cout << "FALSE\n\n";

	cout << "Number of blocks read: " << BLOCKS_READ << "\n";
	cout << "Number of blocks written: " << BLOCKS_WRITTEN << "\n";
	cout << "Number of blocks accessed: " << BLOCKS_READ + BLOCKS_WRITTEN << "\n\n";
	return;
}