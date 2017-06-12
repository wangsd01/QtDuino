#include <SPI.h> //for communication to shield
#define countPin  2
#define r1Pin  4
#define r2Pin  5
#define r3Pin  6
#define SIGNPIN  7

#define pulseWidth  100
#define counterMode 'C'
#define stepperMode 'S'
#define pulseSender1  8
#define pulseSender2  9
#define pulseSender3  10
#define pulseSender4  11
#define bPin 19
#define onPin 3

int box[3] = {0,0,0};
int pulsePin = 4;
//int pulseSender = 8;
int ledPin = LED_BUILTIN;
int in;
char mode;
volatile int count = 0;

//counter stuff
int points;
int stepTime;  //in milli seconds
int stepsPerPoint;

//encoder stuff
int chipSelectPin1=10;
int chipSelectPin2=9;
int chipSelectPin3=8;

long encoder1Value = 0;
long encoder2Value = 0;
long encoder3Value = 0;

void setup() 
{
  attachInterrupt(digitalPinToInterrupt(countPin),counter,FALLING);
  pinMode(digitalPinToInterrupt(countPin), INPUT_PULLUP);

  pinMode(onPin,OUTPUT);
  digitalWrite(onPin,HIGH);
  
  pinMode(ledPin,OUTPUT);
  pinMode(r1Pin,OUTPUT);
  pinMode(r2Pin,OUTPUT);
  pinMode(r3Pin,OUTPUT);
  digitalWrite(r1Pin,LOW);
  digitalWrite(r2Pin,LOW);
  digitalWrite(r3Pin,LOW);
  
  pinMode(pulseSender1,OUTPUT);
  pinMode(pulseSender2,OUTPUT);
  pinMode(pulseSender3,OUTPUT);
  pinMode(pulseSender4,OUTPUT);
  
  //digitalWrite(pulseSender,LOW);
  pinMode(SIGNPIN,OUTPUT);
  digitalWrite(SIGNPIN,HIGH);

  //encoder shield pins
  pinMode(chipSelectPin1, OUTPUT);
  pinMode(chipSelectPin2, OUTPUT);
  pinMode(chipSelectPin3, OUTPUT);
  LS7366_Init();
  delay(100);
  
  Serial.begin(9600);
  Serial.println("Arduino is ready!");
  digitalWrite(pulseSender1,LOW);
  digitalWrite(pulseSender2,LOW);
  digitalWrite(pulseSender3,LOW);
  digitalWrite(pulseSender4,LOW);
}

void loop() 
{
  if(Serial.available()>0)
  {
    //String input = Serial.readString();
    mode = Serial.read();

    if(mode == stepperMode)
    {
      pulsePin = Serial.parseInt(); //S: pulse pin
      digitalWrite(pulsePin,HIGH); // choose the right box
      in = Serial.parseInt();//S:number of pulses to send out
      if(in != 0 && pulsePin <= 6 && pulsePin >= 4)
      {
        Serial.print("I received: ");
        Serial.print(pulsePin);//Serial.print(c);
        Serial.println(in);
        //pulsePin = LED_BUILTIN;  // REMOVE WHEN USING
        pulse(in);
        mode = 'N';
      }
      digitalWrite(r1Pin,LOW);
      digitalWrite(r2Pin,LOW); 
      digitalWrite(r3Pin,LOW); 
    }

    else if(mode == counterMode)
    {
      pulsePin = Serial.parseInt();
      digitalWrite(pulsePin,HIGH);
      
      stepsPerPoint = Serial.parseInt();
      stepTime = Serial.parseInt();
      points = Serial.parseInt();
      Serial.print("I am a counter: ");Serial.print(stepsPerPoint);Serial.print(",");Serial.print(stepTime);
      Serial.print(",");Serial.print(points);Serial.print(",Pin:");Serial.println(pulsePin);

      for(int i = 0; i<points;i++)
      {
        count = 0;
        pulse(stepsPerPoint);
        delay(stepTime);
        Serial.println(count);
      }
      digitalWrite(pulsePin,LOW);
      mode = 'N';
      
    }
        else if(mode == 'x')
    {
      encValues();
      mode = 'N';
    }
  }

}

void pulse(int a) //send pulses to pulseSender
{
  int temp = count;
   //Serial.println(a);
  //digitalWrite(pulsePin,HIGH);
  if(a<0)
  {
    digitalWrite(SIGNPIN,LOW);
    a = a*(-1);
    //Serial.print("In sign statement: ");
    //Serial.println(a);
  }
  else
  {
    digitalWrite(SIGNPIN,HIGH);
    //Serial.println("Positive:"+a);
  }
  
  for(long i = 0;i<a;i++)
  {
    int index = 8+(i%4);
    digitalWrite(index,HIGH);
    //Serial.println(8+(i%4));
    //Serial.print("count: ");Serial.println(i);
    delay(pulseWidth);
    digitalWrite(index,LOW);
  }
  
  //digitalWrite(pulseSender,LOW);
  digitalWrite(pulsePin,LOW);
  digitalWrite(pulseSender1,LOW);
  digitalWrite(pulseSender2,LOW);
  digitalWrite(pulseSender3,LOW);
  digitalWrite(pulseSender4,LOW);

  count = temp;

}

void encValues()
{
   encoder1Value = getEncoderValue(1);
 Serial.print("Encoder1:");Serial.print(getEncoderValue(1));Serial.print(", ");Serial.println(encoder1Value);
  Serial.print("Encoder2:");Serial.print(getEncoderValue(2));Serial.print(", ");Serial.println(encoder2Value);
  Serial.print("Encoder3:");Serial.print(getEncoderValue(3));Serial.print(", ");Serial.println(encoder3Value);
  
}

void blink()
{
  if(digitalRead(pulsePin) == LOW)
  {
    digitalWrite(LED_BUILTIN,LOW);
  }
  else if (digitalRead(pulsePin) == HIGH){
    digitalWrite(LED_BUILTIN,HIGH);
  }
}


//all the encoder shield methods

//*****************************************************  
long getEncoderValue(int encoder)
//*****************************************************
{
    unsigned int count1Value, count2Value, count3Value, count4Value;
    long result;
    
    selectEncoder(encoder);
    
     SPI.transfer(0x60); // Request count
    count1Value = SPI.transfer(0x00); // Read highest order byte
    count2Value = SPI.transfer(0x00);
    count3Value = SPI.transfer(0x00);
    count4Value = SPI.transfer(0x00); // Read lowest order byte
    
    deselectEncoder(encoder);
   
    result= ((long)count1Value<<24) + ((long)count2Value<<16) + ((long)count3Value<<8) + (long)count4Value;
    
    return result;
}//end func


//*************************************************
void selectEncoder(int encoder)
//*************************************************
{
  switch(encoder)
  {
     case 1:
        digitalWrite(chipSelectPin1,LOW);
        break;
     case 2:
       digitalWrite(chipSelectPin2,LOW);
       break;
     case 3:
       digitalWrite(chipSelectPin3,LOW);
       break;    
  }//end switch
  
}//end func

//*************************************************
void deselectEncoder(int encoder)
//*************************************************
{
  switch(encoder)
  {
     case 1:
        digitalWrite(chipSelectPin1,HIGH);
        break;
     case 2:
       digitalWrite(chipSelectPin2,HIGH);
       break;
     case 3:
       digitalWrite(chipSelectPin3,HIGH);
       break;    
  }//end switch
  
}//end func


// LS7366 Initialization and configuration
//*************************************************
void LS7366_Init(void)
//*************************************************
{
   
    
    // SPI initialization
    SPI.begin();
    //SPI.setClockDivider(SPI_CLOCK_DIV16);      // SPI at 1Mhz (on 16Mhz clock)
    delay(10);
   
   digitalWrite(chipSelectPin1,LOW);
   SPI.transfer(0x88); 
   SPI.transfer(0x03);
   digitalWrite(chipSelectPin1,HIGH); 
   
   
   digitalWrite(chipSelectPin2,LOW);
   SPI.transfer(0x88); 
   SPI.transfer(0x03);
   digitalWrite(chipSelectPin2,HIGH); 
   
   
   digitalWrite(chipSelectPin3,LOW);
   SPI.transfer(0x88); 
   SPI.transfer(0x03);
   digitalWrite(chipSelectPin3,HIGH); 
   
}//end func

void counter()
{
  count++;
}

