/*
    Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleServer.cpp
    Ported to Arduino ESP32 by Evandro Copercini
    updates by chegewara
*/

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
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

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

void setup() {
  Serial.begin(115200);
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
  } else {
    Serial.println("Sensor no encontrado :(");
    while (1) {
      digitalWrite(LedIndicador, 0);
      delay(500);
      digitalWrite(LedIndicador, 1);
      delay(500);
    }

  }
  
  BLEDevice::init("El sistema de conexion esta enviando informacion satisfactoriamente");
  BLEServer *pServer = BLEDevice::createServer();
  BLEService *pService = pServer->createService(SERVICE_UUID);
  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE
                                       );

  pCharacteristic->setValue("Hello World says Neil");
  pService->start();
  // BLEAdvertising *pAdvertising = pServer->getAdvertising();  // this still is working for backward compatibility
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  Serial.println("Characteristic defined! Now you can read it in your phone!");
}

void loop() {
  // Aqui deberia poner la logica de esperar a presionar los botones y estar pendiente de presionar el lector
  
  delay(2000);
}

void BuscarHuella() {
  if (!finger.is_pressed()) {
    return;
  }
  uint16_t rc = finger.capture_finger();
  //finger.set_led(true);
  if (rc != GT5X_OK) {
    return;
  }
  uint16_t IDSecreto;
  rc = finger.search_database(&IDSecreto);
  if (rc != GT5X_OK) {
    Serial.println("No encontre la huella");
    //SerialBT.print('F');
    return;
  }

  Serial.print("Encontrado el ID: ");
  Serial.println(IDSecreto);
    //finger.set_led(false);
}

void Escanear(char Letra) {
  //SE NECESITA CAMBIAR PARA FUNCIONAR A BASE DE EL BOTON ROJO!!!!if (Letra == 'd' || Letra == 'D') {
    uint16_t rc = finger.delete_id(0);

    Serial.print(rc);
    switch (rc) {
      case GT5X_OK:
        Serial.println("Borrando huella 0");
        SerialBT.print('T');
        break;
      default:
        Serial.println("Error Fatal");
        SerialBT.print('F');
        break;
    }
  }
 // eSTA OPCION TIENE QUE CONECTARSE CON EL CONDICIONAL DEL BOTON BLANCO!!! else if (Letra == 'a' || Letra == 'A') {
    Enroll();
  }
  else if (Letra == 'c' || Letra == 'C') {
    uint16_t rc = finger.is_enrolled(0);
    switch (rc) {
      case GT5X_OK:
        Serial.println("Esta lleno");
        //SerialBT.print('L');
        break;
      case GT5X_NACK_IS_NOT_USED:
        Serial.println("Esta vacio");
        //SerialBT.print('V');
        break;
      default:
        Serial.print("Error code: 0x"); Serial.println(rc, HEX);
        //SerialBT.print('F');
        break;
    }
  }
}

void Enroll() {
  uint16_t rc = finger.is_enrolled(0);

  switch (rc) {
    case GT5X_OK:
      Serial.println("Ya esta asignado");
      //SerialBT.print('F');
      return;
    case GT5X_NACK_IS_NOT_USED:
      Serial.println("Disponible el ID #0");
      break;
    default:
      Serial.print("Error code: 0x"); Serial.println(rc, HEX);
      //SerialBT.print('F');
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
    //SerialBT.print('P');
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
          //SerialBT.print('F');
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
        //SerialBT.print('F');
        //finger.set_led(false);
        return;
    }

    Serial.println("Remove finger.");
    //finger.set_led(false);
    //SerialBT.print('R');
    while (finger.is_pressed()) {
      yield();
    }
    Serial.println();
  }

  /* wr're done so turn it off */
  //finger.set_led(false);

  Serial.println("Enroll complete.");
  //SerialBT.print('T');
  return;
}
