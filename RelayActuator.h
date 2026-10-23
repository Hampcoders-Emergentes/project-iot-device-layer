#pragma once
#include <Arduino.h>
#include "Actuator.h"
#include "Config.h"

// =============================================================
// RelayActuator.h - Actuador del rele de corte de carga
// Controla el encendido/apagado de la linea electrica supervisada.
// Soporta logica activa alta o activa baja segun el modulo fisico.
// =============================================================

class RelayActuator : public Actuator {
private:
  int pin;
  bool active;

  void writeState() {
    digitalWrite(pin, active == RELAY_ACTIVE_HIGH ? HIGH : LOW);
  }

public:
  explicit RelayActuator(int gpioPin) : pin(gpioPin), active(false) {}

  void begin() override {
    pinMode(pin, OUTPUT);
    turnOff();
  }

  void turnOn() override {
    active = true;
    writeState();
  }

  void turnOff() override {
    active = false;
    writeState();
  }

  // Atajo para compatibilidad con el codigo anterior
  void set(bool value) { value ? turnOn() : turnOff(); }
  void on()  { turnOn(); }
  void off() { turnOff(); }

  bool isOn() const override { return active; }
};
