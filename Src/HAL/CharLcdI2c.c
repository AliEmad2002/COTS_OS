 /*
 * CharLcdI2c.c
 *
 * Created: 24/03/2022 08:52:07 م
 *  Author: Ali Emad
 */

/*	LIB	*/
#include "stdint.h"

/*	MCAL (Ported)	*/


/*	RTOS	*/
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/*	COTS-OS	*/
#include "HAL/I2C/I2C.h"

/*	SELF	*/
#include "HAL/CharLcdI2c/CharLcdI2c.h"

/*******************************************************************************
 * Helping structures:
 ******************************************************************************/
typedef struct{
	uint8_t ucRs : 1;
	uint8_t ucRw : 1;
	uint8_t ucEn : 1;
	uint8_t ucLedEn : 1;
	uint8_t ucHByte : 4;
}xBusVal_t;

/*******************************************************************************
 * Static (Private) functions:
 ******************************************************************************/
/*
 * Writes value of the bus connecting I2C-IO extender and the LCD module by the
 * value stored in "pxHandle->ucBusVal".
 */
static void vWriteBus(xHOS_CharLcdI2c_t* pxHandle)
{
	xHOS_I2C_transreceiveParams_t xParams = {
		.ucUnitNumber = pxHandle->ucI2cUnitNumber,
		.usAddress = pxHandle->ucAddress,
		.ucIs7BitAddress = 1,
		.pucTxArr = &pxHandle->ucBusVal,
		.uiTxSize = 1,
		.uiRxSize = 0
	};

	ucHOS_I2C_masterTransReceive(&xParams);
}

static void vLatch(xHOS_CharLcdI2c_t* pxHandle)
{
	xBusVal_t* pcBusVal = (xBusVal_t*)(&pxHandle->ucBusVal);

	/*	LCD module latches by a falling edge on the EN pin	*/

	/*	En high	*/
	pcBusVal->ucEn = 1;
	vWriteBus(pxHandle);

	/*	Settle delay	*/
	vTaskDelay(pdMS_TO_TICKS(1));

	/*	En low	*/
	pcBusVal->ucEn = 0;
	vWriteBus(pxHandle);
}

static void vSendByteAndLatch(xHOS_CharLcdI2c_t* pxHandle, uint8_t ucByte)
{
	xBusVal_t* pcBusVal = (xBusVal_t*)(&pxHandle->ucBusVal);

	/*	Select write mode (RW low)	*/
	pcBusVal->ucRw = 0;
	vWriteBus(pxHandle);

	/*	Write higher 4-bits	*/
	pcBusVal->ucHByte = ucByte >> 4;
	vWriteBus(pxHandle);
	vLatch(pxHandle);

	/*	Write lower 4-bits	*/
	pcBusVal->ucHByte = ucByte & 0x0F;
	vWriteBus(pxHandle);
	vLatch(pxHandle);
}

static void vSendCmd(xHOS_CharLcdI2c_t* pxHandle, uint8_t ucCmd)
{
	xBusVal_t* pcBusVal = (xBusVal_t*)(&pxHandle->ucBusVal);

	/*	Select the command register of the LCD-module (RS low)	*/
	pcBusVal->ucRs = 0;
	vWriteBus(pxHandle);

	/*	Write and latch CMD */
	vSendByteAndLatch(pxHandle, ucCmd);

	/*	Wait for execution to complete	*/
	vTaskDelay(pdMS_TO_TICKS(3));
}



/*******************************************************************************
 * API functions:
 ******************************************************************************/
void vHOS_CharLcdI2c_init(xHOS_CharLcdI2c_t* pxHandle)
{
	/*	Bus is initially all ones	*/
	pxHandle->ucBusVal = 255;
	vWriteBus(pxHandle);

	/*	Initialize LCD module in 4-bit mode	*/
	vSendCmd(pxHandle, 0x02);
	vSendCmd(pxHandle, 0x28);

	/*	Clear display	*/
	vHOS_CharLcdI2c_clearDisplay(pxHandle);
}

/*
 * See header for info.
 */
inline void vHOS_CharLcdI2c_setCursorMode(xHOS_CharLcdI2c_t* pxHandle, uint8_t ucMode)
{
	vSendCmd(pxHandle, ucMode);
}

/*
 * See header for info.
 */
void vHOS_CharLcdI2c_clearDisplay(xHOS_CharLcdI2c_t* pxHandle)
{
	/*	Clear	*/
	vSendCmd(pxHandle, 0x01);

	/*	Reset cursor's position	*/
	vSendCmd(pxHandle, 0x80);
	pxHandle->ucCursorPos = 0;

	/*	Here, execution time is more that other instructions	*/
	vTaskDelay(pdMS_TO_TICKS(1));
}

/*
 * See header for info.
 */
void vHOS_CharLcdI2c_printChar(xHOS_CharLcdI2c_t* pxHandle, char cCh)
{
	/*	Check if cursor needs to be shifted in row (LCD module won't do it on its own)	*/
	if (pxHandle->ucCursorPos == 4*20)
	{
		vHOS_CharLcdI2c_clearDisplay(pxHandle);
	}

	else if (pxHandle->ucCursorPos%20 == 0)
	{
		/*	Force the cursor to the beginning of the next line	*/
		uint8_t ucAddress;
		uint8_t ucLine = pxHandle->ucCursorPos / 20;

		if (ucLine == 0)
			ucAddress = 0x80 + pxHandle->ucCursorPos;
		else if (ucLine == 1)
			ucAddress = 0x80 + pxHandle->ucCursorPos%20 + 64;
		else if (ucLine == 2)
			ucAddress = 0x80 + pxHandle->ucCursorPos%20 + 20;
		else if (ucLine == 3)
			ucAddress = 0x80 + pxHandle->ucCursorPos%20 + 84;

		vSendCmd(pxHandle, ucAddress);
	}
	
	/*	If the byte is end-line byte	*/
	if (cCh == '\n')
	{
		pxHandle->ucCursorPos += 20;
		pxHandle->ucCursorPos = (pxHandle->ucCursorPos / 20) * 20;
	}

	/*	Otherwise	*/
	else
	{
		xBusVal_t* pcBusVal = (xBusVal_t*)(&pxHandle->ucBusVal);

		pxHandle->ucCursorPos++;
	
		/*	Select the data register of the LCD-module (RS high)	*/
		pcBusVal->ucRs = 1;
		vWriteBus(pxHandle);
	
		/*	Write data	*/
		vSendByteAndLatch(pxHandle, (uint8_t)cCh);
	}
}

/*
 * See header for info.
 */
void HLCD_voidPrintStr(xHOS_CharLcdI2c_t* pxHandle, char* pcStr)
{
	for (uint32_t i = 0; pcStr[i] != '\0'; i++)
	{
		vHOS_CharLcdI2c_printChar(pxHandle, pcStr[i]) ;
	}
}
