



//      Pinout Table:
//      -------------
//     | Arduino Pin | Fan Connector Pin |  Function          |
//     | ----------- | ---------------- | ------------------ |
//     |      1      |        1         |  Ground            |
//     |      2      |        2         |  VCC               |
//     |      3      |        3         |  Data (Input/Output)|
//     |      4      |        4         |  Clock (Input)     |
//     |      5      |        5         |  PWM (Output)      |
//     |      6      |        6         |  Tachometer (Input)|





#define PIN_SENSE 2 //where we connected the fan sense pin. Must be an interrupt capable pin (2 or 3 on Arduino Uno)
#define DEBOUNCE 0 //0 is fine for most fans, crappy fans may require 10 or 20 to filter out noise
#define FANSTUCK_THRESHOLD 500 //if no interrupts were received for 500ms, consider the fan as stuck and report 0 RPM
//Interrupt handler. Stores the timestamps of the last 2 interrupts and handles debouncing

unsigned long volatile ts1=0,ts2=0;
void tachISR() {
    unsigned long m=millis();
    if((m-ts2)>DEBOUNCE){
        ts1=ts2;
        ts2=m;
    }
}

unsigned long calcRPM(){
    if(millis()-ts2<FANSTUCK_THRESHOLD&&ts2!=0){
        return (60000/(ts2-ts1))/2;
    }else return 0;
}



void analogWrite25k(int pin, int value)    // Setting up a function to drive the 25KHz Pwm
{
    switch (pin) {
        case 9:
            OCR1A = value;
            break;
        case 10:
            OCR1B = value;
            break;
        default:
            // no other pin will work
            break;
    }
}


void setup()  
{  
  
    int i= 0;
    TCCR1A = 0;           // undo the configuration done by...
    TCCR1B = 0;           // ...the Arduino core library
    TCNT1  = 0;           // reset timer
    TCCR1A = _BV(COM1A1)  // non-inverted PWM on ch. A
           | _BV(COM1B1)  // same on ch; B
           | _BV(WGM11);  // mode 10: ph. correct PWM, TOP = ICR1
    TCCR1B = _BV(WGM13)   // ditto
           | _BV(CS10);   // prescaler = 1
    ICR1   = 320;         // TOP = 320
  
  pinMode(9, OUTPUT); // the declared pin must be among the PWM pins. 
  pinMode(13, OUTPUT); 
  pinMode(PIN_SENSE,INPUT_PULLUP); //set the sense pin as input with pullup resistor
  attachInterrupt(digitalPinToInterrupt(PIN_SENSE),tachISR,FALLING); //set tachISR to be triggered when the signal on the sense pin goes low
  Serial.begin(9600);
}  
void loop()  // Add or append code as per your use case, can be mapped to temperature sensors to control fan speed
{  
  for(i=0;i<16;i++){                  // Basic for loop to ramp up the fan over 16 seconds to full speed
    analogWrite25k(9, 20*i);
    Serial.print("RPM:");
    Serial.println(calcRPM());
    digitalWrite(13, HIGH);
    delay(100); 
    digitalWrite(13, LOW); 
    delay(1000);
  }

  delay(3000);                       // Maintaining full speed for 3 sec

  for(;i>0;i--){                     // For loop to ramp down the fan over 16 seconds
    analogWrite25k(9, 20*i);
    Serial.print("RPM:");
    Serial.println(calcRPM());
    digitalWrite(13, HIGH);
    delay(100); 
    digitalWrite(13, LOW); 
    delay(1000);
  }
}  
