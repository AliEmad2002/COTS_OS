/*
 * Port_Assert.h
 *
 *  Created on: Jun 23, 2023
 *      Author: ali20
 */

#ifndef PORT_TRY_PORT_ASSERT_H_
#define PORT_TRY_PORT_ASSERT_H_

#include "Lib/Print.h"
#include "MCAL_Port/Port_Breakpoint.h"

#define vLib_ASSERT(exp, errCode)                                         \
{                                                                         \
	if ((exp) == 0)                                                       \
	{                                                                     \
		/*vLIB_PRINT("Assertion failed. Error code: %d\n.", (errCode));*/     \
		volatile uint8_t cont = 0;                                        \
		while(cont == 0)                                                  \
			vPort_Breakpoint();                                           \
	}                                                                     \
}

/*******************************************************************************
 * Port layer error codes:
 ******************************************************************************/
#define usPORT_ERR_SPI_WRONG_BAUDRATE_PRESCALER			0



#endif /* PORT_TRY_PORT_ASSERT_H_ */
