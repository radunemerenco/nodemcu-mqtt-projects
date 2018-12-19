/*
 Basic ESP8266 MQTT example

 This sketch demonstrates the capabilities of the pubsub library in combination
 with the ESP8266 board/library.

 It connects to an MQTT server then:
  - publishes "hello world" to the topic "outTopic" every two seconds
  - subscribes to the topic "inTopic", printing out any messages
    it receives. NB - it assumes the received payloads are strings not binary
  - If the first character of the topic "inTopic" is an 1, switch ON the ESP Led,
    else switch it off

 It will reconnect to the server if the connection is lost using a blocking
 reconnect function. See the 'mqtt_reconnect_nonblocking' example for how to
 achieve the same result without blocking the main loop.

 To install the ESP8266 board, (using Arduino 1.6.4+):
  - Add the following 3rd party board manager under "File -> Preferences -> Additional Boards Manager URLs":
       http://arduino.esp8266.com/stable/package_esp8266com_index.json
  - Open the "Tools -> Board -> Board Manager" and click install for the ESP8266"
  - Select your ESP8266 in "Tools -> Board"

*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager
#include <ArduinoJson.h>

// Update these with values suitable for your network.

const char* wifiManAp  = "AutoConnectAP";
const char* wifiManPw  = "password";

const char* mqtt_server = "broker.shiftr.io";
const char* mqttUser = "miner-assist";
const char* mqttPassword = "3a2ed873f926466c";

int relayPin = 13;

const char* STARTUP = "STARTUP";
const char* SHUTDOWN = "SHUTDOWN";
const char* HARD_RESTART = "HARD_RESTART";
const char* NODEMCU_RESET = "NODEMCU_RESET";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

void setup_wifi() {
  WiFiManager wifiManager;
  
  delay(10);
  
  wifiManager.autoConnect(wifiManAp, wifiManPw);

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("MAC address: ");
  Serial.println(WiFi.macAddress());
}


char* startUp() {
//  digitalWrite(relayPin, HIGH);
  digitalWrite(relayPin, LOW);
  delay(1000);
//  digitalWrite(relayPin, LOW);
  digitalWrite(relayPin, HIGH);
  return "Started Up";
}

char* shutDown() {
//  digitalWrite(relayPin, HIGH);
  digitalWrite(relayPin, LOW);
  delay(6000);
//  digitalWrite(relayPin, LOW);
  digitalWrite(relayPin, HIGH);
  return "Shutted Down";
}

char* reStart() {
  shutDown();
  delay(6000);
  startUp();
  return "Restarted";
}

char* mcuReset() {
  WiFi.disconnect();
  ESP.restart();
  return "MCU RESET. Reconnect to WiFi...";
}

char* combineCharArrays(char* arr1, char* arr2) {
  int resultLength = sizeof(arr1) + sizeof(arr2);
  char* result = (char*) malloc(resultLength * sizeof(char));
  for (int i = 0; i < sizeof(arr1); i++) {
    result[i] = arr1[i];
  }
  for (int i = 0; i < sizeof(arr2); i++) {
    result[sizeof(arr1) + i] = arr2[i];
  }
  return result;
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  char prefix[] = "PC Status: ";
  char* pcStatus;

  

  if (length > 0) {

    DynamicJsonBuffer jsonBuffer(200);
    JsonObject& root = jsonBuffer.parseObject(payload);

    // Test if parsing succeeds.
    if (!root.success()) {
      Serial.println("parseObject() failed");
      return;
    }

    const char* mac = root["mac"];
    const char* action = root["action"];
    if (strcmp(mac, WiFi.macAddress().c_str()) != 0) {
      return;
    }
    if (strcmp(action, NODEMCU_RESET) == 0) {
      pcStatus = mcuReset();
    }
    else {
      if (strcmp(action, STARTUP) == 0) pcStatus = startUp();
      else if (strcmp(action, SHUTDOWN) == 0) pcStatus = shutDown();
      else if (strcmp(action, HARD_RESTART) == 0) pcStatus = reStart();
      else pcStatus = "[SAME_STATE]";
    }
    char message[50];
    strcpy(message, prefix);
    strcat(message, pcStatus);
    client.publish("outTopic", message);
  }

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(WiFi.macAddress().c_str(), mqttUser, mqttPassword)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      char message[50];
      strcpy(message, WiFi.macAddress().c_str());
      strcat(message, " - hello world");
      client.publish("outTopic", message);
      // ... and resubscribe
      client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  pinMode(relayPin, OUTPUT);     // Initialize the relayPin pin as an output
  digitalWrite(relayPin, HIGH);
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 5000) {
    lastMsg = now;
    ++value;
    snprintf (msg, 100, "%s - Ping #%ld", WiFi.macAddress().c_str(), value);
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish("outTopic", msg);
  }
}
