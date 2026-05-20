#include <WiFi.h>          // Activa las funciones de Wi-Fi del ESP32 📶
#include <PubSubClient.h>  // Controla la comunicación MQTT 🦅
#include <HTTPClient.h>    // Permite al ESP32 descargar archivos pesados 🌐
#include <Update.h>        // Escribe el nuevo firmware en la memoria Flash 🛠️

// 📶 Mantenemos las mismas credenciales de red
const char* ssid = "TU_SSID_WIFI";          
const char* password = "TU_PASSWORD_WIFI";  

// ☁️ El mismo canal MQTT para poder recibir futuras actualizaciones si es necesario
const char* mqtt_server = "broker.hivemq.com";     
const char* ota_topic = "alarma/comunitaria/ota";  

// 💡 Los mismos pines de hardware
const int LED_A = 12;  
const int LED_B = 14;  

WiFiClient espClient;           
PubSubClient client(espClient); 

void ejecutarOTA(String url) {
  HTTPClient http;      
  http.begin(url);      
  int httpCode = http.GET(); 
  
  if (httpCode == HTTP_CODE_OK) { 
    int contentLength = http.getSize(); 
    bool canBegin = Update.begin(contentLength); 
    
    if (canBegin) { 
      WiFiClient* stream = http.getStreamPtr(); 
      size_t written = Update.writeStream(*stream); 
      
      if (written == contentLength) { 
        if (Update.end()) { 
          if (Update.isFinished()) { 
            ESP.restart(); // Se reiniciaría de nuevo si vuelves a mandar otro código en el futuro
          }
        }
      }
    }
  }
  http.end(); 
}

void callback(char* topic, byte* payload, unsigned int length) {
  String mensaje = ""; 
  for (int i = 0; i < length; i++) { mensaje += (char)payload[i]; } 
  if (mensaje.startsWith("http")) { 
    ejecutarOTA(mensaje); 
  }
}

// 🎬 Configuración Inicial de la VERSIÓN 2
void setup() {
  pinMode(LED_A, OUTPUT); 
  pinMode(LED_B, OUTPUT); 
  
  // 🔵 ¡EL CAMBIAZO! En esta versión apagamos el LED A y encendemos el LED B
  digitalWrite(LED_A, LOW);   // Apagamos el LED verde de la versión anterior
  digitalWrite(LED_B, HIGH);  // Encendemos el LED azul que demuestra el éxito de la OTA

  WiFi.begin(ssid, password); 
  while (WiFi.status() != WL_CONNECTED) { delay(500); } 

  client.setServer(mqtt_server, 1883); 
  client.setCallback(callback); 
}

void loop() {
  if (!client.connected()) { 
    if (client.connect("ESP32_Alarma_Prueba")) { 
      client.subscribe(ota_topic); 
    }
  }
  client.loop(); 
}