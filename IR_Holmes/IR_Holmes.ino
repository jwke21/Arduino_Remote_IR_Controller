#include <IRremote.hpp>
#include <SPI.h>
#include <Ethernet.h>

#ifndef IR_RECEIVE_PIN
#define IR_RECEIVE_PIN 2 // The pin the IR receiver is connected to
#endif
#ifndef IR_SEND_PIN
#define IR_SEND_PIN 3 // The pin the IR sender is connected to
#endif
#ifndef PORT
#define PORT 50500 // Arduino will listen for packets on this port
#endif
#define MAX_BUF_SIZE 4096

void receiveIR();
void printKeyValue(IRData irData);
void iRFunctions();
void sendHTTPSuccessResponse(EthernetClient client);
void sendHTMLPage(EthernetClient client);

// Signal data transmitted to fan
const uint16_t rawData[23] = {1230,420, 1230,470, 380,1220, 1280,370, 1280,420, 380,1270, 380,1270, 380,1270, 430,1220, 430,1220, 1230,420, 430};
// Ethernet shield's MAC address (sticker on bottom of device)
const byte mac[] = { 0xA8, 0x61, 0x0A, 0xAE, 0x95, 0xBE };
// Buffer to read client's HTTP method field
char httpMethodBuf[5] = { 0 };

EthernetServer server = EthernetServer(PORT); // Initializes arduino as 

void setup() {
  // Set data rate to 9600 baud (i.e. bits per second) for serial data transmission
  Serial.begin(9600);
  while (!Serial) {} // Wait for serial port to connect
  // Start the IR receiver
  IrReceiver.begin(IR_RECEIVE_PIN, DISABLE_LED_FEEDBACK);
  // Start the IR transmitter
  IrSender.begin(IR_SEND_PIN, ENABLE_LED_FEEDBACK);
  // Start the Ethernet shield
  Ethernet.begin(mac); // IP address will be requested from router (via DHCP)
  // Start the server
  server.begin();
  Serial.print("Server initialized at: ");
  Serial.println(Ethernet.localIP());
}

void loop() {
  // Listen for incoming clients
  EthernetClient client = server.available();
  if (client) {
    Serial.print("Client connected from: ");
    Serial.print(client.remoteIP());
    Serial.print(":");
    Serial.println(client.remotePort());

    boolean currentLineIsBlank = true;
    int i = 0;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        // Read the HTTP command into buffer
        if (i < 4) {
          httpMethodBuf[i++] = c;
        }
        if (c == '\n' && currentLineIsBlank) {
          Serial.println(httpMethodBuf);
          // If the HTTP method was POST, then send IR signal
          if (strcmp(httpMethodBuf, "POST") == 0) {
            iRFunctions();
          }
          // Send HTTP response to client
          sendHTTPSuccessResponse(client);
          sendHTMLPage(client);
          break;
        }
        if (c == '\n') {
          currentLineIsBlank = true;
        } else if (c != '\r') {
          currentLineIsBlank = false;
        }
      }
    }
    delay(1);
    client.stop(); // Close connection
    Serial.println("Client disconnected");
  }
}

void sendHTMLPage(EthernetClient client) {
  client.println("<!DOCTYPE html>");
  client.println("<html>");
  client.println("<head>");
  client.println("<title>Fan Power Control</title>\r\n");
  client.println("<style>");
  client.println(".container{ height: 200px; position: relative;}");
  client.println(".center{ margin: 0; position: absolute; top: 50%; left: 50%; -ms-transform: translate(-50%, -50%); transform: translate(-50%, -50%); }");
  client.println("</style>");
  client.println("</head>");
  client.println("<body>");
  client.println("<div class='container'>");
  client.println("<div class='center'>");
  client.println("    <form action='' method='post'>");
  client.println("        <input type='submit' value='Fan Power' />");
  client.println("    </form>");
  client.println("</div>");
  client.println("</div>");
  client.println("</body>");
  client.println("</html>\r\n");
}

void sendHTTPSuccessResponse(EthernetClient client) {
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("Connection: close");
  client.println();
}

void iRFunctions() {
  // Send the code
  Serial.println("Transmitting IR signal");
  // IrSender.sendRaw(rawTicks, sizeof(rawTicks) / sizeof(rawTicks[0]), NEC_KHZ);
  IrSender.sendRaw(rawData, sizeof(rawData) / sizeof(rawData[0]), NEC_KHZ);
  
  if (IrReceiver.decode()) {
    // Print short summary of receieved data
    receiveIR();
  }
}

void receiveIR() {
  // Print the key value
  printKeyValue(IrReceiver.decodedIRData);
  IrReceiver.resume();
}

void printKeyValue(IRData irData) {
  switch (irData.decodedRawData) {
    case 0x143226DB:
      Serial.println("Power/Speed");
      break;
    case 0xE0984BB6:
      Serial.println("Timer Control");
      break;
    case 0x39D41DC6:
      Serial.println("Oscillation Control");
      break;
    case 0x371A3C86:
      Serial.println("Natural Breeze/Sleep Breeze Mode");
      break;
    default:
      // Any other junk value
      // IrReceiver.printActiveIRProtocols(&Serial);
      Serial.println("Unknown value");
  }
}
