#pragma once
#include <Arduino.h>
#include "Config.h"

// =============================================================
// AlertService.h - Evaluacion de estados de alerta
// Clase utilitaria que compara la corriente medida con los limites
// dinamicos globales (currentNormalLimit, currentAlertLimit) para
// determinar el nivel de riesgo del sistema en cada instante.
// =============================================================

enum AlertStatus {
  STATUS_NORMAL,   // Corriente dentro del rango seguro
  STATUS_ALERT,    // Corriente sobre el limite normal, en zona de prevencion
  STATUS_DANGER    // Corriente sobre el limite de peligro -> cortar rele inmediatamente
};

class AlertService {
public:
  // Evalua la corriente contra los umbrales globales (dinamicos o estaticos).
  static AlertStatus evaluate(float current) {
    if (current < currentNormalLimit) return STATUS_NORMAL;
    if (current < currentAlertLimit)  return STATUS_ALERT;
    return STATUS_DANGER;
  }

  static String toText(AlertStatus status) {
    switch (status) {
      case STATUS_NORMAL: return "NORMAL";
      case STATUS_ALERT:  return "ALERTA";
      case STATUS_DANGER: return "PELIGRO";
      default:            return "UNKNOWN";
    }
  }
};
