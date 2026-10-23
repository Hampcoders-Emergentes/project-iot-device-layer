#pragma once
#include <Arduino.h>
#include "Sensor.h"
#include "Config.h"

// =============================================================
// Acs712Sensor.h - Sensor de corriente analogico por efecto Hall
// Lee la corriente a traves del ADC del ESP32. Es el sensor de
// respaldo analogico. Requiere calibracion en vacio antes de usar.
// =============================================================

class Acs712Sensor : public Sensor {
private:
  int pin;
  float zeroVoltage;

  float readVoltageAverage(int samples = 120) {
    long sum = 0;
    for (int i = 0; i < samples; i++) {
      sum += analogRead(pin);
      delay(2);
    }
    float average = sum / (float)samples;
    return average * (3.3 / 4095.0);
  }

public:
  explicit Acs712Sensor(int gpioPin) : pin(gpioPin), zeroVoltage(1.65) {}

  bool begin() override {
    pinMode(pin, INPUT);
    return true;
  }

  // Calibra el punto cero leyendo el voltaje de offset en vacio (sin carga).
  // Llamar una sola vez en setup(), antes de cualquier lectura.
  void calibrate() {
    zeroVoltage = readVoltageAverage(180);
    Serial.print("[ACS712] Zero voltage calibrado: ");
    Serial.println(zeroVoltage, 3);
  }

  float readCurrent() override {
    float voltage = readVoltageAverage(80);
    float current = abs((voltage - zeroVoltage) / ACS712_SENSITIVITY);
    if (current < 0.08) current = 0.0;  // Filtro de ruido de fondo
    return current;
  }

  float getZeroVoltage() const { return zeroVoltage; }

  const char* getName() const override { return "ACS712 (Analogico)"; }
};
