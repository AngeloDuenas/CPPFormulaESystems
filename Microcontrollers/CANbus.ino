#include <SPI.h>
#include <mcp_can.h>

// Define Chip Select pin for MCP2515
#define CAN_CS_PIN 10

// Create MCP_CAN object
MCP_CAN CAN(CAN_CS_PIN); 

// Function to decode temperature from rxBuf[]
void getTemperature(const String& input, byte rxBuf[], byte len) {
  input.trim(); // Remove leading/trailing spaces

  if (input.startsWith("Board")) {
    // Gate Driver Board temperature (first two bytes)
    uint16_t tempRaw = (rxBuf[0] << 8) | rxBuf[1];
    float temperatureCelsius = tempRaw / 100.0;
    Serial.print("Gate Driver Board Temperature: ");
    Serial.print(temperatureCelsius);
    Serial.println(" °C");
  } else if (input.startsWith("Mod")) {
    char moduleLabel = input.charAt(input.indexOf('A') + 4); // Get module label ('A', 'B', etc.)
    byte moduleIndex = moduleLabel - 'A'; // Convert 'A' to 0, 'B' to 1, etc.

    // Calculate position in rxBuf[]
    byte index = 2 + (moduleIndex * 2); // Module A starts at byte 2
    if (index < len - 1) { // Ensure valid index
      uint16_t tempRaw = (rxBuf[index] << 8) | rxBuf[index + 1];
      float temperatureCelsius = tempRaw / 100.0;
      Serial.print("Module ");
      Serial.print(moduleLabel);
      Serial.print(" Temperature: ");
      Serial.print(temperatureCelsius);
      Serial.println(" °C");
    } else {
      Serial.println("Error: Invalid module index!");
    }
  } else {
    Serial.println("Error: Unrecognized input format!");
  }
}

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

    // Decode all temperatures
    Serial.println("Decoded Temperatures:");
    for (byte i = 0; i < len; i += 2) {
      if (i + 1 < len) {
        uint16_t tempRaw = (rxBuf[i] << 8) | rxBuf[i + 1];
        float temperatureCelsius = tempRaw / 100.0;

        if (i == 0) {
          Serial.print("Gate Driver Board: ");
        } else {
          char moduleLetter = 'A' + ((i / 2) - 1);
          Serial.print("Module ");
          Serial.print(moduleLetter);
          Serial.print(": ");
        }

        Serial.print(temperatureCelsius);
        Serial.println(" °C");
      }
    }
    Serial.println();

    // Check for user input in the Serial Monitor
    if (Serial.available() > 0) {
      // Read the input from Serial Monitor
      String userInput = Serial.readStringUntil('\n'); // Read until newline character
      Serial.print("Checking Temperature: ");
      Serial.println(userInput);

      // Process the user input
      getTemperature(userInput, rxBuf, len);
    }
  }

  delay(1000); // Wait for 1 second
}
