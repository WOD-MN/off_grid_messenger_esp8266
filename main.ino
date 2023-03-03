#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <WiFiServer.h>

const char* ssid = "my_off_grid_network";
const char* password = "my_off_grid_password";
IPAddress apIP(192, 168, 4, 1);

WiFiServer server(80);
WiFiClient messagingClient;

void setup() {
  Serial.begin(9600);
  delay(10);

  // Start the Wi-Fi access point
  WiFi.softAP(ssid, password);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  delay(100);

  // Start the server
  server.begin();
  Serial.println("Server started");
}

void loop() {
  // Wait for a new client to connect
  WiFiClient client = server.available();
  if (client) {
    Serial.println("New client connected");

    // Read the HTTP request
    String request = "";
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        if (c == '\n') {
          if (request.indexOf("GET /send_message") != -1) {
            // Handle the send_message request
            String senderName = client.readStringUntil('&');
            senderName.remove(0, senderName.indexOf('=') + 1);
            String receiverName = client.readStringUntil('&');
            receiverName.remove(0, receiverName.indexOf('=') + 1);
            String messageText = client.readStringUntil('\n');
            messageText.remove(0, messageText.indexOf('=') + 1);

            // Send the message to the messaging server
            if (messagingClient.connect("192.168.4.1", 8080)) {
              messagingClient.println(senderName + "," + receiverName + "," + messageText);
              Serial.println("Message sent: " + senderName + "," + receiverName + "," + messageText);
              messagingClient.stop();
            } else {
              Serial.println("Error connecting to messaging server");
            }

            // Send the response
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: text/html");
            client.println("");
            client.println("<html><body>Message sent!</body></html>");
          } else {
            // Handle other requests (e.g. serving the web page)
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: text/html");
            client.println("");
            client.println("<html><body><form action=\"/send_message\" method=\"get\">");
            client.println("Sender: <input type=\"text\" name=\"sender\"><br>");
            client.println("Receiver: <input type=\"text\" name=\"receiver\"><br>");
            client.println("Message: <input type=\"text\" name=\"message\"><br>");
            client.println("<input type=\"submit\" value=\"Send\">");
            client.println("</form></body></html>");
          }
          break;
        } else {
          request += c;
        }
      }
    }

    // Disconnect the client
    client.stop();
    Serial.println("Client disconnected");
  }
}

