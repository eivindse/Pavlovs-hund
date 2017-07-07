// Sikle slave - COM5 //
#include <Wire.h>

// Resistor table goes here
const char table[] = {15,13,11,9,7,5,3,1};

const int pin_empty = 9; 		// D9
const int pin_sikle = 7; 		// D7
const int r1pin = 2; 			// D3
const int r2pin = 3; 			// D4
const int r3pin = 4;			// D5
const int tomPin = 1;			// A1


// Defying this slave's I2C adress. 
const int this_adress = 8;

// Holder variable for recieved data
byte recieved = 100; 			// 100 is no operation default value
boolean busy = false, sikling = false;
unsigned long startedAt = 0;
const int emptyTime = 3000; 	// 3 seconds
const int sikleTime = 2000; 	// 2 seconds as max limit
unsigned long finishAt = 0;

void setup(){
	// Slave arduino need to be set up this way
	Serial.begin(115200);
	Serial.print("Initializing\n");
	Wire.begin(this_adress);
	Wire.onReceive(getNewByte);
	
	pinMode(pin_empty,OUTPUT);
	pinMode(pin_sikle,OUTPUT);
	pinMode(r1pin,OUTPUT);
	pinMode(r2pin,OUTPUT);
	pinMode(r3pin,OUTPUT);
	
	Serial.println("Started");
}

/*
Recieved table
100  = no operation
0    = emptying
1-99 = sikle speeds
*/

void loop(){
  busy = false;
	delay(50);
	
	if (recieved != 100){
	Serial.print("Recieved command. ");
		busy = true;
		if (recieved == 0){
		// Emptying
			Serial.print("Emptying...");
			if (sikling == true){
				digitalWrite(pin_sikle,LOW);
			}
			digitalWrite(pin_empty,HIGH);
			delay(700);
			while (analogRead(tomPin) < 1023/5*4.1){
				// No operation
			}
			digitalWrite(pin_empty,LOW);
		} else if (recieved < 100 && sikling == false){
			// Enabling droodling
			Serial.print("Sikling started at speed ");
			Serial.print(recieved);
			sikling = true;
			setR(recieved);
			finishAt = millis() + sikleTime;
			digitalWrite(pin_sikle,HIGH);
		} else if (millis() < finishAt && recieved < 100){
			// Changing droodling speed to new speed, resetting timer
			Serial.print("Sikling changed to speed ");
			Serial.print(recieved);
			setR(recieved);
			finishAt = millis() + sikleTime;
		} 
		recieved = 100;
		Serial.print(". Done\n");
	} else if (!(millis() < finishAt) && sikling == true){
	// Sikling timeout
		busy = true;
		Serial.print("Sikling timed out...");
		digitalWrite(pin_sikle,LOW);
		sikling = false;
		Serial.print(". Done\n");
	}
}

void getNewByte(int dummy){
	Serial.println("\nINTERRUPTED");
	if (busy == false){
		recieved = Wire.read(); 
	}
}

// Set variable resistor value function
// Only works for n=4 resistors like this
void setR(char goal){
	digitalWrite(r1pin, HIGH && (table[goal-1] & B00010));
	digitalWrite(r2pin, HIGH && (table[goal-1] & B00100));
	digitalWrite(r3pin, HIGH && (table[goal-1] & B01000));
}
