#include <SPI.h>
#include <mcp_can.h>

// Define Chip Select pin for MCP2515
#define CAN_CS_PIN 10

// Create MCP_CAN object
MCP_CAN CAN(CAN_CS_PIN); 

void setup() {
  Serial.begin(115200); // Initialize serial monitor

  // Initialize MCP2515 at 250kbps baud rate, 8MHz clock 
  if (CAN.begin(MCP_ANY, CAN_250KBPS, MCP_8MHZ) == CAN_OK) {
    Serial.println("CAN MCP2515 initialized successfully");
  } else {
    Serial.println("Error initializing CAN MCP2515");
    while (1); // Halt if CAN initialization fails
  }

  // Set the MCP2515 in normal operating mode
  CAN.setMode(MCP_NORMAL);

  Serial.println("CAN MCP2515 is in normal mode");
}

void loop() {
  // Check if a message has been received
  if (CAN.checkReceive() == CAN_MSGAVAIL) { 
    unsigned long rxId;
    byte len = 0;
    byte rxBuf[8];

    // Read the received CAN message
    CAN.readMsgBuf(&rxId, &len, rxBuf);

    // Display the received message ID
    Serial.print("Received message with ID: 0x");
    Serial.println(rxId, HEX);

    // Display the raw CAN data
    Serial.print("Data: ");
    for (byte i = 0; i < len; i++) {
      Serial.print(rxBuf[i], HEX);
      Serial.print(" ");
    }
    Serial.println();

    // Decode and display the temperatures
    Serial.println("Decoded Temperatures:"); // Converts hex into temperature number
    for (byte i = 0; i < len; i += 2) {
      if (i + 1 < len) {
        uint16_t tempRaw = (rxBuf[i] << 8) | rxBuf[i + 1];
        float temperatureCelsius = tempRaw / 100.0;

        if (i == 0) {
          Serial.print("Gate Driver Board: "); // Makes Gate Driver Board statement
        } else {
          char moduleLabel = 'A' + ((i / 2) - 1);
          Serial.print("Module "); // Makes Module statement
          Serial.print(moduleLabel);
          Serial.print(": ");
        }

        Serial.print(temperatureCelsius); // Gives temperature in celsius
        Serial.println(" °C");
      }
    }
    Serial.println();
  }

  delay(1000); // Wait for 1 second
}
