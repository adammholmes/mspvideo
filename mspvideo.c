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

#include "mspvideo.h"


/* Horizontal line width */
#define HWIDTH 1018
/* Horizontal sync pulse width */
#define HSYNC 75
/* Vertical sync pulse width */
#define VSYNC 942
/* Luminance output delay */
#define PICTUREDELAY 175

/* Image array */
char image[48][8];
/* Current line of output */
unsigned int line = 0;


void 
initialize ()
{
  WDTCTL = WDTPW + WDTHOLD;               // Stop watchdog timer

  BCSCTL1 = CALBC1_16MHZ;                 // Set clock to 16MHz
  DCOCTL = CALDCO_16MHZ;
    
  /* P1 Control */
  P1DIR  |= 0b00000100;                   // Toggle P1.2 for luminance output
  P1OUT  |= 0b00000000;
  P1SEL  |= 0b00000100;
  P1SEL2 |= 0b00000100;
 
  /* P2 Control */
  P2DIR |= 0b00000010;                    // Toggle P2.1 for sync output
  P2OUT |= 0b00000000;
  P2SEL |= 0b00000010;
    
  /* SPI Master Out */
  UCA0CTL0 |= UCCKPH + UCMST + UCSYNC;    // Phase, Master, 8-bit
  UCA0CTL1 |= UCSSEL_2;                   // Use 16MHz clock rate
  UCA0BR0 = 11;                           // Slow output to fit screen
  UCA0CTL1 &= ~UCSWRST;                   // Software reset
    
  /* TIMER_A1 */
  TA1CCR0 = HWIDTH;                       // Each line is 63.625us
  TA1CCR1 = HSYNC;                        // Horizontal Sync (low) for 5us
  TA1CCR2 = PICTUREDELAY;                 // Luminance output delay
  TA1CCTL0 = CCIE;                        // Interrupt after every line
  TA1CCTL1 = OUTMOD_3;                    // PWM output (set/reset) for sync
  TA1CCTL2 = 0;                           // Luminance output ISR disabled
  TA1CTL = TASSEL_2 + ID_0 + MC_1;        // Use DCO, no division, up mode
    
  __bis_SR_register(GIE);                 // Start interrupts
}


void 
printStringSmall (int x, int y, char s[])
{
  int i = 0;
  int c = (x)/4;
  int l = (x)/4;
  char *letter = &s[0];
    
  while (*letter && c < 16) {
    // Is this character on the left?
    if ((l & 1) == 0 || l == 0) {       
      for (i = 0; i < 6; i++) {
        // Set high nibble as character
        image[((y) + i)][c] |= 
        chars4x6[*letter-1][i];
      }
    } else {
      for (i = 0; i < 6; i++) {
        // Set low nibble as character
        image[((y) + i)][c] |= 
        chars4x6[*letter-1][i]*16;  
      }
      c++;
    }
    letter++;
    l++;
  }
}


void 
printStringLarge (int x, int y, char s[])
{
  int i = 0;
  int c = x / 8;
  char *letter = &s[0];
  
  // Output each character
  while (*letter && c < 8) {
    // Each character (large font) has an 8 pixel height          
    for (i = 0; i < 8; i++) {           
      image[((y) + i)][c] = chars8x8[*letter-1][i];
    }
    c++;
    letter++;
  }
}


int 
getPixel (int x, int y)
{
  if (x < 63 && y < 48) {
    // Determine nth bit to check
    int bit = 8 - ((x) % 8);
    // Is the bit set?           
    if ((1 << bit) & image[y][x/8]) {   
      return 1;
    } else {
      return 0;
    }
  } else {
    return 0;
  }
}


void 
drawPixel (int x, int y, int c) 
{
  if (x < 63 && y < 48) {
    // Bit offset of target byte
    int bitOffset = (x) % 8;            
    int byte = 1;
    
    // 2^(bitoffset) = byte with bit set
    while (bitOffset) {                 
      byte *= 2;
      bitOffset--;
    }
        
    if (c > 0) {
      // Set bit to 1 (white)
      image[y][x/8] |= byte;          
    } else {
      // Reset bit to 0 (black)
      image[y][x/8] ^= byte;          
    }
  }
}


void 
drawLine (int x1, int y1, int x2, int y2, int c)
{
  int dx = abs(x2 - x1);
  int dy = abs(y2 - y1);
    
  int error = dx - dy;
  int error2 = error * 2;
    
  int sx = -1;
  int sy = -1;
    
  if (x1 < x2) {
    sx = 1;
  }
  if (y1 < y2) {
    sy = 1;
  }
    
  while ( !((x1 == x2) && (y1 == y2)) ) {
    drawPixel(x1, y1, c);
    error2 = error * 2;
    if (error2 > -dy) {
      error = error - dy;
      x1 += sx;
    }
    if (error2 < dx) {
      error = error + dx;
      y1 += sy;
    }
  }
}


void 
drawCircle (int x, int y, int r, int c)
{
  int err = 1 - r;
  int ddfx = 1;
  int ddfy = -2 * r;
  int x0 = 0;
  int y0 = r;
    
  drawPixel(x, y + r, c);
  drawPixel(x, y - r, c);
  drawPixel(x + r, y, c);
  drawPixel(x - r, y, c);
    
  while (x0 < y0) {
    
    if (err >= 0) {
      y0--;
      ddfy += 2;
      err += ddfy;
    }
    
    x0++;
    ddfx += 2;
    err += ddfx;
        
    drawPixel(x + x0, y + y0, c);
    drawPixel(x - x0, y - y0, c);
    drawPixel(x + x0, y - y0, c);
    drawPixel(x - x0, y + y0, c);
    drawPixel(x + y0, y + x0, c);
    drawPixel(x - y0, y - x0, c);
    drawPixel(x + y0, y - x0, c);
    drawPixel(x - y0, y + x0, c);
  }
}


void 
drawRect (int x, int y, int w, int h, int c)
{
  drawLine(x,y,x+w,y,c);
  drawLine(x+w,y,x+w,y+h,c);
  drawLine(x+w,y+h,x,y+h,c);
  drawLine(x,y+h,x,y,c);
}


void 
clearScreen ()
{
  memset(image, 0, sizeof(image));
}


void 
delay (int t)
{
  while (t--) {
    __delay_cycles(8000);
  }
}


/* 
 * Controls pulse values for NTSC signal generation 
 */

interrupt (TIMER1_A0_VECTOR) 
TA1CCR0_ISR (void)
{
  line++;
  switch (line) {
    case 25:
      __bis_SR_register_on_exit(LPM0_bits);
      break;
    case 29: 
      // Start displaying picture
      __delay_cycles(128);
      TA1CCTL2 = CCIE;                
      break;
    case 221: 
      // Stop displaying picture
      TA1CCTL2 = 0;                   
      __bic_SR_register_on_exit(LPM0_bits);
      break;
    case 247: 
      // Start vertical sync
      TA1CCR1 = VSYNC;                
      break;
    case 249:
      // Stop vertical sync
      TA1CCR1 = HSYNC;                
      break;
    case 261: 
      // Start new frame
      line = 0;                       
      break;
  }
}


/* 
 * Sends array byte values to SPI buffer 
 */

interrupt (TIMER1_A1_VECTOR) 
TA1CCR1_ISR (void)
{
  // Draw each line four times
  unsigned char y = ((line-29) / 4);      

  // Start pushing image to buffer
  UCA0TXBUF = image[y][0];
  while (!(IFG2 & UCA0TXIFG));
  UCA0TXBUF = image[y][1];
  while (!(IFG2 & UCA0TXIFG));
  UCA0TXBUF = image[y][2];
  while (!(IFG2 & UCA0TXIFG));
  UCA0TXBUF = image[y][3];
  while (!(IFG2 & UCA0TXIFG));
  UCA0TXBUF = image[y][4];
  while (!(IFG2 & UCA0TXIFG));
  UCA0TXBUF = image[y][5];
  while (!(IFG2 & UCA0TXIFG));
  UCA0TXBUF = image[y][6];
  while (!(IFG2 & UCA0TXIFG));
  UCA0TXBUF = image[y][7];
  
  // Reset state machine
  UCA0CTL1 &= ~UCSWRST;                   
  
  // Clear interrupt flag
  TA1IV;                                  
}
