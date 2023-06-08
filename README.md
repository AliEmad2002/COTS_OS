# HAL_OS
FreeRTOS based HAL library.

# Features:
  * Easly ported:
    * As FreeRTOS itself is portable, change its files with these of your target and configure "FreeRTOS_Config.h".
    * MCAL dependancies are ported using "MCAL_Port.h".
  * A lot more time-efficient than baremetal drivers.
  * Uses DMA (if available) for excessive memory using consuming drivers (like SD-card).

# Available drivers:
  * SD-Card (FAT32).
  * Colored-LCD.
  * Keypad.
  * Push buttons.
