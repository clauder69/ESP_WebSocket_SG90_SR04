#include <ESP8266WiFiMulti.h>
#include <WebSocketsServer.h>
#include <ESP8266WebServer.h>
#include <Servo.h>
#define trigPin 4
#define echoPin 5
#define servoPin 13
Servo servo;
ESP8266WiFiMulti WiFiMulti;
ESP8266WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);
int clientID;
//
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  switch (type) {
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Disconnected!\n", num);
      break;
    case WStype_CONNECTED: {
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
        clientID = num;
        webSocket.sendTXT(num, "Connected");
      }
      break;
    case WStype_TEXT:
      Serial.printf("[%u] get Text: %s\n", num, payload);
      String rec = (char * )payload;
      int a = rec.toInt();
      servo.write(a);
      break;
  }
}
//
void wifiSetup() {
  Serial.println("Connecting");
  WiFiMulti.addAP("SSID", "PASS");
  while (WiFiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.printf("\nConnected to %s\n", WiFi.SSID().c_str());
  Serial.printf("IP address:%s\n", WiFi.localIP().toString().c_str());
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  server.on("/", []() {
    server.send(200, "text/html", "<head>  <script>  var connection = new WebSocket('ws://' + location.hostname + ':81/', ['arduino']);  connection.onopen = function() {    connection.send('Connect ' + new Date());   };  connection.onerror = function(error) {    console.log('WebSocket Error ', error);   };  connection.onmessage = function(e) {    console.log('Server: ', e.data);    d.value=e.data  };  </script> </head> <body> WebSocket<br><br> Servo:<input id='r' type='range' min='0' max='180' step='1' oninput='connection.send(this.value);'><br> Distancia: <input id='d' type='INPUT'> </body> </html>");
  });
  server.begin();
}
//
int HC_SR04 (void) {
  long duration;
  int distance;
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distance = duration * 0.034 / 2;
  return distance;
}
//
void setup() {
  Serial.begin(9600);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  servo.attach(servoPin);
  servo.write(90);
  wifiSetup();
}
//
int d = 0;
String texto;
long tempo = millis();
//
void loop() {
  webSocket.loop();
  server.handleClient();
//
  if (tempo < millis()) {
    tempo = millis() + 1000;
    d = HC_SR04();
    Serial.printf("Distancia em CM:%u\n", d);
    texto = String(d);
    webSocket.sendTXT(clientID, texto);
  }
}
