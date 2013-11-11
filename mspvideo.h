/* mspvideo.c:
 *
 * A simple library written for the purpose of generating monochrome text and 
 * graphics via composite NTSC on an MSP430G2553 (or similar with enough RAM). 
 * Using an image bitmap of 384 bytes (48*64 pixels) held in SRAM, one can 
 * draw simple graphics such as dots, lines, circles, rectangles, text and 
 * more. The image is generated using a "fake-progressive" NTSC method,
 * powered by timer triggered interrupts.
 *
 *
 * Usage:
 *
 * P1.2 ----- 470 OHM Resistor -----
 *                                  |---- RCA Video
 * P2.1 ----- 220 OHM Resistor -----
 *
 * GND  --------------------------------- RCA GND 
 *
 *
 * Author: Adam M. Holmes
 */


#ifndef MSPVIDEO_H
#define MSPVIDEO_H

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <msp430.h>
#include <legacymsp430.h>
#include "fonts/font8x8.h"
#include "fonts/font4x6.h"


/* Initialize register settings, CALL THIS FIRST */
inline void initialize ();

/* Print a 4 * 6 pixel string at the given coordinates */
void printStringSmall (int x, int y, char s[]);

/* Print an 8 * 8 pixel string at the given coordinates */
void printStringLarge (int x, int y, char s[]);

/* Return zero if the pixel is black, 1 if white */
int getPixel (int x, int y);

/* Given two points and a color, draw a pixel */
void drawPixel (int x, int y, int c);

/* Given two points and a color, draw a line segment */
void drawLine (int x1, int y1, int x2, int y2, int c);

/* Given two points, a radius, and a color: draw a circle */
void drawCircle (int x, int y, int r, int c);

/* Given x and y, width and height, and a color: draw a rectangle */
void drawRect (int x1, int y1, int x2, int y2, int c);

/* Resets every pixel in the image to 0 (black) */
void clearScreen ();

/* Arbitrary delay function, useful for animation */
void delay( int t);


#endif
