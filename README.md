# Projecte-Final: Funcionament d'una porta amb sensor

El nostre projecte consisteix en el funcionament d'una porta, la qual té les següents funcions:

- Llegir targetes // Albert i Sonia
- Comparar-les amb la nostra i encendre els leds // Albert i Sonia
- Si la targeta és correcta genera una pàgina web on apareix qui entra i on viu // Sonia
- Poder picar un timbre i que s'encengui un zumbador// Sonia
- Que el "conserge" pugui obrir des de dins // Sonia

# Diagrama de blocs

![Captura de Pantalla 2022-06-06 a les 13 17 43](https://user-images.githubusercontent.com/100155905/172151261-b4eb2bba-bfff-48c8-b642-dee77c2bfdaf.png)

# Esquema de pins

![Esquema de pins](https://user-images.githubusercontent.com/100155905/172151389-0a046371-8d98-4d78-9a28-fb0a503f7ebe.jpeg)

# Codi

```
#include <Arduino.h>
// Lector de targetas
#include <SPI.h>
#include <MFRC522.h>
#define RST_PIN 17 
#define SS_PIN 4
 
// Wifi
#include <WiFi.h>
const char* ssid = "iPhone de Sonia";
const char* password = "sonia123";

// Pagina web
#include <WebServer.h>
WebServer server(80);
 
// Zumbador
#include <EasyBuzzer.h>
 
// Variables
int ledblanc = 12;
int ledverd = 13;
int ledvermell = 14;
bool correcte = true;
int zumbador = 2;
MFRC522 mfrc522(SS_PIN, RST_PIN);
 
// Funciones
void PICAR (); // Picar al timbre
void PASA (); // Esta sera la funcion que "habrira la puerta"
void COMPARAR();
void DENEGADO ();
void handle_root (void); // Pagina web

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
  EasyBuzzer.setPin(zumbador);
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
  EasyBuzzer.beep (1000, 900000); // frequencia en Hz, duracion del pitido en ms
  EasyBuzzer.stopBeep();
}
// Esta función encendera el led verde y la pagina web
void PASA (){
  // encendemos led verde
  digitalWrite(ledverd, HIGH);
  delay(500);               // el deixem ences un moment
  digitalWrite(ledverd, LOW);
  Serial.println("Targeta ACEPTADA");
  server.handleClient(); // pag web
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

```


# Codi explicat

**Llibreries:**

Primer li hem afegit les llibreries necessaries per a cada component:

```
#include <Arduino.h>
```


Les llibreries necessaries per utilitzar el lector eren la SPI, ja que es el canal al que esta connectat i la 'MFRC522'. També hem definit els pins del RST i el SS.

```

// Lector de targetas
#include <SPI.h>
#include <MFRC522.h>
#define RST_PIN 17 
#define SS_PIN 4

```

Pel WiFi hem agregat la llibreria de wifi i hem iniciat les constants del wifi i la contrasenya:

```
// Wifi
#include <WiFi.h>
const char* ssid = "iPhone de Sonia";
const char* password = "sonia123";
```
Per la pagina web hem afegit la llibreria WebServer i hem inicialitzat el server:

```
// Pagina web
#include <WebServer.h>
WebServer server(80);

```
Finalment, pel zumbador hem agregat la llibreria 'EasyBuzzer':

```
// Zumbador
#include <EasyBuzzer.h>
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
  pinMode (2, OUTPUT); // zumbador
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
  EasyBuzzer.beep (1000, 900000); // frequencia en Hz, duracion del pitido en ms
  EasyBuzzer.stopBeep();
}
```

La següent funcio és 'pasa', que és la que s'activa quan el lector de targetes detecta una targeta correcta. Aquesta funció activa el led verd, escriu pr pantalla que la targeta ha sigut acceptada, i escriu a la pagina web les dades de la targeta entrant:

```
// Esta función encendera el led verde y la pagina web
void PASA (){
  // encendemos led verde
  digitalWrite(ledverd, HIGH);
  delay(500);               // el deixem ences un moment
  digitalWrite(ledverd, LOW);
  Serial.println("Targeta ACEPTADA");
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

Finalment tenim la funció 'denegado' que és la que s'activa quan el lector de targetes llegeix una targeta incorrecta, en aquest cas s'encendrà un led vermell i es mostrarà per pantalla que la targeta ha sigut denegada:

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

```
video: https://www.youtube.com/watch?v=GqsDBOWtVCQ






