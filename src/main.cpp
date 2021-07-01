#include <Arduino.h>
#include <ArduinoWebsockets.h>
#include <ESP8266WiFi.h>
#include <secrets.h>

using namespace websockets;

const byte maxClients = 4;

WebsocketsClient clients[maxClients];
WebsocketsServer server;

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);

  WiFi.begin(SECRET_SSID, SECRET_PASS);

  for (int i = 0; i < 15 && WiFi.status() != WL_CONNECTED; i++) {
    Serial.print("=-=-");
    delay(1000);
  }

  Serial.println(">>");
  IPAddress ip(192, 168, 1, 32);
  IPAddress gateway(192, 168, 1, 1);

  Serial.print("Setting static ip to : ");
  Serial.println(ip);
  IPAddress subnet(255, 255, 255, 0);
  WiFi.config(ip, gateway, subnet);

  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());

  server.listen(6969);

  Serial.print("Listening on ws://");
  Serial.print(WiFi.localIP());
  Serial.println(":6969/");
}
void handleMessage(WebsocketsClient &client, WebsocketsMessage message) {
  auto data = message.data();

  Serial.print("Got Message: ");
  Serial.println(data);

  client.send("Echo: " + data);
}

void handleEvent(WebsocketsClient &client, WebsocketsEvent event, String data) {
  if (event == WebsocketsEvent::ConnectionClosed) {
    Serial.println("Connection closed");
  }
}

int8_t getFreeClientIndex() {
  for (byte i = 0; i < maxClients; i++) {
    if (!clients[i].available())
      return i;
  }
  return -1;
}

void listenForClients() {
  if (server.poll()) {
    int8_t freeIndex = getFreeClientIndex();
    if (freeIndex >= 0) {
      WebsocketsClient newClient = server.accept();
      Serial.printf("Accepted new websockets client at index %d\n", freeIndex);
      newClient.onMessage(handleMessage);
      newClient.onEvent(handleEvent);
      newClient.send("Hello from Teensy");
      clients[freeIndex] = newClient;
    }
  }
}

void pollClients() {
  for (byte i = 0; i < maxClients; i++) {
    clients[i].poll();
  }
}

void loop() {
  listenForClients();
  pollClients();
}