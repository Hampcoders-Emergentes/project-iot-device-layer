#pragma once
#include <Arduino.h>
#include "Sensor.h"
#include "Config.h"

// =============================================================
// Ina219Sensor.h - Sensor de corriente y voltaje digital (I2C)
//
// Modos de operacion (controlado desde Config.h):
//
//   WOKWI_SIMULATION = true:
//     No usa I2C. Lee la corriente desde un segundo potenciometro
//     conectado al pin PIN_INA_SIM (pin 35). Permite simular el
//     comportamiento del INA219 en Wokwi sin el componente fisico.
//
//   WOKWI_SIMULATION = false:
//     Usa la libreria Adafruit_INA219 con comunicacion I2C real
//     en la direccion 0x40 (SDA=21, SCL=22). Es el modo de
//     produccion para el hardware fisico ESP32.
// =============================================================

#if !WOKWI_SIMULATION
#include <Wire.h>
#include <Adafruit_INA219.h>
#endif

class Ina219Sensor : public Sensor {
private:
  bool initialized;

#if !WOKWI_SIMULATION
  Adafruit_INA219 ina219;
#endif

  // Modo simulacion: convierte lectura ADC del potenciometro en corriente
  float _readSimulatedCurrent() {
    int raw = analogRead(PIN_INA_SIM);
    // Mapea 0-4095 (ADC 12-bit) a 0.0-2.0 A como rango de simulacion
    return (raw / 4095.0) * 2.0;
  }

public:
  Ina219Sensor() : initialized(false)
#if !WOKWI_SIMULATION
    , ina219(0x40)
#endif
  {}

  bool begin() override {
#if WOKWI_SIMULATION
    // En modo Wokwi inicializamos el pin ADC del potenciometro
    pinMode(PIN_INA_SIM, INPUT);
    initialized = true;
    Serial.println("[INA219] Modo SIMULACION activo (potenciometro en pin 35).");
    return true;
#else
    if (!ina219.begin()) {
      Serial.println("[INA219] ERROR: Sensor no encontrado en bus I2C (0x40).");
      initialized = false;
      return false;
    }
    initialized = true;
    Serial.println("[INA219] Sensor fisico inicializado correctamente.");
    return true;
#endif
  }

  // Retorna corriente en Amperios (simulada o real segun el modo).
  float readCurrent() override {
    if (!initialized) return 0.0;
#if WOKWI_SIMULATION
    return _readSimulatedCurrent();
#else
    return abs(ina219.getCurrent_mA()) / 1000.0;
#endif
  }

  // Voltaje del bus: en simulacion retorna el voltaje de supply constante.
  float readBusVoltage() {
    if (!initialized) return SUPPLY_VOLTAGE;
#if WOKWI_SIMULATION
    return SUPPLY_VOLTAGE;
#else
    return ina219.getBusVoltage_V();
#endif
  }

  // Potencia en Watts.
  float readPower() {
    if (!initialized) return 0.0;
#if WOKWI_SIMULATION
    return readBusVoltage() * readCurrent();
#else
    return ina219.getPower_mW() / 1000.0;
#endif
  }

  bool isInitialized() const { return initialized; }

  const char* getName() const override {
#if WOKWI_SIMULATION
    return "INA219 (Simulado - Potenciometro)";
#else
    return "INA219 (Digital I2C - Hardware)";
#endif
  }
};
