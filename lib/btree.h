#ifndef BTREE_H
# define BTREE_H

#define NODE_ORDER		3   
#define NODE_MAX		(NODE_ORDER*2)-1
#define NODE_POINTERS	(NODE_ORDER*2)

struct BTreeNode {
	char *key[ NODE_MAX ];
	bool leaf;
	int rrn[ NODE_MAX ];
	int count;
	struct BTreeNode *link[ NODE_MAX + 1 ];
};

struct BTree {
	int order;
	struct BTreeNode *root;
};

struct result{
	bool found;
	int position;
	int rrn;
	int depth;
	struct BTreeNode *nodePointer;
};

typedef struct BTreeNode NODE;
typedef struct BTree BTree;
typedef struct result RESULT;

NODE *createNode();

BTree *createBTree();

RESULT *getResult(void);

RESULT *search(NODE *, char *);

void splitChild(NODE *, int , NODE *);

void insertoNotfull(NODE *, char *, int);

NODE *insert(BTree *, char *, int);

void mergeChildren(NODE *, int , NODE *, NODE *);

char *BTreeGetLeftMax(NODE *);

char *BTreeGetRightMin(NODE *);

void BTreeBorrowFromLeft(NODE *, int , NODE *, NODE *);

void BTreeBorrowFromRight(NODE *, int , NODE *, NODE *);

void BTreeDeleteKey(NODE *, char *);

NODE *delete(BTree *, char *);

void printBTree(struct BTreeNode *);

void printNode(NODE *);

#endif