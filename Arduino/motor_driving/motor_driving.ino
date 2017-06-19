#define motorENA 6
#define motorENB 11  //enables both engines

// motor one (Left)
#define MotorLeftPWMIn1 8
#define MotorLeftPWMIn2 7
#define encoderLeftPinA 2
#define encoderLeftPinB 13

// motor two (Right)
#define MotorRightPWMIn3  10
#define MotorRightPWMIn4 9
#define encoderRightPinA 3
#define encoderRightPinB 4

//stepper
#define motorPin1  0     // IN1 on the ULN2003 driver 1
#define motorPin2  1     // IN2 on the ULN2003 driver 1
#define motorPin3  5     // IN3 on the ULN2003 driver 1
#define motorPin4  12     // IN4 on the ULN2003 driver 1

// SERIAL CODES
#define SForward 9
#define SBackward 10
#define SStop 100
#define Sneutral 100


volatile int encoderNumberLeft= 0;
volatile int encoderNumberRight= 0;
int encoderTotal = 0;

int speedRightMotor = 110;
int speedLeftMotor = 110;

int incomingByte = 0; 

int serialInput[4];

void setup() {


  Serial.begin(9600);     
  // put your setup code here, to run once:
  pinMode(motorENB,OUTPUT);
  pinMode(motorENA,OUTPUT);

  
  pinMode(MotorLeftPWMIn1,OUTPUT);
  pinMode(MotorLeftPWMIn2,OUTPUT);
  pinMode (encoderLeftPinA,INPUT_PULLUP);
  pinMode (encoderLeftPinB,INPUT_PULLUP);
  
  pinMode(MotorRightPWMIn3,OUTPUT);
  pinMode(MotorRightPWMIn4,OUTPUT);
  pinMode (encoderRightPinA,INPUT_PULLUP);
  pinMode (encoderRightPinB,INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(encoderRightPinA), encoderLeft, RISING);
  attachInterrupt(digitalPinToInterrupt(encoderLeftPinA), encoderRight, RISING);

}


void loop() {
  // put your main code here, to run repeatedly:

    readSerial();
    //testEncoders();
    delay(500);
    stopCar();
    
    
  
    
    //stopCar();
   
   //speedControl();
   //delay(5000);
}


void testEncoders(){
   if(encoderNumberLeft%100 == 0 ){
     Serial.print("encoder Left:");
    Serial.println(encoderNumberLeft);
   }

   if(encoderNumberRight%100 == 0){
     Serial.print("encoder Right:");
     Serial.println(encoderNumberRight);
   }
  
}
void computeSerial(){
   // drive forward
    if(serialInput[0] == SStop){
      stopCar();
    }else if(serialInput[1] > Sneutral){
      // to the Left
      moveLeft();
    }else if(serialInput[1] < Sneutral){
      // to the Right
      moveRight();
    }else if(serialInput[1] == Sneutral){
      // straight   
      moveForward(speedRightMotor, speedLeftMotor); 
    }
}
void readSerial(){
  if (Serial.available() > 3) {
    for(int i= 0; i<4; i++){         
        serialInput[i] = Serial.read();
       // Serial.write(serialInput[i]);
    }
    computeSerial();
  }else if(Serial.available() == 0){
    for(int i= 0; i<4; i++){         
        serialInput[i] = 0;
    }
  }
}

void stopCar(){
  analogWrite(motorENA,0) ;
  digitalWrite(MotorRightPWMIn4,LOW) ;
  digitalWrite(MotorRightPWMIn3,LOW) ;

  analogWrite(motorENB,0);
  digitalWrite(MotorLeftPWMIn2,LOW); 
  digitalWrite(MotorLeftPWMIn1,LOW);
  
}

void moveLeft(){
  moveRightForward(speedRightMotor);
  moveLeftBackward(speedLeftMotor);
}
void moveRight(){
  moveLeftForward(speedLeftMotor);
  moveRightBackward(speedRightMotor);
}
void moveForward(int speedRightMotor, int speedLeftMotor){
 moveLeftForward(speedLeftMotor);
 moveRightForward(speedRightMotor);
}

void moveBackward(int speedRightMotor, int speedLeftMotor){
 moveLeftBackward(speedLeftMotor);
 moveRightBackward(speedRightMotor);
}

void moveRightBackward(int speedRightMotor){
  analogWrite(motorENA,speedRightMotor) ;
  digitalWrite(MotorRightPWMIn4,LOW) ;
  digitalWrite(MotorRightPWMIn3,HIGH) ;
}

void moveLeftBackward(int speedLeftMotor){
  analogWrite(motorENB,speedLeftMotor);
  digitalWrite(MotorLeftPWMIn2,LOW); 
  digitalWrite(MotorLeftPWMIn1,HIGH);
}


void moveRightForward(int speedRightMotor){
    analogWrite(motorENA,speedRightMotor) ;
    digitalWrite(MotorRightPWMIn3,LOW) ;
    digitalWrite(MotorRightPWMIn4,HIGH) ;
}
void moveLeftForward(int speedLeftMotor){
  analogWrite(motorENB,speedLeftMotor);
  digitalWrite(MotorLeftPWMIn1,LOW); 
  digitalWrite(MotorLeftPWMIn2,HIGH);
}

// TODO
int checkEncoders(){
  return encoderNumberRight - encoderNumberLeft;
}

void speedControl(){
  int motorDifference = (checkEncoders()/25);
  if(( speedLeftMotor + ( motorDifference)) < 256){
    speedLeftMotor = speedRightMotor + ( motorDifference);
  }else{
    speedLeftMotor = 255;
  }  
}
// END TODO


void encoderLeft(){
 // if(digitalRead(encoderLeftPinB) == HIGH){
    encoderNumberLeft++;
//    stopCar();
  //}else{
  //  encoderNumberLeft--;
 // }
 
}
void encoderRight(){
 // if(digitalRead(encoderRightPinB) == HIGH){
    encoderNumberRight++;
 // }else{
   // encoderNumberRight--;
  //}
    //Serial.println(encoderNumberRight);
}


