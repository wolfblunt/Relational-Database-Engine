
  

  

<h2><ins>Project Phase 2 - Report</ins></h2>



  

  

******************************************************************************************

  

<h3><ins>ASSUMPTIONS</ins></h3>

  

- The numbers in the syntax are not comma formatted (Eg, 70,000 is assumed to be 70000)

- The size of a record is less than or equal to the block size.

- For ORDER BY, ordering is on a single column.

- For JOIN, joining is on a single column

- For GROUP BY, there is onle a single grouping column, aggregate column (for HAVING) and return column.

  
  

<br>

  
  

<h2>SORT TABLE</h2>

- SYNTAX : **SORT \<table_name\> BY <column_name_1, column_name_2,  ...  ,  column_name_k> IN <ASC|DESC, ASC|DESC,...,  ASC|DESC>**

  

<h3>Syntactic Checks</h3>

  

- Token 1 must be the word SORT (assuming 1 based indexing)

- Token 3 must be the word BY

- We read the column names until we find the word IN. Syntax error if the word IN does not exist.

- Read the sorting strategy (ASC or DESC) till end of tokens.

- Length of the list of column names and the list of sorting strategies must be the same

  
  

<h3>Semantic Checks</h3>

  

- The relation must exist in the table catalogue.

- The column names must exist in the table.

  

<h3>Algorithm</h3>

  

- Extract the column indices from the column names.

- Recursively break down the page range till we can fit all the pages in the buffer. Eg. if the pages range from 0 to 11 and we can fit 4 pages in buffer, we will break (0, 11) into (0, 3), (4, 7), (8, 11). The range (0, 3) will be further broken into (0, 1), (2, 2), (3, 3).

- Fetch each of these small page ranges into the memory and internally sort them using a custom comparator.

- Now we need to merge these small ranges. If we want to merge $k$ partitions (a range of sorted pages) we will need $k$ pointers for each partition.

- We will fetch a row from each of these partitions and put them in a priority queue based on our custom comparator.

- The priority queue will return the first element of the new range which we will put in the reserve space of 1 block in our buffer. We will also put a new element in the priority queue from the required partition.

- When the reserve block becomes full, we write it to the disk.

- We iterate till the priority queue becomes empty and we have merged all the partitions into a single partition.

- When all the blocks are written into a partition we overwrite the original partition in the relation with the new one since we need to do the sorting in place.

- Doing this recursively we merge till we have a single partition containing the entire range and we have successfully sorted.

- Export the new relation into a csv file using the makePermanent function.

  
  

<h2> JOIN </h2>

  

- SYNTAX : **\<new_relation_name\> <- JOIN \<tablename1\>, \<tablename2\> ON \<column1\>  \<bin_op\>  \<column2\>**

<h3> Syntactic Checks </h3>

- Token 2 must be <- (assuming 1-based indexing).

- Token 3 must be JOIN.

- Token 6 must be must be ON.

- The only permissible binary operators are >, <, >=, <=, ==.

  

<h3> Semantic Checks </h3>

- The new table mustn't already exist.

- The 2 relations upon which JOIN is to be performed, they also must exist previously.

- The attributes upon which JOIN is applied, must exist in the corresponding tables.

  

<h3> Algorithm </h3>

- 2 copies of the pre-existing tables are created and all the necessary attribute values (such as sourceFileName, tableName, columns, columnCount, rowCount, blockCount, etc.) are assigned as per requirement.

- Both the tables are sorted on the corresponding JOIN columns provided in the JOIN query.

- Column names for the new resultant table are added in a sequential manner - the 1st table's column names, followed by the 2nd table's.

- Blocks are fetched for the corresponding tables in a nested loop - reason being that there's no guarantee that the volumn values will be unique, any number of repetitions can occur - hence the worst case scenario is being taken into consideration here.

- For the row-level comparison (within a block), in order to improve the average (and best) case time complexities, a concept of "checkpoint" is used.

- In case of the equality comparison operator, the checkpoint is maintained in such a way that the iteration is done from a range of matching indices only.

- In case of the less than (and less than or equal to) comparison operator, condition checking for a particular column value is done till a particular checkpoint only, as beyond that point, the values get larger and don't satisfy the JOIN criteria.

- In case of the greater than (and greater than or equal to) comparison operator, the 2 tables are reversed and the same criterion of less than (or less than equal to) is applied.

- After the JOIN is done, the 2 temporary tables are deleted and the resultant table is inserted into the tableCatalogue.

  

<h2> ORDER BY </h2>

  

- SYNTAX - **\<new_table\> <- ORDER BY \<attribute\> ASC|DESC ON \<table_name\>**

  

<h3> Syntactic Checks </h3>

  

- Token 2 must be <- (assuming 1 based indexing)

- Token 3 must ORDER

- Token 4 must be BY

- Token 6 must be ASC or DESC

- Token 7 must be ON

  

<h3> Semantic Checks </h3>

  

- The new table must not already exist.

- The relation which we are ordering must exist.

- The attribute must exist in the ordering relation.

  

<h3> Algorithm </h3>

  

- Make a copy of the table with the resultant table name and copy all the pages.

- Sort this copy in place using the sort function and insert it into the table catalogue.

  

<h2> GROUP BY </h2>

  

- SYNTAX - **\<new_table\> <- GROUP BY \<grouping_attribute\> FROM \<table_name\> HAVING \<aggregate(attribute)\> \<bin_op\> \<attribute_value\> RETURN \<aggregate_func(attribute)\>**

  

<h3> Syntactic Checks </h3>

  

- Number of tokens must be 13

- Token 2 must be <- (assuming 1 based indexing)

- Token 3 must be GROUP

- Token 4 must be BY

- Token 6 must be FROM

- Token 8 must be HAVING

- Extract the aggregate function name and attribute from token 9. The aggregate function must be MAX/MIN/AVG/SUM.

- Token 10 must be an operator (<, >, <=, >= or ==)

- Token 12 must be RETURN

- Extract the aggregate function name and attribute from token 13. The aggregate function must be MAX/MIN/AVG/SUM.

  
  

<h3> Semantic Checks </h3>

  

- The resultant relation name must not already exist.

- The grouping relation must exist in the table catalogue.

- Both the attributes and the grouping column must exist in the relation.

<h3> Algorithm </h3>

- Make a copy of the original table and perform the following operations on the copy.

- Sort the table based on the grouping attribute.

- Now iterate from the start of the relation and maintain two variables to store the aggregate values for the HAVING and RETURN clauses.

- The records with same value for grouping attribute are adjacent to each other so we can group them easily and calculate the aggregate values.

- If the aggregate value of the HAVING clause of a chunk does not satisfy the condition, we discard the aggregate value of the RETURN clause.

- Otherwise insert the aggregate value of the RETURN clause and the grouping value into a new relation.

- Delete the copy. In this way, the original relation is not modified.

  

  

******************************************************************************************

  

  

<h3><ins>LEARNINGS</ins></h3>

  

- How external sorting works in databases with large number of records.

- Various applications of sorting in different commands.

- How to optimize different commands by sorting relations first.

- Implementing simple algorithms like (merging sorted lists) while dealing with blocks due to large list sizes.