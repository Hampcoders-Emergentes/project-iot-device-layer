#pragma once
#include <Arduino.h>

class LedIndicators {
private:
  int greenPin;
  int yellowPin;
  int redPin;

public:
  LedIndicators(int green, int yellow, int red)
    : greenPin(green), yellowPin(yellow), redPin(red) {}

  void begin() {
    pinMode(greenPin, OUTPUT);
    pinMode(yellowPin, OUTPUT);
    pinMode(redPin, OUTPUT);
    offAll();
  }

  void offAll() {
    digitalWrite(greenPin, LOW);
    digitalWrite(yellowPin, LOW);
    digitalWrite(redPin, LOW);
  }

  void normal() {
    digitalWrite(greenPin, HIGH);
    digitalWrite(yellowPin, LOW);
    digitalWrite(redPin, LOW);
  }

  void alert() {
    digitalWrite(greenPin, LOW);
    digitalWrite(yellowPin, HIGH);
    digitalWrite(redPin, LOW);
  }

  void danger() {
    digitalWrite(greenPin, LOW);
    digitalWrite(yellowPin, LOW);
    digitalWrite(redPin, HIGH);
  }
};
