/* reference: https://www.geeksforgeeks.org/quick-sort/ */
/* reference: https://www.geeksforgeeks.org/merge-sort/ */
/* reference: https://www.youtube.com/watch?v=TzeBrDU-JaY */
/* reference: https://www.youtube.com/watch?v=COk73cpQbFQ */
/* reference: https://www.youtube.com/watch?v=3Bbm3Prd5Fo */
/* reference: https://www.youtube.com/watch?v=0nlPxaC2lTw */
/* reference: https://linux.die.net/man/3/getopt */
/* reference: test.c provided by professor */
/*reference: https://www.geeksforgeeks.org/map-insert-in-c-stl/ */
/*reference: https://www.tutorialspoint.com/cplusplus-program-to-implement-bucket-sort */
/* reference: https://stackoverflow.com/questions/33048079/test-and-test-and-set-in-c */

/*C LIBRARY HEADER FILES*/
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <stdio.h> 
#include <pthread.h>
#include <unistd.h>
#include <math.h>
#include <bits/stdc++.h> 

/*C++ LIBRARY HEADER FILES*/
#include <iostream>
#include <set> 
#include <cmath>
#include <iterator> 
#include <vector>
#include <algorithm>
#include <sstream> 
#include <atomic>         
#include <thread>         
       
#define LMAX 255                /*to read the input file */
using namespace std;
vector <map <int,int> > bucket; /*Uisng stl vector map to create dynamic arrays*/
atomic<bool> lock_stream = ATOMIC_FLAG_INIT;
atomic<int> next_num ;
atomic<int> now_serving ;

char *non_option_argument;  /*unsorted file.txt which will be passed as command line argument*/
char *sorted_file;          /*file to which the sorted elements from the unsorted array will be put*/
int idx = 0;                /* index of array of no of lines  */
int merge_flag = 0;         /*if algo selected for sorting the unsorted file is merge, merge flag will be 1*/
int quick_flag = 0;         /*if algo selected for sorting the unsorted file is quick, merge flag will be 1*/
int bucket_flag = 0;        /*flag indicating bucket sort is chosen*/
int thread_count = 0;       /*count of number of threads used*/

int lock_used = 0;          /* both lock and barrier are used*/
int barr_used = 0;
pthread_mutex_t lock_pthread_mutex;     /* defining mutex lock*/
pthread_barrier_t barrier_pthread_bar;  /* defining the barrier */
pthread_barrier_t bar;      /*defining barrier*/
struct timespec start, end_time;    /*calculate time at start and end of thread*/
pthread_mutex_t lock1 = PTHREAD_MUTEX_INITIALIZER; /*initialize the mutex lock*/
size_t it = 0;              /*  variable iterator  */

typedef enum lock_choice    /* selecting the type of lock */
{
    mcs,
    tas,
    ttas,
    lockPthread,
    ticket
}lock_choice_number;
lock_choice_number choice_lock;
typedef enum unlock_choice   /* unlock choice */
{
    unlockMCS,
    unlockTAS,
    unlockTTAS,
    unlockPthread,
    unlockTICKET
}unlock_choice_number;
unlock_choice_number choice_unlock;
typedef enum barrier_choice  /* barrier choice */
{
    sense,
    pthread_bar
}barrier_choice_number;
barrier_choice_number choice_barrier;   
void ttas_lock( )   /* ttas lock if a lock is held or lock is tried to be acquired and it fails it will leave */
{
    do 
    {
        while ( lock_stream )  continue;
    } while ( lock_stream.exchange( true )); // actual atomic locking
    return;
}

void ttas_unlock( ) /* changing the flag to false */
{
    lock_stream.store( false );
}

void tas_lock( )             /* wait until the time you are successful in getting lock */
{
  	bool expected, new_value;
    do
    {
        new_value = true;
        expected = false;
    }while(!lock_stream.compare_exchange_strong(expected,new_value));
}

void tas_unlock( )  /* changing the flag to false */
{
    lock_stream.store( false );
}

void ticket_lock( ) /* my number will check where i am in the line in getting the lock */
{
    int my_num = next_num.fetch_add(1, memory_order_seq_cst );
    while(now_serving.load( memory_order_seq_cst)!=my_num ){}
}

void ticket_unlock( )  /* increment the now serving count */
{
    now_serving.fetch_add( 1, memory_order_seq_cst );
}

class Node
{
public:                     /* atomic next and wait */
	atomic<Node*> next;
	atomic<bool> wait;
};

atomic<Node*> tail{NULL} ;  /* atomic tail */


class MCSLock 
{
public:
	void acquire(Node *myNode) {

	Node *oldTail = tail.load(memory_order_seq_cst);
	myNode->next.store(NULL, memory_order_relaxed);
	while (!tail.compare_exchange_strong(oldTail, myNode)) {
		oldTail = tail.load();
	}
// if oldTail == NULL, we’ve
// acquired the lock
// otherwise, wait for it
	if (oldTail != NULL) 
    {
		myNode->wait.store(true, memory_order_relaxed);
		oldTail->next.store(myNode,memory_order_seq_cst);
		while (myNode->wait.load(memory_order_seq_cst)) {}
	}
}
	void release(Node *myNode) {

		Node* temp_node = myNode;
        // no one is waiting, and we just
        // freed the lock
		if (tail.compare_exchange_strong( temp_node, NULL, memory_order_seq_cst )) 
        {

		} 

        // hand lock to next waiting thread
        else 
        {
			while (myNode->next.load(memory_order_seq_cst) == NULL) {}
			myNode->next.load()->wait.store(false,memory_order_seq_cst);
		}
	}
};
MCSLock lockMCS;

/* sense reversal barrier change the sense once all threads reach that stage */
typedef struct sense_variables
{
    atomic<int> cnt ;
    atomic<int> sense ;
    int N;
}barrier_sense_variables;
barrier_sense_variables variables;
void sense_barrier ( )
{
    thread_local bool my_sense = 0;
	if (my_sense == 0) 
    {
		my_sense = 1;
	} 
    else 
    {
		my_sense = 0;
	}

	int cnt_cpy = variables.cnt.fetch_add( 1 );
	if ( cnt_cpy == variables.N - 1 )                       /* if count copy is equal to last thread sense state will change*/
    {
		variables.cnt.store( 0, memory_order_relaxed );
		variables.sense.store( my_sense );
	}   
    else                                                /* read the value of sense */
    {
		while (variables.sense.load() != my_sense);
	}
}

/* phtread lock */
void pthread_lock ()
{
	pthread_mutex_lock(&lock_pthread_mutex);
}

/* phtread unlock */
void pthread_unlock ()
{
	pthread_mutex_unlock(&lock_pthread_mutex);
}

/* phtread barrier */
void pthread_barrier ()
{
	pthread_barrier_wait(&barrier_pthread_bar);
}

typedef struct fjmerge_sort
{ 
    int low_array_index_m;
    int high_array_index_m;
    int thread_number_merge;
    int *thread_array;
}FJ_Merge;

typedef struct fjquick_sort
{ 
    int low_array_index_q;
    int high_array_index_q;
    int thread_number_quick;
    int *thread_array_quick;
}FJ_Quick;

typedef struct lkbucket_sort
{ 
    int array_length;
    int bucket_divider;
    int thread_number_bucket;
    int *thread_array_bucket;
    int low_array_index_b;
    int high_array_index_b;
}LK_Bucket;

//-----------------------------------QUICK SORT-------------------------------------------//
// function which swaps the  two elements 
void swap( int* a, int* b ) 
{ 
    int temp = *a; 
    *a = *b; 
    *b = temp; 
} 
  
/* This function takes the array to be sorted and takes its last element 
as pivot for partition where smaller elements of array are left of pivot
and larger elements compared to pivot are to the right of pivot*/   
int partition ( int arr[], int low, int high ) 
{ 
    int pivot = arr[ high ];   //pivot where the elements of array will be seperated from
    int i = ( low - 1 );       // smaller element index
  
    for ( int j = low; j <= high - 1; j++ ) 
    { 
        //checks if current element is smaller than pivot
        if ( arr[ j ] < pivot ) 
        { 
            i++;    // increments smaller element index  
            swap( &arr[ i ], &arr[ j ]); 
        } 
    } 
    swap( &arr[ i + 1 ], &arr[ high ] ); 
    return ( i + 1 ); 
} 
  
/* QuickSort 
 arr[] : Array to be sorted, 
  low  : Starting index, 
  high : Ending index */
void quickSort( int arr[], int low, int high ) 
{ 
    if ( low < high ) 
    { 
        /* partition_index is partitioning index*/
        /* arr[p] is now at right place */
        int partition_index = partition( arr, low, high ); 
  
        /*Sorts the elements by dividing them into sub arrays and sorting them same as 
        the original array by defining the last element as pivot*/
        quickSort( arr, low, partition_index - 1 ); 
        quickSort( arr, partition_index + 1, high ); 
    } 
} 

//--------------------------------------MERGE SORT-----------------------------//
// The function is used to merges two subarrays of arr[]. 
// First subarray is arr[ left..mid ] 
// Second subarray is arr[ mid + 1eft..right ] 
void merge( int arr[], int left, int mid, int right ) 
{ 
    int i, j, k;            //variable to iterate
    int n1 = mid - left + 1; 
    int n2 = right - mid; 
  
    /*  temporary arrays */
    int L[ n1 ], R[ n2 ]; 
  
    /* Duplicate the data to the temporary arrays left and right */
    for ( i = 0; i < n1; i++ ) 
    {
        L[ i ] = arr[ left + i ]; 
    }
        
    for ( j = 0; j < n2; j++ ) 
    {
        R[ j ] = arr[ mid + 1 + j ]; 
    }
        
    /* Merge the duplicate arrays to main array arr[left..right]*/
    i = 0; // starting index of subarray 1
    j = 0; // starting index of subarray 2
    k = left; // starting index of main array 
    while ( i < n1 && j < n2 ) 
    { 
        if ( L[ i ] <= R[ j ] )   //check if left array's elements is less than right array's elements
        { 
            arr[ k ] = L[ i ];    //if yes, add the left array's element to main array
            i++; 
        } 
        else                      //check if right array's elements is less than left array's elements
        { 
            arr[ k ] = R[ j ];    //if yes, add the right array's element to main array
            j++; 
        } 
        k++; 
    } 
  
    /* Check the array left if it has any elements still remaining, if yes append to main array */
    while ( i < n1 ) 
    { 
        arr[ k ] = L[ i ]; 
        i++; 
        k++; 
    } 
  
    /* Check the array right if it has any elements still remaining, if yes append to main array */
    while ( j < n2 ) 
    { 
        arr[ k ] = R[ j ]; 
        j++; 
        k++; 
    } 
} 
  
/*left is left most index and right  most index of array to be sorted */
void mergeSort( int arr[], int left, int right ) 
{ 
    if ( left < right ) 
    { 
        //mid is the approx middle element of the array to be sorted
        int mid = left + ( right - left ) / 2; 
  
        // Sortleft portion and the right portion before merging it to the final array 
        mergeSort( arr, left, mid ); 
        mergeSort( arr, mid + 1, right ); 
  
        merge( arr, left, mid, right ); 
    } 
} 

/* divider for bucket to calculate which elements of the input array go to which file */
int get_range( int arr[], int size, int threadcount )
{
    int location = 0;
    int divider,c;
    int buckets = threadcount;              /* the number of buckets is equal to the total threads */
    int max = 0;
    for (c = 1; c < size; c++)
    if (arr[c] > arr[location])            /* calculating the location of the array */
        location = c;
    max = *max_element(arr, arr + size); /* used to calculate the max element of the array */
    divider = ceil(float(max+1)/buckets); /* divider used to filter out elements into different buckets */
    return divider;
}

/* thread main function where different threads will execute sorting operation parallely */
void* mergesort_thread( void *args )
{
    int left, right, mid;
    FJ_Merge tid = *(( FJ_Merge* )args );       /* dereference the pointer */

    switch( choice_barrier )
    {
        case( sense ):
        {
            sense_barrier( );
            break;
        }
        case( pthread_bar ): 
        {
            pthread_barrier();
            break;
        }

    }
    if ( tid.thread_number_merge == 1 )
        clock_gettime( CLOCK_MONOTONIC, &start ); /* get clock time at the start */
    left = tid.low_array_index_m;                 /* left index */
    right = tid.high_array_index_m;               /* right index */

    switch( choice_barrier )
    {
        case( sense ):
        {
            sense_barrier( );
            break;
        }
        case( pthread_bar ): 
        {
            pthread_barrier();
            break;
        }

    }
    printf( "\nThread %u reporting for duty\n",tid.thread_number_merge );

    if ( left < right ) 
    { 
        //mid is the approx middle element of the array to be sorted
        mid = left + ( right - left ) / 2; 
  
        // Sortleft portion and the right portion before merging it to the final array 
        mergeSort( tid.thread_array, left, mid ); 
        mergeSort( tid.thread_array, mid + 1, right ); 
        merge( tid.thread_array, left, mid, right ); 
    }
    
    switch( choice_barrier )
    {
        case( sense ):
        {
            sense_barrier( );
            break;
        }
        case( pthread_bar ): 
        {
            pthread_barrier();
            break;
        }

    }
    return 0;    
}

/* Bucket sort for different individual threads parallely where filtering is done based on size of 
element with respect to max element in the array */
void * bucket_sort_thread( void *args )
{
    LK_Bucket tid = *(( LK_Bucket* )args );
	int i = 0, j = 0;
    switch( choice_barrier )                        /* using the type of barrier passed as command line input */
    {
        case( sense ):
        {
            sense_barrier( );
            break;
        }
        case( pthread_bar ): 
        {
            pthread_barrier();
            break;
        }

    }
      
    if(tid.thread_number_bucket == 1 ){
        clock_gettime(CLOCK_MONOTONIC,&start);      /* start the clock */
    }
    switch( choice_barrier )                        /* using the type of barrier passed as command line input */
    {
        case( sense ):
        {
            sense_barrier( );
            break;
        }
        case( pthread_bar ): 
        {
            pthread_barrier();
            break;
        }
    }
    //pthread_barrier_wait(&bar);
    printf( " \n-----------------------------Thread %u reporting for duty------------------------\n",tid.thread_number_bucket );
  

	/* the element size is seen compared to max and put in respective sorted buckets */
	for (i = tid.low_array_index_b; i <= tid.high_array_index_b; i++) 
    {
		j = ( tid.thread_array_bucket[i] / tid.bucket_divider );	

        switch( choice_lock )                                                               /* using the type of lock passed as command line input */
        {
            case( ticket ):
            {
                ticket_lock();
                bucket[j].insert({tid.thread_array_bucket[i],tid.thread_array_bucket[i]}); /* elements inserted in bucket map having key and value */	
                ticket_unlock();
                continue;
            }
            case( tas ):
            {
                tas_lock();
                bucket[j].insert({tid.thread_array_bucket[i],tid.thread_array_bucket[i]}); /* elements inserted in bucket map having key and value */	
                tas_unlock();
                continue;
            }
            case( ttas ):
            {
                ttas_lock();
                bucket[j].insert({tid.thread_array_bucket[i],tid.thread_array_bucket[i]}); /* elements inserted in bucket map having key and value */	
                ttas_unlock();
                continue;
            }
            case( lockPthread ):
            {
                pthread_lock();
                bucket[j].insert({tid.thread_array_bucket[i],tid.thread_array_bucket[i]}); /* elements inserted in bucket map having key and value */	
                pthread_unlock();
                continue;
            }
            case( mcs ):
            {
                Node *mynode = new Node;                                                    /* initializing node with some memory */
                lockMCS.acquire(mynode);
                bucket[j].insert({tid.thread_array_bucket[i],tid.thread_array_bucket[i]}); /* elements inserted in bucket map having key and value */	
                lockMCS.release(mynode);
                continue;
            }
        
        }	
    }
    switch( choice_barrier )        /* using the type of barrier passed as command line input */
    {
        case( sense ):
        {
            sense_barrier( );
            break;
        }
        case( pthread_bar ): 
        {
            pthread_barrier();
            break;
        }

    }

	return 0;
}

/* This function is used to read the input file and copy elements into the array */
int *file()
{
        char **array = NULL;        /* array of pointers to char        */ 
        char *ln = NULL;            /* NULL forces getline to allocate  */
        size_t n = 0;               /* buf size, 0 use getline default  */
        ssize_t nchr = 0;           /* number of chars actually read    */
        // size_t idx = 0;             /* array index for number of lines  */
        size_t lmax = LMAX;         /* current array pointer allocation */
        FILE *fp = NULL;            /* file pointer                     */
        if (! ( fp = fopen ( non_option_argument, "r" ) ) ) 
        { /* open file for reading    */
            fprintf ( stderr, "error: file open failed " );
            return (int *)1;
        }

        /* allocate LMAX pointers and set to NULL. Each of the 255 pointers will
        point to (hold the address of) the beginning of each string read from
        the file below. This will allow access to each string with array[x].
        */
        if (!( array = (char**)calloc ( LMAX, sizeof * array ) ) ) 
        {
            fprintf ( stderr, "error: memory allocation failed." );
            return (int *)1;
        }

        /* prototype - ssize_t getline (char **ln, size_t *n, FILE *fp)
        above we declared: char *ln and size_t n. Why don't they match? Simple,
        we will be passing the address of each to getline, so we simply precede
        the variable with the urinary '&' which forces an addition level of
        dereference making char* char** and size_t size_t *. Now the arguments
        match the prototype.
        */
        while (( nchr = getline ( &ln, &n, fp )) != -1 )    /* read line    */
        {
            while ( nchr > 0 && ( ln[ nchr - 1 ] == '\n' || ln[ nchr - 1 ] == '\r' ) )
                ln[ --nchr ] = 0;     /* strip newline or carriage rtn    */

            /* allocate & copy ln to array - this will create a block of memory
            to hold each character in ln and copy the characters in ln to that
            memory address. The address will then be stored in array[idx].
            (idx++ just increases idx by 1 so it is ready for the next address) 
            There is a lot going on in that simple: array[idx++] = strdup (ln);
            */
            array[ idx++ ] = strdup ( ln );

            if ( idx == lmax ) {      /* if lmax lines reached, realloc   */
                char **tmp = (char**)realloc ( array, lmax * 2 * sizeof * array );
                if ( !tmp )
                    return (int *)-1;
                array = tmp;
                lmax *= 2;
            }
        }
        if ( fp ) 
            fclose ( fp );        /* close file */
        if ( ln ) 
            free ( ln );          /* free memory allocated to ln  */

        int *numbers;

   /* Initial memory allocation */
        numbers = (int *) malloc( idx * sizeof( int ) );    

        /* 
            process/use lines in array as needed
            (simple print all lines example below)
        */

        char *endptr[ idx ]; 
        int count = 0;
        for ( it = 0; it < idx; it++ )   
        {
            numbers[ it ] = strtol( array[ it ], &endptr[ it ], 0 ); 
        }
        printf ("\n");
        for ( it = 0; it < idx; it++ )        /* free array memory    */
            free ( array[ it ] );
        free ( array );                       /* free array */
        return numbers;
}

int main( int argc, char **argv )
{
    int character;                      // character which is passed from command line
    int option_index = 0;               // the index of options
    char *option_to_argument_alg;       // it is the algorithm option, that is merge or quick sort
    char *option_to_argument_bar;       // it is the algorithm option, that is sense or pthread barrier
    char *option_to_argument_lock;       // it is the algorithm option, for lock
    char merge_sort[ 3 ] = "fj";     // for comparing the option to argument alg is merge
    char quick_sort[ 8 ] = "fjquick";     // for comparing the option to argument alg is quick 
    char bucket_sort[ 7 ] = "bucket";
    char barrier_sense[ 6 ] = "sense";     // for comparing the option to argument barrier is sense
    char barrier_pthread[ 8 ] = "pthread";     // for comparing the option to argument barrier is pthread 
    char lock_tas[ 4 ] = "tas";         // for comparing the option to argument lock is tas 
    char lock_ttas[ 9 ] = "ttas";       // for comparing the option to argument lock is ttas 
    char lock_ticket[ 7 ] = "ticket";   // for comparing the option to argument lock is ticket 
    char lock_msc[ 4 ] = "mcs";         // for comparing the option to argument lock is mcs 
    char lock_pthread[ 8 ]="pthread";   // for comparing the option to argument lock is pthread

    /*maintains the long option list of arguments passes in the command line*/
    static struct option long_options[] =  {
                                                { "bar", 1, 0, 'b' },
                                                { "alg", 1, 0, 'a' },
                                                { "lock", 1, 0, 'l' },
                                                { "name", 0, 0, 'n' },
						                        { "thread", 1, 0, 't' },
                                                { 0, 0, 0, 0 }
                                            };
    /*checks if all characters or aguments passed in the command line have been read*/
    while (( character = getopt_long( argc, argv, "a:b:l:no:t:", long_options, &option_index )) != -1 )
    {
        switch( character )
        {
            case 'a':                   // checks if alg is passed
            {
                printf( "\n--alg-> option = %s\n", optarg ); //prints the algorithm option which is merge or quick
                option_to_argument_alg = optarg;             //optarg maintains the argument
                                                             //strcmp the value of option passed to --alg is merge
                if ( strcmp( option_to_argument_alg, merge_sort ) == 0 )
                {
                    int i,j;
                    merge_flag = 1;                          // merge flag is set
                }  
                                                             // strcmp the value of option passed to --alg is quick
                else if ( strcmp( option_to_argument_alg, quick_sort ) == 0 ) 
                {
                    quick_flag = 1;                          // quick flag is set
                }

                else if ( strcmp( option_to_argument_alg, bucket_sort ) == 0 )
                {
                    bucket_flag = 1;                         // bucket flag is set
                }

                break;
            }

            case 'b':                                       // checks if alg is passed
            {
                barr_used = 1;
                printf( "\n--bar-> option = %s\n", optarg ); //prints the algorithm option which is sense or pthread
                option_to_argument_bar = optarg;             //optarg maintains the argument
                                                             //strcmp the value of option passed to --bar is sense
                if ( strcmp( option_to_argument_bar, barrier_sense ) == 0 )
                {
                    choice_barrier = sense;
                }  
                                                             // strcmp the value of option passed to --bar is pthread
                else if ( strcmp( option_to_argument_bar, barrier_pthread ) == 0 ) 
                {
                    choice_barrier = pthread_bar;
                }

                break;
            }

            case 'l':                                          // checks if alg is passed
            {
                lock_used = 1;
                printf( "\n--lock-> option = %s\n", optarg ); //prints the algorithm option which is tas,ttas, mcs, ticket or pthread
                option_to_argument_lock = optarg;             //optarg maintains the argument
                                                             //strcmp the value of option passed to --lock is mcs
                if ( strcmp( option_to_argument_lock, lock_msc ) == 0 )
                {
                    choice_lock = mcs;
                    choice_unlock = unlockMCS;
                }  
                                                             // strcmp the value of option passed to --lock is pthread
                else if ( strcmp( option_to_argument_lock, lock_pthread ) == 0 ) 
                {
                    choice_lock = lockPthread;
                    choice_unlock = unlockPthread;
                }

                else if ( strcmp( option_to_argument_lock, lock_tas ) == 0 ) // strcmp the value of option passed to --lock is tas
                {
                    choice_lock = tas;
                    choice_unlock = unlockTAS;
                }

                else if ( strcmp( option_to_argument_lock, lock_ttas ) == 0 ) // strcmp the value of option passed to --lock is ttas
                {
                    choice_lock = ttas;
                    choice_unlock = unlockTTAS;
                }

                else if ( strcmp( option_to_argument_lock, lock_ticket ) == 0 ) // strcmp the value of option passed to --lock is ticket
                {
                    choice_lock = ticket;
                    choice_unlock = unlockTICKET;
                }

                break;
            }

            case 'o':                     // checks if -o argument is passed and stores the file to be sorted in sorted_file
            {
                sorted_file = optarg;
                printf( "\n-o-> option = %s\n", optarg );
                
                break;
            }
            case 'n':                    // checks if --name argument is passed and prints the name of the user
            {
                printf( "\nHARSH RATHORE\n" );
                break;
            }

            case 't':
            {
                thread_count = atoi(optarg); //checks -t argument and adds thread count to thread_count
                printf( "The number of threads is %s",optarg );
                
                break;
            }

            case '?':                   // checks if no argument is passed to option which requires an argument
            {
                if ( optopt == 'a')
                    printf( "Option %c requires an argument merge or quick", optopt );
                else if ( optopt == 'o' )
                    printf( "Option %c requires an argument which is the file that is the sorted file", optopt );
                else if ( optopt == 't' )
                    thread_count = 5;           /* if no option specified take thread as 5 */
                else if ( optopt == 'b')
                    printf( "Option %c requires an argument pthread or sense", optopt );  
                else if ( optopt == 'l')
                    printf( "Option %c requires an argument pthread mcs tas ttas ticket", optopt ); 
                break;
            }
            default:                    // checks for unkown character
                printf( " returned character %c optopt = %c",character, optopt );
        }
    }

    if (optind < argc) 
    {
        /* checks for the non-option argument passes which in our case will 
        be the unsorted text file*/
        printf("\nnon-option ARGV-elements: ");
        while (optind < argc)
        {
            non_option_argument = argv[ optind ];
            printf("%s ", argv[ optind++ ]);
        }
            
        printf("\n");
    }

    if (barr_used==1 && choice_barrier==pthread_bar)                        /* initialize the barrier pthread*/
    {
        pthread_barrier_init( &barrier_pthread_bar, NULL, thread_count ); 
    }

    if (barr_used==1 && choice_barrier==sense)                        /* initialize the barrier pthread*/
    {
        variables.N = thread_count;
    }

    if ( thread_count <= 0 )
        thread_count = 1;
    else if ( thread_count > 150 )
    {
        printf( "\nERROR: Too many threads\n");
        exit( -1 );
    }
        size_t args[thread_count];

    pthread_t threads[thread_count];
    pthread_barrier_init( &bar, NULL, thread_count );       /* initialize the barrier pthread*/

    if ( merge_flag == 1 )
    {
        size_t i;                                  // iterator
        int ret;                                   // pthread_create return value
        int individual_thread_array_length;        // array length for different threads
        merge_flag = 0;                            // flag is made zero
        int *numbers;                              // the unsorted file is read
        numbers = file();                          // read the file into the array
        int low = 0;
        FJ_Merge thread_array_merge[idx];           // structure for task merge for different threads
        individual_thread_array_length = idx/thread_count; 

        for ( i = 1; i < thread_count; i++ )
        {
            low += individual_thread_array_length;
            args[ i ] = i + 1;
            if ( i != ( thread_count - 1 ) )                        // values of low index, high index, thread number and assigning the merge array
            {
                thread_array_merge[ i ].low_array_index_m = low ;
                thread_array_merge[ i ].high_array_index_m = low + individual_thread_array_length -1;
                thread_array_merge[ i ].thread_array = numbers;
                thread_array_merge[ i ].thread_number_merge = args[ i ];
            }
            else                                                    // values of low index, high index, thread number and assigning the merge array
            {
                thread_array_merge[ i ].low_array_index_m = low ;
                thread_array_merge[ i ].high_array_index_m =  idx - 1;
                thread_array_merge[ i ].thread_array = numbers;
                thread_array_merge[ i ].thread_number_merge = args[ i ];
            }

            ret = pthread_create( &threads[ i ], NULL, &mergesort_thread, &thread_array_merge[ i ]); // create p_threads 
            if ( ret )
            {
                printf( "ERROR: pthread_create: %d\n", ret);
                exit( -1 );
            }
        }

        thread_array_merge[ 0 ].low_array_index_m = 0;                                  // strutrue elements for main thread
        thread_array_merge[ 0 ].high_array_index_m = individual_thread_array_length -1;
        thread_array_merge[ 0 ].thread_array = numbers;
        thread_array_merge[ 0 ].thread_number_merge = 1;
        mergesort_thread( &thread_array_merge[ 0 ] );                                   // calling function for main

        for ( size_t i = 1; i < thread_count; i++ )
        {
            ret = pthread_join( threads[ i ], NULL );                                   // joining p_threads
            if ( ret )
            {
                printf( "ERROR: pthread_join: %d\n",ret );
                exit( -1 );
            }
        }

        FJ_Merge *tskm = &thread_array_merge[0];                                        // merge sub arrays one by one such that we get final merge array
		for (i = 1; i < thread_count; i++) {
			FJ_Merge *tsk = &thread_array_merge[i];
			merge(tsk->thread_array, tskm->low_array_index_m, tsk->low_array_index_m - 1, tsk->high_array_index_m);
		}

        clock_gettime(CLOCK_MONOTONIC,&end_time);

        if (barr_used==1 && choice_barrier==pthread_bar)            //destory pthread barrier and pthread mutex if passed as cmd line input
        {
		    pthread_barrier_destroy(&barrier_pthread_bar);
        }
        if (lock_used==1 && choice_lock==lockPthread)
        {
		    pthread_mutex_destroy(&lock_pthread_mutex);
        }
		pthread_barrier_destroy(&bar);              //destory pthread barrier
        
        FILE *ptri;
        ptri = fopen( sorted_file, "w" );         // the file is opened in write mode which is passes to -o option
        for (it = 0; it < idx; it++) 
        {
            fprintf( ptri ,"%u\n", numbers[ it ] ); //the unsorted file is sorted and elements are stored in sorted file
        }
        if ( ptri ) 
            fclose ( ptri );                        /* close file */
        free( numbers );                            //free numbers pointer
    }
    else if ( quick_flag == 1 )    
    {
        quick_flag = 0;
        int *number;                             // the unsorted file is read
        number = file();
        quickSort( number, 0, idx - 1 );         // quicksort is done on unsorted list
        FILE *ptr;
        ptr = fopen( sorted_file, "w" );         // the file is opened in write mode which is passes to -o option
        for ( it = 0; it < idx; it++ ) 
        {
            fprintf( ptr, "%u\n", number[ it ] ); //the unsorted file is sorted and elements are stored in sorted file
        }
        if ( ptr ) 
            fclose ( ptr );                        /* close file */
        free( number );                            //free number
    }
    else if ( bucket_flag == 1 )
    { 
        map <int,int> single_bucket;                // assigning map single_bucket to buckets
        for (int i=0; i<thread_count; i++)
        {
            bucket.push_back(single_bucket);
        }
        int ret;                                    
        bucket_flag = 0;                            // make bucket flag as zero
        int *numbers_bucket;                        // the unsorted file is read
        numbers_bucket = file();                    // reading file into array
        LK_Bucket thread_array_bucket[thread_count];// initializing structure for bucket sort for different threads
        int divider;
        divider  = get_range( numbers_bucket, idx, thread_count ); // divider calculated for filtering array elements
        int low = 0;
        int individual_thread_array_length;         // array length which each individual thread will take off
        individual_thread_array_length = idx/thread_count;  //
        int index,i;

        for ( int i = 0; i < thread_count; i++, low += individual_thread_array_length )
        {
            args[ i ] = i + 1;                                  // thread number
            index = i*individual_thread_array_length;           // to calculate array length

            if ( i == ( thread_count - 1 ) )                    // fill the structure with array length, divider, array for bucket, thread number, low index and high index
            {
                thread_array_bucket[ i ].array_length = idx - index;
                thread_array_bucket[ i ].bucket_divider = divider;
                thread_array_bucket[ i ].thread_array_bucket = numbers_bucket;
                thread_array_bucket[ i ].thread_number_bucket = args[ i ];
                thread_array_bucket[ i ].low_array_index_b = low;
                thread_array_bucket[ i ].high_array_index_b = idx-1;
            }

            else                                                // fill the structure with array length, divider, array for bucket, thread number, low index and high index
            {
                thread_array_bucket[ i ].array_length = individual_thread_array_length;
                thread_array_bucket[ i ].bucket_divider = divider;
                thread_array_bucket[ i ].thread_array_bucket = numbers_bucket;
                thread_array_bucket[ i ].thread_number_bucket = args[ i ];
                thread_array_bucket[ i ].low_array_index_b = low;
                thread_array_bucket[ i ].high_array_index_b = low + individual_thread_array_length - 1;
            }
           
            ret = pthread_create( &threads[ i ], NULL, &bucket_sort_thread, &thread_array_bucket[ i ]); // create p_threads
            if ( ret )
            {
                printf( "ERROR: pthread_create: %d\n", ret);
                exit( -1 );
            }
        }

        for ( size_t i = 0; i < thread_count; i++ )
        {
            ret = pthread_join( threads[ i ], NULL );           // joining all threads after their parallel operation is done
            if ( ret )
            {
                printf( "ERROR: pthread_join: %d\n",ret );
                exit( -1 );
            }
        }

		int bucket_number = (int)bucket.size();     
        int final_array_count=0;
                                                    // iterating from start to end of all buckets and add it to final array
		for (i = 0; i < bucket_number; i++) {
			for (auto iterator = bucket[i].begin(); iterator != bucket[i].end(); ++iterator)
			{
				numbers_bucket[final_array_count] = iterator->first;
				final_array_count++;
			}
		}
        clock_gettime(CLOCK_MONOTONIC,&end_time);

        if (barr_used==1 && choice_barrier==pthread_bar)    //destory pthread barrier and pthread mutex if passed as cmd line input
        {
		    pthread_barrier_destroy(&barrier_pthread_bar);
        }
        if (lock_used==1 && choice_lock==lockPthread)
        {
		    pthread_mutex_destroy(&lock_pthread_mutex);
        }
        pthread_barrier_destroy(&bar);            // destroy barrier
        FILE *ptri;
        ptri = fopen( sorted_file, "w" );         // the file is opened in write mode which is passes to -o option
        for (it = 0; it < idx; it++) 
        {   
            fprintf( ptri ,"%u\n", numbers_bucket[ it ] ); //the unsorted file is sorted and elements are stored in sorted file
        }
     
        if ( ptri ) 
            fclose ( ptri );                        /* close file */
        free( numbers_bucket );                            //free numbers pointer
    }
    unsigned long long elapsed_ns;
	elapsed_ns = (end_time.tv_sec-start.tv_sec)*1000000000 + (end_time.tv_nsec-start.tv_nsec);
	printf("Elapsed (ns): %llu\n",elapsed_ns);
	double elapsed_s = ((double)elapsed_ns)/1000000000.0;
	printf("Elapsed (s): %lf\n",elapsed_s);

    return 0;
}