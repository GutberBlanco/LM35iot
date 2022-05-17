 #include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "FS.h"
#include <Arduino_JSON.h>
#include "config.h"

boolean flag_mode=true;
const int ledPin=2;
String slider;
// Creamos el servidor AsyncWebServer en el puerto 80
AsyncWebServer server(80);

// Create an Event Source on /events
AsyncEventSource events("/events");

// Json Variable to Hold Sensor Readings
JSONVar readings;

// Timer variables
unsigned long lastTime = 0;  
unsigned long timerDelay = 1000; // actualizacion cada segundo//30000;


// Get Sensor Readings and return JSON object
String getSensorReadings(){

  readings["temperature"] = String(analogRead(A0)*0.488758553275);
  readings["humidity"] =  String((analogRead(A0)*0.488758553275)/10);
  String jsonString = JSON.stringify(readings);
  statusfoco(slider.toInt(), analogRead(A0)*0.488758553275);
  return jsonString;
}
void statusfoco(int nslide, float temp){
  if(nslide<temp){
    digitalWrite(ledPin, LOW);
    Serial.println("S332");
    Serial.println(nslide);
    Serial.println(temp);
  }else{
    digitalWrite(ledPin, HIGH);
    Serial.println("S442");
    Serial.println(nslide);
    Serial.println(temp);
  }  
}

// Initialize LittleFS
void initFS() {
 // Iniciamos  SPIFFS
  if(!SPIFFS.begin())
     { Serial.println("ha ocurrido un error al montar SPIFFS");
       return; }
}

// Inicializando WiFi
void initWiFi() {
// conectamos al Wi-Fi
  WiFi.begin(ssid, password);
  // Mientras no se conecte, mantenemos un bucle con reintentos sucesivos
  while (WiFi.status() != WL_CONNECTED) {
      delay(1000);
      // Esperamos un segundo
      Serial.println("Conectando a la red WiFi..");
    }
  Serial.println();
  Serial.println(WiFi.SSID());
  Serial.print("Direccion IP:\t");
  // Imprimimos la ip que le ha dado nuestro router
  Serial.println(WiFi.localIP());
}

void setup() {
  Serial.begin(115200);

  pinMode(ledPin, OUTPUT);
  initWiFi();
  initFS();

  // Web Server Root URL
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", "text/html");
  });

  server.serveStatic("/", SPIFFS, "/");
  
  // Request for the latest sensor readings
  server.on("/readings", HTTP_GET, [](AsyncWebServerRequest *request){
    String json = getSensorReadings();
    request->send(200, "application/json", json);
    json = String();
  });
  server.on("/ADC", HTTP_GET, [](AsyncWebServerRequest *request){
    char snum[5];
    itoa(analogRead(A0)*0.488758553275, snum, 10);
    
    if(flag_mode){request->send_P(200, "text/plain", snum);}
  Serial.println(getSensorReadings());
  });
  server.on("/SLIDER", HTTP_POST, [](AsyncWebServerRequest *request){
            slider = request->arg("distanceInput1");
            Serial.print("Temperatura: \t");
            Serial.println(slider);
            request->redirect("/");
            });

  events.onConnect([](AsyncEventSourceClient *client){
    if(client->lastId()){
      Serial.printf("Client reconnected! Last message ID that it got is: %u\n", client->lastId());
    }
    // send event with message "hello!", id current millis
    // and set reconnect delay to 1 second
    client->send("hello!", NULL, millis(), 10000);
  });
  server.addHandler(&events);

  // Start server
  server.begin();
}

void loop() {
  if ((millis() - lastTime) > timerDelay) {
    // Send Events to the client with the Sensor Readings Every 30 seconds
    events.send("ping",NULL,millis());
    events.send(getSensorReadings().c_str(),"new_readings" ,millis());
    lastTime = millis();
  }
}
