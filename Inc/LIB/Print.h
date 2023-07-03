/*
 * Print.h
 *
 *  Created on: Jun 23, 2023
 *      Author: Ali Emad
 */

#ifndef LIB_PRINT_H_
#define LIB_PRINT_H_

//#include <diag/trace.h>
#include <stdio.h>

/*
#define vLIB_PRINT(...)                                 \
{                                                       \
    char strBuffer[128];                                \
    sprintf(strBuffer, __VA_ARGS__);                    \
    TFT_PrintStr(strBuffer);                            \
}
*/

#define vLIB_PRINT(...)		(printf(__VA_ARGS__))


#endif /* LIB_PRINT_H_ */
