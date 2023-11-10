#include "global.h"
/**
 * @brief
 * SYNTAX: PRINT MATRIX matrix_name
 */
bool syntacticParsePRINTMATRIX()
{
	logger.log("syntacticParsePRINTMATRIX");
	if (tokenizedQuery.size() != 3)
	{
		cout << "SYNTAX ERROR" << endl;
		return false; 
	}
	parsedQuery.queryType = PRINT_MATRIX;
	parsedQuery.printRelationName = tokenizedQuery[2];
	return true;
}

bool semanticParsePRINTMATRIX()
{
	logger.log("semanticParsePRINTMATRIX");
	if (!matrixCatalogue.isMatrix(parsedQuery.printRelationName))
	{
		cout << "SEMANTIC ERROR: Matrix doesn't exist" << endl;
		return false;
	}
	return true;
}

void executePRINTMATRIX()
{
	logger.log("executePRINTMATRIX");
	Matrix* matrix = matrixCatalogue.getMatrix(parsedQuery.printRelationName);
	matrix->printMatrix();
	return;
}
