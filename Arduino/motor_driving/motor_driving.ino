#define motorENA 6
#define motorENB 11  //enables both engines

// motor one (right)
#define MotorRightPWMIn1 10
#define MotorRightPWMIn2 9
#define encoderRightPinA 3
#define encoderRightPinB 4

// motor two (Left)
#define MotorLeftPWMIn3 8
#define MotorLeftPWMIn4 7
#define encoderLeftPinA 2
#define encoderLeftPinB 13

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


volatile int encoderNumberRight= 0;
volatile int encoderNumberLeft= 0;
int encoderTotal = 0;

int speedLeftMotor = 140;
int speedRightMotor = 140;

int incomingByte = 0; 

int serialInput[4];

void setup() {


  Serial.begin(9600);     
  // put your setup code here, to run once:
  pinMode(motorENB,OUTPUT);
  pinMode(motorENA,OUTPUT);

  
  pinMode(MotorRightPWMIn1,OUTPUT);
  pinMode(MotorRightPWMIn2,OUTPUT);
  pinMode (encoderRightPinA,INPUT_PULLUP);
  pinMode (encoderRightPinB,INPUT_PULLUP);
  
  pinMode(MotorLeftPWMIn3,OUTPUT);
  pinMode(MotorLeftPWMIn4,OUTPUT);
  pinMode (encoderLeftPinA,INPUT_PULLUP);
  pinMode (encoderLeftPinB,INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(encoderLeftPinA), encoderright, FALLING);
  attachInterrupt(digitalPinToInterrupt(encoderRightPinA), encoderleft, FALLING);

}


void loop() {
  // put your main code here, to run repeatedly:

    //readSerial();

    testEncoders();


   
    moveForward(speedLeftMotor, speedRightMotor);

    //if(encoderNumberRight >= (528)){
      
     // stopCar();
      //Serial.println("stop car:");
     // delay(5000);
      
    //}
    
    //stopCar();
   
   //speedControl();
   //delay(5000);
}


void testEncoders(){
   if(encoderNumberRight%100 == 0 ){
     Serial.print("encoder Right:");
    Serial.println(encoderNumberRight);
   }

   if(encoderNumberLeft%100 == 0){
     Serial.print("encoder Left:");
     Serial.println(encoderNumberLeft);
   }
  
}
void computeSerial(){
   // drive forward
    if(serialInput[0] == SStop){
      stopCar();
    }else if(serialInput[1] > Sneutral){
      // to the right
      moveRight();
    }else if(serialInput[1] < Sneutral){
      // to the left
      moveLeft();
    }else if(serialInput[1] == Sneutral){
      // straight   
      moveForward(speedLeftMotor, speedRightMotor); 
    }
}
void readSerial(){
  if (Serial.available() > 3) {
    for(int i= 0; i<4; i++){         
        serialInput[i] = Serial.read();
        Serial.write(serialInput[i]);
    }
    computeSerial();
  }
}

void stopCar(){
  analogWrite(motorENA,0) ;
  digitalWrite(MotorLeftPWMIn4,LOW) ;
  digitalWrite(MotorLeftPWMIn3,LOW) ;

    analogWrite(motorENB,0);
  digitalWrite(MotorRightPWMIn2,LOW); 
  digitalWrite(MotorRightPWMIn1,LOW);
  
}

void moveRight(){
  moveLeftForward(speedLeftMotor);
  moveRightBackward(speedRightMotor);
}
void moveLeft(){
  moveRightForward(speedRightMotor);
  moveLeftBackward(speedLeftMotor);
}
void moveForward(int speedLeftMotor, int speedRightMotor){
 moveRightForward(speedRightMotor);
 moveLeftForward(speedLeftMotor);
}

void moveBackward(int speedLeftMotor, int speedRightMotor){
 moveRightBackward(speedRightMotor);
 moveLeftBackward(speedLeftMotor);
}

void moveLeftBackward(int speedLeftMotor){
  analogWrite(motorENA,speedLeftMotor) ;
  digitalWrite(MotorLeftPWMIn4,LOW) ;
  digitalWrite(MotorLeftPWMIn3,HIGH) ;
}

void moveRightBackward(int speedRightMotor){
  analogWrite(motorENB,speedRightMotor);
  digitalWrite(MotorRightPWMIn2,LOW); 
  digitalWrite(MotorRightPWMIn1,HIGH);
}


void moveLeftForward(int speedLeftMotor){
    analogWrite(motorENA,speedLeftMotor) ;
    digitalWrite(MotorLeftPWMIn3,LOW) ;
    digitalWrite(MotorLeftPWMIn4,HIGH) ;
}
void moveRightForward(int speedRightMotor){
  analogWrite(motorENB,speedRightMotor);
  digitalWrite(MotorRightPWMIn1,LOW); 
  digitalWrite(MotorRightPWMIn2,HIGH);
}

// TODO
int checkEncoders(){
  return encoderNumberLeft - encoderNumberRight;
}

void speedControl(){
  int motorDifference = (checkEncoders()/25);
  if(( speedRightMotor + ( motorDifference)) < 256){
    speedRightMotor = speedLeftMotor + ( motorDifference);
  }else{
    speedRightMotor = 255;
  }  
}
// END TODO


void encoderright(){
 // if(digitalRead(encoderRightPinB) == HIGH){
    encoderNumberRight++;
//    stopCar();
  //}else{
  //  encoderNumberRight--;
 // }
 
}
void encoderleft(){
 // if(digitalRead(encoderLeftPinB) == HIGH){
    encoderNumberLeft++;
 // }else{
   // encoderNumberLeft--;
  //}
    //Serial.println(encoderNumberLeft);
}


