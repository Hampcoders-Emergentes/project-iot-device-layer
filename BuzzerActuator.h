#pragma once
#include <Arduino.h>
#include "Actuator.h"

// =============================================================
// BuzzerActuator.h - Actuador del zumbador acustico de alarma
// Se activa cuando el sistema detecta una condicion de PELIGRO
// (corriente sobre el limite maximo). Es una alerta sonora local
// que funciona incluso sin conexion a red.
// =============================================================

class BuzzerActuator : public Actuator {
private:
  int pin;
  bool state;

public:
  explicit BuzzerActuator(int gpioPin) : pin(gpioPin), state(false) {}

  void begin() override {
    pinMode(pin, OUTPUT);
    turnOff();
  }

  void turnOn() override {
    digitalWrite(pin, HIGH);
    state = true;
  }

  void turnOff() override {
    digitalWrite(pin, LOW);
    state = false;
  }

  // Atajo para compatibilidad con el codigo anterior
  void on()  { turnOn(); }
  void off() { turnOff(); }

  bool isOn() const override { return state; }
};
