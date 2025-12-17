/*
 * IIDX Controller ESP32 Part
 * Silver Sword <github.com/SilverSword95>
 */

#include <BLECharacteristic.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#define DEVICE_NAME "IIDX Entry model"
#define SERVICE_UUID "ff00"
#define CHARACTERISTIC_UUID "ff01"

#define RX_PIN 16  // UART RX (connected to TX Raspberry Pi Pico)
#define TX_PIN 17  // UART TX (usually not used, only for debug)

#define UART_BAUD 115200
#define UART_TIMEOUT_MS 10000  // 10 seconds of waiting for first package

BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;

bool deviceConnected = false;
bool oldDeviceConnected = false;
bool firstConnect = true;

uint8_t frameCounter = 0;
unsigned long lastDataTime = 0;
const unsigned long DATA_TIMEOUT_MS = 200; // if 200 ms no data - fill null data
const uint8_t START_BYTE = 0xAA;
uint8_t values[10];

uint8_t last_axis0 = 127;
uint8_t last_btn_low7 = 127;
uint8_t last_btn_high7 = 0;

class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
    //Serial.println("Device connected");
  }
  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
    //Serial.println("Device disconnected");
  }
};

void setup() {
  //Serial.begin(115200);
  Serial2.begin(UART_BAUD, SERIAL_8N1, RX_PIN, TX_PIN);
  //Serial.println("ESP32 UART init... waiting for data.");
  
  unsigned long start = millis();
  bool gotData = false;
  
  // Waiting 10 seconds of data appearing
  while (millis() - start < UART_TIMEOUT_MS) {
    if (Serial2.available()) {
      gotData = true;
      break;
    }
    delay(10);
  }
  
  if (!gotData) {
    //Serial.println("No data for 10 seconds. Going to deep sleep.");
    esp_deep_sleep_start();
  }

  // Create the BLE Device
  BLEDevice::init(DEVICE_NAME);
  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService* pService = pServer->createService(SERVICE_UUID);
  // Create a BLE Characteristic
  pCharacteristic = pService->createCharacteristic(
    CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_READ |
    BLECharacteristic::PROPERTY_WRITE |
    BLECharacteristic::PROPERTY_NOTIFY);
  pCharacteristic->addDescriptor(new BLE2902());
  // Start the service
  pService->start();

  // Start advertising
  BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  BLEDevice::startAdvertising();

  //Serial.println("Waiting a client connection to notify...");
}

void loop() {
  static uint8_t buf[4];
  static uint8_t index = 0;
  static unsigned long lastNotify = 0;
  uint8_t count = 0;
  
  //Reading UART per one byte
  while (Serial2.available() && count < 10) {
    uint8_t b = Serial2.read();

    if (index == 0) {
      if (b != START_BYTE) continue; // waiting for start byte
    }

    buf[index++] = b;

    if (index == 4) {
      // full package is ready
      last_axis0     = buf[1];
      last_btn_low7  = buf[2];
      last_btn_high7 = buf[3];

      lastDataTime = millis();
      index = 0; // waiting for next package
    }
	count++;
  }
  
  //If Pico doesn't send data - fill null data instead
  if (millis() - lastDataTime > DATA_TIMEOUT_MS) {
    last_axis0     = 127;
    last_btn_low7  = 0;
    last_btn_high7 = 0;
    lastDataTime = millis();
  }
  
  //BLE notify every ~10 ms
  if (deviceConnected && millis() - lastNotify > 10) {
	values[0] = last_axis0;
    values[1] = 127;
    values[2] = last_btn_low7;
    values[3] = last_btn_high7;
    values[4] = frameCounter++;
    values[5] = last_axis0;
    values[6] = 127;
    values[7] = last_btn_low7;
    values[8] = last_btn_high7;
    values[9] = frameCounter++;
    pCharacteristic->setValue(values, 10);
    pCharacteristic->notify();
    lastNotify = millis();
  }

  // disconnecting
  if (!deviceConnected && oldDeviceConnected) {
    delay(500);                   // give the bluetooth stack the chance to get things ready
    pServer->startAdvertising();  // restart advertising
    //Serial.println("Start advertising...");
    oldDeviceConnected = deviceConnected;
    frameCounter = 0;
  }

  // connecting
  if (deviceConnected && !oldDeviceConnected) {
    // do stuff here on connecting
    oldDeviceConnected = deviceConnected;
    frameCounter = 0;
  }
}