#Agon Electron HAL
The AgonLight is an amazing device combining retro vibes with a modern programming environment.

The Agon Electron Hardware Abstraction Layer will provide virtual hardware abstractions in the ESP32 'governor' that the EZ80 retro chip can talk to.

Although there is an excellent and more complete alternative called Quark Agon-VDP by Dean Benfield, I created my own attempt to something similar called Electron HAL. Because Quarks and Electrons together make (things that) Matter.

#Features
Currently this HAL has the following features:
- Host<->ESP32 terminal interface, bridging to EZ80
- ESP32<->EZ80 terminal interface connecting to Agon-MOS and other future EZ80 os-es like CP/M and MSX-DOS.
- EZ80 ZDI remote debugging
    - EZ80 status
    - EZ80 register contents
    - 4 HW breakpoints
    - Break/Continue/Run
- EZ80 ZDI memory access
    - load
    - save

