
#include "Arduino.h"
#include <avr/pgmspace.h>
#include "prog.h"

#define pinP1_0  5                              
#define pinP1_1  6                              
#define pinP1_2  7                              
#define pinP1_3  8                              
#define pinP1_4  9                              
#define pinP1_5  10                             
#define pinP1_6  11                             
#define pinP1_7  12                             

#define pinXTAL1 4
#define pinRST   3
#define pinPROG  2

#define pinP3_3   14 /*A0*/
#define pinP3_4   15 /*A1*/
#define pinP3_5   16 /*A2*/
#define pinP3_7   17 /*A3*/
#define pin12v    18 /*A4*/

const uint8_t pinsP1[]={pinP1_0,pinP1_1,pinP1_2,pinP1_3,pinP1_4,pinP1_5,pinP1_6,pinP1_7};

/*const uint8_t my_prog[] PROGMEM={
  };
*/

void setup() {

  pinMode(pinRST,OUTPUT);
  pinMode(pinXTAL1,OUTPUT);
  pinMode(pinPROG,OUTPUT);
  pinMode(pin12v,OUTPUT);
  
  pinMode(pinP3_3,OUTPUT);
  pinMode(pinP3_4,OUTPUT);
  pinMode(pinP3_5,OUTPUT);
  pinMode(pinP3_7,OUTPUT);
  
  digitalWrite(pinRST,LOW);
  digitalWrite(pinXTAL1,LOW);
  digitalWrite(pin12v,LOW);
  digitalWrite(pinPROG,HIGH);
  
  Serial.begin(9600);
}
//-------------------------------------------
// If rst was up or 12v then togle rst
// Set P1 to input or ouput, if ouput then set 12v, if iput up rst
// Set p3_3,p3_4,p3_5,p3_6.
bool set3457modeP1(uint8_t v33,uint8_t v34,uint8_t v35,uint8_t v37,uint8_t io){
  if (digitalRead(pin12v)==HIGH) {digitalWrite(pinRST,LOW); delay(10);}
  if (digitalRead(pinRST)==HIGH) {digitalWrite(pinRST,LOW); delay(1);}

  setModeP1(io);
  
  digitalWrite(pinP3_3, v33);
  digitalWrite(pinP3_4, v34);
  digitalWrite(pinP3_5, v35);
  digitalWrite(pinP3_7, v37);

  if (io==OUTPUT) return start12v();

  digitalWrite(pinRST,HIGH);
  delay(1);
  return true;
}
//-------------------------------------------
// Clear rst
inline void clsRST(){
  digitalWrite(pinRST,LOW);
}

//-------------------------------------------
// Process of execute commands
void pulsePROG(uint8_t t=1){
  digitalWrite(pinPROG, LOW);
  delay(t);
  digitalWrite(pinPROG, HIGH);
}

//-------------------------------------------
// Go to next address
void nextAddress(){
  digitalWrite(pinXTAL1,HIGH);
  delay(1);
  digitalWrite(pinXTAL1,LOW);
  delay(1);
}
//-------------------------------------------
// Set mode for port p1 (input or output)
void setModeP1(uint8_t io){
  for (uint8_t i=0; i<8; i++)
    pinMode(pinsP1[i],io);
}
//-------------------------------------------
// Read data from port p1
uint8_t readP1(){
  uint8_t d=0;
  for (uint8_t i=0;i<8;i++){
    d<<=1;
    if (digitalRead(pinsP1[7-i])==HIGH) d|=1;
  }
  return d;  
}
//-------------------------------------------
// Write data to port p1
void writeP1(uint8_t d){
  for (uint8_t i=0; i<8; i++){
    digitalWrite(pinsP1[i],(d&1)? HIGH:LOW);
    d>>=1;
  }
}
//-------------------------------------------
// Start connect 12v
bool start12v(){
  uint8_t symbol;

  Serial.println("For continue, press 'c'");
  
  while (Serial.available() == 0)
    ; //empty cycle
  symbol=Serial.read();
  if (symbol != 'c')  {
    Serial.println("The operation was canseled");
    return false;
  }
  digitalWrite(pin12v,HIGH);
  delay(1000); /*big delay for relay*/
  return true;
}  
//-------------------------------------------
// remove 12v
void stop12v(){
  Serial.println("\n12v was removed");
  digitalWrite(pin12v,LOW);
}

//-------------------------------------------
void eraseChip(){

  if (!set3457modeP1(HIGH,LOW,LOW,LOW,OUTPUT)) return;
  pulsePROG(10);
  Serial.println("Erased");

  stop12v();
}

//-------------------------------------------
void lockBit1(){

  if (!set3457modeP1(HIGH,HIGH,HIGH,HIGH,OUTPUT)) return;
  pulsePROG(1);
  Serial.println("Bit 1 set");

  stop12v();
}
//-------------------------------------------
void lockBit2(){

  if (!set3457modeP1(HIGH,HIGH,LOW,LOW,OUTPUT)) return;
  pulsePROG(1);
  Serial.println("Bit 2 set");

  stop12v();
}

//-------------------------------------------
void writeProg(){

  if (!set3457modeP1(LOW,HIGH,HIGH,HIGH,OUTPUT)) return;
      
  for (uint16_t i=0; i<sizeof(my_prog); i++)  {
    uint8_t d=pgm_read_byte_near(my_prog+i);
    Serial.print((int)d,HEX);
    Serial.print(" ,");
    writeP1(d);
    pulsePROG(1);
    delay(1);
    nextAddress();
  }
  Serial.println("");

  stop12v();
}

//----------------------------------------------
#define MAX_SIZE 2048
void readChip(){

  set3457modeP1(LOW,LOW,HIGH,HIGH,INPUT);
  
  for (uint16_t i=0; i<MAX_SIZE; i++)  {
    uint8_t d = readP1();

    Serial.print((int)d,HEX);
    Serial.print(" ,");

    nextAddress();
  }
  Serial.println("");

  clsRST();
}

//-----------------------------------------------
void readChipAndCompare(){
  bool ok=true;

  set3457modeP1(LOW,LOW,HIGH,HIGH,INPUT);

  for (uint16_t i=0; i<sizeof(my_prog); i++)  {
    uint8_t d1 = readP1();
    uint8_t d2 = pgm_read_byte_near(my_prog+i);
    if (d1!=d2) ok=false;

    nextAddress();
  }

  clsRST();

  if (ok) Serial.println("Compare Ok  :)");
  else    Serial.println("Compare BAD :(");
}

//------------------------------------------------
void readSignature(){
  const uint8_t SIGNATURE_AT89C2051[]={0x1E, 0x21};
  bool at89c2051=true;

  set3457modeP1(LOW,LOW,LOW,LOW,INPUT);  

  for (uint16_t i=0; i<sizeof(SIGNATURE_AT89C2051); i++){
    uint8_t d = readP1();
    Serial.print((int)d,HEX);
    Serial.print(" ,");
    if (d!=SIGNATURE_AT89C2051[i]) at89c2051=false;
    nextAddress();
  }

  Serial.println("");
  if (at89c2051) Serial.println("Signature is at89c2051");
  else           Serial.println("D'nt know this signature");
  
  clsRST();
}

//--------------------------------------------------
typedef struct{
  char letter;                  //Command's letter
  void (*command)();            //Pointer to routines
  char* description;            //Discrition for help
}ar;

const ar Commands[]={
  {'c', &readChipAndCompare,  " - compare with my_prog"},
  {'r', &readChip,            " - read memory from chip"},
  {'s', &readSignature,       " - read signature from chip"},
  {'e', &eraseChip,           " - erase chip"},
  {'w', &writeProg,           " - write my_prog to chip"},
  {'l', &lockBit1,            " - set lock bit 1"},
  {'L', &lockBit2,            " - set lock bit 2"},
};

//--------------------------------------------------
void loop() {
  char    symbol;
  uint8_t i;
  bool    bad_command=true;

  Serial.println("");
  for (i=0; i<sizeof(Commands)/sizeof(ar); i++){
      Serial.print(Commands[i].letter);
      Serial.println(Commands[i].description);
  }

  while  (Serial.available() == 0)
    ;  // empty sycle

  symbol = Serial.read();   // read the incoming byte:
  Serial.print("Typed: ");    Serial.println(symbol);

  for (i=0; i<sizeof(Commands)/sizeof(ar); i++){
    if (Commands[i].letter==symbol){
      Commands[i].command();
      bad_command=false;
    }
  }

  if (bad_command) {
    Serial.println("Bad command");
  }
}
