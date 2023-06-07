// =================================================================================================
// eModbus: Copyright 2020 by Michael Harwerth, Bert Melis and the contributors to ModbusClient
//               MIT license - see license.md for details
// =================================================================================================
// Includes: <Arduino.h> for Serial etc., WiFi.h for WiFi support
#include <Arduino.h>
#include <WiFi.h>

// Modbus server TCP
#include "ModbusServerWiFi.h"

#ifndef MY_SSID
#define MY_SSID "SENAI_IOT"
#endif
#ifndef MY_PASS
#define MY_PASS "V3]u6Nm=b~1H^LG[z<D+lO"
#endif

char ssid[] = MY_SSID;                     // SSID and ...
char pass[] = MY_PASS;                     // password for the WiFi network used

int PinTrigger = 5; // Pino usado para disparar os pulsos do sensor
int PinEcho = 18; // pino usado para ler a saida do sensor

float TempoEcho = 0;
const float VelocidadeSom_mporus = 0.000340; // em metros por microsegundo

unsigned long timer_i = 0;
unsigned long timer_f = 0;

int i = 0;
float SensorMed,ReadMed,ReadFloat,auxMed = 0;

// Create server
ModbusServerWiFi MBserver;

uint16_t memo[32]; // Test server memory: 32 words


float CalculaDistancia(){
  digitalWrite(PinTrigger,HIGH);
  delayMicroseconds(10);
  digitalWrite(PinTrigger,LOW);

  TempoEcho = pulseIn(PinEcho, HIGH);
  return(((TempoEcho*VelocidadeSom_mporus)/2)*100);
}

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

  response.add(uint16_t(ReadMed)); // Dado do sensor
  response.add(uint16_t(ReadFloat)); // Dado do sensor

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

  pinMode(PinTrigger, OUTPUT);
  digitalWrite(PinTrigger, LOW);
  pinMode(PinEcho, INPUT); // configura pino ECHO como entrada

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
  timer_i = micros();
  static unsigned long lastMillis = 0;
  if (millis() - lastMillis > 10000) {
    lastMillis = millis();
    Serial.printf("free heap: %d\n", ESP.getFreeHeap());
  }
  if (millis() - lastMillis > 1000) 
  {
    for (i = 0; i <= 100; i++)
    {
      SensorMed += CalculaDistancia()/100;
    }
  
    auxMed = SensorMed;
    ReadMed = uint16_t(auxMed); // Inteiro
    ReadFloat = auxMed - ReadMed; //Flutuante
    ReadFloat = uint16_t((ReadFloat)*100);
    SensorMed = 0;
    Serial.print(CalculaDistancia());
    Serial.print(" --- ");
    Serial.print(uint16_t(ReadMed));
    Serial.print(" --- ");
    Serial.println(uint16_t(ReadFloat));
    lastMillis = millis();
  }

}