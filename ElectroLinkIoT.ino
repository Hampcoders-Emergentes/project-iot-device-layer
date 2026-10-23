#include <Arduino.h>
#include <Wire.h>

#include "Pins.h"
#include "Config.h"

// Interfaces base
#include "Sensor.h"
#include "Actuator.h"

// Sensores
#include "Acs712Sensor.h"
#include "Ina219Sensor.h"

// Actuadores
#include "RelayActuator.h"
#include "BuzzerActuator.h"

// Infraestructura
#include "LedIndicators.h"
#include "OledDisplay.h"
#include "AlertService.h"
#include "WiFiConnectivityDriver.h"
#include "HttpEdgeGatewayClient.h"

// =============================================================
// Definicion de variables globales de umbrales dinamicos.
// Son declaradas extern en Config.h para que AlertService y
// HttpEdgeGatewayClient puedan leerlas y escribirlas.
// =============================================================
float currentNormalLimit = DEFAULT_NORMAL_LIMIT;
float currentAlertLimit  = DEFAULT_ALERT_LIMIT;
float currentNominalVoltage = DEFAULT_NOMINAL_VOLTAGE;

// =============================================================
// Instanciacion de objetos
// =============================================================
Acs712Sensor    acs712(PIN_ACS712);
Ina219Sensor    ina219;
RelayActuator   relay(PIN_RELAY);
BuzzerActuator  buzzer(PIN_BUZZER);
LedIndicators   leds(PIN_LED_GREEN, PIN_LED_YELLOW, PIN_LED_RED);
OledDisplay     oled;

WiFiConnectivityDriver  wifi(WIFI_SSID, WIFI_PASSWORD);
HttpEdgeGatewayClient   edgeClient(wifi, EDGE_API_URL);

unsigned long lastEdgeSend = 0;
bool oledAvailable = true;

// Muestra en OLED solo si esta disponible; evita bloqueos si fallo.
void _showBootIfOled(const String& line1, const String& line2) {
  if (oledAvailable) oled.showBoot(line1, line2);
}

// =============================================================
// Aplica la respuesta visual y sonora segun el estado de alerta.
// En PELIGRO: apaga el rele inmediatamente de forma local,
// sin esperar confirmacion del servidor (proteccion preventiva).
// Cuando la corriente regresa a NORMAL, el rele se re-activa
// automaticamente (auto-recovery).
// =============================================================
void applyAlert(AlertStatus status) {
  if (status == STATUS_NORMAL) {
    leds.normal();
    buzzer.turnOff();
    if (!relay.isOn()) {
      relay.turnOn();
      Serial.println("[SEGURIDAD] Rele re-activado. Corriente normal.");
    }
  } else if (status == STATUS_ALERT) {
    leds.alert();
    buzzer.turnOff();
  } else {
    // STATUS_DANGER: Proteccion local inmediata sin depender de red
    relay.turnOff();
    buzzer.turnOn();
    leds.danger();
    Serial.println("[SEGURIDAD] Rele apagado por sobrecorriente local.");
  }
}

// =============================================================
// Setup
// =============================================================
void setup() {
  Serial.begin(115200);
  delay(1000);

  analogReadResolution(12);
  Wire.begin(PIN_OLED_SDA, PIN_OLED_SCL);

  // Inicializar actuadores primero por seguridad
  relay.begin();
  buzzer.begin();
  leds.begin();

  // Inicializar pantalla OLED
  if (!oled.begin()) {
    oledAvailable = false;
    Serial.println("[OLED] No encontrada. Verificar direccion 0x3C. Continuando sin pantalla.");
  }

  // Calibrar sensor analogico ACS712
  Serial.println("[ACS712] Calibrando...");
  acs712.begin();
  _showBootIfOled("Calibrando ACS712", "No mover cables...");
  delay(2000);
  acs712.calibrate();

  // Inicializar sensor digital INA219 (mismo bus I2C, dir 0x40)
  Serial.println("[INA219] Iniciando...");
  _showBootIfOled("Iniciando INA219", "Sensor I2C 0x40...");
  if (!ina219.begin()) {
    Serial.println("[INA219] No hallado. Usando solo ACS712.");
    _showBootIfOled("INA219 no hallado", "Usando solo ACS712");
    delay(2000);
  }

#if !OFFLINE_DEMO_MODE
  // Modo red: conectar WiFi y preparar cliente Edge
  _showBootIfOled("ElectroLink listo", "Iniciando WiFi...");
  wifi.begin();
#else
  // Modo demo fisica: usar umbrales estaticos, omitir red
  Serial.println("[Config] OFFLINE_DEMO_MODE activo. Umbrales estaticos.");
  _showBootIfOled("Modo DEMO activo", "Sin conexion de red");
  delay(2000);
#endif
}

// =============================================================
// Loop principal
// =============================================================
void loop() {
  // 1. Leer ambos sensores en paralelo
  float corrienteAcs = acs712.readCurrent();
  float corrienteIna = ina219.readCurrent();
  float voltaje      = ina219.isInitialized() ? ina219.readBusVoltage() : SUPPLY_VOLTAGE;
  float potencia     = ina219.isInitialized() ? ina219.readPower()      : (voltaje * corrienteAcs);

  // 2. Fusionar: usar el valor maximo de ambos sensores para la seguridad
  float corrienteMaxima = max(corrienteAcs, corrienteIna);

  // 3. Evaluar el estado de alerta con los umbrales actuales (estaticos o dinamicos)
  AlertStatus alertStatus = AlertService::evaluate(corrienteMaxima);
  String statusText = AlertService::toText(alertStatus);

  // 4. Aplicar respuesta de actuadores (incluye corte local si es PELIGRO)
  applyAlert(alertStatus);

  // 5. Actualizar pantalla OLED con lectura del INA219 como principal
  if (oledAvailable) oled.render(corrienteIna, potencia, relay.isOn(), statusText, currentNominalVoltage);

  // 6. Log en Serial para depuracion
  Serial.print("[ACS] ");
  Serial.print(corrienteAcs, 3);
  Serial.print(" A | [INA] ");
  Serial.print(corrienteIna, 3);
  Serial.print(" A | V: ");
  Serial.print(voltaje, 2);
  Serial.print(" V | P: ");
  Serial.print(potencia, 3);
  Serial.print(" W | Relay: ");
  Serial.print(relay.isOn() ? "ON" : "OFF");
  Serial.print(" | Estado: ");
  Serial.println(statusText);

#if !OFFLINE_DEMO_MODE
  // 7. Envio periodico al Edge Gateway (solo en modo con red)
  if (millis() - lastEdgeSend >= EDGE_SEND_INTERVAL_MS) {
    edgeClient.sendReading(
      DEVICE_ID,
      corrienteAcs,
      corrienteIna,
      voltaje,
      potencia,
      relay.isOn(),
      statusText
    );
    lastEdgeSend = millis();
  }
#endif

  delay(LOOP_DELAY_MS);
}
