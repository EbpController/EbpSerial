#ifndef EBPSERIAL_H
#define EBPSERIAL_H

#include <Arduino.h>

#define BAUDRATE 115200
#define TLG_SIZE 4
#define TX_BUFFER_SIZE 1024

class EbpSerial {

  private:
    unsigned int myAddress;
    byte tlgReceived[TLG_SIZE];
    byte checksum;

    byte txBuffer[TX_BUFFER_SIZE];
    int txBufferPutPointer;
    int txBufferGetPointer;

    void tlgWrite(void);
    byte tlgRead(void);
    void pushTlgInTxBuffer(byte[]);

  public:
    EbpSerial();
    void init(unsigned int);
    byte poll(void);
    void sendData(byte);
};

#endif
