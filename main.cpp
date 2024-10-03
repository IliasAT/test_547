#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

BLEServer *pServer = NULL;
BLECharacteristic *pLedCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;

const int ledPin = 2; // GPIO pin to control LED

#define SERVICE_UUID "5f3832be-6883-47a5-8fbb-d64c9fbf6d2a"
#define LED_CHARACTERISTIC_UUID "5f3832be-6883-47a5-8fbb-d64c9fbf6d2a"

class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer *pServer) {
    deviceConnected = true;
  }

  void onDisconnect(BLEServer *pServer) {
    deviceConnected = false;
  }
};

class MyCharacteristicCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pLedCharacteristic) {
    std::string ledValue = pLedCharacteristic->getValue();
    if (ledValue.length() > 0) {
      int receivedValue = static_cast<int>(ledValue[0]);
      if (receivedValue == 1) {
        digitalWrite(ledPin, HIGH); // Turn LED on
      } else {
        digitalWrite(ledPin, LOW); // Turn LED off
      }
    }
  }
};

void setup() {
  Serial.begin(9600);
  pinMode(ledPin, OUTPUT);

  // Create the BLE Device
  BLEDevice::init("ESP32_Ilias");

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic for LED control
  pLedCharacteristic = pService->createCharacteristic(
      LED_CHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_WRITE);

  // Register the callback for the LED characteristic
  pLedCharacteristic->setCallbacks(new MyCharacteristicCallbacks());

  // Start the service
  pService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x0); 
  BLEDevice::startAdvertising();
  Serial.println("Waiting for client connection to control LED...");
}

void loop() {
  // Handle disconnection and reconnection
  if (!deviceConnected && oldDeviceConnected) {
    Serial.println("Device disconnected.");
    pServer->startAdvertising();
    Serial.println("Start advertising");
    oldDeviceConnected = deviceConnected;
  }

  if (deviceConnected && !oldDeviceConnected) {
    Serial.println("Device Connected");
    oldDeviceConnected = deviceConnected;
  }
}
