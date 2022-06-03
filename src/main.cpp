#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#ifdef __AVR__
 #include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif

const uint8_t PIXEL_PIN = 10;

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)
Adafruit_NeoPixel RGB = Adafruit_NeoPixel(1, PIXEL_PIN, NEO_GRB + NEO_KHZ800);


const uint8_t LATCH_PIN =  8;
const uint8_t CLOCK_PIN = 12;
const uint8_t DATA_PIN  = 11;

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<RGB.numPixels(); i++) {
    RGB.setPixelColor(i, c);
    RGB.show();
    delay(wait);
  }
}


void setup() {
  Serial.begin(9600);

  pinMode(LATCH_PIN, OUTPUT);
  pinMode(CLOCK_PIN, OUTPUT);
  pinMode(DATA_PIN, OUTPUT);

  RGB.begin();
  RGB.setBrightness(50);
  RGB.show(); // Initialize all pixels to 'off'
}

void loop() {
  Serial.println("loop");

  colorWipe(RGB.Color(255, 0, 0), 50); // Red


  digitalWrite(LATCH_PIN, LOW);
  shiftOut(DATA_PIN, CLOCK_PIN, LSBFIRST, 0b00000001);
  digitalWrite(LATCH_PIN, HIGH);

  delay(1000);
  digitalWrite(LATCH_PIN, LOW);
  shiftOut(DATA_PIN, CLOCK_PIN, LSBFIRST, 0b00000000);
  digitalWrite(LATCH_PIN, HIGH);
  delay(1000);
}