# COTS_OS
FreeRTOS based HAL, LIB layers library.

# Features:
  * Easly ported:
    * As FreeRTOS itself is portable, change its files with these of your target and configure "FreeRTOS_Config.h".
    * MCAL dependancies are ported using files in "Port" folder.
  * A lot more time-efficient (less CPU load) than baremetal drivers.
  * Uses DMA (if available) with drivers which do long memory load/store operations (like: SD-card, WiFi module, colored-LCD).

# Available drivers:
  * SD-Card (FAT32).
  * ESP8266 WiFi module.
  * Colored-LCD.
  * Keypad.
  * SPI
    * Has multiple versions for multiple application purposes. See SPI driver documentation: https://github.com/AliEmad2002/HAL_OS/blob/main/Doc/SPI%20Driver%20documentation.pdf
    * Has a DMA accelerated version for targets which have DMA.
  * Push buttons
    * Uses N-samples filtering method.
    * Example available at: https://github.com/AliEmad2002/HAL_OS/blob/main/examples/Button_example_1.c
       
       https://github.com/AliEmad2002/HAL_OS/assets/99054912/809e45ac-fcf3-4692-9f22-9152126abfa4
  * Seven-segment multiplexer
    * Example available at: https://github.com/AliEmad2002/HAL_OS/blob/main/examples/Button_And_SevenSegmentMux_example.c
    

      https://github.com/AliEmad2002/HAL_OS/assets/99054912/e4ce0098-8968-4b07-a7b3-8f55cbbe2e6d
  * Rotary encoder
    * Uses N-samples filtering method.
    * Example available at: https://github.com/AliEmad2002/COTS_OS/blob/main/examples/RotaryEncoder_And_SevenSegmentMux_example.c
      

      https://github.com/AliEmad2002/COTS_OS/assets/99054912/9719c2fc-1fef-4b5e-89d4-3d95f3119ea5




    

# Important notes:
  * Priority of HAL drivers **must** be higher than or equal to other non-hard real time tasks. Also, hard real time tasks **must** not be ready/running all the time. Otherwise, HAL drivers will not work properly. (in other words, HAL_OS drivers are of "normal priority")
  * Object are recommended to be statically allocated (in .bss) (remember: heap==>dynamic, stack==>function, .data==>initialized static/global, .bss==>uninitialized static/global)
  * **Porting notice**: remember that static variables/functions are created each time the file gets included, and non-static definitions in header cause "redefinition error". Therefore, variables (if any) and non-static nor inline functions better be defined in "Port_xxx.c" and externed in "Port_xxx.h". On the other hand, static and inline functions are normally defined in a header "Port_xxx.h".
