// BIBLIOTEKER
#include <Wire.h>

// I2C-kommunikasjon
const int SENSOR_ADDRESS = 7; // I2C linje mellom Hoved- og sensor arduino. Ikke en pin
char sensor[3] = "";// sensor inneholder info om skuffe, mat, bjelle

// S E N S O R P I N S
const int BJELLE_PIN = A0;                         // måler spenning på bjellen
const int MAT_LYS = A1;                            // lyssensor som detekterer mat
const int MAT_magnet1 = A2;                        // magnetsensor for fremtidlig implementering
const int MAT_magnet2 = A3;                        // magnetsensor for fremtidlig implementering
const int SKUFF_PIN = 4;                           // måler spenningen på skuffen

// K O N S T A N T E R
const int BJELLE_GRENSE = 70;                      // spenningsgrensen for at bjelle er ringt 
const int LYSGRENSE = 5;                           // spenningsgrensen for at mat er gitt

// K O N T R O L L V A R I A B L E R
int bjelleVerdi;                                   // spenningen på bjellen
bool bjelle = false, mat = false, skuff = false;   // informasjon om sensorene
uint32_t tid1=0, tid2=0;                           // tidvariabler
int matLysVerdi = 0;                               // spenningen over lyssensor

void setup( ){
	Serial.begin(9600);     
  Wire.begin(SENSOR_ADDRESS);                      // oppretter I2C-kommunikasjon
	Wire.onRequest(requestEvent);                    // blir kjørt når kalt
}

void loop(){
		sjekkBjelle();   // sjekker bjelle
		sjekkSkuff();    // sjekker skuff
		sjekkMat();      // sjekker mat
}//loop

void requestEvent(int dummy){
  tid2 = millis();
  if(tid2-tid1<2000){                     // forhindrer for mange godbiter
    Serial.println("mat slettet");
    sensor[1]='0';
  }
  
  Wire.write(sensor);                     // Sender informasjonen

  if(sensor[1]=='m'){                     // forhindrer for mange godbiter
    tid1 = millis();
  }

  // sletter informasjon som ble sendt
  sensor[0] = '0'; 
  sensor[1] = '0';
  sensor[2] = '0';
}

bool sjekkBjelle(){
    bjelleVerdi = analogRead(BJELLE_PIN);       // måler spenning på bjelle pin
	  bjelle = (bjelleVerdi > BJELLE_GRENSE);     // sjekker om bjellen er ringt
    if(bjelle){
      sensor[0] = 'b';                          // setter bjelle variablen høy
      bjelle = false;                           // resetter bjelle
    }
}

bool sjekkMat(){
    matLysVerdi = analogRead(MAT_LYS);          // måler spenningen over lyssensor
     if(matLysVerdi<LYSGRENSE){                 // sjekker om det ble gitt mat
      sensor[1] = 'm';                          // setter mat variablen høy
     }
}

bool sjekkSkuff(){
   skuff = digitalRead(SKUFF_PIN);              // sjekker om skuffen ble åpnet
   if (!skuff) {                        
     sensor[2] = 's';                           // setter skuff variablen høy
   }
}

