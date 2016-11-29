#include "FastLED.h"

FASTLED_USING_NAMESPACE

// FastLED "100-lines-of-code" demo reel, showing just a few 
// of the kinds of animation patterns you can quickly and easily 
// compose using FastLED.  
//
// This example also shows one easy way to define multiple 
// animations patterns and have them automatically rotate.
//
// -Mark Kriegsman, December 2014
//-- Modified for Scott McIndroe

#if defined(FASTLED_VERSION) && (FASTLED_VERSION < 3001000)
#warning "Requires FastLED 3.1 or later; check github for latest code."
#endif

//-- Pins on Arduino
#define DATA_PIN    4
#define SWITCH_PIN  7

//-- Debounce for button
int debounceMS = 100;

//#define CLK_PIN   4
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
#define NUM_LEDS    100
CRGB leds[NUM_LEDS];

#define BRIGHTNESS          96
#define FRAMES_PER_SECOND  120

//-- To ADD a new sequence, you will:
//-- (1) change the NUM_SEQUENCES
//-- (2) add a #define four your sequence name, it should be the next number in the sequence
//-- (3) make a funciton for that sequence, that service it each loop
//-- (4) add a case which calls that sequence in runSequence()

#define   NUM_SEQUENCES (4)
#define   SEQUENCE_BUILT_IN (0)
#define   SEQUENCE_YELLOW_FADE (1)
#define   SEQUENCE_EQUINOX (2)
#define   SEQUENCE_OFF (3)

int currentSequenceNum = 0;

#define MAX_BRIGHTNESS (255)
#define MIN_BRIGHTNESS (0)
int currentBrightness = 0;
int brightnessDir = -5;


void setup() {
  delay(3000); // 3 second delay for recovery

  pinMode(DATA_PIN, OUTPUT);  // 
  pinMode(SWITCH_PIN, INPUT);  // Your switch pin (though technically speaking, this code is not needed)
  
  // tell FastLED about the LED strip configuration
  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  //FastLED.addLeds<LED_TYPE,DATA_PIN,CLK_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);

  // set master brightness control
  FastLED.setBrightness(BRIGHTNESS);
}


// List of patterns to cycle through.  Each is defined as a separate function below.
typedef void (*SimplePatternList[])();
SimplePatternList gPatterns = { rainbow, rainbowWithGlitter, confetti, sinelon, juggle, bpm };

uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current
uint8_t gHue = 0; // rotating "base color" used by many of the patterns
  
void loop()
{
  checkButton();
  runSequence();
}

void checkButton() {
  if( digitalRead(SWITCH_PIN) == true  ) {   // button is pressed
        delay(debounceMS); // debounce
         
         // wait for button to be released
         while( digitalRead(SWITCH_PIN) == true ) {
           ;  // do nothing
         }
         delay(debounceMS); // debounce
         
       // increment the sequence numver
       currentSequenceNum++;
       if( currentSequenceNum == NUM_SEQUENCES )
         currentSequenceNum = 0;
   }
}

//- gets called on the loop and will run one cycle of the current sequence
void runSequence() {
  switch(currentSequenceNum) {
    case SEQUENCE_BUILT_IN:
      sequenceBuiltIn();
      break;
    
    case SEQUENCE_YELLOW_FADE:
      sequenceYellowFade();
      break;

     case SEQUENCE_EQUINOX:
      sequenceEquinox();
      break;

     case SEQUENCE_OFF:
      sequenceOff();
      break;
    
  }
}

void sequenceBuiltIn() {
  // Call the current pattern function once, updating the 'leds' array
  gPatterns[gCurrentPatternNumber]();

  // send the 'leds' array out to the actual LED strip
  FastLED.show();  
  // insert a delay to keep the framerate modest
  FastLED.delay(1000/FRAMES_PER_SECOND); 

  // do some periodic updates
  EVERY_N_MILLISECONDS( 20 ) { gHue++; } // slowly cycle the "base color" through the rainbow
  EVERY_N_SECONDS( 10 ) { nextPattern(); } // change patterns periodically
}

void sequenceYellowFade() {

  currentBrightness += brightnessDir;
  if( currentBrightness > 255 ) {
    currentBrightness = 255;
    brightnessDir = -brightnessDir;
  }
  else if( currentBrightness < 0) {
    currentBrightness = 0;
    brightnessDir = -brightnessDir;
  }
  
  for( int i = 0; i < NUM_LEDS; i++) 
    leds[i] = CRGB(255,255,153);


  FastLED.setBrightness(currentBrightness);
  
  // send the 'leds' array out to the actual LED strip
  FastLED.show();  
  // insert a delay to keep the framerate modest
  FastLED.delay(1000/FRAMES_PER_SECOND); 
}

void sequenceEquinox() {
  //-- just all blue
  // set master brightness control
  FastLED.setBrightness(BRIGHTNESS);
  
  for( int i = 0; i < NUM_LEDS; i++) 
    leds[i] = CRGB(0,0,255);
  
  // send the 'leds' array out to the actual LED strip
  FastLED.show();  
  
  // insert a delay to keep the framerate modest
  FastLED.delay(1000/FRAMES_PER_SECOND); 
}

void sequenceOff() {
  // set master brightness control
  FastLED.setBrightness(BRIGHTNESS);

  for( int i = 0; i < NUM_LEDS; i++) 
    leds[i] = CRGB(0,0,0);
  
  // send the 'leds' array out to the actual LED strip
  FastLED.show();  
  
  // insert a delay to keep the framerate modest
  FastLED.delay(1000/FRAMES_PER_SECOND); 
}

//-- ALL THESE ARE THE BUILT-IN SEQUENCES
#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

void nextPattern()
{
  // add one to the current pattern number, and wrap around at the end
  gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE( gPatterns);
}

void rainbow() 
{
  // FastLED's built-in rainbow generator
  fill_rainbow( leds, NUM_LEDS, gHue, 7);
}

void rainbowWithGlitter() 
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

void confetti() 
{
  // random colored speckles that blink in and fade smoothly
  fadeToBlackBy( leds, NUM_LEDS, 10);
  int pos = random16(NUM_LEDS);
  leds[pos] += CHSV( gHue + random8(64), 200, 255);
}

void sinelon()
{
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( leds, NUM_LEDS, 20);
  int pos = beatsin16(13,0,NUM_LEDS);
  leds[pos] += CHSV( gHue, 255, 192);
}

void bpm()
{
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  uint8_t BeatsPerMinute = 62;
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
  for( int i = 0; i < NUM_LEDS; i++) { //9948
    leds[i] = ColorFromPalette(palette, gHue+(i*2), beat-gHue+(i*10));
  }
}

void juggle() {
  // eight colored dots, weaving in and out of sync with each other
  fadeToBlackBy( leds, NUM_LEDS, 20);
  byte dothue = 0;
  for( int i = 0; i < 8; i++) {
    leds[beatsin16(i+7,0,NUM_LEDS)] |= CHSV(dothue, 200, 255);
    dothue += 32;
  }
}

