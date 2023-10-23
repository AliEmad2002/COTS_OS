/*
 * String.h
 *
 *  Created on: Sep 15, 2023
 *      Author: Ali Emad
 */

#ifndef COTS_OS_INC_LIB_STRING_H_
#define COTS_OS_INC_LIB_STRING_H_


/*
 * Converts a string represented hex value, to its actual numerical representation.
 *
 * Notes:
 * 		-	"pcStr": is the ASCII string containing the value to be converted.
 * 			Example: "7635E8C5C2" or "7635e8c5c2".
 *
 * 		-	"pucHex": is the byte array where the resultant hex value would be stored.
 *
 * 		-	"uiNumberOfBytes": is the number of bytes to be processed (length of
 * 			"pucHex"). It also equal to length of "pcStr" / 2.
 */
void vLIB_String_str2hex(char* pcStr, uint8_t* pucHex, uint32_t uiNumberOfBytes);

/*
 * Converts an array of bytes to a printable string format (in hexadecimal).
 *
 * Notes:
 * 		-	"pucHex": is the byte array to be converted.
 *
 * 		-	"pcStr": is the string where the resultant hex-printable value would be stored.
 *
 * 		-	"uiNumberOfBytes": is the number of bytes to be processed (length of
 * 			"pucHex"). It also equal to length of "pcStr" / 2.
 */
void vLIB_String_hex2str(uint8_t* pucHex, char* pcStr, uint32_t uiNumberOfBytes);

#endif /* COTS_OS_INC_LIB_STRING_H_ */
