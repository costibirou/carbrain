#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

//Radio pin numbers
RF24 radio(7, 8); // CE, CSN
const byte address[6] = "00555";


class JoyStick {
  private:
    int xAxis, yAxis, pushed;
    int xPin, yPin, swPin;
  public:
    JoyStick(int xPin, int yPin, int swPin) {
      this -> xAxis = 0;
      this -> yAxis = 0;
      this -> pushed = 0;
      
      this -> xPin = xPin;
      this -> yPin = yPin;
      this -> swPin = swPin;
    }
    int getXAxis() {
      return xAxis;
    }
    int getYAxis() {
      return yAxis;
    }
    int getPushed() {
      return pushed;
    }
    void setup() {
      //Joystick pushed pin setup
      pinMode(swPin, INPUT);
      digitalWrite(swPin, HIGH);
    }
    void sample() {
      this -> xAxis = analogRead(xPin);
      this -> yAxis = analogRead(yPin);
      this -> pushed = 1 - digitalRead(swPin);
    }
};

struct DataPkg {
  //left joystick
  byte throttle;
  byte push1;

  //right joystick
  byte xAxis;
  byte yAxis;
  byte push2;
};

JoyStick leftJoyStick(A1, A0, 2);
//JoyStick rightJoyStick(A3, A2, 3);

DataPkg pkg;


void setup() {
  // Open serial
  Serial.begin(9600);
  
  leftJoyStick.setup();

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

void loop() {
  leftJoyStick.sample();
  calculatePackage();
  transmit();
  debug();
}

void calculatePackage() {
    //The calibration numbers used here should be measured for
    //your joysticks until they send the correct value
    pkg.throttle   = mapCarThrottle(leftJoyStick.getYAxis(), 512, true);
    pkg.push1      = leftJoyStick.getPushed();
    
    pkg.xAxis      = mapAxis(leftJoyStick.getXAxis(),  4, 4, 1023, true);
    //pkg.yAxis      = mapAxis(rightJoyStick.getYAxis(),  3, 4, 1023, true);
    //pkg.push2      = rightJoyStick.getSwPin();
}

void transmit() {
    radio.write(&pkg, sizeof(DataPkg));
}

void resetData() {
  pkg.throttle = 0;
  pkg.push1 = 0;
  pkg.xAxis = 127;
  pkg.yAxis = 127;
  pkg.push2 = 0;
}

int mapAxis(int val, int lower, int middle, int upper, bool reverse) {
  val = constrain(val, lower, upper);
  if (val < middle)
    val = map(val, lower, middle, 1, 255);
  else
    val = map(val, middle, upper, 1, 255);

  return (reverse ? 255 - val : val);
}
/*
 * Maps one half of joystick input to PWM throttle
 * Input     middle, 1023
 * middle is the middle of the joystick after calibration
 * Output    0  , 254
 * Reverse - tells which half of the joystick is used for throttle
 */
int mapCarThrottle(int val, int middle, bool reverse) {
  val = (reverse ? 1023 - val : val);
  val = constrain(val, middle, 1023);
  val = map(val, middle, 1023, 0, 254);
  return val;
}

unsigned long lastDebugTime = 0;
int debugDelay = 120;
void debug() {
  if (millis() - lastDebugTime > debugDelay) {
    //printJoystickValues();
    printPkgValues();
    lastDebugTime = millis();
  }
}

void printJoystickValues() {
  Serial.print("xAxis        "); Serial.print(leftJoyStick.getXAxis());
  Serial.print(" yAxis       "); Serial.print(leftJoyStick.getYAxis());
  Serial.print(" pushed      "); Serial.print(leftJoyStick.getPushed());

  //Serial.print(" A2       "); Serial.print(analogRead(A2));
  //Serial.print(" A3       "); Serial.print(analogRead(A3));
  //Serial.print(" D3       "); Serial.println(digitalRead(3));
  Serial.println();
}

void printPkgValues() {
  Serial.print("throttle  "); Serial.print(pkg.throttle);
  Serial.print(" push1    "); Serial.print(pkg.push1);
//  Serial.print(" xAxis    "); Serial.print(pkg.xAxis);
//  Serial.print(" yAxis    "); Serial.print(pkg.yAxis);
//  Serial.print(" push2    "); Serial.print(pkg.push2);
  Serial.println();
}
