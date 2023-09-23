/*
 * CompleteBinaryTree.c
 *
 *  Created on: Sep 23, 2023
 *      Author: Ali Emad
 */


#include "stdint.h"

#include "LIB/CompleteBinaryTree.h"


void vLIB_CompleteBinaryTree_init(xLIB_CompleteBinaryTree_t* pxTree)
{
	/*	Initialize tree's vector	*/
	vLIB_Vector_init(&pxTree->xVector);

	/*
	 * First element of the vector is just a place holder.
	 * TODO: turn around the whole implementation to make use of this 1st element.
	 */
	vLIB_VECTOR_ADD_BACK(&pxTree->xVector, (xCONF_VECTOR_ELEM_TYPE){0});
}
