#include <Arduino.h>
#include <WiFi.h>
#include <HCSR04.h>

// Modbus server TCP
#include "ModbusServerWiFi.h"

#ifndef MY_SSID
#define MY_SSID "HUAWEI-2.4G-tgnh"
#endif
#ifndef MY_PASS
#define MY_PASS "UFMDZgVg"
#endif

char ssid[] = MY_SSID;                     // SSID and ...
char pass[] = MY_PASS;                     // password for the WiFi network used

UltraSonicDistanceSensor distanceSensor(18, 5);  // Initialize sensor that uses digital pins 18 and 5.
int i = 0;
float SensorMed,auxMed,AuxReadFloat = 0;
uint16_t ReadMed,ReadFloat = 0;
// Create server
ModbusServerWiFi MBserver;

uint16_t memo[32];                     // Test server memory: 32 words

// Server function to handle FC 0x03 and 0x04
ModbusMessage FC03(ModbusMessage request) {
  ModbusMessage response;      // The Modbus message we are going to give back
  uint16_t addr = 0;           // Start address
  uint16_t words = 0;          // # of words requested
  request.get(2, addr);        // read address from request
  request.get(4, words);       // read # of words from request

  // Address overflow?
  if ((addr + words) > 20) {
    // Yes - send respective error response
    response.setError(request.getServerID(), request.getFunctionCode(), ILLEGAL_DATA_ADDRESS);
  }
  // Set up response
  response.add(request.getServerID(), request.getFunctionCode(), (uint8_t)(words * 2));
  // Request for FC 0x03?
  if (request.getFunctionCode() == READ_HOLD_REGISTER) {
    response.add(ReadMed); // Dado do sensor
    response.add(ReadFloat); // Dado do sensor

    Serial.print(distanceSensor.measureDistanceCm());
    Serial.print("--");
    Serial.print(ReadMed);
    Serial.print("--");
    Serial.print(AuxReadFloat);
    Serial.print("--");
    Serial.println(ReadFloat);
  } else {
    // No, this is for FC 0x04. Response is random
    for (uint8_t i = 0; i < words; ++i) {
      // send increasing data values
      response.add((uint16_t)random(1, 65535));
    }
  }
  // Send response back
  return response;
}

// Setup() - initialization happens here
void setup() {
// Init Serial monitor
  Serial.begin(115200);
  while (!Serial) {}
  Serial.println("__ OK __");

// Connect to WiFi
  WiFi.begin(ssid, pass);
  delay(200);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(". ");
    delay(500);
  }
  IPAddress wIP = WiFi.localIP();
  Serial.printf("WIFi IP address: %u.%u.%u.%u\n", wIP[0], wIP[1], wIP[2], wIP[3]);

// Set up test memory
  for (uint16_t i = 0; i < 32; ++i) {
    memo[i] = (i * 2) << 8 | ((i * 2) + 1);
  }

  MBserver.registerWorker(1, READ_HOLD_REGISTER, &FC03);      // FC=03 for serverID=1
  MBserver.registerWorker(1, READ_INPUT_REGISTER, &FC03);     // FC=04 for serverID=1
  MBserver.start(502, 1, 20000);
}

// loop() - nothing done here today!
void loop() {
  static unsigned long lastMillis = 0;
  if (millis() - lastMillis > 10000) {
    lastMillis = millis();
    Serial.printf("free heap: %d\n", ESP.getFreeHeap());
  }
    // DEBUG
      for(i = 0; i < 100;i++)
    {
      SensorMed += distanceSensor.measureDistanceCm()/100;
    }

    auxMed = SensorMed;
    ReadMed = uint16_t(auxMed); // Inteiro
    AuxReadFloat = round(ReadMed - auxMed);  //Flutuante
    ReadFloat = uint16_t(AuxReadFloat);
    SensorMed = 0;

    // Serial.print(distanceSensor.measureDistanceCm());
    // Serial.print("--");
    // Serial.print(ReadMed);
    // Serial.print("--");
    // Serial.println(ReadFloat);
}