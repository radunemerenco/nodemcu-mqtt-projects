#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager

const char* apSSID = "AutoConnectAP";
const char* apPASS = "password";

ESP8266WebServer server(80);

const int relayPin = 13;

const String STARTUP = "STARTUP";
const String SHUTDOWN = "SHUTDOWN";
const String HARD_RESTART = "HARD_RESTART";

// Return the response
String getHtmlContent(String message) {
  String html = "<!DOCTYPE HTML>";
  html += "<html lang=\"en\">";
  html += "<head>";
  html += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1, shrink-to-fit=no\">";
  html += "<link rel=\"stylesheet\" href=\"https://stackpath.bootstrapcdn.com/bootstrap/4.1.1/css/bootstrap.min.css\" integrity=\"sha384-WskhaSGFgHYWDcbwN70/dfYBj47jz9qbsMId/iRN3ewGhXQFZCSftd1LZCfmhktB\" crossorigin=\"anonymous\">";
  html += "</head>";
  html += "<body>";


  html += "<div class=\"jumbotron text-center\">";
  html += "<h1 class=\"display-5\">" + message +"</h1>";
  html += "<hr class=\"my-4\">";
  
  html += "<h4>Rig functions</h4>";
  
  html += "<div class=\"btn-group\" role=\"group\" aria-label=\"Rig Functions\">";
  html += "<a class=\"btn btn-success\" href=\"/?func=" + STARTUP + "\" role=\"button\">" + STARTUP + "</a>";
  html += "<a class=\"btn btn-danger\" href=\"/?func=" + SHUTDOWN + "\" role=\"button\">" + SHUTDOWN + "</a>";
  html += "</div>";
  html += "<br />";
  html += "<a class=\"btn btn-warning mt-2\" href=\"/?func=" + HARD_RESTART + "\" role=\"button\">" + HARD_RESTART + "</a>";
  
  html += "<hr class=\"my-4\">";

  html += "<h4>NodeMCU Functions</h4>";
  html += "<a class=\"btn btn-primary\" href=\"/?MCU=RESET\" role=\"button\">RESET WIFI (Do not press!!!)</a>";
  html += "</div>";
  
  html += "<script src=\"https://code.jquery.com/jquery-3.3.1.slim.min.js\" integrity=\"sha384-q8i/X+965DzO0rT7abK41JStQIAqVgRVzpbzo5smXKp4YfRvH+8abtTE1Pi6jizo\" crossorigin=\"anonymous\"></script>";
  html += "<script src=\"https://cdnjs.cloudflare.com/ajax/libs/popper.js/1.14.3/umd/popper.min.js\" integrity=\"sha384-ZMP7rVo3mIykV+2+9J3UJ46jBk0WLaUAdn689aCwoqbBJiSnjAK/l8WvCWPIPm49\" crossorigin=\"anonymous\"></script>";
  html += "<script src=\"https://stackpath.bootstrapcdn.com/bootstrap/4.1.1/js/bootstrap.min.js\" integrity=\"sha384-smHYKdLADwkXOn1EmN1qk/HfnUcbVRZyYmZ4qpPea6sjB/pTJ0euyQp0Mk8ck+5T\" crossorigin=\"anonymous\"></script>";

  html += "</body>";
  html += "</html>";
  
  return html;
}


String startUp() {
//  digitalWrite(relayPin, HIGH);
  digitalWrite(relayPin, LOW);
  delay(1000);
//  digitalWrite(relayPin, LOW);
  digitalWrite(relayPin, HIGH);
  return "Started Up";
}

String shutDown() {
//  digitalWrite(relayPin, HIGH);
  digitalWrite(relayPin, LOW);
  delay(6000);
//  digitalWrite(relayPin, LOW);
  digitalWrite(relayPin, HIGH);
  return "Shutted Down";
}

String reStart() {
  shutDown();
  delay(6000);
  startUp();
  return "Restarted";
}

String mcuReset() {
  WiFi.disconnect();
  ESP.restart();
  return "MCU RESET. Reconnect to WiFi...";
}

void handleRoot() {

  String pcStatus = "PC Status: ";

  if (sizeof(server.args()) > 0) {
    if (server.argName(0) == "func") {
      String argValue = server.arg(0);
      if (argValue == STARTUP) pcStatus += startUp();
      else if (argValue == SHUTDOWN) pcStatus += shutDown();
      else if (argValue == HARD_RESTART) pcStatus += reStart();
      else pcStatus += "[SAME_STATE]";
    }
    else if (server.argName(0) == "MCU" && server.arg(0) == "RESET") {
      pcStatus = mcuReset();
    }
    else pcStatus += "[SAME_STATE]";
  }
  else pcStatus += "[SAME_STATE]";
  server.send(200, "text/html", getHtmlContent(pcStatus));
}

void handleNotFound(){
//  digitalWrite(relayPin, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
//  digitalWrite(relayPin, 0);
}


void setup(void){
  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;
  
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, HIGH);
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);

  
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
  

  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);

  server.on("/inline", [](){
    server.send(200, "text/plain", "this works as well");
  });

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
}

void loop(void){
  server.handleClient();
}
