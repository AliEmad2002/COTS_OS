/*
 * Heap.c
 *
 *  Created on: Sep 23, 2023
 *      Author: Ali Emad
 */


#include "stdint.h"

#include "LIB/Heap/Heap.h"


void vLIB_Heap_init(xLIB_Heap_t* pxHeap)
{
	/*	Initialize CompleteBinaryTree handle.	*/
	vLIB_CompleteBinaryTree_init(&pxHeap->xTree);
}

void vLIB_Heap_addElem(xLIB_Heap_t* pxHeap, xCONF_VECTOR_ELEM_TYPE* pxNewElem)
{
	/*	Add new element to end of the tree	*/
	vLIB_COMPLETE_BINARY_TREE_ADD_LAST(&pxHeap->xTree, *pxNewElem);

	/*	Get pointer to the newElem in the tree	*/
	xCONF_VECTOR_ELEM_TYPE* pxNewElemInTree = pxLIB_COMPLETE_BINARY_TREE_GET_LAST_PTR(&pxHeap->xTree);

	xCONF_VECTOR_ELEM_TYPE* pxNewElemParent;

	while(1)
	{
		/*	If the newElem is currently the root, break	*/
		if (ucLIB_COMPLETE_BINARY_TREE_IS_ROOT(&pxHeap->xTree, pxNewElemInTree))
			break;

		/*	Get pointer to parent of the newElem in the tree	*/
		pxNewElemParent = pxLIB_COMPLETE_BINARY_TREE_GET_PARENT_PTR(&pxHeap->xTree, pxNewElemInTree);

		/*
		 * If the newElem is of higher priority to be the root, than its parent,
		 * swap it with its parent.
		 */
		if (ucCONF_HEAP_COMPARE(*pxNewElemInTree, *pxNewElemParent))
		{
			vLIB_COMPLETE_BINARY_TREE_SWAP(pxNewElemInTree, pxNewElemParent);
			pxNewElemInTree = pxNewElemParent;
		}

		/*
		 * Otherwise, if parent is of higher priority, the newElem is hence positioned
		 * in the right place. break.
		 */
		else
			break;
	}
}

void vLIB_Heap_removeTop(xLIB_Heap_t* pxHeap)
{
	/*	Assert if heap is already empty	*/
	vLib_ASSERT(uiLIB_HEAP_GET_SIZE(pxHeap) > 0, 1);

	/*	Get pointer to last element of the tree	*/
	xCONF_VECTOR_ELEM_TYPE* pxLastElem = pxLIB_COMPLETE_BINARY_TREE_GET_LAST_PTR(&pxHeap->xTree);

	/*	Get pointer to root element of the tree	*/
	xCONF_VECTOR_ELEM_TYPE* pxRootElem = pxLIB_COMPLETE_BINARY_TREE_GET_ROOT_PTR(&pxHeap->xTree);

	/*	Swap last element with the root	*/
	vLIB_COMPLETE_BINARY_TREE_SWAP(pxLastElem, pxRootElem);

	/*	Remove last element	*/
	vLIB_COMPLETE_BINARY_TREE_REMOVE_LAST(&pxHeap->xTree);

	/*	If the heap becomes empty, return	*/
	if (uiLIB_HEAP_GET_SIZE(pxHeap) == 0)
		return;

	/*
	 * Otherwise, tree's root is now a floating node, and may not be in its correct
	 * position, the following loop moves it through the tree to assure it is in
	 * its correct position.
	 */
	xCONF_VECTOR_ELEM_TYPE* pxFloatingElem = pxRootElem;

	uint8_t ucHasLeft, ucHasRight;

	xCONF_VECTOR_ELEM_TYPE *pxLeftChild, *pxRightChild;

	xCONF_VECTOR_ELEM_TYPE* pxHigherPriChild;

	while(1)
	{
		/*	Get floating node's left and right children state of existence	*/
		ucHasLeft = ucLIB_COMPLETE_BINARY_TREE_HAS_LEFT(&pxHeap->xTree, pxFloatingElem);
		ucHasRight = ucLIB_COMPLETE_BINARY_TREE_HAS_RIGHT(&pxHeap->xTree, pxFloatingElem);

		/*	If floating node has no children, return	*/
		if (!ucHasLeft && !ucHasRight)
			return;

		/*	Otherwise, get pointer to the left child	*/
		pxLeftChild = pxLIB_COMPLETE_BINARY_TREE_GET_LEFT_PTR(&pxHeap->xTree, pxFloatingElem);

		/*	if floating node has only a left child	*/
		if (ucHasLeft && !ucHasRight)
		{
			/*
			 * Compare floating node with its left child, if floating node is of
			 * higher priority to be the root, then it's  in the right place, return.
			 */
			if (ucCONF_HEAP_COMPARE(*pxFloatingElem, *pxLeftChild))
				return;

			/*	Otherwise, swap them	*/
			else
			{
				vLIB_COMPLETE_BINARY_TREE_SWAP(pxFloatingElem, pxLeftChild);
				pxFloatingElem = pxLeftChild;
			}
		}

		/*	Otherwise, if floating node has both left and right children	*/
		else
		{
			/*	Get pointer to the right child	*/
			pxRightChild = pxLIB_COMPLETE_BINARY_TREE_GET_RIGHT_PTR(&pxHeap->xTree, pxFloatingElem);

			/*
			 * Get pointer to the node of higher priority to be the root, among
			 * left and right children.
			 */
			pxHigherPriChild = ucCONF_HEAP_COMPARE(*pxLeftChild, *pxRightChild) ? pxLeftChild : pxRightChild;

			/*
			 * If floating node is of higher priority to be the root than
			 * "pxHigherPriChild", then it's  in the right place, return.
			 */
			if (ucCONF_HEAP_COMPARE(*pxFloatingElem, *pxHigherPriChild))
				return;

			/*	Otherwise, swap them	*/
			else
			{
				vLIB_COMPLETE_BINARY_TREE_SWAP(pxFloatingElem, pxHigherPriChild);
				pxFloatingElem = pxHigherPriChild;
			}
		}
	}
}









