/*
Little Rays Firmware v0.03
Matthew Fan
April 20th, 2018

Adapted from the H20hNo! driver by Nat Seidle.
 
Little Rays is a wearable nightlight
Designed to be cheap to build, with a long-lasting battery life.

Only consumes ~6mA at full power! That's Wild!

Just a note-  Make sure you compile with the correct clock speed, or delays will be off.
              The Attiny85s that I tested with had clock speeds of 1MHz (default)
 */

#include <avr/sleep.h> //Needed for sleep_mode
#include <avr/wdt.h> //Needed to enable/disable watch dog timer

/* Inputs and Outputs*/
#define light 2 //the little rays output light
#define dark1 A2 //higher value corresponds to "darker" room
#define dark2 A3
#define button 1 //Button for manual re-lighting
#define power 0 //Power to the darkness circuits

/*Other constants*/
//FEEL FREE TO ADJUST AS NEEDED
#define AUTO_SHUTDOWN_TIME_IN_SECS 900 //Time in seconds before the night-light automatically shuts off
#define REFRESH_DELAY 100 //refresh delay in ms. Please use numbers that evenly divide 1000
#define INITIAL_LIGHT_THRESHOLD 570. //Maxes out around 650. Ideal value seems to be around 570

/*Variables*/
int light_threshold = INITIAL_LIGHT_THRESHOLD;
int auto_shutdown_time = (1000/REFRESH_DELAY)*AUTO_SHUTDOWN_TIME_IN_SECS; //Specify time to automatically shut down, in ms.
int shutdown_counter = 0;
bool timed_out = false;
bool is_dark = false;

ISR(WDT_vect) {
  //Don't do anything. This is just here so that we wake up.
}

void setup()
{
  //Setting Pin Modes
  pinMode(light, OUTPUT);
  pinMode(power, OUTPUT);
  pinMode(button, INPUT);

  //Turning systems off
  digitalWrite(light, LOW);
  digitalWrite(power, LOW);
  
  //Power down various bits of hardware to lower power usage  
  set_sleep_mode(SLEEP_MODE_PWR_DOWN); //Power down everything, wake up from WDT
  sleep_enable();
}

void checkStates(){
  //checks states and updates globals
  digitalWrite(light,LOW);//turning off light so darkness circuits can get accurate measurements
  digitalWrite(power, HIGH); //turning on power for darkness circuits
  is_dark = analogRead(dark1) > light_threshold && analogRead(dark2) > light_threshold; //is the room dark?
  digitalWrite(power, LOW); //Shutting down darkness circuits
  if(digitalRead(button) == HIGH){//If the button is pressed, reset shutdown counter
    shutdown_counter = 0;
  }
  timed_out = shutdown_counter >= auto_shutdown_time; //is the shutdown timer timed out?
}

void loop() 
{
  ADCSRA &= ~(1<<ADEN); //Disable ADC, saves ~230uA
  setup_watchdog(6); //Setup watchdog to go off after 1sec
  sleep_mode(); //Go to sleep! Wake up 1sec later and check for darkness

  //Checking if room is dark
  ADCSRA |= (1<<ADEN); //Enable ADC
  checkStates();
  
  if(is_dark && !timed_out){
    wdt_disable(); //Turn off the WDT!!

    //Loop to keep light on
    while(is_dark && !timed_out){
      digitalWrite(light, HIGH);
      shutdown_counter++;
      //Re-evaluating exit conditions for loop.
      delay(REFRESH_DELAY);
      checkStates();
    } 
    
  }else if(!is_dark && shutdown_counter > 0){//Slowly bring shutdown counter back to 0 if lights are on. 
    shutdown_counter--;
    delay(REFRESH_DELAY);
  }
  digitalWrite(light,LOW);//turns off the light
}

//Sets the watchdog timer to wake us up, but not reset
//0=16ms, 1=32ms, 2=64ms, 3=128ms, 4=250ms, 5=500ms
//6=1sec, 7=2sec, 8=4sec, 9=8sec
//From: http://interface.khm.de/index.php/lab/experiments/sleep_watchdog_battery/
void setup_watchdog(int timerPrescaler) {

  if (timerPrescaler > 9 ) timerPrescaler = 9; //Limit incoming amount to legal settings

  byte bb = timerPrescaler & 7; 
  if (timerPrescaler > 7) bb |= (1<<5); //Set the special 5th bit if necessary

  //This order of commands is important and cannot be combined
  MCUSR &= ~(1<<WDRF); //Clear the watch dog reset
  WDTCR |= (1<<WDCE) | (1<<WDE); //Set WD_change enable, set WD enable
  WDTCR = bb; //Set new watchdog timeout value
  WDTCR |= _BV(WDIE); //Set the interrupt enable, this will keep unit from resetting after each int
}
