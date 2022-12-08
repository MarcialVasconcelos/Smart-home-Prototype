#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Servo.h>
#include <ArduinoJson.h>

#include <ESP32ports.h>

// #include <string.h>
#define MSG_BUFFER_SIZE (50)
#define JSON_BUFFER_SIZE 256
#define TYPE 5
#define QUANTITY 5
int count = 0;

bool door1IsLocked = false;
bool door1IsOpen = false;

Servo door01;

int states[TYPE][QUANTITY] = {0};

// WIFI CONFIGS
const char *WIFI_SSID = "Alunos";
const char *WIFI_PASSWORD = "alunos2018";
WiFiClient espClient;

// MQTT CONFIGS
const char *mqtt_server = "broker.emqx.io";
const int mqtt_port = 1883;
// const char *mqtt_server = "test.mosquitto.org";
// const int mqtt_port = 8080;
const char *mqtt_generic_topic = "esp8266";
// String clientId = "mqttx_4a9e4351";
PubSubClient client(espClient);

// MQTT message buffer
unsigned long lastMsg = 0;
char msg[MSG_BUFFER_SIZE];
int value = 0;

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void lamp (int on, int index, int port) {
  Serial.print("Lamp");
  Serial.print(index);
  Serial.print(" ");
  if (on) {
    digitalWrite(port, HIGH); 
    Serial.print("on");
      states[0][index] = 1;
  }
  else {
    digitalWrite(port, LOW); 
    Serial.print("off");
      states[0][index] = 0;
  }
  Serial.println();
}

void door (int open, int index, Servo s) {
  if(open == 1){
    if(door1IsOpen) {
      Serial.println("Door is opened");
      return;
    }
    if(!door1IsLocked) {
      Serial.println("Opening the door");
      door1IsOpen = true;
      for (int i=0 ; i<=180 ; i+=10) {
        s.write(i);
        delay(1);
      }s.write(180);
      states[2][0] = 1;
    } else Serial.println("Unlock the door first");
  } else if(open == 0){
    if(!door1IsOpen) {
      Serial.println("Door is closed");
      return;
    }
    door1IsOpen = false;
    Serial.println("Closing the door");
    for (int i=180 ; i>=0 ; i-=10) {
      s.write(i);
      delay(1);
    }s.write(0);
      states[2][0] = 0;
  }
}

void lock (int lock, int index, int port) {
  if(lock == 1){
    if(!door1IsOpen) {
      door1IsLocked = true;
      digitalWrite(port, HIGH); 
      Serial.println("Locking door");
      states[1][0] = 1;
    } else Serial.println("Close the door first");
  } else if(lock == 0){
    door1IsLocked = false;
    digitalWrite(port, LOW);
    Serial.println("Unlocking door");
    states[1][0] = 0;
  }
}

void sendStates() {
  // JSON config  
  StaticJsonDocument<JSON_BUFFER_SIZE> doc;
  char buffer[JSON_BUFFER_SIZE];
  doc["Device"] = "ESP32";
  doc["Room"] = "02";
  JsonArray lampStates = doc.createNestedArray("Lamps");
  JsonArray lockStates = doc.createNestedArray("Locks");
  JsonArray doorStates = doc.createNestedArray("Doors");
  lampStates.add(states[0][0]);
  lampStates.add(states[0][1]);
  lampStates.add(states[0][2]);
  lampStates.add(states[0][3]);
  lockStates.add(states[1][0]);
  doorStates.add(states[2][0]);

  serializeJson(doc, buffer);
  client.publish("AutomataHomeStates", buffer);
}

void callback(char *topic, byte *payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

  for (unsigned int i = 0; i < length; i++) {
    msg[i] = (char)payload[i];
    Serial.print((char)payload[i]);
  }
  Serial.println();
  
  int type, index, data = 0;

  // Old Data input
  // type  = (int)payload[0]-48;
  // index = (int)payload[1]-48;
  // int j = 1;
  // for (unsigned int i = length-1; i > 1; i--) {
  //   data += j*((int)payload[i]-48);
  //   j *= 10;
  // }

  type  = (int)payload[0];
  index = (int)payload[1];
  data  = (int)payload[2];
  
  // Serial.println("type: "+type);
  // Serial.println("index: "+index);
  // Serial.println("data: "+data);

  //Lampadas
  if (type == 0){
    switch (index) {
    case 0: lamp(data, index, D0); break;
    case 1: lamp(data, index, D1); break;
    case 2: lamp(data, index, D2); break;
    case 3: lamp(data, index, D3); break;
    default: Serial.println("Invalid lamp"); break;
    } 
  }

  //Trancas
  if (type == 1){
    switch (index) {
    case 0: lock(data, index, D4); break;
    default: Serial.println("Invalid lock"); break;
    } 
  }  
  
  //Portas
  if (type == 2){
    switch (index) {
    case 0: door(data, index, door01); break;
    default: Serial.println("Invalid door"); break;
    } 
  }
  
  sendStates();
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish(mqtt_generic_topic, "hello world");
      // ... and resubscribe
      client.subscribe("Room02");
    }
    else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  pinMode (D0,OUTPUT);  // Initialize the LED pin as an output
  pinMode (D1,OUTPUT);  // Initialize the LED pin as an output
  pinMode (D2,OUTPUT);  // Initialize the LED pin as an output
  pinMode (D3,OUTPUT);  // Initialize the LED pin as an output
  pinMode (D4,OUTPUT);  // Initialize the LED pin as an output
  door01.attach(D5);    // Initialize the D6 pin as an output to control the servo
  door01.write(0);      // Initialize the servo to 0 degrees
  Serial.begin(9600);
  setup_wifi();

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

}

void loop() {
  if (!client.connected()) reconnect();

  client.loop();  
  
  if (count == 30000){
    count = 0;
    sendStates();
  }
  count++;
  delay(1);
}