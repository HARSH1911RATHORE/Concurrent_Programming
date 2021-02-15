
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
#include <stdio.h> 
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

typedef struct Node_rw {
    pthread_rwlock_t lock;
    int key;
    int value;
    struct Node_rw *left;
    struct Node_rw *right;
    
}Node_rw;

typedef struct rw_range 
{ 
   int key_begin;
   int key_last;
   Node_rw *node;
}size_rw_range_info; 
vector <size_rw_range_info> range_info_rw_function[2];

typedef struct Tree_rw {
    pthread_rwlock_t lock;
    Node_rw *root  ;
}Tree_rw;

Tree_rw *tree_rw=NULL;

typedef struct Node {
    pthread_mutex_t lock;
    int key;
    int value;
    struct Node *left;
    struct Node *right;
    
}Node;

typedef struct fg_range 
{ 
   int key_begin;
   int key_last;
   Node *node;
}size_fg_range_info; 
vector <size_fg_range_info> range_info_function[2];

pthread_rwlock_t lock_rw_function;
pthread_mutex_t lock_fg_function;
typedef struct Tree {
    pthread_mutex_t lock;
    Node *root  ;
}Tree;

Tree *tree=NULL;

void insert_helper_rw(Node_rw *root, int key, int value, Node_rw *parent) {
    if (root) {
        pthread_rwlock_wrlock(&root->lock);
        if (parent) pthread_rwlock_unlock(&parent->lock);
        if (root->key > key) {
            if (root->left) {
                insert_helper_rw(root->left, key, value, root);
            } else {
                Node_rw *n = (Node_rw*)malloc(sizeof(Node_rw));
				pthread_rwlock_init(&n->lock, NULL);
                n->key = key;
                n->value = value;
                n->left = NULL;
                n->right = NULL;
                root->left = n;
                pthread_rwlock_unlock(&root->lock);
            }
        } else if (root->key < key){
            if (root->right) {
                insert_helper_rw(root->right,key, value, root);
            } else {
                Node_rw *n = (Node_rw*)malloc(sizeof(Node_rw));
				pthread_rwlock_init(&n->lock, NULL);
                n->value = value;
                n->key = key;
                n->left = NULL;
                n->right = NULL;
                root->right = n;
               pthread_rwlock_unlock(&root->lock);
            }
        } else {
            // do nothing, value exists
           root->value = value;
           pthread_rwlock_unlock(&root->lock);
            return;
        }
    }
}

void insert_rw(Tree_rw *tree, int key, int value) {

    // Case 1: tree is empty
             pthread_rwlock_wrlock(&tree->lock);

    if (!tree->root) {
        Node_rw *n = (Node_rw*)malloc(sizeof(Node_rw));
		pthread_rwlock_init(&n->lock, NULL);
        n->value = value;
        n->key = key;
        n->left = NULL;
        n->right = NULL;
        tree->root = n;
         pthread_rwlock_unlock(&tree->lock);
        return;
    }

    // Case 2: Tree is non-empty
     pthread_rwlock_unlock(&tree->lock);
    insert_helper_rw(tree->root, key, value, NULL);
}

Node_rw * get_helper_rw(Node_rw *root, int key, Node_rw *parent) {
    if (root) {
        pthread_rwlock_rdlock(&root->lock);
        if (parent) pthread_rwlock_unlock(&parent->lock);
        if (root->key > key) {
            if (root->left) {
                get_helper_rw(root->left, key, root);
            } else {

                pthread_rwlock_unlock(&root->lock);
                return NULL;
            }
        } else if (root->key < key){
            if (root->right) {
                get_helper_rw(root->right,key, root);
            } else {

               pthread_rwlock_unlock(&root->lock);
               return NULL;
            }
        } else {
            // do nothing, value exists


           pthread_rwlock_unlock(&root->lock);
           return root;
        }
    }
}

Node_rw * get_rw(Tree_rw *tree, int key) {

    // Case 1: tree is empty
    pthread_rwlock_rdlock(&tree->lock);

    if (!tree->root) {

        pthread_rwlock_unlock(&tree->lock);
        return NULL;
    }

    // Case 2: Tree is non-empty
    pthread_rwlock_unlock(&tree->lock);
    get_helper_rw(tree->root, key, NULL);
}

//fetch all the nodes in the given range
void nodes_in_between_range_rw(Node_rw *root, int key_begin, int key_last)
{
	if (root == NULL)
	{
		pthread_rwlock_rdlock(&lock_rw_function);
		if (tree->root == NULL)
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
			nodes_in_between_range_rw(root->left, key_begin, key_last);
			pthread_rwlock_rdlock(&root->lock);
		} 
		
	}

	if ((key_begin <= root->key) && (key_last >= root->key))
	{
		range_info_rw_function[0].push_back({key_begin, key_last, root});
	}
	
	if (key_last > root->key)
    {
        if (root->right != NULL)
        {
        	pthread_rwlock_rdlock(&root->right->lock);
        	pthread_rwlock_unlock(&root->lock);
        	nodes_in_between_range_rw(root->right, key_begin, key_last);
        	pthread_rwlock_rdlock(&root->lock);
        } 
    }

    pthread_rwlock_unlock(&root->lock);

}

void info_range_between_start_and_end_rw(Tree_rw *tree, int key_begin, int key_last)
{
	if (tree->root == NULL) {
		return;
	}

	Node_rw *start_node = get_rw(tree, key_begin);
	Node_rw *end_node = get_rw(tree, key_last);

	if (start_node == NULL) {
		printf("Invalid Query. Node with key %d is not present in the tree\n", key_begin);
	} else if (end_node == NULL) {
		printf("Invalid Query. Node with key %d is not present in the tree\n", key_last);
	} else {
		pthread_rwlock_rdlock(&lock_rw_function);
		pthread_rwlock_rdlock(&tree->root->lock);
		pthread_rwlock_unlock(&lock_rw_function);
		nodes_in_between_range_rw(tree->root, key_begin, key_last);
    }
}

Node_rw* getMin_rw(Node_rw *root, Node_rw *parent) {

    if (root) {
        pthread_rwlock_rdlock(&root->lock);
        if (parent) pthread_rwlock_unlock(&parent->lock);
        if (root->left) {
            return getMin_rw(root->left, root);
        } else {
            pthread_rwlock_unlock(&root->lock);
            return root;
        }
    } else {
        // Shouldn't happen
        
        return NULL;
    }
}

Node_rw *delete_helper_rw(Node_rw *root, int key, int value, Node_rw *parent) {

    if (root) {
        pthread_rwlock_rdlock(&root->lock);
        if (root->key == key) {
            // Case 1: root has no children
            if (!root->left && !root->right) {
                Node_rw *tmp = root;
                root = NULL;
                // Modify parent after this node is fixed
                if (parent) pthread_rwlock_unlock(&parent->lock);
                pthread_rwlock_unlock(&tmp->lock);
                free(tmp);
                return root;
            }
            // Case 2: root has 1 child
            if (!root->left && root->right) {
                Node_rw *tmp = root;
                root = root->right;
                // Modify parent after this node is fixed
                if (parent) pthread_rwlock_unlock(&parent->lock);
                pthread_rwlock_unlock(&tmp->lock);
                free(tmp);
                return root;
            }
            if (!root->right && root->left) {
                Node_rw *tmp = root;
                root = root->left;
                // Modify parent after this node is fixed
                if (parent) pthread_rwlock_unlock(&parent->lock);
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
        else if (root->key > key) {
            // We can releas parent as we modify root and root->left now
            if (parent) pthread_rwlock_unlock(&parent->lock);
            // Delete value node from left sub-tree
            root->left = delete_helper_rw(root->left, key, value, root);
        } else {
            // We can releas parent as we modify root and root->right now
            if (parent) pthread_rwlock_unlock(&parent->lock);
            // Delete value node from right sub-tree
            root->right = delete_helper_rw(root->right, key, value, root);
        }
    } else {
        // Nothing to delete
        return root;
    }
}

void deleteid_rw(Tree_rw *tree, int key, int value) {

    // Case 1: tree is empty
    pthread_rwlock_rdlock(&tree->lock);
    if (!tree->root){
        pthread_rwlock_unlock(&tree->lock);
        return;
    }

    // Case 2: Tree is non-empty
            pthread_rwlock_unlock(&tree->lock);

    tree->root = delete_helper_rw(tree->root, key, value, NULL);
}

void insert_helper(Node *root, int key, int value, Node *parent) {
    if (root) {
        pthread_mutex_lock(&root->lock);
        if (parent) pthread_mutex_unlock(&parent->lock);
        if (root->key > key) {
            if (root->left) {
                insert_helper(root->left, key, value, root);
            } else {
                Node *n = (Node*)malloc(sizeof(Node));
                n->key = key;
                n->value = value;
                n->left = NULL;
                n->right = NULL;
                pthread_mutex_init(&n->lock, NULL);
                root->left = n;
                pthread_mutex_unlock(&root->lock);
            }
        } else if (root->key < key){
            if (root->right) {
                insert_helper(root->right,key, value, root);
            } else {
                Node *n = (Node*)malloc(sizeof(Node));
                n->value = value;
                n->key = key;
                n->left = NULL;
                n->right = NULL;
                root->right = n;
               pthread_mutex_unlock(&root->lock);
            }
        } else {
            // do nothing, value exists
           root->value = value;
           pthread_mutex_unlock(&root->lock);
            return;
        }
    }
}

void insert(Tree *tree, int key, int value) {

    // Case 1: tree is empty
            pthread_mutex_lock(&tree->lock);

    if (!tree->root) {
        Node *n = (Node*)malloc(sizeof(Node));
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

Node * get_helper(Node *root, int key, Node *parent) {
    if (root) {
        pthread_mutex_lock(&root->lock);
        if (parent) pthread_mutex_unlock(&parent->lock);
        if (root->key > key) {
            if (root->left) {
                get_helper(root->left, key, root);
            } else {

                pthread_mutex_unlock(&root->lock);
                return NULL;
            }
        } else if (root->key < key){
            if (root->right) {
                get_helper(root->right,key, root);
            } else {

               pthread_mutex_unlock(&root->lock);
               return NULL;
            }
        } else {
            // do nothing, value exists

           pthread_mutex_unlock(&root->lock);
            return root;
        }
    }
}

Node * get(Tree *tree, int key) {

    // Case 1: tree is empty
            pthread_mutex_lock(&tree->lock);

    if (!tree->root) {
 
        pthread_mutex_unlock(&tree->lock);
        return NULL;
    }

    // Case 2: Tree is non-empty
    pthread_mutex_unlock(&tree->lock);
    get_helper(tree->root, key, NULL);
}

void nodes_in_between_range(Node *root, int key_begin, int key_last)
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
			nodes_in_between_range(root->left, key_begin, key_last);
			pthread_mutex_lock(&root->lock);
		} 
		
	}

	if ((key_begin <= root->key) && (key_last >= root->key))
	{
		range_info_function[0].push_back({key_begin, key_last, root});
	}
	
	if (key_last > root->key)
    {
        if (root->right != NULL)
        {
        	pthread_mutex_lock(&root->right->lock);
        	pthread_mutex_unlock(&root->lock);
        	nodes_in_between_range(root->right, key_begin, key_last);
        	pthread_mutex_lock(&root->lock);
        } 
    }

    pthread_mutex_unlock(&root->lock);

}

void info_fg_range_between_start_and_end(Tree *tree, int key_begin, int key_last)
{
	if (tree->root == NULL) {
		return;
	}

	Node *start_node = get(tree, key_begin);
	Node *end_node = get(tree, key_last);

	if (start_node == NULL) {
		printf("Invalid Query. Node with key %d is not present in the tree\n", key_begin);
	} else if (end_node == NULL) {
		printf("Invalid Query. Node with key %d is not present in the tree\n", key_last);
	} else {
		pthread_mutex_lock(&lock_fg_function);
		pthread_mutex_lock(&tree->root->lock);
		pthread_mutex_unlock(&lock_fg_function);
		nodes_in_between_range(tree->root, key_begin, key_last);
	}
}



Node* getMin(Node *root, Node *parent) {

    if (root) {
        pthread_mutex_lock(&root->lock);
        if (parent) pthread_mutex_unlock(&parent->lock);
        if (root->left) {
            return getMin(root->left, root);
        } else {
            pthread_mutex_unlock(&root->lock);
            return root;
        }
    } else {
        // Shouldn't happen
        
        return NULL;
    }
}

Node *delete_helper(Node *root, int key, int value, Node *parent) {

    if (root) {
        pthread_mutex_lock(&root->lock);
        if (root->key == key) {
            // Case 1: root has no children
            if (!root->left && !root->right) {
                Node *tmp = root;
                root = NULL;
                // Modify parent after this node is fixed
                if (parent) pthread_mutex_unlock(&parent->lock);
                pthread_mutex_unlock(&tmp->lock);
                free(tmp);
                return root;
            }
            // Case 2: root has 1 child
            if (!root->left && root->right) {
                Node *tmp = root;
                root = root->right;
                // Modify parent after this node is fixed
                if (parent) pthread_mutex_unlock(&parent->lock);
                pthread_mutex_unlock(&tmp->lock);
                                // unlock(tmp->lock);
                free(tmp);
                return root;
            }
            if (!root->right && root->left) {
                Node *tmp = root;
                root = root->left;
                // Modify parent after this node is fixed
                if (parent) pthread_mutex_unlock(&parent->lock);
                pthread_mutex_unlock(&tmp->lock);
                free(tmp);
                return root;
            }
            // Case 3: root has 2 children
            // Replace root with highest value smaller than root
            Node *minRight = getMin(root->right, NULL);
            root->key = minRight->key;
            // Now delete minRight node; set its parent's left child to NULL or
            // to minRight node's right child
            // We pass NULL as parent as we unlock root after deletion is complete
            root->right = delete_helper(root->right, minRight->key, minRight->value, NULL);
            // Unlock after this sub-tree root is fixed to avoid violating
            // BST invariants when other threads insert simultaneously
            pthread_mutex_unlock(&root->lock);
        }
        else if (root->key > key) {
            // We can releas parent as we modify root and root->left now
            if (parent) pthread_mutex_unlock(&parent->lock);
            // Delete value node from left sub-tree
            root->left = delete_helper(root->left, key, value, root);
        } else {
            // We can releas parent as we modify root and root->right now
            if (parent) pthread_mutex_unlock(&parent->lock);
            // Delete value node from right sub-tree
            root->right = delete_helper(root->right, key, value, root);
        }
    } else {
        // Nothing to delete
        return root;
    }
}

void deleteid(Tree *tree, int key, int value) {

    // Case 1: tree is empty
    pthread_mutex_lock(&tree->lock);
    if (!tree->root){
       pthread_mutex_unlock(&tree->lock);
        return;
    }

    // Case 2: Tree is non-empty
            pthread_mutex_unlock(&tree->lock);

    tree->root = delete_helper(tree->root, key, value, NULL);
}

// A utility function to do inorder traversal of BST 
void print_nodes_inorder_rw(Node_rw* root) 
{ 
    if (root != NULL) { 
        print_nodes_inorder_rw(root->left); 
        printf("key_rw:%d value_rw:%d\n", root->key, root->value); 
        print_nodes_inorder_rw(root->right); 
    } 
} 

// A utility function to do inorder traversal of BST 
void print_nodes_inorder(Node* root) 
{ 
    if (root != NULL) { 
        print_nodes_inorder(root->left); 
        printf("key:%d value:%d\n", root->key, root->value); 
        print_nodes_inorder(root->right); 
    } 
} 


void array_with_tree_nodes(Node *root, int array[])
{
    static int location = 0;
    if(!root) 
        return;

    array_with_tree_nodes(root->left, array);
    array[location++] = root->key;
    array_with_tree_nodes(root->right, array);

}

bool get_function_unit_test()
{

	Node *temp;

	for (int i = 0; i < 15; i++) 
    {
		temp = get(tree, i);
		if (!temp)
			return false;
		assert(temp->value == i+10);
	}
	return true;
}


bool insert_function_unit_test()
{
     for(int i=0; i<15; i++)
     {
         insert(tree, i, i+10);
     }
    
    int *array = new int[15];
    array_with_tree_nodes(tree->root, array);
    
     for(int i=0; i<15; i++)
     {
         assert(i == array[i]);
     }
    
    free(array);
    return true;
     
 }



int main()
{
    
        tree=(Tree*)malloc(sizeof(Tree));

        pthread_mutex_init(&tree->lock, NULL);
        tree->root = NULL;
     	pthread_mutex_init(&lock_fg_function, NULL);

    	if (insert_function_unit_test()) 
        {
            printf("Put function test passed\n");
        }
    
        if (get_function_unit_test() ) 
        {
            printf("Get function test passed\n");
        }

        insert(tree, 1, 10);
        insert(tree,2, 20);
        insert(tree,2, 30);
        insert(tree, 1, 10);
        insert(tree, 1, 100);
        insert(tree, 1, 200);
        insert(tree, 1, 500);
        for (int i =0 ; i<15; i++)
        {
            insert(tree, i+10, i+100);
        }
        int test=0;
        deleteid(tree, 10, 60);
        printf("Delete function test passed\n");

        info_fg_range_between_start_and_end(tree, 1, 20);
    
        printf("Range function test passed\n");
        printf("\n\n Range range_info_function result:\n");
            for (int i = 0; i < 2; i++) {
                for (int j = 0; j < range_info_function[i].size(); j++) {
                    printf("Value of range nodes with thread  %d for %d to %d = %d\n",i, range_info_function[i][j].key_begin, range_info_function[i][j].key_last, range_info_function[i][j].node->key);
                    printf("Range function test passed\n"); 
                }
            }
            print_nodes_inorder(tree->root);
  

        Node* node = get(tree, 11);
        printf("key %d has value %d",node->key,node->value);

        free(tree);

        tree_rw=(Tree_rw*)malloc(sizeof(Tree_rw));
        pthread_rwlock_init(&tree_rw->lock, NULL);
        pthread_rwlock_init(&lock_rw_function, NULL);
        tree_rw->root = NULL;
        insert_rw(tree_rw, 1, 10);
        insert_rw(tree_rw,2, 20);
        insert_rw(tree_rw,2, 30);
        insert_rw(tree_rw, 1, 10);
        insert_rw(tree_rw, 1, 100);
        insert_rw(tree_rw, 1, 200);
        insert_rw(tree_rw, 1, 500);
        deleteid_rw(tree_rw, 2,50);
        for (int i =0 ; i<15; i++)
        {
            insert_rw(tree_rw, i+10, i+100);
        }
        print_nodes_inorder_rw(tree_rw->root);
        Node_rw* nod = get_rw(tree_rw, 1);
        printf("key %d has value %d",nod->key,nod->value);
        info_range_between_start_and_end_rw(tree_rw, 1, 20);
                printf("\n\n Range range_info_function result:\n");
            for (int i = 0; i < 2; i++) {
                for (int j = 0; j < range_info_rw_function[i].size(); j++) {
                    printf("Range range_info_function by thread %d for %d to %d = %d\n",i, range_info_rw_function[i][j].key_begin, range_info_rw_function[i][j].key_last, range_info_rw_function[i][j].node->key);
                }
            }

    return 0;
}
