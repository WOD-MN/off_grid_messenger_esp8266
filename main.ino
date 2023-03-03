#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <WiFiServer.h>
#include <ESP8266WebServer.h>

const char* ssid = "my_off_grid_network";
const char* password = "my_off_grid_password";
IPAddress apIP(192, 168, 4, 1);

ESP8266WebServer server(80);
WiFiClient messagingClient;

void setup() {
  Serial.begin(9600);
  delay(10);

  // Start the Wi-Fi access point
  WiFi.softAP(ssid, password);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  delay(100);

  // Print the access point IP address
  Serial.print("Access Point IP address: ");
  Serial.println(WiFi.softAPIP());

  // Start the server
  server.on("/", handleRoot);
  server.on("/send_message", handleSendMessage);
  server.begin();
  Serial.println("Server started");
}

void loop() {
  // Check if a device is connected to the access point
  if (WiFi.softAPgetStationNum() > 0) {
    // Open the IP address in the default browser
    Serial.println("Device connected, opening IP address...");
    delay(100);
    String url = "http://" + WiFi.softAPIP().toString();
    Serial.println(url);
    delay(100);
    WiFi.softAPdisconnect(true);
    WiFi.mode(WIFI_STA);
    delay(100);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(100);
      Serial.println("Connecting to network...");
    }
    Serial.println("Connected to network");
    delay(100);
    ESP8266WebServer server(80);
    server.on("/", handleRoot);
    server.on("/send_message", handleSendMessage);
    server.begin();
    delay(100);
    ESP8266WebServer::send(200, "text/html", "<html><body><h1>Connected to Access Point</h1></body></html>");
  }

  // Handle incoming client requests
  server.handleClient();
}

void handleRoot() {
  String html = "<html><body><form action=\"/send_message\" method=\"get\">";
  html += "Sender: <input type=\"text\" name=\"sender\"><br>";
  html += "Receiver: <input type=\"text\" name=\"receiver\"><br>";
  html += "Message: <input type=\"text\" name=\"message\"><br>";
  html += "<input type=\"submit\" value=\"Send\">";
  html += "</form></body></html>";
  server.send(200, "text/html", html);
}

void handleSendMessage() {
  String senderName = server.arg("sender");
  String receiverName = server.arg("receiver");
  String messageText = server.arg("message");

  // Send the message to the messaging server
  if (messagingClient.connect("192.168.4.1", 8080)) {
    messagingClient.println(senderName + "," + receiverName + "," + messageText);
    Serial.println("Message sent: " + senderName + "," + receiverName + "," + messageText);
    messagingClient.stop();
  } else {
    Serial.println("Error connecting to messaging server");
  }

  // Send the response
  String html = "<html><body>Message sent!</body></html>";
  server.send(200, "text/html", html);
}
