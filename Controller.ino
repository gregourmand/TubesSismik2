// William J.T.R.S 13219007
// Gregory K.M 13219035

// Controller
// Referensi
// multiple input : https://forum.arduino.cc/t/serial-input-basics-updated/382007/5
// Timer : https://forum.arduino.cc/t/using-timer1-to-blink-a-led-once-per-second-for-50ms/588721/8

#include <SoftwareSerial.h>
#include <EEPROM.h>

const byte rxPin = 7;
const byte txPin = 8;

SoftwareSerial sSerial(rxPin,txPin);

const byte numChars = 32;
char receivedChars[numChars];
char tempChars[numChars];        

float input = 0.0;
float kp = 0.0;
float ki = 0.0;
float kd = 0.0;
float output = 0.0;

float elapsedTime;
float error = 0.0 ;
float lastError = 0.0;
float cumError = 0.0;
float rateError = 0.0;
float out = 0.0;

float input_pot;
int pot = A5;
boolean newData = false;

int timer1_counter;

float input_motor = 0;    // input ke plant motor x[n]
float output_motor = 0;   // output dari plant motor jadi feedback y[n]
float input_motor1 = 0;   // input ke plant motor sebelumnya x[n-1]
float output_motor1 = 0;  // output dari plant motor sebelumnya y[n-1]

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  sSerial.begin(9600);
  Serial.println("input data <input, kp, ki, kd>");

  // cek data input, kp, kd, ki di EEPROM
  input = EEPROM.read(0);
  kp = EEPROM.read(1);
  kd = EEPROM.read(2);
  ki = EEPROM.read(3);

  noInterrupts();           // disable all interrupts
  TCCR1A = 0;
  TCCR1B = 0;
  
  timer1_counter = 0xF3CB;  // 50ms 0.05/((1/16M)*64) - 1 = 12499 = 0xF3CB
  
  TCNT1 = timer1_counter;   // preload timer
  TCCR1B |= (1 << CS12);    // 256 prescaler 
  TIMSK1 |= (1 << TOIE1);   // enable timer overflow interrupt
  interrupts();             // enable all interrupts
}

int max_control;
int min_control;

ISR(TIMER1_OVF_vect) {
    TCNT1 = timer1_counter;

    error = input - output;
    
    //out = -A1/A0*u[k-1] - A2/A0*u[k-2] + B0/A0*e[k] + B1/A0*e[k-1] + B2/A0*e[k-2]
    elapsedTime = 5;        //compute time elapsed from previous computation

    error = input - output;                    // determine error
    cumError = cumError+error ;                // compute integral
    
    rateError = (error - lastError);   // compute derivative

//    Serial.print(error);
//    Serial.print(" + ");
//    Serial.print(cumError);
//    Serial.print(" + ");
//    Serial.println(rateError);
    
    out = kp*error + (ki*elapsedTime)*cumError + (kd/elapsedTime)*rateError;                //PID output               

    sSerial.println(out);
     
    lastError = error;                      
}

void loop() {
    input_pot = analogRead(pot);  // input potensiometer
    
    recvWithStartEndMarkers();
    if (newData == true) {
        strcpy(tempChars, receivedChars);
        
        input = 0.0;
        kp = 0.0;
        ki = 0.0;
        kd = 0.0;
        output = 0.0;

        error = 0.0 ;
        lastError = 0.0;
        cumError = 0.0;
        rateError = 0.0;
        out = 0.0;
        
        parseData();
        showParsedData();
        newData = false;
    }
    
    //showParsedData();
    
    if (sSerial.available() > 0){
      output = sSerial.parseFloat();
      //error = input - output;
    }
    
}


void recvWithStartEndMarkers() {
    static boolean recvInProgress = false;
    static byte ndx = 0;
    char startMarker = '<';
    char endMarker = '>';
    char rc;

    while (Serial.available() > 0 && newData == false) {
        rc = Serial.read();

        if (recvInProgress == true) {
            if (rc != endMarker) {
                receivedChars[ndx] = rc;
                ndx++;
                if (ndx >= numChars) {
                    ndx = numChars - 1;
                }
            }
            else {
                receivedChars[ndx] = '\0'; // terminate the string
                recvInProgress = false;
                ndx = 0;
                newData = true;
            }
        }

        else if (rc == startMarker) {
            recvInProgress = true;
        }
    }
}

void parseData() {      // split the data into its parts

    char * strtokIndx; // this is used by strtok() as an index

    strtokIndx = strtok(tempChars,",");      // get the first part - the string
    input = atof(strtokIndx);
    //EEPROM.update(0, input); 
 
    strtokIndx = strtok(NULL, ","); // this continues where the previous call left off
    kp = atof(strtokIndx); 
    EEPROM.update(1, kp);

    strtokIndx = strtok(NULL, ","); // this continues where the previous call left off
    ki = atof(strtokIndx); 
    EEPROM.update(2, ki);

    strtokIndx = strtok(NULL, ","); // this continues where the previous call left off
    kd = atof(strtokIndx); 
    EEPROM.update(3, kd);
}

void showParsedData() {
    Serial.print("input: ");
    Serial.println(input);
    Serial.print("Kp: ");
    Serial.println(kp);
    Serial.print("Ki: ");
    Serial.println(ki);
    Serial.print("Kd: ");
    Serial.println(kd);
    Serial.println();
}
