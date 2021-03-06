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

void setup() {
  Serial.begin(9600);
  radio.begin();

  radio.setPALevel(RF24_PA_MIN);
  radio.setDataRate(RF24_250KBPS);
  radio.setAutoAck(false);
  radio.startListening();

  radio.openReadingPipe(0, address);
}
unsigned long lastRecvTime = 0;

void loop() {
  recvData();
  //setPPMValuesFromData();
  //stopOnLostSignal();
  //debug();
  //delay(50);
}

bool debugTransmissionGap = true;
void recvData(){
  if (radio.available()){
    radio.read(&pkg, sizeof(DataPkg));
    if (debugTransmissionGap) {
      Serial.print(" gap       "); Serial.println(millis() - lastRecvTime);
    }
    lastRecvTime = millis();
  }
  
}

void setPPMValuesFromData() {
  //d3 throttle forward
  analogWrite(3, pkg.throttle);
}

// no reception stop
int lostSignalTimeout = 2000;
void stopOnLostSignal() {
  if (millis() - lastRecvTime > lostSignalTimeout) {
    resetData();
  }
}

//'safe' values to use when no radio input is detected
void resetData(){
  //left joystick
  pkg.throttle = 0;
  pkg.push1 = 0;

  //right joystick
  pkg.xAxis = 127;
  pkg.yAxis = 127;
  pkg.push2 = 0;
}

bool debug = false;
unsigned long lastDebugTime = 0;
int debugDelay = 100;
void timedDebug() {
  if (debug && (millis() - lastDebugTime > debugDelay)) {
    printPkgValues();
    lastDebugTime = millis();
  }
  Serial.println();
}

void printPkgValues() {
  Serial.print("throttle   "); Serial.print(pkg.throttle);
  Serial.print(" push1     "); Serial.print(pkg.push1);      
  Serial.print(" xAxis     "); Serial.print(pkg.xAxis);
  //Serial.print(" lastRcvTime "); Serial.print(lastRecvTime);
//  Serial.print(" yAxis     "); Serial.print(pkg.yAxis);
//  Serial.print(" push2     "); Serial.print(pkg.push2);
  
}
