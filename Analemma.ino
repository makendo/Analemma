#include "FastLED.h"

FASTLED_USING_NAMESPACE

#define DATA_PIN    4
#define LED_TYPE    WS2811
#define COLOR_ORDER RGB
#define NUM_LEDS    364
CRGB leds[NUM_LEDS];

int num_leds = NUM_LEDS;
int day_of_year = 337; // change to todays date

int frames_per_second = 120;

//-- switch setup
int button_pin = 7; // set to 7 for button to switch modes
//int button_pin = -1; // set to -1 to ignore button
int debounceMS = 100; //-- Debounce for button
int mode = 0; // modes: 0 -> static, 1 -> ?, 2 -> ?, 3 -> rainbow
int num_modes = 5;

int min_brightness = 0;
int max_brightness = 255;
int brightness = 255;  // ***master brightness dial***
int brightness_step = -5;

// unsigned 8 bit int will count from 0 to 255 and overflow back to 0
uint8_t current_brightness = 0;
uint8_t rainbow_hue = 0;

// static leds to light
int num_firsts = 12;
int firsts[] = {1, 32, 61, 92, 122, 153, 183, 214, 245, 275, 306, 336}; // firsts of each month should be red
int solstices[] = {171, 355};
int equinoxes[] = {80, 266};

// sunrise vars
int riseness = 0;
int rise_k = 4; // bigger -> tighter sunrise spread
int max_rise = 500;
int rise_reds[] = {0, 31, 127, 180, 255};
int rise_greens[] = {0, 0, 0, 127, 255};
int rise_blues[] = {0, 15, 0, 0, 0};
int rise_steps = 5; // number of colors in rise reds & greens

// interpolate rise to an rgb color
CRGB lerpRise(int y) {
  int lerp_step = max_rise / (rise_steps - 1);
  int r_step = y / lerp_step;
  if (r_step >= (rise_steps - 1)) {
    return CRGB(rise_reds[rise_steps - 1], rise_greens[rise_steps - 1], 0);
  }
  int r_red = map(y % lerp_step, 0, lerp_step, rise_reds[r_step], rise_reds[r_step+1]);
  int r_green = map(y % lerp_step, 0, lerp_step, rise_greens[r_step], rise_greens[r_step+1]);
  int r_blue = map(y % lerp_step, 0, lerp_step, rise_blues[r_step], rise_blues[r_step+1]);
  return CRGB(r_red, r_green, r_blue);
}

void setup() {
  Serial.begin(9600);
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
  if (mode == 1) stepSunrise();
  if (mode == 2) stepSunset();
  if (mode == 3) stepRainbow();
  if (mode == 4) stepDark();

  // set today to white
  if (mode != 4) leds[get_day(day_of_year)] = CRGB(255, 255, 127); // white

  // send the 'leds' array out to the actual LED strip
  FastLED.show();  
  
  // insert a delay to keep the framerate modest
  //FastLED.delay(1000/frames_per_second); 

  EVERY_N_MILLISECONDS( 20 ) { rainbow_hue++; } // slowly cycle the "base color" through the rainbow
}

int get_day(int day) {
  day = day - 2;
  if (day < 0) day = 364 + day;
  return day % num_leds;
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
        riseness = 0; // reset sunrise
       }
   }
}

void stepDark() {
  for (int i = 0; i < num_leds; i++) {
    leds[i] = CRGB(0, 0, 0); // black
  }
}

void stepStatic() {
  
  // set all leds to yellow
  for (int i = 0; i < num_leds; i++) {
    leds[i] = CRGB(15, 15, 0); // yellow
  }

  // set firsts to red
  for (int i = 0; i < num_firsts; i++) {
    // firsts[i] % num leds -> wrap around if this first is bigger than num_leds
    leds[get_day(firsts[i])] = CRGB(255, 0, 0); // red
  }

  // set solstices to blue
  for (int i = 0; i < 2; i++) {
    leds[get_day(solstices[i])] = CRGB(0, 0, 255); // blue
  }  

  // set equinoxes to green
  for (int i = 0; i < 2; i++) {
    leds[get_day(equinoxes[i])] = CRGB(0, 255, 0); // green
  }
}

// 
void stepSunrise() {
  //Serial.println("\nstepping");
  if (riseness < (max_rise + 200)) { // count up to twice max rise
    riseness++;
    renderSun();
  }
}

void stepSunset() {
  if (riseness > 0) {
    riseness--;
    renderSun();
  }
}

void renderSun() {
  //Serial.println(riseness);
  int pivot = day_of_year % num_leds;
  leds[pivot] = lerpRise(riseness);
  int left_p = pivot;
  int right_p = pivot;
  for (int d = 0; d < num_leds/2; d++) {
    int y = max(0, riseness - (d << 1));
    //Serial.println(y);
    left_p--;
    if (left_p < 0) left_p = num_leds - 1;
    right_p++;
    if (right_p >= num_leds) right_p = 0;
    leds[left_p] = leds[right_p] = lerpRise(y);
  }
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

