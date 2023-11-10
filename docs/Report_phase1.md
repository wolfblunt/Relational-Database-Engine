
<h2><ins>Project 1 - Report</ins></h2>


  ******************************************************************************************
  
<h3><ins>ASSUMPTIONS</ins></h3>

- Matrix provided should be a square matrix.
- The rows and columns  should contain integers and not any blank or null values.
- The input should be provided so that there are no overflows in the COMPUTE operation.
- The matrix should not be so large that the pages (int data/temp) do not fit into the disk.

******************************************************************************************

  

<h3><ins>DESIGN CHANGES</ins></h3>
There are mainly 2 design modifications. One for the pages when the row size exceeds a page size and one for buffer management.


**PAGE DESIGN MODIFICATION**

 To divide a matrix into blocks, there are 2 possible cases-
 - <ins>The size of a row is less than the page size</ins> - In this case each block will contain at least one full row. If a row cannot fit entirely into a block, it will be inserted into the next block.

	Eg. If we have an 16x16 matrix and a page can contain at most 40 elements, we will get 8 pages each having a 2x16 matrix (containing 32 elements). 
	
-	<ins>The size of a row is greater than the page size</ins> - In this case even a single row cannot fir entirely into a block. A row is divided into segments and each page contains only one row but as many elements from one row of the matrix as possible.

	Eg. If we have a 8x8 matrix and a page can contain at most 3 elements, the first row of the matrix will span across Page0, Page1, Page2 containing 1x3, 1x3, 1x2 elements respectively. Similarly other rows will also need 3 pages each totalling 24 pages.

Assuming a page size of 1KB, for a 1001x1001 matrix which consumes about 4MB of space, the internal fragmentation will be at most 1MB (about 25% of the matrix size). For a 10001x10001 matrix which consumes about 400MB of space, the internal fragmentation will be about 10MB (about 2.5% of the matrix size). Thus the total disk space consumed is always reasonable and we can comfortably handle even large matrices with this page design.
<br>

**BUFFER DESIGN MODIFICATION**

For commands like transpose, checksymmetry and compute, we need to fetch pages from the disk using the buffer manager. The current problem is that the memory in the buffer may become stale when we write to the disk.

Suppose we need to read from Page0 and Page1. These blocks are fetched from memory and stored in the cache. Next lets say we write to Page0. If we write directly to the disk, there is a state mismatch between Page0 in the cache and Page0 on the disk. Subsequent reads for Page0 will hit the cache and the cache will provide the stale Page0 value.

There are two possible approaches to solve this-
- <ins> Write Through Method </ins> - In this case if we want to write to a page, we write to BOTH the memory and the disk. This way the cache value of a page and the disk value of a page are always matching. But this makes the cache useless in write operations as each write always does a block access in the disk.

- <ins> Write Back Method </ins> - In this case we have a dirty bit corresponding to each page in the cache which is 1 if it has been modified and 0 otherwise. We write only to the page in the cache and turn the dirty bit to one. Once a page needs to be evicted from the cache, if the dirty bit is one, we write is to the disk otherwise we simply discard it.

<b> We have used the Write Back Method for buffer management.</b>
 
  

******************************************************************************************

  
  

<h3><ins>DETAILS OF COMMANDS IMPLEMENTED</ins></h3>

  

**LOAD MATRIX**

- SYNTAX : **LOAD MATRIX <matrix_name>**
- First get the metadata of the matrix. This includes the row/column count, maximum number of rows per block and the maximum number of integers that can fit into a block. For a page size of 1KB, the maximum number of elements in a block is 250.
- Now we have to divide the matrix into blocks. This is done using the scheme mentioned in the page design section.
- Parse the matrix file and write to the disk.
- Insert an entry for the matrix in the matrix catalogue.
- Number of blocks accessed is the same as the number of blocks required for the matrix.
- We have used syntactic checks (the number of tokens should be exactly 3) and semantic checks (matrix should not be already present in the matrix catalogue and the csv file should be present on the disk).
	

  

**PRINT MATRIX**

- SYNTAX : **PRINT MATRIX <matrix_name>**
- First, fetch the details of the matrix cursor (pageDetails, pageIndex, matrixName) that we want to print.
- Then, check if the number of elements present in a row is greater than the page size or not. To do this, keep track using a class variable.
- If the number of row elements is less than the page size, fetch the page blocks from temporary memory and print them on the console.
- If the number of row elements is greater than the page size, calculate the block/page number in which the current row is saved. Based on this, iterate through the number of blocks to fetch the current row and display it on the console.
- If the size of the matrix (n) is large (n > 20), only print the first 20 rows and columns.

  

**RENAME MATRIX**

- SYNTAX : **RENAME MATRIX <old_matrix_name>  <new_matrix_name>**
- Syntactic checks (number of inputs in the terminal) are done initially, followed by semantic checks (whether the *old_matrix* is actually there, or the *new_matrix* is already there).
- The matrix pointer for *old_matrix* is obtained from matrix_catalogue.
- The relevant parameters are then sent to *renameMatrix()* function in *matrix.cpp*.
- The page files of the *old_matrix* (located in ../data/temp) are renamed to the corresponding *new_matrix*'s.
- The *matrixName* and *sourceFileName* of the *old_matrix* are changed to the *new_matrix*'s, after retrieving the *old_matrix*'s pointer from *matrixCatalogue.cpp*.
- The entry associated with the *new_matrix* is inserted into the unordered_map of *matrixCatalogue* and then the entry associated with the *old_matrix* is deleted. 

  

**EXPORT MATRIX**

- SYNTAX : **EXPORT MATRIX <matrix_name>**
- Initially, Syntactic checks (number of inputs in the terminal) and Semantic checks (whether the matrix actually exists in the unordered_map associated with *matrixCatalogue*) are performed.
- A check is performed whether the current matrix is already exported or not, if not already exported, then the corresponding sourceFile is deleted.
- Matrix values are written into the corresponding *sourceFile* according to the way in which the row and column elements are stored in blocks.
  

**TRANSPOSE MATRIX**

- SYNTAX : **TRANSPOSE MATRIX <matrix_name>**
- For a matrix of size *nxn* we iterate over all pairs ($i$, $j$) such that $1 \leq i \lt j \leq n$ and we swap the ($i$, $j$) element of the matrix with the ($j$, $i$) element of the matrix.
- For this we calculate the page index for both of these elements and fetch the pages to the memory using the buffer manager.
- Next find the row index and column index for each of these values inside their respective pages. 
- Create new pages with the modified cell values and write to the cache using the Write Back Method.
- The number of blocks read and written is the same as each block which is read is modified and eventually evicted from the cache. Without the use of any cache or buffer manager, we will have around $2*n^2$ page access. ($n^2/2$ pairs and each pair reads/writes 2 pages). With the write back cache policy, this number reduces a lot even for a cache of size 2 for a sufficiently large $n$.
- Syntactic checks (the number of tokens should be exactly 2) and semantic checks (matrix should be already present in the matrix catalogue) have been used.

  

**CHECKSYMMETRY**

- SYNTAX : **CHECKSYMMETRY <matrix_name>**
- For a matrix of size *nxn* we iterate over all pairs ($i$, $j$) such that $1 \leq i \lt j \leq n$ and we swap the ($i$, $j$) element of the matrix with the ($j$, $i$) element of the matrix.
- For this we calculate the page index for both of these elements and fetch the pages to the memory using the buffer manager.
- Next find the row index and column index for each of these values inside their respective pages. 
- Read the cell values and check if they are equal. If they are unequal, the matrix is not symmetric.
- The number of blocks written is 0 as no modifications are needed to check equality. Similar to TRANSPOSE, there are about $n^2$ reads without any cache which reduces a lot with a cache.
- Syntactic checks (the number of tokens should be exactly 2) and semantic checks (matrix should be already present in the matrix catalogue) have been used.

  

**COMPUTE**

- SYNTAX : **COMPUTE <matrix_name>**
- First copy the matrix pages and name the new pages with the format <matrix_name>_RESULT<page_number>.
- Next create a new entry in the matrix catalogue for this new matrix by copying the metadata (the metadata remains the same for both matrices).
- Next we perform in-place operation by using the steps below
-  For a matrix of size *nxn* we iterate over all pairs ($i$, $j$) such that $1 \leq i \lt j \leq n$ and we swap the ($i$, $j$) element of the matrix with the ($j$, $i$) element of the matrix.
- For this we calculate the page index for both of these elements and fetch the pages to the memory using the buffer manager.
- Next find the row index and column index for each of these values inside their respective pages. 
- Find the values $val1$ and $val2$ inside the cells of the page.
- Create new pages such that cell ($i$, $j$) contains $val1 - val2$ and cell ($j$, $i$) contains $val2 - val1$ and write to the memory.
- The number of blocks read and written is the same as each block which is read is modified and eventually evicted from the cache. Without the use of any cache or buffer manager, we will have around $2*n^2$ page access. ($n^2/2$ pairs and each pair reads/writes 2 pages). With the write back cache policy, this number reduces a lot even for a cache of size 2 for a sufficiently large $n$.
- Syntactic checks (the number of tokens should be exactly 2) and semantic checks (matrix should be already present in the matrix catalogue) have been used.

  

******************************************************************************************

<h3><ins>LEARNINGS</ins></h3>

- We learnt how to create **matrix** and **matrixCatalogu**e source files and classes, how they are similar to that of Relations and how they differ from them.
- We learnt how to add custom functions to the page and bufferManager source files when the row of a matrix is greater than the block size.
- We learnt about various edge cases and tested for them and included the necessary things in our code.
- We learnt about various buffer management techniques like write through and write back algorithms.

 
******************************************************************************************
