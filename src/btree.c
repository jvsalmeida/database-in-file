#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "../lib/btree.h"

NODE *createNode() {
	NODE *newNode = (NODE*)malloc(sizeof(NODE));

	if(!newNode) {
		printf("ERROR ! Can`t create Node");
		exit(0);
	}

	for(int i = 0; i < NODE_MAX; ++i)
		newNode->key[ i ] = '\0';

	for(int i = 0; i < NODE_MAX; ++i)
		newNode->rrn[ i ] = -1;

	for(int i = 0; i < NODE_POINTERS; ++i)
		newNode->link[ i ] = NULL;

	newNode->leaf = 1;
	newNode->count = 0;

	return newNode;
}


BTree *createBTree() {
	BTree *newRoot = (BTree*)malloc(sizeof(BTree));

	if(!newRoot)
		return NULL;

	NODE *head = createNode();

	if(!head)
		return NULL;

	newRoot->order = NODE_ORDER;
	newRoot->root = head;

	return newRoot;
}


RESULT *getResult(void) {
	RESULT *ret = (RESULT *)malloc(sizeof(RESULT));

	if(!ret){
		printf("ERROR! Can`t getResult");
		exit(0);
	}

	ret->found = 0;
	ret->position = -1;
	ret->rrn = -1;
	ret->depth = 0;
	ret->nodePointer = NULL;

	return ret;
}


RESULT *search(NODE *node, char *primary) {
	int i = 0;

	while((i < node->count) && (strcmp(primary, node->key[ i ]) > 0)) {
		++i;
	}

	// Verificar se achamos a chave
	if((node != NULL) && (i <= node->count) && (node->key[ i ] != NULL) && (strcmp(primary, node->key[ i ]) == 0)) {
		RESULT *result = getResult();
		result->nodePointer = node;
		result->position = i;
		result->rrn = node->rrn[ i ];
		result->found = 1;

		return result;
	}

	// Verificar se eh folha
	if(node->leaf) {
		RESULT *result = getResult();
		result->nodePointer = node;
		result->found = 0;

		return result;
	}
	else {
		RESULT *result = getResult();

		return search(node->link[ i ], primary);
	}
}


void splitChild(NODE *parentNode, int number, NODE *childNode) {
	int j;

	// Alocar um novo nó para o filho
	NODE *newNode  = createNode();
	newNode->leaf  = childNode->leaf;
	newNode->count = NODE_ORDER - 1;

	// Move os valores da metade da direita do nó filho para o novo nó
	for(j = 0; j < (NODE_ORDER - 1); ++j) {
		newNode->key[ j ] = childNode->key[ NODE_ORDER + j ];
		newNode->rrn[ j ] = childNode->rrn[ NODE_ORDER + j ];
	}

	//Se o childNode não for folha, então mover os ponteiros para o novo nó filho
	if(childNode->leaf == 0) {
		for(j = 0; j < NODE_ORDER; ++j)
			newNode->link[ j ] = childNode->link[ NODE_ORDER + j ];
	}
	childNode->count = (NODE_ORDER - 1);

	// Move para a direita os ponteiros do pai
	for(j = parentNode->count; j >= number; --j)
		parentNode->link[ j + 1 ] = parentNode->link[ j ];

	parentNode->link[ number ] = newNode;

	// Mover os valores para a direita, do indice number-1
	for(j = parentNode->count; j >= number; --j) {
		parentNode->key[ j ] = parentNode->key[ j - 1 ];
		parentNode->rrn[ j ] = parentNode->rrn[ j - 1 ];
	}

	// Põe o valor do pai[number - 1] para a posição média do childNode
	parentNode->key[ number - 1 ] = childNode->key[ NODE_ORDER - 1 ];
	parentNode->rrn[ number - 1 ] = childNode->rrn[ NODE_ORDER - 1 ];

	parentNode->count++;
}


void insertoNotfull(NODE *node, char *primary, int offset) {
	int number = node->count;

	if(node->leaf) {
		// Mover até caber
		while(number >= 1 && strcmp(primary, node->key[number - 1]) < 0) {
			node->key[ number ] = node->key[ number - 1 ];
			node->rrn[ number ] = node->rrn[ number - 1 ];
			--number;
		}
		node->key[ number ] = strdup(primary);
		node->rrn[ number ] = offset;
		node->count++;

	
	} else {
		while(number >= 1 && strcmp(primary, node->key[number - 1]) < 0)
			--number;

		if(node->link[ number ]->count == NODE_MAX) {
			splitChild(node, number + 1, node->link[ number ]);

			if(strcmp(primary, node->key[ number ]) > 0)
				++number;
		}

		insertoNotfull(node->link[ number ], primary, offset);
	}
}


NODE *insert(BTree *tree, char *primary, int offset) {

	NODE *head = tree->root;

	if(head->count == NODE_MAX) {
		NODE *newNode = createNode();

		// novo nó é a raiz agora
		tree->root = newNode;		
		newNode->leaf = 0;

		// acho q vou deletar
		newNode->count = 0;			
		newNode->link[0] = head;

		splitChild(newNode, 1, head);
		insertoNotfull(newNode, primary, offset);

	} else {
		insertoNotfull(tree->root, primary, offset);
	}

	return tree->root;
}


void mergeChildren(NODE *root, int index, NODE *child1, NODE *child2) {
	child1->count = NODE_MAX;
	int i;

	for(i = NODE_ORDER; i < NODE_MAX; ++i) {
		child1->key[ i ] = child2->key[ i - NODE_ORDER ];
		child1->rrn[ i ] = child2->rrn[ i - NODE_ORDER ];
	}
	child1->key[ NODE_ORDER - 1 ] = root->key[ index ];
	child1->rrn[ NODE_ORDER - 1 ] = root->rrn[ index ];

	if(child2->leaf == 0) {
		for(i = NODE_ORDER; i < NODE_POINTERS; ++i)
			child1->link[ i ] = child2->link[ i - NODE_ORDER ];
	}

	for(i = (index + 1); i < root->count; ++i) {
		root->key[ i - 1 ] = root->key[ i ];
		root->rrn[ i - 1 ] = root->rrn[ i ];
		root->link[ i ] = root->link[ i + 1];
	}
	root->count--;

	free(child2);
}


char *BTreeGetLeftMax(NODE *node) {
	if(node->leaf == 0) {
		return BTreeGetLeftMax(node->link[ node->count ]);
	} else {
		return node->key[ node->count-1 ];
	}
}


char *BTreeGetRightMin(NODE *node) {
	if(node->leaf == 0) {
		return BTreeGetRightMin(node->link[ 0 ]);
	} else {
		return node->key[ 0 ];
	}
}


void BTreeBorrowFromLeft(NODE *root, int index, NODE *leftPtr, NODE *curPtr) {
	int i;
	curPtr->count++;

	for(i = curPtr->count-1; i > 0; --i) {
		curPtr->key[ i ] = curPtr->key[ i - 1 ];
		curPtr->rrn[ i ] = curPtr->rrn[ i - 1 ];
	}
	curPtr->key[ 0 ] = root->key[ index ];
	curPtr->rrn[ 0 ] = root->rrn[ index ];
	
	root->key[ index ] = leftPtr->key[ leftPtr->count-1 ];
	root->rrn[ index ] = leftPtr->rrn[ leftPtr->count-1 ];
	
	if(leftPtr->leaf == 0)
		for(i = curPtr->count; i > 0 ; --i)
			curPtr->link[i] = curPtr->link[ i - 1 ];
	curPtr->link[ 0 ] = leftPtr->link[ leftPtr->count ];
	leftPtr->count--;
}


void BTreeBorrowFromRight(NODE *root, int index, NODE *rightPtr, NODE *curPtr) {
	curPtr->count++;

	curPtr->key[ curPtr->count-1 ] = root->key[ index ];
	curPtr->rrn[ curPtr->count-1 ] = root->rrn[ index ];

	root->key[ index ] = rightPtr->key[ 0 ];
	root->rrn[ index ] = rightPtr->rrn[ 0 ];
	
	int i;
	
	for(i = 0; i < rightPtr->count-1; ++i) {
		rightPtr->key[ i ] = rightPtr->key[ i + 1 ];
		rightPtr->rrn[ i ] = rightPtr->rrn[ i + 1 ];
	}
	
	if(rightPtr->leaf == 0){
		curPtr->link[ curPtr->count ] = rightPtr->link[ 0 ];
		for(i = 0; i<rightPtr->count; ++i)
			rightPtr->link[ i ] = rightPtr->link[ i + 1 ];
	}

	rightPtr->count--;
}


int BTreeGetLeftMaxInt(NODE *tree){
	if(tree->leaf == 0){
		return BTreeGetLeftMaxInt(tree->link[ tree->count ]);
	}else{
		return tree->rrn[ tree->count-1 ];
	}
}

int BTreeGetRightMinInt(NODE *tree){
	if(tree->leaf == 0){
		return BTreeGetRightMinInt(tree->link[ 0 ]);
	}else{
		return tree->rrn[ 0 ];
	}
}


void BTreeDeleteKey(NODE *root, char *primary) {
	int i;

	if(root->leaf == 1) {
		i = 0;
		while( (i < root->count) && (strcmp(primary, root->key[ i ]) > 0) )
			++i;

		if(strcmp(primary, root->key[ i ]) == 0) {
			for(; i < root->count - 1; ++i) {
				root->key[ i ] = root->key[ i + 1 ];
				root->rrn[ i ] = root->rrn[ i + 1 ];
			}
			root->count--;  
		}
		else {
			printf("Node not found.\n");
			return;
		}
	}
	else {
		i = 0;
		NODE *prevPtr = NULL;
		NODE *nextPtr = NULL;

		while( (i < root->count) && (strcmp(primary, root->key[ i ]) > 0) )
			++i;

		if( (i < root->count) && (strcmp(primary, root->key[ i ]) == 0) ) {
			prevPtr = root->link[ i ];
			nextPtr = root->link[ i + 1 ];

			if(prevPtr->count > (NODE_ORDER - 1)) {
				char *precursor = BTreeGetLeftMax(prevPtr);
				int precursorInt = BTreeGetLeftMaxInt(prevPtr);

				free(root->key[ i ]);		
				root->key[ i ] = strdup(precursor);
				root->rrn[ i ] = precursorInt;

				BTreeDeleteKey(prevPtr, precursor);
			}
			else if(nextPtr->count > (NODE_ORDER-1)) {
				char *sucessor = BTreeGetRightMin(nextPtr);
				int sucessorInt = BTreeGetRightMinInt(nextPtr);

				free(root->key[ i ]);
				root->key[ i ] = strdup(sucessor);
				root->rrn[ i ] = sucessorInt;

				BTreeDeleteKey(nextPtr, sucessor);
			}
			else {
				mergeChildren(root, i, prevPtr, nextPtr);
				BTreeDeleteKey(prevPtr, primary);
			}
		}
		else {
			prevPtr = root->link[ i ];
			NODE *leftBro = NULL;

			if(i < root->count)
				nextPtr = root->link[ i + 1 ];
			if(i > 0)
				leftBro = root->link[ i - 1 ];

			if((NODE_ORDER - 1) == prevPtr->count) {
				if((leftBro != NULL) && (leftBro->count > NODE_ORDER-1)){
					BTreeBorrowFromLeft(root, i-1, leftBro, prevPtr);
				}
				else if((nextPtr != NULL) && (nextPtr->count > NODE_ORDER-1)) {
					BTreeBorrowFromRight(root, i, nextPtr, prevPtr);
				}
				else if(leftBro != NULL) {
					mergeChildren(root, i-1, leftBro, prevPtr);
					prevPtr = leftBro;
				}
				else
					mergeChildren(root, i, prevPtr, nextPtr);
			}

			BTreeDeleteKey(prevPtr, primary);
		}
	}
}


NODE *delete(BTree *tree, char *primary) {

	if(tree->root->count == 1) {
		NODE *child1 = tree->root->link[ 0 ];
		NODE *child2 = tree->root->link[ 1 ];

		if((child1 != NULL) && (child2 != NULL)) {
			if(((NODE_ORDER - 1) == child1->count) && ((NODE_ORDER - 1) == child2->count)) {
				mergeChildren(tree->root, 0, child1, child2);
				free(tree->root);
				BTreeDeleteKey(child1, primary);

				return child1;
			}
		}
	}
	BTreeDeleteKey(tree->root, primary);

	return tree->root;
}


void printBTree(NODE *myNode) {
	int i;
	if (myNode) {
		for (i = 0; i < myNode->count; ++i) {
			printBTree(myNode->link[ i ]);
			printf("%s ", myNode->key[ i ]);
			printf("\n");
		}
		printBTree(myNode->link[ i ]);
	}

	return;
}