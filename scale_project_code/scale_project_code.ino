#include<Statistic.h>

#include <Linear2DRegression.hpp>
// INFO FOR REGRESSION TOOL
// https://github.com/nkaaf/Arduino-Regression?tab=readme-ov-file
// EX: https://nkaaf.github.io/Arduino-Regression/html/class_linear2_d_regression.html

#include <LiquidCrystal.h>
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

const int buttonPin = 8;
const int calibration_mode_indicator = 7;
const int calibration_ready_indicator = 6;
const int sensorPin = A0;

const int cal_weights[] = {0, 50, 500, 1500};

const int sensor_val_display_line = 0;
const bool calibration_mode_state = LOW;

Statistic stats;
Linear2DRegression reg;

void setup() {

  Serial.begin(9600);
  pinMode(buttonPin, INPUT_PULLUP);
  digitalWrite(buttonPin, LOW);
  pinMode(calibration_mode_indicator, OUTPUT);
  digitalWrite(calibration_mode_indicator, LOW);
  lcd.begin(16, 2);

  pinMode(LED_BUILTIN, OUTPUT);
  Serial.print("The next number should be 5\n");
  Serial.print(int(stats.range()));

  // Baked in regression
  reg.addPoint(27, 0); // (sensor_reading, known weight (in grams))
  reg.addPoint(60, 50);
  reg.addPoint(600, 1000);
}

void loop() {

  int sensor_reading = readSensorValue();
  double predicted_weight = reg.calculate(sensor_reading);

  lcd.setCursor(0, 0);
  displaySensorValue(sensor_reading, predicted_weight);

  lcd.setCursor(0, 1);
  lcd.print("Measure Mode");

  Serial.println("Predicted weight: " + String(reg.calculate(sensor_reading)));

  if (digitalRead(buttonPin) == HIGH) {

    clearLine(1);
    lcd.print("Calibration Mode");
    togglePin(calibration_mode_indicator);

    reg.reset(); // Clear previous regression

    // Measurement 1
    delay(500);
    clearLine(1);
    lcd.print("0g Weight");

    int stable_reading = getStableReading(sensorPin);
    reg.addPoint(stable_reading, cal_weights[0]);
    Serial.println("0g measurement(saved to regression): " + String(stable_reading));
    while(digitalRead(buttonPin)){
      delay(1);
    }
    while(digitalRead(buttonPin) == LOW){
      displaySensorValue(readSensorValue(), reg.calculate(readSensorValue()));
    }

    // Measurement 2
    delay(50);
    clearLine(1);
    lcd.print("50g Weight");

    stable_reading = getStableReading(sensorPin);
    reg.addPoint(stable_reading, cal_weights[1]);
    Serial.println("50g measurement(saved to regression): " + String(stable_reading));
    
    while(digitalRead(buttonPin)){
      delay(1);
    }
    while(digitalRead(buttonPin) == LOW){
      displaySensorValue(readSensorValue(), reg.calculate(readSensorValue()));
    }

    // Measurement 3
    delay(50);
    clearLine(1);
    lcd.print("1000g Weight");

    stable_reading = getStableReading(sensorPin);
    reg.addPoint(stable_reading, cal_weights[2]);
    Serial.println("1000g measurement(saved to regression): " + String(stable_reading));
    while(digitalRead(buttonPin)){
      delay(1);
    }
    while(digitalRead(buttonPin) == LOW){
      displaySensorValue(readSensorValue(), reg.calculate(readSensorValue()));
    }

    clearLine(1);
    lcd.print("CAL COMPLETE");
    delay(200);
    //debugBlink();
    
    clearLine(1);
    togglePin(calibration_mode_indicator);
  }

}

// void calibrationMode() {
//   while(True) {

//   }
// }
void togglePin(int pin_num) {
  digitalWrite(pin_num, !digitalRead(pin_num));
}

int readSensorValue() {
  return int(analogRead(sensorPin));
}

void clearLine(int line) {
  lcd.setCursor(0, line);
  lcd.print("                ");
  lcd.setCursor(0, line);
}

void debugBlink(){
  digitalWrite(LED_BUILTIN, LOW);
  delay(100);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(100);
  digitalWrite(LED_BUILTIN, LOW);
  delay(100);
  digitalWrite(LED_BUILTIN, HIGH);
}

void displaySensorValue(int sensor_reading, double predicted_weight) {
  lcd.setCursor(0, sensor_val_display_line);
  lcd.print("R:" + String(sensor_reading) + "  P:" + String(predicted_weight));
}

int getStableReading(int sensorPin) {
  for (int i = 0; i < 10; i++) {
    stats.add(readSensorValue());
    togglePin(calibration_ready_indicator);
    delay(10);
    togglePin(calibration_ready_indicator);
  }
  if (stats.range() < 5) {
    togglePin(calibration_ready_indicator);
    return int(stats.average());
  } else {
    togglePin(calibration_ready_indicator);
    stats.clear();
    getStableReading(sensorPin);
  }
}

/*
-----PSEUDOCODE-----

Read resistance from photoresistor module

Modes (Button Operation):
  1. Default (Measure Mode)
  2. Hold Button (Calibration Mode)
    a. Display the expected weight of the calibration mass.
    b. Put the displayed mass onto the plate
    c. Press button after the scale is at equilibrium and move onto the next mass.
  3. Optional: Have it sleep after a bit and the button wakes it?

Calibration Mode:
  Known weights are placed on the pad in a known order and a button is pressed to lock in the measurement on the device.

  Using the known measurements a regression can be used in order to determine where on the curve the light level (resistance) the unknown weight causes.

Measure Mode:
  Using the regressed curve from the previous calibration mode, an unknown weight can be measured.

  Displayed on the LCD panel
*/