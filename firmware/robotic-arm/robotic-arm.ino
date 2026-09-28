#include <ESP32Servo.h>

class Joint {
private:
  int potPin;
  int servoPin;
  int adcMin;
  int adcMax;
  int angleMin;
  int angleMax;
  float alpha;
  int deadband;
  Servo servo;
  float filteredValue;
  int lastAngle;

  int readADC() {
    long sum = 0;
    const int samples = 10;
    for (int i = 0; i < samples; i++) {
      sum += analogRead(potPin);
      delayMicroseconds(150);
    }
    return sum / samples;
  }

  int adcToAngle(int adcValue) {
    adcValue = constrain(adcValue, adcMin, adcMax);
    float normalized =
      (float)(adcValue - adcMin) / (float)(adcMax - adcMin);
    int angle = angleMin + normalized * (angleMax - angleMin);
    return constrain(angle, angleMin, angleMax);
  }

public:
  Joint(
    int pot,
    int servoP,
    int adcMinValue = 0,
    int adcMaxValue = 4095,
    int angleMinValue = 0,
    int angleMaxValue = 180,
    float alphaValue = 0.15,
    int deadbandValue = 2
  ) {
    potPin = pot;
    servoPin = servoP;
    adcMin = adcMinValue;
    adcMax = adcMaxValue;
    angleMin = angleMinValue;
    angleMax = angleMaxValue;
    alpha = alphaValue;
    deadband = deadbandValue;
    filteredValue = 0;
    lastAngle = 90;
  }

  void begin() {
    pinMode(potPin, INPUT);
    analogSetPinAttenuation(potPin, ADC_11db);
    servo.setPeriodHertz(50);
    servo.attach(servoPin, 500, 2400);
    int initialValue = readADC();
    filteredValue = initialValue;
    lastAngle = adcToAngle(initialValue);
    servo.write(lastAngle);
  }

  void update() {
    int rawValue = readADC();
    filteredValue = filteredValue + alpha * (rawValue - filteredValue);
    int angle = adcToAngle((int)filteredValue);
    if (abs(angle - lastAngle) >= deadband) {
      servo.write(angle);
      lastAngle = angle;
    }
  }

  int getAngle() {
    return lastAngle;
  }

  int getFilteredValue() {
    return (int)filteredValue;
  }

  void setCalibration(int minADC, int maxADC) {
    adcMin = minADC;
    adcMax = maxADC;
  }

  void setAngleLimits(int minAngle, int maxAngle) {
    angleMin = minAngle;
    angleMax = maxAngle;
  }
};

//potentiometer
#define POT_SHOULDER 32
#define POT_ELBOW    33
#define POT_HAND     34
#define POT_CLAW     35

//servos
#define SERVO_SHOULDER 18
#define SERVO_ELBOW    19
#define SERVO_HAND     21
#define SERVO_CLAW     22

Joint shoulder(POT_SHOULDER, SERVO_SHOULDER);
Joint elbow(POT_ELBOW, SERVO_ELBOW);
Joint hand(POT_HAND, SERVO_HAND);
Joint claw(POT_CLAW, SERVO_CLAW);

void setup() {
  Serial.begin(115200);
  analogReadResolution(12);
  shoulder.begin();
  elbow.begin();
  hand.begin();
  claw.begin();
}

void loop() {
  shoulder.update();
  elbow.update();
  hand.update();
  claw.update();

  Serial.print("Shoulder: ");
  Serial.print(shoulder.getAngle());
  Serial.print(" | Elbow: ");
  Serial.print(elbow.getAngle());
  Serial.print(" | Hand: ");
  Serial.print(hand.getAngle());
  Serial.print(" | Claw: ");
  Serial.println(claw.getAngle());
  delay(20);
}
