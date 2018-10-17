#include "sorter_thread.h"
#include "mergesort.c"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <dirent.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <limits.h>
#include <sys/mman.h>
#include <pthread.h>
//Use this to compile
// gcc -pthread sorterSolo.c -o sorterSolo
//Use this to run
//./sorterSolo -c col2 -d /ilab/users/nap166/Desktop/testing2/aFolder -o /ilab/users/nap166/Documents/dani/aFolder

 unsigned long tidArray[100000];

pthread_mutex_t lock1; // Used for Directory detection
pthread_mutex_t lock2; // Used for CSV detection
pthread_mutex_t lock3;  // Used for end of oldMain
pthread_mutex_t lockColIndex; // used for gloIndexOfColumnToSort
pthread_mutex_t lockNumCols; // used for gloNumOfColumns
pthread_mutex_t lockTypeSort; //used for typeOfSort
pthread_mutex_t lockReNQ;
pthread_mutex_t lockID1;
pthread_mutex_t lockID2;
 

int tidLock = 0;
typedef struct{
	char* gloColumnToSort;
	char* gloOutputPath;
	int gloNumOfColumns;
	int gloIndexOfColumnToSort;
	int gloTypeOfSort;
	int queueAmt;
	int gloTotalThreads;
}GLOBAL;

GLOBAL gloVars={
	.gloNumOfColumns = -1,
	.gloIndexOfColumnToSort = -1,
	.gloTypeOfSort = -1,
	.queueAmt = 0, //Number of nodes inside the queueue	
	.gloTotalThreads = 0,
};

typedef struct test{
	char *directoryPath;
	char *fileName;
}p;


struct qNode{
	struct movieMeta* data;
	struct qNode* next;
};
struct qNode* qFront = NULL;
struct qNode* qRear  = NULL;





//Returns 1 if string has letters; else 0
int hasLetters(char* str){
	//check if it has letters or symbols
	if (strlen(str) < 1)
		return 0;

	int hasLets = 0;
	int x = 0;
	for (x = 0; x < strlen(str); x++)
	{
		if((str[x] >= 'A' && str[x] <= 'Z') ||
		   (str[x] >= 'a' && str[x] <= 'z'))
		{
			hasLets = 1;
			break;
		}
	}

	return hasLets;
}
//Returns 1 if string has a . ; else 0
int hasDecimal(char* str){
	if (strlen(str) < 1)
		return 0;
	return indexOf(str, ".");
}
//Returns 1 if string has a number from 0~9
int hasNumbers(char* str){
	if (strlen(str) < 1)
		return 0;

	//check if has numbers and no letters
	int hasNums = 0;
	int x = 0;
	for (x = 0; x < strlen(str); x++)
	{
		if(str[x] >= '0' && str[x] <= '9')
		{
			hasNums = 1;
			break;
		}
	}

	return hasNums;
}
//Determines the type of sort based on the contents of the field
int determineTypeOfSort(struct movieMeta *movie, int numOfRows, int numOfColumns, int indexOfColumnToSort){
	int r= 0;
	int a = 0, b = 0, c = 0;
	//printf("a %d ; b %d; c %d; numOfRows %d; numOfColumns %d; indexToSort %d\n", a, b, c, numOfRows, numOfColumns, indexOfColumnToSort);
	for (r = 0; r < numOfRows; r++)
	{
		if (a == 0)
			a = hasLetters(movie[r].data[indexOfColumnToSort]);

		if (b == 0)
			b = hasDecimal(movie[r].data[indexOfColumnToSort]);

		if (c == 0)
			c = hasNumbers(movie[r].data[indexOfColumnToSort]);
	}

	//printf("a %d ; b %d; c %d\n", a, b, c);
	if (a == 1)
		return 0; // String no matter what it is
	else
	{
		if ((b == 0 && c == 0) || (b == 1 && c == 0))
			return 0; // String is null  || String is ......
		else if (b == 0 && c == 1)
			return 1; // Long
		else if (b == 1 && c == 1)
			return 2; // Double
		}

	return -1;
}
//checks is string is empty
int isEmpty(const char *word) {
  while (*word != '\0') {
	if (!isspace((unsigned char)*word))
	  return 0;
	word++;
  }
  return 1;
}
//Trims whitespace around the word
char* trimWord(char* orgWord){
	//orgWord=original Word and newWord=trimmed word. Strings in C are array of chars
	if(orgWord==NULL){
		return NULL;
	}

	if(orgWord[0]=='\0'){
		return orgWord;
	}
	//checks if word is entirely spaces. "     "


	int isSpace = isEmpty(orgWord);

	if(isSpace==1){
		orgWord="";
	}
	else if (isSpace == 0){

		int i=0;
		int start; //index position of start of trimmed word
		int length=strlen(orgWord); //length of word
		int j=length; //j to start at end
		int stop=length; //index of where the last position of a letter is starting from the back
		start=i; //start to equal i just in case there is no beginninng space.

		while(orgWord[i]==' '){
			i++;//if its a space goes to next position
			start=i; //gets the new position of the string
		}//at the end of the loop, start will be at the index of the first letter position

		while(orgWord[j-1]==' '){
			j=j-1;
			stop=j;
		}

	   int lengthOfSubstr = stop - start; //length of string
	   orgWord=orgWord + start;//orgword starts at beginning;
	   char* newString=strdup(orgWord);
	   newString[lengthOfSubstr]='\0';
	   return newString;
   }

   return orgWord;//trying to do a substring method to get from start to stop.
}
//loops through and counts the number of columns detected in the csv file
int getNumOfColumns(char*tempRow){
	char* currentHeaderValue;
	int columnCounter = 0;
	currentHeaderValue = strtok(tempRow,",");

	while (currentHeaderValue!=NULL) {
		columnCounter++;
		currentHeaderValue = strtok(NULL, ",\n");
	}
	return columnCounter;
}
//returns the index of the column we need to sort
int getSortingColumnIndex(char* headerNames[], int numOfHeadColumns, char* columnToSort){
	int i;
	//printf("headerName: '%s' vs '%s'\n", headerNames[0], columnToSort);
	for (i = 0; i < numOfHeadColumns; i++)
	{
		if (strcmp(headerNames[i], columnToSort) == 0)
			return i;
	}

	return -1;
}
//checks if current line is the last line of the table
int isAtEndOfTable(int currentLine, int totalRows){
	if (currentLine == (totalRows-1))
		return 1;

	return 0;
}
//prints out struct with appropriate format
void confirmationPrint(struct movieMeta* movie, int numOfRows, int numOfColumns){
	//-- prints the contents of the movie structure
	int r= 0;
	int c = 0;
	for (r = 0; r < numOfRows; r++)
	{
		for (c = 0; c < numOfColumns ; c++)
		{
			//actual csv output

			if (indexOf(movie[r].data[c], ",") == 1)
				printf("\"%s\",", movie[r].data[c]); // put quotations around field and add comma
			else
				printf("%s,",movie[r].data[c]); // add a comma


			//debugging
			//printf("[%s]\t",movie[r].data[c]);
		}
		printf("\n");
	}
}
void csvPrint(struct movieMeta* movie, int numOfRows, int numOfColumns, FILE * fileOutput){
	//-- prints the contents of the movie structure
	int r= 0;
	int c = 0;
	for (r = 0; r < numOfRows; r++)
	{
		for (c = 0; c < numOfColumns ; c++)
		{
			//actual csv output

			if (indexOf(movie[r].data[c], ",") == 1){
				//printf("\"%s\",", movie[r].data[c]); // put quotations around field and add comma
				fprintf(fileOutput, "\"%s\",", movie[r].data[c]); // put quotations around field and add comma
			}
			else{
				//printf("%s,",movie[r].data[c]); // add a comma
				  fprintf(fileOutput, "%s,", movie[r].data[c]);
                 }

			//debugging
			//printf("[%s]\t",movie[r].data[c]);
		}
		//debugging
		//printf("\n");
		fprintf(fileOutput,"\n");
	}
}
//takes a string, tokenizes it for commas and / characters and returns an array.
char ** splitParam(char* str){

	char* string = strdup(str);
	int indexOfComma = indexOf(string, ",");
	int indexOfSlash = indexOf(string, "/");

	char** array = (char**) malloc (sizeof(char*) * 255);
	int x = 0;
	for (x = 0; x < 255; x++)
	{
		array[x] = malloc (sizeof(char)*(200+1));
	}

	char* seperator = "NULL";
	if (indexOfComma > 0)
	{
		seperator = ",";
	}

	if (indexOfSlash > 0)
	{
		seperator = "/";
	}
	//printf("[, %d | / %d]\n", indexOfComma, indexOfSlash);
	char* token;
	int arrayIndex = 0;
	while ((token = strsep (&string, seperator))!= NULL){
		//printf("TOKEN:[%s]..\n", token);
		array[arrayIndex] = token;
		arrayIndex++;
	}

	return array;
}
//custom indexOf Method to search if a character exists in the given string
int indexOf(char* word, char* character){
	char* temp = word;
	while (*temp)
	{
		if (strchr(character, *temp))
			return 1;
		temp++;
	}

	return 0;
}

//============================== QUEUE =============================================
int getNumOfStructRows (struct movieMeta* movies) {
	int x = 0;
	while(movies[x].data != NULL)
		x++;
	return x;
}

void largeEnqueue(struct movieMeta* movie){
	
	//Create a qNode to hold the array
	struct qNode* node = (struct qNode*) malloc (sizeof(struct qNode));
	
	node->data = movie;

	
	if ((qRear == NULL) && (qFront == NULL)){
		qRear = qFront = node;
		return;
	}

	node->next = NULL;
	qRear->next = node;
	qRear = node;
}

struct movieMeta* largeDequeue2(){
	struct qNode* node = qFront;
	if (qFront == NULL){
		printf("Queue Is Empty...\n");
		return;
	}
	if (qFront == qRear){
		qFront = qRear = NULL;
	}
	else {
		qFront = qFront->next;
	}
	
	return node->data;
}

void printStruct(struct movieMeta* arr, int totalRows, int a){
	
	int index=0;
	for (index = 0; index< totalRows; index++) {
		printf("   [%s]     %d\n", arr[index].data[a], index);
	}
}

//JOIN CSVS 
struct movieMeta* joinCSVs(){
	
	struct movieMeta* topHalf;
	struct movieMeta* lowHalf;
	int numRows = 0;
	
	while(gloVars.queueAmt >= 2)
	{
		//printf(">>> Loop starts\n");
		//printf("There are %d CSVs in the queue.... \n", gloVars.queueAmt);
		
		//CSV 1
		topHalf = largeDequeue2();
		gloVars.queueAmt -= 1;
		//int topHalfrow  = getNumOfStructRows(topHalf);
		//printf("A T1 = %d | T2 = %d ROWS\n", getNumOfStructRows(topHalf), 0);
		int topHalfrow = 0;
		while(topHalf[topHalfrow].data != NULL)
			topHalfrow++;


		
		//CSV 2
		lowHalf = largeDequeue2();
		gloVars.queueAmt -= 1;
		//int lowHalfrow = getNumOfStructRows(lowHalf);
		int lowHalfrow = 0;
		while(lowHalf[lowHalfrow].data != NULL){
			lowHalfrow++;
		}

		//printf("B T1 = %d | T2 = %d \n", getNumOfStructRows(topHalf), getNumOfStructRows(lowHalf));
		
		numRows = topHalfrow + lowHalfrow;
		//printf("  numRows: %d\n", numRows);
		
		
		
		
		//Increase size of topHalf to incorporate lowHalf
		topHalf = realloc(topHalf, (numRows)*sizeof(struct movieMeta) * sizeof(char*)*(gloVars.gloNumOfColumns));
		//printf("C T1 = %d | T2 = %d \n", getNumOfStructRows(topHalf), getNumOfStructRows(lowHalf));
		
		int r= 0; 
		for (r = topHalfrow; r < (numRows); r++) {
			topHalf[r].data = malloc(sizeof(char*) * (gloVars.gloNumOfColumns) ); //trying to figure out how to make room for string fields
		}
		

		//Store data
		int tIndex=0;
		int lIndex = 0; 
		for (tIndex = topHalfrow; tIndex < numRows; tIndex++) {
			topHalf[tIndex].data = lowHalf[lIndex].data;
			lIndex++;
		}
		 

		//Call Merge sort on combined pieces
		MergeSort(topHalf, numRows, gloVars.gloNumOfColumns, gloVars.gloIndexOfColumnToSort, gloVars.gloTypeOfSort);
		
		
		pthread_mutex_lock(&lockReNQ);	
			//Enqueue Joined struct
			largeEnqueue(topHalf);
			//printf("ENQUEUED.\n");
			gloVars.queueAmt+=1;
		pthread_mutex_unlock(&lockReNQ);
		
		
		//printf("F TEMP 1 = %d | TEMP 2 = %d \n", getNumOfStructRows(topHalf), getNumOfStructRows(lowHalf)); 
		//printf(">>> Loop Ends\n\n");
		free(lowHalf);
	}
	//Mergesort once again for complete confidence
	//printf("  numRows: %d\n", numRows);
	MergeSort(topHalf, numRows, gloVars.gloNumOfColumns, gloVars.gloIndexOfColumnToSort, gloVars.gloTypeOfSort);
	//printStruct(topHalf, numRows, gloVars.gloIndexOfColumnToSort);
	return topHalf;
} 

void completeOutput(struct movieMeta *fullcsv){
	printf("Output reached\n");
	//Get total number of rows for struct
	int totalRows = 0;
	while(fullcsv[totalRows].data != NULL){
		totalRows++;
	}
	printf("TotalRowsMade: %d\n", totalRows);
	printf("\n");

	char* outFilePrefix = "AllFiles-sorted-";
	char* fileName = malloc(255*sizeof(char*));
	memset(fileName, '\0', sizeof(fileName));
	strcpy(fileName, outFilePrefix);
	strcat(fileName, gloVars.gloColumnToSort);
	strcat(fileName, ".csv");
    printf("Filename made: [%s]\n", fileName);
	
	
	
	
	printf("Output Path: [%s]", gloVars.gloOutputPath);
	DIR* outDIR = opendir(gloVars.gloOutputPath);
	char* pathFileName[PATH_MAX];
	strcpy(pathFileName, gloVars.gloOutputPath);
	strcat(pathFileName, "/");
	strcat(pathFileName, fileName);
	
    printf("Path made: [%s]\n", pathFileName);
	FILE *outFile = fopen(pathFileName, "w");
	printf("FILE opened %d %d\n", totalRows, gloVars.gloNumOfColumns);
	csvPrint(fullcsv, totalRows, gloVars.gloNumOfColumns, outFile);
	printf("Print Complete!\n");
	//free(pathFileName);

}


/* ============== OLD MAIN ============================= OLD MAIN ============================ OLD MAIN ============ */

void oldmain (void*package){
	
	pthread_mutex_lock(&lockID2); 		
		//insertTID(pthread_self());
		tidArray[gloVars.gloTotalThreads] = pthread_self();
		gloVars.gloTotalThreads++;
	pthread_mutex_unlock(&lockID2);
	
	//Unpack for variables
	struct test *info = (struct test*) package;
	char* fileName = info->fileName;
	char* path = info->directoryPath;
	//printf("[T %lu]\n  Started sorting [%s]\n", pthread_self(),fileName);

	//Set global variables
	char* columnToSort = gloVars.gloColumnToSort;
	char outputPath[PATH_MAX];
	strcpy(outputPath, gloVars.gloOutputPath);

	//CSV input prep
	char fileStr[PATH_MAX];
	strcpy(fileStr, path);
	if (fileStr[(strlen(fileStr))-1] != '/'){
		strcat(fileStr, "/");
	}
	strcat(fileStr, fileName);
	// printf("fileStr: %s\n", fileStr);
	FILE* fileInput = fopen(fileStr, "r"); //read the file
	if(fileInput == NULL){
		printf("NULL PTR\n");
	}



	

	//Part 1: Use Header row to set up values
	//Grabs current row (first row at this point) and creates a temprow for safety
	char currentRow[1000];
	fgets(currentRow, 1000 , fileInput); //gets current row with all the commas
	char* tempRow = strdup(currentRow);
	int numOfHeadColumns = getNumOfColumns(tempRow); //Allows dynamic columns
	free(tempRow);

	tempRow = strdup(currentRow);
	tempRow[strlen(tempRow)-1]='\0';
	//populating headerNames to eventually find columnToSort
	char *headerNames[numOfHeadColumns];

	char* currentHeaderValue = strtok(tempRow,",");
	int i = 0;
	for (i = 0; i < numOfHeadColumns; i++)
	{
		headerNames[i] = malloc(strlen(currentHeaderValue)+1); // allocates mem for String array
		
		strcpy(headerNames[i],currentHeaderValue); //copies previous value into current position

		currentHeaderValue= strtok(NULL, ",\n"); // goes to next value
	}

	//TODO: CREATE A STRUCT HERE TO STORE HEADER NAMES.
	//FIXME:  USE MUTEX LOCK HERE.


	//gets the index of the column we have to sort
	
	if (gloVars.gloIndexOfColumnToSort == -1){
		pthread_mutex_lock(&lockColIndex);
		gloVars.gloIndexOfColumnToSort = getSortingColumnIndex(headerNames, numOfHeadColumns, columnToSort);
		pthread_mutex_unlock(&lockColIndex);
	}
	if (gloVars.gloNumOfColumns == -1){
		pthread_mutex_lock(&lockNumCols);
		gloVars.gloNumOfColumns = numOfHeadColumns;
		pthread_mutex_unlock(&lockNumCols);
	}

	int indexOfColumnToSort = gloVars.gloIndexOfColumnToSort;
	if (indexOfColumnToSort < 0){
		printf("ERROR: header title \"%s\" at columnIndex %d does not exist for %s at %s.\n", columnToSort, numOfHeadColumns, fileName, path);
		exit(0);
	}
	free (tempRow);



	//============ SETUP COMPLETE ==============
	/* Part 2: Create an Array of Strings to store stdinputs */
	// initialize a standard array of 10 strings
	int stdnumOfRows = 10;
	char** dataRows = (char**) malloc(sizeof(char*)* stdnumOfRows);
	int x = 0;
	for (x = 0; x < stdnumOfRows; x++){
		dataRows[x] = malloc(sizeof(char)*(500+1));
	}

	//Using getline because it gets entirety of row
	char* stdline = NULL;
	size_t len = 0;
	ssize_t read;
	int stdrowNumber = 0; //Counter to keep track of how many rows were made ; dynamic rows
	while ((read = getline(&stdline, &len,fileInput)) != -1)
	{
		tempRow = strdup(stdline); //duplicate row for safety
		//printf("[%s]\n", tempRow);

		if(stdrowNumber == stdnumOfRows) // if the table is full...
		{
			dataRows = (char**) realloc(dataRows, 2* stdnumOfRows*sizeof(char*)); //realloc overall array with 2x the current size
			stdnumOfRows = stdnumOfRows*2; // update the size

			// and initialize the new rows with the same parameters as before
			int x = 0;
			for (x = stdrowNumber; x < stdnumOfRows; x++ )
				dataRows[x] = malloc(sizeof(char)*(500+1));
		}
		//copy data into array and increase index
		strcpy(dataRows[stdrowNumber],tempRow);
		stdrowNumber++;
	}
	int a = 0;
	for(a = 0; a < stdrowNumber; a++)
	{
		//printf("%s\n", dataRows[a]);
	}


	/* Part 3: Store data from ArrayOfStrings into ArrayOfStructs*/

	//Declare size
	int numOfRows = stdrowNumber; //dynamic, but static values work too
	int numOfColumns = numOfHeadColumns; // dynamic, but static values work too
	struct movieMeta* movie = malloc( numOfRows* sizeof(struct movieMeta) + sizeof(char*)*numOfColumns);
	int r=0;
	for (r = 0; r < numOfRows; r++){
		movie[r].data = malloc(sizeof(char*)*numOfColumns);
	}

	//==== Tokenizing with while loop


	r = 0;
	int currentLineIndex = 0;
	for (currentLineIndex = 0; currentLineIndex < numOfRows; currentLineIndex++) // For each line in dataRows
	{
		tempRow = strdup(dataRows[currentLineIndex]); // duplicate currentLine for safety
		tempRow[strlen(tempRow)-1]='\0';
		char* token;
		int c=0;
		while( (token = strsep(&tempRow,",")) != NULL ) //for each tokenized/seperation with comma....
		{
			movie[r].data[c] = malloc(strlen(token)+1); // malloc space for the incoming string
			//Check if the first character is a ", true? then fix it.

			if (token[0] == ('\"'))
			{
				token = strtok(token, "\""); //remove first quote

				char fix[500] = "";
				strcat(fix, token);
				//continuously append until the token contains a ".
				while(indexOf(token, "\"") != 1)
				{
					//Append the next string
					fix[strlen(fix)] = ','; // add comma to the end of token
					token = strsep(&tempRow, ","); // get next token
					strcat(fix, token); // append
				}

				token = strdup(fix); // Replace current with fixed
				token[strlen(token)-1] = '\0'; // remove remaining quotation mark
				token = trimWord(token); // trim whitespace
			}

			token = trimWord(token);
			movie[r].data[c] = token;
			c++;
		}
		r++;
		free(tempRow);
	}

	//Post-Storage output
	//confirmationPrint(movie, currentLineIndex, numOfColumns);


	//determine the type of Sort to perform, string, long, double
	if (gloVars.gloTypeOfSort == -1){
		pthread_mutex_lock(&lockTypeSort);
		gloVars.gloTypeOfSort = determineTypeOfSort(movie, currentLineIndex,numOfColumns, indexOfColumnToSort);
		pthread_mutex_unlock(&lockTypeSort);
	}


	int typeOfSort = gloVars.gloTypeOfSort;




	//merge sort!
	pthread_mutex_lock(&lock3);
    	MergeSort(movie,currentLineIndex,numOfColumns, indexOfColumnToSort, typeOfSort);
	

	    //Use Queue's O(1) time to join the csvs together
	
		largeEnqueue(movie);
		gloVars.queueAmt++;
	pthread_mutex_unlock(&lock3);
	
	fclose(fileInput);
	
	
	pthread_exit(NULL);
}



void printTID(){
	int i = 0; 
	while(i < gloVars.gloTotalThreads){
		printf("%lu, ", tidArray[i]);
		i+=2;
	}
}
//==========  NEW CODE and METHODS==========
// traverses directories and forks when appropriate
void traverseAndThread (void * package){
    pthread_t threadHandles[400];
	int threadIndex = -1;
	
	pthread_mutex_lock(&lockID1); 		
		//insertTID(pthread_self());
		if (tidLock == 0){
			tidArray[gloVars.gloTotalThreads] = pthread_self();
			gloVars.gloTotalThreads++;
		}
	pthread_mutex_unlock(&lockID1);
	
	char* dir_name = (char*) package;
    //printf("TESTING: %s\n", dir_name);
	//printf("tIndex: %d\n", threadIndex);
    DIR * dirStream;

    /* Open the directory specified by "dir_name". */
    dirStream = opendir (dir_name);

    /* Check it was opened. */
    if (!dirStream) {
        fprintf (stderr, "Cannot open directory '%s': %s\n",
		dir_name, strerror (errno));
        exit (EXIT_FAILURE);
    }

    while (1) {
        struct dirent * dirPTR;
        const char * d_name;

        /* "Readdir" gets subsequent entries from "d". */
        dirPTR = readdir (dirStream);
        if (!dirPTR) {
            break;    /* There are no more entries in this directory, so break out of the while loop. */
        }
        
        
        d_name = dirPTR->d_name;
        /* Print the name of the file and directory. */
		
		//PRINT EVERY dirPTR: 
		//printf ("%s/%s\n", dir_name, d_name);

		if (dirPTR->d_type & DT_DIR) {       //THIS IS A DIRECTORY
		/* Check that the directory is not "d" or d's parent. */
			if (strcmp (d_name, "..") == 0 || strcmp (d_name, ".") == 0)
				continue;
			
			//PRINT DIRECTORY IM POINTING AT NOW:
			//printf ("[T %lu] %s/%s\n", pthread_self(), dir_name, d_name);

			//LOCK 1
			
				char newInputPath [PATH_MAX];
				strcpy(newInputPath, dir_name);
				strcat(newInputPath,"/");
				strcat(newInputPath,dirPTR->d_name);
				
				
				//CHANGE VALUE OF PATH
				int inpath_length = strlen(newInputPath);

				//Basecase: kill if path is too long to handle.
				if (inpath_length >= PATH_MAX) {
					fprintf (stderr, "Input Path length has got too long.\n");
					exit (EXIT_FAILURE);
				}
			
				/* Recursively call "list_dir" with the new path. */
				threadIndex++;
				pthread_mutex_lock(&lock1);
					gloVars.gloTotalThreads += 1;
					tidLock = 0;
				pthread_mutex_unlock(&lock1);
				int err = pthread_create(&threadHandles[threadIndex], NULL, (void*)&traverseAndThread, (void*)newInputPath);
				if (err)
					printf("ThreadCreationFailed: %d\n", err);
				
				
				pthread_join(threadHandles[threadIndex], NULL);
			//UNLOCK 1
			    //pthread_exit(NULL);
			
	  
		}
		else //THIS IS A FILE
		{ 
			if ( (!(strstr(d_name, "-sorted-"))) && (strstr((&d_name[strlen(d_name) -4]), ".csv") )){
				
				if (strcmp (d_name, "..") == 0 || strcmp (d_name, ".") == 0)
					continue;
				
		
			//PRINT FILE IM POINTING AT RIGHT NOW: 
			//printf ("[T %lu] Pointing at: %s/%s\n", pthread_self(), dir_name, d_name);
			
			//LOCK 2
			
				threadIndex++;
				struct test info; 
				info.directoryPath = dir_name;
				info.fileName = d_name;
			pthread_mutex_lock(&lock2);		
				gloVars.gloTotalThreads += 1;
			pthread_mutex_unlock(&lock2);
				int err = pthread_create(&threadHandles[threadIndex], NULL, (void*)&oldmain, (void*)&info);
				if (err){
					printf("ThreadCreationFailed: %d\n", err);
				}
			//UNLOCK 2	
				pthread_join(threadHandles[threadIndex], NULL);
			
				
			}
		}
		
    }

    /* After going through all the entries, close the directory. */
    if (closedir (dirStream)) {
        fprintf (stderr, "Could not close '%s': %s\n",
		dir_name, strerror (errno));
        exit (EXIT_FAILURE);
    }
    
    
        //Joins Threads
  
    int i = 0;
    while(i <= threadIndex){
        pthread_join(threadHandles[i], NULL);
        i++;
    }
    
}

//NEW MAIN METHOD
int main(int argc, char *argv[]){
	
	//Part 1: Parameter Management =================Part 1: Parameter Management
	int paramIndex = 0, flagc = 0, flagd=0, flago=0;
	//create 3 strings to store list of items
	char* columnsToSortStr = "NULL"; //one column or list of columns
	char* inputDirStr = "NULL";      //list of directories
	char* outputDirStr = "NULL";     //list of directories

	//Loops through input and stores value of column(s) and directories in a string format.
	while (argv[paramIndex]!= NULL){
		if (strcmp(argv[paramIndex], "-c") == 0){
			columnsToSortStr = argv[paramIndex+1];
			flagc = 1;
		}

		if (strcmp(argv[paramIndex], "-d") == 0){
			inputDirStr = argv[paramIndex+1];
			flagd=1;
		}

		if (strcmp(argv[paramIndex], "-o") == 0){
			outputDirStr = argv[paramIndex+1];
			flago = 1;
		}
		paramIndex++;
	}

    //printf("-c: %s\n", columnsToSortStr);
	//printf("-d: %s\n", inputDirStr);
	//printf("-o: %s\n", outputDirStr);
	

	//Column name(s) are a MUST. so kill program if it doesn't exist.
	if (flagc ==0){
		printf("ERROR: -c is missing. \n");
		exit(0);
	}
	
	
	
	
	
	//Break up list into arrays
	char** columnsToSortArr;
	if (strcmp(columnsToSortStr, "") != 0)
		columnsToSortArr = splitParam(columnsToSortStr);
	

	//Part 2: Directory Management ========================= Part 2: Directory Management
	gloVars.gloColumnToSort = columnsToSortArr[0];//SET THIS TO GLOBAL 
	
	char* path;
	char* outputPath;
	char currentWorkingDir[5000];
	
	
	//INPUT DIRECTORY
	if (getcwd(currentWorkingDir, sizeof(currentWorkingDir)) == NULL)
		printf("Current Working Directory error for input");
	
	
	//current path and output path are set to currentWorkingDirectory by default
	path = currentWorkingDir;
	//printf("CurrentWorkingDirectory: %s\n", path);
	outputPath = currentWorkingDir;

	//testing if custom inputDirectory is valid
	DIR * dirStream;
    if (flagd == 1){
		dirStream = opendir(inputDirStr);
		path = inputDirStr;
	}
	else{
		dirStream = opendir(path);
	}

	if (!dirStream){
		fprintf (stderr, "ERROR: Cannot Open INPUT DIR - %s. [%s]\n",strerror (errno), path);
		exit (EXIT_FAILURE);
	}
   	closedir(dirStream);
	
  
	DIR * outStream;
	char* outPath = outputPath;
	if (flago == 1){
		outStream = opendir(outputDirStr);
		outPath = outputDirStr;
	}
	else{
		outStream = opendir(outputPath);
	}

	if (!outStream){
		fprintf (stderr, "ERROR: Cannot Open OUTPUT DIR - %s. [%s]\n",strerror (errno), path);
		exit (EXIT_FAILURE);
	}
	closedir(outStream);

	gloVars.gloOutputPath = outPath;
    //printf("Current Directory: %s\n", path);
    //printf("Output Directory:  %s\n", gloVars.gloOutputPath);
    //printf("ColumnToSort:      %s\n", gloVars.gloColumnToSort);

	//Part 3: Call the method
    //Activates Locks
    //printf("Creating Locks now.");
    
    printf("\nINITIAL TID: \n");
	if(pthread_mutex_init(&lock1, NULL) != 0){
		printf("Error initializing lock1.\n");
	}
	if(pthread_mutex_init(&lock2, NULL) != 0){
		printf("Error intiializing lock2.\n");
	}
	if(pthread_mutex_init(&lock3, NULL) != 0){
		printf("Error intiializing lock3.\n");
	}
	if(pthread_mutex_init(&lockColIndex, NULL) != 0){
		printf("Error intiializing lockColIndex.\n");
	}
	if(pthread_mutex_init(&lockNumCols, NULL) != 0){
		printf("Error intiializing lockNumCols.\n");
	}
	if(pthread_mutex_init(&lockTypeSort, NULL) != 0){
		printf("Error intiializing lockTypeSort.\n");
	}
	if(pthread_mutex_init(&lockReNQ, NULL) != 0){
		printf("Error intiializing lockReNQ.\n");
	}
	if(pthread_mutex_init(&lockID2, NULL) != 0){
		printf("Error intiializing lockID.\n");
	}
	if(pthread_mutex_init(&lockID2, NULL) != 0){
		printf("Error intiializing lockID.\n");
	}


	//**Call TRAVERSAL Method**//
	
	//printf("Starting program now...\n");
	traverseAndThread ((void*)path);
	
	//printf("CSVs stored in queue. \nJoining CSVs...");
	
	//JoinCSVs together for one big struct
	struct movieMeta* largeStruct = joinCSVs();
	//printf("CSVs joined.\n");
	//printf("Outputting now...\n");
	completeOutput(largeStruct);
	//printf("Output complete!\nDestroying Locks\n");
	
	//Destroy Locks
    pthread_mutex_destroy(&lock1);
    pthread_mutex_destroy(&lock2);
	pthread_mutex_destroy(&lock3);
	pthread_mutex_destroy(&lockColIndex);
	pthread_mutex_destroy(&lockNumCols);
	pthread_mutex_destroy(&lockTypeSort);
	pthread_mutex_destroy(&lockReNQ);
	pthread_mutex_destroy(&lockID1);	
	pthread_mutex_destroy(&lockID2);	
    free(largeStruct);
    
    
    
    
	
	
	printTID(); 
	printf("\nTotal number of threads: %d\n", gloVars.gloTotalThreads);
    


	return 0;
}


