#include <TempController.h>

// We always have to include the library
#include "LedControl.h"

// Number of 
#define HOLDPERIOD0 333

/* we always wait a bit between updates of the display */
int count = 0;
int holdcount = 0;
int holdperiod = HOLDPERIOD0;
byte state, prevstate;
byte temp_a, temp_b;
static DisplayBoard displayBoard;

// Time per loop
#define DELAYTIME 2

#define STATE_READ 0
#define STATE_WAIT_BUTTON_UP 1
#define STATE_WAIT_BUTTON_DOWN 2
#define STATE_COUNT_DOWN 3
#define STATE_COUNT_UP 4

#define WHICH_BUTTON_PUSHED (displayBoard.isButtonPushed(BUTTON_DOWN) ? "down" : (displayBoard.isButtonPushed(BUTTON_UP) ? "up" : "none"))

void printButtons() {
    Serial.print("UP = ");
    Serial.println(displayBoard.isButtonPushed(BUTTON_UP));
    Serial.print("DOWN = ");
    Serial.println(displayBoard.isButtonPushed(BUTTON_DOWN));
    Serial.print("RIGHT = ");
    
    Serial.print("Switch is ");
    if (displayBoard.isSwitchRight()) {
      Serial.println("RIGHT");
    }
    
    if (displayBoard.isSwitchLeft()) {
      Serial.println("LEFT");
    }
    
    if (displayBoard.isSwitchCenter()) {
      Serial.println("CENTER");
    }
}

// Forward reference declaration
void updateDisplay();

void setup() {
    // Serial
    Serial.begin(9600);
    while (!Serial) {
    }
    
    printButtons();
    
    Serial.println("Push right button to count up, left button to count down");

    state = STATE_WAIT_BUTTON_DOWN;
    prevstate = 0xff;
    count = 0;
    holdcount = 1000 / DELAYTIME; // After 2 seconds, start counting fast
    holdperiod = HOLDPERIOD0;
    temp_a = temp_b = 0;
    updateDisplay();    
}

#define TEST_CHANNELS 1

#ifdef TEST_CHANNELS

void updateDisplay()
{
  displayBoard.temp(CHANNEL_A, temp_a);
  displayBoard.temp(CHANNEL_B, temp_b);
  
  displayBoard.hot(CHANNEL_A, temp_a&1);
  displayBoard.hot(CHANNEL_B, temp_b&1);

  displayBoard.cold(CHANNEL_A, temp_a&2);
  displayBoard.cold(CHANNEL_B, temp_b&2);
  
}

void countUp(byte& temp_x) {
  temp_x++;
  Serial.println(temp_x);
  updateDisplay();
}

void countDown(byte& temp_x) {
  temp_x--;
  Serial.println(temp_x);
  updateDisplay();
}

void loop() {

  if (state != prevstate) {
     Serial.print("State = ");
     Serial.println(state);
     prevstate = state;
  }
  

  switch (state) {
          
    case STATE_READ: {
      
        printButtons();
        
        boolean count_up = displayBoard.isButtonPushed(BUTTON_UP);
  
        if (displayBoard.isSwitchCenter() || displayBoard.isSwitchLeft()) {
          (count_up ? countUp : countDown)(temp_a);
        }
        if (displayBoard.isSwitchCenter() || displayBoard.isSwitchRight()) {
          (count_up ? countUp : countDown)(temp_b);
        }
        state = STATE_WAIT_BUTTON_UP;
      }
      break;
      
    case STATE_WAIT_BUTTON_DOWN:
      if (displayBoard.isButtonPushed()) {
        Serial.print("button-down: ");
        Serial.println(WHICH_BUTTON_PUSHED);
      
        state = STATE_READ;
      }
      break;

      
    case STATE_WAIT_BUTTON_UP:
      if (!displayBoard.isButtonPushed()) {
        Serial.println("button-up");
        holdcount = 2000 / DELAYTIME;
        holdperiod = HOLDPERIOD0;
        state = STATE_WAIT_BUTTON_DOWN;
      }
      // Timeout of holdcount while waiting causes countUp/Down
      if (holdcount-- <= 0) {
        holdcount = holdperiod / DELAYTIME; // 3 counts/sec is fast count
        holdperiod = (holdperiod * 95) / 100;
        if (holdperiod = 0) { holdperiod = 1; }
        state = STATE_READ;
      }
      break;
      
    default:
      Serial.print("Illegal state ");
      Serial.println(state);
      delay(10000);
      state = STATE_WAIT_BUTTON_DOWN;
      break;

  }
  delay(DELAYTIME);
}
#endif

#ifdef TEST_4_DIGIT_COUNTER
/*
void updateDisplay()
{
    unsigned int n = (unsigned int)count;

    //Serial.println(count);
    displayBoard.digit(DIGIT1, n / 1000, n&8);
    n %= 1000;
    displayBoard.digit(DIGIT2, n / 100, n&4);
    n %= 100;
    displayBoard.digit(DIGIT3, n / 10, n&2);
    n %= 10;
    displayBoard.digit(DIGIT4, n, n&1);
    
    displayBoard.leds(n & 0xf);

}
void countUp() {
  count = (count + 1) % 10000;
  updateDisplay();
}
void countDown() {
  count = (count + 9999) % 10000;
  updateDisplay();
}
void loop() {
  if (state != prevstate) {
     Serial.print("State = ");
     Serial.println(state);
     prevstate = state;
  }

//  if (displayBoard.isButtonPushed(BUTTON_LEFT)) {
//    state = STATE_COUNT_DOWN;
//  }
  
//  if (displayBoard.isButtonPushed(BUTTON_RIGHT)) {
//    state = STATE_COUNT_UP;
//  }
  

  switch (state) {
    
    case STATE_COUNT_DOWN:
      if (displayBoard.isSwitchCenter()) {
        state = STATE_WAIT_BUTTON_DOWN;
        DELAYTIME = 2;
      } else {
        countDown();
      }
      break;
          
    case STATE_COUNT_UP:
      if (displayBoard.isSwitchCenter()) {
        state = STATE_WAIT_BUTTON_DOWN;
        DELAYTIME = 2;
      } else {
        countUp();
      }
      break;
          
    case STATE_READ:
    
      printButtons();
      
      if (displayBoard.isButtonPushed(BUTTON_UP)) {
          countUp();
      }
      if (displayBoard.isButtonPushed(BUTTON_DOWN)) {
          countDown();
      }
      
      state = STATE_WAIT_BUTTON_UP;
      break;
      
    case STATE_WAIT_BUTTON_DOWN:
          
      if (displayBoard.isSwitchLeft()) {
        state = STATE_COUNT_DOWN;
        DELAYTIME = 500;
        break;
      }
      
      if (displayBoard.isSwitchRight()) {
        state = STATE_COUNT_UP;
        DELAYTIME = 500;
        break;
      }
      
      if (displayBoard.isButtonPushed(BUTTON_UP) || displayBoard.isButtonPushed(BUTTON_DOWN)) {
        Serial.println("button-down");
        state = STATE_READ;
      }
      break;
      
    case STATE_WAIT_BUTTON_UP:
      if (!(displayBoard.isButtonPushed(BUTTON_UP) || displayBoard.isButtonPushed(BUTTON_DOWN))) {
        Serial.println("button-up");
        state = STATE_WAIT_BUTTON_DOWN;
      }
      break;
      
    default:
      Serial.print("Illegal state ");
      Serial.println(state);
      delay(10000);
      state = STATE_WAIT_BUTTON_DOWN;
      break;

  }
  delay(DELAYTIME);
}
*/
#endif

