# ElectroLink IoT - Guía de Simulación Wokwi

## Requisitos

1. Una cuenta gratuita en [Wokwi](https://wokwi.com)
2. Navegador web moderno (Chrome, Edge, Firefox)
3. Opcional: [ngrok](https://ngrok.com) para integrar con la Edge API local

## Cómo abrir en Wokwi

### Opción 1: Web IDE (arrastrar carpeta)

1. Ve a [Wokwi.com](https://wokwi.com) → **New Project** → **ESP32**
2. En el editor de la izquierda, haz clic en los archivos y reemplázalos uno por uno:
   - `diagram.json` → pega el contenido de este archivo
   - `libraries.txt` → pega el contenido de este archivo
   - `ElectroLinkIoT.ino` → pega el contenido de este archivo
   - Agrega cada `.h` como nuevo archivo
3. Haz clic en **Start Simulation**

### Opción 2: Proyecto local (CLI)

```bash
# Instalar wokwi-cli (si no lo tienes)
pip install wokwi-cli

# En la carpeta del proyecto
cd ElectroLinkIoT/
wokwi start
```

## Diagrama de Conexiones

| Componente Wokwi | Pin ESP32 | Propósito |
|---|---|---|
| `pot_acs` (potenciómetro) | GPIO 34 (ADC) | Simula sensor ACS712 |
| `pot_ina` (potenciómetro) | GPIO 35 (ADC) | Simula sensor INA219 |
| `oled1` (SSD1306) | SDA→GPIO 21, SCL→GPIO 22 | Pantalla OLED I2C (0x3C) |
| `relay1` (relé) | IN→GPIO 26 | Corte de carga |
| `bz1` (buzzer) | +→GPIO 33 | Alarma sonora |
| `led_green` + R 220Ω | Ánodo→GPIO 14 | LED verde (NORMAL) |
| `led_yellow` + R 220Ω | Ánodo→GPIO 27 | LED amarillo (ALERTA) |
| `led_red` + R 220Ω | Ánodo→GPIO 25 | LED rojo (PELIGRO) |

> **Nota:** En Wokwi no existen componentes nativos para sensores ACS712 o INA219.
> Por eso se simulan con **potenciómetros** conectados a pines ADC.
> Al pasar a hardware físico, se usan los sensores reales.

## Modos de Operación

El firmware tiene dos flags de compilación en `Config.h`:

### `WOKWI_SIMULATION` (bool)

| Valor | Comportamiento |
|---|---|
| `true` | **Wokwi.** INA219 se lee del potenciómetro en GPIO 35 |
| `false` | **Hardware físico.** INA219 real se comunica por I2C (dir 0x40) |

### `OFFLINE_DEMO_MODE` (bool)

| Valor | Comportamiento |
|---|---|
| `false` | **Modo online.** Conecta WiFi, sincroniza umbrales con Edge API |
| `true` | **Modo demo.** Sin WiFi, usa umbrales estáticos (`DEFAULT_NORMAL_LIMIT` y `DEFAULT_ALERT_LIMIT`) |

**Configuración recomendada para Wokwi:**
```cpp
#define WOKWI_SIMULATION true
#define OFFLINE_DEMO_MODE false
```

## Cómo Probar en Wokwi

### Test 1: Corriente Normal

- Coloca ambos potenciómetros al **mínimo** (giro completo a la izquierda)
- LED **verde** debe encenderse
- Relé debe estar **ON** (simulado con el LED del módulo relé)
- Buzzer debe estar **apagado**

### Test 2: Alerta (sobrecorriente media)

- Gira el potenciómetro del INA219 (`pot_ina`) hasta pasar ~0.20 A
- LED **amarillo** debe encenderse
- Relé debe permanecer **ON** (solo advertencia visual)

### Test 3: Peligro (sobrecorriente crítica)

- Sigue girando `pot_ina` hasta pasar ~0.60 A
- LED **rojo** debe encenderse
- Relé debe apagarse **inmediatamente** (corte preventivo local)
- Buzzer debe sonar

### Test 4: Auto-Recovery

- Vuelve ambos potenciómetros al mínimo
- El relé debe re-activarse automáticamente al volver a estado NORMAL

### Test 5: Activar Modo Demo Offline

```cpp
#define OFFLINE_DEMO_MODE true
```

- El ESP32 **no** debe intentar conectar WiFi
- Todos los tests anteriores deben funcionar con umbrales estáticos

## Integración con Edge API

### Con ngrok (desarrollo local)

```bash
# 1. Inicia la Edge API
cd electrolink-edge-api/
python main.py

# 2. Abre túnel ngrok
ngrok http 3000

# 3. Copia la URL de ngrok (ej: https://abc123.ngrok.io)
#    y actualiza Config.h:
// #define EDGE_API_URL "http://192.168.1.100:3000/api/electrolink/readings"
#define EDGE_API_URL "https://abc123.ngrok.io/api/electrolink/readings"
```

### Payload que envía el ESP32

```json
{
  "deviceId": "electrolink-esp32-01",
  "currentAcs": 0.235,
  "currentIna": 0.241,
  "voltage": 5.0,
  "power": 1.175,
  "relay": true,
  "status": "NORMAL",
  "timestamp": 15000
}
```

### Respuesta esperada del Edge

```json
{
  "status": "success",
  "thresholds": {
    "normalLimit": 0.20,
    "alertLimit": 0.60
  }
}
```

## Troubleshooting

### "No SIM card" o error de compilación

- Verifica que `libraries.txt` tenga todas las dependencias listadas
- Asegúrate de que el `diagram.json` esté en el mismo nivel que el `.ino`
- Si algún componente no carga, reinicia la simulación

### La pantalla OLED no se ve

- Revisa las conexiones I2C: SDA→GPIO 21, SCL→GPIO 22
- La dirección debe ser `0x3C` (está configurada por defecto)
- Si no se detecta, el firmware continúa sin pantalla (no se cuelga)

### El relé no se activa/desactiva

- Verifica valor de `RELAY_ACTIVE_HIGH` en `Config.h`
- En Wokwi el relé se ve como un LED azul en el módulo
- El relé se corta automáticamente en estado `PELIGRO` y se re-activa al volver a `NORMAL`

### No se ven ambos sensores en el Serial

- Busca en la terminal Serial los mensajes `[ACS]` e `[INA]`
- Si solo ves mensajes `[ACS]`, el INA219 simulado no está inicializado:
  - Revisa que `WOKWI_SIMULATION` sea `true`
  - Verifica que `PIN_INA_SIM` (GPIO 35) esté conectado al potenciómetro

### Error de conexión WiFi

- Wokwi usa la red `Wokwi-GUEST` (sin contraseña)
- Si falla, activa `OFFLINE_DEMO_MODE true` para operar localmente
- El firmware funciona sin red; solo deja de sincronizar con el Edge

## Archivos del Proyecto

| Archivo | Propósito |
|---|---|
| `ElectroLinkIoT.ino` | Punto de entrada, setup y loop principal |
| `Config.h` | Constantes de configuración y modo de operación |
| `Pins.h` | Asignación de pines GPIO |
| `Sensor.h` | Interfaz abstracta base para sensores |
| `Acs712Sensor.h` | Sensor analógico ACS712 |
| `Ina219Sensor.h` | Sensor digital INA219 (o simulación por potenciómetro) |
| `Actuator.h` | Interfaz abstracta base para actuadores |
| `RelayActuator.h` | Control del relé de corte |
| `BuzzerActuator.h` | Control del zumbador de alarma |
| `LedIndicators.h` | Terna de LEDs de estado (verde/amarillo/rojo) |
| `OledDisplay.h` | Pantalla OLED I2C 128x64 |
| `AlertService.h` | Evaluación de alertas contra umbrales |
| `WiFiConnectivityDriver.h` | Gestión de conexión WiFi |
| `HttpEdgeGatewayClient.h` | Cliente HTTP para comunicación con Edge API |
| `diagram.json` | Circuito del simulador Wokwi |
| `libraries.txt` | Dependencias de librerías Wokwi |
