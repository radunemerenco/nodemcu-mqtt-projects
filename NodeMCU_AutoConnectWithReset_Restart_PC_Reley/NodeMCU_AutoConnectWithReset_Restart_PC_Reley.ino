#include <FS.h>                   //this needs to be first, or it all crashes and burns...

#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino

//needed for library
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager


int ledPin = 13; // GPIO13

ESP8266WebServer server(80);

void setup() {
  
  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;
  
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println();


  pinMode(ledPin, OUTPUT);
//  digitalWrite(ledPin, LOW);
  digitalWrite(ledPin, HIGH);

  //exit after config instead of connecting
  wifiManager.setBreakAfterConfig(true);

  //reset settings - for testing
//  wifiManager.resetSettings();


  //tries to connect to last known settings
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP" with password "password"
  //and goes into a blocking loop awaiting configuration
  if (!wifiManager.autoConnect("AutoConnectAP", "password")) {
    Serial.println("failed to connect, we should reset as see if it connects");
    delay(3000);
    ESP.reset();
    delay(5000);
  }

  //if you get here you have connected to the WiFi
  Serial.println("WiFi connected");
  
  // Start the server
  server.begin();
  Serial.println("Server started");
 
  // Print the IP address
  Serial.print("Use this URL to connect: ");
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.println("/");
  Serial.println("");
 
}



String startUp() {
//  digitalWrite(ledPin, HIGH);
  digitalWrite(ledPin, LOW);
  delay(1000);
//  digitalWrite(ledPin, LOW);
  digitalWrite(ledPin, HIGH);
  return "Started Up";
}

String shutDown() {
//  digitalWrite(ledPin, HIGH);
  digitalWrite(ledPin, LOW);
  delay(6000);
//  digitalWrite(ledPin, LOW);
  digitalWrite(ledPin, HIGH);
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

  if (request.indexOf("/MCU=RESET") != -1) {
    WiFi.disconnect();
    ESP.restart();
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
  client.println("<a href=\"/MCU=RESET\"\"><button>MCU Reset </button></a><br />");  
  client.println("</html>");
 
  delay(1);
  Serial.println("Client disonnected");

  Serial.println("");
 
}
