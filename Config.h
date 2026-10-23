#pragma once

// =============================================================
// Config.h - Configuracion global del firmware ElectroLink IoT
// WOKWI_SIMULATION : Activar para simulacion en Wokwi.
//   - La pantalla usa board-ssd1306 (I2C real simulado).
//   - El ACS712 es simulado por el potenciometro en PIN_ACS712 (pin 34).
//   - El INA219 es simulado por un segundo potenciometro en PIN_INA_SIM (pin 35).
// OFFLINE_DEMO_MODE: Activar para la expo fisica sin internet.
// Aqui se concentran todas las constantes de configuracion del
// dispositivo. Cambiar OFFLINE_DEMO_MODE a true para la expo
// fisica donde no hay conexion a internet disponible.
// =============================================================

#define DEVICE_ID "electrolink-esp32-01"

// --- Red WiFi ---
// Para Wokwi usar: SSID="Wokwi-GUEST", PASSWORD=""
#define WIFI_SSID     "Wokwi-GUEST"
#define WIFI_PASSWORD ""

// --- Endpoint del Edge Gateway ---
// En desarrollo local: URL de ngrok (ej: https://xxxx.ngrok.io/api/electrolink/readings)
// En produccion: URL del servicio en Koyeb/Render
#define EDGE_API_URL "https://axonometric-marchelle-ommatidial.ngrok-free.dev/api/electrolink/readings"

// --- Pantalla OLED ---
#define OLED_ADDRESS  0x3C
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 64

// --- Parametros del ACS712 ---
#define SUPPLY_VOLTAGE      5.0
#define ACS712_SENSITIVITY  0.185   // ACS712 5A = 185 mV/A

// --- Modo de operacion ---
// true  = Wokwi: INA219 se lee del potenciometro en PIN_INA_SIM
// false = Hardware fisico: INA219 real por I2C (0x40)
#define WOKWI_SIMULATION true

// false = modo online (umbrales dinamicos desde el Edge)
// true  = modo exposicion fisica (umbrales estaticos, sin red)
#define OFFLINE_DEMO_MODE false

// --- Umbrales por defecto (se sobreescriben si OFFLINE_DEMO_MODE=false y hay red) ---
#define DEFAULT_NORMAL_LIMIT 0.20   // A: Corriente maxima en estado NORMAL
#define DEFAULT_ALERT_LIMIT  0.60   // A: Corriente maxima antes de cortar el rele
#define DEFAULT_NOMINAL_VOLTAGE 220.0 // V: Voltaje nominal del electrodomestico (AC)

// --- Rele ---
#define RELAY_ACTIVE_HIGH true

// --- Temporizacion ---
#define EDGE_SEND_INTERVAL_MS 5000
#define LOOP_DELAY_MS         500

// =============================================================
// Variables globales mutables de umbrales (definidas en el .ino)
// Se declaran extern aqui para que todos los archivos .h puedan
// leerlas y el HttpEdgeGatewayClient pueda actualizarlas.
// =============================================================
extern float currentNormalLimit;
extern float currentAlertLimit;
extern float currentNominalVoltage;
