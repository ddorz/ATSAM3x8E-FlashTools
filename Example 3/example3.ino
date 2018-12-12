/* **********************************************************************************************************
 * FlashTools - Example program.                                                                                
 * Simple example program using FlashTools to read Cortex-M3 MPU's 128-bit unique id
 * *********************************************************************************************************/
#include "FlashTools.h"
#include <Arduino.h>

/* 128-bit unique identifier will be stored in 32-bit parts */
#define UNIQUE_ID_SIZE 128u/32u

/* Set up */
void setup() { 
  SerialUSB.begin(9600);
}

/* Program loop */
void loop() {
  // Define flash tools class and array for unique id
  FlashTools flash1;
  uint32_t unique_id[UNIQUE_ID_SIZE] {0};

  // Get the unique id
  flash1.getUniqueID(unique_id);

  // Print the unique id to serial monitor
  Serial.println("Unique ID:);
  for (uint32_t i {0}; i < UNIQUE_ID_SIZE; ++i) {
    Serial.println(unique_id[i]);
  }

  // Sleep for 5s
  delay(5000);
}
