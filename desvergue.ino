/*********
  Rui Santos
  Complete project details at https://randomnerdtutorials.com  
*********/

// Load Wi-Fi library
#include <WiFi.h>
#include <GT5X.h>
#if defined(ARDUINO_ARCH_ESP32)
#include <HardwareSerial.h>
#elif
#include <SoftwareSerial.h>
#endif
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

// Replace with your network credentials
const char* ssid     = "Servidor sensor huellas test";
const char* password = "killerpa";

// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

// Auxiliar variables to store the current output state
String output26State = "off";
String output27State = "off";

// Assign output variables to GPIO pins
const int output26 = 26;
const int output27 = 27;
String HUELLA = "false";

void setup() {
   Serial.begin(115200);
   Serial.println("Iniciando sistema");
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
    //finger.set_led(true);
  } else {
    Serial.println("Sensor no encontrado :(");
    while (1) {
      //digitalWrite(LedIndicador, 0);
      delay(500);
      //digitalWrite(LedIndicador, 1);
      delay(500);
    }

  }


  // Connect to Wi-Fi network with SSID and password
  Serial.print("Iniciando el punto de acceso…");
  // Remove the password parameter, if you want the AP (Access Point) to be open
  WiFi.softAP(ssid, password);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);
  
  server.begin();
}

void loop(){


  if (Serial.available()) {
    char Letra = Serial.read();
    Serial.write(Letra);
    Escanear(Letra);
  }

  BuscarHuella();

  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,
    Serial.println("New Client.");          // print a message out in the serial port
     BuscarHuella();
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
      
            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons 
            // Feel free to change the background-color and font-size attributes to fit your preferences
    
            // Web Page Heading
            client.println("<body><h1>ESP32 Web Server</h1>");
            
            // Display current state, and ON/OFF buttons for GPIO 26  
            client.println("<p>GPIO 26 - State " + HUELLA + "</p>");
          
            client.println("</body></html>");
            
            // The HTTP response ends with another blank line
            client.println();
             BuscarHuella();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
     BuscarHuella();
    // Close the connection
  }
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
  HUELLA= "true";
    //finger.set_led(false);
}

void Escanear(char Letra) {
if (Letra == 'd' || Letra == 'D') {
    uint16_t rc = finger.delete_id(0);

    Serial.print(rc);
    switch (rc) {
      case GT5X_OK:
        Serial.println("Borrando huella 0");
        //SerialBT.print('T');
        break;
      default:
        Serial.println("Error Fatal");
        //SerialBT.print('F');
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
