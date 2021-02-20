#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

//Radio pin numbers
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

void resetData() {
  pkg.throttle = 0;
  pkg.push1 = 0;
  pkg.xAxis = 127;
  pkg.yAxis = 127;
  pkg.push2 = 0;
}

void setup() {
  // Open serial
  Serial.begin(9600);

  //Joystick setup
  pinMode(2, INPUT);
  digitalWrite(2, HIGH);
  pinMode(3, INPUT);
  digitalWrite(3, HIGH);

  //Radio setup
  pinMode(7, OUTPUT);
  pinMode(8, OUTPUT);
  radio.begin();
  radio.setAutoAck(false);
  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_MIN);
  radio.setDataRate(RF24_250KBPS);
  radio.stopListening();
  resetData();
}

int mapJoystickValues(int val, int lower, int middle, int upper, bool reverse) {
  val = constrain(val, lower, upper);
  if (val < middle)
    val = map(val, lower, middle, 1, 255);
  else
    val = map(val, middle, upper, 1, 255);

  return (reverse ? 255 - val : val);
}

int mapThrottle(int val, int middle, bool reverse) {
  val = (reverse ? 1023 - val : val);
  val = constrain(val, middle, 1023);
  val - map(val, middle, 1023, 0, 254);
  return val;
}

void loop() {
  //const char text[] = "CM: Hi guys " + analogRead(X_pin);
  //radio.write(&text, sizeof(text));

//  printJoystickValues();

  //The calibration numbers used here should be measured for
  //your joysticks until they send the correct value
  pkg.throttle   = mapThrottle(analogRead(A0), 512, true);
  pkg.push1      = 1 - digitalRead(2);
  pkg.xAxis      = mapJoystickValues(analogRead(A3),  4, 4, 1023, true);
  pkg.yAxis      = mapJoystickValues(analogRead(A2),  3, 4, 1023, true);
  pkg.push2      = 1 - digitalRead(3);

  //delay(50);
  printJoystickValues();
  //printPkgValues();
  radio.write(&pkg, sizeof(DataPkg));
}

void printJoystickValues() {
  Serial.print("A0        "); Serial.print(analogRead(A0));
  Serial.print(" A1       "); Serial.print(analogRead(A1));
  Serial.print(" D2       "); Serial.print(digitalRead(2));

  Serial.print(" A2       "); Serial.print(analogRead(A2));
  Serial.print(" A3       "); Serial.print(analogRead(A3));
  Serial.print(" D3       "); Serial.println(digitalRead(3));
}

void printPkgValues() {
  Serial.print(" Sender throttle  "); Serial.print(pkg.throttle);
  Serial.print(" push1    "); Serial.print(pkg.push1);
  Serial.print(" xAxis    "); Serial.print(pkg.xAxis);
  Serial.print(" yAxis    "); Serial.print(pkg.yAxis);
  Serial.print(" push2    "); Serial.print(pkg.push2);
  Serial.println();
}

int isJoystickResting(int x, int y) {
  return isAxisResting(x) && isAxisResting(y);
}
int isAxisResting(int a) {
  return (a > 495) && (a < 529);
}
