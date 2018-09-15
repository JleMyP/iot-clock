#ifndef _DISPLAY_H_
#define _DISPLAY_H_


#include <Arduino.h>

#define DATA_PIN  D8
#define CP_PIN    D0
#define DOTS_PIN  D2


void initDisplayPins();

void enableDots();
void disableDots();

void writeBit(uint8_t databit);
void writeByte(uint8_t databyte);
void writeData(uint8_t* data, uint8_t datalength);

char ByteCodeToASCII(uint8_t code);
uint8_t ASCIItoByteCode(char c);
uint8_t getByteCode(uint8_t number);


#endif // _DISPLAY_H_