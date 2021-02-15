

/* reference: https://linux.die.net/man/3/getopt */
/* reference: https://www.geeksforgeeks.org/map-insert-in-c-stl/ */
/* https://www.geeksforgeeks.org/binary-search-tree-set-1-search-and-insertion/ */
/* https://github.com/sorabhgandhi01/Concurrent-Programming/tree/master/final_project */
/* https://stackoverflow.com/questions/9181146/freeing-memory-of-a-binary-tree-c */
/* https://www.geeksforgeeks.org/print-all-nodes-between-two-given-levels-in-binary-tree/ */
/* https://www.tutorialspoint.com/data_structures_algorithms/binary_search_tree.htm */
/* reference:https://gist.github.com/eknight7/7d717227c6abc414ceb4 */

/*C LIBRARY HEADER FILES*/
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <math.h>
#include <bits/stdc++.h> 

/*C++ LIBRARY HEADER FILES*/
#include <iostream>
#include <cmath>
#include <vector>
#include <algorithm>
#include <set> 
#include <iterator> 

using namespace std;

#define LMAX 255                /*to read the input file */

char *non_option_argument;  /*unsorted file.txt which will be passed as command line argument*/
char *put_file;            /*file from which the nodes keys and value will be put in bst*/
char *get_file;            /*file from which the nodes keys which needs to be searched for will be obtained*/
char *range_file;          /*file from which the nodes keys start and end will be obtained for finding nodes*/
char *deleteid_file;       /*file from which the nodes to be deleted will be obtained */

int idx = 0;                /* index of array of no of lines  */
int rw_lock_flag = 0;         /*if algo selected for sorting the unsorted file is merge, merge flag will be 1*/
int fg_lock_flag = 0;         /*if algo selected for sorting the unsorted file is quick, merge flag will be 1*/
int thread_count = 0;       /*count of number of threads used*/

struct timespec start, end_time;    /*calculate time at start and end of thread*/

pthread_rwlock_t lock_rw_function;  /*initialize the rd wr lock*/

pthread_mutex_t lock_fg_function; /*initialize the mutex lock*/

pthread_mutex_t lock_fine;

// struct node
typedef struct Node_rw 
{
    pthread_rwlock_t lock;
    int key;
    int value;
    struct Node_rw *left;
    struct Node_rw *right;
    
}Node_rw;

// struct to find range querries
typedef struct rw_range 
{ 
   int key_begin;
   int key_last;
   Node_rw *node;
}size_rw_range_info; 
vector <size_rw_range_info> range_info_rw_function[2];

// struct Tree
typedef struct Tree_rw 
{
    pthread_rwlock_t lock;
    Node_rw *root  ;
}Tree_rw;

Tree_rw *tree_rw = NULL;

size_t it = 0;              /*  variable iterator  */

// all thread based information
typedef struct data_thread_iterations
{
	int iteration_number;
	int iteration_length;
	int iteration_item;
}data_thread_iterations;

// struct node
typedef struct node 
{ 
    pthread_mutex_t lock;
    int key; 
    int value;
    struct node *left, *right; 
}Node;

// Root of binary search tree
typedef struct Tree 
{
    pthread_mutex_t lock;
    Node *root  ;
}Tree;

Tree *tree;

// struct that range querries 
typedef struct fg_range 
{ 
   int key_begin;
   int key_last;
   Node *node;
}size_fg_range_info; 
vector <size_fg_range_info> range_info_fg_function[2];

// function that creates nodes with key  and value
Node* newNode(int item, int val) 
{ 
    Node* temp 
        = (Node*)malloc(sizeof(Node)); 
    temp->key = item; 
    temp->value = val; 
    temp->left = temp->right = NULL; 
    return temp; 
} 

// function that inserts nodes with key and value 
void insert_helper_rw(Node_rw *root, int key, int value, Node_rw *parent) 
{
    if (root) 
    {
        pthread_rwlock_wrlock(&root->lock);
        if (parent) pthread_rwlock_unlock(&parent->lock);
        if (root->key > key) 
        {
            if (root->left) 
            {
                insert_helper_rw(root->left, key, value, root);
            } 
            else 
            {
                Node_rw *n = (Node_rw*)malloc(sizeof(Node_rw));
                pthread_rwlock_init(&n->lock, NULL);
                n->key = key;
                n->value = value;
                n->left = NULL;
                n->right = NULL;
                root->left = n;
                pthread_rwlock_unlock(&root->lock);
            }
        } 
        else if (root->key < key)
        {
            if (root->right) 
            {
                insert_helper_rw(root->right,key, value, root);
            } 
            else 
            {
                Node_rw *n = (Node_rw*)malloc(sizeof(Node_rw));
                pthread_rwlock_init(&n->lock, NULL);
                n->value = value;
                n->key = key;
                n->left = NULL;
                n->right = NULL;
                root->right = n;
                pthread_rwlock_unlock(&root->lock);
            }
        } 
        else 
        {
            // do nothing, value exists
            root->value = value;
            pthread_rwlock_unlock(&root->lock);
            return;
        }
    }
}

// function that inserts nodes with key and value
void insert_rw(Tree_rw *tree, int key, int value) 
{

    // Case 1: tree is empty
    pthread_rwlock_wrlock(&tree->lock);

    if (!tree->root) 
    {
        Node_rw *n = (Node_rw*)malloc(sizeof(Node_rw));
        pthread_rwlock_init(&n->lock, NULL);
        n->value = value;
        n->key = key;
        n->left = NULL;
        n->right = NULL;
        tree->root = n;
        pthread_rwlock_unlock(&tree->lock);
        // unlock(tree->lock);
        return;
    }

    // Case 2: Tree is non-empty
    pthread_rwlock_unlock(&tree->lock);
    insert_helper_rw(tree->root, key, value, NULL);
}

// function that gets nodes with key 
Node_rw * get_helper_rw(Node_rw *root, int key, Node_rw *parent)
{
    if (root) 
    {
        pthread_rwlock_rdlock(&root->lock);
        if (parent) pthread_rwlock_unlock(&parent->lock);
        if (root->key > key) 
        {
            if (root->left) 
            {
                get_helper_rw(root->left, key, root);
            } 
            else 
            {
                pthread_rwlock_unlock(&root->lock);
                return NULL;
            }
        } 
        else if (root->key < key)
        {
            if (root->right) 
            {
                get_helper_rw(root->right,key, root);
            } 
            else 
            {
               pthread_rwlock_unlock(&root->lock);
               return NULL;
            }
        } 
        else 
        {
            // do nothing, value exists
            pthread_rwlock_unlock(&root->lock);
            return root;
        }
    }
}

// function that gets nodes with key 
Node_rw * get_rw(Tree_rw *tree, int key) 
{
    // Case 1: tree is empty
    pthread_rwlock_rdlock(&tree->lock);
    if (!tree->root) 
    {
        pthread_rwlock_unlock(&tree->lock);
        return NULL;
    }

    // Case 2: Tree is non-empty
    pthread_rwlock_unlock(&tree->lock);
    get_helper_rw(tree->root, key, NULL);
}

// function that gets nodes between range
void nodes_beween_range_rw(Node_rw *root, int key_begin, int key_last, int thread_number)
{
	if (root == NULL)
	{
		pthread_rwlock_rdlock(&lock_rw_function);
		if (tree_rw->root == NULL)
		{
			pthread_rwlock_unlock(&lock_rw_function);
			return;
		}
		pthread_rwlock_rdlock(&tree_rw->root->lock);
		root = tree_rw->root;
		pthread_rwlock_unlock(&lock_rw_function);
	}

	if (key_begin < root->key)
	{
		if (root->left != NULL)
		{
			pthread_rwlock_rdlock(&root->left->lock);
			pthread_rwlock_unlock(&root->lock);
			nodes_beween_range_rw(root->left, key_begin, key_last, thread_number);
			pthread_rwlock_rdlock(&root->lock);
		} 
		
	}

	if ((key_last >= root->key) && (key_begin <= root->key))
	{
		range_info_rw_function[thread_number].push_back({key_begin, key_last, root});
	}
	
	if (key_last > root->key)
    {
        if (root->right != NULL)
        {
        	pthread_rwlock_rdlock(&root->right->lock);
        	pthread_rwlock_unlock(&root->lock);
        	nodes_beween_range_rw(root->right, key_begin, key_last, thread_number);
        	pthread_rwlock_rdlock(&root->lock);
        } 
    }
    pthread_rwlock_unlock(&root->lock);

}

void info_range_between_start_and_end_rw(Tree_rw *tree, int key_begin, int key_last, int thread_number)
{
	if (tree->root == NULL) 
    {
		return;
	}
	Node_rw *start_node = get_rw(tree, key_begin);
	Node_rw *end_node = get_rw(tree, key_last);

	if (start_node == NULL) 
    {
		printf("Error: Key starting with %d does not exist\n", key_begin);
	} 
    else if (end_node == NULL) 
    {
		printf("Error: Key ending with %d does not exist\n", key_last);
	} 
    else 
    {
		pthread_rwlock_rdlock(&lock_rw_function);
		pthread_rwlock_rdlock(&tree->root->lock);
		pthread_rwlock_unlock(&lock_rw_function);
		nodes_beween_range_rw(tree->root, key_begin, key_last, thread_number);
    }
}

Node_rw* getMin_rw(Node_rw *root, Node_rw *parent) 
{

    if (root) 
    {
        pthread_rwlock_rdlock(&root->lock);
        if (parent) 
            pthread_rwlock_unlock(&parent->lock);
        
        if (root->left) 
        {
            return getMin_rw(root->left, root);
        } 
        else 
        {
            pthread_rwlock_unlock(&root->lock);
            return root;
        }
    } 
    else 
    {
        // Shouldn't happen
        
        return NULL;
    }
}

// function that deletes nodes 
Node_rw *delete_helper_rw(Node_rw *root, int key, int value, Node_rw *parent) 
{
    if (root) 
    {
        pthread_rwlock_rdlock(&root->lock);
        if (root->key == key) 
        {
            // Case 1: root has no children
            if (!root->left && !root->right) 
            {
                Node_rw *tmp = root;
                root = NULL;
                // Modify parent after this node is fixed
                if (parent) 
                    pthread_rwlock_unlock(&parent->lock);

                pthread_rwlock_unlock(&tmp->lock);
                free(tmp);
                return root;
            }
            // Case 2: root has 1 child
            if (!root->left && root->right) 
            {
                Node_rw *tmp = root;
                root = root->right;
                // Modify parent after this node is fixed
                if (parent) 
                    pthread_rwlock_unlock(&parent->lock);

                pthread_rwlock_unlock(&tmp->lock);
                free(tmp);
                return root;
            }
            if (!root->right && root->left) 
            {
                Node_rw *tmp = root;
                root = root->left;
                // Modify parent after this node is fixed
                
                if (parent) 
                    pthread_rwlock_unlock(&parent->lock);
                pthread_rwlock_unlock(&tmp->lock);
                free(tmp);
                return root;
            }

            // Case 3: root has 2 children
            // Replace root with highest value smaller than root
            Node_rw *minRight = getMin_rw(root->right, NULL);
            root->key = minRight->key;
            // Now delete minRight node; set its parent's left child to NULL or
            // to minRight node's right child
            // We pass NULL as parent as we unlock root after deletion is complete
            root->right = delete_helper_rw(root->right, minRight->key, minRight->value, NULL);
            // Unlock after this sub-tree root is fixed to avoid violating
            // BST invariants when other threads insert simultaneously
            
            pthread_rwlock_unlock(&root->lock);
        }
        else if (root->key > key) 
        {
            // We can releas parent as we modify root and root->left now
            if (parent) 
                pthread_rwlock_unlock(&parent->lock);
            
            // Delete value node from left sub-tree
            root->left = delete_helper_rw(root->left, key, value, root);
        } else 
        {
            // We can releas parent as we modify root and root->right now
            if (parent) pthread_rwlock_unlock(&parent->lock);
            // Delete value node from right sub-tree
            root->right = delete_helper_rw(root->right, key, value, root);
        }
    } 
    else 
    {
        // Nothing to delete
        return root;
    }
}

// function that deletes nodes
void deleteid_rw(Tree_rw *tree, int key, int value) {

    // Case 1: tree is empty
    pthread_rwlock_rdlock(&tree->lock);
    if (!tree->root)
    {
        pthread_rwlock_unlock(&tree->lock);
        return;
    }

    // Case 2: Tree is non-empty
    pthread_rwlock_unlock(&tree->lock);

    tree->root = delete_helper_rw(tree->root, key, value, NULL);
}

// function that prints nodes with key and value 
void print_nodes_inorder_rw(Node_rw* root) 
{ 
    if (root != NULL) { 
        print_nodes_inorder_rw(root->left); 
        printf("key_rw:%d value_rw:%d\n", root->key, root->value); 
        print_nodes_inorder_rw(root->right); 
    } 
} 

// function that prints nodes with key and value
void print_nodes_inorder(Node* root) 
{ 
    if (root != NULL) { 
        print_nodes_inorder(root->left); 
        printf("key:%d value:%d\n", root->key, root->value); 
        print_nodes_inorder(root->right); 
    } 
} 


// function that inserts nodes with key value
void insert_helper(Node *root, int key, int value, Node *parent) 
{
    if (root) 
    {
        pthread_mutex_lock(&root->lock);
        if (parent) 
        {
            pthread_mutex_unlock(&parent->lock);
        }
        
        if (root->key > key) 
        {
            if (root->left) 
            {
                insert_helper(root->left, key, value, root);
            } 
            else 
            {
                Node *n = (Node*)malloc(sizeof(Node));
                pthread_mutex_init(&n->lock, NULL);
                n->key = key;
                n->value = value;
                n->left = NULL;
                n->right = NULL;
                root->left = n;
                pthread_mutex_unlock(&root->lock);
            }
        } 
        else if (root->key < key)
        {
            if (root->right) 
            {
                insert_helper(root->right,key, value, root);
            } 
            else 
            {
                Node *n = (Node*)malloc(sizeof(Node));
                pthread_mutex_init(&n->lock, NULL);
                n->value = value;
                n->key = key;
                n->left = NULL;
                n->right = NULL;
                root->right = n;
               pthread_mutex_unlock(&root->lock);
            }
        } 
        else 
        {
            root->value = value;
            pthread_mutex_unlock(&root->lock);
            return;
        }
    }
}

// function that inserts nodes with key value
void insert(Tree *tree, int key, int value) 
{
    // Case 1: tree is empty
    
    pthread_mutex_lock(&tree->lock);
            

    if (!tree->root) 
    {
        Node *n = (Node*)malloc(sizeof(Node));
        pthread_mutex_init(&n->lock, NULL);
        n->value = value;
        n->key = key;
        n->left = NULL;
        n->right = NULL;
        tree->root = n;
        pthread_mutex_unlock(&tree->lock);
        return;
    }

    // Case 2: Tree is non-empty
    pthread_mutex_unlock(&tree->lock);
    insert_helper(tree->root, key, value, NULL);
}

// function that gets nodes with key value 
Node* get_helper ( Node* root, int key, Node* parent ) 
{
    if (root) 
    {
        pthread_mutex_lock(&root->lock);
        if (parent)
        {
            pthread_mutex_unlock(&parent->lock);
        }
        if (root->key > key) 
        {
            if (root->left) 
            {
                get_helper(root->left, key, root);
            } 
            else 
            {
                pthread_mutex_unlock(&root->lock);  
                return NULL;
            }
        } 
        else if (root->key < key)
        {
            if (root->right) 
            {
                get_helper(root->right,key, root);
            } 
            else 
            {
                pthread_mutex_unlock(&root->lock);
                return NULL;
            }
        } 
        else 
        {
            // do nothing, value exists
            pthread_mutex_unlock(&root->lock);
            return root;
        }
    }
}


// function that gets nodes with key value
Node * get(Tree *tree, int key) 
{
    // Case 1: tree is empty
    pthread_mutex_lock(&tree->lock);

    if (!tree->root) {

        pthread_mutex_unlock(&tree->lock);
        printf("value is null");
        return NULL;
    }

    // Case 2: Tree is non-empty
    pthread_mutex_unlock(&tree->lock);
    get_helper(tree->root, key, NULL);
}


// function that gets in range nodes
void nodes_beween_range(Node *root, int key_begin, int key_last, int thread_number)
{
	if (root == NULL)
	{
		pthread_mutex_lock(&lock_fg_function);
		if (tree->root == NULL)
		{
			pthread_mutex_unlock(&lock_fg_function);
			return;
		}

		pthread_mutex_lock(&tree->root->lock);
		root = tree->root;
		pthread_mutex_unlock(&lock_fg_function);
	}

	if (key_begin < root->key)
	{
		if (root->left != NULL)
		{
			pthread_mutex_lock(&root->left->lock);
		    pthread_mutex_unlock(&root->lock);
			nodes_beween_range(root->left, key_begin, key_last, thread_number);
			pthread_mutex_lock(&root->lock);
		} 
		
	}

	if ((key_begin <= root->key) && (key_last >= root->key))
	{
		range_info_fg_function[thread_number].push_back({key_begin, key_last, root});
	}
	
	if (key_last > root->key)
    {
        if (root->right != NULL)
        {
        	pthread_mutex_lock(&root->right->lock);
            pthread_mutex_unlock(&root->lock);
        	nodes_beween_range(root->right, key_begin, key_last, thread_number);
        	pthread_mutex_lock(&root->lock);
        } 
    }

    pthread_mutex_unlock(&root->lock);
}

// function that gets range of nodes 
void info_fg_range_between_start_and_end(Tree *tree, int key_begin, int key_last, int thread_number)
{
	if (tree->root == NULL) 
    {
		return;
	}

	Node *start_node = get(tree, key_begin);
	Node *end_node = get(tree, key_last);

	if (start_node == NULL) 
    {
		printf("Error: Key starting with %d does not exist\n", key_begin);
	} 
    else if (end_node == NULL) 
    {
		printf("Error: Key ending with %d does not exist\n", key_last);
	} 
    else 
    {
		pthread_mutex_lock(&lock_fg_function);
		pthread_mutex_lock(&tree->root->lock);
		pthread_mutex_unlock(&lock_fg_function);
		nodes_beween_range(tree->root, key_begin, key_last, thread_number);
	}
}

// function that gets minimum node value 
Node* getMin(Node *root, Node *parent) 
{
    if (root) 
    {
        pthread_mutex_lock(&root->lock);
        if (parent) 
            pthread_mutex_unlock(&parent->lock);
        if (root->left) 
        {
            return getMin(root->left, root);
        } 
        else 
        {
            pthread_mutex_unlock(&root->lock);
            return root;
        }
    } 
    else 
    {
        // Shouldn't happen
        return NULL;
    }
}

// delete node function
Node *delete_helper(Node *root, int key, int value, Node *parent) 
{
    if (root) 
    {
        pthread_mutex_lock(&root->lock);
        if (root->key == key) 
        {
            // Case 1: root has no children
            if (!root->left && !root->right) 
            {
                Node *tmp = root;
                root = NULL;
                // Modify parent after this node is fixed
                if (parent) 
                    pthread_mutex_unlock(&parent->lock);
                
                pthread_mutex_unlock(&tmp->lock);
                free(tmp);
                return root;
            }

            // Case 2: root has 1 child
            if (!root->left && root->right) 
            {
                Node *tmp = root;
                root = root->right;
                // Modify parent after this node is fixed
                if (parent) 
                    pthread_mutex_unlock(&parent->lock);
                pthread_mutex_unlock(&tmp->lock);
                free(tmp);
                return root;
            }

            if (!root->right && root->left) 
            {
                Node *tmp = root;
                root = root->left;
                // Modify parent after this node is fixed
                if (parent) 
                    pthread_mutex_unlock(&parent->lock);
                pthread_mutex_unlock(&tmp->lock);
                free(tmp);
                return root;
            }
            // Case 3: root has 2 children
            // Replace root with highest value smaller than root
            Node *minRight = getMin(root->right, NULL);
            
            root->key = minRight->key;
            
            printf("root key %d ",root->key);
            // Now delete minRight node; set its parent's left child to NULL or
            // to minRight node's right child
            // We pass NULL as parent as we unlock root after deletion is complete
            root->right = delete_helper(root->right, minRight->key, minRight->value, NULL);
            // Unlock after this sub-tree root is fixed to avoid violating
            // BST invariants when other threads insert simultaneously
            pthread_mutex_unlock(&root->lock);
        }
        else if (root->key > key) 
        {
            // We can releas parent as we modify root and root->left now
            if (parent) 
                pthread_mutex_unlock(&parent->lock);
            // Delete value node from left sub-tree
            root->left = delete_helper(root->left, key, value, root);
        } 
        else 
        {
            // We can releas parent as we modify root and root->right now
            if (parent) 
                pthread_mutex_unlock(&parent->lock);
            // Delete value node from right sub-tree
            root->right = delete_helper(root->right, key, value, root);
        }
    } 
    else
    {
        // Nothing to delete
        return root;
    }
}

void deleteid(Tree *tree, int key, int value) 
{
    // Case 1: tree is empty
    pthread_mutex_lock(&tree->lock);
    if (!tree->root)
    {
        pthread_mutex_unlock(&tree->lock);
        return;
    }

    // Case 2: Tree is non-empty
    pthread_mutex_unlock(&tree->lock);
    tree->root = delete_helper(tree->root, key, value, NULL);
}


// free all nodes in tree
void free_all_nodes_in_bst_rw(Node_rw *root)
{
	if (root != NULL) 
    {
        free_all_nodes_in_bst_rw(root->right);
        free_all_nodes_in_bst_rw(root->left);
        pthread_rwlock_destroy(&root->lock);
        free(root);
    }
 }

// free all nodes in tree
void free_all_nodes_in_bst(Node *root)
{
	if (root != NULL) 
    {
        free_all_nodes_in_bst(root->right);
        free_all_nodes_in_bst(root->left);
        pthread_mutex_destroy(&root->lock);
        free(root);
    }
 }

// handler for put function invocation
void *handler_insert_rw(void *arg)
{
	data_thread_iterations *thread_information = (data_thread_iterations *)arg;
	int key = 0, value = 0, i = 0;

	FILE *file_ptr = fopen(put_file, "r");
	
	if (file_ptr == NULL) 
    {
		printf("ERROR: File could not be opened\n");
		exit(0);
	}

	while ((i < thread_information->iteration_item) && (!feof (file_ptr))) 
    {
		fscanf(file_ptr, "%d %d\n", &key, &value);
		i++;
	}

	for (i = 0; i < thread_information->iteration_length; i++)
	{
		fscanf(file_ptr, "%d %d\n", &key, &value);
		insert_rw(tree_rw, key, value);
	}	

	fclose(file_ptr);	

	return 0;
}

// handler for get function invocation
void *handler_get_rw(void *arg)
{
	data_thread_iterations *thread_information = (data_thread_iterations *)arg;
	Node_rw *node;
	int key = 0, i = 0;

	FILE *file_ptr = fopen(get_file, "r");
	
	if (file_ptr == NULL) 
    {
		printf("ERROR: File could not be opened\n");
		exit(0);
	}

	while ((i < thread_information->iteration_item) && (!feof (file_ptr))) 
    {
		fscanf(file_ptr, "%dn", &key);
		i++;
	}

	for (i = 0; i < thread_information->iteration_length; i++)
	{
		fscanf(file_ptr, "%d\n", &key);

		node = get_rw(tree_rw, key);
		if (node == NULL) {
			printf("Node with key %d not found\n", key);
		} else {
			printf("Node with key %d contains value %d\n", node->key, node->value);
		}	
		node = NULL;
	}

    fclose(file_ptr);	

    return 0;
}


// handler for range function invocation
void *handler_range_rw(void *arg)
{
	data_thread_iterations *thread_information = (data_thread_iterations *)arg;
    int key_begin = 0, key_last = 0, i = 0;

    FILE *file_ptr = fopen(range_file, "r");

    if (file_ptr == NULL) 
    {
		printf("ERROR: File could not be opened\n");
		exit(0);
	}

	while ((i < thread_information->iteration_item) && (!feof (file_ptr))) 
    {
		fscanf(file_ptr, "%d %d\n", &key_begin, &key_last);
		i++;
	}

	for (i = 0; i < thread_information->iteration_length; i++)
	{
		fscanf(file_ptr, "%d %d\n", &key_begin, &key_last);
		if (key_last > key_begin) {
			nodes_beween_range_rw(tree_rw->root, key_begin, key_last, thread_information->iteration_number);
		}
	}

	fclose(file_ptr);

	return 0;
}



//thread handler for put operation
void *handler_insert(void *arg)
{
    pthread_mutex_lock(&lock_fine);
	data_thread_iterations *thread_information = (data_thread_iterations *)arg;
	int key = 0, value = 0, i = 0;

	FILE *file_ptr = fopen(put_file, "r");
	
	if (file_ptr == NULL) 
    {
		printf("ERROR: File could not be opened\n");
		exit(0);
	}

	while ((i < thread_information->iteration_item) && (!feof (file_ptr))) 
    {
		fscanf(file_ptr, "%d %d\n", &key, &value);

		i++;
	}

	for (i = 0; i < thread_information->iteration_length; i++)
	{
		fscanf(file_ptr, "%d %d\n", &key, &value);
		insert(tree, key, value);
	}	

	fclose(file_ptr);	
pthread_mutex_unlock(&lock_fine);
	return 0;
}

// handler for delete function invocation
void *handler_delete(void *arg)
{
    pthread_mutex_lock(&lock_fine);
	data_thread_iterations *thread_information = (data_thread_iterations *)arg;
	int key = 0, value = 0, i = 0;

	FILE *file_ptr = fopen(deleteid_file, "r");
	
	if (file_ptr == NULL) 
    {
		printf("ERROR: File could not be opened\n");
		exit(0);
	}

	while ((i < thread_information->iteration_item) && (!feof (file_ptr))) 
    {
		fscanf(file_ptr, "%d %d\n", &key, &value);

		i++;
	}

	for (i = 0; i < thread_information->iteration_length; i++)
	{
		fscanf(file_ptr, "%d %d\n", &key, &value);
        printf("delete\n");
		deleteid(tree, key, value);
	}	

	fclose(file_ptr);	
    pthread_mutex_unlock(&lock_fine);
	return 0;
}

// handler for get function invocation
void *handler_get(void *arg)
{
    pthread_mutex_lock(&lock_fine);
	data_thread_iterations *thread_information = (data_thread_iterations *)arg;
	Node *node;
	int key = 0, i = 0;

	FILE *file_ptr = fopen(get_file, "r");
	
	if (file_ptr == NULL) 
    {
		printf("ERROR: File could not be opened\n");
		exit(0);
	}

	while ((i < thread_information->iteration_item) && (!feof (file_ptr))) 
    {
		fscanf(file_ptr, "%dn", &key);
		i++;
	}

	for (i = 0; i < thread_information->iteration_length; i++)
	{
		fscanf(file_ptr, "%d\n", &key);

		node = get(tree, key);
		if (node == NULL) 
        {
			printf("Node with key %d not found\n", key);
		} 
        else
        {
			printf("Node with key %d contains value %d\n", node->key, node->value);
		}	
		node = NULL;
	}

    fclose(file_ptr);	
    pthread_mutex_unlock(&lock_fine);
    return 0;
}

// handler for range function invocation
void *handler_range(void *arg)
{
    pthread_mutex_lock(&lock_fine);
	data_thread_iterations *thread_information = (data_thread_iterations *)arg;
    int key_begin = 0, key_last = 0, i = 0;

    FILE *file_ptr = fopen(range_file, "r");

    if (file_ptr == NULL) 
    {
		printf("ERROR: File could not be opened\n");
		exit(0);
	}

	while ((i < thread_information->iteration_item) && (!feof (file_ptr))) 
    {
		fscanf(file_ptr, "%d %d\n", &key_begin, &key_last);
		i++;
	}

	for (i = 0; i < thread_information->iteration_length; i++)
	{
		fscanf(file_ptr, "%d %d\n", &key_begin, &key_last);
		if (key_last > key_begin) {
			printf("Invoking range query result for %d to %d for thread %d\n", key_begin, key_last, thread_information->iteration_number);
			nodes_beween_range(tree->root, key_begin, key_last, thread_information->iteration_number);
		}
	}

	fclose(file_ptr);
    pthread_mutex_unlock(&lock_fine);
	return 0;
}

/* This function is used to read the input file and copy elements into the array */
int *file(char *required_file)
{
        char **array = NULL;        /* array of pointers to char        */ 
        char *ln = NULL;            /* NULL forces getline to allocate  */
        size_t n = 0;               /* buf size, 0 use getline default  */
        ssize_t nchr = 0;           /* number of chars actually read    */
        // size_t idx = 0;             /* array index for number of lines  */
        size_t lmax = LMAX;         /* current array pointer allocation */
        FILE *fp = NULL;            /* file pointer                     */
        if (! ( fp = fopen ( required_file, "r" ) ) ) 
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
    char *option_to_argument_lock;       // it is the algorithm option, that is merge or quick sort
    char read_write_lock[ 8 ] = "rw_lock";     // for comparing the option to argument alg is merge
    char fine_grained_lock[ 8 ] = "fg_lock";     // for comparing the option to argument alg is quick 

    /*maintains the long option list of arguments passes in the command line*/
    static struct option long_options[] =  {
                                                { "deleteid_file", 1, 0, 'd' },
                                                { "put_file", 1, 0, 'p' },
                                                { "get_file", 1, 0, 'g' },
                                                { "lock", 1, 0, 'l' },
                                                { "range_file", 1, 0, 'r' },
                                                { "help", 0, 0, 'h' },
                                                { "name", 0, 0, 'n' },
						                        { "thread", 1, 0, 't' },
                                                { 0, 0, 0, 0 }
                                            };
    /*checks if all characters or aguments passed in the command line have been read*/
    while (( character = getopt_long( argc, argv, "d:l:p:g:r:t:hn", long_options, &option_index )) != -1 )
    {
        switch( character )
        {
            case 'l':                   // checks if alg is passed
            {
                printf( "\n--alg-> option = %s\n", optarg ); //prints the algorithm option which is merge or quick
                option_to_argument_lock = optarg;             //optarg maintains the argument
                                                             //strcmp the value of option passed to --alg is merge
                if ( strcmp( option_to_argument_lock, read_write_lock ) == 0 )
                {
                    int i,j;
                    rw_lock_flag = 1;                          // merge flag is set
                }  
                                                             // strcmp the value of option passed to --alg is quick
                else if ( strcmp( option_to_argument_lock, fine_grained_lock ) == 0 ) 
                {
                    fg_lock_flag = 1;                          // quick flag is set
                }
                break;
            }

            case 'h':                    // checks if --name argument is passed and prints the name of the user
            {
                printf( "\nUSAGE: ./bst -p [file to put key, value] -g [file to get value based on key] -r [file which takes start and end key and return values in between] -d [delete file.txt] -t [Number of threads] --lock=[rw lock or fine grained lock]\n" );
                break;
            }
                
            case 'd':
            {
                deleteid_file = optarg;
                printf( "\n-d-> option = %s\n", optarg );  
                break;
            }
            case 'g':                     // checks if -o argument is passed and stores the file to be get in get_file
            {
                get_file = optarg;
                printf( "\n-g-> option = %s\n", optarg );

                
                break;
            }
            case 'p':                     // checks if -o argument is passed and stores the file to be put in put_file
            {
                put_file = optarg;
                printf( "\n-p-> option = %s\n", optarg );
                
                break;
            }
            case 'r':                     // checks if -r argument is passed and stores the file to be range in range_file
            {
                range_file = optarg;
                printf( "\n-r-> option = %s\n", optarg );
                
                
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
                if ( optopt == 'd' )
                    printf( "Option %c requires an argument which is the file where node will be deleted", optopt );
                if ( optopt == 'g' )
                    printf( "Option %c requires an argument which is the file where key and value will be inserted", optopt );
                else if ( optopt == 'p' )
                    printf( "Option %c requires an argument which is the file where the node based on the key will be returned", optopt );
                else if ( optopt == 'r' )
                    printf( "Option %c requires an argument which is the file where the range of nodes between two different keys will be returned", optopt );
               
                else if ( optopt == 't' )
                    thread_count = 5;           /* if no option specified take thread as 5 */
                    
                break;
            }
            default:                    // checks for unkown character
                printf( " returned character %c optopt = %c",character, optopt );
        }
    }

    if ( thread_count <= 6 )
        thread_count = 10;
    else if ( thread_count > 150 )
    {
        printf( "\nERROR: Too many threads\n");
        exit( -1 );
    }
    
    size_t args[thread_count];

    pthread_t threads[thread_count];

    if ( fg_lock_flag == 1 )
    {
        pthread_mutex_init(&lock_fine,NULL);
        pthread_mutex_init(&lock_fg_function, NULL);
        fg_lock_flag = 0;                            // flag for type of lock is made zero
        int *numbers_put;                            // the put file is read
        numbers_put = file(put_file);                          
        int number_of_put = idx;
        printf( "number_of_put put %d\n",number_of_put );
        idx = 0;
        free(numbers_put);
        int *numbers_range;                              // the range file is read
        numbers_range = file(range_file);                          
        int number_of_range = idx;
        printf( "number_of_range range %d\n",number_of_range );
        idx = 0;
        free(numbers_range);
        int *numbers_get;                              // the get file is read
        numbers_get = file(get_file);                          
        int number_of_get = idx;
        printf( "number_of_get get %d\n\n",number_of_get );
        idx = 0;
        free(numbers_get);   
        int *numbers_delete;                              // the delete file is read
        printf("delete id file = %s",deleteid_file);
        numbers_delete = file(deleteid_file);             
        int number_of_delete = idx;
        printf( "number_of_delete delete %d\n\n",number_of_delete );
        free(numbers_delete);
        
        // initialize tree and its lock
        tree=(Tree*)malloc(sizeof(Tree));

        pthread_mutex_init(&tree->lock, NULL);
        tree->root = NULL;
        
        if (number_of_put < thread_count) 
        {
            thread_count = (number_of_put / 2);
        }

        if (thread_count < 10) 
        {
            thread_count = 11;
        }

        data_thread_iterations thread_information[thread_count];
        pthread_t th[thread_count];
        int i = 0;
        
        int size_of_task = (number_of_put / (thread_count - 5));
        printf("size_of_task:%d\n",size_of_task);
        printf("thread_count:%d\n",thread_count);
        int item_key = 0, iterations = 0;
        
        clock_gettime(CLOCK_MONOTONIC,&start);
        
        // put function handling
		for (i = 0; i < (thread_count - 5); i++) 
        {
			item_key = i*size_of_task;
			if (i == (thread_count - 6)) 
            {
                thread_information[i].iteration_length = (number_of_put - item_key);
				thread_information[i].iteration_number = i;
				thread_information[i].iteration_item = item_key;
			} 
            else 
            {
                thread_information[i].iteration_length = size_of_task;
				thread_information[i].iteration_number = i;				
				thread_information[i].iteration_item = item_key;
			}
            
			if (pthread_create(&th[i], NULL, handler_insert, (void *)&thread_information[i]) != 0) 
			{
				printf("Error on creating the thread\n");
				exit(0);
			} 
 
		}

		size_of_task = (number_of_get/ 2);
		item_key = 0;
		iterations = 0;

        // get function handling
		for (; i < (thread_count -3); i++) 
        {
			item_key = iterations*size_of_task;

			if (i == (thread_count - 4)) 
            {
                thread_information[i].iteration_length = (number_of_get - item_key);
				thread_information[i].iteration_number = i;				
				thread_information[i].iteration_item = item_key;
			} 
            else 
            {
                thread_information[i].iteration_length = size_of_task;
				thread_information[i].iteration_number = i;				
				thread_information[i].iteration_item = item_key;
			}

			//invoke the get threads
			if (pthread_create(&th[i], NULL, handler_get, (void *)&thread_information[i]) != 0) 
            {
					printf("Error on creating the thread\n");
					exit(0);
			} 

			iterations++;
		}

        // range function handling
		size_of_task = (number_of_range/ 2);
		item_key = 0;
		iterations = 0;
		for (; i < (thread_count-1); i++) 
        {
			item_key = iterations*size_of_task;

			if (i == (thread_count - 2)) 
            {
                thread_information[i].iteration_length = (number_of_range - item_key);
				thread_information[i].iteration_number = iterations;				
				thread_information[i].iteration_item = item_key;
			} 
            else 
            {
                thread_information[i].iteration_length = size_of_task;
				thread_information[i].iteration_number = iterations;				
				thread_information[i].iteration_item = item_key;
			}

			//invoke the range querry threads
			if (pthread_create(&th[i], NULL, handler_range, (void *)&thread_information[i]) != 0) 
            {
					printf("Error on creating the thread\n");
					exit(0);
			} 

			iterations++;
		}

        
        for (i = 0; i < thread_count-1; i++) 
        {
			pthread_join(th[i], NULL);
		}
       
        
        clock_gettime(CLOCK_MONOTONIC,&end_time);
        printf("\n\n Range querry result:\n");
		for (int i = 0; i < 2; i++) 
        {
			for (int iterations = 0; iterations < range_info_fg_function[i].size(); iterations++) 
            {
				~~~~~~ printf("Value of range nodes with thread %d for %d to %d = %d\n",i, range_info_fg_function[i][iterations].key_begin, range_info_fg_function[i][iterations].key_last, range_info_fg_function[i][iterations].node->key);
			}
		}
        print_nodes_inorder(tree->root);

        pthread_mutex_destroy(&lock_fg_function);
        pthread_mutex_destroy(&tree->lock);
        pthread_mutex_destroy(&lock_fine);
        free(tree);
        free_all_nodes_in_bst(tree->root);
    }
    else if ( rw_lock_flag == 1 )    
    {
        rw_lock_flag = 0;
        int *numbers_put;                              // the unsorted file is read
        numbers_put = file(put_file);                          // read the file into the array
        int number_of_put = idx;
        printf( "number_of_put put %d\n",number_of_put );
        idx = 0;
        free(numbers_put);
        int *numbers_range;                              // the unsorted file is read
        numbers_range = file(range_file);                          // read the file into the array
        int number_of_range = idx;
        printf( "number_of_range range %d\n",number_of_range );
        idx = 0;
        free(numbers_range);
        int *numbers_get;                              // the unsorted file is read
        numbers_get = file(get_file);                          // read the file into the array
        int number_of_get = idx;
        printf( "number_of_get get %d\n",number_of_get );
        idx = 0;
        free(numbers_get);

        data_thread_iterations thread_information[thread_count];
        pthread_t th[thread_count];
        int i = 0;

        int size_of_task = (number_of_put / (thread_count - 2));

        int item_key = 0, iterations = 0;

        tree_rw=(Tree_rw*)malloc(sizeof(Tree_rw));
        pthread_rwlock_init(&tree_rw->lock, NULL);
        pthread_rwlock_init(&lock_rw_function, NULL);
        tree_rw->root = NULL;

        if (number_of_put < thread_count) 
        {
            thread_count = (number_of_put / 2);
        }

		clock_gettime(CLOCK_MONOTONIC,&start);

		for (i = 0; i < (thread_count - 2); i++) 
        {

			item_key = i * size_of_task ;
            printf("size_of_task_rw = %d \n",size_of_task);
            printf("item_key_rw = %d \n",item_key);
            printf("i_rw = %d \n",i);

			if (i == (thread_count - 3)) 
            {
                thread_information[i].iteration_length = (number_of_put - item_key);
				thread_information[i].iteration_number = i;				
				thread_information[i].iteration_item = item_key;
			} 
            else 
            {
                thread_information[i].iteration_length = size_of_task;
				thread_information[i].iteration_number = i;				
				thread_information[i].iteration_item = item_key;
			}

			if (pthread_create(&th[i], NULL, handler_insert_rw, (void *)&thread_information[i]) != 0) 
			{
				printf("Error on creating the thread\n");
				exit(0);
			} 

		}

		size_of_task = (number_of_get/ 2);
		item_key = 0;
		iterations = 0;

		for (; i < (thread_count ); i++) 
        {
			item_key = iterations*size_of_task;
            printf("size_of_task_rw_1 = %d \n",size_of_task);
            printf("item_key_rw_1 = %d \n",item_key);
            printf("i_rw_1 = %d \n",i);
            printf("iterations_rw_1 = %d \n",iterations);

			if (i == (thread_count - 1)) 
            {
                thread_information[i].iteration_length = (number_of_get - item_key);
				thread_information[i].iteration_number = i;				
				thread_information[i].iteration_item = item_key;
			} 
            else 
            {
                thread_information[i].iteration_length = size_of_task;
				thread_information[i].iteration_number = i;				
				thread_information[i].iteration_item = item_key;
			}

			if (pthread_create(&th[i], NULL, handler_get_rw, (void *)&thread_information[i]) != 0) 
            {
					printf("Error on creating the thread\n");
					exit(0);
			} 

			iterations++;
		}

		for (i = 0; i < thread_count; i++) 
        {
			pthread_join(th[i], NULL);
		}
        clock_gettime(CLOCK_MONOTONIC,&end_time);

        print_nodes_inorder_rw(tree_rw->root);
        
        pthread_rwlock_destroy(&lock_rw_function);
        pthread_rwlock_destroy(&tree_rw->lock);
        free(tree_rw);
        free_all_nodes_in_bst_rw(tree_rw->root);

    }

    unsigned long long elapsed_ns;
	elapsed_ns = (end_time.tv_sec-start.tv_sec)*1000000000 + (end_time.tv_nsec-start.tv_nsec);
	printf("Elapsed (ns): %llu\n",elapsed_ns);
	double elapsed_s = ((double)elapsed_ns)/1000000000.0;
	printf("Elapsed (s): %lf\n",elapsed_s);

    return 0;
}






