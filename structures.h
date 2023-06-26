#ifndef __STRUCTURES_H_
#define __STRUCTURES_H_

/* The structure used for the status of the input 
files in the Mapper's function */
typedef struct {
    char *fileName;
    bool finished;
} statusOfFiles;

/* The struct used for storing partial result for all
the mappers */
typedef struct {
    int *listOfPowers;
    int size;
} listOfLists;

/* The struct each of the mappers uses in order
to communicate and get the information needed */
typedef struct {
    pthread_barrier_t *barrier;
    pthread_mutex_t *mutex;
    statusOfFiles *fileStatus;
    listOfLists **lists;
    int numberOfFiles;
    int numberOfReducers;
} mappersStruct;

/* The struct each of the mappers and reducers use because 
they need access to the same information. Mappers use just the
communStruct and the id and the reducers use the other fields
as well. */
typedef struct {
    mappersStruct *communStruct;
    int *uniqueList;
    int size;
    int id;
    int numberOfMappers;
} individualStruct;

#endif /* __STRUCTURES_H_ */