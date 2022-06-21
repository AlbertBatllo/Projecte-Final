#include <Arduino.h>
// Lector de targetas
#include <SPI.h>
#include <MFRC522.h>
#define RST_PIN 17 
#define SS_PIN 4
 
// Wifi
#include <WiFi.h>
const char* ssid = "iPhone";
const char* password = "ajajaj222";

// Pagina web
#include <WebServer.h>
WebServer server(80);
 
// Zumbador
#include <FS.h>
#include <SD.h>
#define BUZZER 15

//Display
#include <Wire.h>
#include <Adafruit_I2CDevice.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
 
// Variables
int ledblanc = 12;
int ledverd = 13;
int ledvermell = 14;
bool correcte = true;
int zumbador = 15;
MFRC522 mfrc522(SS_PIN, RST_PIN);
 
// Funciones
void PICAR (); // Picar al timbre
void PASA (); // Esta sera la funcion que "habrira la puerta"
void COMPARAR();
void DENEGADO ();
void handle_root (void); // Pagina web

void setup() {
  Serial.begin(9600); //Iniciamos la comunicación serial
  SPI.begin(); //Iniciamos el Bus SPI
  mfrc522.PCD_Init(); // Iniciamos el MFRC522
  Serial.println("Lectura del UID");
  // Leds
  pinMode(ledblanc, OUTPUT);
  pinMode(ledverd, OUTPUT);
  pinMode(ledvermell, OUTPUT);
  pinMode (21, INPUT_PULLUP); // boton para abrir la puerta
  pinMode (5, INPUT_PULLUP); // timbre
  pinMode (BUZZER, OUTPUT); // zumbador
  // Zumbador
  digitalWrite(BUZZER,HIGH);
  // Wifi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
  delay(1000);
  Serial.print(".");
  }
  Serial.print("Got IP: ");
  Serial.println(WiFi.localIP()); //Show ESP32 IP on serial
  // Pagina web
  server.begin();
  server.on("/", handle_root);
  //Display
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println("SSD1306 allocation failed");
    for(;;);
  }

  delay(2000);
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
}

byte Usuario1[4]= {0x50, 0x60, 0x13, 0x4E} ; //código del usuario 1

void loop() {
  int timbre = digitalRead (5);
  int boto = digitalRead (21);
  // Comprobamos si los botones estan presionados y llamamos a sus respectivas funciones
  if (timbre==LOW){
     PICAR();
  }
  if (boto==LOW){
     PASA();
  }
  // Revisamos si hay nuevas tarjetas
  if ( mfrc522.PICC_IsNewCardPresent())
  {
      //Seleccionamos la tarjeta
      if ( mfrc522.PICC_ReadCardSerial())
      {
        // Encendemos el led de control (blanco)
        digitalWrite(ledblanc, HIGH);
        delay(500);
        digitalWrite(ledblanc, LOW);

        COMPARAR();
        if (correcte){
          PASA();
        }
        else {
          DENEGADO();
        }
      }
  }
  
}
// Esta funcion simula un timbre
void PICAR (){
  digitalWrite(BUZZER,HIGH);
  delay(1000);
  digitalWrite(BUZZER,LOW);
}
// Esta función encendera el led verde y la pagina web
void PASA (){
  // encendemos led verde
  digitalWrite(ledverd, HIGH);
  delay(500);               // el deixem ences un moment
  digitalWrite(ledverd, LOW);
  // Escribim pel display
  Serial.println("Targeta ACEPTADA");
  display.println("Bienvenido!");
  display.display();
   delay(4000);
  display.clearDisplay(); 
  // pag web 
  server.handleClient(); 
}
void COMPARAR(){
  correcte = true;
  for (byte i = 0; i < mfrc522.uid.size && correcte==true; i++) {
  // comparamos la targeta leida con la de ejemplo
    if (mfrc522.uid.uidByte[i] != Usuario1[i]){
        correcte = false;
    }
  }
}

void DENEGADO (){
// encendemos led roja
  digitalWrite(ledvermell, HIGH);
  delay(500);
  digitalWrite(ledvermell, LOW);
  Serial.println();
  // Terminamos la lectura de la tarjeta actual
  mfrc522.PICC_HaltA();
 Serial.println("Targeta DENEGADA");
 // Escribim pel display
 display.println("Targeta DENEGADA");
 display.display();
 delay(6000);
  display.clearDisplay(); 
}

String HTML ="<!DOCTYPE html>\
<html>\
  <head>\
  <meta charset='utf-8' />\
  <title>PORTAL </title>\
  </head>\
  <body>\
  <center>\
  <h2> <strong> ENTRADA PORTAL EDIFICIO E </strong> </h2>\
  <p> <strong>Acaba de entrar por la porteria el Usuario1</strong> </p>\
  <p> Propietario del 2º 1ª </p>\
  <br>\
  </center>\
  </body>\
</html>";

void handle_root(){
server.send(200, "text/html", HTML);
}
