#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// The remote service we wish to connect to.
static BLEUUID serviceUUID("4fafc201-1fb5-459e-8fcc-c5c9c331914b");
// The characteristic of the remote service we are interested in.
static BLEUUID charUUID("beb5483e-36e1-4688-b7f5-ea07361b26a8");

BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;
uint8_t valueToSend = 0;

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};

void setup() {
  Serial.begin(115200);

  // Create the BLE Device
  BLEDevice::init("Speechster-1000");
  
  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(serviceUUID);

  // Create a BLE Characteristic
  pCharacteristic = pService->createCharacteristic(
                      charUUID,
                      BLECharacteristic::PROPERTY_READ |
                      BLECharacteristic::PROPERTY_WRITE |
                      BLECharacteristic::PROPERTY_NOTIFY
                    );
  
  // Create a BLE Descriptor for the characteristic
  pCharacteristic->addDescriptor(new BLE2902());

  // Set the initial value for the characteristic
  pCharacteristic->setValue(&valueToSend, 1);
  
  // Start the service
  pService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(serviceUUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferredScanResponseTime(500);
  pAdvertising->setMinPreferredScanResponseTime(1000);
  BLEDevice::startAdvertising();
  Serial.println("Waiting for a client to connect...");
}

void loop() {
    // If a new client is connected
    if (deviceConnected) {
      // Notify the client of a value change
      valueToSend++;
      pCharacteristic->setValue(&valueToSend, 1);
      pCharacteristic->notify();
      Serial.print("Notifying client with value: ");
      Serial.println(valueToSend);
      delay(1000); // Wait for a second
    }

    // Disconnect handling
    if (!deviceConnected && oldDeviceConnected) {
        delay(500); // Give the BLE stack time to recover
        pServer->startAdvertising(); // Restart advertising
        Serial.println("Advertising restarted.");
        oldDeviceConnected = deviceConnected;
    }

    // Connect handling
    if (deviceConnected && !oldDeviceConnected) {
        // do stuff once on connection
        oldDeviceConnected = deviceConnected;
    }
}