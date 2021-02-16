## LAB1 of Concurrent Programming Course

### INTRODUCTION
This application consists of three locking algorithms and one barrier algorithm. These primitives are used in  BucketSort algorithm and in a
stand-alone \counter" micro-benchmark. 'perf' is used to investigate the performance of code.
The algorithms include:
 Test-and-set lock
 Test-and-test-and-set lock
 Ticket lock
 MCS Lock (grads only)
 Sense-reversal barrier


### CODE ORGANIZATION


    lab0
    |
    |----bucket_main.c	-   This is the main application file that contains the sequential flow operations
    |			    and the API declaration and definition for bucket sort 
    |----counter_main.c	-   This is the main application file that contains the sequential flow of operations
    |			    for counter application
    |
    |----helper.c	-   This file contains definition of all the helper functions to parse the command 
    |                       line argument, read the elements from file and populate sorted elements to the file
    |
    |----helper.h	-   This file contains declaration of all the helper functions
    |
    |----lock.c		-   This file contains the implementation of the locks - tas, ttas, mcs, ticket and thread
    |
    |----lock.h		-   This file contains the definition of the lock functions
    |
    |----Makefile	-   This file contains the project build and cleanup script


### BUILD INSTRUCTION AND RUN STEPS

    1.) Go to the project directory using the command - cd lab1
    2.) Run "make" inorder to build the project
    3.) Run the executable - mysort with appropriate command-line arguments
    4.) Run "make clean" inorder to clean the directory and remove all the object files and exe


### Parallelization strategy for Merge Sort


    1.) Divide the input elements in equal halves based on number of threads being used.

	2.) Calculate the start and end array value for each thread and store it in a structure.

	3.) Create thread with merge function handler and the populated structure as an argument.

	4.) The thread handler function will perform merge sort in their respective sub-array.

	5.) Then join all the threads and merge all the sub-array to the main array. 

### Parallelization strategy for Bucket Sort

	1.) Have the number of buckets equal to number of threads

	2.) Calculate the maximum element in the array and based of that calculate range of each bucket

	3.) Divide the input elements in equal halves based on number of bucket being used and store in the
		argument structure which will be passed to the thread handler.

	4.) Create thread with bucket function handler and the populated structure as an argument.

	5.) The thread handler function will assign the elements in their sub-array to an appropriate bucket

	6.) Then join all the threads and merge all the sub-array to the main array.

### Locks and Barrier performance stats on counter application

    |  Locks |Run Time(s)|  L1 cache hit rate | Brance Prediction hit rate | page-fault count |
    |--------|-----------|--------------------|----------------------------|------------------|
    |tas     | 0.007997  |  96.74%	          | 98.64%                     |    236           |
    |ttas    | 0.000802  |  94.31%            | 97.88%                     |    238           |
    |mcs     | 40.79373  |  94.99%            | 99.99%                     |    375           |
    |pthread | 0.261842  |  96.29%            | 97.92%                     |    231           |
    |ticket  | 56.121955 |  95.99%            | 99.99%                     |    229           |

    | Barrier| Run Time  |  L1 cache hit rate | Brance Prediction hit rate | page-fault count |
    |--------|-----------|--------------------|----------------------------|------------------|
    |sense   |170.149125 |  99.99%            | 99.99%                     |    232           |
    |pthread | 0.261842  |  87.86%            | 97.92%                     |    228           |

All the above experiments are performed with 50 threads and 100 iteration.

### Locks and Barrier performance stats on bucket sort application

    |  Locks |Run Time(s)|  L1 cache hit rate | Brance Prediction hit rate | page-fault count |
    |--------|-----------|--------------------|----------------------------|------------------|
    |tas     | 0.554375  |  98.46%            | 99.59%                     |    8424          |
    |ttas    | 0.492622  |  99.26%            | 99.73%                     |    8421          |
    |mcs     | 0.589436  |  99.40%            | 99.83%                     |    13519         |
    |pthread | 0.848636  |  94.54%            | 98.89%                     |    8426          |
    |ticket  | 0.609425  |  99.47%	          | 99.82%			           |    8424	      |

All the above experiments are performed with an input file consisting of 650000 elements

### Observation

    1.) According to run-time analysis, the ticket lock and MCS lock is slower as it takes 56 and 40 seconds respectively,
    	beacuse there are more cache misses. TTAS avoids contention miss by waiting threads by leveraging multiple read
	copies. In ticket lock, all waiting threads have cache miss every time lock is released. MCS is also a similiar 
	FIFO lock which uses a queue for the waiting thread.

    2.) TAS has the best L1 cache hit rate and MCS has the worst cache hit rate.
	
    3.) In bucket sort application, similiar performance is observed because the data is feteched from the local cache
    
    4.) Among barriers, the sense reveral barrier takes more time but has a higher cache hit rate.

    5.) In bucket sort application, the results does not match the theoritical assumption and we suspect this is due to the 
        way the OS is handling it.

