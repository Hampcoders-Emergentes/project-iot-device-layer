#pragma once
#include <Arduino.h>
#include <WiFi.h>

// =============================================================
// WiFiConnectivityDriver.h - Manejo de conexion WiFi
// Encapsula toda la logica de conexion, reconexion y estado de
// la red. El archivo principal .ino no toca directamente la
// libreria WiFi.h, delegandolo completamente a esta clase.
// =============================================================

class WiFiConnectivityDriver {
private:
  const char* ssid;
  const char* password;
  static const unsigned long CONNECT_TIMEOUT_MS = 10000;
  static const unsigned long RECONNECT_INTERVAL_MS = 30000;
  unsigned long lastReconnectAttempt;

public:
  WiFiConnectivityDriver(const char* wifiSsid, const char* wifiPassword)
    : ssid(wifiSsid), password(wifiPassword), lastReconnectAttempt(0) {}

  // Intenta conectar a la red. Bloquea hasta conectar o agotar el timeout.
  void begin() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    Serial.print("[WiFi] Conectando a '");
    Serial.print(ssid);
    Serial.print("'");

    unsigned long startedAt = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startedAt < CONNECT_TIMEOUT_MS) {
      delay(500);
      Serial.print(".");
    }
    Serial.println();

    if (isConnected()) {
      Serial.print("[WiFi] Conectado. IP: ");
      Serial.println(WiFi.localIP());
    } else {
      Serial.println("[WiFi] No se pudo conectar. Continuando en modo offline.");
    }
  }

  // Verifica si hay conexion activa. Si no, intenta reconectar periodicamente.
  bool ensureConnected() {
    if (isConnected()) return true;

    unsigned long now = millis();
    if (now - lastReconnectAttempt < RECONNECT_INTERVAL_MS) return false;

    Serial.println("[WiFi] Perdida de conexion. Reconectando...");
    lastReconnectAttempt = now;
    WiFi.reconnect();
    delay(3000);

    if (isConnected()) {
      Serial.println("[WiFi] Reconexion exitosa.");
      return true;
    }
    return false;
  }

  bool isConnected() const {
    return WiFi.status() == WL_CONNECTED;
  }
};
