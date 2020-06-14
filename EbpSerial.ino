const unsigned int myAddress = 4;
const int inputPin = 2;
bool inputPinStateOld = false;

#include "EbpSerial.h"
EbpSerial EbpSerial;

void setup() {
  // initialise the ebp serial port
  EbpSerial.init(myAddress);
  // setup input and output ports
  pinMode(inputPin, INPUT_PULLUP);
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  // poll
  processData(EbpSerial.poll());
  // test input pin
  testInputPin();
}

void processData(byte newData) {
  if (newData == 0xff) {
    // 0b11111111 = no data available
  }
  else {
    // data available
    if ((newData & 0x01) == 0x01) {
      // LED on
      digitalWrite(LED_BUILTIN, HIGH);
    }
    else {
      // LED off
      digitalWrite(LED_BUILTIN, LOW);
    }
  }
}

void testInputPin() {
  bool inputPinStateNew = digitalRead(inputPin);

  // test the state of the input pin, if changed then send a telegram with new state information
  if (inputPinStateNew != inputPinStateOld) {
    inputPinStateOld = inputPinStateNew;
    if (inputPinStateNew) {
      EbpSerial.sendData(1);
    }
    else {
      EbpSerial.sendData(0);
    }
  }
}
