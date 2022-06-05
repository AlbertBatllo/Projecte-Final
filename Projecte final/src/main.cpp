#include <Arduino.h>
// Lector de targetas
#include <SPI.h>
#include <MFRC522.h>
#define RST_PIN 17 //Pin 9 para el reset del RC522
#define SS_PIN 4 //Pin 10 para el SS (SDA) del RC522
 
// Wifi
String ssid =     "*******";
String password = "*******";
 
// Zumbador
#include <EasyBuzzer.h>
 
// Display
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_I2CDevice.h"
#include <Adafruit_NeoPixel.h>
 
#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64
#define I2Cdisplay_SDA 22
#define I2Cdisplay_SCL 16
TwoWire I2Cdisplay = TwoWire(1);
// Variables
int ledblanc = 12;
int ledverd = 13;
int ledvermell = 14;
bool correcte = true;
bool si = true;
int zumbador = 2;
MFRC522 mfrc522(SS_PIN, RST_PIN);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &I2Cdisplay, -1);
 
// Funciones
void PICAR (); //Picar al timbre
void PASA (); //Esta sera la funcion que "abrira la puerta"
void COMPARAR(); //Compara el pin de la targeta con los que tiene guardados
void DENEGADO (); //Funcion que se activa cuando la targeta es incorrecta
void PANTALLA (); //Funcion que hace el display
//void PANTALLA(bool si);
void PAGWEB ();

void setup() {
  Serial.begin(115200); //Iniciamos la comunicación serial
  SPI.begin(); //Iniciamos el Bus SPI
  mfrc522.PCD_Init(); // Iniciamos el MFRC522
  Serial.println("Lectura del UID");
  // Leds
  pinMode(ledblanc, OUTPUT);
  pinMode(ledverd, OUTPUT);
  pinMode(ledvermell, OUTPUT);
  pinMode (21, INPUT_PULLUP); // boton para abrir la puerta
  pinMode (15, INPUT_PULLUP); // timbre
  pinMode (2, OUTPUT); // zumbador
  // Zumbador
  //configuracion vacia (){ // de forma predeterminada, la libreria esta configurada para usar el pin 4, cabiamos de pin llamando a la configurazion
  EasyBuzzer.setPin(zumbador);
  //Display
  I2Cdisplay.begin(I2Cdisplay_SDA, I2Cdisplay_SCL, 100000); // inicializamos el display
  Wire.begin();
  while (!Serial);
  Serial.println("\nI2C Scanner");
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
  Serial.println(F("SSD1306 allocation failed"));
  for(;;); // Don't proceed, loop forever
  display.display();
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.drawPixel(10, 10, SSD1306_WHITE);
 
}
byte Usuario1[4]= {0x50, 0x60, 0x13, 0x4E} ; //código del usuario 1
void loop() {
  int timbre = digitalRead (15);
  int boto = digitalRead (21);
  // Comprobamos si los botones estan presionados y llamamos a sus respectivas funciones
  if (timbre==LOW){
     PICAR();
  }
  if (boto==LOW){
     PASA();
  }
  // Revisamos si hay nuevas tarjetas presenfalsetes
  if ( mfrc522.PICC_IsNewCardPresent()){
      //Seleccionamos una tarjeta
      if ( mfrc522.PICC_ReadCardSerial())
      {
        // Encendemos el led de control (blanco)
        digitalWrite(ledblanc, HIGH);
        delay(500);               // el deixem ences un moment
        digitalWrite(ledblanc, LOW);
        // Enviamos serialemente su UID
        Serial.print("Card UID:"); //..........................POSIBLE DISPLAY
        COMPARAR();
        if (correcte){
          PASA();
        }
        else {
          DENEGADO();
        }
      }
  }
// Esta funcion simula un timbre
void PICAR (){
  EasyBuzzer.beep (1000, 900000); // frequencia en Hz, duracion del pitido en ms
  //EasyBuzzer.setOnDuration (900000);
  EasyBuzzer.stopBeep();
}
// Esta función encendera el led verde y la pagina web...................................................................................................
void PASA (){
  // encendemos led verde
  digitalWrite(ledverd, HIGH);
  delay(500);               // el deixem ences un moment
  digitalWrite(ledverd, LOW);
  si = true;
  //PANTALLA(si);
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
  // mostramos por pantalla#include <Arduino.h>
 
void loop() {
 // put your main code here, to run repeatedly:
  byte i;
  Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
  Serial.print(mfrc522.uid.uidByte[i], HEX);
  // Terminamos la lectura de la tarjeta actual
  mfrc522.PICC_HaltA();
}
void DENEGADO (){
// encendemos led roja
  digitalWrite(ledvermell, HIGH);
  delay(500);
  digitalWrite(ledvermell, LOW);
  Serial.println();//display.drawPixel(10, 10, SSD1306_WHITE);
  // Terminamos la lectura de la tarjeta actual
  mfrc522.PICC_HaltA();
  si = false;
 //PANTALLA(si);
}
 
 void PANTALLA(bool si){
 display.clearDisplay();
 display.drawPixel(10, 10, SSD1306_WHITE);
 display.setTextColor(WHITE);
 display.setTextSize(5);
 display.setCursor(19,5);
 if (si){
   display.print("Bienvenido");
 }
 else{
   display.print("Tarjeta incorrecta");
 }
 }
}