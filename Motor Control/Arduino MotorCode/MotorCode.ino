
//Program written by:
//Joel Freund, Matt Amyotte, Balazs Gyenes, Tim Ferguson
//As well as credit to the original author of FreqMeasure

#include <FreqMeasure.h>

void setup() 
{
  FreqMeasure.begin();
  pinMode(6, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(2, INPUT);
  attachInterrupt(1,Cruise,RISING);
}


//Variable initialization
double sum=0;
int count=0;
double spd=0;
double frequency=0;
double PedalInValue = 0;
int PedalOutValue = 0;
double CruiseSignal = 0;
int data[4];
double intermediate = 0;
double CurrentIn = 0;
double Average1=0;
double AvCount=0;
long ti=0;
double Current=0;
double r = 0.28;
volatile int NoRun = LOW;


void loop()
{
  
//to output reverse state (1=forward)


//check on/off state (1=on)
  if(digitalRead(11)==HIGH)
   {
    data[1]=1;
   }
  else
   {
    data[1]=0;
   }

//measure input current




//Start of Speed Detection Section
  //Goes into once peak is detected   
  if (FreqMeasure.available()) 
   {
    // average several reading together
    sum = sum + FreqMeasure.read();
    count = count + 1;     
    if (millis() > (ti + 100)) 
     {
       //Converts freq into speed and displays
       frequency = F_CPU / (sum / count);
       sum = 0;
       count = 0;
       ti = millis();
       spd=2.261946711*r*frequency;//constant = 2*pi*3.6/10
       data[2] = spd;
       Current = Average1/AvCount;
       Average1 = 0;
       AvCount = 0;
       data[3] = Current;
      }
   }
  if ((millis() > (ti+200)))
   {
    spd = 0;
    data[2] = spd;
    sum = 0;
    count = 0;
    ti = millis();
    Current = Average1/AvCount;
    data[3] = Current;
    Average1 = 0;
    AvCount = 0;
   }
 //End of Speed Detection

 
// Pedal Functions
  if(CruiseSignal != 0)
   {
    intermediate = (CruiseSignal*1.14 + 0.7);
    PedalOutValue = intermediate*255/5;
    analogWrite(6,PedalOutValue);
   }
  else
   {
    PedalInValue = analogRead(0);
    intermediate = PedalInValue*5/1024;
    intermediate = (intermediate*1.14 + 0.5);
    PedalOutValue = intermediate*255/5;
    analogWrite(6,PedalOutValue);
    analogWrite(5,PedalOutValue);
   }
  if(digitalRead(2)==LOW)
   {
    CruiseSignal=0;
   }
}


void Cruise()
{
  if(analogRead(0) != 0)
  {
   intermediate = analogRead(0);
   CruiseSignal= intermediate*5/1024;
  }
}




