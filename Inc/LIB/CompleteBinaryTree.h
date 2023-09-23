/*
 * CompleteBinaryTree.h
 *
 *  Created on: Sep 23, 2023
 *      Author: Ali Emad
 */

#ifndef COTS_OS_INC_LIB_COMPLETEBINARYTREE_H_
#define COTS_OS_INC_LIB_COMPLETEBINARYTREE_H_

#include "LIB/Vector/Vector.h"

typedef struct{
	/*		PRIVATE		*/
	xLIB_Vector_t xVector;
}xLIB_CompleteBinaryTree_t;

/*******************************************************************************
 * Private:
 ******************************************************************************/
/*
 * Gets index of a node given its pointer and its tree.
 */
#define uiLIB_COMPLETE_BINARY_TREE_GET_INDEX(pxTree, pxNode)	\
	(	(pxNode) - &xLIB_VECTOR_GET_ELEM(&(pxTree)->xVector, 0)	)

/*******************************************************************************
 * Public:
 ******************************************************************************/
/*
 * Mutex locking and unlocking of a CompleteBinaryTree handle.
 *
 * Notes:
 * 		-	CompleteBinaryTree handle should not be used without being locked first.
 *
 * 		-	CompleteBinaryTree handle can only be locked and unlocked form inside a
 * 			task, it is not possible from ISR.
 *
 * 		-	CompleteBinaryTree handle does not actually have its own mutex, it explicitly
 * 			uses that of its vector handle.
 */
#define ucLIB_COMPLETE_BINARY_TREE_LOCK(pxTree, xTimeout)	\
	(	ucLIB_VECTOR_LOCK(&(pxTree)->xVector, (xTimeout))	)

#define vLIB_COMPLETE_BINARY_TREE_RELEASE(pxVector)	\
	(	vLIB_VECTOR_RELEASE(&(pxTree)->xVector)	)

/*
 * Initializes tree handle.
 */
void vLIB_CompleteBinaryTree_init(xLIB_CompleteBinaryTree_t* pxTree);

/*
 * Gets tree's size.
 */
#define uiLIB_COMPLETE_BINARY_TREE_GET_SIZE(pxTree)	\
	(	uiLIB_VECTOR_GET_SIZE(&(pxTree)->xVector) - 1	)

/*
 * Gets pointer to the root node.
 */
#define pxLIB_COMPLETE_BINARY_TREE_GET_ROOT_PTR(pxTree)	\
	(	&xLIB_VECTOR_GET_ELEM(&(pxTree)->xVector, 1)	)

/*
 * Gets pointer to the left child of a given node.
 */
#define pxLIB_COMPLETE_BINARY_TREE_GET_LEFT_PTR(pxTree, pxParent)	\
	(	&xLIB_VECTOR_GET_ELEM(&(pxTree)->xVector, 2 * uiLIB_COMPLETE_BINARY_TREE_GET_INDEX(pxTree, pxParent))	)

/*
 * Gets pointer to the parent of a given node.
 */
#define pxLIB_COMPLETE_BINARY_TREE_GET_PARENT_PTR(pxTree, pxNode)	\
	(	&xLIB_VECTOR_GET_ELEM(&(pxTree)->xVector, uiLIB_COMPLETE_BINARY_TREE_GET_INDEX(pxTree, pxNode) / 2)	)

/*
 * Gets pointer to the right child of a given node.
 */
#define pxLIB_COMPLETE_BINARY_TREE_GET_RIGHT_PTR(pxTree, pxParent)	\
	(	&xLIB_VECTOR_GET_ELEM(&(pxTree)->xVector, 2 * uiLIB_COMPLETE_BINARY_TREE_GET_INDEX(pxTree, pxParent) + 1)	)

/*
 * Gets pointer to the last node in a tree.
 */
#define pxLIB_COMPLETE_BINARY_TREE_GET_LAST_PTR(pxTree)	\
	(	&xLIB_VECTOR_GET_ELEM(&(pxTree)->xVector, uiLIB_COMPLETE_BINARY_TREE_GET_SIZE(pxTree) + 1)	)

/*
 * Checks whether a node is the root or not.
 */
#define ucLIB_COMPLETE_BINARY_TREE_IS_ROOT(pxTree, pxNode)	\
	(	uiLIB_COMPLETE_BINARY_TREE_GET_INDEX(pxTree, pxNode) == 1	)

/*
 * Checks whether a node has left child or not.
 */
#define ucLIB_COMPLETE_BINARY_TREE_HAS_LEFT(pxTree, pxParent)	\
	(	2 * uiLIB_COMPLETE_BINARY_TREE_GET_INDEX(pxTree, pxParent) <= uiLIB_COMPLETE_BINARY_TREE_GET_SIZE(pxTree)	)

/*
 * Checks whether a node has right child or not.
 */
#define ucLIB_COMPLETE_BINARY_TREE_HAS_RIGHT(pxTree, pxParent)	\
	(	2 * uiLIB_COMPLETE_BINARY_TREE_GET_INDEX(pxTree, pxParent) + 1 <= uiLIB_COMPLETE_BINARY_TREE_GET_SIZE(pxTree)	)

/*
 * Adds node after last node in the tree.
 */
#define vLIB_COMPLETE_BINARY_TREE_ADD_LAST(pxTree, pxNewElem)	\
	(	vLIB_VECTOR_ADD_BACK(&(pxTree)->xVector, pxNewElem)	)

/*
 * Removes last node of a given tree.
 */
#define vLIB_COMPLETE_BINARY_TREE_REMOVE_LAST(pxTree)	\
	(	vLIB_VECTOR_REMOVE_BACK(&(pxTree)->xVector)	)

/*
 * Swaps contents of two nodes, given their pointers.
 */
#define vLIB_COMPLETE_BINARY_TREE_SWAP(pxNode1, pxNode2)	\
{                                                           \
	xCONF_VECTOR_ELEM_TYPE _xTemp = *pxNode1;               \
	*pxNode1 = *pxNode2;                                    \
	*pxNode2 = _xTemp;                                      \
}







#endif /* COTS_OS_INC_LIB_COMPLETEBINARYTREE_H_ */
