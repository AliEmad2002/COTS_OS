/*
 * CharLcdI2c.h
 *
 * Created: 24/03/2022 08:52:21 م
 *  Author: Ali Emad
 */ 


#ifndef CHARLCDI2C_H_
#define CHARLCDI2C_H_


#define ucHOS_CHAR_LCD_I2C_CURSOR_MODE_CURSOR_OFF_BLINK_OFF		0x0C
#define ucHOS_CHAR_LCD_I2C_CURSOR_MODE_CURSOR_OFF_BLINK_ON		0x0D
#define ucHOS_CHAR_LCD_I2C_CURSOR_MODE_CURSOR_ON_BLINK_OFF		0x0E
#define ucHOS_CHAR_LCD_I2C_CURSOR_MODE_CURSOR_ON_BLINK_ON		0x0F

typedef struct 
{
	/*			PUBLIC		*/
	/*	Number of the I2C unit on which the LCD is connected	*/
	uint8_t ucI2cUnitNumber;

	/*	Address of the LCD I2C interface	*/
	uint8_t ucAddress;

	/*	Power enable pin (used for forced power reset)	*/
	uint8_t ucPowerEnPort;
	uint8_t ucPowerEnPin;

	/*
	 * Mutex of the LCD handle (Should be locked before using and unlocked after,
	 * not using this mutex is user's responsibility).
	 */
	SemaphoreHandle_t xMutex;
	StaticSemaphore_t xMutexStatic;

	/*		PRIVATE		*/
	/*	Value of the bus between I2C IO extender, and the LCD module	*/
	uint8_t ucBusVal;

	/*	Current position of the cursor	*/
	uint8_t ucCursorPos;
}xHOS_CharLcdI2c_t;


void vHOS_CharLcdI2c_init(xHOS_CharLcdI2c_t* pxHandle);

void vHOS_CharLcdI2c_setCursorMode(xHOS_CharLcdI2c_t* pxHandle, uint8_t ucMode);

void vHOS_CharLcdI2c_clearDisplay(xHOS_CharLcdI2c_t* pxHandle);

void vHOS_CharLcdI2c_printChar(xHOS_CharLcdI2c_t* pxHandle, char cCh);

void vHOS_CharLcdI2c_printStr(xHOS_CharLcdI2c_t* pxHandle, char* pcStr);


#endif /* CHARLCDI2C_H_ */
