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
String slider2;
String slider3;
const int EchoPin = D8;
const int TriggerPin = D7;
const int EchoPin2 = D3;
const int TriggerPin2 = D4;
const int EchoPin3 = D0;
const int TriggerPin3 = D1;
const int LedPin = D2;
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
void statusfoco(int nslide, int distance){
  if(nslide<distance){
    digitalWrite(ledPin, LOW);
    Serial.println("S332");
    Serial.println(nslide);
    Serial.println(distance);
  }else{
    digitalWrite(ledPin, HIGH);
    Serial.println("S442");
    Serial.println(nslide);
    Serial.println(distance);
  }  
}
void statusfoco2(int nslide, int distance){
  if(nslide<distance){
    digitalWrite(ledPin, LOW);
    Serial.println("S332");
    Serial.println(nslide);
    Serial.println(distance);
  }else{
    digitalWrite(ledPin, HIGH);
    Serial.println("S442");
    Serial.println(nslide);
    Serial.println(distance);
  }  
}
void statusfoco3(int nslide, int distance){
  if(nslide<distance){
    digitalWrite(ledPin, LOW);
    Serial.println("S332");
    Serial.println(nslide);
    Serial.println(distance);
  }else{
    digitalWrite(ledPin, HIGH);
    Serial.println("S442");
    Serial.println(nslide);
    Serial.println(distance);
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
  pinMode(LedPin, OUTPUT);
  pinMode(TriggerPin, OUTPUT);
  pinMode(EchoPin, INPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(TriggerPin2, OUTPUT);
  pinMode(EchoPin2, INPUT);
  pinMode(TriggerPin3, OUTPUT);
  pinMode(EchoPin3, INPUT);
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
    itoa(ping(TriggerPin, EchoPin), snum, 10);
    char snum2[5];
    //itoa(ping2(TriggerPin2, EchoPin2), snum, 10);
    itoa(0, snum2, 10);
    char snum3[5];
    itoa(0, snum3, 10);
    //itoa(ping3(TriggerPin3, EchoPin3), snum, 10);
    char buffer2[20];
    strcat(strcpy(buffer2, snum), ";");
    char buffer3[20];
    strcat(strcpy(buffer3, buffer2), snum2);
    char buffer4[20];
    strcat(strcpy(buffer4, buffer3), ";");
    char buffer5[20];
    strcat(strcpy(buffer5, buffer4), snum3);
    statusfoco(slider.toInt(), ping(TriggerPin, EchoPin));
    if(flag_mode){request->send_P(200, "text/plain", buffer5);}
    Serial.println(ping(TriggerPin, EchoPin));
    Serial.println(buffer2);
  });
  server.on("/SLIDER", HTTP_POST, [](AsyncWebServerRequest *request){
            slider = request->arg("distanceInput1");
            Serial.print("Distancia limite 1: \t");
            Serial.println(slider);
            request->redirect("/");
            });
  server.on("/SLIDER1", HTTP_POST, [](AsyncWebServerRequest *request){
            slider2 = request->arg("distanceInput11");
            Serial.print("Distancia limite 2: \t");
            Serial.println(slider2);
            request->redirect("/");
            });
  server.on("/SLIDER2", HTTP_POST, [](AsyncWebServerRequest *request){
            slider3 = request->arg("distanceInput111");
            Serial.print("Distancia limite 3: \t");
            Serial.println(slider3);
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

int ping(int TriggerPin, int EchoPin) {
   long duration, distanceCm=0;
   
   digitalWrite(TriggerPin, LOW);  //para generar un pulso limpio ponemos a LOW 4us
   delayMicroseconds(4);
   digitalWrite(TriggerPin, HIGH);  //generamos Trigger (disparo) de 10us
   delayMicroseconds(10);
   digitalWrite(TriggerPin, LOW);
   
   duration = pulseIn(EchoPin, HIGH);  //medimos el tiempo entre pulsos, en microsegundos
   
   distanceCm = duration * 10 / 292/ 2;   //convertimos a distancia, en cm
   return distanceCm;
}
int ping2(int TriggerPin, int EchoPin) {
   long duration, distanceCm=0;
   
   digitalWrite(TriggerPin, LOW);  //para generar un pulso limpio ponemos a LOW 4us
   delayMicroseconds(4);
   digitalWrite(TriggerPin, HIGH);  //generamos Trigger (disparo) de 10us
   delayMicroseconds(10);
   digitalWrite(TriggerPin, LOW);
   
   duration = pulseIn(EchoPin, HIGH);  //medimos el tiempo entre pulsos, en microsegundos
   
   distanceCm = duration * 10 / 292/ 2;   //convertimos a distancia, en cm
   return distanceCm;
}
int ping3(int TriggerPin, int EchoPin) {
   long duration, distanceCm=0;
   
   digitalWrite(TriggerPin, LOW);  //para generar un pulso limpio ponemos a LOW 4us
   delayMicroseconds(4);
   digitalWrite(TriggerPin, HIGH);  //generamos Trigger (disparo) de 10us
   delayMicroseconds(10);
   digitalWrite(TriggerPin, LOW);
   
   duration = pulseIn(EchoPin, HIGH);  //medimos el tiempo entre pulsos, en microsegundos
   
   distanceCm = duration * 10 / 292/ 2;   //convertimos a distancia, en cm
   return distanceCm;
}
