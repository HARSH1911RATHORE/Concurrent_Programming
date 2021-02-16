## LAB1 of Concurrent Programming Course

### INTRODUCTION
This application consists of three locking algorithms and one barrier algorithm. These primitives are used in  BucketSort algorithm and in a
stand-alone \counter" micro-benchmark. 'perf' is used to investigate the performance of code.
The algorithms include:
-  Test-and-set lock
-  Test-and-test-and-set lock
-  Ticket lock
-  MCS Lock (grads only)
-  Sense-reversal barrier


### CODE ORGANIZATION
-  	Consists of main.counter,cpp, bucket_main.cpp and Makefile
-  	Makefile compiles the code main_counter.cpp, bucket_main.cpp and generates executable that can be run by typing make. 
-  	main_counter.cpp and bucket_main.cpp contains the main two executables of the project.
-  	The bucketsort program file takes the inputs as  - - name,  -o sortedfile.txt   - - alg=fj or - - alg=bucket, -t thread_count and –lock={tas, ttas, mcs, ticket, pthread} and –bar={sense, pthread}. --name argument prints the full name of the person and alg argument takes options as fj and bucket which sort the file according to the option specified. the locks options need to be one of the selected lock  among the options above. The barrier type will be one of the selected barriers out of the required options. 
-  	The counter program file takes the inputs as - - name, -t thread_count, -o out.txt -i iterations and        - -lock={tas, ttas, mcs, ticket, pthread} or –bar={sense, pthread}. --name argument prints the full name of the person. The concurrency primitives need to be one of the selected lock or one of the selected barriers out of the required options, both cannot be specified together.
-  	For both -t option takes input as number of threads which needs to be run in parallel and for counter program -i option for each thread is the number of iterations to be incremented by.
-  	Bucketsort program takes a non-option argument which is the unsortedfile.txt. This file will have multiple lines which each line having integer of size 0 till INT MAX.
-  	Both the program takes -o argument which in case of mysort program takes option as the sorted file eg.sortedfile.txt with sorted output form unsorted file. The numbers will be sorted and arranged line by line in the sorted file, in this case sortedfile.txt. For counter program, final value of count incremented by all threads will be put in the output text file.
-  	The bucket_main.cpp contains the merge sort algorithm functions, bucket sort algorithm function and function to read and write to the file. Reading the file is done by using char array and converting the array to an integer 1d array. This array will be passed to the mergesort and bucketsort functions depending on the request from the user.
-  	The sorted array will then be written to a new or an existing file line by line.
-  	The file is read using getline which takes row of the file as row of the array and columns of the file as columns of the array. This read array is converted using strtol to an integer array. The char  array which reads input of from the unsorted file uses dynamic memory allocation and reallocates for file content greater than size of initially defined array.
-  	The main_counter.cpp file contains counter_value which is incremented by each thread, i or iteration number of times. This final count value is then written to the output text file at the completion of the program.
-  	In both the programs the elapsed time at the end will be printed on terminal in sec and nanosec.


### COMPILATION AND EXECUTION INSTRUCTIONS

    1.) Eg. of how to run the bucketsort project. First compile the file using make which creates an executable of the name mysort. 
	Eg. of how to run the counter project. First compile the file using make which creates an executable of the name counter. 

    2.) Eg. of how to run the bucketsort project. First compile the file using make which creates an executable of the name mysort. 
	Eg. of how to run the counter project. First compile the file using make which creates an executable of the name counter. 

    3.) Then run the counter executable using the command by typing
 	./counter - - name -o sortedfile.txt -t num_threads – i num_iteration - - lock=<tas, ttas, ticket, mcs, pthread> or  - - bar=<sense, pthread> which starts the counter 		value incremented by each of the num_threads num_iteration times and prints final counter value in the output text file. 


### Parallelization strategy for Merge Sort


    	1.) •	The input file copied into an array will be divided into sub arrays where each individual thread will work on approximately same number of array elements.

	2.) •	The low array index, high array index, array to be populated, thread number is calculated and passed for each thread by using a structure of these values

	3.) •	New threads are created, and their specific indexes and unique parameters are passed to the merge thread function which will sort individual sub arrays using recursive merge sort.

	4.) •	The threads are joined on completion of their unique tasks.

	5.) •	The individual sub arrays are then finally merged and is sent to the sorted_file.txt  

### Parallelization strategy for Bucket Sort

	1.) The input file copied into an array will be divided into sub arrays where each individual thread will work on approximately same number of array elements.

	2.) The number of buckets will be equal to number of threads.

	3.) The low array index, high array index, array to be populated, thread number, individual array length and bucket divider is calculated and passed for each thread by using a structure of these values.

	4.) The low array index, high array index, array to be populated, thread number, individual array length and bucket divider is calculated and passed for each thread by using a structure of these values.

	5.) New threads are created, and their specific indexes and unique parameters are passed to the bucket sort thread function where each thread elements are checked to see which bucket they will go to according to their values in comparison to value of max element in the total array. They are then added to individual sorted buckets.

	6.) The threads are joined on completion of their unique tasks
	
	7.) The individual sub arrays are then finally combined to the final main array to create a list of elements present in a sorted order. This sorted array is then written into a new or an existing text file.

### Locks and Barrier performance stats on counter application
Counter with 10 threads and 10000 iterations with locks or barriers respectively
![]()


All the above experiments are performed with 50 threads and 100 iteration.

### Locks and Barrier performance stats on bucket sort application
Sorting with 10 threads, 382 numbers and 8 digit max number with locks and barriers respectively
![]()


### Observations

    1.) Mcs for counter program has significantly higher page faults and lowest is for tas. The sense barrier has generally more page faults for bucketsort program. The reason for mcs could be link list type implementation, using a queue for waiting threads. Branch prediction rate is highest for ttas in both programs but lowest for pthread in case of bucketsort and counter program. Ttas lock branches predicted is high.

    2.) Ticket lock has the max runtime and tas lock has minimum runtime in both programs.
	
    3.) Tas lock has maximum cache hits for counter program and ttas has maximum cache for bucket sort. The lowest cache hits is pthread lock in both programs. It could have a multiple unique acceses in its internal structure hence, pthread has such performance.
    
    4.) The sense revival barrier has more cache hits for counter in counter program and bucketsort program.

    5.) The reason for better performance for sense barrier is that each thread keeps it own, local sense to detect changes and maintain same phase for all threads. 
    
    6.) The analysis of all the different locks shows us that tas and ttas have higher cache hits than mcs, ticket and pthread lock generally. As mcs and ticket are FIFO locks, they result in better fairness but more cache misses than tas and ttas.
    
    7.) The analysis of all the different locks shows us that tas and ttas have higher cache hits than mcs, ticket and pthread lock generally. As mcs and ticket are FIFO locks, they result in better fairness but more cache misses than tas and ttas. 
    
    8.) LIFO locks like tas and ttas might bottleneck the system due to same core thread trying to acquire lock. We reduce coherence traffic with ttas compared to tas by avoiding cache line pinging all around the system if lock is already held and wait till it is released.
    
    9.) There are some results in both the programs which are other than expected. The reason could be the handling of the program by the operating system and the hardware with different capabilities. 

