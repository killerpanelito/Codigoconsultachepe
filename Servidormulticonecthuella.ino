/*
    Video: https://www.youtube.com/watch?v=oCMOYS71NIU
    Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleNotify.cpp
    Ported to Arduino ESP32 by Evandro Copercini
    updated by chegewara

   Create a BLE server that, once we receive a connection, will send periodic notifications.
   The service advertises itself as: 4fafc201-1fb5-459e-8fcc-c5c9c331914b
   And has a characteristic of: beb5483e-36e1-4688-b7f5-ea07361b26a8

   The design of creating the BLE server is:
   1. Create a BLE Server
   2. Create a BLE Service
   3. Create a BLE Characteristic on the Service
   4. Create a BLE Descriptor on the characteristic
   5. Start the service.
   6. Start advertising.

   A connect hander associated with the server starts a background task that performs notification
   every couple of seconds.
*/
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

//Codigo agregado!!!!!!!!!!!!!!!!!!!
#include <GT5X.h>
/* Enroll fingerprints */
#if defined(ARDUINO_ARCH_ESP32)
/* select UART1 for ESP32 */
HardwareSerial fserial(1);
#elif
/* RX = 2, TX = 3 */
SoftwareSerial fserial(2, 3);
#endif

GT5X finger(&fserial);
GT5X_DeviceInfo info;
 //Codigo agregado!!!!!!!!!!!!!!!!!

BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;
uint32_t value = 0;

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"


class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      BLEDevice::startAdvertising();
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};



void setup() {
  Serial.begin(115200);
  
  
  //CODIGO AREGADO !!!!!!!!!!!!!!!!
 Serial.println("Iniciando el sistema");
#if defined(ARDUINO_ARCH_ESP32)
  /* RX = IO16, TX = IO17 */
  Serial.println("Soy ESP32");
  fserial.begin(9600, SERIAL_8N1, 16, 17);
#elif
  fserial.begin(9600);
#endif
  
  if (finger.begin(&info)) {
    Serial.println("Found fingerprint sensor!");
    Serial.print("Firmware Version: "); Serial.println(info.fwversion);
    finger.set_led(true);
  }
    else {
    Serial.println("Sensor no encontrado :(");
    while (1) {
     // digitalWrite(LedIndicador, 0);
      delay(500);
    //  digitalWrite(LedIndicador, 1);
      delay(500);
    }

  }
  // Create the BLE Device
  BLEDevice::init("ESP32");

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY |
                      BLECharacteristic::PROPERTY_INDICATE
                    );

  // https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.descriptor.gatt.client_characteristic_configuration.xml
  // Create a BLE Descriptor
  pCharacteristic->addDescriptor(new BLE2902());

  // Start the service
  pService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x0);  // set value to 0x00 to not advertise this parameter
  BLEDevice::startAdvertising();
  Serial.println("Waiting a client connection to notify...");
}

void loop() {
    // notify changed value
    
    if (deviceConnected) {

      if (Serial.available()) {
      char Letra = Serial.read();
      Serial.write(Letra);
      Escanear(Letra);
      }
    
        BuscarHuella();

        pCharacteristic->setValue("MENSAJE A ENVIAR");
        pCharacteristic->notify();
        delay(500); // bluetooth stack will go into congestion, if too many packets are sent, in 6 hours test i was able to go as low as 3ms
    }
    // disconnecting
    if (!deviceConnected && oldDeviceConnected) {
        delay(500); // give the bluetooth stack the chance to get things ready
        pServer->startAdvertising(); // restart advertising
        Serial.println("start advertising");
        oldDeviceConnected = deviceConnected;
    }
    // connecting
    if (deviceConnected && !oldDeviceConnected) {
        // do stuff here on connecting
        oldDeviceConnected = deviceConnected;
    }
}


void BuscarHuella() {
  if (!finger.is_pressed()) {
    return;
  }
  uint16_t rc = finger.capture_finger();
  if (rc != GT5X_OK) {
    return;
  }
  uint16_t IDSecreto;
  rc = finger.search_database(&IDSecreto);
  if (rc != GT5X_OK) {
    Serial.println("No encontre la huella");
    return;
  }

  Serial.print("Encontrado el ID: ");
  Serial.println(IDSecreto);
  pCharacteristic->setValue("MENSAJE NUEVO AL ENCONTRAR HUELLA");
  pCharacteristic->notify();
        delay(500); 
  
  delay(1000);
  
}


void Escanear(char Letra) {
  //A CAMBIAR
  if (Letra == 'd' || Letra == 'D') {
    uint16_t rc = finger.delete_id(0);

    Serial.print(rc);
    switch (rc) {
      case GT5X_OK:
        Serial.println("Borrando huella 0");
        break;
      default:
        Serial.println("Error Fatal");
        break;
    }
  }
  else if (Letra == 'a' || Letra == 'A') {
    Enroll();
  }
  else if (Letra == 'c' || Letra == 'C') {
    uint16_t rc = finger.is_enrolled(0);
    switch (rc) {
      case GT5X_OK:
        Serial.println("Esta lleno");
        break;
      case GT5X_NACK_IS_NOT_USED:
        Serial.println("Esta vacio");
        break;
      default:
        Serial.print("Error code: 0x"); Serial.println(rc, HEX);
        break;
    }
  }
}

void Enroll() {
  uint16_t rc = finger.is_enrolled(0);

  switch (rc) {
    case GT5X_OK:
      Serial.println("Ya esta asignado");
      return;
    case GT5X_NACK_IS_NOT_USED:
      Serial.println("Disponible el ID #0");
      break;
    default:
      Serial.print("Error code: 0x"); Serial.println(rc, HEX);
      return;
  }
  enroll_finger(0);
}


void enroll_finger(uint16_t fid) {
  uint16_t p = finger.start_enroll(fid);
  switch (p) {
    case GT5X_OK:
      Serial.print("Enrolling ID #"); Serial.println(fid);
      break;
    default:
      Serial.print("No empezo a enroal, Error code: 0x"); Serial.println(p, HEX);
      return;
  }

  /* turn on led for print capture */
  finger.set_led(true);

  /* scan finger 3 times */
  for (int scan = 1; scan < 4; scan++) {
    // finger.set_led(true);
    Serial.println("Place your finger.");
    p = finger.capture_finger(true);

    while (p != GT5X_OK) {
      p = finger.capture_finger(true);
      switch (p) {
        case GT5X_OK:
          Serial.println("Image taken.");
          break;
        case GT5X_NACK_FINGER_IS_NOT_PRESSED:
          Serial.println(".");
          break;
        default:
          Serial.print("Error code: 0x"); Serial.println(p, HEX);
          //finger.set_led(false);
          return;
      }
      yield();
    }

    p = finger.enroll_scan(scan);
    switch (p) {
      case GT5X_OK:
        Serial.print("Scan "); Serial.print(scan);
        Serial.println(" complete.");
        break;
      default:
        Serial.print("Error code: 0x"); Serial.println(p, HEX);
        //finger.set_led(false);
        return;
    }

    Serial.println("Remove finger.");
    //finger.set_led(false);
    while (finger.is_pressed()) {
      yield();
    }
    Serial.println();
  }

  /* wr're done so turn it off */
  //finger.set_led(false);

  Serial.println("Enroll complete.");
  return;
}
