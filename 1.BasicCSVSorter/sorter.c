#include "sorter.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

//Use this to compile
/* gcc -Wall -Werror -c -o sorter.o sorter.c
 * gcc -Wall -Werror -c -o Mergesort.o Mergesort.c
 * gcc -Wall -Werror -o sorter sorter.o Mergesort.o*/

//Use this to run
/* cat ?????.csv | ./sorter -c ????? */
  
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

/* =========================================== MAIN ======================================== */
int main (int argc, char *argv[]){
	// gets the string name of the column we need to sort
	char* columnToSort = argv[2];


	//Grabs current row (first row at this point) and creates a temprow for safety
	char currentRow[1000];
	fgets(currentRow, 1000 , stdin); //gets current row with all the commas
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
		printf("ERROR: header title \"%s\" at columnIndex %d does not exist. Please try again.\n", columnToSort, numOfHeadColumns);
		return 0;
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
	while ((read = getline(&stdline, &len,stdin)) != -1)
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
	
	
	//Post-MergeSort output
	confirmationPrint(movie, currentLineIndex, numOfColumns);

	return 0; //End of program
}


