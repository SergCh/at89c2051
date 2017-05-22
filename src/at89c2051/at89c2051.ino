
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
// 
inline void set3457(uint8_t v33,uint8_t v34,uint8_t v35,uint8_t v37){
  digitalWrite(pinP3_3, v33);
  digitalWrite(pinP3_4, v34);
  digitalWrite(pinP3_5, v35);
  digitalWrite(pinP3_7, v37);
}
//-------------------------------------------
// процесс записывания
void pulsePROG(uint8_t t=1){
  digitalWrite(pinPROG, LOW);
  delay(t);
  digitalWrite(pinPROG, HIGH);
}
//-------------------------------------------
// установка сигнала RST когда не подведено 12в
void setRST(uint8_t v){
  digitalWrite(pinRST, v);
//  switch (v){
//    case LOW:     digitalWrite(pinRST,LOW); 
//                  break;
//    case HIGH:    /*digitalWrite(pinRST12,LOW);*/
//                  digitalWrite(pinRST,HIGH);
//                  break;
//  }
}

//-------------------------------------------
// переходим на следующий адрес
void nextAddress(){
  digitalWrite(pinXTAL1,HIGH);
  delay(1);
  digitalWrite(pinXTAL1,LOW);
  delay(1);
}
//-------------------------------------------
// установка режима чтения или записи
void setP1(uint8_t IO){
  uint8_t i;
  for (i=0;i<8;i++)
    pinMode(pinsP1[i],IO);
}
//-------------------------------------------
// считывание байта данный с порта P1
uint8_t readP1(){
  uint8_t d,i;
  for (d=i=0;i<8;i++){
    d<<=1;
    if (digitalRead(pinsP1[7-i])==HIGH) d|=1;
  }
  return d;  
}
//-------------------------------------------
// установка байта данный на порт P1
void writeP1(uint8_t d){
  uint8_t i;
  for (i=0;i<8;i++){
    digitalWrite(pinsP1[i],(d&1)? HIGH:LOW);
    d>>=1;
  }
}
//-------------------------------------------
uint8_t start12v(){
  uint8_t s;
/*  Serial.println("When put 12v, press 'c'");
  while (Serial.available() == 0);

  s=Serial.read();
  if (s!='c')  {
    Serial.println("Operation was canseled, remove 12 v");
    return 0;
  }
  return 1;*/

  Serial.println("For continue, press 'c'");
  while (Serial.available() == 0);

  s=Serial.read();
  if (s!='c')  {
    Serial.println("Operation was canseled, remove 12 v");
    return 0;
  }
  digitalWrite(pin12v,HIGH);
  delay(10);
  return 1;
}  
//-------------------------------------------
void stop12v(){
/*  Serial.println("\nRemove 12v");*/
  digitalWrite(pin12v,LOW);
}
//-------------------------------------------
void eraseChip(){
  if (!start12v()) return;

  set3457(HIGH,LOW,LOW,LOW);
  pulsePROG(10);
  Serial.println("Erased");

  stop12v();
}
//-------------------------------------------
void writeProg(){
  uint16_t i;
  uint8_t d;
  set3457(LOW,HIGH,HIGH,HIGH);                      //согласно мануалу
  setP1(OUTPUT);                                    //будем передавать данные

  if (!start12v()) return;
  
  for (i=0; i<sizeof(my_prog); i++)  {
    d=pgm_read_byte_near(my_prog+i);
    Serial.print((int)d,HEX);
    Serial.print(" ,");
    writeP1(d);
    pulsePROG(1);
    delay(1);
    nextAddress();
    delay(1);
  }
  Serial.println("");

  stop12v();
}

//----------------------------------------------
void readChip(uint16_t len){
  uint16_t i,d;
  
  set3457(LOW,LOW,HIGH,HIGH);
  
  setRST(HIGH);
  setP1(INPUT);
  delay(1);
  
  for (i=0;i<len;i++)  {
    d = readP1();

    Serial.print((int)d,HEX);
    Serial.print(" ,");

    nextAddress();
  }
  Serial.println("");

  setRST(LOW);
}

//-----------------------------------------------
void readChipAndCompare(){
  uint16_t i,d1,d2,len=sizeof(my_prog);
  uint8_t flag=1;
  set3457(LOW,LOW,HIGH,HIGH);
  
  setRST(HIGH);
  setP1(INPUT);
  delay(1);
  
  for (i=0;i<len;i++)  {
    d1 = readP1();
    d2 = pgm_read_byte_near(my_prog+i);
    if (d1!=d2) flag=0;

    nextAddress();
  }

  setRST(LOW);

  if (flag) Serial.println("Compare Ok");
  else      Serial.println("Compare BAD!!!!");
}

//------------------------------------------------
void readSignature(uint16_t len){
  uint16_t i,d;
  
  set3457(LOW,LOW,LOW,LOW);
  
  setRST(HIGH);
  setP1(INPUT);
  delay(1);
  
  for (i=0;i<len;i++)  {
    d = readP1();
    Serial.print((int)d,HEX);
    Serial.print(" ,");

    nextAddress();
  }

  setRST(LOW);
}

//--------------------------------------------------
void loop() {
  static uint8_t sm=1;
  uint8_t s;
  uint8_t d;

  if(sm==1){
    Serial.println("\ns-signature,\nr-read chip,\ne-erase chip,\nw-write prog,\nc-compare\n");
    sm=0;
  }

  while  (Serial.available() == 0)
          ; 

  s = Serial.read();   // read the incoming byte:

  Serial.print("I received: ");    Serial.println((char)s);

  switch (s){
  case 'c':       //compare with my_prog
        readChipAndCompare();
        break;
  case 'w':       //write my_prog
        writeProg();
        break;
  case 's':       //read signature
        readSignature(16);
        break;
  case 'r':       //read memory
        readChip(2048);
        break;
  case 'e':       //erase memory
        eraseChip();

        break;
  case '\n':      //ignore
  case '\r': break;
  default:
        Serial.println("Bad command");
  }
  set3457(LOW,LOW,LOW,LOW);
  sm=1;


}

