/*
 * BinaryFilter.h
 *
 *  Created on: Aug 5, 2023
 *      Author: Ali Emad
 *
 * This is an N-samples based filter for binary values, usually digital signals
 * from input pins.
 */

#ifndef COTS_OS_INC_LIB_BINARYFILTER_BINARYFILTER_H_
#define COTS_OS_INC_LIB_BINARYFILTER_BINARYFILTER_H_


typedef struct{
	/*			PRIVATE			*/
	uint8_t ucCount;
	uint8_t ucPrevLevel : 1;

	/*			PUBLIC			*/
	uint8_t ucLevelFiltered : 1; 		// read-only.
	uint8_t ucPrevLevelFiltered : 1;	// read-only.
	uint8_t ucNFilter;
}xLIB_BinaryFilter_t;


/*
 * Initializes handle
 */
void vLIB_BinaryFilter_init(xLIB_BinaryFilter_t* pxHandle, uint8_t ucInitialState);

/*
 * Updates filter handle with a new sample value.
 */
void vLIB_BinaryFilter_updateFilter(xLIB_BinaryFilter_t* pxHandle, uint8_t ucNewLevel);


#endif /* COTS_OS_INC_LIB_BINARYFILTER_BINARYFILTER_H_ */
