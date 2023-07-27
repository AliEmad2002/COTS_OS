/*
 * RF.c
 *
 *  Created on: Jul 26, 2023
 *      Author: Ali Emad
 */


/*	LIB	*/
#include "stdint.h"

/*	MCAL (ported)	*/
#include "MCAL_Port/Port_Breakpoint.h"
#include "MCAL_Port/Port_DIO.h"

/*	OS	*/
#include "FreeRTOS.h"
#include "task.h"
#include "RTOS_PRI_Config.h"

/*	HAL	*/

/*	SELF	*/
#include "HAL/RF/RF_private.h"
#include "HAL/RF/RF.h"


/*******************************************************************************
 * Task function:
 ******************************************************************************/
static void vRxTask(void* pvParams)		/*	TODO	*/
{
	xHOS_RF_t* pxHandle = (xHOS_RF_t*)pvParams;
}


/*******************************************************************************
 * Physical layer functions:
 ******************************************************************************/
extern void xHOS_RFPhysical_init(xHOS_RF_t* pxHandle);
extern void xHOS_RFPhysical_enable(xHOS_RF_t* pxHandle);
extern void xHOS_RFPhysical_disable(xHOS_RF_t* pxHandle);
extern void xHOS_RFPhysical_startTransmission(xHOS_RF_t* pxHandle);

/*******************************************************************************
 * API functions:
 ******************************************************************************/
/*	See header for info	*/
void vHOS_RF_init(xHOS_RF_t* pxHandle)
{
	/*	Initialize physical layer	*/
	xHOS_RFPhysical_init(pxHandle);

	/*	Initialize task	*/

}

/*	See header for info	*/
void vHOS_RF_enable(xHOS_RF_t* pxHandle)
{
	/*	Enable physical layer	*/
	xHOS_RFPhysical_enable(pxHandle);


}

/*	See header for info	*/
void vHOS_RF_disable(xHOS_RF_t* pxHandle)
{
	/*	Disable physical layer	*/
	xHOS_RFPhysical_disable(pxHandle);
}

/*	See header for info	*/
void vHOS_RF_send(	xHOS_RF_t* pxHandle,
					uint8_t ucDestAddress,
					uint8_t* pucData,
					uint32_t uiDataSizeInBytes,
					uint16_t usCRC	)
{
	/*
	 * Check if TxBuffer is empty. If not, raise the overrun flag and return.
	 */
	if (pxHandle->ucTxEmptyFalg == 1)
	{
		pxHandle->ucTxEmptyFalg = 0;
	}
	else
	{
		pxHandle->ucOverrunFlag = 1;
		return;
	}

	/*	Create pointer to the TxFrame stored in TxShiftRegister	*/
	xHOS_RF_Frame_t* pxTxFrame = (xHOS_RF_Frame_t*)(pxHandle->pucTxShiftRegister);

	/*	Write SOF and EOF to their constant values	*/
	pxTxFrame->ucSOF = ucRF_SOF;
	pxTxFrame->ucEOF = ucRF_EOF;

	/*	Write data to the TxShiftRegister, with the reset of data field cleared	*/
	uint32_t i = 0;

	for (; i < uiDataSizeInBytes; i++)
		pxTxFrame->pucData[i] = pucData[i];

	for (; i < uiRF_DATA_BYTES_PER_FRAME; i++)
			pxTxFrame->pucData[i] = 0;

	/*	Write CRC to the TxShiftRegister	*/
	pxTxFrame->ucCRC0 = usCRC & 0xFF;
	pxTxFrame->ucCRC1 = usCRC >> 8;

	/*	Write SrcAddress and DestAddress to the TxShiftRegister	*/
	pxTxFrame->ucSrcAddress = pxHandle->ucSelfAddress;
	pxTxFrame->ucDestAddress = ucDestAddress;

	/*	This  is not an ACK frame	*/
	pxTxFrame->ucIsAck = 0;

	/*	Start the transmission in physical layer	*/
	xHOS_RFPhysical_startTransmission(pxHandle);
}
























