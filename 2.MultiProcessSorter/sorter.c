#include "sorter.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <dirent.h>
#include <sys/types.h>
#include <unistd.h>     
#include <errno.h>
#include <sys/wait.h>
#include "mergesort.c"

//Use this to compile
/* gcc sorterSolo.c -o sorterSolo*/

//Use this to run

//./sorterSolo -c col2 -d /ilab/users/nap166/Desktop/testing2/aFolder -o /ilab/users/nap166/Documents/dani/aFolder
  
  
  int pidArray[255];
  int pidIndex = 0;
  int pid;
  int initialpid;
  
  
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

//joins two arrays together


//splits one array into left and right sub arrays


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
			
			if (indexOf(movie[r].data[c], ",") == 1)
				//printf("\"%s\",", movie[r].data[c]); // put quotations around field and add comma
				fprintf(fileOutput, "\"%s\",", movie[r].data[c]); // put quotations around field and add comma
				
			else
				//printf("%s,",movie[r].data[c]); // add a comma
				  fprintf(fileOutput, "%s,", movie[r].data[c]);
			
			
			//debugging
			//printf("[%s]\t",movie[r].data[c]);
		}
		fprintf(fileOutput,"\n");
	}
}

/* ============== OLD MAIN ============================= OLD MAIN ============================ OLD MAIN ============ */
void oldmain (char* fileName, char* path, char* columnToSort, char* outputPath){
    //Opens file
    
    char fileStr[PATH_MAX];
    strcpy(fileStr, path);
    if (fileStr[(strlen(fileStr))-1] != '/'){
		strcat(fileStr, "/");
	}
	strcat(fileStr, fileName);
	
	FILE* fileInput = fopen(fileStr, "r"); //read the file
	
	if(fileInput == NULL){
		printf("NULL PTR\n");
	}

	//Grabs current row (first row at this point) and creates a temprow for safety
	char currentRow[1000];
	fgets(currentRow, 1000 , fileInput); //gets current row with all the commas
	char* tempRow = strdup(currentRow);
	int numOfHeadColumns = getNumOfColumns(tempRow); //Allows dynamic columns
	free(tempRow);

	tempRow = strdup(currentRow);
	tempRow[strlen(tempRow)-1]='\0';
	//populating headerNames to eventually find columnToSort
	//TODO: currently set to 28, find a way to dynamically find how many columns there are
	char *headerNames[numOfHeadColumns];

	char* currentHeaderValue = strtok(tempRow,",");
	int i = 0;
	for (i = 0; i < numOfHeadColumns; i++)
	{
		headerNames[i] = malloc(strlen(currentHeaderValue)+1); // allocates mem for String array
		strcpy(headerNames[i],currentHeaderValue); //copies previous value into current position

		currentHeaderValue= strtok(NULL, ",\n"); // goes to next value
	}
	
	//gets the index of the column we have to sort
	int indexOfColumnToSort = getSortingColumnIndex(headerNames, numOfHeadColumns, columnToSort);
	if (indexOfColumnToSort < 0)
	{
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
	int typeOfSort = determineTypeOfSort(movie, currentLineIndex,numOfColumns, indexOfColumnToSort);
	
	//merge sort!
	MergeSort(movie,currentLineIndex,numOfColumns, indexOfColumnToSort, typeOfSort);
	fclose(fileInput);
	
	
	int pathLen = strlen(outputPath);
	int fileNameLen = strlen(fileName);
	int trim = fileNameLen - 4; //no .csv
	char* newFileName = malloc(sizeof(char*) *trim);
	memset(newFileName, '\0', sizeof(newFileName));
	
	strncpy (newFileName, fileName, trim);
	strcat(newFileName, "-sorted-");
	strcat(newFileName, columnToSort);
	strcat(newFileName, ".csv");
	
	char* newOutPath = malloc(sizeof(char*) *(pathLen + trim+1 ));
	strncpy(newOutPath, outputPath, pathLen);
	strcat (newOutPath, "/");
	strcat (newOutPath, newFileName);
	
	FILE* fileOutput = fopen(newOutPath, "w");
	//printf("Current input path: [%s]\n", path);
	csvPrint(movie, currentLineIndex, numOfColumns, fileOutput);
	
	
	
	free (movie);
	
	
	

	exit(0); //End of program
}



//==========  NEW CODE and METHODS==========

//New Method specifically for project 2
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

// traverses directories and forks when appropriate
void traverseAndFork  (char * dirPath, char* columnToSort, char* outputPath){
	
	//Open the directory we set 
    DIR * dirStream;
    dirStream = opendir (dirPath);
	char newInputPath [PATH_MAX];
	char newOutputPath[PATH_MAX]; 
	
	
    //Kill Program if directory doesn't open.
    if (! dirStream) {
        fprintf (stderr, "ERROR: Cannot open directory '%s': %s\n", dirPath, strerror (errno));
        exit (EXIT_FAILURE);
    }
    
    struct dirent * dirPTR;
    
    //Main traverseLoop
    while ((dirPTR = readdir(dirStream))!= 0) {
         // a pointer to each entity in the current directory opened by dirStream
        char * d_name;
        if (! dirPTR) {
            break;
        }
        
        d_name = dirPTR->d_name; // TEST THIS ONE FOR .CSV	
		
        if (dirPTR-> d_type == DT_DIR)  // THIS IS A DIRECTORY
        {
			if (strcmp (d_name, "..") == 0 || strcmp (d_name, ".") == 0){
				continue;
			}
			pid = fork();
			pidArray[pidIndex] = pid;
			pidIndex++;

			
			if(pid == 0){
				strcpy(newInputPath, dirPath);
				strcat(newInputPath,"/");
				strcat(newInputPath,dirPTR->d_name);
				//CHANGE VALUE OF PATH 
				int inpath_length = strlen(newInputPath);
				
				//Basecase: kill if path is too long to handle.
				if (inpath_length >= PATH_MAX) {
					fprintf (stderr, "Input Path length has got too long.\n");
					exit (EXIT_FAILURE);
				}
				 dirPath = newInputPath;
				
				dirStream = opendir(dirPath);
				continue;
			}
			else
			{

				//printf("%d Parent Folder : [%s]\n", pid, dirPTR->d_name);
				continue;
			}
			
		}      
		else // THIS IS A FILE
		{ 
			if ( (!(strstr(d_name, "-sorted-"))) && (strstr(d_name, ".csv") )){
				if (strcmp (d_name, "..") == 0 || strcmp (d_name, ".") == 0){
					continue;
				}
				pid = fork();
	
				pidArray[pidIndex] = pid;
				pidIndex++;
				
				if (pid != 0)
				{
					//printf("%d Parent file : [%s]\n", pid, dirPTR->d_name);
				}
				else
				{   pidArray[pidIndex] = pid;
					pidIndex++;
					oldmain(dirPTR->d_name, dirPath, columnToSort, outputPath);

				} 
			}
		}
		 //else end 
    }
    
    
    
    
    /* After going through all the entries, close the directory. */
    if (closedir (dirStream)) {
        fprintf (stderr, "Could not close '%s': %s\n",
                 dirPath, strerror (errno));
        exit (EXIT_FAILURE);
    }

	
    //WAIT FOR PROCESSES TO COMPLETE
		int x = 0;
		while (x != pidIndex){ //-1 for impossible process = waits for everything
		    wait(NULL);
			waitpid(pidArray[x],NULL, 0);
			x++;
		}
}


void printPIDs()
{
	printf("INITIAL PID: %d\n", pidArray[0]);
	printf("PIDS of all child processes: ");
	int x = 0; 
	for(x = 0; x < pidIndex; x++)
	{
		printf("%d,", pidArray[x]);
	}                             
	printf("\nTotal number of processes: %d\n", pidIndex);
}
 //NEW MAIN METHOD 
int main(int argc, char *argv[]){   
	//Part 1: Parameter Management =================Part 1: Parameter Management 
	int paramIndex = 0, flagc = 0, flagd=0, flago=0; 
	//create 3 strings to store list of items
	char* columnsToSortStr = "NULL"; //one column or list of columns
	char* inputDirStr = "NULL"; //list of directories
	char* outputDirStr = "NULL"; //list of directories
	
	//Loops through input and stores value of column(s) and directories in a string format.
	while (argv[paramIndex]!= NULL){
		if (strcmp(argv[paramIndex], "-c") == 0){
			columnsToSortStr = argv[paramIndex+1];
			//printf("Line68: %s\n", columnsToSortStr);
			flagc = 1;
		}

		if (strcmp(argv[paramIndex], "-d") == 0){
			inputDirStr = argv[paramIndex+1];
			flagd=1;
			//printf("Line73: %s\n", inputDirStr);
		}
		
		if (strcmp(argv[paramIndex], "-o") == 0){
			outputDirStr = argv[paramIndex+1];
			flago = 1;
			//printf("Line78: %s\n", outputDirStr);
		}	
		paramIndex++;
	}
	
	//Column name(s) are a MUST. so kill program if it doesn't exist. 
	if (flagc ==0){
		printf("ERROR: -c is missing. \n");
		exit(0);
	}
	

	//Break up list into arrays
	char** columnsToSortArr;
	if (strcmp(columnsToSortStr, "") != 0)
		columnsToSortArr = splitParam(columnsToSortStr);


	char** inputDirArr;
	if (strcmp(inputDirStr, "") != 0 )
		inputDirArr = splitParam(inputDirStr);

	
	char** outputDirArr;
	if (strcmp(outputDirStr, "") != 0 )
		outputDirArr = splitParam(outputDirStr);

	
	
	
	//Part 2: Directory Management ========================= Part 2: Directory Management
	char* columnToSort = columnsToSortArr[0]; 
	
	
	
	char* path;
    char* outputPath;
    char currentWorkingDir[5000];
	if (getcwd(currentWorkingDir, sizeof(currentWorkingDir)) != NULL){
	   
	}
	else
	   printf("getcwd() error for input");
	   
	path = currentWorkingDir;
	outputPath = currentWorkingDir;
	
	
    //printf("CURRENT WORKING DIRECTORY: %s\n", currentWorkingDir);
	DIR * dirStream;
	 
	if (flagd == 1){
		dirStream = opendir(inputDirStr);
		path = inputDirStr;
	}
	else
	{
		dirStream = opendir(path);
	}
	
	if (!dirStream){
		fprintf (stderr, "Cannot open INPUT directory '%s': %s\n",path, strerror (errno));
        exit (EXIT_FAILURE);
	}
	
	closedir(dirStream);
	
	
	DIR * outStream;
	char* outpath; 
	if (flago == 1){
		outStream = opendir(outputDirStr);
		outpath = outputDirStr;
	}
	else
	{
		outStream = opendir(outputPath);
	}
	
	if (!outStream){
		fprintf (stderr, "Cannot open OUTPUT directory '%s': %s\n",outputPath, strerror (errno));
        exit (EXIT_FAILURE);
	}
	
	closedir(outStream);
	//Part 3: Call the traversing method

	traverseAndFork(path, columnToSort, outputPath);
	
	
	
	if(pid > 0){
		
		int x = 0;
		while (x != pidIndex){ //-1 for impossible process = waits for everything
		    wait(NULL);
			waitpid(pidArray[x],NULL, 0);
			x++;
		}
	}
	printPIDs();
	exit(0); 
}
	


