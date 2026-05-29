#include <Arduino.h>
#include <WiFi.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include "ir_codes.h" 

#include <SinricPro.h>
#include <SinricProTV.h>
#include <SinricProLight.h>

// ==========================================
// 1. CONFIGURACIÓN DE TU RED WIFI
// ==========================================
#define WIFI_SSID "Redmi Note 13"
#define WIFI_PASS "0503jara"

// ==========================================
// 2. CREDENCIALES DE SINRIC PRO
// ==========================================
#define APP_KEY    "ec57fabd-d22b-4ec7-b673-26817a8eca71"
#define APP_SECRET "3a6c6512-e0eb-4688-aee0-54d0d40aa2e7-07694a2c-66b1-41fa-a54f-706e84e4fdec"

#define TV_ID      "6a068044baa50bf9bf33d08c"
#define AUDIO_ID   "6a068424f9b5f15fa7d54d5f"
#define FOCO_ID    "6a0684bef9b5f15fa7d54e26"

const uint16_t kIrLed = 15; 
IRsend irsend(kIrLed);

// ==========================================
// FUNCIONES DEL TELEVISOR
// ==========================================
bool onPowerStateTV(const String &deviceId, bool &state) {
  Serial.printf("Alexa ordenó: TV %s\n", state ? "ENCENDIDA" : "APAGADA");
  irsend.sendRaw(DECO_POWER_RAW, sizeof(DECO_POWER_RAW) / sizeof(DECO_POWER_RAW[0]), 38);
  return true;
}

bool onAdjustVolumeTV(const String &deviceId, int &volumeDelta, bool volumeDefault) {
  Serial.printf("Alexa ordenó: TV Volumen %s\n", volumeDelta > 0 ? "SUBIR" : "BAJAR");
  if (volumeDelta > 0) {
    irsend.sendRaw(DECO_VOL_UP_RAW, sizeof(DECO_VOL_UP_RAW) / sizeof(DECO_VOL_UP_RAW[0]), 38);
  } else {
    irsend.sendRaw(DECO_VOL_DOWN_RAW, sizeof(DECO_VOL_DOWN_RAW) / sizeof(DECO_VOL_DOWN_RAW[0]), 38);
  }
  volumeDelta = (volumeDelta > 0) ? 1 : -1;
  return true;
}

// HACK NAVEGACIÓN: Solo 4 direcciones (Arriba, Abajo, Izquierda, Derecha)
bool onMediaControlTV(const String &deviceId, String &control) {
  Serial.printf("Alexa ordenó: TV Movimiento -> '%s'\n", control.c_str());

  if (control == "Next") {
    irsend.sendRaw(DECO_RIGHT_RAW, sizeof(DECO_RIGHT_RAW) / sizeof(DECO_RIGHT_RAW[0]), 38);
    Serial.println("Disparando: DERECHA");
  } 
  else if (control == "Previous") {
    irsend.sendRaw(DECO_LEFT_RAW, sizeof(DECO_LEFT_RAW) / sizeof(DECO_LEFT_RAW[0]), 38);
    Serial.println("Disparando: IZQUIERDA");
  } 
  else if (control == "FastForward") {
    irsend.sendRaw(DECO_UP_RAW, sizeof(DECO_UP_RAW) / sizeof(DECO_UP_RAW[0]), 38);
    Serial.println("Disparando: ARRIBA");
  } 
  else if (control == "Rewind") {
    irsend.sendRaw(DECO_DOWN_RAW, sizeof(DECO_DOWN_RAW) / sizeof(DECO_DOWN_RAW[0]), 38);
    Serial.println("Disparando: ABAJO");
  }

  // SEGURO ANTICOLISIÓN TV
  delay(500);
  
  return true;
}

// MUTE REAL: Dispara la orden para silenciar la TV
bool onMuteTV(const String &deviceId, bool &mute) {
  Serial.printf("Alexa ordenó: TV %s -> Disparando: MUTE\n", mute ? "Silenciar" : "Quitar Silencio");
  irsend.sendRaw(DECO_MUTE_RAW, sizeof(DECO_MUTE_RAW) / sizeof(DECO_MUTE_RAW[0]), 38);
  return true;
}

// ==========================================
// FUNCIONES DEL EQUIPO DE SONIDO (Modificadas con pausas reales de 200ms)
// ==========================================
bool onPowerStateAudio(const String &deviceId, bool &state) {
  Serial.printf("Alexa ordenó: EQUIPO DE SONIDO %s\n", state ? "ENCENDIDO" : "APAGADO");
  for (int i = 0; i < 3; i++) {
    irsend.sendRaw(AUDIO_POWER_RAW, sizeof(AUDIO_POWER_RAW) / sizeof(AUDIO_POWER_RAW[0]), 40);
    delay(200); // <-- Pausa ampliada para que el equipo lo procese bien
  }
  return true;
}

bool onAdjustVolumeAudio(const String &deviceId, int &volumeDelta, bool volumeDefault) {
  Serial.printf("Alexa ordenó: EQUIPO DE SONIDO Volumen %s\n", volumeDelta > 0 ? "SUBIR" : "BAJAR");
  if (volumeDelta > 0) {
    for (int i = 0; i < 3; i++) {
      irsend.sendRaw(AUDIO_VOL_UP_RAW, sizeof(AUDIO_VOL_UP_RAW) / sizeof(AUDIO_VOL_UP_RAW[0]), 40);
      delay(200); // <-- Pausa ampliada
    }
  } else {
    for (int i = 0; i < 3; i++) {
      irsend.sendRaw(AUDIO_VOL_DOWN_RAW, sizeof(AUDIO_VOL_DOWN_RAW) / sizeof(AUDIO_VOL_DOWN_RAW[0]), 40);
      delay(200); // <-- Pausa ampliada
    }
  }
  volumeDelta = (volumeDelta > 0) ? 1 : -1;
  return true;
}

// Hack: Cambiar Función usando "Siguiente" o "Anterior"
bool onMediaControlAudio(const String &deviceId, String &control) {
  Serial.printf("Alexa ordenó: EQUIPO DE SONIDO Media -> '%s'\n", control.c_str());
  if (control == "Next" || control == "Previous") {
    for (int i = 0; i < 3; i++) {
      irsend.sendRaw(AUDIO_FUNCTION_RAW, sizeof(AUDIO_FUNCTION_RAW) / sizeof(AUDIO_FUNCTION_RAW[0]), 40);
      delay(200); // <-- Pausa ampliada
    }
  }
  return true;
}

// ==========================================
// FUNCIONES DEL FOCO
// ==========================================
bool onPowerStateFoco(const String &deviceId, bool &state) {
  Serial.printf("Alexa ordenó: FOCO %s\n", state ? "ENCENDIDO" : "APAGADO");
  irsend.sendRaw(FOCO_POWER_RAW, sizeof(FOCO_POWER_RAW) / sizeof(FOCO_POWER_RAW[0]), 38);
  return true;
}

bool onAdjustBrightnessFoco(const String &deviceId, int &brightnessDelta) {
  Serial.printf("Alexa ordenó: FOCO Brillo %s\n", brightnessDelta > 0 ? "SUBIR" : "BAJAR");
  if (brightnessDelta > 0) {
    irsend.sendRaw(FOCO_BRIGHT_UP_RAW, sizeof(FOCO_BRIGHT_UP_RAW) / sizeof(FOCO_BRIGHT_UP_RAW[0]), 38);
  } else {
    irsend.sendRaw(FOCO_BRIGHT_DOWN_RAW, sizeof(FOCO_BRIGHT_DOWN_RAW) / sizeof(FOCO_BRIGHT_DOWN_RAW[0]), 38);
  }
  return true;
}

// ==========================================
// INICIALIZACIÓN PRINCIPAL
// ==========================================
void setupWiFi() {
  Serial.printf("\nConectando al WiFi %s ", WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\n¡Conectado al WiFi!");
  Serial.print("IP del ESP32: ");
  Serial.println(WiFi.localIP());
}

void setupSinricPro() {
  // Configurar Televisor
  SinricProTV &miTV = SinricPro[TV_ID];
  miTV.onPowerState(onPowerStateTV);
  miTV.onAdjustVolume(onAdjustVolumeTV);
  miTV.onMediaControl(onMediaControlTV);  
  miTV.onMute(onMuteTV); 

  // Configurar Equipo de Sonido
  SinricProTV &miSonido = SinricPro[AUDIO_ID];
  miSonido.onPowerState(onPowerStateAudio);
  miSonido.onAdjustVolume(onAdjustVolumeAudio);
  miSonido.onMediaControl(onMediaControlAudio); 

  // Configurar Foco
  SinricProLight &miFoco = SinricPro[FOCO_ID];
  miFoco.onPowerState(onPowerStateFoco);
  miFoco.onAdjustBrightness(onAdjustBrightnessFoco);

  // Iniciar conexión
  SinricPro.onConnected([](){ Serial.println("Conectado a la Nube de Sinric Pro"); });
  SinricPro.onDisconnected([](){ Serial.println("Desconectado de Sinric Pro"); });
  SinricPro.begin(APP_KEY, APP_SECRET);
}

void setup() {
  Serial.begin(115200);
  irsend.begin(); 
  
  setupWiFi();
  setupSinricPro();
  
  Serial.println("=========================================");
  Serial.println(" SISTEMA IR POR VOZ ACTIVO Y ESCUCHANDO  ");
  Serial.println("=========================================");
}

void loop() {
  SinricPro.handle(); 
}
