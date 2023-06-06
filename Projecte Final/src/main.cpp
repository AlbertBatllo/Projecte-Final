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


             

  //pruebas 
 /* if(currentHour < 10){
    Serial.print("0");
  Serial.print(currentHour);}
  else Serial.print(currentHour); 
  Serial.print(":");
  if(currentMinute < 10){
    Serial.print("0");
  Serial.print(currentMinute);}
  else Serial.print(currentMinute); 
  Serial.print(":");
   if(currentSecond < 10){
    Serial.print("0");
  Serial.print(currentSecond);}
  else Serial.print(currentSecond);    
  Serial.println("");*/
  

/*Serial.print(decho);
Serial.print("  ");
Serial.print(udho);
Serial.print("  ");
Serial.print(decmin);
Serial.print("  ");
Serial.print(udmin);
Serial.println("");*/


 
 /*Serial.print(currentMinute);
  Serial.print(" ");
  Serial.print(value2);
  Serial.println("");*/

  
}
  