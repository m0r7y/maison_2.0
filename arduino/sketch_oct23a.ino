#include <Arduino.h>
#include <Servo.h>
#include "pitches.h"

#define pinG0 2
#define pinG1 8
#define trig 6
#define eco 7
#define ventillo0 4
#define pompe 5
#define ascensseur0 10
#define ascensseur1 9
#define led1 A5
#define led2 11
#define led4 12
#define led5 13
#define led6 A2
#define ledGarage A13
#define pompeServo A3
#define interupteur0 A1
#define interupteur1 A10
#define vibration A6
#define frontDoor0 15
#define frontDoor1 14
#define alarm0 A7
#define alarm1 A8

#define pGauche 25
#define pDroite 160

#define pinG0 2
#define pinG1 8

void closeGarage(byte, byte);
void openGarage(byte, byte);
void stopDoor(byte, byte);
void func0(byte, byte, short *, short *, unsigned long *);
void funcC(byte, byte, short *, short *, unsigned long *);


short stateAscensseur = 0;
short stateOpenGarage = 0;
short stateCloseGarage = 0;
short statePermanentGarage = 0;
short statePermanentFrontDoor = 0;
short pompeDirection = 1;
short pompePosition = pGauche;
short pompeDelay = 25;
short pompeStatus = 0;
short direction = 1;
short activateSonnerie = 0;
byte allLed[] = {led1, led2, led4, led5, ventillo0};
short siz = sizeof(allLed)/sizeof(allLed[0]);
unsigned long previousTimeGarage = 0;
unsigned long previousTimePompe = 0;

// new variables
short witness = 0;
short stateCloseFrontDoor = 0;
short stateOpenFrontDoor = 0;
short statusSonnerie = 0;
short pauseBetweenNotes = 0;
short statusTone = 0;
short statusAlarme = 0;
boolean sAlarme = true;
unsigned long previousTime = 0;
unsigned long previousTimeFrontDoor = 0;
unsigned long previousTimeAlarme = 0;


short melody[] = {
  50, 100
};

int noteDurations[] = {
  4, 4
};

Servo myservo;


void setup() {  
  Serial.begin(9600);
  for (short i = 0; i < siz; ++i)
    pinMode(allLed[i], OUTPUT);
  
  pinMode(ledGarage, OUTPUT);
  pinMode(pinG0, OUTPUT);
  pinMode(pinG1, OUTPUT);
  pinMode(trig, OUTPUT);
  pinMode(eco, INPUT);
  pinMode(pompe, OUTPUT);
  pinMode(ascensseur0, OUTPUT);
  pinMode(ascensseur1, OUTPUT);
  pinMode(interupteur0, INPUT);
  pinMode(interupteur1, INPUT);
  // exception
  digitalWrite(ventillo0, HIGH);
  digitalWrite(pompe, HIGH);
  // fin
  myservo.attach(pompeServo);
  // new features
  pinMode(vibration, INPUT);
  pinMode(frontDoor0, OUTPUT);
  pinMode(frontDoor1, OUTPUT);
  pinMode(alarm0, OUTPUT);
  pinMode(alarm1, OUTPUT);
}


void loop() {
  char a = Serial.read();
  
  digitalWrite(trig, LOW);
  delay(2);
  digitalWrite(trig, HIGH);
  delay(2);
  digitalWrite(trig, LOW);
  float distance = pulseIn(eco, HIGH)/58;


  short a1 = analogRead(interupteur1);
  short a0 = analogRead(interupteur0);

  switch(a)
  {
    case 'o': funcO(pinG0, pinG1, &stateCloseGarage, &stateOpenGarage, &statePermanentGarage, &previousTimeGarage);
              break;
    case 'c': funcC(pinG0, pinG1, &stateCloseGarage, &stateOpenGarage, &statePermanentGarage, &previousTimeGarage);
              break;
    case 'e': digitalWrite(led1, HIGH);
              break;
    case 'd': digitalWrite(led1, LOW);
              break;
    case 'w': digitalWrite(led2, HIGH);
              break;
    case 'x': digitalWrite(led2, LOW);
              break;
    case 't': digitalWrite(led4, HIGH);
              break;
    case 'y': digitalWrite(led4, LOW);
              break;
    case 'j': digitalWrite(led5, HIGH);
              break;
    case 'k': digitalWrite(led5, LOW);
              break;
    case 'i': digitalWrite(ventillo0, HIGH);
              break;
    case 'u': digitalWrite(ventillo0, LOW);
              break;
    case 'l': digitalWrite(ledGarage, HIGH);
              break;
    case 'm': digitalWrite(ledGarage, LOW);
              break;
    case '2': digitalWrite(pompe, HIGH);
              pompeStatus = 0;
              break;
    case '1': digitalWrite(pompe, LOW);
              pompeStatus = 1;
              break;
    case 'v': openGarage(ascensseur0, ascensseur1);
              stateAscensseur = 1;
              break;
    case 'b': closeGarage(ascensseur0, ascensseur1);
              stateAscensseur = -1;
              break;
    case 'a': digitalWrite(led1, HIGH);
              digitalWrite(led2, HIGH);
              digitalWrite(led4, HIGH);
              digitalWrite(led5, HIGH);
              digitalWrite(ledGarage, HIGH);
              break;
    case 'z': funcC(pinG0, pinG1, &stateCloseGarage, &stateOpenGarage, &statePermanentGarage, &previousTimeGarage);
              funcC(frontDoor1, frontDoor0, &stateCloseFrontDoor, &stateOpenFrontDoor, &statePermanentFrontDoor, &previousTimeFrontDoor);
              digitalWrite(led1, LOW);
              digitalWrite(led2, LOW);
              digitalWrite(led4, LOW);
              digitalWrite(led5, LOW);
              digitalWrite(ventillo0, HIGH);
              digitalWrite(ledGarage, LOW);
              break;
    case '3': funcO(frontDoor1, frontDoor0, &stateCloseFrontDoor, &stateOpenFrontDoor, &statePermanentFrontDoor, &previousTimeFrontDoor);
              break;
    case '4': funcC(frontDoor1, frontDoor0, &stateCloseFrontDoor, &stateOpenFrontDoor, &statePermanentFrontDoor, &previousTimeFrontDoor);
              break;
    case '6': statusSonnerie = 0;
              activateSonnerie = 0;
              witness = 0;
              statusAlarme = 0;
              break;
    case '7': activateSonnerie = 1;
              statusAlarme = 1;
              if (!statusAlarme){
                previousTimeAlarme = millis();
                statusAlarme = 1;
              }
              break;
    case '9': short stateAll[11] = {statePermanentGarage, digitalRead(led1), digitalRead(led2), 
                                    digitalRead(led4),  digitalRead(led5), digitalRead(ventillo0), 
                                    digitalRead(ledGarage), pompeStatus, statePermanentFrontDoor,
                                    statusAlarme, activateSonnerie};
                                    
              for (int i = 0; i < 11; ++i)
                Serial.write(stateAll[i]);
              break;
  }
  if (pompeStatus){
    servoRotation(&myservo, &previousTimePompe, &pompePosition, &direction, pGauche, pDroite, pompeDelay);
  }

  // garage, détecteur de présence
  if (millis() - previousTimeGarage >= 2000 || distance <= 14 && stateCloseGarage){
    statePermanentGarage = 2;
    stopDoor(pinG0, pinG1);
    stateCloseGarage = stateOpenGarage = 0;
  }

  //ascensseur
  if (a1 >= 900 && stateAscensseur == -1 || a0 >= 900 && stateAscensseur == 1 || a == '3'){
     stopDoor(ascensseur0, ascensseur1);
     stateAscensseur = 0;
  }

  // frontDoor 
  if (millis() - previousTimeFrontDoor >= 225){
    stopDoor(frontDoor0, frontDoor1);
    stateCloseFrontDoor = stateOpenFrontDoor = 0;
  }

  // vibration et sonnerie
  if (activateSonnerie && (digitalRead(vibration) || statusSonnerie)){
    statusSonnerie = 1;
    funcC(pinG0, pinG1, &stateCloseGarage, &stateOpenGarage, &statePermanentGarage, &previousTimeGarage);
    funcC(frontDoor1, frontDoor0, &stateCloseFrontDoor, &stateOpenFrontDoor, &statePermanentFrontDoor, &previousTimeFrontDoor);
              digitalWrite(led1, HIGH);
              digitalWrite(led2, HIGH);
              digitalWrite(led4, HIGH);
              digitalWrite(led5, HIGH);
              digitalWrite(ventillo0, HIGH);
              digitalWrite(ledGarage, HIGH);
    laser();
   
  // bouton 5
    if (statusAlarme){
      digitalWrite(alarm1, sAlarme);
      if (millis() - previousTimeAlarme >= 1000){
        sAlarme = !sAlarme;
      }
    }
  }

  //send states
  if (a == '9'){
    short stateAll[11] = {statePermanentGarage, digitalRead(led1), digitalRead(led2), 
                          digitalRead(led4),  digitalRead(led5), digitalRead(ventillo0), 
                          digitalRead(ledGarage), pompeStatus, statePermanentFrontDoor,
                          statusAlarme, activateSonnerie};          
    for (int i = 0; i < 11; ++i)
      Serial.write(stateAll[i]);
  }
}

void funcO(byte p0, byte p1, short *stateClose, short *stateOpen, short *statePermanent, unsigned long *previousTime)
{
  if (*stateClose){
      stopDoor(p0, p1);
      *statePermanent = 2;
    }
    else{
      openGarage(p0, p1);
      *statePermanent = 1;
      if (*stateOpen != 1)
        *previousTime = millis();
      *stateOpen = 1;
    }
}

void funcC(byte p0, byte p1, short *stateClose, short *stateOpen, short *statePermanent, unsigned long *previousTime)
{
  if (*stateOpen){
      stopDoor(p0, p1);
      *statePermanent = 2;
    }
    else{
      closeGarage(p0, p1);
      *statePermanent = 0;
      if (*stateClose != 1)
        *previousTime = millis();
      *stateClose = 1;
    }
}

void closeGarage(byte p0, byte p1)
{
  digitalWrite(p0, HIGH);
  digitalWrite(p1, LOW);
}

void openGarage(byte p0, byte p1)
{
  digitalWrite(p0, LOW);
  digitalWrite(p1, HIGH);
}

void stopDoor(byte p0, byte p1)
{
  digitalWrite(p0, LOW);
  digitalWrite(p1, LOW);
}

void servoRotation(Servo *p, unsigned long *t, short *position, short *direc, short mG, short mD, short d){
  p->write(*position);
  if (millis() - *t >= d){
    *t = millis();
    if (*direc)
      (*position)++;
    else
      (*position)--;
  }
  if (*position == mG){
    *direc = 1;
    *position = mG + 1;
  }
  if (*position == mD){
    *direc = 0;
    *position = mD - 1;
  }
}

void laser(){
  short noteDuration = 1000 / noteDurations[witness];
  tone(alarm0, melody[witness], noteDuration);
  if (!statusTone){
    statusTone = 1;
    pauseBetweenNotes = noteDuration*1.30;
  }
  if (millis() - previousTime >= pauseBetweenNotes){
    previousTime = millis();
    witness++;
    statusTone = 0;
    noTone(alarm0);
  }
  if (witness == 2){
    witness = 0;
  }
}
