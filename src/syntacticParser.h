#include "tableCatalogue.h"
#include "matrixCatalogue.h"

using namespace std;

enum QueryType
{
    CLEAR,
    CROSS,
    DISTINCT,
    EXPORT,
    EXPORT_MATRIX,
    INDEX,
    JOIN,
    LIST,
    LOAD,
    LOAD_MATRIX,
    PRINT,
    PRINT_MATRIX,
    PROJECTION,
    RENAME,
    RENAME_MATRIX,
    SELECTION,
    SORT,
    SOURCE,
    TRANSPOSE,
    CHECKSYMMETRY,
    COMPUTE,
    ORDERBY,
    GROUPBY,
    UNDETERMINED
};

enum BinaryOperator
{
    LESS_THAN,
    GREATER_THAN,
    LEQ,
    GEQ,
    EQUAL,
    NOT_EQUAL,
    NO_BINOP_CLAUSE
};


enum SelectType
{
    COLUMN,
    INT_LITERAL,
    NO_SELECT_CLAUSE
};

class ParsedQuery
{

    public:
    QueryType queryType = UNDETERMINED;

    string clearRelationName = "";

    string crossResultRelationName = "";
    string crossFirstRelationName = "";
    string crossSecondRelationName = "";

    string distinctResultRelationName = "";
    string distinctRelationName = "";

    string exportRelationName = "";

    IndexingStrategy indexingStrategy = NOTHING;
    string indexColumnName = "";
    string indexRelationName = "";

    BinaryOperator joinBinaryOperator = NO_BINOP_CLAUSE;
    string joinResultRelationName = "";
    string joinFirstRelationName = "";
    string joinSecondRelationName = "";
    string joinFirstColumnName = "";
    string joinSecondColumnName = "";

    string loadRelationName = "";
    
    string printRelationName = "";

    string projectionResultRelationName = "";
    vector<string> projectionColumnList;
    string projectionRelationName = "";

    string renameFromColumnName = "";
    string renameToColumnName = "";
    string renameRelationName = "";
    string oldMatrixName = "";
    string newMatrixName = "";

    SelectType selectType = NO_SELECT_CLAUSE;
    BinaryOperator selectionBinaryOperator = NO_BINOP_CLAUSE;
    string selectionResultRelationName = "";
    string selectionRelationName = "";
    string selectionFirstColumnName = "";
    string selectionSecondColumnName = "";
    int selectionIntLiteral = 0;

    vector<SortingStrategy> sortingStrategy;
    string sortResultRelationName = "";
    vector<string> sortColumnName;
    string sortRelationName = "";
    string resultantRelationName = "";
    string orderByRelationName = "";
    string orderByColumnName = "";
    SortingStrategy orderByStrategy = ASC;

    string groupByRelationName = "";
    string groupByColumnName = "";
    SortingStrategy groupByStrategy = ASC;
    AggregateStrategy conditionAggregateFunction = MIN;
    string conditionAggregateColumn = "";
    string conditionOperator = "";
    int conditionAttributeValue = 0;
    AggregateStrategy returnAggregateFunction = MIN;
    string returnAggregateColumn = "";

    string sourceFileName = "";
    string transposeRelationName = "";
    string checksymmetryRelationName = "";
    string computeRelationName = "";

    ParsedQuery();
    void clear();
};

bool syntacticParse();
bool syntacticParseCLEAR();
bool syntacticParseCROSS();
bool syntacticParseDISTINCT();
bool syntacticParseEXPORT();
bool syntacticParseEXPORTMATRIX();
bool syntacticParseINDEX();
bool syntacticParseJOIN();
bool syntacticParseLIST();
bool syntacticParseLOAD();
bool syntacticParseLOADMATRIX();
bool syntacticParsePRINT();
bool syntacticParsePRINTMATRIX();
bool syntacticParsePROJECTION();
bool syntacticParseRENAME();
bool syntacticParseRENAMEMATRIX();
bool syntacticParseSELECTION();
bool syntacticParseSORT();
bool syntacticParseSOURCE();
bool syntacticParseCHECKSYMMETRY();
bool syntacticParseTRANSPOSE();
bool syntacticParseCOMPUTE();
bool syntacticParseORDERBY(string resultantRelationName);
bool syntacticParseGROUPBY(string resultantRelationName);

bool isFileExists(string tableName);
bool isQueryFile(string fileName);
