Bloțiu Mihnea-Andrei - 333CA - Algoritmi paraleli și distribuiți - Tema 1
Mappers and Reducers - 17.11.2022


The overall point of this homework was to implement a parallel program in
pthreads in order to find the numbers bigger than 0 that are perfect powers
from a file set and return the number of unique numbers for each exponent.

This was done accordingly to the indications we had and that being said, the
project was structured as follows:
    - main.cpp the file where the logic of the program happens;
    - functions.cpp the extra functions used by the main functions;
    - functions.h the headers of those functions mentioned above;
    - structures.h the structures used in this project.
    - Makefile
    - README - the documentation of the project

The whole flow of the project is described by the main.cpp file as folows:
    - First of all we read the input from the command line: the number of
    mappers, the number of reducers and the input file;
    - I parse the input file, dividing its content into the number of files
    and each of those files. That being said, I create an array of files
    that memorizes the name of the input file and a boolean representing
    if the file was already taken by a mapper or not yet;
    - I allocate the threads for the mappers and reducers that we are going
    to use later, the barrier (used for being sure that all the mappers have
    finished before the reducers start their work) and the mutex (used to
    assure that at a specific moment in time, just one mapper will take a 
    new file);
    - I allocate the general struct for each of the mappers containing the
    barrier, the mutex, the status of the files, the matrix for the partial
    results each of them is going to give, the number of files that have to
    be processed and the number of reducers in order for the mappers to know
    until which power they have to check for perfect numbers;
    - We launch all the threads, first threads being for the mappers and the
    last threads for the reducers using an individual struct as the argument
    of their function. This contains the general struct (the struct of mappers)
    with common information and some particular information for each of them
    such as their id or each unique list for each reducer that they are going
    to build after mappers finished their job;
    - We join all the threads so we wait for all the threads to finish their
    tasks and at the end we free all the used resources before exiting the
    program;

structures.h:
    - statusOfFiles: the structure used for memorizing all the input files
    and if they were already taken by a mapper or not;
    - listOfLists: the struct that mappers are using of each of the powers
    they are calculating perfect numbers (each of the perfect numbers starting
    from the second power until number of reducers + 1 power is going to be stored
    in one of those structs. So, there will be number of reducers structs for each
    of the mappers);
    - mappersStruct: the commun struct each of the threads (including reducers)
    are sharing. It is called the mappersStruct because the mappers are populating
    this struct;
    - individualStruct: the struct that contains the commun information (the 
    mappers struct) plus their particular information such as their id used for
    both the mappers and the reducers but also the uniqueList and the size of this
    list (used just by the reducers);

functions.cpp:
    - getInput:
        * This is the function that just parses the argv from the main function and
        selects the number of reducers, the number of mappers and the input file from
        it;
    
    - fileRead:
        * Now, after we have the input file, we need to parse it and create the
        file status array;
        * We allocate the array and the name for each of the files in the array. We
        read files one by one and we store them in the array marking them all as
        not being yet taken by any mapper;
        * At the end, we return the file status array to the main function;
    
    - allocateThreads:
        * Now we can also allocate space for the mappers and reducers;

    - allocateMappersStruct:
        * Now we can create the commun struct that each of the mappers and reducers is
        going to use in their process;
        * We allocate space for the struct and we put in this struct the already defined
        variables such as the mutex, the barrier, the number of files, the status of the
        files and the number of reducers;
        * We allocate space for the matrix of partial results (where each of the mappers
        is going to have one row). So, in this matrix, there are going to be number of
        mappers rows, and each of the rows is going to have number of reducers columns
        (because each of the mappers is going to determine perfect numbers for number of
        reducers powers). Each of the elements in the matrix is going to be an array of 
        all the elements that the current mapper found to be perfect numbers of the 
        current power;
        * We return the just created structure to the main function.

    - allocateIndividualStruct:
        * After we created the commun struct that the mappers are going to populate,
        we can create the individual struct for each of the mappers and reducers
        that is going to have the mappers struct plus the individual information
        for each of the threads;
        * We return the struct to main;

    - mappersFunction:
        * Having the individual structs created, we can start the mappers threads;
        * Firstly each of the mappers takes its individual struct and iterates
        through the status of files struct trying to find dinamically a undeveloped
        file;
        * This action is a critical zone because we have to make sure with the mutex
        that just one thread tries to take one file at a time in order not to
        have two threads working in the same time at the same file;
        * After one thread finds a available file, it marks it as taken an then lets
        the other threads search for undeveloped files;
        * Everytime a thread finds a available file, it opens the file and interrogates
        it trying to find the perfect powers inside that file;
        * After all the files were interrogated, we can free the individual struct
        of each of the mappers and then put a barrier in order to wait for all the
        mappers to finish their files;
        * This is a dinamic way of dividing the files among all of the threads which is
        in my opinion good enough because it replicates the repetivite workers design.
        We just have an array of lists and every time, each of the threads is going to
        take the first available file. That being said, a thread that has the luck of
        taking a smaller file at the beginning is going to finish it faster and it is
        going to take another file in the mean time while other threads work on longer
        files;
    
    - interrogateFile:
        * Once a mapper decided which of the files is available for him is going to
        interrogate that file;
        * It reads the number of numbers in that file and iterates through the file
        taking each number in the file one by one;
        * For each of those numbers, it tries to find if the current number is a
        perfect number from the second power until the number of reducers + 1 power;
        * For each of those powers, it does a binary search between 1 and the square
        root of the current number + 1 (it is good enough because the first power that
        we are going to search for is 2 so the second power of the square root + 1 is
        already bigger than the number itself);
        * If it found a number that powered to the current power equals the current
        number, then it adds it to his row in the matrix to the corresponding column
        which has a direct association with the current power that the mapper was
        searching for;
        * At the end of the files, each of the mappers would have already populated his
        row in the matrix;

    - binraySearch:
        * How we said before, for each of the powers, we have to make a binary search
        between 1 and the square root of the number + 1 in order to see if in
        this interval it exists a number that raised to the current power it equals
        the current number we are investigating;
        * To do that we do a binary search as follows. We always take the middle
        value inside this intervale. We raise it to the current power and if the
        result is equal to the current number then it means that the current number
        is a perfect number;
        * Otherwise, if the result is smaller than the number we have to search in the
        right part of the interval, or if the result is bigger than the current number
        then we have to search in the left part of the interval.

    - reducersFunction:
        * After all the mappers populated their row in the matrix, we start the
        reducers function where the first thing that is done is that each of the
        reducers gathers all the numbers from the same power from all the different
        mappers in one list;
        * Then each of the reducers remove the duplicates from its own list;
        * Opens the corresponding file and writes inside the file the length of
        the final list, after the duplicates were removed.

    - reduceLists:
        * The first operation that a reducer does how I said above is to create a
        unique list with all the perfect numbers of the same power from all the mappers;
        * To do that, each of the reducers iterates through every row in the partial
        result matrix, goes to the column equal to its id and adds all the values to
        its unique list;
        * At the end of this process, each of the reducers is going to return to the
        reducers functions a unique list, each one representing all the perfect numbers
        of one value;

    - removeDuplicates:
        * After each of the reducers has a unique list with all the values from one
        power it has to remove the duplicates inside it;
        * To do that we just add all the values inside a list to a set (because we know
        that the set cannot have duplicate values) and returns just the final
        size of the set that is going to be written inside the exit file.


    
