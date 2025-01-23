#include <SPI.h>
#include <mcp_can.h>

#define CAN_CS_PIN 10

MCP_CAN CAN(CAN_CS_PIN); 

void getTemperature(const String& input, byte rxBuf[], byte len) {
  input.trim();

  if (input.startsWith("Board")) {
    uint16_t tempRaw = (rxBuf[0] << 8) | rxBuf[1];
    float temperatureCelsius = tempRaw / 100.0;
    Serial.print("Gate Driver Board Temperature: ");
    Serial.print(temperatureCelsius);
    Serial.println(" °C");
  } else if (input.startsWith("Mod")) {
    char moduleLabel = input.charAt(input.indexOf('A') + 4);
    byte moduleIndex = moduleLabel - 'A';

    byte index = 2 + (moduleIndex * 2);
    if (index < len - 1) {
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
  Serial.begin(115200);

  if (CAN.begin(MCP_ANY, CAN_250KBPS, MCP_8MHZ) == CAN_OK) {
    Serial.println("CAN MCP2515 initialized successfully");
  } else {
    Serial.println("Error initializing CAN MCP2515");
    while (1);
  }

  CAN.setMode(MCP_NORMAL);

  Serial.println("CAN MCP2515 is in normal mode");
}

void loop() {
  if (CAN.checkReceive() == CAN_MSGAVAIL) { 
    unsigned long rxId;
    byte len = 0;
    byte rxBuf[8];

    CAN.readMsgBuf(&rxId, &len, rxBuf);

    Serial.print("Received message with ID: 0x");
    Serial.println(rxId, HEX);

    Serial.print("Data: ");
    for (byte i = 0; i < len; i++) {
      Serial.print(rxBuf[i], HEX);
      Serial.print(" ");
    }
    Serial.println();

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

    if (Serial.available() > 0) {
      String userInput = Serial.readStringUntil('\n');
      Serial.print("Checking Temperature: ");
      Serial.println(userInput);

      getTemperature(userInput, rxBuf, len);
    }
  }

  delay(1000);
}
