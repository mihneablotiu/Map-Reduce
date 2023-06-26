#ifndef __FUNCTIONS_H_
#define __FUNCTIONS_H_

#include "structures.h"

/* The function that interrogates each of the files for the mappers and puts the partial
results in their corresponding list from the corresponding row */
void interrogateFile(FILE *currentFile, int numberOfReducers, individualStruct *currentStruct);

/* The function that removes duplicates from a given list */
unsigned long int removeDuplicates(int *currentList, int size);

/* The function that iterates through the partial results of the mappers and gathers
them all together in a final list */
void* reduceLists(int reducerId, int numberOfMappers, individualStruct *currentStruct);

/* The function that does the binary search for each of the numbers and powers*/
bool binarySearch (int searchedNumber, int left, int right, int power);

/* The function that each of the mappers execute */
void *mappersFunction(void *arguments);

/* The function that each of the reducers executes */
void *reducersFunction(void *arguments);

/* The function that takes the structOfMappers which is a shared piece of memory
and adds the individual id of each of the threads and packs them in an inidvidual
struct for each of the threads */
void *allocateIndividualStruct(mappersStruct *structOfMappers, int currentId);

/* The function that allocates the general struct that each of the mappers and reducers is
going to use durring their specific function */
void *allocateMappersStruct(pthread_barrier_t *barrier, pthread_mutex_t *mutex, int numberOfFiles,
                            int numberOfReducers, int numberOfMappers, statusOfFiles *fileStatus);

/* The function the allocates dinamically the space for the mappers and reducers */ 
void allocateThreads(pthread_t **mappers, pthread_t **reducers, int numberOfMappers, int numberOfReducers);

/* The function that frees the memory for the fileStatus structure */
void fileStatusMemoryFree(statusOfFiles **fileStatus, int numberOfFiles);

/* The function that frees the individual struct for each of the threads
before the shared memory structOfMappers gets freed */
void freeIndividualStruct(individualStruct **currentStruct);

/* The function that parses the argv parameter from main function and puts
them in the corresponding variables */
char *getInput(int *numberOfMappers, int *numberOfReducers, char **argv);

/* The function that reads the input file and creates the statusOfFiles struct
in order to memorize for each of the input files if it was already taken by a
mapper or not */
void *fileRead(FILE *inFile, char *inputFile, int *numberOfFiles);

#endif /* __FUNCTIONS_H_ */