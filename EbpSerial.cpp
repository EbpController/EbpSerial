#include "EbpSerial.h"

// constructor
EbpSerial::EbpSerial() {
}

void EbpSerial::init(unsigned int address) {
  // set the baudrate 
  Serial.begin(BAUDRATE);
  // set the address of my device
  myAddress = address;
}

byte EbpSerial::poll(void) {
  // write the telgram(s) located in the TX buffer
  tlgWrite();
  // poll the serial port
  // if a telegram is received, check the address  
  // if the address is equal to my device address, then return the data
  // otherwise return 0xff (no data) and forward the telegram to the next device
  return tlgRead();
}

void EbpSerial::tlgWrite() {
  // if 'TX buffer get pointer' is equal to 'TX buffer put pointer' no more bytes to send
  // if serial write buffer is full, wait to send next byte(s)
  while ((txBufferGetPointer != txBufferPutPointer) && (Serial.availableForWrite() > 0)) {
    // write byte at 'TX buffer get pointer' to serial port
    Serial.write(txBuffer[txBufferGetPointer]);
    // update 'TX (ring)buffer get pointer'
    txBufferGetPointer = (txBufferGetPointer + 1) % TX_BUFFER_SIZE;
  }
}

byte EbpSerial::tlgRead(void) {
  // if received data = 0b11111111, no data is reveived
  byte receivedData = 0xff;

  // if RX buffer is not empty and
  // if received data still equal to 0b11111111, read a byte from the serial port
  while ((Serial.available() > 0) && (receivedData == 0xff)) {
    // read the byte from the serial port
    byte receivedByte = Serial.read();

    // analyse the received byte
    if ((receivedByte & 0b10000000) == 0) {
      // read the address
      if ((receivedByte & 0b01000000) == 0) {
        // read the 5 msb's of the address
        tlgReceived[0] = receivedByte;
        // update the checksum
        checksum = receivedByte;
      }
      else {
        // read the 5 lsb's of the address
        tlgReceived[1] = receivedByte;
        // update the checksum
        checksum ^= receivedByte;
      }
    }
    else {
      // read the data and the checksum
      if ((receivedByte & 0b01000000) == 0) {
        // read the data
        tlgReceived[2] = receivedByte;
        // update the checksum
        checksum ^= receivedByte;
      }
      else {
        // read the checksum
        tlgReceived[3] = receivedByte; 
        // check the address and checksum in the telegram       
        if (((tlgReceived[0] & 0x1f) << 5) + (tlgReceived[1] & 0x1f) == myAddress) {
          if (((tlgReceived[0] & 0b00100000) == 0) && ((tlgReceived[1] & 0b00100000) == 0)){
            if (checksum == receivedByte) {
              // get the data
              receivedData = tlgReceived[2] & 0b00111111;  
            }
          }
          else {
            // telegram from another device, send this telegram to the PC
          pushTlgInTxBuffer(tlgReceived);
          }
        }
        else {
          // received address <> my address, send telegram to the next device
        pushTlgInTxBuffer(tlgReceived);
        }
      }
    }
  }
  return receivedData;
}

void EbpSerial::sendData(byte data) {
  // make a telegram and push it to the TX buffer
  byte tlgToWrite[TLG_SIZE];

  // adrress 5msb's
  tlgToWrite[0] = 0b00100000 | ((myAddress >> 5) & 0x1f);
  // address 5 lsb's
  tlgToWrite[1] = 0b01100000 | (myAddress  & 0x1f);
  // data (6 lsb's)
  tlgToWrite[2] = 0b10000000 | (data  & 0x3f);
  //  checksum
  tlgToWrite[3] = tlgToWrite[0] ^ tlgToWrite[1] ^ tlgToWrite[2];

  pushTlgInTxBuffer(tlgToWrite);
}

void EbpSerial::pushTlgInTxBuffer(byte tlg[]){
  for (int i; i < TLG_SIZE; i++) {
    // push the bytes on the TX buffer
    txBuffer[txBufferPutPointer] = tlg[i];
    // update 'TX (ring)buffer put pointer'
    txBufferPutPointer = (txBufferPutPointer + 1) % TX_BUFFER_SIZE;
  }
}
