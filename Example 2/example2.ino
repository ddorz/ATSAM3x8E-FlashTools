/* **********************************************************************************************************
 * FlashTools - Example program.                                                                                
 * Simple example program writing and reading data to flash
 * *********************************************************************************************************/
#include "FlashTools.h"
#include <Arduino.h>
#define SIZE 30

void setup() { 
  SerialUSB.begin(9600);
}

void loop() {
  
  // Create dueEFC object
  FlashTools flash1;
 
  // Get address of flash page 1025
  uint32_t flash1_addr = flash1.getPageAddress(1025);

  // Create data buffer to be written to flash
  uint32_t data[SIZE] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30};
    
  // Declare data buffer to hold the above data copied back from flash
  uint32_t copied_data[SIZE];
  
  // Write data to flash
  flash1.write<uint32_t>(flash1_addr, data, SIZE);
  
  // Read the data from flash 1 word at a time
  SerialUSB.println("Data read from flash:");
  for (int i = 0; i < SIZE; ++i) {
    SerialUSB.println(flash1.read((uint32_t*)flash1_addr + i));
  }
}
