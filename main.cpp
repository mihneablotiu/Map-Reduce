#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

#include "structures.h"
#include "functions.h"

int main(int argc, char **argv) {
    // First of all we check if the program is tested as it should be
    if (argc != 4) {
        fprintf(stderr, "%s\n", "Run format: ./tema1 <numar_mapperi> <numar_reduceri> <fisier_intrare>");
        exit(1);
    }

    // The input file
    FILE *inFile = NULL;
    int numberOfMappers, numberOfReducers, numberOfFiles;

    // We get the input from the argv main parameter
    char *inputFile = getInput(&numberOfMappers, &numberOfReducers, argv);

    /* We read in an array of structs called fileStatus, all the files from the input file
    and we mark them as being unfinished yet */
    statusOfFiles *fileStatus = (statusOfFiles *) fileRead(inFile, inputFile, &numberOfFiles);

    /* We allocate numberOfMappers mappers and numberOfReducers reducers*/
    pthread_t *mappers, *reducers;
    void *status;
    allocateThreads(&mappers, &reducers, numberOfMappers, numberOfReducers);

    /* We initiate a barrier and a mutex that we are going to use in the
    mappers and reducers functions */
    pthread_barrier_t barrier;
    pthread_barrier_init(&barrier, NULL, numberOfMappers + numberOfReducers);

    pthread_mutex_t mutex;
    pthread_mutex_init(&mutex, NULL);

    /* We create the struct of the Mappers that we are going to send to each of them
    when we start the threads */
    mappersStruct *structOfMappers = (mappersStruct *) allocateMappersStruct(&barrier, &mutex,
                                                       numberOfFiles, numberOfReducers, numberOfMappers ,fileStatus);

    // We start numberOfMappers + numberOfReducers threads as follows
    for (int i = 0; i < numberOfMappers + numberOfReducers; i++) {
        int returnValue;
        // The first numberOfMappers threads ar made for the mappers
        if (i < numberOfMappers) {
            individualStruct *currentStruct = (individualStruct *) allocateIndividualStruct(structOfMappers, i);
            returnValue = pthread_create(&mappers[i], NULL, mappersFunction, currentStruct);
        // The next numberOfReducers threads are made for the reducers
        } else {
            individualStruct *currentStruct = (individualStruct *) allocateIndividualStruct(structOfMappers, i - numberOfMappers);
            currentStruct->numberOfMappers = numberOfMappers;
            returnValue = pthread_create(&reducers[i - numberOfMappers], NULL, reducersFunction, currentStruct);
        }

        // We check if the creation of the thread was a success
        if (returnValue != 0) {
            fprintf(stderr, "Error at creating the thread %d\n", i);
            exit(-1);
        }
    }

    /* After the creation of the threads we wait for them using numberOfMappers
    + numberOfReducers joins */
    for (int i = 0; i < numberOfMappers + numberOfReducers; i++) {
        int returnValue;
        if (i < numberOfMappers) {
            returnValue = pthread_join(mappers[i], &status);
        } else {
            returnValue = pthread_join(reducers[i - numberOfMappers], &status);
        }

        if (returnValue != 0) {
            fprintf(stderr, "Error at waiting the thread %d\n", i);
            exit(-1);
        }
    }

    /* Before the end of the program we free all the used memory
    in order not to have memory leaks */
    pthread_barrier_destroy(&barrier);
    pthread_mutex_destroy(&mutex);
    fileStatusMemoryFree(&fileStatus, numberOfFiles);
    free(mappers);
    free(reducers);
    for (int i = 0; i < numberOfMappers; i++) {
        for (int j = 0; j < numberOfReducers; j++) {
            free(structOfMappers->lists[i][j].listOfPowers);
        }

        free(structOfMappers->lists[i]);
    }

    free(structOfMappers->lists);
    free(structOfMappers);

    return 0;
}