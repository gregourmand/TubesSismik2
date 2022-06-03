// William J.T.R.S 13219007
// Gregory K.M 13219035

// Plant motor DC
// Menggunakan filter digital
// Library SoftwareSerial : https://docs.arduino.cc/learn/built-in-libraries/software-serial
// https://elektrologi.iptek.web.id/membuat-filter-digital-dari-fungsi-transfer-dengan-transformasi-bilinear/
// https://randomnerdtutorials.com/arduino-eeprom-explained-remember-last-led-state/

#include <SoftwareSerial.h>

const byte rxPin = 7;
const byte txPin = 8;

SoftwareSerial sSerial(rxPin,txPin);

float input_motor = 0;    // input ke plant motor x[n]
float output_motor = 0;   // output dari plant motor jadi feedback y[n]
float input_motor1 = 0;   // input ke plant motor sebelumnya x[n-1]
float output_motor1 = 0;  // output dari plant motor sebelumnya y[n-1]

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  
  sSerial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  if(sSerial.available() > 0){
    input_motor = sSerial.parseFloat();

    // coba make rumus web elektrologi
    // y(n)=0.0476 x(n) + 0.0476 x(n-1) + 0.9048 y(n-1)
    output_motor = 0.1286 * input_motor + 0.1286 * input_motor1 + 0.7153 * output_motor1;

    Serial.println(output_motor);
    sSerial.println(output_motor);
    
    // Loop selanjutnya
    output_motor1 = output_motor;
    input_motor1 = input_motor;
    
  }
}
