#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(7, 8); // CE, CSN
const byte address[6] = "00555";

struct DataPkg {
  //left joystick
  byte throttle;
  byte push1;

  //right joystick
  byte xAxis;
  byte yAxis;
  byte push2;
};

DataPkg pkg;

//'safe' values to use when no radio input is detected
void resetData(){
  //left joystick
  pkg.throttle = 0;
  pkg.push1 = 0;
  
  //right joystick
  pkg.xAxis = 127;
  pkg.yAxis = 127;
  pkg.push2 = 0;
  //setPPMValuesFromData();
}

void setup() {
  //pinMode(A3, OUTPUT);
    
  Serial.begin(9600);
  radio.begin();
  
  radio.setPALevel(RF24_PA_MIN);
  radio.setDataRate(RF24_250KBPS);
  radio.setAutoAck(false);
  radio.startListening();
  
  radio.openReadingPipe(0, address);

  //d3 throttle forward
}

unsigned long lastRecvTime = 0;
void recvData(){
  while (radio.available()){
    radio.read(&pkg, sizeof(pkg));
    lastRecvTime = millis();
  }
}

void loop() {
  recvData();
  unsigned long now = millis();
  /*
  if (now - lastRecvTime > 1000) {
    //Signal lost
    resetData();
  }
  */
  Serial.print("Receiver throttle   "); Serial.print(pkg.throttle);
  Serial.print(" push1     "); Serial.print(pkg.push1);      
  Serial.print(" xAxis     "); Serial.print(pkg.xAxis);     
  Serial.print(" yAxis     "); Serial.print(pkg.yAxis);     
  Serial.print(" push2     "); Serial.print(pkg.push2);
  Serial.println();
  analogWrite(3, pkg.throttle);
  //delay(50);
}
