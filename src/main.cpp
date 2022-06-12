/************************************************
 * File: main.cpp
 * Project: Lillering
 * MCU Board: Arduino Pro Micro (BTE13-010B, www.betemcu.cn) 16MHz atmega328 5V
 * Github: https://github.com/taunoe/lillering
 * 
 * Last edited: 06.06.2022
 * 
 * Copyright 2022 Tauno Erik
 ************************************************/
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

// Arduino pin names
const uint8_t LATCH_PIN =  8;  // PB0;
const uint8_t CLOCK_PIN = 12;  // PB4;
const uint8_t DATA_PIN  = 11;  // PB3;

// Atmega PortB pin names
const uint8_t PORTB_LATCH_PIN = 0;  // PB0;
const uint8_t PORTB_CLOCK_PIN = 4;  // PB4;
const uint8_t PORTB_DATA_PIN = 3;   // PB3;

const uint8_t NUM_7SEGS = 5;

uint8_t data[NUM_7SEGS] = {
  0b01010101,
  0b01010101,
  0b01010101,
  0b01010101,
  0b01010101
};

uint32_t current_time = 0;
uint32_t prev_time_1 = 0;
uint32_t prev_time_2 = 0;

/* Shift Register Functions */
void all_off(){
  for (uint8_t i = 0; i < NUM_7SEGS; i++){
    data[i] = 0;
  }
}

// Väljasta kogu data massiivi korraga
void output_all() {
  for (uint8_t i = 0; i < NUM_7SEGS; i++) {
    digitalWrite(LATCH_PIN, LOW);
    shiftOut(DATA_PIN, CLOCK_PIN, LSBFIRST, data[i]);
    digitalWrite(LATCH_PIN, HIGH);
  }
}

/*
Shift data to 7-segments.
*/

void shift_out_all_data(uint8_t data[]) {
  for (uint8_t segment = 0; segment < NUM_7SEGS; segment++) {

    PORTB &= ~(1<<PORTB_LATCH_PIN);  // digitalWrite(LATCH_PIN, LOW);

    uint8_t d = data[segment];

    for (uint8_t i = 0; i < 8; i++) {
      uint8_t bit = (d & 128) != 0;
      if (bit == 0) {
        PORTB &= ~(1 << PORTB_DATA_PIN);
      } else {
        PORTB |= (1 << PORTB_DATA_PIN);
      }
      d <<= 1;

      PORTB |= (1<<PORTB_CLOCK_PIN);   // Set to HIGH
      PORTB &= ~(1<<PORTB_CLOCK_PIN);  // Set to LOW
    }
    
    PORTB |= (1<<PORTB_LATCH_PIN);  // digitalWrite(LATCH_PIN, HIGH);
  }
}

/*
  One by one: CounterClockWise
 */
void one_by_one_CCW(uint32_t delay_time) {
  all_off();

  for (uint8_t seg = NUM_7SEGS; seg > 0; seg--) {
    data[seg-1] = 1;
    shift_out_all_data(data);
    delay(delay_time);

    for (uint8_t bit = 0; bit < 8; bit++) {
      data[seg-1] = (data[seg-1]<<1);
      shift_out_all_data(data);
      delay(delay_time);
    }

    data[seg-1] = 0;
    shift_out_all_data(data);
  }
}

/*
  One by one: ClockWise
 */
void one_by_one_CW(uint32_t delay_time) {
  all_off();

  for (uint8_t seg = 0; seg < NUM_7SEGS; seg++) {
    data[seg] = 0b10000000;
    shift_out_all_data(data);
    delay(delay_time);

    for (uint8_t bit = 0; bit < 8; bit++) {
      data[seg] = (data[seg]>>1);
      shift_out_all_data(data);
      delay(delay_time);
    }
    data[seg] = 0;
    shift_out_all_data(data);
  }
}

const uint8_t COLS[8] = {
  0b10000000,
  0b01000000,
  0b00100000,
  0b00010000,
  0b00001000,
  0b00000100,
  0b00000010,
  0b00000001
};


void one_by_one_in_two_directions(uint32_t delay_time) {
  all_off();

  for (uint8_t row = 0; row < NUM_7SEGS; row++) {

    for (size_t c = 0; c < 8; c++) {
      // If first column in row
      if (c == 0) {
        // Set last in previous row to Low
        if (row == 0) {
          data[NUM_7SEGS] &= ~(COLS[7]);      // set back to Low
          data[0] &= ~(COLS[0]);
        } else {
          data[row-1] &= ~(COLS[7]);      // set back to Low
          data[4-row+1] &= ~(COLS[0]);      // set back to LOW
        }
      } else {
        // Set previous in current row Low
        data[row] &= ~(COLS[c-1]);    // set back to Low
        data[4-row] &= ~(COLS[7-c+1]);  // set back to LOW
      }

      data[row] |= COLS[c];           // set to High
      data[4-row] |= COLS[7-c];         // set to High

      shift_out_all_data(data);     // Display
      delay(delay_time);            // Wait
    }
  }
}


void two_directions(uint32_t delay_1, uint32_t delay_2) {
  all_off();

  for (uint8_t row = 0; row < NUM_7SEGS; row++) {

    for (size_t c = 0; c < 8; c++) {
      // If first column in row
      if (c == 0) {
        // Set last in previous row to Low
        if (row == 0) {
          data[NUM_7SEGS] &= ~(COLS[7]);      // set back to Low
          data[0] &= ~(COLS[0]);
        } else {
          data[row-1] &= ~(COLS[7]);      // set back to Low
          data[4-row+1] &= ~(COLS[0]);      // set back to LOW
        }
      } else {
        // Set previous in current row Low
        data[row] &= ~(COLS[c-1]);    // set back to Low
        data[4-row] &= ~(COLS[7-c+1]);  // set back to LOW
      }

      data[row] |= COLS[c];           // set to High
      data[4-row] |= COLS[7-c];         // set to High

      shift_out_all_data(data);     // Display
      delay(delay_time);            // Wait
    }
  }
}

/* NeoPixel Functions */

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
  current_time = millis();

  colorWipe(RGB.Color(255, 0, 0), 50); // Red

  // One by one: CounterClockWise
  one_by_one_CCW(10);
  one_by_one_CCW(10);

  // One by one: ClockWise
  one_by_one_CW(10);
  one_by_one_CW(10);

  //
  one_by_one_in_two_directions(10);
  one_by_one_in_two_directions(10);

  two_directions(10, 20);

}