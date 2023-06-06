# Projecte-Final: Despertador LED

El nostre projecte consisteix en el funcionament d'un despertador amb llums LED, el qual té les següents funcions:

- Donar la hora digital amb tires de 7 LEDs
- Poder fixar una alarma a partir d'una pàgina web
- Poder canviar el color dels LEDs a partir de la mateixa pàgina web
- Fer que al sonar l'alarma s'activi un fitxer d'audio localitzat a una micro SD

## Diagrama de blocs

![image](https://github.com/AlbertBatllo/Projecte-Final/assets/100155905/96654b81-50cd-4446-a813-441cac2e833d)

## Esquema de pins


# Codi

```
#include <Arduino.h>
#include <WebServer.h>
#include <WiFi.h>
#include<Adafruit_NeoPixel.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include "Audio.h"
#include "SD.h"
#include "FS.h"

//Variables programa
int udantic1=10;
int udantic2=10;
int hantic1=10;
int hantic2=10;

int posValue1; 
  int posValue2;
  int endValue1;
  int endValue2;

  int value1=25;
  int value2=61;


int R=0;
int G=0;
int B=100;
int oR=0;
int oG=0;
int oB=100;
bool sonando=false;

//Variables server
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0; 
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

//const char *ssid     = "Redmi Note 8 Pro";
//const char *password = "bhvgdgrxsfrh583";
const char *ssid     = "LAPTOP-7DU9NPMG 3407";
const char *password = "79437!dA";
// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

WebServer server(80);



/// Utilitats

int de(int n){
int b,c;
b=n/10;
c=b%10;
return c;}

int ud(int n){
int b;
b=n%10;
return b;}




//PERIFERICOS



// microSD
#define SD_CS          5
#define SPI_MOSI      23 
#define SPI_MISO      19
#define SPI_SCK       18
 
// I2S Connections

#define I2S_DOUT      22
#define I2S_BCLK      26
#define I2S_LRC       25
//ledpins
#define PIN_1       27//ud ho
#define PIN_2        13//dec ho
#define PIN_3       33 //ud min
#define PIN_4       32//dec min
#define NUMPIXELS 7
Adafruit_NeoPixel h1(NUMPIXELS, PIN_1, NEO_GRB + NEO_KHZ400); 
Adafruit_NeoPixel h2(NUMPIXELS, PIN_2, NEO_GRB + NEO_KHZ400);
Adafruit_NeoPixel m1(NUMPIXELS, PIN_3, NEO_GRB + NEO_KHZ400);
Adafruit_NeoPixel m2(NUMPIXELS, PIN_4, NEO_GRB + NEO_KHZ400);
//altavoz
Audio audio;


//boton  

struct Button {
  const uint8_t PIN;
  bool pressed;

};
Button boton = {14,false};

/*void IRAM_ATTR isr() {
  boton.pressed = true;
  audio.stopSong();
  Serial.println("Se ha detenido la alarma");
  
}*/



//WEBSERVER UTILITIES
void handleRoot() {
  String html = "<html><body>";
  html += "<h1>Control de variables</h1>";
  html += "<form action='/color' method='POST'>";
  html += "Color: <input type='color' name='color' value='#" + String(R, HEX) + String(G, HEX) + String(B, HEX) + "'><br>";
  html += "<input type='submit' value='Cambiar color'>";
  html += "</form>";
  html += "<br>";
  html += "<form action='/values' method='POST'>";
  html += "Hora: <input type='number' name='value1' value='" + String(value1) + "'><br>";
  html += "Minuto: <input type='number' name='value2' value='" + String(value2) + "'><br>";
  html += "<input type='submit' value='Establecer Alarma'>";
  html += "<input type='button' value='Borrar Alarma' onclick='establecerValores()' onclick='actualizarFrase()'>";
  html += "</form>";
  html += "<br>";
  html += "<br>";
  html += "<p id='mensaje' style='display:";
  html += (value1 >= 24 || value2 >= 60) ? "none" : "block"; // Oculta la frase si se cumplen las condiciones
  html += "'>La alarma esta establecida a las " + String(value1) + ":" + String(value2) + "</p>";
  html += "<script>";
  html += "function establecerValores() {";
  html += "document.getElementsByName('value1')[0].value = 25;";
  html += "document.getElementsByName('value2')[0].value = 61;";
  html += "actualizarFrase();";
  html += "}";
  html += "function actualizarFrase() {";
  html += "var mensaje = document.getElementById('mensaje');";
  html += "mensaje.style.display = (document.getElementsByName('value1')[0].value >= 24 || document.getElementsByName('value2')[0].value >= 60) ? 'none' : 'block';";
  html += "}";
  html += "</script>";
  html += "</body></html>";
  server.send(200, "text/html", html);
}

void handleColor() {
  if (server.hasArg("color")) {
    String colorValue = server.arg("color");
    colorValue.replace("#", "");

    if (colorValue.length() == 6) {
      R = strtol(colorValue.substring(0, 2).c_str(), NULL, 16);
      G = strtol(colorValue.substring(2, 4).c_str(), NULL, 16);
      B = strtol(colorValue.substring(4, 6).c_str(), NULL, 16);
    }
  }
  server.sendHeader("Location", "/");
  server.send(302);
}

void handleValues() {
  if (server.hasArg("value1") && server.hasArg("value2")) {
    value1 = server.arg("value1").toInt();
    value2 = server.arg("value2").toInt();
  }
  server.sendHeader("Location", "/");
  server.send(302);
}


//SETUP
void setup() {
  Serial.begin(115200); //INICIAR SERIAL

  // Set microSD Card CS as OUTPUT and set HIGH
    pinMode(SD_CS, OUTPUT);      
    digitalWrite(SD_CS, HIGH); 
    
 // Initialize SPI bus for microSD Card
    SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);

// INICIAR TIRAS
  h1.begin();
  m1.begin();
  h2.begin();
  m2.begin();

  
  // Start microSD Card
    if(!SD.begin(SD_CS))
    {
      Serial.println("Error accessing microSD card!");
      while(true); 
    }

  
  
  // Connect to Wi-Fi
  Serial.println("");
  Serial.println("");
  Serial.println("");
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(WiFi.localIP());

//audio
 audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);// pins Audio
  audio.setVolume(21);//volumen
   

//boton 
pinMode(boton.PIN, INPUT_PULLUP);
  // attachInterrupt(boton.PIN, isr, FALLING);

// Initialize a NTPClient to get time
  timeClient.begin();
  timeClient.setTimeOffset(7200);//ZONA HORARIA EN SEG
  timeClient.update();//OBTENCION HORA
  //server
 server.on("/", handleRoot);//ENCENDER SERVER
  server.on("/color", handleColor);//ENCENDER SERVER
  server.on("/values", handleValues);//ENCENDER SERVER

   
   server.begin();

 
 
 
}




void loop() {
 //server 
server.handleClient();
audio.loop();
//obtencion de horas
  timeClient.update();
int currentHour = timeClient.getHours();
int currentMinute = timeClient.getMinutes();
int decmin=de(currentMinute);
    int udmin=ud(currentMinute);
    int decho=de(currentHour);
    int udho=ud(currentHour);




// Print comprobar numeros
/*Serial.print(decho);
Serial.print("  ");
Serial.print(udho);
Serial.print("  ");
Serial.print(decmin);
Serial.print("  ");
Serial.print(udmin);
Serial.println("");      
Serial.println(udantic1);
 */




//control LEDS

 //led unitat MIN
int min1;                   
if(udantic1 != udmin or oR != R or oG != G or oB != B){ 
  
  
  min1=udmin;
  if(min1==0){
  m1.clear();
  m1.setPixelColor(0, m1.Color(R,G,B));
  m1.setPixelColor(1, m1.Color(R,G,B));
  m1.setPixelColor(2, m1.Color(R,G,B));
  m1.setPixelColor(3, m1.Color(R,G,B));
  m1.setPixelColor(4, m1.Color(R,G,B));
  m1.setPixelColor(5, m1.Color(R,G,B));
  m1.show();}
  else if (min1==1){
    m1.clear();
  m1.setPixelColor(0, m1.Color(R,G,B));
  m1.setPixelColor(5, m1.Color(R,G,B));  
  m1.show();
  }
   else if (min1==2){
    m1.clear();
  m1.setPixelColor(0, m1.Color(R,G,B));
  m1.setPixelColor(1, m1.Color(R,G,B));
  m1.setPixelColor(6, m1.Color(R,G,B));
  m1.setPixelColor(3, m1.Color(R,G,B)); 
  m1.setPixelColor(4, m1.Color(R,G,B));
  m1.show();
  }
   else if (min1==3){
    m1.clear();
  m1.setPixelColor(0, m1.Color(R,G,B));
  m1.setPixelColor(1, m1.Color(R,G,B)); 
  m1.setPixelColor(6, m1.Color(R,G,B));
  m1.setPixelColor(5, m1.Color(R,G,B)); 
  m1.setPixelColor(4, m1.Color(R,G,B));
  m1.show();
  }
   else if (min1==4){
  m1.clear();
  m1.setPixelColor(0, m1.Color(R,G,B));
  m1.setPixelColor(2, m1.Color(R,G,B));  
  m1.setPixelColor(6, m1.Color(R,G,B));
  m1.setPixelColor(5, m1.Color(R,G,B)); 
  m1.show();
  }
  else if (min1==5){
    m1.clear();
  m1.setPixelColor(1, m1.Color(R,G,B));
  m1.setPixelColor(2, m1.Color(R,G,B));  
  m1.setPixelColor(6, m1.Color(R,G,B));
  m1.setPixelColor(5, m1.Color(R,G,B)); 
  m1.setPixelColor(4, m1.Color(R,G,B));
  m1.show();
  }
  else if (min1==6){
    m1.clear();
  m1.setPixelColor(1, m1.Color(R,G,B));
  m1.setPixelColor(2, m1.Color(R,G,B));  
  m1.setPixelColor(6, m1.Color(R,G,B));
  m1.setPixelColor(3, m1.Color(R,G,B)); 
  m1.setPixelColor(4, m1.Color(R,G,B));
  m1.setPixelColor(5, m1.Color(R,G,B));
  m1.show();
  }
   else if (min1==7){
    m1.clear();
  m1.setPixelColor(0, m1.Color(R,G,B));
  m1.setPixelColor(1, m1.Color(R,G,B)); 
  m1.setPixelColor(5, m1.Color(R,G,B));
  m1.show();
  }
   else if (min1==8){
    m1.clear();
  m1.setPixelColor(0, m1.Color(R,G,B));
  m1.setPixelColor(1, m1.Color(R,G,B));
  m1.setPixelColor(2, m1.Color(R,G,B));
  m1.setPixelColor(3, m1.Color(R,G,B)); 
  m1.setPixelColor(4, m1.Color(R,G,B));
  m1.setPixelColor(5, m1.Color(R,G,B));
  m1.setPixelColor(6, m1.Color(R,G,B));
  m1.show();
  }
    else if (min1==9){
    m1.clear();
  m1.setPixelColor(0, m1.Color(R,G,B));  
  m1.setPixelColor(1, m1.Color(R,G,B));
  m1.setPixelColor(2, m1.Color(R,G,B));
  m1.setPixelColor(6, m1.Color(R,G,B));  
  m1.setPixelColor(5, m1.Color(R,G,B));  
  m1.setPixelColor(4, m1.Color(R,G,B)); 
  m1.show();
    }
    else{m1.clear();}
 
 udantic1=min1;
 }

//led Decenes MIN
int min2;
if(udantic2 != decmin or oR != R or oG != G or oB != B){ 
  min2=decmin;
  if(min2==0){
  m2.clear();
  m2.setPixelColor(0, m2.Color(R,G,B));
  m2.setPixelColor(1, m2.Color(R,G,B));
  m2.setPixelColor(2, m2.Color(R,G,B));
  m2.setPixelColor(3, m2.Color(R,G,B));
  m2.setPixelColor(4, m2.Color(R,G,B));
  m2.setPixelColor(5, m2.Color(R,G,B));
  m2.show();}
  else if (min2==1){
    m2.clear();
  m2.setPixelColor(0, m2.Color(R,G,B));
  m2.setPixelColor(5, m2.Color(R,G,B));  
  m2.show();
  }
   else if (min2==2){
    m2.clear();
  m2.setPixelColor(0, m2.Color(R,G,B));
  m2.setPixelColor(1, m2.Color(R,G,B));
  m2.setPixelColor(6, m2.Color(R,G,B));
  m2.setPixelColor(4, m2.Color(R,G,B)); 
  m2.setPixelColor(3, m2.Color(R,G,B));
  m2.show();
  }
   else if (min2==3){
    m1.clear();
  m2.setPixelColor(0, m2.Color(R,G,B));
  m2.setPixelColor(1, m2.Color(R,G,B)); 
  m2.setPixelColor(6, m2.Color(R,G,B));
  m2.setPixelColor(5, m2.Color(R,G,B)); 
  m2.setPixelColor(4, m2.Color(R,G,B));
  m2.show();
  }
   else if (min2==4){
  m2.clear();
  m2.setPixelColor(0, m2.Color(R,G,B));
  m2.setPixelColor(2, m2.Color(R,G,B));  
  m2.setPixelColor(6, m2.Color(R,G,B));
  m2.setPixelColor(5, m2.Color(R,G,B)); 
  m2.show();
  }
  else if (min2==5){
    m2.clear();
  m2.setPixelColor(1, m2.Color(R,G,B));
  m2.setPixelColor(2, m2.Color(R,G,B));  
  m2.setPixelColor(6, m2.Color(R,G,B));
  m2.setPixelColor(5, m2.Color(R,G,B)); 
  m2.setPixelColor(4, m2.Color(R,G,B));
  m2.show();
  }
  else if (min2==6){
    m2.clear();
  m2.setPixelColor(1, m2.Color(R,G,B));
  m2.setPixelColor(2, m2.Color(R,G,B));  
  m2.setPixelColor(6, m2.Color(R,G,B));
  m2.setPixelColor(4, m2.Color(R,G,B)); 
  m2.setPixelColor(5, m2.Color(R,G,B));
  m2.setPixelColor(3, m2.Color(R,G,B));
  m2.show();
  }
   else if (min2==7){
    m2.clear();
  m2.setPixelColor(0, m2.Color(R,G,B));
  m2.setPixelColor(1, m2.Color(R,G,B)); 
  m2.setPixelColor(5, m2.Color(R,G,B)); 
  m2.show();
  }
   else if (min2==8){
    m2.clear();
  m2.setPixelColor(0, m2.Color(R,G,B));
  m2.setPixelColor(1, m2.Color(R,G,B));
  m2.setPixelColor(2, m2.Color(R,G,B));
  m2.setPixelColor(3, m2.Color(R,G,B)); 
  m2.setPixelColor(4, m2.Color(R,G,B));
  m2.setPixelColor(5, m2.Color(R,G,B));
  m2.setPixelColor(6, m2.Color(R,G,B));
  m2.show();
  }
    else if (min2==9){
    m2.clear();
  m2.setPixelColor(0, m2.Color(R,G,B));  
  m2.setPixelColor(1, m2.Color(R,G,B));
  m2.setPixelColor(2, m2.Color(R,G,B));
  m2.setPixelColor(5, m2.Color(R,G,B));   
  m2.setPixelColor(6, m2.Color(R,G,B));
  m2.setPixelColor(4, m2.Color(R,G,B));
  m2.show();
    }
    else{m2.clear();}

 udantic2=min2;
 }

//led Horas UNI
 int ho1;
if(hantic1 != udho or oR != R or oG != G or oB != B){   
  ho1=udho;
if(ho1==0){
  h1.clear();
  h1.setPixelColor(0, h1.Color(R,G,B));
  h1.setPixelColor(1, h1.Color(R,G,B));
  h1.setPixelColor(2, h1.Color(R,G,B));
  h1.setPixelColor(3, h1.Color(R,G,B));
  h1.setPixelColor(4, h1.Color(R,G,B));
  h1.setPixelColor(5, h1.Color(R,G,B));
  h1.show();}
  else if (ho1==1){
    h1.clear();
  h1.setPixelColor(0, h1.Color(R,G,B));
  h1.setPixelColor(5, h1.Color(R,G,B));  
  h1.show();
  }
   else if (ho1==2){
    h1.clear();
  h1.setPixelColor(0, h1.Color(R,G,B));
  h1.setPixelColor(1, h1.Color(R,G,B));
  h1.setPixelColor(3, h1.Color(R,G,B));
  h1.setPixelColor(4, h1.Color(R,G,B)); 
  h1.setPixelColor(6, h1.Color(R,G,B));
  h1.show();
  }
   else if (ho1==3){
    h1.clear();
  h1.setPixelColor(0, h1.Color(R,G,B));
  h1.setPixelColor(1, h1.Color(R,G,B)); 
  h1.setPixelColor(6, h1.Color(R,G,B));
  h1.setPixelColor(5, h1.Color(R,G,B)); 
  h1.setPixelColor(4, h1.Color(R,G,B));
  h1.show();
  }
   else if (ho1==4){
  h1.clear();
  h1.setPixelColor(0, h1.Color(R,G,B));
  h1.setPixelColor(2, h1.Color(R,G,B));  
  h1.setPixelColor(6, h1.Color(R,G,B));
  h1.setPixelColor(5, h1.Color(R,G,B)); 
  h1.show();
  }
  else if (ho1==5){
    h1.clear();
  h1.setPixelColor(1, h1.Color(R,G,B));
  h1.setPixelColor(2, h1.Color(R,G,B));  
  h1.setPixelColor(6, h1.Color(R,G,B));
  h1.setPixelColor(5, h1.Color(R,G,B)); 
  h1.setPixelColor(4, h1.Color(R,G,B));
  h1.show();
  }
  else if (ho1==6){
    h1.clear();
  h1.setPixelColor(1, h1.Color(R,G,B));
  h1.setPixelColor(2, h1.Color(R,G,B));  
  h1.setPixelColor(6, h1.Color(R,G,B));
  h1.setPixelColor(3, h1.Color(R,G,B)); 
  h1.setPixelColor(4, h1.Color(R,G,B));
  h1.setPixelColor(5, h1.Color(R,G,B));
  h1.show();
  }
   else if (ho1==7){
    h1.clear();
  h1.setPixelColor(0, h1.Color(R,G,B));
  h1.setPixelColor(1, h1.Color(R,G,B)); 
  h1.setPixelColor(5, h1.Color(R,G,B));
  h1.show();
  }
   else if (ho1==8){
    h1.clear();
  h1.setPixelColor(0, h1.Color(R,G,B));
  h1.setPixelColor(1, h1.Color(R,G,B));
  h1.setPixelColor(2, h1.Color(R,G,B));
  h1.setPixelColor(3, h1.Color(R,G,B)); 
  h1.setPixelColor(4, h1.Color(R,G,B));
  h1.setPixelColor(5, h1.Color(R,G,B));
  h1.setPixelColor(6, h1.Color(R,G,B));
  h1.show();
  }
    else if (ho1==9){
    h1.clear();
  h1.setPixelColor(0, h1.Color(R,G,B));  
  h1.setPixelColor(1, h1.Color(R,G,B));
  h1.setPixelColor(2, h1.Color(R,G,B)); 
  h1.setPixelColor(5, h1.Color(R,G,B));   
  h1.setPixelColor(6, h1.Color(R,G,B));
  h1.setPixelColor(4, h1.Color(R,G,B));
  h1.show();
    }
    else{h1.clear();}

  
 hantic1=ho1;
 }

 //led Horas dec
 int ho2;
if(hantic2 != decho or oR != R or oG != G or oB != B){   
  ho2=decho;
 if(ho2==0){
  h2.clear();
  h2.setPixelColor(0, h2.Color(R,G,B));
  h2.setPixelColor(5, h2.Color(R,G,B));
  h2.setPixelColor(1, h2.Color(R,G,B));
  h2.setPixelColor(4, h2.Color(R,G,B));
  h2.setPixelColor(2, h2.Color(R,G,B));
  h2.setPixelColor(3, h2.Color(R,G,B));
  h2.show();}
  else if (ho2==1){
    h2.clear();
  h2.setPixelColor(0, h2.Color(R,G,B));
  h2.setPixelColor(5, h2.Color(R,G,B));  
  h2.show();
  }
   else if (ho2==2){
    h2.clear();
  h2.setPixelColor(0, h2.Color(R,G,B));
  h2.setPixelColor(1, h2.Color(R,G,B));
  h2.setPixelColor(6, h2.Color(R,G,B));
  h2.setPixelColor(3, h2.Color(R,G,B)); 
  h2.setPixelColor(4, h2.Color(R,G,B));
  h2.show();
  }
   else if (ho2==3){
    h2.clear();
  h2.setPixelColor(0, h2.Color(R,G,B));
  h2.setPixelColor(1, h2.Color(R,G,B)); 
  h2.setPixelColor(6, h2.Color(R,G,B));
  h2.setPixelColor(5, h2.Color(R,G,B)); 
  h2.setPixelColor(4, h2.Color(R,G,B));
  h2.show();
  }
   else if (ho2==4){
  h2.clear();
  h2.setPixelColor(0, h2.Color(R,G,B));
  h2.setPixelColor(2, h2.Color(R,G,B));  
  h2.setPixelColor(6, h2.Color(R,G,B));
  h2.setPixelColor(5, h2.Color(R,G,B)); 
  h2.show();
  }
  else if (ho2==5){
    h2.clear();
  h2.setPixelColor(1, h2.Color(R,G,B));
  h2.setPixelColor(2, h2.Color(R,G,B));  
  h2.setPixelColor(6, h2.Color(R,G,B));
  h2.setPixelColor(5, h2.Color(R,G,B)); 
  h2.setPixelColor(4, h2.Color(R,G,B));
  h2.show();
  }
  else if (ho2==6){
    h2.clear();
  h2.setPixelColor(1, h2.Color(R,G,B));
  h2.setPixelColor(2, h2.Color(R,G,B));  
  h2.setPixelColor(6, h2.Color(R,G,B));
  h2.setPixelColor(3, h2.Color(R,G,B)); 
  h2.setPixelColor(4, h2.Color(R,G,B));
  h2.setPixelColor(5, h2.Color(R,G,B));
  h2.show();
  }
   else if (ho2==7){
    h2.clear();
  h2.setPixelColor(0, h2.Color(R,G,B));
  h2.setPixelColor(1, h2.Color(R,G,B)); 
  h2.setPixelColor(5, h2.Color(R,G,B));
  h2.show();
  }
   else if (ho2==8){
    h1.clear();
  h2.setPixelColor(0, h2.Color(R,G,B));
  h2.setPixelColor(1, h2.Color(R,G,B));
  h2.setPixelColor(2, h2.Color(R,G,B));
  h2.setPixelColor(3, h2.Color(R,G,B)); 
  h2.setPixelColor(4, h2.Color(R,G,B));
  h2.setPixelColor(5, h2.Color(R,G,B));
  h2.setPixelColor(6, h2.Color(R,G,B));
  h2.show();
  }
    else if (ho2==9){
    h2.clear();
  h2.setPixelColor(0, h2.Color(R,G,B));  
  h2.setPixelColor(1, h2.Color(R,G,B));
  h2.setPixelColor(2, h2.Color(R,G,B));  
  h2.setPixelColor(5, h2.Color(R,G,B));   
  h2.setPixelColor(6, h2.Color(R,G,B));
  h2.setPixelColor(4, h2.Color(R,G,B));
  h2.show();
  }

    else{h2.clear();}
 hantic2=ho2;
 }

oR=R;
 oG=G;
 oB=B;



 int bb = value2-1;

 if(currentHour==value1 && currentMinute==bb){

audio.connecttoFS(SD,"/alarm.mp3");//abrir archivo mp3
}


int botonEstado = digitalRead(boton.PIN);
if(botonEstado==LOW){
  audio.stopSong();
  //boton.pressed=false;
  }

```


# Codi explicat

**Llibreries:**

Primer li hem afegit les llibreries necessaries per a cada component:

```
#include <Arduino.h>
```


Les llibreries necessaries per utilitzar el lector eren la SPI, ja que es el canal al que esta connectat i hem agregat la llibreria 'MFRC522'. També hem definit els pins del RST i el SS.

```

// Lector de targetas
#include <SPI.h>
#include <MFRC522.h>
#define RST_PIN 17 
#define SS_PIN 4

```

Pel WiFi hem utilitzat la llibreria de wifi i hem iniciat les constants del wifi i la contrasenya:

```
// Wifi
#include <WiFi.h>
const char* ssid = "iPhone de Sonia";
const char* password = "sonia123";
```
Per la pagina web hem utilitzat la llibreria WebServer i hem inicialitzat el server:

```
// Pagina web
#include <WebServer.h>
WebServer server(80);

```
Pel zumbador hem utilitzat les llibreries 'FS' i 'SD', i hem definit el pin del zumbador al 2:

```
#include <FS.h>
#include <SD.h>
#define BUZZER 2
```
Finalment, pel display hem agregat les llibreries 'Adafruit GFX Library' i 'Adafruit SSD1306' i hem utilitzat la llibreria 'Wire'. També hem definit les dimensions del display:

```
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
 
```

**Variables:**

A continuació hem declarat les variables del projecte, entre les quals es troben els leds i el zumbador entre d'altres:

```
int ledblanc = 12;
int ledverd = 13;
int ledvermell = 14;
bool correcte = true;
int zumbador = 2;
MFRC522 mfrc522(SS_PIN, RST_PIN);
```

També hem declarat les funcions que programarem més endavant:

```
void PICAR (); // Picar al timbre
void PASA (); // Esta sera la funcion que "habrira la puerta"
void COMPARAR();
void DENEGADO ();
void handle_root (void); // Pagina web
```

**SET UP:**

Dins del set up primer inicialitzarem la comunicació serial:

```
void setup() {
  Serial.begin(115200); //Iniciamos la comunicación serial
  
```
A continuació, per a utilitzar el lector de targetes inicialitzarem el bus SPI i el MFRC522:

```
  SPI.begin(); //Iniciamos el Bus SPI
  mfrc522.PCD_Init(); // Iniciamos el MFRC522
  Serial.println("Lectura del UID");
  
```
Inicialitzem tots els pins (leds, pulsadors i zumbador)

```
  // Leds
  pinMode(ledblanc, OUTPUT);
  pinMode(ledverd, OUTPUT);
  pinMode(ledvermell, OUTPUT);
  pinMode (21, INPUT_PULLUP); // boton para abrir la puerta
  pinMode (15, INPUT_PULLUP); // timbre
  pinMode (BUZZER, OUTPUT); // zumbador
```

Inicialitzarem també el wifi, que anirà imprimint punts fins que es connecti:

```
WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
  delay(1000);
  Serial.print(".");
 }
```

Mostrem la IP de l'ordinador per poder cercar la pàgina web i posem els comandaments necessaris perquè ens funcioni

```
Serial.print("Got IP: ");
Serial.println(WiFi.localIP()); //Show ESP32 IP on serial
server.begin();
server.on("/", handle_root);
```
Incialtzem el Display i especifiquem les seves propietats:

```
if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println("SSD1306 allocation failed");
    for(;;);
  }

  delay(2000);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
```

tanquem el set up:

```
}
```


Fora del set up declararem la variable Usuari1 que actuarà com a targeta correcta d'exemple:

```
byte Usuario1[4]= {0x50, 0x60, 0x13, 0x4E} ; //código del usuario 1

```

**Loop:**

Primer crearem dues variables per a llegir el timbre i el botó del conserge

```
void loop() {
  int timbre = digitalRead (15);
  int boto = digitalRead (21);
```
Ara farem un if per cada un dels dos pulsadors per a comprovar si algun d'aquests està sent presionat, el qual cridaria a les seves respectives funcións, les quals programarem més endavant;

```
  // Comprobamos si los botones estan presionados y llamamos a sus respectivas funciones
  if (timbre==LOW){
     PICAR();
  }
  if (boto==LOW){
     PASA();
  }
```
Fem un altre if per a comprovar si hi han noves targetes en el lector, i en cas positiu s'encen el led blanc i es crida a la funció 'comparar' que, com indica el nom, es dedicara a comparar la targeta entrant amb les que tenim guardades:

```
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
 ```
 En cas de que el pin de la targeta sigui correcte es cridarà la funció 'pasa', en canvia, si es incorrecte es crida la funcio 'denegado':
 
 ```
   if (correcte){
       PASA();
   }
   else {
       DENEGADO();
   }
 ```
Tanquem el loop:

```
}
```

**Funcions:**

La primera funció es 'picar', la qual simula un timbre, es a dir, en cas de presionar el pulsador del timbre de la porta s'activa el zumbador amb la durada i frequencia que nosaltres indiquem:

```
// Esta funcion simula un timbre
void PICAR (){
  digitalWrite(BUZZER,HIGH);
  delay(1000);
  digitalWrite(BUZZER,LOW);
}
```

La següent funcio és 'pasa', que és la que s'activa quan el lector de targetes detecta una targeta correcta. Aquesta funció activa el led verd, escriu per pantalla que la targeta ha sigut acceptada i pel display "Bienvenido!", i escriu a la pagina web les dades de la targeta entrant:

```
// Esta función encendera el led verde y la pagina web
void PASA (){
  // encendemos led verde
  digitalWrite(ledverd, HIGH);
  delay(500);               // el deixem ences un moment
  digitalWrite(ledverd, LOW);
  Serial.println("Targeta ACEPTADA");
  // Escribim pel display
  display.println("Bienvenido!");
  display.display();
   delay(4000);
  display.clearDisplay(); 
  server.handleClient(); // pag web
}
```

A continuació tenim la funció 'comparar', que es dedica a comparar les targetes entrants en el lector amb les que tenim guardades i emetre un bool que diu si la targeta es correcte o si no ho es:

```
void COMPARAR(){
  correcte = true;
  for (byte i = 0; i < mfrc522.uid.size && correcte==true; i++) {
  // comparamos la targeta leida con la de ejemplo
    if (mfrc522.uid.uidByte[i] != Usuario1[i]){
        correcte = false;
    }
  }
 }
```

Finalment tenim la funció 'denegado' que és la que s'activa quan el lector de targetes llegeix una targeta incorrecta, en aquest cas s'encendrà un led vermell, es mostrarà per pantalla que la targeta ha sigut denegada i s'escriura pel Display "Targeta DENEGADA":

```
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
}
```

Al final de tot tenim el codi html i la funció handle_root els quals són els responsables de que la pagina web funcioni:

```
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
```

Enllaç del video de youtube de la presentació:

video: https://www.youtube.com/watch?v=GqsDBOWtVCQ






