#include <WiFi.h>

//http get request for webpage
httpd_uri_t uri_get = {
    .uri = "/",  //the path
    .method = HTTP_GET,  //the http method
    .handler = get_req_handler , //pointer to function that will handle requests to this URI
    .user_ctx = NULL //user context data
};

//get request at /ws and handles WebSocket connections
httpd_uri_t ws = {
  .uri = "/ws",
  .method = HTTP_GET,
  .handler = handle_ws_req,
  .user_ctx = NULL,
  .is_websocket = true //marks as websocket handler
};


const char* ssid = "Kamryn";
const char* password = "zj658hbcnv8tt";

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println(WiFi.localIP());

  //registering handlers
  if (httpd_start(&server, &config) == ESP_OK) {
    httpd_register_uri_handler(server, &uri_get);
    httpd_register_uri_handler(server, &ws);
  }
}




void loop() {
  // put your main code here, to run repeatedly:

}
