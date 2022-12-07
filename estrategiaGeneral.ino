#include "CytronMotorDriver.h"
#include <Time.h>


// Configure the motor driver.
CytronMD motor1(PWM_DIR, 9, 12);  // PWM 1 = Pin 3, DIR 1 = Pin 4.
CytronMD motor2(PWM_DIR, 3, 4); // PWM 2 = Pin 9, DIR 2 = Pin 12. creo que el dir debe ser 8, probare asi
bool initTurn = 1;
bool searchStrat = 0;

#define line3sh 110 //this is the threshold for the line sensor
#define maxSpeed 255 //velocidad maxima
#define maxReverseSpeed -255
#define waitStartTime 5000
#define switchPin 13


long int stateStartTime;
long int currentTime;

unsigned int lineSensors[2] = {unsigned(analogRead(A0)),unsigned(analogRead(A1))};
bool objectSensors[3]; //for safety we dont assign the pins yet (we declare them as input first)

enum direction {left, mid, right};
#define leftIRPin 6
#define midIRPin 7
#define rightIRPin 8

#define leftFacePin 11
#define backFacePin 10
#define rightFacePin 5
#define frontJabPin 2



uint8_t counter=0;

bool start = digitalRead(switchPin); //todavia no defini

bool corrected=0;

inline void turn180() __attribute__((always_inline));
inline void allIn() __attribute__((always_inline));
inline void boundCorrection() __attribute__((always_inline));
inline void forward70() __attribute__((always_inline));
inline void forward50() __attribute__((always_inline));
inline void correctAttackLeft() __attribute__((always_inline));
inline void correctAttackRight() __attribute__((always_inline));
inline void leftTurn() __attribute__((always_inline));
inline void rightTurn() __attribute__((always_inline));
inline void searching() __attribute__((always_inline));
inline void moderateForward() __attribute__((always_inline));
inline void turnRight90() __attribute__((always_inline));
inline void turnLeft90() __attribute__((always_inline));
inline void backAway() __attribute__((always_inline));

bool frontFace, rightFace, leftFace, backFace, frontJab;

void setup(){
  pinMode(A0,INPUT);
  pinMode(A1,INPUT);
  pinMode(6,INPUT);
  pinMode(7,INPUT);
  pinMode(8,INPUT);
  pinMode(13,INPUT);

  frontFace=0;
  rightFace=0; 
  leftFace=0; 
  backFace=0; 
  frontJab=0;

  start = digitalRead(switchPin);


//    Serial.begin(9600);
//    Serial.println("Hola mundo");
//  Serial.println(leftFace);
//  Serial.println(rightFace);
//  Serial.println(backFace);
//  Serial.println(frontJab);
//  Serial.println(frontFace);
//  delay(10000);
 
  while(!start){
    start = digitalRead(switchPin);
  }
  delay(5000);


}

void loop(){
  if (digitalRead(leftFacePin)){
    leftFace = 1;
  }
  else if (digitalRead(rightFacePin)){
    rightFace = 1;
  }
  else if (digitalRead(backFacePin)){
    backFace = 1;
  }
  else if (digitalRead(frontJabPin)){
    frontJab = 1;
  }
  else frontFace=1;
  
  lineSensors[0] = analogRead(A0);
  lineSensors[1] = analogRead(A1);

  objectSensors[left] = digitalRead(leftIRPin);
  objectSensors[mid] = digitalRead(midIRPin); 
  objectSensors[right] = digitalRead(rightIRPin);

  if(leftFace){
    if (initTurn){
    turnRight90();
    initTurn=0;
    }
    //para 90* este ya se que funciona aproximado
    moderateForward();
    
    if(!objectSensors[mid]){
      allIn();
    }
    objectSensors[left] = digitalRead(leftIRPin);
    objectSensors[mid] = digitalRead(midIRPin);
    objectSensors[right] = digitalRead(rightIRPin);
    if (!objectSensors[left] && objectSensors[right] && !corrected){
      correctAttackLeft();
      corrected = 1;
    }
    else if(!objectSensors[right] && objectSensors[left] && !corrected){
      correctAttackRight();
      corrected = 1;
    }
    else{
      allIn();
      start = digitalRead(switchPin);
    }
    lineSensors[0] = analogRead(A0);
    lineSensors[1] = analogRead(A1);
    if ( lineSensors[0] < line3sh || lineSensors[1] < line3sh ) {
      searchStrat = 1;
      boundCorrection();
    }


    while(searchStrat){
    
      stateStartTime = millis();
      currentTime = millis();

      while( (currentTime - stateStartTime) <200 ){
        objectSensors[left] = digitalRead(leftIRPin);
        objectSensors[mid] = digitalRead(midIRPin);
        objectSensors[right] = digitalRead(rightIRPin);
        corrected = 0;
        while(!objectSensors[mid]){
          lineSensors[0] = analogRead(A0);
          lineSensors[1] = analogRead(A1);
          if (lineSensors[0] < line3sh ||  lineSensors[1] < line3sh ) { //esto era un "or" antes por si acaso haya errores
            boundCorrection();
          }
          objectSensors[left] = digitalRead(leftIRPin);
          objectSensors[mid] = digitalRead(midIRPin);
          objectSensors[right] = digitalRead(rightIRPin);
          if (!objectSensors[left] && objectSensors[right] && !corrected){
            correctAttackLeft();
            corrected = 1;
          }
          else if(!objectSensors[right] && objectSensors[left] && !corrected){
            correctAttackRight();
            corrected = 1;
          }
          else{
            allIn();
            start = digitalRead(switchPin);
          }
          if (!start) break;
        }
        
        if (!objectSensors[left]){
          leftTurn();
          continue; //empieza el while de nuevo
        }
        
        else if (!objectSensors[right]){
          rightTurn();
          //delay(80); //antes estaban en 100
          continue;
        }
        else currentTime = millis();
      }
        
        
      if (counter == 4) {
        counter = 0;
        forward50();   
      }
      else{
        searching();
        //usar un counter
        counter +=1;
      }
    
    }


    
    start = digitalRead(switchPin); //this is duplicated because 
    while(!start){
      start = digitalRead(switchPin);
      motor1.setSpeed(0);    // Motor 1 runs forward at 100% speed.
      motor2.setSpeed(0);
    }

  }
  else if (rightFace){
    if (initTurn){
    turnLeft90();
    initTurn=0;
    }
    //para 90* este ya se que funciona aproximado
    moderateForward();
    
    if(!objectSensors[mid]){
      allIn();
    }

    objectSensors[left] = digitalRead(leftIRPin);
    objectSensors[mid] = digitalRead(midIRPin);
    objectSensors[right] = digitalRead(rightIRPin);
    if (!objectSensors[left] && objectSensors[right] && !corrected){
      correctAttackLeft();
      corrected = 1;
    }
    else if(!objectSensors[right] && objectSensors[left] && !corrected){
      correctAttackRight();
      corrected = 1;
    }
    else{
      allIn();
      start = digitalRead(switchPin);
    }

    
    lineSensors[0] = analogRead(A0);
    lineSensors[1] = analogRead(A1);
    if ( lineSensors[0] < line3sh || lineSensors[1] < line3sh ) {
      searchStrat = 1;
      boundCorrection();
    }


    while(searchStrat){
    
      stateStartTime = millis();
      currentTime = millis();

      while( (currentTime - stateStartTime) <200 ){
        objectSensors[left] = digitalRead(leftIRPin);
        objectSensors[mid] = digitalRead(midIRPin);
        objectSensors[right] = digitalRead(rightIRPin);
        corrected = 0;
        while(!objectSensors[mid]){
          lineSensors[0] = analogRead(A0);
          lineSensors[1] = analogRead(A1);
          if (lineSensors[0] < line3sh ||  lineSensors[1] < line3sh ) { //esto era un "or" antes por si acaso haya errores
            boundCorrection();
          }
          objectSensors[left] = digitalRead(leftIRPin);
          objectSensors[mid] = digitalRead(midIRPin);
          objectSensors[right] = digitalRead(rightIRPin);
          if (!objectSensors[left] && objectSensors[right] && !corrected){
            correctAttackLeft();
            corrected = 1;
          }
          else if(!objectSensors[right] && objectSensors[left] && !corrected){
            correctAttackRight();
            corrected = 1;
          }
          else{
            allIn();
            start = digitalRead(switchPin);
          }
          if (!start) break;
        }
        
        if (!objectSensors[left]){
          leftTurn();
          continue; //empieza el while de nuevo
        }
        
        else if (!objectSensors[right]){
          rightTurn();
          //delay(80); //antes estaban en 100
          continue;
        }
        else currentTime = millis();
      }
        
        
      if (counter == 4) {
        counter = 0;
        forward50();   
      }
      else{
        searching();
        //usar un counter
        counter +=1;
      }
    
    }
    start = digitalRead(switchPin); //this is duplicated because 
    while(!start){
      start = digitalRead(switchPin);
      motor1.setSpeed(0);    // Motor 1 runs forward at 100% speed.
      motor2.setSpeed(0);
    }

  }
  else if(backFace){
    if (initTurn){
    turn180();
    initTurn=0;
    }
    //para 90* este ya se que funciona aproximado
    moderateForward();
    
    if(!objectSensors[mid]){
      allIn();
    }

    objectSensors[left] = digitalRead(leftIRPin);
    objectSensors[mid] = digitalRead(midIRPin);
    objectSensors[right] = digitalRead(rightIRPin);
    if (!objectSensors[left] && objectSensors[right] && !corrected){
      correctAttackLeft();
      corrected = 1;
    }
    else if(!objectSensors[right] && objectSensors[left] && !corrected){
      correctAttackRight();
      corrected = 1;
    }
    else{
      allIn();
      start = digitalRead(switchPin);
    }
    
    lineSensors[0] = analogRead(A0);
    lineSensors[1] = analogRead(A1);
    if ( lineSensors[0] < line3sh || lineSensors[1] < line3sh ) {
      searchStrat = 1;
      boundCorrection();
    }


    while(searchStrat){
    
      stateStartTime = millis();
      currentTime = millis();

      while( (currentTime - stateStartTime) <200 ){
        objectSensors[left] = digitalRead(leftIRPin);
        objectSensors[mid] = digitalRead(midIRPin);
        objectSensors[right] = digitalRead(rightIRPin);
        corrected = 0;
        while(!objectSensors[mid]){
          lineSensors[0] = analogRead(A0);
          lineSensors[1] = analogRead(A1);
          if (lineSensors[0] < line3sh ||  lineSensors[1] < line3sh ) { //esto era un "or" antes por si acaso haya errores
            boundCorrection();
          }
          objectSensors[left] = digitalRead(leftIRPin);
          objectSensors[mid] = digitalRead(midIRPin);
          objectSensors[right] = digitalRead(rightIRPin);
          if (!objectSensors[left] && objectSensors[right] && !corrected){
            correctAttackLeft();
            corrected = 1;
          }
          else if(!objectSensors[right] && objectSensors[left] && !corrected){
            correctAttackRight();
            corrected = 1;
          }
          else{
            allIn();
            start = digitalRead(switchPin);
          }
          if (!start) break;
        }
        
        if (!objectSensors[left]){
          leftTurn();
          continue; //empieza el while de nuevo
        }
        
        else if (!objectSensors[right]){
          rightTurn();
          //delay(80); //antes estaban en 100
          continue;
        }
        else currentTime = millis();
      }
        
        
      if (counter == 4) {
        counter = 0;
        forward50();   
      }
      else{
        searching();
        //usar un counter
        counter +=1;
      }
    
    }


    
    start = digitalRead(switchPin); //this is duplicated because 
    while(!start){
      start = digitalRead(switchPin);
      motor1.setSpeed(0);    // Motor 1 runs forward at 100% speed.
      motor2.setSpeed(0);
    }

  }
  else if (frontJab){
    if (initTurn){
      backAway();
      initTurn=0;
    }
    if(!objectSensors[mid]){
      allIn();
    }
    objectSensors[left] = digitalRead(leftIRPin);
    objectSensors[mid] = digitalRead(midIRPin);
    objectSensors[right] = digitalRead(rightIRPin);
    if (!objectSensors[left] && objectSensors[right] && !corrected){
      correctAttackLeft();
      corrected = 1;
    }
    else if(!objectSensors[right] && objectSensors[left] && !corrected){
      correctAttackRight();
      corrected = 1;
    }
    else{
      allIn();
      start = digitalRead(switchPin);
    }
    lineSensors[0] = analogRead(A0);
    lineSensors[1] = analogRead(A1);
    if ( lineSensors[0] < line3sh || lineSensors[1] < line3sh ) {
      searchStrat = 1;
      boundCorrection();
    }


    while(searchStrat){
    
      stateStartTime = millis();
      currentTime = millis();

      while( (currentTime - stateStartTime) <200 ){
        objectSensors[left] = digitalRead(leftIRPin);
        objectSensors[mid] = digitalRead(midIRPin);
        objectSensors[right] = digitalRead(rightIRPin);
        corrected = 0;
        while(!objectSensors[mid]){
          lineSensors[0] = analogRead(A0);
          lineSensors[1] = analogRead(A1);
          if (lineSensors[0] < line3sh ||  lineSensors[1] < line3sh ) { //esto era un "or" antes por si acaso haya errores
            boundCorrection();
          }
          objectSensors[left] = digitalRead(leftIRPin);
          objectSensors[mid] = digitalRead(midIRPin);
          objectSensors[right] = digitalRead(rightIRPin);
          if (!objectSensors[left] && objectSensors[right] && !corrected){
            correctAttackLeft();
            corrected = 1;
          }
          else if(!objectSensors[right] && objectSensors[left] && !corrected){
            correctAttackRight();
            corrected = 1;
          }
          else{
            allIn();
            start = digitalRead(switchPin);
          }
          if (!start) break;
        }
        
        if (!objectSensors[left]){
          leftTurn();
          continue; //empieza el while de nuevo
        }
        
        else if (!objectSensors[right]){
          rightTurn();
          //delay(80); //antes estaban en 100
          continue;
        }
        else currentTime = millis();
      }
        
        
      if (counter == 4) {
        counter = 0;
        forward50();   
      }
      else{
        searching();
        //usar un counter
        counter +=1;
      }
    
    }


    
    start = digitalRead(switchPin); //this is duplicated because 
    while(!start){
      start = digitalRead(switchPin);
      motor1.setSpeed(0);    // Motor 1 runs forward at 100% speed.
      motor2.setSpeed(0);
    }

  }
  else{
    //para 90* este ya se que funciona aproximado
    allIn();
    
    if(!objectSensors[mid]){
      allIn();
    }

    objectSensors[left] = digitalRead(leftIRPin);
    objectSensors[mid] = digitalRead(midIRPin);
    objectSensors[right] = digitalRead(rightIRPin);
    if (!objectSensors[left] && objectSensors[right] && !corrected){
      correctAttackLeft();
      corrected = 1;
    }
    else if(!objectSensors[right] && objectSensors[left] && !corrected){
      correctAttackRight();
      corrected = 1;
    }
    else{
      allIn();
      start = digitalRead(switchPin);
    }
    
    lineSensors[0] = analogRead(A0);
    lineSensors[1] = analogRead(A1);
    if ( lineSensors[0] < line3sh || lineSensors[1] < line3sh ) {
      searchStrat = 1;
      boundCorrection();
    }


    while(searchStrat){
    
      stateStartTime = millis();
      currentTime = millis();

      while( (currentTime - stateStartTime) <200 ){
        objectSensors[left] = digitalRead(leftIRPin);
        objectSensors[mid] = digitalRead(midIRPin);
        objectSensors[right] = digitalRead(rightIRPin);
        corrected = 0;
        while(!objectSensors[mid]){
          lineSensors[0] = analogRead(A0);
          lineSensors[1] = analogRead(A1);
          if (lineSensors[0] < line3sh ||  lineSensors[1] < line3sh ) { //esto era un "or" antes por si acaso haya errores
            boundCorrection();
          }
          objectSensors[left] = digitalRead(leftIRPin);
          objectSensors[mid] = digitalRead(midIRPin);
          objectSensors[right] = digitalRead(rightIRPin);
          if (!objectSensors[left] && objectSensors[right] && !corrected){
            correctAttackLeft();
            corrected = 1;
          }
          else if(!objectSensors[right] && objectSensors[left] && !corrected){
            correctAttackRight();
            corrected = 1;
          }
          else{
            allIn();
            start = digitalRead(switchPin);
          }
          if (!start) break;
        }
        
        if (!objectSensors[left]){
          leftTurn();
          continue; //empieza el while de nuevo
        }
        
        else if (!objectSensors[right]){
          rightTurn();
          //delay(80); //antes estaban en 100
          continue;
        }
        else currentTime = millis();
      }
        
        
      if (counter == 4) {
        counter = 0;
        forward50();
        delay(50);   
      }
      else{
        searching();
        //usar un counter
        counter +=1;
      }
    
    }

    start = digitalRead(switchPin); //this is duplicated because 
    while(!start){
      start = digitalRead(switchPin);
      motor1.setSpeed(0);    // Motor 1 runs forward at 100% speed.
      motor2.setSpeed(0);
    }


  }


}
void turnRight90() {
  motor1.setSpeed(maxSpeed);
  motor2.setSpeed(maxReverseSpeed);
  delay(240); //en el otro es 270 pero gira de mas por este lado
}

void allIn(){
  motor1.setSpeed(maxSpeed);
  motor2.setSpeed(maxSpeed);
}

void boundCorrection(){
  motor1.setSpeed(0.7*maxReverseSpeed);
  motor2.setSpeed(0.7*maxReverseSpeed);
  delay(300); //un num arbitario para ir probando nada mas
  motor1.setSpeed(0.7*maxSpeed);
  motor2.setSpeed(0.7*maxReverseSpeed);
  delay(400);
}

void forward70(){
  motor1.setSpeed(0.7*maxSpeed);
  motor2.setSpeed(0.7*maxSpeed); 
}

void forward50(){
  motor1.setSpeed(0.5*maxSpeed);
  motor2.setSpeed(0.5*maxSpeed);
}

void correctAttackLeft(){
  motor1.setSpeed(0);
  motor2.setSpeed(maxSpeed);
  delay(50); //tiempo a modificar
}

void correctAttackRight(){
  motor1.setSpeed(maxSpeed);
  motor2.setSpeed(0);
  delay(50);
}

void leftTurn(){
  motor1.setSpeed(maxReverseSpeed);
  motor2.setSpeed(maxSpeed);
}

void rightTurn(){
  motor1.setSpeed(maxSpeed);
  motor2.setSpeed(maxReverseSpeed);
}

void searching(){
  motor1.setSpeed(maxReverseSpeed);
  motor2.setSpeed(maxSpeed);
  delay(200);
  motor1.setSpeed(0.5*maxSpeed);
  motor2.setSpeed(0.5*maxSpeed);
}

void moderateForward(){
  motor1.setSpeed(0.55*maxSpeed);
  motor2.setSpeed(0.55*maxSpeed);
}

void turnLeft90() {
  motor1.setSpeed(maxReverseSpeed);
  motor2.setSpeed(maxSpeed);
  delay(200); //estaba en 270 recien
}

void backAway(){
  motor1.setSpeed(maxReverseSpeed);
  motor2.setSpeed(maxSpeed);
  delay(180);
  motor1.setSpeed(maxReverseSpeed);
  motor2.setSpeed(maxReverseSpeed);
  delay(750); //antes tenia 700
  motor1.setSpeed(maxSpeed);
  motor2.setSpeed(maxSpeed);
}

void turn180() {
  motor1.setSpeed(maxReverseSpeed);
  motor2.setSpeed(maxSpeed);
  delay(420); //estaba en mas pero calibramos
}
