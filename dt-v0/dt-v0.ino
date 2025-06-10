#include <WiFi.h>
#include <SPIFFS.h>
#include <ESPAsyncWebServer.h>
#include <WebSocketsServer.h>
#define trigPin 4    // ESP32 GPIO4 to Trig
#define echoPin 3   // ESP32 GPIO3 to Echo
//constant
const char *ssid = "ESP32-AP";
const char *password = "LetMeInPls";
const char *msg_toggle_led = "toggleLED";
const char *msg_get_led = "getLEDstate";
const int dns_port = 53;
const int http_port = 80;
const int ws_port = 1337;


//globals
AsyncWebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(1337);
char msg_buf[50];  //large enough for a JSON message




/******************************************************
 * functions
 */


//callback: receiving any websocket message
void onWebSocketEvent(uint8_t client_num,
                      WStype_t type,
                      uint8_t * payload,
                      size_t length) {
  
  //figure out type of websocket event
  switch(type) {
    
    //client has disconnected
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Disconnected!\n", client_num);
      break;

    //new client has connected 
    case WStype_CONNECTED:
      {
        IPAddress ip = webSocket.remoteIP(client_num);
        Serial.printf("[%u] Connection from ", client_num);
        Serial.println(ip.toString());
        
      }

     //handle text messages from client 
     case WStype_TEXT:

      //print out raw message
      Serial.printf("[%u] Recieved text: %s\n", client_num, payload);

      break;


    //for everything else: do nothing
    case WStype_BIN:
    case WStype_ERROR:
    case WStype_FRAGMENT_TEXT_START:
    case WStype_FRAGMENT_BIN_START:
    case WStype_FRAGMENT:
    case WStype_FRAGMENT_FIN:
    default:        
      break;
  }

}


//Callback: send homepage
void onIndexRequest (AsyncWebServerRequest *request){
  IPAddress remote_ip = request->client()->remoteIP();
  Serial.println("[" + remote_ip.toString() +
                  " ] HTTP GET request of " + request->url());
  request->send(SPIFFS, "/index.html", "text/html");
}

//Callback: send style sheet
void onCSSRequest (AsyncWebServerRequest *request){
  IPAddress remote_ip = request->client()->remoteIP();
  Serial.println("[" + remote_ip.toString() +
                 "] HTTP GET request of" + request->url());
  request->send(SPIFFS, "/style.css", "text/css");
}

//Callback: send 404 if requested file does not exist
void onPageNotFound (AsyncWebServerRequest *request){
  IPAddress remote_ip = request->client()->remoteIP();
  Serial.println("[" + remote_ip.toString() +
                 "] HTTP GET request of" + request->url());
  request->send(404, "text/plain", "Not Found");
}



/************************************************************
 * Main
 */



void setup() {
  // init dist sensor
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);


  //start serial port
  Serial.begin(115200);

  //make sure we can read the file system
  if( !SPIFFS.begin(true)){
      Serial.println("error mounting SPIFFS");
      while(1);
  }

  //start access point
  WiFi.softAP(ssid, password);

  //print our access point
  Serial.println();
  Serial.println("AP Running");
  Serial.print("My IP Address: ");
  Serial.println(WiFi.softAPIP());

  //on HTTP reuqest for root, provide index.html file
  server.on("/", HTTP_GET, onIndexRequest);

  //on HTTP reuqest for style sheet, provide style.css
  server.on("/style.css", HTTP_GET, onCSSRequest);

  //handle request for pages not found
  server.onNotFound(onPageNotFound);

  //start web server
  server.begin();

  //start websocket server and assign callback
  webSocket.begin();
  webSocket.onEvent(onWebSocketEvent);
}

void loop() {
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    long duration = pulseIn(echoPin, HIGH);
    int distance = duration * 0.034 / 2;
  
    Serial.print("Distance: ");
    Serial.print(distance);
    Serial.println(" cm");

    //send distance to websocket clients as JSON
    snprintf(msg_buf, sizeof(msg_buf), "{\"distance\":%d}", distance);
    webSocket.broadcastTXT(msg_buf);

    delay(100);
    //look for and  handle websocket data
    webSocket.loop();

}
