// Definition of interrupt names
#include < avr/io.h >
// ISR interrupt service routine
#include < avr/interrupt.h >

#define VSYNC_CLOCK_ON ((3 << WGM12) | (6 << CS10))
#define VSYNC_CLOCK_OFF (3 << WGM12)
#define HSYNC_CLOCK_ON ((3 << WGM32) | (1 << CS30))
#define HSYNC_CLOCK_OFF (3 << WGM32)

#define PORT_VSYNC PORTB
#define DDR_VSYNC DDRB
#define VSYNC_BIT (1 << PORTB5)

#define PORT_HSYNC PORTC
#define DDR_HSYNC DDRC
#define HSYNC_BIT (1 << PORTC6)

#define PORT_COLOUR PORTF
#define DDR_COLOUR DDRF
#define COLOUR(R, G, B) ((R << PORTF5) | (G << PORTF6) | (B << PORTF7))

volatile unsigned char scanline_num;
#define NUM_SCANLINES 200

volatile unsigned char frame_num;


void setup() {
  char cSREG;
  cSREG = SREG;
  noInterrupts();

  // modeline 320x200_60,00Hz_15.7kHz 6.162 320 328 357 392 200 221 224 262 -hsync -vsync

  //////////////////
  // Configure HSYNC
  //////////////////
  // WGM3 = 14 -> Fast PWM, up to ICR
  // COM3A = 3 -> -hsync
  // COM3B = 0 -> GNDN
  // COM3C = 0 -> GNDN
  // CS3 = 1 -> /1
  //
  // WGM31:0 = 2 (bottom of 14)
  TCCR3A = (3 << COM3A0) | (2 << WGM30);
  // WGM33:2 = 3 (top of 14)
  TCCR3B = HSYNC_CLOCK_OFF;
  // Start counter at 0
  TCNT3 = 0;
  // HSYNC clock is 16000000Hz / 1 = 16MHz
  // 16MHz / 6.162MHz = 2.6 system clock to pixel clock multiplier
  // 392 * 2.6 ~ 1018
  ICR3 = 1018;
  // (357 - 328) * 2.6 ~ 75
  OCR3A = 75;
  // HSYNC + HBackPorch
  // (392 - 328) * 2.6 ~ 166
  OCR3B = 166;
  // Listen for the end of the back porch
  TIFR3 = 0;
  TIMSK3 = (1 << OCIE3B);
  // OC3A => PC6 => pin 5
  // Start with HSYNC not in progress.
  // PORTC7 is the LED
  PORT_HSYNC = HSYNC_BIT;
  DDR_HSYNC = HSYNC_BIT | (1 << PORTC7);


  //////////////////
  // Configure VSYNC
  //////////////////
  // WGM1 = 14 -> Fast PWM up to ICR
  // COM1A = 3 -> -vsync
  // COM1B = 0 -> GNDN
  // COM1C = 0 -> GNDN
  // CS1 = 6 -> external Tn pin, falling edge
  //
  // WGM11:0 = 2 (bottom of 14)
  TCCR1A = (3 << COM1A0) | (2 << WGM10);
  // WGM13:2 = 3 (top of 14)
  TCCR1B = VSYNC_CLOCK_ON;
  // Start counter at 0
  TCNT1 = 0;
  // VSYNC clock is row clock, straight from the modeline
  // IRC1 counts _inclusive_, and we start at 0, so 262 - 1 = 261.
  ICR1 = 261;
  // 224 - 221 = 3 lines
  OCR1A = 3;
  // VSYNC + VBackPorch
  // 262 - 221 = 41 lines
  OCR1B = 41;
  // Listen for the end of the back porch
  TIFR1 = 0;
  TIMSK1 = (1 << OCIE1B);
  // OC1A => PB5 => pin 9
  // Start with VSYNC not in progress.
  PORT_VSYNC = VSYNC_BIT;
  DDR_VSYNC = VSYNC_BIT;

  // Enable RGB
  PORT_COLOUR = COLOUR(0, 0, 0);
  DDR_COLOUR = COLOUR(1, 1, 1);

  // Enable the VSYNC!
  frame_num = 0;
  PORTC = 0;
  TCCR1B = HSYNC_CLOCK_ON;

  SREG = cSREG;
  interrupts();
}

// Triggered at the end of the VSYNC back porch
ISR(TIMER1_COMPB_vect) {
  // Enable the HSYNC! 
  // Every frame/64 switches the LED state.
  // LED is PORTC7, the frame/64 bit is bit 5.
  // So we get the 5th bit, then shift it over to 7th.
  PORT_HSYNC = (frame_num++ & 0x20) << 2;
  scanline_num = 0;
}

// Triggered at the end of the HSYNC back porch
ISR(TIMER3_COMPB_vect) {
  if (scanline_num >= NUM_SCANLINES) {
    return;
  } else {
    scanline_num++;
  }

  // Display all 8 colours, for 8 lines each.
  // RGB is PORTF5:7, so masking the lower 3 bits and shifting by 5.
  PORT_COLOUR = ((scanline_num >> 3) & 0x07) << 5;

  // Now I have 700 clock cycles to kill.
  // Or I could do a sleep.
  // 47us is the whole display, so 40 should be left-justified.
  delayMicroseconds(40);

  PORT_COLOUR = COLOUR(0, 0, 0);
}

void loop() {
}
