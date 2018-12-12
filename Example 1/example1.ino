/* **********************************************************************************************************
 * FlashTools - Example program.                                                                              
 * On first power up, the varible blinks = 0 is written to flash. 
 * 
 * Then, each time the MCU is powered on the varibale 'blinks' is read from flash, incremented and written
 * back to flash.
 * 
 * The connected LED will blink 'blinks' time before a 5 second delay. 
 * 
 * Once blinks >= 3, MPU is enabled and the flash region where blinks is written is protected as RO.
 * The subsuqent write triggers a Memory Management Fault, and the interrupt service routine defined
 * here will print an error message to the serial monitor.
 * *********************************************************************************************************/
#include "FlashTools.h"
#include <Arduino.h>

int ledPin = 13;       // LED connected to this pin
uint32_t blinks[1];    // Value determines how many times the LED blinks
uint32_t *flash1_addr; // Pointer to hold flash address
FlashTools flash1;     // FlashTools object

/* Interrupt Service Routine for Memory Management Faults */
ISR(MemManage_Handler) {
    while (1) {
      SerialUSB.println("Error: Memory Management Fault");
    }
}

/* Set up - Runs once on power up*/
void setup() { 
  // Get start address for flash page 1030
  flash1_addr = flash1.getPageAddress<uint32_t>(1030);
  
  // Read in value 'blinks'
  uint32_t val = flash1.read<uint32_t>(flash1_addr);
  blinks[0] = val == 0xffffffff ? 0 : val;
  
  // If blinks read in as >=3, enable MPU and protect flash region
  if (blinks[0] >= 3) {
    flash1.MPUConfigureRegion(flash1_addr, 4, 0, 0b000, 1, 0, 1, 0b101, 1);
  } 
  
  blinks[0] = blinks[0] = 1;
  
  // Write blinks back to flash at the same address
  flash1.write<uint32_t>(flash1_addr, blinks, sizeof(uint32_t));  
}

/* Main program loop */
void loop() {
  for (int i = 0; i < blinks[0]; ++i) {
      digitalWrite(ledPin, HIGH);
      delay(500);
      digitalWrite(ledPin, LOW);
      delay(200);
  }
  // Sleep for 5 seconds
  delay(5000);
}
