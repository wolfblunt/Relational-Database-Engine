//#include "cursor.h"
#include <unordered_set>
#include <unordered_map>

using namespace std;

class Matrix
{
    public:
    string sourceFileName = "";
    string matrixName = "";
    uint columnCount = 0;
    long long rowCount = 0;
    uint blockCount = 0;
    uint maxRowsPerBlock = 0; // max number of rows in a block (can be 0 if row size is larger)
    vector<uint> rowsPerBlockCount; // contains the number of rows in each page respectively
    int maxColumn = 20;
    int maxElementsPerBlock = 0;

    
    bool extractMetadata(string firstLine);
    bool blockify();
    // void updatint maxElemeneStatistics(vector<int> row);
    Matrix();
    Matrix(string matrixName);
    bool load();
    void renameMatrix(string oldName, string newName);
    // void print();
    void printMatrix();
    void makePermanent();
    bool isPermanent();
    void getNextPage(Cursor* cursor);
    // Cursor getCursor();
    void unload();
    bool isSymmetric();
    void transpose();
    void compute();
    void copyMatrix(string destMatrixName);

    /**
     * @brief Static function that takes a vector of valued and prints them out in a
     * comma seperated format.
     *
     * @tparam T current usages include int and string
     * @param row 
     */
    template <typename T>
    void writeRow(vector<T> row, ostream &fout)
    {
        logger.log("Matrix::printRow");
        // int ssize = row.size();
        // int ccsize = min(ssize, maxColumn);
        for (int columnCounter = 0; columnCounter < row.size(); columnCounter++)
        {
            if (columnCounter != 0)
                fout << ", ";
            fout << row[columnCounter];
        }
        fout << endl;
    }   


    /**
     * @brief Static function that takes a vector of valued and prints them out in a
     * comma seperated format.
     *
     * @tparam T current usages include int and string
     * @param row 
     */
    template <typename T>
    void writePrintRow(vector<T> row, ostream &fout)
    {
        logger.log("Matrix::writePrintRow");
        int ssize = row.size();
        int ccsize = min(ssize, maxColumn);
        for (int columnCounter = 0; columnCounter < ccsize; columnCounter++)
        {
            if (columnCounter != 0)
                fout << ", ";
            fout << row[columnCounter];
        }
        fout << endl;
    } 


    /**
     * @brief Static function that takes a vector of valued and prints them out in a
     * comma seperated format.
     *
     * @tparam T current usages include int and string
     * @param row 
     */
    template <typename T>
    void writeRow(vector<T> row)
    {
         logger.log("Matrix::printRow");
         ofstream fout(this->sourceFileName, ios::app);
         this->writeRow(row, fout);
         fout.close();
    }
};
