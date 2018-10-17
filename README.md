# SP-CSVSorter

As part of a semester long project, we were required to sort a large CSV file of the imdb movie database in multiple ways. 
## 1. Basic CSV Sort
- Simple sorting using Mergesort, on a column the user chooses.
- Determines what type of sorting route it will take based on the data in the seleced column: Strings, Long, Double

## 2. MultiProcess Sort
- Same as #1 but now handles multiple files using Multiprocess methods.
- Each process = 1 CSV file to sort. 
- Automatically fails gracefully if a bad path is inputted.
 
## 3. MultiThreaded Sort
- Same as #1 but now handles multiple files using Multithreading methods.
- Each thread = 1 CSV file to sort, or 1 folder to traverse
- Automatically fails gracefully if a bad path is inputted.

## 4. ServerClient Sort
- Same as #3 but across a server and client scenario using network sockets.
- Each client sends all CSVs to the Server => Server adds all of them into a Queue structure and joins them together. 
- Server sends back 1 large CSV file back to the client, independant of other client's data. 
- Only 1 client's data is being manipulated by the server at a time, through use of Mutex Locks
