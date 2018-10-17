struct qNode{
	struct movieMeta* data;
	struct qNode* next;
};
struct qNode* qFront = NULL;
struct qNode* qRear  = NULL;

typedef struct{
	//char* gloColumnToSort;
	//char* gloOutputPath;
	int gloNumOfColumns;
	int gloIndexOfColumnToSort;
	int gloTypeOfSort;
	int queueAmt;
	//int gloTotalThreads;
}GLOBAL;
GLOBAL gloVars={
	.gloNumOfColumns = -1,
	.gloIndexOfColumnToSort = -1,
	.gloTypeOfSort = -1,
	.queueAmt = 0, //Number of nodes inside the queueue	
	//.gloTotalThreads = 0,
};