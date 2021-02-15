/* reference: https://www.geeksforgeeks.org/quick-sort/ */
/* reference: https://www.geeksforgeeks.org/merge-sort/ */
/* reference: https://www.youtube.com/watch?v=TzeBrDU-JaY */
/* reference: https://www.youtube.com/watch?v=COk73cpQbFQ */
/* reference: https://www.youtube.com/watch?v=3Bbm3Prd5Fo */
/* reference: https://www.youtube.com/watch?v=0nlPxaC2lTw */
/* reference: https://linux.die.net/man/3/getopt */
/* https://stackoverflow.com/questions/31057175/reading-text-file-of-unknown-size */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <stdio.h> 
#include <omp.h>

#define LMAX 255

char *non_option_argument;  /*unsorted file.txt which will be passed as command line argument*/
char *sorted_file;          /*file to which the sorted elements from the unsorted array will be put*/
size_t idx = 0;             /* index of array of no of lines  */
int merge_flag=0;           /*if algo selected for sorting the unsorted file is merge, merge flag will be 1*/
int quick_flag=0;           /*if algo selected for sorting the unsorted file is quick, merge flag will be 1*/
size_t it = 0;              /*  variable iterator  */

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

        /* With omp parallel sections each section executes in parallel by a different thread */
        #pragma omp parallel sections
		{
            /* one half of the array */
			#pragma omp section     
			{
                quickSort( arr, low, partition_index - 1 ); 
			}
            /* other half of the array */
			#pragma omp section
			{
                quickSort( arr, partition_index + 1, high ); 
			}
		}
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
        /* With omp parallel sections each section executes in parallel by a different thread */
		#pragma omp parallel sections
		{
            /* left half of the array */
			#pragma omp section
			{
				mergeSort( arr, left, mid ); 
			}
            /* right half of the array */
			#pragma omp section
			{
				mergeSort( arr, mid + 1, right ); 
			}
		}
        merge( arr, left, mid, right ); 
    } 
} 

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
        if (!( array = calloc ( LMAX, sizeof * array ) ) ) 
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
                char **tmp = realloc ( array, lmax * 2 * sizeof * array );
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



        for ( it = 0; it < idx; it++ )   
        {
            numbers[ it ] = strtol( array[ it ], &endptr[ it ], 0 ); 
        }

        printf ("\n");

        for ( it = 0; it < idx; it++ )        /* free array memory    */
            free ( array[ it ] );
        
        free ( array );
        
        return numbers;
}

int main( int argc, char **argv )
{
    int character;                      // character which is passed from command line
    int option_index = 0;               // the index of options

    /*maintains the long option list of arguments passes in the command line*/
    static struct option long_options[] =   {
                                                { "name", 0, 0, 'n' },
                                                { 0, 0, 0, 0 }
                                            };
    /*checks if all characters or aguments passed in the command line have been read*/
    while (( character = getopt_long( argc, argv, "a:no:", long_options, &option_index )) != -1 )
    {
        switch( character )
        {
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
            case '?':                   // checks if no argument is passed to option which requires an argument
            {
                if ( optopt == 'o' )
                    printf( "Option %c requires an argument which is the file that is the sorted file", optopt );
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

    int *numbers;                             // the unsorted file is read
    numbers = file();
    mergeSort( numbers, 0, idx - 1 );         // mergesort is done on unsorted list
    FILE *ptri;
    ptri = fopen( sorted_file, "w" );         // the file is opened in write mode which is passes to -o option
    for (it = 0; it < idx; it++) 
    {
        fprintf( ptri ,"%u\n", numbers[ it ] ); //the unsorted file is sorted and elements are stored in sorted file
    }
    if ( ptri ) 
        fclose ( ptri );                        /* close file */
    free( numbers );                            //free numbers pointer
    return 0;
}






