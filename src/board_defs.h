#ifndef CONTROLLER_CONFIG_H
#define CONTROLLER_CONFIG_H

#define SW_GPIO_SIZE 12               // Number of switches
#define LED_GPIO_SIZE 12              // Number of switch LEDs
#define ENC_GPIO_SIZE 1               // Number of encoders
#define ENC_PPR 35                    // Encoder PPR
#define MOUSE_SENS 5                  // Mouse sensitivity multiplier
#define ENC_DEBOUNCE true            // Encoder Debouncing
#define SW_DEBOUNCE_TIME_US 4000      // Switch debounce delay in us
#define ENC_PULSE (ENC_PPR * 4)       // 4 pulses per PPR
#define REACTIVE_TIMEOUT_MAX 1000000  // HID to reactive timeout in us
#define WS2812B_LED_SIZE 10           // Number of WS2812B LEDs
#define WS2812B_LED_ZONES 2           // Number of WS2812B LED Zones
#define WS2812B_LEDS_PER_ZONE \
  WS2812B_LED_SIZE / WS2812B_LED_ZONES  // Number of LEDs per zone

#define TUD_MOUNT_TIMEOUT	1500
#define ENCODER_Z_MILLIS_TOLERANCE 150  // Amount of miliseconds to wait and change state of turntable buttons
#define ENC_DELTA 1                     // Normal direction - 1, reversed - -1, don't forget that oficial controller has inverted axis already

#define I2C_PORT i2c0
#define I2C_SCL 5
#define I2C_SDA 4
#define I2C_FREQ 733*1000

//Check psxSPI.pio for PlayStation mode pins and tusb_config.h for NFC module enable

#ifdef PICO_GAME_CONTROLLER_C

// MODIFY KEYBINDS HERE, MAKE SURE LENGTHS MATCH SW_GPIO_SIZE
const uint8_t SW_KEYCODE[] = {HID_KEY_Z, HID_KEY_S, HID_KEY_X, HID_KEY_D,
                              HID_KEY_C, HID_KEY_F, HID_KEY_V, HID_KEY_G,
                              HID_KEY_ENTER, HID_KEY_TAB};
const uint8_t SW_GPIO[] = {
    6, 7, 8, 9, 10, 11, 12, 2, 15, 14, 0, 1,
};
const uint8_t LED_GPIO[] = {
    21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21,
};
const uint8_t ENC_GPIO[] = {26};  // TT(26, 27)
const uint8_t WS2812B_GPIO = 23;

#endif

extern bool joy_mode_check;

#endif