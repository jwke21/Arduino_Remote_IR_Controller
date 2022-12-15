#include <IRremote.hpp>
#include <stdio.h>
#include <inttypes.h>

#ifndef IR_RECEIVE_PIN
#define IR_RECEIVE_PIN 2 // The pin the IR reciever is connected to
#endif

void setup() {
  // Set data rate to 9600 baud (i.e. bits per second) for serial data transmission
  Serial.begin(9600);

  // Start the IR receiver and enable LED feedback
  IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);
  IRData data;
}

void loop() {
  // When data is received, decode it into an IRData struct and determine the protocol
  if (IrReceiver.decode()) {
    // Print short summary of receieved data
    IrReceiver.printIRResultShort(&Serial);
    // Print info about how to send signal
    IrReceiver.printIRSendUsage(&Serial);
    // Output the results in RAW format
    IrReceiver.printIRResultRawFormatted(&Serial, false);
    // Output the results as uint16_t source code array of microsecond bursts
    IrReceiver.compensateAndPrintIRResultAsCArray(&Serial, true);

    char s[1024];
    sprintf(s, "%d", IrReceiver.decodedIRData.decodedRawData);
    Serial.println(s);

    // Enable receiving of next packet
    IrReceiver.resume();
  }
}