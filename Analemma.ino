#include "FastLED.h"

FASTLED_USING_NAMESPACE

#define DATA_PIN    4
#define LED_TYPE    WS2811
#define COLOR_ORDER RGB
#define NUM_LEDS    100
CRGB leds[NUM_LEDS];

int num_leds = NUM_LEDS;
int day_of_year = 315; // change to todays date

int frames_per_second = 120;

//-- switch setup
// int button_pin = 7; // set to 7 for button to switch modes
int button_pin = -1; // set to -1 to ignore button
int debounceMS = 100; //-- Debounce for button
int mode = 1; // modes: 0 -> static, 1 -> ?, 2 -> ?, 3 -> rainbow
int num_modes = 4;

int min_brightness = 0;
int max_brightness = 255;
int brightness = 100;  // ***master brightness dial***
int brightness_step = -5;

// unsigned 8 bit int will count from 0 to 255 and overflow back to 0
uint8_t current_brightness = 0;
uint8_t rainbow_hue = 0;

// static leds to light
int num_firsts = 12;
int firsts[] = {1, 32, 61, 92, 122, 153, 183, 214, 245, 275, 306, 336}; // firsts of each month should be red
int solstices[] = {194, 356};
int equinoxes[] = {80, 266};

void setup() {
  delay(500); // half second delay for recovery

  pinMode(DATA_PIN, OUTPUT);  // 
  pinMode(button_pin, INPUT);  // Your switch pin (though technically speaking, this code is not needed)
  
  // tell FastLED about the LED strip configuration
  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);

  // set master brightness control
  FastLED.setBrightness(brightness);
}
  
void loop()
{
  // check for button press
  checkButton();

  // write current mode pattern to leds array
  if (mode == 0) stepStatic();
  if (mode == 1) stepRainbow();

  // send the 'leds' array out to the actual LED strip
  FastLED.show();  
  
  // insert a delay to keep the framerate modest
  FastLED.delay(1000/frames_per_second); 

  EVERY_N_MILLISECONDS( 20 ) { rainbow_hue++; } // slowly cycle the "base color" through the rainbow
}

void checkButton() {
  if( button_pin < 0 ) return; // ignore button if pin isnt set
  
  if( digitalRead(button_pin) == true  ) {   // button is pressed
        delay(debounceMS); // debounce
         
         // wait for button to be released
         while( digitalRead(button_pin) == true ) {
           ;  // do nothing
         }
         delay(debounceMS); // debounce
         
       mode++;
       if (mode >= num_modes) {
        mode = 0;
       }
   }
}

void stepStatic() {
  
  // set all leds to yellow
  for (int i = 0; i < num_leds; i++) {
    leds[i] = CRGB( 255, 255, 0); // yellow
  }

  // set firsts to red
  for (int i = 0; i < num_firsts; i++) {
    // firsts[i] % num leds -> wrap around if this first is bigger than num_leds
    leds[firsts[i] % num_leds] = CRGB(255, 0, 0); // red
  }

  // set solstices to blue
  for (int i = 0; i < 2; i++) {
    leds[solstices[i] % num_leds] = CRGB(0, 0, 255); // blue
  }  

  // set equinoxes to green
  for (int i = 0; i < 2; i++) {
    leds[equinoxes[i] % num_leds] = CRGB(0, 255, 0); // green
  }

  // set today to white
  leds[day_of_year % num_leds] = CRGB(255, 255, 127); // white
}

void rainbow() 
{
  // FastLED's built-in rainbow generator
  fill_rainbow( leds, NUM_LEDS, rainbow_hue, 7);
}

void stepRainbow() 
{
  // built-in FastLED rainbow, plus some random sparkly glitter
  rainbow();
  addGlitter(80);
}

void addGlitter( fract8 chanceOfGlitter) 
{
  if( random8() < chanceOfGlitter) {
    leds[ random16(NUM_LEDS) ] += CRGB::White;
  }
}

