/*
 * Heap.h
 *
 *  Created on: Sep 23, 2023
 *      Author: Ali Emad
 *
 * References:
 * 		-	"Data Structures and Algorithms in C++. Michael Goodrich": 8.3.1 The
 * 			Heap Data Structure.
 */

#ifndef COTS_OS_INC_LIB_HEAP_H_
#define COTS_OS_INC_LIB_HEAP_H_

#include "LIB/CompleteBinaryTree.h"

#include "LIB/Heap/Heap_Config.h"

typedef struct{
	xLIB_CompleteBinaryTree_t xTree;
}xLIB_Heap_t;


/*
 * Initializes heap handle.
 */
void vLIB_Heap_init(xLIB_Heap_t* pxHeap);

/*
 * Mutex locking and unlocking of a heap handle.
 *
 * Notes:
 * 		-	Heap handle should not be used without being locked first.
 *
 * 		-	Heap handle can only be locked and unlocked form inside a task, it
 * 			is not possible from ISR.
 *
 * 		-	Heap handle does not actually have its own mutex, it explicitly locks
 * 			and releases its tree handle.
 */
#define ucLIB_HEAP_LOCK(pxHeap, xTimeout)	\
	(	ucLIB_COMPLETE_BINARY_TREE_LOCK(&(pxHeap)->xTree, (xTimeout))	)

#define vLIB_HEAP_RELEASE(pxHeap)	\
	(	vLIB_COMPLETE_BINARY_TREE_RELEASE(&(pxHeap)->xTree)	)

/*
 * Gets heap's size.
 */
#define uiLIB_HEAP_GET_SIZE(pxHeap)	\
	(	uiLIB_COMPLETE_BINARY_TREE_GET_SIZE(&(pxHeap)->xTree)	)

/*
 * Adds new element.
 *
 * Notes:
 * 		-	This function is of complicity: O(log(n)).
 */
void vLIB_Heap_addElem(xLIB_Heap_t* pxHeap, xCONF_VECTOR_ELEM_TYPE* pxNewElem);

/*
 * Gets pointer to top of the heap.
 *
 * Notes:
 * 		-	This macro is of complicity: O(1).
 */
#define pxLIB_HEAP_GET_TOP_PTR(pxHeap)	\
	(	pxLIB_COMPLETE_BINARY_TREE_GET_ROOT_PTR(&(pxHeap)->xTree)	)

/*
 * Gets value of top of the heap.
 *
 * Notes:
 * 		-	This macro is of complicity: O(1).
 */
#define pxLIB_HEAP_GET_TOP_VAL(pxHeap)	\
	(	*pxLIB_HEAP_GET_TOP_PTR(pxHeap)	)

/*
 * Removes top of the heap.
 *
 * Notes:
 * 		-	This function is of complicity: O(log(n)).
 */
void vLIB_Heap_removeTop(xLIB_Heap_t* pxHeap);




















#endif /* COTS_OS_INC_LIB_HEAP_H_ */
