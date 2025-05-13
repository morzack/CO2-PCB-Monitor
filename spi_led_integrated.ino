
#include <Arduino.h>
#include <SensirionI2CScd4x.h>
#include <Wire.h>

SensirionI2CScd4x scd4x;


// 11: MOSI/DIN
// 13: SCK/CLK
// 9: !CS (load)
// last 16 bits of serial data are latched on LOAD's rising edge

// to load data, take LOAD low, serially transmit the 16 bits of data, then set load high

// spi_clk -- PB5 = dig 13
// spi_mosi -- PB3 = dig 11
// spi_cs_max -- PB1 = dig 9
#define PIN_DIN 11
#define PIN_CLK 13
#define PIN_LOAD 9

#define PIN_EN_3V 2 // PD2 = dig 2

// #define PIN_DIN 11
// #define PIN_CLK 13
// #define PIN_LOAD 8

// #define PIN_CLK 6
// #define PIN_LOAD 5
// #define PIN_DIN 13

void send_bit(uint8_t bit);
void send_data_raw(uint16_t dat);
void write_reg(uint8_t reg, uint8_t dat);

#define CHAR_0 0x0
#define CHAR_1 0x1
#define CHAR_2 0x2
#define CHAR_3 0x3
#define CHAR_4 0x4
#define CHAR_5 0x5
#define CHAR_6 0x6
#define CHAR_7 0x7
#define CHAR_8 0x8
#define CHAR_9 0x9
#define CHAR_DASH 0xA
#define CHAR_E 0xB
#define CHAR_H 0xC
#define CHAR_L 0xD
#define CHAR_P 0xE
#define CHAR_BLANK 0xF

void write_digit(uint8_t digit, uint8_t character);

void make_co2_reading();
uint16_t last_co2;
float last_humid;
float last_temp;

void setup() {
  Wire.begin();
  uint16_t error;

  pinMode(PIN_DIN, OUTPUT);
  pinMode(PIN_CLK, OUTPUT);
  pinMode(PIN_LOAD, OUTPUT);
  pinMode(PIN_EN_3V, OUTPUT);

  digitalWrite(PIN_DIN, LOW);
  digitalWrite(PIN_CLK, LOW);
  digitalWrite(PIN_LOAD, HIGH);
  digitalWrite(PIN_EN_3V, LOW);

  // soft bringup the max chip
  delay(1000);
  digitalWrite(PIN_EN_3V, HIGH);
  delay(2000);

  // turn on only digit 0
  write_reg(0x9, 0xFF); // decode mode: 
  write_reg(0xA, 0x0F); // intensity
  write_reg(0xB, 0x03); // 0x3 for 4 digits
  write_reg(0xC, 0x01); // shutdown

  delay(1000);

  // bring up co2 sense
  scd4x.begin(Wire);
  error = scd4x.stopPeriodicMeasurement();
  error = scd4x.startLowPowerPeriodicMeasurement();

  write_num(0);
}

void loop() {
  // cycle through test digits
  // uint8_t x = 0;
  // while (x <= 9) {    
  //   write_digit(0, x);

  //   if (x == 9) {
  //     x = 0;
  //   }
  //   x++;

  //   delay(1000);
  // }

  // uint16_t a = 0;
  // uint16_t inc = 59;
  // while (a <= 9999) {
  //   write_num(a);
  //   if (a > 9999) {
  //     a %= 9999;
  //   }
  //   else {
  //     a += inc;
  //   }
  //   delay(250);
  // }

  make_co2_reading();
  write_num(last_co2);

  unsigned long startMillis = millis();
  while (millis() - startMillis < (1000UL * 60UL * 5UL)); // wait 5m between readings
}


void make_co2_reading() {
  bool is_data_ready = false;
  uint16_t err = scd4x.getDataReadyFlag(is_data_ready);
  if (err) {
    return;
  }

  if (!is_data_ready) {
    return;
  }

  uint16_t tmp_co2;
  err = scd4x.readMeasurement(tmp_co2, last_temp, last_humid);
  if (err) {
    return;
  }

  if (tmp_co2 != 0) {
    last_co2 = tmp_co2;
  }
}

void send_bit(uint8_t bit) {
  // bring data to value
  digitalWrite(PIN_DIN, bit == 0 ? LOW : HIGH);
  // wait a bit
  delay(1); // 1
  // bring clock high to latch value
  digitalWrite(PIN_CLK, HIGH);
  delay(1); // 3
  
  // bring clock low/hold, prep for next
  digitalWrite(PIN_CLK, LOW);
  delay(1); // 4
}

void send_data_raw(uint16_t dat) {
  digitalWrite(PIN_CLK, LOW);
  digitalWrite(PIN_LOAD, LOW);
  digitalWrite(PIN_DIN, LOW);
  delay(1); // 50
  for (uint8_t i=0; i<16; i++) {
    uint8_t b = (dat>>(15-i)) & 0x01;
    send_bit(b);
  }
  digitalWrite(PIN_LOAD, HIGH);
  digitalWrite(PIN_CLK, LOW);
  digitalWrite(PIN_DIN, LOW);
}

void write_reg(uint8_t reg, uint8_t dat) {
  uint16_t sending = 0x00;
  sending |= dat;
  sending |= (reg << 8);
  send_data_raw(sending);
}

void write_digit(uint8_t digit, uint8_t character) {
  // assume digit is zero-indexed
  write_reg(digit+1, character);
}

void write_num(uint16_t num) {
  uint8_t a = num % 10;
  uint8_t b = (num/10) % 10;
  uint8_t c = (num/100) % 10;
  uint8_t d = (num/1000) % 10;
  write_digit(3, a);
  write_digit(2, b);
  write_digit(1, c);
  write_digit(0, d);
}