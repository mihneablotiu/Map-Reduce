#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <math.h>
#include <bits/stdc++.h>
#include "functions.h"

#define MAX_INT_VALUE 2147483647

/* The function that does the binary search for each of the numbers and powers*/
bool binarySearch (int searchedNumber, int left, int right, int power) {
    while(left <= right) {
        /* We take the mid value between the left and the right which at the
        beginning are 1 and sqrt(searchedNumber) + 1 */
        int mid = (left + right) / 2;

        int number = mid;
        long perfectPower = mid;

        /* We power this mid number to the power that we are searching for */
        for (int i = 0; i < power - 1; i++) {
            perfectPower = perfectPower * number;

            /* We take care in order not to get out of the int range (case in
            which we are 100% sure that mid is too big) */
            if (perfectPower > MAX_INT_VALUE) {
                break;
            }
        }

        /* After powering to the current power, if the searched number is equal to
        the result of powering, it means that the searched number is a perfect power
        of another number so we can add it to the corresponding list. Otherwise we
        adjust the searching interval accordingly. */
        if (perfectPower == searchedNumber) {
            return true;
        } else if (perfectPower < searchedNumber) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }

    return false;
}

/* The function that interrogates each of the files for the mappers and puts the partial
results in their corresponding list from the corresponding row */
void interrogateFile(FILE *currentFile, int numberOfReducers, individualStruct *currentStruct) {
    mappersStruct *structOfMappers = currentStruct->communStruct;

    // We check how many numbers we have in each file
    int numberOfNumbers;
    fscanf(currentFile, "%d", &numberOfNumbers);

    /* For each of those numbers we try to see if it is a perfect power of every number
    between 2 and the numberOfReducers + 1. If it is a perfect power, we add it to the
    corresponding list and we incerement the size of that list */
    for (int i = 0; i < numberOfNumbers; i++) {
        int currentNumber;
        fscanf(currentFile, "%d", &currentNumber);
        
        for (int power = 2; power <= numberOfReducers + 1; power++) {
            /* To see if a number is a perfect power we just do a binary search between
            1 and the square root of the current number + 1. This is enough because the
            minimum power that we try is 2. */
            if (binarySearch(currentNumber, 1, (int) sqrt(currentNumber) + 1, power) == true) {
                int currentPosition = structOfMappers->lists[currentStruct->id][power - 2].size;
                structOfMappers->lists[currentStruct->id][power - 2].listOfPowers[currentPosition] = currentNumber;
                structOfMappers->lists[currentStruct->id][power - 2].size++;
            }
        }
    }
}

/* The function that each of the mappers execute */
void *mappersFunction(void *arguments) {
    // We take our parameters from the arguments
    individualStruct *currentStruct = (individualStruct *) arguments;
    currentStruct->size = 0;
    mappersStruct *structOfMappers = currentStruct->communStruct;

    /* We iterate over the fileStatus structure and if we find a file 
    that has not been taken yet, we interrogate the file. Otherwise,
    we just check the next file in the list */
    for (int i = 0; i < structOfMappers->numberOfFiles; i++) {
        pthread_mutex_lock(structOfMappers->mutex);
        if (structOfMappers->fileStatus[i].finished == false) {
            structOfMappers->fileStatus[i].finished = true;
            pthread_mutex_unlock(structOfMappers->mutex);

            FILE *currentFile = fopen(structOfMappers->fileStatus[i].fileName, "rt");
            interrogateFile(currentFile, structOfMappers->numberOfReducers, currentStruct);
            fclose(currentFile);
        } else {
            pthread_mutex_unlock(structOfMappers->mutex);
        }
    }
    

    freeIndividualStruct(&currentStruct);

    /* We have to wait for all the mappers to finish with their
    files before we go for the reducers part */
    pthread_barrier_wait(structOfMappers->barrier);
    return NULL;
}

/* The function that iterates through the partial results of the mappers and gathers
them all together in a final list */
void* reduceLists(int reducerId, int numberOfMappers, individualStruct *currentStruct) {
    // We create the final list
    currentStruct->uniqueList = (int *) malloc(500000 * sizeof(int));
    currentStruct->size = 0;

    // We iterate through the corresponding lists of all the mappers
    for (int i = 0; i < numberOfMappers; i++) {
        int size = currentStruct->communStruct->lists[i][reducerId].size;

        // We add all the values to the final list
        for (int j = 0; j < size; j++) {
            int nextElement = currentStruct->communStruct->lists[i][reducerId].listOfPowers[j];
            currentStruct->uniqueList[currentStruct->size] = nextElement;
            currentStruct->size++;
        }
    }

    return currentStruct->uniqueList;
}

/* The function that removes duplicates from a given list */
unsigned long int removeDuplicates(int *currentList, int size) {
    std::unordered_set<int> uniqueSet;

    /* We just add all the elements to a set because we know
    that sets cannot have duplicates */
    for (int i = 0; i < size; i++) {
        uniqueSet.insert(currentList[i]);
    }

    // We return the size of the set
    return uniqueSet.size();
}

/* The function that each of the reducers executes */
void *reducersFunction(void *arguments) {
    // We take our parameters from the arguments
    individualStruct *currentStruct = (individualStruct *) arguments;
    mappersStruct *structOfMappers = currentStruct->communStruct;

    // We wait for all the mappers to finish before the reducers start
    pthread_barrier_wait(structOfMappers->barrier);

    /* We reduce the lists of all the mappers meaning that we iterate through the lists of the same power
    from all the mappers and we gather all the values in only one list. */
    int *uniqueList = (int *) reduceLists(currentStruct->id, currentStruct->numberOfMappers, currentStruct);

    /* We remove the duplicates from that list and return the final size of the list without the duplicates. */
    unsigned long int finalSize = removeDuplicates(uniqueList, currentStruct->size);

    /* We create the output file with the corresponding name */
    char *outFile = (char *) calloc(1000, sizeof(char));
    char number[100];
    strcat(outFile, "out");
    sprintf(number, "%d", currentStruct->id + 2);
    strncat(outFile, number, strlen(number));
    strcat(outFile, ".txt");

    /* We open the file, we write the final size in the file and we close it. */
    FILE *out = fopen(outFile, "wt");
    fprintf(out, "%d", (int) finalSize);
    fclose(out);

    /* We free all the used resources */
    free(outFile);
    free(uniqueList);
    freeIndividualStruct(&currentStruct);
    return NULL;
}

/* The function that takes the structOfMappers which is a shared piece of memory
and adds the individual id of each of the threads and packs them in an inidvidual
struct for each of the threads */
void *allocateIndividualStruct(mappersStruct *structOfMappers, int currentId) {
    individualStruct *currentStruct = (individualStruct *) malloc(sizeof(individualStruct));

    if (currentStruct == NULL) {
        fprintf(stderr, "%s\n", "Error at memory allocation");
        exit(1);
    }

    currentStruct->communStruct = structOfMappers;
    currentStruct->id = currentId;

    return currentStruct;
}

/* The function that allocates the general struct that each of the mappers and reducers is
going to use durring their specific function */
void *allocateMappersStruct(pthread_barrier_t *barrier, pthread_mutex_t *mutex, int numberOfFiles,
                            int numberOfReducers, int numberOfMappers, statusOfFiles *fileStatus) {

    // We allocate the struct of the mappers
    mappersStruct *structOfMappers = (mappersStruct *) malloc(sizeof(mappersStruct));
    if (structOfMappers == NULL) {
        fprintf(stderr, "%s\n", "Eroare at memory malloc for the mappers structure");
        exit(1);
    }

    /* Each of the mappers is going to use the same mutex, the same number of files,
    the same file status struct, the same barrier and the same number of reducers
    (in order to know until which power we have to check each number) */
    structOfMappers->mutex = mutex;
    structOfMappers->numberOfFiles = numberOfFiles;
    structOfMappers->fileStatus = fileStatus;
    structOfMappers->barrier = barrier;
    structOfMappers->numberOfReducers = numberOfReducers;

    /* Each of the mappers is going to have a row of lists so we need a matrix of
    numberOfMappers rows */
    structOfMappers->lists = (listOfLists **) malloc(numberOfMappers * sizeof(listOfLists *));
    if (structOfMappers->lists == NULL) {
        fprintf(stderr, "%s\n", "Error at memory malloc for the lists structure");
        exit(1);
    }

    /* Each of the rows is going to have exactly numberOfReducers lists because check powers starting
    from 2 until numberOfReducers + 1 (which is numberOfReducers + 1 - 2 + 1 = numberOfReducers powers)*/
    for (int i = 0; i < numberOfMappers; i++) {
        structOfMappers->lists[i] = (listOfLists *) malloc(numberOfReducers * sizeof(listOfLists));
        if (structOfMappers->lists[i] == NULL) {
            fprintf(stderr, "%s\n", "Error at memory malloc for the lists structure");
            exit(1);
        }

        /* Each of the lists from each position of each row is going to have maximum 500000 perfect
        powers as it is stated in the requirements of the homework */
        for (int j = 0; j < numberOfReducers; j++) {
            structOfMappers->lists[i][j].listOfPowers = (int *) malloc(500000 * sizeof(int));
            if (structOfMappers->lists[i][j].listOfPowers == NULL) {
                fprintf(stderr, "%s\n", "Error at memory malloc for the lists structure");
                exit(1);
            }

            structOfMappers->lists[i][j].size = 0;
        }

        
    }

    return structOfMappers;
}

/* The function the allocates dinamically the space for the mappers and reducers */
void allocateThreads(pthread_t **mappers, pthread_t **reducers,
                     int numberOfMappers, int numberOfReducers) {

    // We allocate numberOfMappers threads
    *mappers = (pthread_t *) malloc(numberOfMappers * sizeof(pthread_t));
    if (mappers == NULL) {
        fprintf(stderr, "%s\n", "Eroare at memory malloc for mappers");
        exit(1);
    }
    
    // We allocate numberOfReducers threads
    *reducers = (pthread_t *) malloc(numberOfReducers * sizeof(pthread_t));
    if (reducers == NULL) {
        fprintf(stderr, "%s\n", "Eroare at memory malloc for reducers");
        exit(1);
    }
    

    return;
}

/* The function that frees the memory for the fileStatus structure */
void fileStatusMemoryFree(statusOfFiles **fileStatus, int numberOfFiles) {
    // First of all we free the name for each of the files
    for (int i = 0; i < numberOfFiles; i++) {
        free((*fileStatus)[i].fileName);
    }

    // We free the entire structure
    free(*fileStatus);
    *fileStatus = NULL;

    return;
}

/* The function that frees the individual struct for each of the threads
before the shared memory structOfMappers gets freed */
void freeIndividualStruct(individualStruct **currentStruct) {
    free(*currentStruct);

    currentStruct = NULL;
    return;
}

/* The function that parses the argv parameter from main function and puts
them in the corresponding variables */
char *getInput(int *numberOfMappers, int *numberOfReducers, char **argv) {
    *numberOfMappers = atoi(argv[1]);
    *numberOfReducers = atoi(argv[2]);
    
    return argv[3];
}

/* The function that reads the input file and creates the statusOfFiles struct
in order to memorize for each of the input files if it was already taken by a
mapper or not */
void *fileRead(FILE *inFile, char *inputFile, int *numberOfFiles) {
    // We open the input file and read the number of input files we have in this file
    inFile = fopen(inputFile, "rt");
    fscanf(inFile, "%d", numberOfFiles);

    // We allocate memory for the statusOfFiles struct
    statusOfFiles *filesStatus = (statusOfFiles *) malloc(*numberOfFiles * sizeof(statusOfFiles));
    if (filesStatus == NULL) {
        fprintf(stderr, "%s\n", "Eroare at memory malloc for fileStatus array");
        exit(1);
    }

    /* For each of the files, we allocate enough memory for the memorization of the file's
    name, we read it from the input file, we store it and we mark it as being unfinished yet*/
    for (int i = 0; i < *numberOfFiles; i++) {
        char *currentFile = (char *) malloc(100000 * sizeof(char));
        if (currentFile == NULL) {
            fprintf(stderr, "%s\n", "Error at memory malloc for file's name");
            exit(1);
        }

        fscanf(inFile, "%s", currentFile);
        
        filesStatus[i].fileName = strdup(currentFile);
        if (filesStatus[i].fileName == NULL) {
            fprintf(stderr, "%s\n", "Error at memory malloc for file's name");
            exit(1);
        }

        filesStatus[i].finished = false;

        free(currentFile);
    }

    /* We close the input file and return the just created structure to main */
    int returnValue = fclose(inFile);
    if (returnValue != 0) {
        fprintf(stderr, "%s\n", "Error at closing the file");
        exit(1);
    }

    return filesStatus;
}