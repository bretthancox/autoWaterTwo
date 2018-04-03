#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/wdt.h>

// Data logging configuration.

#define PUMP_PIN             4
#define READ_PIN            2
#define LED_PIN             3

// Internal state used by the sketch.

volatile bool watchdogActivated = false;
void takeReading(int* a);
void waterDecision(int* value);
void blinkLed(int del, int reps);
void setup_watchdog(int ii);
void sleep();
int highReading;
int highTime;
int wdt_count = 20;



ISR(WDT_vect)
{
  watchdogActivated = true;
  wdt_count += 1;
}



// Put the Arduino to sleep.

void sleep() {
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);

  // Turn off the ADC while asleep.
  power_adc_disable();

  // Enable sleep and enter sleep mode.
  sleep_mode();
  sleep_disable();
  power_all_enable();
}



void setup(void) {  
  pinMode(PUMP_PIN, OUTPUT);
  digitalWrite(PUMP_PIN, HIGH);
  pinMode(READ_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);

  // Setup the watchdog timer to run an interrupt which
  // wakes the Arduino from sleep every 8 seconds.
  // Note that the default behavior of resetting the Arduino
  // with the watchdog will be disabled.

  // This next section of code is timing critical, so interrupts are disabled.
  // See more details of how to change the watchdog in the ATmega328P datasheet
  // around page 50, Watchdog Timer.

  noInterrupts();

  // Set the watchdog reset bit in the MCU status register to 0. Tilde is the NOT operator. In all equations, the code (e.g. WDRF) represents it's bitwise position). So in this example, it shifts the number 1 three times, because WDRF is bit 3.
  // MCUSR = MCUSR & ~(1<<WDRF)
  // MCUSR = 00001000 & ~(0000000)
  // MCUSR = 00001000 & 11111111
  // MCUSR = 00001000
/*
  MCUSR &= ~(1<<WDRF);

  // Set WDCE and WDE bits in the watchdog control register. "|=" is compound operator. (a |= (1<<b)) == (a = a | (1<<b))
  // WDTCR == 0      0    0     0     X    0     0     0 (where x is either 1 or 0 depending on WDCE)
  //         WDIF  WDIE  WDP3  WDCE  WDE  WDP2  WDP1  WDP0
  // The following equation changes WDCE and WDE to be 1 and then assigns the new value of the entire register to WDTCR:
  // WDTCR = 00000000 | 00010000 | 00001000 = 00011000 Which means watchdog timer is running and action on time-out is 'interrupt'

  WDTCR |= (1<<WDCE) | (1<<WDE);

  // Set watchdog clock prescaler bits to a value of 8 seconds.
  // WDTCR == 00011000 at this point
  // WDTCR =  00000001 | 00100000 = 00100001

  WDTCR = (1<<WDP0) | (1<<WDP3);

  // Enable watchdog as interrupt only (no reset).
  // WDTCR = WDTCR | (1<<WDIE)
  // WDTCR = 00110001 | 01110001 = 01110001

  WDTCR |= (1<<WDIE);
*/
  // Enable interrupts again.

  setup_watchdog(9);
  interrupts();
}


void loop(void) {
  // Don't do anything unless the watchdog timer interrupt has fired.
  if (wdt_count >= 20)
  {
    takeReading( &highReading );
    waterDecision( &highReading );
    watchdogActivated = false;
    wdt_count = 0;
  }
  // Go to sleep!
  sleep();
}





void takeReading(int* a) {

highTime = 0;
for(int i=0;i<10;i++) {
  highTime += pulseIn(READ_PIN, HIGH);
  }

*a = highTime/10;
}


void waterDecision(int* value) {
 if (*value == 0) {
  blinkLed(150, 9);
 }
 else if (*value < 150) {
   digitalWrite(PUMP_PIN,LOW);
   delay(10000);
   digitalWrite(PUMP_PIN,HIGH);
   blinkLed(500, 3);
   delay(1000);
 }
 else if (*value >= 150) {
  //digitalWrite(PUMP_PIN,HIGH);
  blinkLed(2000, 1);
 }
}


void blinkLed(int del, int reps) {
  for (int i=0; i <= reps; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(del);
    digitalWrite(LED_PIN, LOW);
    delay(del);
  }
}


// 0=16ms, 1=32ms,2=64ms,3=128ms,4=250ms,5=500ms
// 6=1 sec,7=2 sec, 8=4 sec, 9= 8sec

void setup_watchdog(int ii) {

  byte bb;
  int ww;
  if (ii > 9 ) ii=9;
  bb=ii & 7;
  if (ii > 7) bb|= (1<<5);
  bb|= (1<<WDCE);
  ww=bb;

  MCUSR &= ~(1<<WDRF);

  // start timed sequence
  WDTCR |= (1<<WDCE) | (1<<WDE);

  // set new watchdog timeout value
  WDTCR = bb;

  WDTCR |= _BV(WDIE);
}
