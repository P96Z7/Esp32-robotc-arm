# Telemetric Robotic Arm Firmware

This project is the firmware for a robotic arm controlled by an ESP32.

The current version uses:

- 1 ESP32
- 4 potentiometers
- 4 servo motors
- Object-Oriented Programming in C++
- ADC filtering
- Servo deadband
- Calibration support
- Angle limits

The basic idea is:

```text
Potentiometer
     |
     v
ESP32 ADC
     |
     v
Digital Filter
     |
     v
Angle Conversion
     |
     v
Deadband
     |
     v
Servo Motor
```

Each joint has one potentiometer and one servo motor.

The current joints are:

- Shoulder
- Elbow
- Hand
- Claw

---

## Pin Mapping

### Potentiometers

| Joint | ESP32 ADC Pin |
|---|---:|
| Shoulder | GPIO 32 |
| Elbow | GPIO 33 |
| Hand | GPIO 34 |
| Claw | GPIO 35 |

### Servo Motors

| Joint | ESP32 PWM Pin |
|---|---:|
| Shoulder | GPIO 18 |
| Elbow | GPIO 19 |
| Hand | GPIO 21 |
| Claw | GPIO 22 |

---

# Software Architecture

The firmware uses a C++ class called `Joint`.

A `Joint` represents one robotic joint.

Each object knows:

- which potentiometer pin it must read
- which servo pin it must control
- the ADC calibration values
- the minimum and maximum angles
- the digital filter configuration
- the deadband value
- the current filtered ADC value
- the last servo angle

This makes the code easier to understand and easier to scale.

Example:

```cpp
Joint shoulder(POT_SHOULDER, SERVO_SHOULDER);
Joint elbow(POT_ELBOW, SERVO_ELBOW);
Joint hand(POT_HAND, SERVO_HAND);
Joint claw(POT_CLAW, SERVO_CLAW);
```

All joints use the same logic, but each object uses different GPIO pins.

---

# The `Joint` Class

The class starts like this:

```cpp
class Joint {
```

It is a model for every joint of the robotic arm.

---

## Private Variables

The private section contains internal information.

```cpp
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
```

These variables should not be changed directly from outside the class.

This is called **encapsulation**.

The class controls its own internal data.

---

## Potentiometer and Servo Pins

```cpp
int potPin;
int servoPin;
```

These variables store the GPIO pins.

For example:

```text
potPin = 32
servoPin = 18
```

For the shoulder, this means:

```text
Read potentiometer -> GPIO 32
Control servo      -> GPIO 18
```

---

# ADC Calibration

```cpp
int adcMin;
int adcMax;
```

The ESP32 ADC uses a 12-bit value.

The theoretical range is:

```text
0 to 4095
```

But a real potentiometer may not use the full range.

For example:

```text
Minimum ADC = 280
Maximum ADC = 3800
```

The firmware can use these real values:

```cpp
shoulder.setCalibration(280, 3800);
```

Now:

```text
280  = minimum position
3800 = maximum position
```

This is more accurate than always using `0` and `4095`.

---

# Angle Limits

```cpp
int angleMin;
int angleMax;
```

These values define the safe servo range.

A servo can support a large angle range, but the mechanical arm may not support the same range.

Example:

```cpp
shoulder.setAngleLimits(20, 150);
```

Now the servo will only move between:

```text
20 degrees
and
150 degrees
```

This helps protect the mechanical structure.

---

# ADC Reading

The function below reads the potentiometer:

```cpp
int readADC() {
  long sum = 0;

  const int samples = 10;

  for (int i = 0; i < samples; i++) {
    sum += analogRead(potPin);
    delayMicroseconds(150);
  }

  return sum / samples;
}
```

Instead of reading the ADC only one time, the ESP32 reads it 10 times.

Example:

```text
2001
1998
2005
2002
1999
2010
2003
1997
2004
2001
```

The values are added and divided by 10.

This creates an average.

The average is more stable than one single ADC reading.

---

# Why Use Multiple ADC Samples?

The ESP32 ADC can have small electrical noise.

Without filtering:

```text
2000
2025
1980
2010
1995
```

If every value directly controls the servo, the servo can shake.

With average filtering, the value becomes more stable.

The first filtering step is:

```text
ADC
 |
 v
10 samples
 |
 v
Average
```

---

# Digital Low-Pass Filter

After the average, the firmware uses another filter.

```cpp
filteredValue =
  filteredValue +
  alpha * (rawValue - filteredValue);
```

This is an EMA filter:

**Exponential Moving Average**

The current project uses:

```cpp
alpha = 0.15;
```

A simple way to understand this is:

```text
15% new value
85% previous filtered value
```

Example:

```text
Previous filtered value = 2000
New ADC value            = 2100
Alpha                    = 0.15
```

The new filtered value is:

```text
2000 + 0.15 * (2100 - 2000)
```

Result:

```text
2015
```

The value does not jump directly from:

```text
2000 -> 2100
```

It moves more slowly:

```text
2000
2015
2028
2039
...
```

This reduces servo jitter.

---

# What Does `alpha` Change?

A larger alpha gives a faster response.

Example:

```text
alpha = 0.8
```

This reacts quickly but filters less noise.

A smaller alpha gives more filtering.

Example:

```text
alpha = 0.05
```

This gives a smoother signal but also adds more delay.

The current value:

```text
alpha = 0.15
```

is a starting point and can be changed during tests.

---

# Convert ADC to Angle

The function:

```cpp
int adcToAngle(int adcValue)
```

converts the ADC value into a servo angle.

First, the firmware limits the ADC value:

```cpp
adcValue = constrain(adcValue, adcMin, adcMax);
```

If the calibrated range is:

```text
300 to 3800
```

then a value below 300 becomes 300, and a value above 3800 becomes 3800.

---

## Normalize the ADC Value

The code uses:

```cpp
float normalized =
  (float)(adcValue - adcMin) /
  (float)(adcMax - adcMin);
```

This converts the ADC value to a value between:

```text
0.0 and 1.0
```

Example:

```text
ADC minimum = 0
ADC maximum = 4095
ADC current = 2048
```

The normalized value is close to:

```text
0.5
```

This means the potentiometer is around 50% of its range.

---

## Convert the Normalized Value to a Servo Angle

The code uses:

```cpp
int angle =
  angleMin +
  normalized * (angleMax - angleMin);
```

Example:

```text
angleMin = 0
angleMax = 180
normalized = 0.5
```

Result:

```text
90 degrees
```

So:

```text
0%   -> 0 degrees
50%  -> 90 degrees
100% -> 180 degrees
```

---

# Deadband

Even after filtering, the ADC may change by a very small amount.

For example:

```text
90 degrees
91 degrees
90 degrees
91 degrees
```

The servo does not need to react to every small change.

The firmware uses:

```cpp
if (abs(angle - lastAngle) >= deadband)
```

The current deadband is:

```text
2 degrees
```

Example:

```text
Last angle = 90
New angle  = 91
Difference = 1
```

The servo does not move.

But:

```text
Last angle = 90
New angle  = 93
Difference = 3
```

Now the servo moves.

This helps reduce small and unnecessary movements.

---

# Constructor

The constructor creates a new `Joint` object.

```cpp
Joint(
  int pot,
  int servoP,
  int adcMinValue = 0,
  int adcMaxValue = 4095,
  int angleMinValue = 0,
  int angleMaxValue = 180,
  float alphaValue = 0.15,
  int deadbandValue = 2
)
```

Only two values are required:

```text
potentiometer pin
servo pin
```

For example:

```cpp
Joint shoulder(32, 18);
```

---

# `begin()`

The `begin()` function prepares one joint.

```cpp
void begin()
```

It configures:

- ADC input pin
- ADC attenuation
- servo PWM frequency
- servo pulse limits
- initial ADC value
- initial servo angle

Important lines:

```cpp
pinMode(potPin, INPUT);
```

The potentiometer GPIO becomes an input.

```cpp
analogSetPinAttenuation(potPin, ADC_11db);
```

This configures the ESP32 ADC range for higher input voltage.

```cpp
servo.setPeriodHertz(50);
```

The servo uses a 50 Hz control signal.

```cpp
servo.attach(servoPin, 500, 2400);
```

This connects the servo object to the GPIO.

The pulse range is:

```text
500 us to 2400 us
```

These values can be calibrated for the real servo.

---

# Initial Position

The firmware reads the potentiometer when the system starts:

```cpp
int initialValue = readADC();
```

Then:

```cpp
filteredValue = initialValue;
```

This is important because the filter starts from the real potentiometer position.

After that:

```cpp
lastAngle = adcToAngle(initialValue);
servo.write(lastAngle);
```

The servo moves to the position connected to the potentiometer.

---

# `update()`

The main control function is:

```cpp
void update()
```

Its flow is:

```text
Read ADC
   |
   v
Average 10 samples
   |
   v
EMA filter
   |
   v
Convert ADC to angle
   |
   v
Compare with previous angle
   |
   v
Deadband check
   |
   v
Move servo if needed
```

This function is called for every joint.

---

# Getters

The class has functions to read internal information.

Example:

```cpp
int getAngle()
```

returns the last servo angle.

Usage:

```cpp
shoulder.getAngle();
```

Another getter is:

```cpp
int getFilteredValue()
```

This returns the filtered ADC value.

These functions are useful for the Serial Monitor and debugging.

---

# Calibration Functions

The class has:

```cpp
void setCalibration(int minADC, int maxADC)
```

Example:

```cpp
shoulder.setCalibration(250, 3820);
```

It also has:

```cpp
void setAngleLimits(int minAngle, int maxAngle)
```

Example:

```cpp
shoulder.setAngleLimits(20, 160);
```

---

# Setup

The Arduino `setup()` runs one time when the ESP32 starts.

```cpp
void setup() {
  Serial.begin(115200);

  analogReadResolution(12);

  shoulder.begin();
  elbow.begin();
  hand.begin();
  claw.begin();

  Serial.println("Arm initialized.");
}
```

The ADC resolution is:

```text
12 bits
```

This gives:

```text
0 to 4095
```

Then all four joints are initialized.

---

# Main Loop

The Arduino `loop()` runs continuously.

```cpp
void loop() {
  shoulder.update();
  elbow.update();
  hand.update();
  claw.update();

  delay(20);
}
```

Every cycle:

```text
Update shoulder
Update elbow
Update hand
Update claw
```

Then the process starts again.

---

# Full Firmware Flow

```text
ESP32 starts
     |
     v
setup()
     |
     +--> Serial
     |
     +--> ADC configuration
     |
     +--> Shoulder begin()
     |
     +--> Elbow begin()
     |
     +--> Hand begin()
     |
     +--> Claw begin()
     |
     v
loop()
     |
     +--> Shoulder update()
     |
     +--> Elbow update()
     |
     +--> Hand update()
     |
     +--> Claw update()
     |
     +--> Serial Monitor
     |
     +--> delay
     |
     +------------------+
                        |
                        v
                     repeat
```

---

# Current Control Pipeline

The current firmware uses different protection layers:

```text
Potentiometer
      |
      v
Hardware RC Filter
      |
      v
ESP32 ADC
      |
      v
10-sample Average
      |
      v
EMA Digital Filter
      |
      v
ADC -> Angle Conversion
      |
      v
Deadband
      |
      v
Servo Motor
```

The hardware and software filters work together to reduce jitter.

---

# Important Hardware Note

The ESP32 must not power all servo motors directly.

The servos should use an external 5 V power supply.

All grounds must be connected together:

```text
ESP32 GND
Servo GND
Power Supply GND
Potentiometer GND
```

The servo power and ESP32 logic voltage are different:

```text
Potentiometers -> 3.3 V
Servos         -> 5 V
GND            -> common
```

---

# Future Improvements

Possible next versions:

- non-blocking timing with `millis()`
- remove `delay()`
- automatic potentiometer calibration
- per-joint filter configuration
- Wi-Fi control
- ESP-NOW telemetry
- web control interface
- second ESP32 for a master-slave system
- movement recording
- inverse kinematics
- emergency stop
- current monitoring
- position feedback
- telemetry dashboard

A future architecture can be:

```text
RoboticArm
   |
   +-- Joint: Shoulder
   +-- Joint: Elbow
   +-- Joint: Hand
   +-- Joint: Claw
   |
   +-- Motion Controller
   +-- Telemetry
   +-- Safety
```

---

# Main Goal

The goal of this firmware is to create a simple and scalable control system for a telemetric robotic arm.

The project is also used to study:

- ESP32
- C++
- Object-Oriented Programming
- ADC
- PWM
- servo motors
- potentiometers
- digital filters
- embedded software architecture
- hardware and software integration
