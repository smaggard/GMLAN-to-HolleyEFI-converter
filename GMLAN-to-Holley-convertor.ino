/*
 * Reads certain parameters from GM E38 ECU and sends them as CAN inputs to a Holley EFI. 
 * This is firmware for an AutoSportLabs ESP32-CAN-X2 board 
 * and will send several factory messages to a Holley EFI system over CAN.
*/

// Include required libraries
#include <driver/twai.h>
#include <mcp_can.h>    // ADD library https://github.com/coryjfowler/MCP_CAN_lib
#include <SPI.h>

// Define CAN1 (TWAI) Pins
#define CAN1_RX_PIN GPIO_NUM_6
#define CAN1_TX_PIN GPIO_NUM_7

// Define CAN2 (MCP2515) Custom SPI Pins
#define CAN2_CS_PIN GPIO_NUM_10
#define CAN2_SPI_SCK GPIO_NUM_12
#define CAN2_SPI_MISO GPIO_NUM_13
#define CAN2_SPI_MOSI GPIO_NUM_11

// Define CAN2 CAN
SPIClass CAN2_SPI(HSPI); // HSPI bus
MCP_CAN CAN2(&CAN2_SPI, CAN2_CS_PIN);

byte led_state = LOW;

// Define the CAN PIDS to Emulate a Holley CAN I/O Module
unsigned long fuelLevelID = 0x1E202001; //CAN ID 1, input 1
unsigned long acKickID = 0x1E206001; // CAN ID 1 input 2
unsigned long acHighSidePressureID = 0x1E20A001; // CAN ID 1 input 3
unsigned long reducedPowerID = 0x1E20E001; // CAN ID 1 input 4
unsigned long outsideAirID = 0x1E212001; // CAN ID 1 input 5


void floatToHexArray(float value, unsigned char *hexArray) {
  // Function to convert float values to hex arrays to be sent as a CAN message.
  memcpy(hexArray, &value, sizeof(float));
  std::swap(hexArray[0], hexArray[3]);
  std::swap(hexArray[1], hexArray[2]);
}

void sendDataCAN2(unsigned long can_id, byte ext_flag, byte *data, unsigned long data_len)
{
  Serial.print("Got a message from ID ");
  Serial.println(can_id);
  if (can_id == 1233)
  {
    float fuelLevel = data[5] * 0.392157;
    unsigned char fuelHexArray[4];
    unsigned char rpHexArray[4];
    float reducedPower = bitRead(data[3], 7);
    floatToHexArray(fuelLevel, fuelHexArray);
    floatToHexArray(reducedPower, rpHexArray);
    // Send the message via CAN2
    CAN_sendMsg(fuelLevelID, fuelHexArray);
    CAN_sendMsg(reducedPowerID, rpHexArray);
  }
  else if (can_id == 977)
  {
    float acKick = bitRead(data[4], 5);
    unsigned char acHexArray[4];
    floatToHexArray(acKick, acHexArray);
    CAN_sendMsg(acKickID, acHexArray);
  }
  else if (can_id == 1017)
  {
    float acHighSidePressure = (data[7] * 14) / 6.895;
    unsigned char hexArray[4];
    floatToHexArray(acHighSidePressure, hexArray);
    // Send the message via CAN2
    CAN_sendMsg(acHighSidePressureID, hexArray);
  }
  else if (can_id == 1009)
  {
    float outsideAirTemp = ((((data[7] *0.5) - 40) * 1.8)+32);
    unsigned char hexArray[4];
    floatToHexArray(outsideAirTemp, hexArray);
    // Send message via CAN2
    CAN_sendMsg(outsideAirID, hexArray);
  }
  else 
  {
    // not matched!
  }
}

bool CAN_sendMsg(unsigned long canid, unsigned char hexArray[4]) 
{
  byte result = CAN2.sendMsgBuf(canid, 1, sizeof(hexArray), hexArray);
    if (result == CAN_OK)
    {
      return true;
    }
    else
    {
      return false;
    }
}

void CAN1_readMsg(void *parameters)
{
  twai_message_t message;
  while (twai_receive(&message, 0) == ESP_OK) // Non-blocking
  {
    // Debug
    // Check and see if frame is STD or EXT along with the identifier
    Serial.printf("[CAN1 0x%X %s]: ", message.identifier, message.extd ? "EXT" : "STD");
    // Print the message
    for (int i = 0; i < message.data_length_code; i++)
    {
      Serial.printf("0x%02X", message.data[i]);
      if (i < message.data_length_code - 1)
        Serial.print(", ");
    }
    Serial.println();
    // End Debug
    sendDataCAN2(message.identifier, message.extd, message.data, message.data_length_code); 
  }
}

bool setupCAN1(void)
{
  twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT((gpio_num_t)CAN1_TX_PIN, (gpio_num_t)CAN1_RX_PIN, TWAI_MODE_NORMAL);
  twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();   // CAN1 at 500 kbps
  twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL(); // Accept all messages

  if (twai_driver_install(&g_config, &t_config, &f_config) != ESP_OK)
  {
    Serial.println("Failed to install TWAI driver");
    return false;
  }

  if (twai_start() != ESP_OK)
  {
    Serial.println("Failed to start TWAI driver");
    return false;
  }

  Serial.println("CAN1 (TWAI) started. Waiting for messages...");
  return true;
}

bool setupCAN2()
{
  CAN2_SPI.begin(CAN2_SPI_SCK, CAN2_SPI_MISO, CAN2_SPI_MOSI, CAN2_CS_PIN);

  if (CAN2.begin(MCP_STDEXT, CAN_1000KBPS, MCP_16MHZ) == CAN_OK)
  {
    CAN2.setMode(MCP_NORMAL);
    return true;
  }
  else
  {
    return false;
  }
}

void setup()
{
  Serial.begin(115200);

  //configure LED
  pinMode(LED_BUILTIN, OUTPUT);

  // Setup CAN1 (TWAI)
  Serial.println("Initializing CAN1...");
  if (!setupCAN1())
  {
    Serial.println("Failed to setup CAN1. Exiting...");
    while (1)
      ;
  }

  // Setup CAN2 (MCP2515)
  Serial.println("Initializing CAN2...");
  if (!setupCAN2())
  {
    Serial.println("Failed to initialize CAN2. Exiting...");
    while (1)
      ;
  }
}

void loop()
{
  CAN1_readMsg(); // Read from CAN1 and forward to CAN2 with data modification
}
