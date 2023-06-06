# Projecte-Final: Despertador LED

El nostre projecte consisteix en el funcionament d'un despertador amb llums LED, el qual té les següents funcions:

- Donar la hora digital amb tires de 7 LEDs
- Poder fixar una alarma a partir d'una pàgina web
- Poder canviar el color dels LEDs a partir de la mateixa pàgina web
- Fer que al sonar l'alarma s'activi un fitxer d'audio localitzat a una micro SD

## Diagrama de blocs


## Esquema de pins


# Codi

```

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






