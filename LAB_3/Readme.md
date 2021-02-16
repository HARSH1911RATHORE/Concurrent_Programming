LAB3
-------------------------------------

INTRODUCTION
------------
Parallelizing a sorting algorithm using OpenMP, either using MergeSort or QuickSort.

CODE ORGANIZATION
-----------------
- Consists of main.c and Makefile
- Makefile compiles the main.c and generates executable that can be run by typing make
- mysort is the main executable of the project.
- main.c contains the main code of the project. This file takes the inputs as  - - name. Name argument prints the full name of the person written the code.
- It also takes a non-option argument which is the unsortedfile.txt. This file will have multiple lines which each line having integer of size 0 till INT_MAX, which is 2147483647.
- It also takes -o argument which takes option as the sorted file eg.sortedfile.txt which will be created and contain the numbers in sorted manner for the unsorted file, in this case unsortedfile.txt. The numbers will be sorted and arranged line by line in the sorted file, in this case sortedfile.txt.
- It also takes -o argument which takes option as the sorted file eg.sortedfile.txt which will be created and contain the numbers in sorted manner for the unsorted file, in this case unsortedfile.txt. The numbers will be sorted and arranged line by line in the sorted file, in this case sortedfile.txt.
- The sorted array will then be written to a new or an existing file line by line.
- Merge sort is an out of place sorting algorithm which needs to create duplicate arrays to execute sorting. The time complexity of merge sort is O(nlogn) and in worst case is O(n^2).
- The file is read using getline which takes row of the file as row of the array and columns of the file as columns of the array. This read array is converted using strtol to an integer array. The char  array which reads input of from the unsorted file uses dynamic memory allocation and reallocates for file content greater than size of initially defined array.


CPMPILATION INSTRUCTION AND EXECUTION
-------------------------------
    1.) Eg of how to run the project. First compile the file using make which creates an executable of the name mysort. 
    2.) Then run the executable using the command by typing
 ./mysort - - name unsortedfile.txt -o sortedfile.txt  which sorts the unsorted file using merge sort utilizing OpenMP library functions.


COMPARISON BETWEEN OPENMP PARALLELIZATION STRATEGY AND PTHREAD 
-------------------

    1.) In OpenMP, the left and right sections of array are parallelized using different OpenMP sections. 

    2.) Using pragma omp section where the thread which gets to the section will execute the left and right respectively. It has an implied barrier at the end.

    3.) Finally, the main thread merges with the forked thread and final sorted in the sorted array which is output to the text file.
    
    4.) This is comparatively very easy in execution as it can parallelize a serial program without much effort compared to pthread.
    
    5.) In pthread, the input file is copied into an array will be divided into sub arrays where each individual thread will work on approximately same number of array elements.
    
    6.) The low array index, high array index, array to be populated, thread number is calculated and passed for each thread by using a structure of these values.
    
    7.) New threads are created, and their specific indexes and unique parameters are passed to the merge thread function which will sort individual sub arrays using recursive merge sort.
    
    8.) The threads are joined on completion of their unique tasks.
    
    9.) The individual sub arrays are then finally merged and is sent to the sorted_file.txt 
    
    10.) With pthread, we need to create separate sub arrays of our large array and pass them to the thread main function separately and finally we need to merge all those sorted sub arrays together, This complexity is reduced in OpenMP as we can choose parts of program we need to parallelize and pass them in the OpenMP parallel functions as and when needed.

PARALLELIZATION STRATEGIES
--------------------------
- The unsorted input file copied into an array where the array will be sorted using mergesort.
- The input file is sorted using recursive merge sort algorithm where each of the two recursive mergesort functions are parallelized using the openMP parallel sections which has an implied barrier at the end.
- This creates different threads for those mergesort calls and executes each of those sections parallelly thus parallelizing the serial program with ease.
- The array is then finally merged and is sent to the sorted_file.txt 


