#pragma once

// =============================================================
// Sensor.h - Interfaz base abstracta para sensores de corriente
// Toda clase que represente un sensor de corriente debe heredar
// de esta clase e implementar sus metodos virtuales puros.
// =============================================================

class Sensor {
public:
  // Inicializa el hardware del sensor. Retorna true si tuvo exito.
  virtual bool begin() = 0;

  // Lee y retorna la corriente medida en Amperios (A).
  virtual float readCurrent() = 0;

  // Nombre descriptivo del sensor para logs de depuracion.
  virtual const char* getName() const = 0;

  virtual ~Sensor() {}
};
