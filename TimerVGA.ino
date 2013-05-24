// Definition of interrupt names
#include < avr/io.h >
// ISR interrupt service routine
#include < avr/interrupt.h >

#define VSYNC_CLOCK_ON ((3 << WGM12) | (3 << CS10))
#define VSYNC_CLOCK_OFF (3 << WGM12)
#define HSYNC_CLOCK_ON ((1 << WGM32) | (1 << CS30))
#define HSYNC_CLOCK_OFF (1 << WGM32)

#define PORT_VSYNC PORTB
#define DDR_VSYNC DDRB
#define VSYNC_BIT (1 << PORTB5)

#define PORT_HSYNC PORTC
#define DDR_HSYNC DDRC
#define HSYNC_BIT (1 << PORTC6)

#define PORT_COLOUR PORTF
#define DDR_COLOUR DDRF
#define COLOUR(R, G, B) ((R << PORTF5) | (G << PORTF6) | (B << PORTF7))

void setup() {
  char cSREG;
  cSREG = SREG;
  noInterrupts();

  //////////////////
  // Configure VSYNC
  //////////////////
  // WGM1 = 14 -> Fast PWM up to ICR
  // COM1A = 3 -> -vsync
  // COM1B = 0 -> GNDN
  // COM1C = 0 -> GNDN
  // CS1 = 3 -> /64
  //
  // WGM11:0 = 2 (top of 14)
  TCCR1A = (3 << COM1A0) | (2 << WGM10);
  // WGM13:2 = 3 (top of 14)
  TCCR1B = VSYNC_CLOCK_OFF;
  // Start counter at 0
  TCNT1 = 0;
  // VSYNC clock is 16000000Hz / 64 = 250kHz
  // 250000Hz / 60 Hz = 4166
  ICR1 = 4166;
  // 250kHz * 0.2msSync = 50
  OCR1A = 50;
  // VSYNC + VBackPorch (0.2ms + 1ms)
  OCR1B = 50 + 250;
  // Listen for the end of the back porch
  TIFR1 = 0;
  TIMSK1 = (1 << OCIE1B);
  // OC1A => PB5 => pin 9
  // Start with VSYNC not in progress.
  PORT_VSYNC = VSYNC_BIT;
  DDR_VSYNC = VSYNC_BIT;


  //////////////////
  // Configure HSYNC
  //////////////////
  // WGM3 = 7 -> Fast PWM, 10-bit, up to 0x3FF
  // COM3A = 3 -> -hsync
  // COM3B = 0 -> GNDN
  // COM3C = 0 -> GNDN
  // CS3 = 1 -> /1
  //
  // WGM31:0 = 3 (top of 7)
  TCCR3A = (3 << COM3A0) | (3 << WGM30);
  // WGM33:2 = 1 (top of 7)
  TCCR3B = HSYNC_CLOCK_OFF;
  // Start counter at 0
  TCNT3 = 0;
  // HSYNC clock is 16000000Hz / 1 = 16MHz
  // 16MHz / 1024 = 15.6kHz
  // 16MHz * 4.7usSync = 75
  OCR3A = 75;
  // VSYNC + VBackPorch (4.7us + 7us)
  OCR3B = 75 + 112;
  // Listen for the end of the back porch
  TIFR3 = 0;
  TIMSK3 = (1 << OCIE3B);
  // OC3A => PC6 => pin 5
  // Start with HSYNC not in progress.
  // PORTC7 is the LED
  PORT_HSYNC = HSYNC_BIT;
  DDR_HSYNC = HSYNC_BIT | (1 << PORTC7);

  // Enable RGB
  PORT_COLOUR = COLOUR(0, 0, 0);
  DDR_COLOUR = COLOUR(1, 1, 1);

  // Enable the VSYNC!
  PORTB = 0;
  TCCR1B = VSYNC_CLOCK_ON;

  SREG = cSREG;
  interrupts();
}

ISR(TIMER1_COMPB_vect) {
  // Enable the HSYNC!
  PORTC = 0;
  TCCR3B |= 
}

void loop() {
}
