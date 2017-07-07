// BIBLIOTEKER
#include <Servo.h> //Biblioteket som gjør kommunikasjon med opplerometeret mulig
#include <Adafruit_NeoPixel.h> //Biblioteket som gjør kommunikasjon med highscore mulig
#include <Wire.h> //Biblioteket gjør I2C-kommunikasjon mulig

//      P I N S
const int LED1 = 2;  //Highscore digital pin
const int SERVOPIN = 9;  //opplerometer digital pin

//      K O N S T A N T E R
const int PUMPE_adress = 8;        // I2C adressen til pumpe arduino
const int SENSOR_adress = 7;       // I2C adressen til sensor arduino
const int MAX_GODBITER = 6;        // godbiter ved start
const int OPPLERING_GRENSE = 90;   // if (opplering > OPPLERING_GRENSE && bjelle) {sikle();}
const int LED_CELLER = 30;         // antallet LEDs på stripa. Hvis vi kutter av noe blir dette mindre.
const int MAX_SERVO = 170;         // servoen kan flytte seg mellom 0 og 170
const int MIN_SERVO = 0;           // servoen kan flytte seg mellom 0 og 170

//      F U N K S J O N E R
void sikle(byte sikkel);           // sender informasjon til pumpearduino
void startSpill();                 // starter nytt spill
void ferdigSpill();                // viser highscore og venter på nytt spill
void opplereJack(int grader);      // flytter opplerometeret
void avlereJack(int grader);       // flytter opplerometeret

//      S P I L L V A R I A B L E R
int ventetid = 3000;                          // venetetid mellom bjelle og mat
int opplering = 170;                          // hvor mye hunden er opplært, fra 170 til 0
int score = 2;                                // score går fra 0 til 27 (antall LEDs)
int godbiter = MAX_GODBITER;                  // godbiter ved start
char sensor[3];                               // verdiene fra sensor arduino lagres her
char bjelle = 0, mat = 0, skuff = 0;          // verdiene til sensor
int i = 0;                                    // telle variabel

//      K O N T R O L L
byte tom_byte = 0;                            // tom_byte sendes til pumpe arduino for å tomme pumpen
byte sikleVariabel = 1;                       // sikleVariabel bestemmer intensiteten Jack sikler
bool apenSkuff = false;                       // apenSkuff er true om skuffen er åpen
bool matHentet = false;                       // holder styr på om mat er hentet
bool nyttSpill = true;                        // hvis TRUE, kjøres nyttSpill()
bool bjelle_vent = true;                      // hvis (t2-t1 < VENTETID) && mat ikke har blitt gitt ennå 
uint32_t tid1, tid2;                          // variabler for å måle tid mellom bjelle og mat
uint32_t spillTid, startTid;                  // sjekker hvor lang tid siden sist hendelse
bool ferdigSpillKalt = false;                 // hindrer at ferdigSpill blir kalt flere ganger


//LED-strips, begge viser score
Adafruit_NeoPixel scoreLED = Adafruit_NeoPixel(LED_CELLER, LED1, NEO_GRB + NEO_KHZ800);

//oppretter servo med navn opplerometer
Servo opplerometer; 

void setup() {
  Serial.begin(9600);  
  Wire.begin();                      // oppretter I2C-kommunikasjon
  opplerometer.attach(SERVOPIN);     // aktiverer servoen
  scoreLED.begin();                  // aktiverer LED-stripen for highscore
}

void loop(){

  //Fjerner forrige loops sensor variabler
  for (int j=0;j<3;j++){
    sensor[j]='0'; 
  }
 
  i = 0;
  delay(100);
  Wire.requestFrom(SENSOR_adress, 3);    // spør om 3 bokstaver fra sensor arduino
  while (Wire.available()) { 
    char b = Wire.read(); 
    sensor[i] = b;
    i++;
  }
  
  bjelle = sensor[0];
  mat = sensor[1];
  skuff = sensor[2];

// Sjekker om skuff er åpen
  if(skuff=='s'){ 
     apenSkuff = true;
     nyttSpill = true;
     matHentet = true;
  }else{
     apenSkuff = false;
  }
  
   if(nyttSpill && !apenSkuff && matHentet){ 
     startSpill();
   }
   else if(nyttSpill && !ferdigSpillKalt){    
     ferdigSpill();
   }
  
  if (bjelle=='b'||!bjelle_vent){             // venter på mat etter at bjellen er ringt
    if (bjelle_vent){
      bjelle_vent = false;                    // setter ventevariablen
      tid1 = millis();                        // Starter å telle fra bjellen ble ringt
      
      if(opplering<OPPLERING_GRENSE){
        if(godbiter == 0){
          ventetid = 1000;                    // trekker fra tid hvis det er tomt for mat 
        }
        else{
        ventetid = 4000;                      // legger til tid hvis det er mat igjen
        }
        if(opplering<30){ 
         sikle(6);
         score += 3;
        }
        else if(opplering<60){
         sikle(6);
         score += 2;
        }
        else{
         sikle(2);
         score += 1;
        }
      }
  }
    tid2 = millis();
      if(tid2-tid1>ventetid || mat=='m'){ //sjekker om det har gått 3 sek eller det er gitt mat.||
      bjelle_vent = true; // slår av ventevariablen
      bjelle = 'b';
      ventetid = 3000;
    }
  }
 

  if (bjelle_vent && !nyttSpill){ // hopper over resten hvis den venter eller spill ferdig
    if(mat=='m' && bjelle=='b' && godbiter!=0){ 
        sikle(5);
        godbiter--;
        opplereJack(40); 
        score+=3;
        startTid = millis();
    }
    else if (bjelle=='b'&& (opplering < OPPLERING_GRENSE)){
           avlereJack(30);
           startTid = millis();
    }
    else if(mat=='m' && godbiter!=0) {            // hvis det kommer mat
         sikle(5);                                // det skal sikles uansett hvis Jack får mat
         godbiter--;                              // det skal bli mindre mat
         avlereJack(30);
         score+=0;
         startTid = millis();
    }
  }

// hvis det er tomt for mat og hunden ikke er opplært, starter nytt spill
  if ((godbiter==0 &&!nyttSpill) && opplering >= OPPLERING_GRENSE){  
    nyttSpill = true;
  }
  spillTid = millis();
     
  if(spillTid-startTid>60000){                    // spillet slutter etter 60 sekunder
    nyttSpill = true;
  }
}

void startSpill(){
  startTid = millis();
  
  // slår av LED-stripen
    for(int i=0;i<30;i++){
     scoreLED.setPixelColor(i, scoreLED.Color(0,0,0));
     scoreLED.show();
     delay(20);
  }
  
  // resetter spillvariabler
    matHentet = false;
    nyttSpill = false;
    godbiter = MAX_GODBITER;
    score = 2;
    ferdigSpillKalt = false;
 
  sikle(tom_byte); // tømmer oppsamleren
  delay(500);
}

void sikle(byte sikleVariabel){
    Wire.beginTransmission(PUMPE_adress);    //Starter kommunikasjon med pumpearduino
    Wire.write(sikleVariabel);               //Sender informasjon
    Wire.endTransmission();
}

void ferdigSpill(){
 opplerometer.attach(SERVOPIN);              // aktiverer opplerometer
 opplering = 170;
 opplerometer.write(opplering);
 ferdigSpillKalt = true;                     // hindrer at ferdigSpill blir kalt flere ganger
 score = score*27/37;
  
  // Viser highscore
  for(int i=0;i<score;i++){                  
     scoreLED.setPixelColor(i, scoreLED.Color(0,0,200));
     scoreLED.show();
     delay(50);
  }
  delay(800); // så servoen har tid til å bevegeseg før den blir detached
 opplerometer.detach();                       // deaktiverer opplerometer
}

void avlereJack(int grader){ 
  opplerometer.attach(SERVOPIN);              // aktiverer opplerometer
  if(grader>170-opplering){                   // forhindet at opplerometeret går for langt
  grader=170-opplering;
  }
  for(int i = 0; i<(grader);i++){             // flytter opplerometeret
    opplering += 1;
    opplerometer.write(opplering);
    delay(40);
  }
  opplerometer.detach();                      // deaktiverer opplerometer
}

void opplereJack(int grader){ 
  opplerometer.attach(SERVOPIN);              // aktiverer opplerometer
  if(grader>opplering-8){                     // forhindet at opplerometeret går for langt
  grader=opplering-8;
  }
  for(int i = 0; i<grader;i++){               // flytter opplerometeret
    opplering -= 1;
    opplerometer.write(opplering);
    delay(40);
  }
   opplerometer.detach();                     // deaktiverer opplerometer
}

