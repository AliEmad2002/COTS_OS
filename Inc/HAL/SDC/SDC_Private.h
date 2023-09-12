/*
 * SDC_Private.h
 *
 *  Created on: Apr 28, 2023
 *      Author: ali20
 */

#ifndef INCLUDE_HAL_SDC_SDC_PRIVATE_H_
#define INCLUDE_HAL_SDC_SDC_PRIVATE_H_

/*
 * SD-Card's OCR register.
 * (defined in p161 of the doc: Physical Layer Simplified Specification Version 6.00)
 */
typedef struct{
	uint32_t uiVddVoltageWindow : 24;
	uint32_t uiSwitchAccepted   : 1;
	uint32_t uiReserved         : 4;
	uint32_t uiUhs              : 1;
	uint32_t uiCcs              : 1;
	uint32_t uiBusy             : 1;
}SDC_OCR_t;

/*
 * Card interface condition
 */
typedef struct{
	uint8_t ucCheckPattern     : 8;
	uint8_t ucVoltageAccepted  : 4;
	uint8_t ucReserved0        : 4;
	uint8_t ucReserved1        : 4;
	uint8_t ucReserved2        : 4;
	uint8_t ucReserved3        : 4;
	uint8_t ucCommandVersion   : 4;
}SDC_CIC_t;

/*	Responses	*/
typedef struct{
	uint8_t ucInIdleState   : 1;
	uint8_t ucEraseRst      : 1;
	uint8_t ucIlligalCmdErr : 1;
	uint8_t ucCmdCrcErr     : 1;
	uint8_t ucEraseSeqErr   : 1;
	uint8_t ucAddressErr    : 1;
	uint8_t ucParameterErr  : 1;
	uint8_t ucStartBit      : 1;		// always received 0
}SDC_R1_t;

typedef struct{

}SDC_R2_t;

typedef struct{
	SDC_OCR_t xOcr;
	SDC_R1_t xR1;
}SDC_R3_t;

typedef struct{
	SDC_CIC_t xCic;
	SDC_R1_t xR1;
}SDC_R7_t;

typedef struct{
	uint8_t ucStartBit : 1;
	uint8_t ucStatus   : 3;
	uint8_t ucEndBit   : 1;
}SDC_Data_Response_t;

typedef enum{
	SDC_Data_Response_Status_Accepted = 0b010,
	SDC_Data_Response_Status_CrcErr   = 0b101,
	SDC_Data_Response_Status_WrtErr   = 0b110
}SDC_Data_Response_Status_t;

typedef struct{
	uint8_t ucBootFlag;
	uint32_t uiChsBegin : 24;
	uint8_t ucTypeCode;
	uint32_t uiChsEnd : 24;
	uint32_t uiLbaBegin;
	uint32_t uiNumberOfSectors;
}SDC_Partition_Entry_t;

typedef struct{
	uint8_t ucReadOnly  : 1;
	uint8_t ucHidden    : 1;
	uint8_t ucSystem    : 1;
	uint8_t ucVolumeId  : 1;
	uint8_t ucDirectory : 1;
	uint8_t ucArchive   : 1;
	uint8_t ucUnused0   : 1;
	uint8_t ucUnused1   : 1;
}SDC_DirAttrib_t;

typedef struct{
	char pcShortFileName[11];
	SDC_DirAttrib_t xAttrib;
	uint32_t uiReserved0[2];
	uint16_t usFirstClusterHigh;
	uint16_t usReserved1;
	uint16_t usReserved2;
	uint16_t usFirstClusterLow;
	uint32_t uiFileSize;
}SDC_DirData_t;

typedef enum{
	SDC_DirRecordType_Normal,
	SDC_DirRecordType_LongFileName,
	SDC_DirRecordType_Unused,
	SDC_DirRecordType_EndOfDir,
	SDC_DirRecordType_Unknown
}SDC_DirRecordType_t;

typedef struct{
	uint32_t uiIndexInSector : 7;  // Index of the 32-bit integer in the sector (described next),
								// which contain next cluster number.

	uint32_t uiSectorNumber  : 25; // Number of the sector of the File Allocation
								// Table (FAT) in which next cluster number is written.
}SDC_NextClusterEntry_t;












#endif /* INCLUDE_HAL_SDC_SDC_PRIVATE_H_ */
