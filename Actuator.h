#pragma once

// =============================================================
// Actuator.h - Interfaz base abstracta para actuadores de salida
// Toda clase que represente un actuador fisico (rele, buzzer, etc.)
// debe heredar de esta clase e implementar sus metodos virtuales puros.
// =============================================================

class Actuator {
public:
  // Inicializa el pin y estado inicial del actuador.
  virtual void begin() = 0;

  // Activa el actuador.
  virtual void turnOn() = 0;

  // Desactiva el actuador.
  virtual void turnOff() = 0;

  // Retorna el estado actual del actuador.
  virtual bool isOn() const = 0;

  virtual ~Actuator() {}
};
