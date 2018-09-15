#include "display.h"


void initDisplayPins() {
    pinMode(DATA_PIN, OUTPUT);
    digitalWrite(DATA_PIN, LOW);
    pinMode(CP_PIN, OUTPUT);
    digitalWrite(CP_PIN, LOW);
    pinMode(DOTS_PIN, OUTPUT);
    digitalWrite(DOTS_PIN,LOW);
}

void enableDots() {
    digitalWrite(DOTS_PIN, HIGH);
}

void disableDots() {
    digitalWrite(DOTS_PIN, LOW);
}

void writeBit(uint8_t databit) {
    digitalWrite(CP_PIN, LOW);
    digitalWrite(DATA_PIN, databit);
    delayMicroseconds(10);
    digitalWrite(CP_PIN, HIGH);
    delayMicroseconds(10);
}

void writeByte(uint8_t databyte) {
    for (uint8_t i = 0; i < 8; i++) {
        writeBit((databyte >> i) & 0x01);
    }
}

void writeData(uint8_t* data, uint8_t datalength) {
    for (int i = 0; i < datalength; i++) {
        writeByte(data[i]);
    }

    char *c = (char *)calloc(5,sizeof(char));
    for(uint8_t i = 0; i < 4; i++) {
        c[i] = ByteCodeToASCII(data[i]);
    }
}

char ByteCodeToASCII(uint8_t code) {
    switch (code) {
        case 0b11111100:
            return '0';
        case 0b00001100:
            return '1';
        case 0b11011010:
            return '2';
        case 0b10011110:
            return '3';
        case 0b00101110:
            return '4';
        case 0b10110110:
            return '5';
        case 0b11110110:
            return '6';
        case 0b00011100:
            return '7';
        case 0b11111110:
            return '8';
        case 0b10111110:
            return '9';
        case 0b00100000:
            return '-';
        default:
            return 0;
    }
}

uint8_t ASCIItoByteCode(char c) {
    switch (c) {
        case '0':
            return 0b11111100;
        case '1':
        return 0b00001100;
        case '2':
            return 0b11011010;
        case '3':
            return 0b10011110;
        case '4':
            return 0b00101110;
        case '5':
            return 0b10110110;
        case '6':
            return 0b11110110;
        case '7':
            return 0b00011100;
        case '8':
            return 0b11111110;
        case '9':
            return 0b10111110;
        case '-':
            return 0b00100000;
        case 'C':
            return 0b11110000;
        case 'H':
            return 0b01101110;
        case 'P':
            return 0b01111010;
        default:
            return 0;
    }
}

uint8_t getByteCode(uint8_t number) {
  switch (number) {
    case 0:
      return 0b11111100;
    case 1:
      return 0b00001100;
    case 2:
      return 0b11011010;
    case 3:
      return 0b10011110;
    case 4:
      return 0b00101110;
    case 5:
      return 0b10110110;
    case 6:
      return 0b11110110;
    case 7:
      return 0b00011100;
    case 8:
      return 0b11111110;
    case 9:
      return 0b10111110;
    default:
      return 0;
  }
}
