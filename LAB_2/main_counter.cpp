
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
#include <cmath>
#include <sstream>
#include <thread>   
#include <vector>
#include <iterator> 
#include <algorithm>
#include <set> 
#include <atomic>      
       
using namespace std;

atomic<bool> lock_stream = ATOMIC_FLAG_INIT;
atomic<int> next_num ;
atomic<int> now_serving ;

int count_value=0;
char *non_option_argument;  /*unsorted file.txt which will be passed as command line argument*/
char *out_file;          /*file to which the sorted elements from the unsorted array will be put*/

int thread_count = 0;       /*count of number of threads used*/
int iterations;             /* the number of iterations for counter to increment by */

int lock_used = 0;          /* both lock and barrier are used*/
int barr_used = 0;
pthread_mutex_t lock_pthread_mutex;  /* defining mutex lock*/
pthread_barrier_t barrier_pthread_bar;
pthread_barrier_t bar;               /* defining barrier */
struct timespec start, end_time;    /*calculate time at start and end of thread*/
size_t it = 0;                      /*  variable iterator  */

typedef enum lock_choice            /* selecting the type of lock */
{
    mcs,
    tas,
    ttas,
    lockPthread,
    ticket
}lock_choice_number;
lock_choice_number choice_lock;
typedef enum unlock_choice          /* unlock choice */
{
    unlockMCS,
    unlockTAS,
    unlockTTAS,
    unlockPthread,
    unlockTICKET
}unlock_choice_number;
unlock_choice_number choice_unlock;
typedef enum barrier_choice         /* barrier choice */
{
    sense,
    pthread_bar
}barrier_choice_number;
barrier_choice_number choice_barrier;
void ttas_lock( )                   /* ttas lock if a lock is held or lock is tried to be acquired and it fails it will leave */
{
    do 
    {
        while ( lock_stream )  continue;
    } while ( lock_stream.exchange( true )); // actual atomic locking
    return;
}

void ttas_unlock( )                 /* changing the flag to false */
{
    lock_stream.store( false );
}

void tas_lock( )                    /* wait until the time you are successful in getting lock */
{
	bool expected, new_value;
    do
    {
        new_value = true;
        expected = false;

    }while(!lock_stream.compare_exchange_strong(expected,new_value));
}

void tas_unlock( )                  /* changing the flag to false */
{
    lock_stream.store( false );
}

void ticket_lock( )                 /* my number will check where i am in the line in getting the lock */
{
    int my_num = next_num.fetch_add(1, memory_order_seq_cst );
    while(now_serving.load( memory_order_seq_cst)!=my_num ){}
}

void ticket_unlock( )               /* increment the now serving count */
{
    now_serving.fetch_add( 1, memory_order_seq_cst );
}

class Node                          
{
public:                             /* atomic next and wait */
	atomic<Node*> next;
	atomic<bool> wait;
};

atomic<Node*> tail {NULL} ;         /* atomic tail */


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
    int N ;
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

	int cnt_cpy = variables.cnt.fetch_add( 1, memory_order_seq_cst );
	if ( cnt_cpy == variables.N -1 )                                       /* if count copy is equal to last thread sense state will change*/
    {
		variables.cnt.store( 0, memory_order_relaxed );
		variables.sense.store( my_sense, memory_order_seq_cst );
	} 
    else 
    {
		while (variables.sense.load( memory_order_seq_cst ) != my_sense); /* read the value of sense */
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

/* thread main function where different threads will execute sorting operation parallely */
void* thread_main_func( void *args )
{

    size_t thread_number = *((size_t*)args);
    int iteration_total = thread_count * iterations;     /* total iterations for all threads */
    pthread_barrier_wait( &bar );                        /* barrier wait */
    if ( thread_number == 1 )
    {
        if (( lock_used == 1 ) && ( choice_lock==lockPthread )) /* if lock is pthread initialize new pthread lock*/
            pthread_mutex_init(&lock_pthread_mutex, NULL);
        clock_gettime( CLOCK_MONOTONIC, &start ); /* get clock time at the start */
    }
    pthread_barrier_wait(&bar);                   /* barrier wait */  
    printf( "\nThread %zu reporting for duty\n",thread_number );

    for (int i = 0; i < iteration_total; i++ )
    {
       if( (i % thread_count) ==  thread_number-1 ) // if mod of iteration is equal to thread number, this way all threads will have same number of iterations
       {
            if (lock_used==1)
            {
                switch( choice_lock )           /* using the type of lock passed as command line input */
                {
                    case( ticket ):
                    {
                        ticket_lock();
                        count_value++;
                        ticket_unlock();
                        continue;
                    }
                    case( tas ):
                    {
                        tas_lock();
                        count_value++;
                        tas_unlock();
                        continue;
                    }
                    case( ttas ):
                    {
                        ttas_lock();
                        count_value++;
                        ttas_unlock();
                        continue;
                    }
                    case( lockPthread ):
                    {
                        pthread_lock();
                        count_value++;
                        pthread_unlock();
                        continue;
                    }
                    case( mcs ):
                    {
                        Node *mynode = new Node;
                        lockMCS.acquire(mynode);
                        count_value++;
                        lockMCS.release(mynode);
                        continue;
                    }
                
                }
            }
            else if( lock_used==0 && barr_used==1 )     
            {
                count_value++;
            }

        }
        
        if ( barr_used == 1 )               /* using the type of barrier passed as command line input */
        {
            switch( choice_barrier )
            {
                case( sense ):
                {
                    sense_barrier( );
                    continue;
                }
                case( pthread_bar ): 
                {
                    pthread_barrier();
                    continue;
                }

            }
        }

        
    }
    
    pthread_barrier_wait( &bar );                      // barrier wait for clock timing 
    if ( thread_number == 1 )
        clock_gettime( CLOCK_MONOTONIC, &end_time );   /* end clock timer */
    return 0;    
}

int main( int argc, char **argv )
{
    int character;                      // character which is passed from command line
    int option_index = 0;               // the index of options
    char *option_to_argument_bar;       // it is the algorithm option, that is sense or pthread barrier
    char *option_to_argument_lock;       // it is the algorithm option, for lock
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
                                                { "iterations", 1, 0, 'i' },
                                                { "lock", 1, 0, 'l' },
                                                { "name", 0, 0, 'n' },
						                        { "thread", 1, 0, 't' },
                                                { 0, 0, 0, 0 }
                                            };
    /*checks if all characters or aguments passed in the command line have been read*/
    while (( character = getopt_long( argc, argv, "b:no:t:l:i:", long_options, &option_index )) != -1 )
    {
        switch( character )
        {
            case 'b':                   // checks if alg is passed
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
            case 'i':                   // checks if alg is passed
            {
                iterations = atoi(optarg); //checks -t argument and adds thread count to thread_count
                printf( "The number of iterations for each thread is %s",optarg );                
                break;
            }
            case 'l':                   // checks if alg is passed
            {
                lock_used = 1;
                printf( "\n--lock-> option = %s\n", optarg ); //prints the algorithm option which is tas,ttas, mcs, ticket or pthread
                option_to_argument_lock = optarg;             //optarg maintains the argument
                                                             //strcmp the value of option passed to --lock is msc
                if ( strcmp( option_to_argument_lock, lock_msc ) == 0 )
                {
                    choice_lock = mcs;
                    choice_unlock = unlockMCS;
                }  
                                                             //strcmp the value of option passed to --lock is pthread
                else if ( strcmp( option_to_argument_lock, lock_pthread ) == 0 ) 
                {
                    choice_lock = lockPthread;
                    choice_unlock = unlockPthread;
                }

                else if ( strcmp( option_to_argument_lock, lock_tas ) == 0 ) //strcmp the value of option passed to --lock is tas
                {
                    choice_lock = tas;
                    choice_unlock = unlockTAS;
                }

                else if ( strcmp( option_to_argument_lock, lock_ttas ) == 0 )   //strcmp the value of option passed to --lock is ttas
                {
                    choice_lock = ttas;
                    choice_unlock = unlockTTAS;
                }

                else if ( strcmp( option_to_argument_lock, lock_ticket ) == 0 )  //strcmp the value of option passed to --lock is ticket
                {
                    choice_lock = ticket;
                    choice_unlock = unlockTICKET;
                }

                break;
            }

            case 'o':                     // checks if -o argument is passed and stores the file to be sorted in out_file
            {
                out_file = optarg;
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
                else if ( optopt == 'b')
                    printf( "Option %c requires an argument pthread or sense", optopt );  
                else if ( optopt == 'l')
                    printf( "Option %c requires an argument pthread mcs tas ttas ticket", optopt );
                else if ( optopt == 't' )
                    thread_count = 5;           /* if no option specified take thread as 5 */
                else if ( optopt == 'i' )   
                    iterations = 5; 
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

    if ( thread_count <= 0 )
        thread_count = 1;
    else if ( thread_count > 150 )
    {
        printf( "\nERROR: Too many threads\n");
        exit( -1 );
    }
    if ( iterations <= 0 )                      //if iterations are less than equal to zero, make it one
        iterations = 1;
        
    if (barr_used==1 && lock_used==1)           // Lock and barrier cannot be used together
    {
        printf("Invalid args error: Cannot use lock and barrier together");
        exit(1);
    }
    if (barr_used==1 && choice_barrier==pthread_bar) /* initialize the barrier pthread*/ 
    {
        pthread_barrier_init( &barrier_pthread_bar, NULL, thread_count ); 
    }
    if (barr_used==1 && choice_barrier==sense) /* initialize the barrier pthread*/ 
    {
        variables.N = thread_count;
    }

    size_t args[thread_count] = {0};
    size_t thread_one_number = 1;
    pthread_t threads[thread_count];
    pthread_barrier_init( &bar, NULL, thread_count ); /* initialize the barrier pthread*/

    size_t i;                                  // iterator
    int ret;                                   // pthread_create return value

    for ( i = 1; i < thread_count; i++ )
    {
        args[ i ] = i + 1;
        printf( " Creating thread %zu\n", args[ i ] );


        ret = pthread_create( &threads[ i ], NULL, &thread_main_func, &args[ i ]); // create p_threads 
        if ( ret )
        {
            printf( "ERROR: pthread_create: %d\n", ret);
            exit( -1 );
        }
    }
    thread_main_func( &thread_one_number );                                  // calling function for main

    for ( size_t i = 1; i < thread_count; i++ )
    {
        ret = pthread_join( threads[ i ], NULL );                                   // joining p_threads
        if ( ret )
        {
            printf( "ERROR: pthread_join: %d\n",ret );
            exit( -1 );
        }
        printf( "Joined thread %zu\n", i + 1 );
    }

    clock_gettime(CLOCK_MONOTONIC,&end_time);
    pthread_barrier_destroy(&bar);
    if (barr_used==1 && choice_barrier==pthread_bar)   //destory pthread barrier and pthread mutex if passed as cmd line input
    {
        pthread_barrier_destroy(&barrier_pthread_bar);
    }
    if (lock_used==1 && choice_lock==lockPthread)
    {
        pthread_mutex_destroy(&lock_pthread_mutex);
    }
    FILE *ptri;
    ptri = fopen( out_file, "w" );         // the file is opened in write mode which is passes to -o option
    printf("THe final count value=%d",count_value);

    fprintf( ptri ,"%d\n", count_value ); //the unsorted file is sorted and elements are stored in sorted file

    if ( ptri ) 
        fclose ( ptri );                        /* close file */

    unsigned long long elapsed_ns;
	elapsed_ns = (end_time.tv_sec-start.tv_sec)*1000000000 + (end_time.tv_nsec-start.tv_nsec);
	printf("Elapsed (ns): %llu\n",elapsed_ns);
	double elapsed_s = ((double)elapsed_ns)/1000000000.0;
	printf("Elapsed (s): %lf\n",elapsed_s);

    return 0;
}

