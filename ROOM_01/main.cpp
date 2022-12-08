#include <ESP8266WiFi.h>
#include <Arduino.h>
#include <PubSubClient.h>
#include <Servo.h>
#include <string.h>
#include <ArduinoJson.h>
#define MSG_BUFFER_SIZE (50)

Servo servo;
#define D0 16
#define D1 5
#define D2 4
#define D3 0
#define D4 2
#define D5 14

// WIFI CONFIGS
const char *WIFI_SSID = "Alunos";
const char *WIFI_PASSWORD = "alunos2018";
WiFiClient espClient;

// MQTT CONFIGS
const char *mqtt_server = "broker.emqx.io";
const int mqtt_port = 1883;
const char *mqtt_generic_topic = "esp8266-33";
const char *listening_topic = "esp8266/1";
// String clientId = "mqttx_4a9e4351";
PubSubClient client(espClient);

// MQTT message buffer
unsigned long lastMsg = 0;
char msg[MSG_BUFFER_SIZE];
int value = 0;

/*Definições de Estados*/
#define JSON_BUFFER_SIZE 256
#define TYPE 5
#define QUANTITY 5
int count = 0;
int states[TYPE][QUANTITY] = {0};
void sendStates();

void setup_wifi()
{

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

char *getSpecifiedNumberOfBytes(byte *payload, int length, int start, int end)
{
  char result[end - start + 1];
  for (int i = start, j = 0; i <= end; i++, j++)
  {
    result[j] = (char)payload[i];
  }
  return result;
}

void setServoPosition(int angle)
{
  servo.write(angle);
}

void callback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  Serial.println();

  // char message[length];
  for (int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  switch (payload[0])
  {
  case 0x01:
  {
    Serial.println("Led");
    int index = (int)payload[1];
    int ledState = (int)payload[2];

    switch (index)
    {
    case 0x01:
    {
      int port = (int)payload[1];
      Serial.println(port);
      Serial.println(ledState);
      digitalWrite(D1, ledState);
      states[0][0] = ledState;
      sendStates();
    }
    default:
      // Serial.println("Lamp " + index + " does not exist.");
      break;
    }
  }
  break;
  case 0x02:
  {
    Serial.println("Servo");
    int angle = (int)payload[1];
    Serial.println(angle);

    // setServoPosition(angle);
    int currentAngle = states[1][0];
    if (angle > currentAngle)
    {
      for (int i = currentAngle; i < angle; i += 5)
      {
        setServoPosition(i);
        // delay(1);
        states[1][0] = i;
        sendStates();
      }
      setServoPosition(angle);
    }
    else
    {
      for (int i = currentAngle; i > angle; i -= 5)
      {
        setServoPosition(i);
        // delay(1);
        states[1][0] = i;
        sendStates();
      }
      setServoPosition(angle);
    }

    states[1][0] = angle;
    sendStates();
  }
  break;
  default:
  {
    Serial.println("Invalid header");
  }
  break;
  }
}

// void callback(char *topic, byte *payload, unsigned int length)
// {
//   Serial.print("Message arrived [");
//   Serial.print(topic);
//   Serial.print("] ");

//   Serial.println(payload[0]);

//   char message[length];
//   for (int i = 0; i < length; i++)
//   {
//     message[i] = (char)payload[i];
//     Serial.print((char)payload[i]);
//   }
//   Serial.println();
//   Serial.println(message);

//   if (strcmp(message, "D4-1") == 0)
//   {
//     digitalWrite(LED_BUILTIN, LOW); // Turn the LED on (Note that LOW is the voltage level
//                                     // but actually the LED is on; this is because
//                                     // it is active low on the ESP-01)
//     servo.write(0);
//   }
//   else if (strcmp(message, "D4-0") == 0)
//   {
//     digitalWrite(LED_BUILTIN, HIGH); // Turn the LED off by making the voltage HIGH
//     servo.write(180);
//   }
//   else if (strcmp(message, "D1-0") == 0)
//   {
//     digitalWrite(D1, LOW);
//   }
//   else if (strcmp(message, "D1-1") == 0)
//   {
//     digitalWrite(D1, HIGH);
//   }
//   else
//   {
//     Serial.println("Invalid message");
//   }
// }

void reconnect()
{
  // Loop until we're reconnected
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str()))
    {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish(mqtt_generic_topic, "hello world");
      // ... and resubscribe
      client.subscribe(listening_topic);
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void sendStates()
{
  // JSON config
  StaticJsonDocument<JSON_BUFFER_SIZE> doc;
  char buffer[JSON_BUFFER_SIZE];
  doc["Device"] = "ESP32";
  doc["Room"] = "01";
  JsonArray lampStates = doc.createNestedArray("Lamps");
  JsonArray servoStates = doc.createNestedArray("Servo");
  lampStates.add(states[0][0]);
  servoStates.add(states[1][0]);

  serializeJson(doc, buffer);
  client.publish("AutomataHomeStates", buffer);
}

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT); // Initialize the LED_BUILTIN pin as an output
  pinMode(D1, OUTPUT);          // Initialize the LED_BUILTIN pin as an output
  servo.attach(2);              // Initialize the D4 pin as an output to control the servo
  servo.write(0);               // Initialize the servo to 0 degrees
  Serial.begin(9600);
  setup_wifi();

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

// the loop function runs over and over again forever
void loop()
{
  // digitalWrite(LED_BUILTIN, LOW); // Turn the LED on (Note that LOW is the voltage level
  // digitalWrite(D1, LOW);          // Turn the LED on (Note that LOW is the voltage level

  // // but actually the LED is on; this is because
  // // it is active low on the ESP-01)
  // delay(1000);                     // Wait for a second
  // digitalWrite(LED_BUILTIN, HIGH); // Turn the LED off by making the voltage HIGH
  // digitalWrite(D1, HIGH);          // Turn the LED off by making the voltage HIGH
  // delay(1000);                     // Wait for two seconds (to demonstrate the active low LED)

  if (!client.connected())
  {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  if (now - lastMsg > 2000)
  {
    lastMsg = now;
    ++value;
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish("outTopic", msg);
  }

  if (now - lastMsg > 9999)
  {
    lastMsg = now;
    sendStates();
  }
}