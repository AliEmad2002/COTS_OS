/*
 * Heap_Config.h
 *
 *  Created on: Sep 23, 2023
 *      Author: Ali Emad
 */

#ifndef COTS_OS_INC_LIB_HEAP_HEAP_CONFIG_H_
#define COTS_OS_INC_LIB_HEAP_HEAP_CONFIG_H_

/*
 * This macro compares two elements of the type "xCONF_VECTOR_ELEM_TYPE".
 * It evaluates to 1 if "xElem1" considered of more priority to be closer to heap's
 * top than "xElem2". Otherwise, it evaluates to 0.
 */
#define ucCONF_HEAP_COMPARE(xElem1, xElem2)		\
	(	(xElem1) < (xElem2)	)


#endif /* COTS_OS_INC_LIB_HEAP_HEAP_CONFIG_H_ */
