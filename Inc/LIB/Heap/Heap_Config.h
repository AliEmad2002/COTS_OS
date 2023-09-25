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
 *
 * As this driver is so-far used only in HWTime, it is much faster to do the comparison
 * in a macro than a callback function. Anyways, if this driver is to be used in other
 * code, it may be copied and changed per the new other code, or it may be modified
 * and added a comparison callback function.
 */
#define ucCONF_HEAP_COMPARE(xElem1, xElem2)		\
	(	(xElem1) <= (xElem2)	)


#endif /* COTS_OS_INC_LIB_HEAP_HEAP_CONFIG_H_ */
