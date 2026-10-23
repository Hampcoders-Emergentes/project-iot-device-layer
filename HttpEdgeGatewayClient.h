#pragma once
#include <Arduino.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include "WiFiConnectivityDriver.h"

// =============================================================
// HttpEdgeGatewayClient.h - Cliente HTTP del Edge Gateway
// Reemplaza a EdgeApiClient. Serializa las lecturas de AMBOS
// sensores en un JSON, realiza el POST al Edge Gateway y parsea
// la respuesta para actualizar dinamicamente los umbrales de
// alerta definidos globalmente en Config.h.
//
// Soporta HTTP y HTTPS automaticamente.
// Para HTTPS usa WiFiClientSecure como miembro persistente
// para evitar errores de scope/stack.
// =============================================================

class HttpEdgeGatewayClient {
private:
  WiFiConnectivityDriver& wifi;
  const char* endpoint;
  WiFiClientSecure client;
  bool clientReady = false;

public:
  HttpEdgeGatewayClient(WiFiConnectivityDriver& wifiDriver, const char* apiUrl)
    : wifi(wifiDriver), endpoint(apiUrl) {}

  // Envia la lectura dual (ACS712 + INA219) al Edge Gateway.
  // Si la respuesta incluye "thresholds", actualiza los limites globales en caliente.
  bool sendReading(
    const String& deviceId,
    float currentAcs,   // Corriente del ACS712 en Amperios
    float currentIna,   // Corriente del INA219 en Amperios
    float voltage,      // Voltaje del bus (del INA219 o constante)
    float power,        // Potencia calculada en Watts
    bool relayOn,
    const String& status
  ) {
    if (!wifi.ensureConnected()) return false;

    // Construir el payload JSON con lecturas duales
    JsonDocument doc;
    doc["deviceId"]   = deviceId;
    doc["currentAcs"] = serialized(String(currentAcs, 3));
    doc["currentIna"] = serialized(String(currentIna, 3));
    doc["voltage"]    = serialized(String(voltage, 2));
    doc["power"]      = serialized(String(power, 3));
    doc["relay"]      = relayOn;
    doc["status"]     = status;
    doc["timestamp"]  = millis();

    String payload;
    serializeJson(doc, payload);

    HTTPClient http;
    bool useHttps = strncmp(endpoint, "https://", 8) == 0;
    if (useHttps) {
      if (!clientReady) {
        client.setInsecure();
        clientReady = true;
      }
      http.begin(client, endpoint);
    } else {
      http.begin(endpoint);
    }
    http.addHeader("Content-Type", "application/json");
    int code = http.POST(payload);

    Serial.print("[Edge] POST -> HTTP ");
    Serial.println(code);

    // Parsear la respuesta para actualizar umbrales dinamicamente
    if (code >= 200 && code < 300) {
      String responseBody = http.getString();
      _parseThresholds(responseBody);
    }

    http.end();
    return code >= 200 && code < 300;
  }

private:
  // Intenta extraer y aplicar los umbrales de la respuesta del Edge.
  // Espera el formato: { "thresholds": { "normalLimit": 0.2, "alertLimit": 0.6, ... } }
  void _parseThresholds(const String& responseBody) {
    JsonDocument res;
    DeserializationError err = deserializeJson(res, responseBody);
    if (err) return;

    if (res.containsKey("thresholds")) {
      JsonObject t = res["thresholds"];

      float newNormal = t["normalLimit"] | -1.0;
      float newAlert  = t["alertLimit"]  | -1.0;

      if (newNormal > 0 && newAlert > 0 && newAlert > newNormal) {
        currentNormalLimit = newNormal;
        currentAlertLimit  = newAlert;

        // Read optional fields
        float vnom  = t["nominalVoltage"]        | currentNominalVoltage;
        float mw    = t["maxConsumptionWatts"]   | -1.0;
        float mc    = t["maxCurrentAmps"]        | -1.0;
        float pf    = t["minPowerFactor"]        | -1.0;
        float freq  = t["nominalFrequency"]      | -1.0;
        int   dtm   = t["disconnectionThresholdMin"] | -1;

        currentNominalVoltage = vnom;

        Serial.println("[Edge] Thresholds recibidos (8 campos):");
        Serial.print("  normalLimit: ");               Serial.println(newNormal, 2);
        Serial.print("  alertLimit: ");                Serial.println(newAlert, 2);
        Serial.print("  nominalVoltage: ");            Serial.println(vnom, 0);
        Serial.print("  maxConsumptionWatts: ");       Serial.println(mw, 0);
        Serial.print("  maxCurrentAmps: ");            Serial.println(mc, 2);
        Serial.print("  minPowerFactor: ");            Serial.println(pf, 2);
        Serial.print("  nominalFrequency: ");          Serial.println(freq, 0);
        Serial.print("  disconnectionThresholdMin: "); Serial.println(dtm);
      }
    }
  }
};
