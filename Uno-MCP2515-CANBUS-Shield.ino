/*
Castellated Dreams - Uno MCP2515 CAN Bus Shield

This is an example program for the Castellated Dreams Uno MCP2515 CAN Bus Shield.
The CAN bus shield can be used as a debugger or a controller for a CAN bus network, with its analog
joystick and OLED display.
The code demonstrates how to send and receive CAN messages using the MCP2515 library as well as displaying
information on the OLED display.

Author: Rein Å. Torsvik
Date: 2024-12-26
*/

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <mcp2515.h>

#define CAN_ADDRESS 0x42



//Pin definitions
//____________________________________________________________________________________________________
#define PIN_JOY_X A1
#define PIN_JOY_Y A0
#define PIN_JOY_BTN 3

#define PIN_CAN_CS 10



// Objects
//____________________________________________________________________________________________________
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(128, 32, &Wire, OLED_RESET);

struct can_frame msgSend;
struct can_frame msgRecieve;
MCP2515 can(PIN_CAN_CS); // CS pin 10



// Variables
//____________________________________________________________________________________________________
uint8_t joy_x = 0;
uint8_t joy_y = 0;
bool joy_btn = false;

uint8_t joy_x_prev = 0;
uint8_t joy_y_prev = 0;
bool joy_btn_prev = false;

uint32_t sent_count = 0;
uint32_t received_count = 0;



// Function to update the display
//____________________________________________________________________________________________________
void updateDisplay() {
  // Clear the buffer
  display.clearDisplay();

  // Set text size and color
  display.setTextSize(1);      // Text size (2x larger font)
  display.setTextColor(SSD1306_WHITE); // Text color

  // Display sent message
  display.setCursor(0, 0);     // Start at top-left corner
  display.print("TX ID: ");
  display.print(msgSend.can_id, HEX);
  display.print(" n: ");
  display.print(sent_count);
  display.println();
  display.print("   data: ");
  display.print(msgSend.data[0]);
  display.print(" ");
  display.print(msgSend.data[1]);
  display.print(" ");
  display.println(msgSend.data[2]);

  // Display received message
  display.setCursor(0, 16);     // Start at the third line
  display.print("RX ID: ");
  display.print(msgRecieve.can_id, HEX);
  display.print(" n: ");
  display.print(received_count);
  display.println();
  display.print("   data: ");
  display.print(msgRecieve.data[0]);
  display.print(" ");
  display.print(msgRecieve.data[1]);
  display.print(" ");
  display.println(msgRecieve.data[2]);

  // Update the display with the rendered content
  display.display();
}



// Setup
//____________________________________________________________________________________________________
void setup() {

  // Initialize serial communication
  Serial.begin(115200);
  Serial.println("Castellated Dreams - Uno CAN bus shield");
  delay(100);

  // Initialize the display
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  Serial.print("Initialising display... \t");
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("[ERROR] SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  Serial.println("[OK]");
  delay(100);

  // Initialise CAN bus
  Serial.print("Initialising CAN bus... \t");
  can.reset();
  if (can.setBitrate(CAN_250KBPS, MCP_8MHZ) != MCP2515::ERROR_OK) {
    Serial.println(F("[ERROR] CAN bus initialization failed"));
    for(;;); // Don't proceed, loop forever
  }
  can.setNormalMode();
  Serial.println("[OK]");
  delay(100);

  Serial.println("Initialisation complete");

  // Display splash screen
  display.clearDisplay();
  display.setTextSize(1);      // Text size (2x larger font)
  display.setTextColor(SSD1306_WHITE); // Text color
  display.println("Castellated Dreams");
  display.println("Uno CAN bus shield");
  display.display();

  delay(2000);
}



// Loop
//____________________________________________________________________________________________________
void loop() {

  joy_x = analogRead(PIN_JOY_X)/4;
  joy_y = analogRead(PIN_JOY_Y)/4;
  joy_btn = digitalRead(PIN_JOY_BTN);

  // Send CAN message only if joystick values have changed
  if (abs(joy_x - joy_x_prev) > 1 || abs(joy_y - joy_y_prev) > 1 || joy_btn != joy_btn_prev) {
    joy_x_prev = joy_x;
    joy_y_prev = joy_y;
    joy_btn_prev = joy_btn;

    // Send CAN message
    msgSend.can_id = CAN_ADDRESS;
    msgSend.can_dlc = 8;
    msgSend.data[0] = joy_x;
    msgSend.data[1] = joy_y;
    msgSend.data[2] = joy_btn;
    can.sendMessage(&msgSend);

    // Increment sent message count
    sent_count++;

    // Print debug info
    Serial.print("joy_x: ");
    Serial.print(joy_x);
    Serial.print(" joy_y: ");
    Serial.print(joy_y);
    Serial.print(" joy_btn: ");
    Serial.println(joy_btn);
  }

  // Check for received CAN message
  if(can.readMessage(&msgRecieve) == MCP2515::ERROR_OK) {
    Serial.print("Received: ");
    Serial.print(msgRecieve.can_id, HEX);
    Serial.print(" ");
    Serial.print(msgRecieve.can_dlc);
    for(int i = 0; i < msgRecieve.can_dlc; i++) {
      Serial.print(" ");
      Serial.print(msgRecieve.data[i], HEX);
    }
    Serial.println();

    // Increment received message count
    received_count++;
  }

  // Update the display with the sent and received messages
  updateDisplay();

  // Wait 10ms
  delay(10);
}