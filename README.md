# COTS_OS
FreeRTOS based HAL, LIB layers library.

# Features:
  * Easly ported:
    * As FreeRTOS itself is portable, change its files with these of your target and configure "FreeRTOS_Config.h".
    * MCAL dependancies are ported using files in "Port" folder.
  * A lot more time-efficient (less CPU busy wait load) than baremetal drivers.
  
# Available drivers:
  * SPI (Uses DMA if available).
  * I2C.
  * DMA.
  * Colored-LCD (Uses DMA if available).
  * Binary consecutive N-samples filter (Useful with input digital signals).
  * Push buttons.
  * Seven-segment multiplexer.
  * Rotary encoder.
  * MPU6050 (gyroscope and acceleromter).
  * Stepper motor (with multiple steppers synchronization driver).
  * Ultra-sonic distance sensor (with multiple sensors synchronization driver).
  * RF communication.
  * PID controller.
  
    

# Important notes:
  * Priority of HAL drivers **must** be higher than or equal to other non-hard real time tasks. Also, hard real time tasks **must** not be ready/running all the time. Otherwise, HAL drivers will not work properly. (in other words, HAL_OS drivers are of "normal priority")
  * Object are recommended to be statically allocated (in .bss) (remember: heap==>dynamic, stack==>function, .data==>initialized static/global, .bss==>uninitialized static/global)
  * **Porting notice**: remember that static variables/functions are created each time the file gets included, and non-static definitions in header cause "redefinition error". Therefore, variables (if any) and non-static nor inline functions better be defined in "Port_xxx.c" and externed in "Port_xxx.h". On the other hand, static and inline functions are normally defined in a header "Port_xxx.h".
