#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 12  //pt temp
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

LiquidCrystal_I2C lcd(0x27, 16, 2);

const int joystickX = A0;
const int joystickY = A1;
const int joystickButton = 7;

int redPin = 10;  //rosu si verde sunt inversati
int greenPin = 9;
int bluePin = 11;
int noTriggerForTheFirstTime=1;
const int debounceTime = 300;

const int shockPin = 2;
const int flamePin = A2;
const int buzzerPin = 8;
const int tiltPin = 3;
const int reedPin = 4;
const int buttonPin = 5;
const int soundPin = A4;

const int passwordLength = 4;
int password[passwordLength] = {0, 0, 3, 3};
int inputSequence[passwordLength];
int currentIndex = 0;


bool isPasswordAllowed = false;
bool shockTriggered = false;
const int tempTreshHold = 100;
const int soundTreshHold = 230; //225 am facut in camera mea
bool isDeactivated = false;

int xValue = 0;
int yValue = 0;
volatile bool buttonPressed = false;

// Meniul
int menuIndex = 0;  // 0 = Activare Alarmă, 1 = Dezactivare Alarmă
int menuSoundIndex = 0;  // 0 = sound mode, 1 = silent mode
bool activeSystem = false;
bool alarmActive = false;

unsigned long lastPressTime = 0;
unsigned long lastAlarmTime = 0;
int previousSoundIndex = -1;

void setup() {
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();
  pinMode(joystickButton, INPUT_PULLUP);
  pinMode(shockPin, INPUT);
  pinMode(flamePin, INPUT);
  pinMode(buzzerPin, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(joystickButton), buttonInterrupt, FALLING); 
  pinMode(tiltPin, INPUT);
  pinMode(reedPin, INPUT);
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  sensors.begin();
  
}

void loop() {
  sensors.requestTemperatures();
  float temp = sensors.getTempCByIndex(0);
  Serial.print("Temperature: ");
  Serial.print(temp);
  Serial.println(" °C");

  // Citirea valorilor joystick-ului
  xValue = analogRead(joystickX);
  yValue = analogRead(joystickY);
  buttonPressed = !digitalRead(joystickButton);
  int tiltState = digitalRead(tiltPin);
  int reedState = digitalRead(reedPin);
  int soundValue = analogRead(soundPin);

  shockTriggered = digitalRead(shockPin) == HIGH;
  int flameSensorValue = analogRead(flamePin);
  // Serial.print("Flame Sensor: ");
  // Serial.println(flameSensorValue);
  Serial.print("Sound Sensor: ");
  Serial.println(soundValue);

  if (digitalRead(buttonPin) == LOW && alarmActive == true) {
    isPasswordAllowed = true;
  } else {
    isPasswordAllowed = false;
  }

  if(alarmActive == false) {
    if (yValue < 300) { // Joystick-ul în sus
      menuIndex = 0;
    } else if (yValue > 700) { // Joystick-ul în jos
      menuIndex = 1;
    } else if (menuIndex == 0) {
        if(xValue < 300)
          menuSoundIndex = 0;
        else if (xValue > 700)
          menuSoundIndex = 1;
      }
  }
  // Afișarea meniului pe LCD
  lcd.setCursor(0, 0);
  if (menuIndex == 0) {
    lcd.print("System activated");
    activeSystem = true;
    isDeactivated = false;
  } else if (menuIndex == 1  && !isDeactivated) {
    // lcd.setCursor(0, 0);
    // lcd.print("                ");
    // lcd.setCursor(0, 0);
    isDeactivated = true;
    lcd.clear();
    lcd.print("Sys deactivated");
    activeSystem = false;
    deactivateAlarm();
  }

  if (activeSystem && !alarmActive) {
    if (shockTriggered) {
      activateAlarm("SHOCK", 255, 0, 0);
    } else 
    if (flameSensorValue > 23 && flameSensorValue <= 70) {
      activateAlarm("LIGHTS", 10, 200, 100);
      Serial.print("Flame Sensor Value: ");
      Serial.println(flameSensorValue);
    } else if (flameSensorValue <= 18) {
      activateAlarm("FLAME", 0, 0, 255);
    } else if (tiltState == LOW) {
      activateAlarm("TILT", 0, 255, 255);
    } else if (reedState == HIGH) {
      activateAlarm("CLOSED DOR", 255, 0, 255);
    } else if (temp > tempTreshHold) {
      activateAlarm("HIGH TEMP", 255, 255, 255);
    } else if (soundValue > soundTreshHold) {
      activateAlarm("HIGH SOUND", 155, 165, 0);
    } else {
      setColor(0, 255, 0);
      if (noTriggerForTheFirstTime || menuSoundIndex != previousSoundIndex) {
        noTriggerForTheFirstTime = 0;
        previousSoundIndex = menuSoundIndex;
        lcd.setCursor(0, 1);
        lcd.print("                ");
        lcd.setCursor(0, 1);
        if (menuSoundIndex == 0) {
          lcd.print("No Trigger,S ON");
        }
        else if (menuSoundIndex)
          lcd.print("No Trigger,S OFF");
      }
    }
  }

  if (buttonPressed) {
    if (millis() - lastPressTime > debounceTime) {
      lastPressTime = millis();
      deactivateAlarm();
      buttonPressed = false;
    }
  }
  
  if (isPasswordAllowed) {
    if (yValue < 300) {
        checkPassword(0);
        delay(300);
    } else if (yValue > 700) {
        checkPassword(1);
        delay(300);
    }

    if (xValue < 300) {
        checkPassword(2);
        delay(300);
    } else if (xValue > 700) {
        checkPassword(3);
        delay(300);
    }
}

  
}

void buttonInterrupt() {
  buttonPressed = true;
}


void setColor(int red, int green, int blue) {
  analogWrite(redPin, red);
  analogWrite(greenPin, green);
  analogWrite(bluePin, blue);
}

void activateAlarm(const char* message, int red, int green, int blue) {
  lcd.setCursor(0, 1);
  lcd.print("                ");
  lcd.setCursor(0, 1);
  lcd.print(message);
  alarmActive = true;
  noTriggerForTheFirstTime = 1;
  setColor(red, green, blue);
  lastAlarmTime = millis();
  if (menuSoundIndex == 0) {
    tone(buzzerPin, 500);
  }
}

void deactivateAlarm() {
  alarmActive = false;
  noTone(buzzerPin);
  lcd.setCursor(0, 1);
  lcd.print("                ");
  lcd.setCursor(0, 1);
  lcd.print("Reset");
  setColor(0, 255, 0);
  noTriggerForTheFirstTime = 1;
  menuSoundIndex = 0;
  currentIndex = 0;
  isPasswordAllowed = false;
}

void checkPassword(int direction) {
  inputSequence[currentIndex] = direction;

  if (currentIndex == passwordLength - 1) {
    if (comparePassword()) {
      deactivateAlarm();
      lcd.clear();
      lcd.print("Password correct!");
      delay(300);
    }
    currentIndex = 0;
  } else {
    currentIndex++;
  }
}

bool comparePassword() {
  for (int i = 0; i < passwordLength; i++) {
    if (inputSequence[i] != password[i]) {
      return false;
    }
  }
  return true;
}

