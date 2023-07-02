# AKM320
MIDIPLUS AKM320 - MIDI 2.0 Research Project
## About
This is a simple project to learn (or attempt to) about MIDI 2.0. Goal was to learn on an actual product and not on some development board that is useless to me at the end. I chose the AKM320 as it is cheap and well designed. This code was pounded out fairly quickly on nights/weekends, I wouldn't consider it ready for prime-time.

There are NO current MIDI 2.0 tools to validate against that is open the public so I have been watching the [Microsoft MIDI](https://github.com/microsoft/MIDI) releases closely. When they are far enough with development, I'll start to test!

This project supports two different builds (sorry, not dynamically switchable), MIDI 1.0 and MIDI 2.0.
## Tools
(only built on Windows thus far)
1. [STM32CubeIDE](https://www.st.com/en/development-tools/stm32cubeide.html)
2. Hardware debugger (you choose)
   1. [J-Link EDU](https://www.segger.com/products/debug-probes/j-link/models/j-link-edu/)
   2. [ST-Link](https://www.st.com/en/development-tools/stlink-v3set.html)
3. 100mil single row pin header (4 pin & 2 pin)
4. Female-female jumper wires to connect main PCB to J-Link
5. Solder iron
## Prep
1. I am not going to go into how to take it apart, grab a screwdriver and figure it out. Open it up far enough to access the main PCB. Proper grounding is advised, ESD sucks.
2. Solder headers to the main PCB
   1. Carefully remove the main PCB (4 screws)
   2. Locate CN2 and solder the pin header to the top
   3. Locate CN3 and solder the pin header to the top
   4. Carefully replace the main PCB
3. Wire up the programmer using the female-female jumper wires.
   1. CN2 pin 1 (GND)
   2. CN2 pin 2 (SWDIO)
   3. CN2 pin 3 (SWCLK)
   4. CN2 pin 4 (RESET)
   5. CN3 pin 1 (VCC)
4. The AKM320 is bus powered, when ready, plug it into a USB port.

![pins](https://github.com/matthewwittenberg/AKM320/blob/main/images/pins.jpeg?raw=true)
## Building
1. The AKM320 is built on an [STM32F103C8](https://www.st.com/en/microcontrollers-microprocessors/stm32f103c8.html) microcontroller. Other than being slightly cramped from a code space standpoint, it is a workhorse.
2. **The folks at MIDIPLUS protected the code flash so there is no readout capability to ever restore what was originally programmed.**
3. Building
   1. Open the project in STM32CUBE
   2. Right mouse-click the project and select "properties"
   3. Browse to "C/C++ Build" -> "Settings"
   4. Assign the MIDI version to build to in "MCU GCC Compiler" -> "Preprocessor" (1 for MIDI 1.0, 2 for MIDI 2.0)
   5. Assign the MIDI version to build to in "MCU G++ Compiler" -> "Preprocessor" (1 for MIDI 1.0, 2 for MIDI 2.0)
   6. "Apply and Close"
   7. At the top menu bar select "Project" -> "Build All"
4. Running (first time)
   1. At the top menu bar select "Run" -> "Debug Configurations"
   2. Create a new "STM32 C/C++ Application"
   3. Under "Debugger", select your hardware programmer and setup as needed
   4. Select "Debug" to start debugging
## Misc
1. As far as I know, all MIDI 1.0 functionality is the same as the original product, if anything was missed it is easy to add (it's just code!).  MIDI 2.0 functionality has NOT been tested other than through some development board code I wrote.
2. Each key on the keyboard was designed with a 2 stage micro-switch. Just barely pressing the key registers the first actuation, continuing the press motion activates the second stage. Noting the delta time between the actuations represents the key velocity (loudness). The circuit is multiplexed so we need to scan at a high rate.
3. [FreeRTOS](https://www.freertos.org/) was used because it makes sense.
4. DO NOT run the application builder as I modified drivers and startup code, they will be overwritten.
5. Commenting is seriously lacking but will be added as I have time.
## Code
1. The meat of the code is in Core/Source.
2. main_app.c owns the main task, it processes keypad presses, ADC changes and proceses MIDI traffic.
3. keyboard.c owns the keyboard task, it scans and processes key presses.
4. usbd_midi_X0_app.c builds and processes MIDI USB messages depending on the version. 

## Peripheral Map
To the best of my knowledge, here is the map of microcontroller pins

| pin | pin name | function |
| --- | --- | --- |
|1|VBAT| |
|2|PC13|button octave plus|
|3|PC14|button octave minus|
|4|PC15|button transpose plus|
|5|OSC_IN|8MHz crystal|
|6|OSC_OUT|8MHz crystal|
|7|NRST|RESET|
|8|VSSA|GND|
|9|VDDA|3.3VA|
|10|PA0| |
|11|PA1| |
|12|PA2| |
|13|PA3| |
|14|PA4|LED 1|
|15|PA5|LED 2|
|16|PA6|LED 3|
|17|PA7|LED 4|
|18|PB0|keyboard group 1|
|19|PB1|keyboard group 2|
|20|PB2|keyboard group 3|
|21|PB10|keyboard key 2 down|
|22|PB11|keyboard key 2 detect|
|23|VSS1|GND|
|24|VDD1|3.3V|
|25|PB12|keyboard key 3 down|
|26|PB13|keyboard key 3 detect|
|27|PB14|keyboard key 4 down|
|28|PB15|keyboard key 4 detect|
|29|PA8|USB signal|
|30|PA9| |
|31|PA10| |
|32|PA11|USBD-|
|33|PA12|USBD+|
|34|PA13|SWDIO|
|35|VSS2|GND|
|36|VDD2|3.3V|
|37|JTCK/PA14|SWCLK|
|38|JTDI/PA15|button transpose minus|
|39|JTDO/PB3|keyboard group 4|
|40|JNTRST/PB4|keyboard group 5|
|41|PB5|keyboard group 6|
|42|PB6|keyboard group 7|
|43|PB7|keyboard group 8|
|44|BOOT0| |
|45|PB8|keyboard key 1 down|
|46|PB9|keyboard key 1 detect|
|47|VSS3|GND|
|48|VDD3|3.3V|

