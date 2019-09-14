	
	

	#include <ESP8266WiFi.h>
	#include <WiFiClient.h> 
	#include <ESP8266WebServer.h> 
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



	
	/* Set these to your desired credentials. */
	const char *ssid = Sensorhuella";
	const char *password = "killerpa";
	int milisInterval = 2000;
	int count = 0;
	ESP8266WebServer server(80);
	

	/* Just a little test message.  Go to http://192.168.4.1 in a web browser
	 * connected to this access point to see it.
	 */
	void handleRoot() {
	Valorhuella = valorhuella;
//0 no detectada  cerrar y 1 si detectada y abrir
	String html ="<!DOCTYPE html> <html> <body> <h1>Light Level</h1> <head><meta http-equiv=\"refresh\" content=\"2\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"></head> <div id=\"light\">";
	html += Valorhuella;
	html +="</div><h1>Random Number</h1><div id=\"random\">";
	html += count;
	html +="</div> </body> </html>";
		server.send(200, "text/html", html);
	  count++;
	}
	

	

	

	void setup() {
		delay(1000);
		Serial.begin(115200);

		Serial.println();
		Serial.print("Configuring access point...");

	

#if defined(ARDUINO_ARCH_ESP32)
  /* RX = IO16, TX = IO17 */
  Serial.println("Soy ESP32");
  fserial.begin(9600, SERIAL_8N1, 16, 17);
#elif
  fserial.begin(115200);
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

/* You can remove the password parameter if you want the AP to be open. */




		WiFi.softAP(ssid, password);
	

		IPAddress myIP = WiFi.softAPIP();
		Serial.print("AP IP address: ");
		Serial.println(myIP);
		server.on("/", handleRoot);
		server.begin();
		Serial.println("HTTP server started");
	}
	

	void loop() {
	server.handleClient();
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
Bool valorhuella = ¡true;
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



	}
