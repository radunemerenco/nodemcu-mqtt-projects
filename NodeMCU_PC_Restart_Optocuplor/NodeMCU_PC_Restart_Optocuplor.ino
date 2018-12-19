#include <ESP8266WiFi.h>
 
const char* hostName = "NodeMCU_Rig1_Optocuplor";
const char* ssid = "HUAWEI-B310-6111";
const char* password = "6DFQEE37E69";
 
int ledPin = 13; // GPIO13
WiFiServer server(80);
 
void setup() {
  Serial.begin(115200);
  delay(10);
 
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
//  digitalWrite(ledPin, HIGH);
 
  // Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.hostname(hostName);
  WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
 
  // Start the server
  server.begin();
  Serial.println("Server started");
 
  // Print the IP address
  Serial.print("Use this URL to connect: ");
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.println("/");
  Serial.print("Host Name: ");
  Serial.println(hostName);
  Serial.println("");
 
}

String startUp() {
  digitalWrite(ledPin, HIGH);
//  digitalWrite(ledPin, LOW);
  delay(1000);
  digitalWrite(ledPin, LOW);
//  digitalWrite(ledPin, HIGH);
  return "Started Up";
}

String shutDown() {
  digitalWrite(ledPin, HIGH);
//  digitalWrite(ledPin, LOW);
  delay(6000);
  digitalWrite(ledPin, LOW);
//  digitalWrite(ledPin, HIGH);
  return "Shutted Down";
}

String reStart() {
  shutDown();
  delay(6000);
  startUp();
  return "Restarted";
}

 
void loop() {
  // Check if a client has connected
  WiFiClient client = server.available();
  if (!client) {
    return;
  }
 
  // Wait until the client sends some data
  Serial.println("new client");
  while(!client.available()){
    delay(1);
  }
 
  // Read the first line of the request
  String request = client.readStringUntil('\r');
  Serial.println(request);
  client.flush();

  String result = "[SAME STATE]";
  // Match the request
  if (request.indexOf("/FUNC=STARTUP") != -1)  {
    result = startUp();
  } 
  if (request.indexOf("/FUNC=SHUTDOWN") != -1)  {
    result = shutDown();
  }
  if (request.indexOf("/FUNC=RESTART") != -1)  {
    result = reStart();
  }
 
  // Return the response
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println(""); //  do not forget this one
  client.println("<!DOCTYPE HTML>");
  client.println("<html>");
 
  client.print("The PC is now: ");
  client.print(result);
 
  client.println("<br><br>");
  client.println("<a href=\"/FUNC=STARTUP\"\"><button>Turn On </button></a>");
  client.println("<a href=\"/FUNC=SHUTDOWN\"\"><button>Turn Off </button></a>"); 
  client.println("<a href=\"/FUNC=RESTART\"\"><button>Restart </button></a><br />");  
  client.println("</html>");
 
  delay(1);
  Serial.println("Client disonnected");

  Serial.println("");
 
}
 
