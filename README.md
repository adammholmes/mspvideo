mspvideo
-------

A simple library written for the purpose of generating monochrome text and 
graphics via composite NTSC on an MSP430G2553 (or similar with enough RAM). 
Using an image bitmap of 384 bytes (48*64 pixels) held in SRAM, one can 
draw simple graphics such as dots, lines, circles, rectangles, text and 
more. The image is generated using a "fake-progressive" NTSC method,
powered by timer triggered interrupts.

usage
-----

```
P1.2 ----- 470 OHM Resistor -----
                                 |---- RCA Video
P2.1 ----- 220 OHM Resistor -----

GND  --------------------------------- RCA GND 
```

notes
-----

In rare cases, a TV set may not include an internal 75 OHM resistor 
from video to ground, so you may need to add this.

This library was written for MSPGCC, modifications may be needed to compile
with CCS or IAR!
