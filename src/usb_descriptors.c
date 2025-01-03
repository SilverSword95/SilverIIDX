/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include "usb_descriptors.h"
#include "pico/unique_id.h"
#include "tusb.h"

/* A combination of interfaces must have a unique product id, since PC will save
 * device driver after the first plug. Same VID/PID with different interface e.g
 * MSC (first), then CDC (later) will possibly cause system error on PC.
 *
 * Auto ProductID layout's Bitmap:
 *   [MSB]         HID | MSC | CDC          [LSB]
 */

//--------------------------------------------------------------------+
// Device Descriptors
//--------------------------------------------------------------------+
tusb_desc_device_t const desc_device_joy = {
    .bLength = sizeof(tusb_desc_device_t),
    .bDescriptorType = TUSB_DESC_DEVICE,
    .bcdUSB = 0x0200,
    .bDeviceClass = 0x00,
    .bDeviceSubClass = 0x00,
    .bDeviceProtocol = 0x00,
    .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,

    .idVendor = 0x1ccf,
    .idProduct = 0x8048,
    .bcdDevice = 0x0100,

    .iManufacturer = 0x01,
    .iProduct = 0x02,
    .iSerialNumber = 0x03,

    .bNumConfigurations = 0x01};

tusb_desc_device_t const desc_device_key = {
    .bLength = sizeof(tusb_desc_device_t),
    .bDescriptorType = TUSB_DESC_DEVICE,
    .bcdUSB = 0x0200,
    .bDeviceClass = 0x00,
    .bDeviceSubClass = 0x00,
    .bDeviceProtocol = 0x00,
    .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,

    .idVendor = 0x1ccf,
    .idProduct = 0x8048,
    .bcdDevice = 0x0100,

    .iManufacturer = 0x01,
    .iProduct = 0x02,
    .iSerialNumber = 0x04,

    .bNumConfigurations = 0x01};

// Invoked when received GET DEVICE DESCRIPTOR
// Application return pointer to descriptor
uint8_t const* tud_descriptor_device_cb(void) {
  return (uint8_t const*)(joy_mode_check ? &desc_device_joy : &desc_device_key);
}

//--------------------------------------------------------------------+
// HID Report Descriptor
//--------------------------------------------------------------------+

uint8_t const desc_hid_report_joy[] = {
    GAMECON_REPORT_DESC_JOYSTICK(HID_REPORT_ID(REPORT_ID_JOYSTICK)),
    GAMECON_REPORT_DESC_LIGHTS(HID_REPORT_ID(REPORT_ID_LIGHTS))};

uint8_t const desc_hid_report_key[] = {
    GAMECON_REPORT_DESC_LIGHTS(HID_REPORT_ID(REPORT_ID_LIGHTS)),
    GAMECON_REPORT_DESC_NKRO(HID_REPORT_ID(REPORT_ID_KEYBOARD)),
    TUD_HID_REPORT_DESC_MOUSE(HID_REPORT_ID(REPORT_ID_MOUSE))};

#ifdef NFC_MODULE	
uint8_t const desc_hid_report_cardio[] = {
    GAMECON_REPORT_DESC_CARDIO,
};
#endif

// Invoked when received GET HID REPORT DESCRIPTOR
// Application return pointer to descriptor
// Descriptor contents must exist long enough for transfer to complete
#ifdef NFC_MODULE
uint8_t const* tud_hid_descriptor_report_cb(uint8_t itf) {
  switch (itf) {
        case 0:
            return (joy_mode_check ? desc_hid_report_joy : desc_hid_report_key);
        case 1:
            return desc_hid_report_cardio;
        default:
            return NULL;
    }
}
#else
uint8_t const* tud_hid_descriptor_report_cb(uint8_t itf) {
  (void)itf;
  return (joy_mode_check ? desc_hid_report_joy : desc_hid_report_key);
}
#endif

//--------------------------------------------------------------------+
// Configuration Descriptor
//--------------------------------------------------------------------+

#ifdef NFC_MODULE
enum { ITF_NUM_HID, ITF_NUM_CARDIO, ITF_NUM_TOTAL };
#define CONFIG_TOTAL_LEN (TUD_CONFIG_DESC_LEN + TUD_HID_DESC_LEN * 2)
#else
enum { ITF_NUM_HID, ITF_NUM_TOTAL };
#define CONFIG_TOTAL_LEN (TUD_CONFIG_DESC_LEN + TUD_HID_DESC_LEN)
#endif

#define EPNUM_HID 0x81
#define EPNUM_CARDIO 0x82

#ifdef NFC_MODULE
uint8_t const desc_configuration_joy[] = {
    // Config number, interface count, string index, total length, attribute,
    // power in mA
    TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, CONFIG_TOTAL_LEN,
                          TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, 100),

    // Interface number, string index, protocol, report descriptor len, EP In
    // address, size & polling interval
    TUD_HID_DESCRIPTOR(ITF_NUM_HID, 0, HID_ITF_PROTOCOL_NONE,
                       sizeof(desc_hid_report_joy), EPNUM_HID,
                       CFG_TUD_HID_EP_BUFSIZE, 1),
					   
	TUD_HID_DESCRIPTOR(ITF_NUM_CARDIO, 22, HID_ITF_PROTOCOL_NONE,
                       sizeof(desc_hid_report_cardio), EPNUM_CARDIO,
                       CFG_TUD_HID_EP_BUFSIZE, 1)};

uint8_t const desc_configuration_key[] = {
    // Config number, interface count, string index, total length, attribute,
    // power in mA
    TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, CONFIG_TOTAL_LEN,
                          TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, 100),

    // Interface number, string index, protocol, report descriptor len, EP In
    // address, size & polling interval
    TUD_HID_DESCRIPTOR(ITF_NUM_HID, 0, HID_ITF_PROTOCOL_NONE,
                       sizeof(desc_hid_report_key), EPNUM_HID,
                       CFG_TUD_HID_EP_BUFSIZE, 1),
					   
	TUD_HID_DESCRIPTOR(ITF_NUM_CARDIO, 22, HID_ITF_PROTOCOL_NONE,
                       sizeof(desc_hid_report_cardio), EPNUM_CARDIO,
                       CFG_TUD_HID_EP_BUFSIZE, 1)};
#else
uint8_t const desc_configuration_joy[] = {
    // Config number, interface count, string index, total length, attribute,
    // power in mA
    TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, CONFIG_TOTAL_LEN,
                          TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, 100),

    // Interface number, string index, protocol, report descriptor len, EP In
    // address, size & polling interval
    TUD_HID_DESCRIPTOR(ITF_NUM_HID, 0, HID_ITF_PROTOCOL_NONE,
                       sizeof(desc_hid_report_joy), EPNUM_HID,
                       CFG_TUD_HID_EP_BUFSIZE, 1)};

uint8_t const desc_configuration_key[] = {
    // Config number, interface count, string index, total length, attribute,
    // power in mA
    TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, CONFIG_TOTAL_LEN,
                          TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, 100),

    // Interface number, string index, protocol, report descriptor len, EP In
    // address, size & polling interval
    TUD_HID_DESCRIPTOR(ITF_NUM_HID, 0, HID_ITF_PROTOCOL_NONE,
                       sizeof(desc_hid_report_key), EPNUM_HID,
                       CFG_TUD_HID_EP_BUFSIZE, 1)};
#endif

// Invoked when received GET CONFIGURATION DESCRIPTOR
// Application return pointer to descriptor
// Descriptor contents must exist long enough for transfer to complete
uint8_t const* tud_descriptor_configuration_cb(uint8_t index) {
  (void)index;  // for multiple configurations
  return (joy_mode_check ? desc_configuration_joy : desc_configuration_key);
}

//--------------------------------------------------------------------+
// String Descriptors
//--------------------------------------------------------------------+

static char serial_number_str[24] = "123456\0";

// array of pointer to string descriptors
#ifdef NFC_MODULE
char const* string_desc_arr[] = {
    (const char[]){0x09, 0x04},  // 0: is supported language is English (0x0409)
    "Konami Amusement",          // 1: Manufacturer
    "beatmania IIDX controller premium model",  // 2: Product
    serial_number_str,  // 3: Serials, should use chip ID
    "Key 1",
    "Key 2",
    "Key 3",
    "Key 4",
    "Key 5",
    "Key 6",
    "Key 7",
    "UNUSED GPIO",
    "Key E1",
    "Key E2",
    "Key E3",
    "Key E4",
    "Red 1",
    "Green 1",
    "Blue 1",
    "Red 2",
    "Green 2",
    "Blue 2",
    "IIDX CardIO", //22: NFC String
};
#else
char const* string_desc_arr[] = {
    (const char[]){0x09, 0x04},  // 0: is supported language is English (0x0409)
    "Konami Amusement",          // 1: Manufacturer
    "beatmania IIDX controller premium model",  // 2: Product
    serial_number_str,  // 3: Serials, should use chip ID
    "Key 1",
    "Key 2",
    "Key 3",
    "Key 4",
    "Key 5",
    "Key 6",
    "Key 7",
    "UNUSED GPIO",
    "Key E1",
    "Key E2",
    "Key E3",
    "Key E4",
    "Red 1",
    "Green 1",
    "Blue 1",
    "Red 2",
    "Green 2",
    "Blue 2",
};
#endif

static uint16_t _desc_str[64];

// Invoked when received GET STRING DESCRIPTOR request
// Application return pointer to descriptor, whose contents must exist long
// enough for transfer to complete
uint16_t const* tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
  (void)langid;

  uint8_t chr_count;

  if (index == 0) {
    memcpy(&_desc_str[1], string_desc_arr[0], 2);
    chr_count = 1;
  } else if (index == 3) {
	pico_unique_board_id_t board_id;
    pico_get_unique_board_id(&board_id);
    sprintf(serial_number_str, "%016llx", *(uint64_t *)&board_id);
  }	else {
    // Note: the 0xEE index string is a Microsoft OS 1.0 Descriptors.
    // https://docs.microsoft.com/en-us/windows-hardware/drivers/usbcon/microsoft-defined-usb-descriptors

    if (!(index < sizeof(string_desc_arr) / sizeof(string_desc_arr[0])))
      return NULL;

    const char* str = string_desc_arr[index];

    // Cap at max char
    chr_count = strlen(str);
    if (chr_count > 63) chr_count = 63;

    // Convert ASCII string into UTF-16
    for (uint8_t i = 0; i < chr_count; i++) {
      _desc_str[1 + i] = str[i];
    }
  }

  // first byte is length (including header), second byte is string type
  _desc_str[0] = (TUSB_DESC_STRING << 8) | (2 * chr_count + 2);

  return _desc_str;
}
