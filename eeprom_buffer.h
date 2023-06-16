#ifndef EEPROM_BUFFER_H
#define EEPROM_BUFFER_H

#include "Adafruit_EEPROM_I2C.h"
#include <cstdint>

#define EEPROM_SIZE 1024

#define EEPROM_DATA_START 10
static volatile bool initialized = false;

class EEPROMBuffer
{
  public:
    static EEPROMBuffer &getInstance();

    void writeData(uint16_t address, uint16_t data);
    uint8_t readData(uint16_t address);

    union EEPROMUnion {
        uint16_t eeprom16[EEPROM_SIZE / 2] = {0}; // Buffer in 16-bit format
        uint8_t eeprom8[EEPROM_SIZE];
    };

    uint16_t *getBuffer16();
    uint8_t *getBuffer8();

    void read(uint16_t addr, uint16_t num);
    void write(uint16_t address, uint16_t num);

  private:
    EEPROMUnion bufferUnion;
    Adafruit_EEPROM_I2C _eeprom;
    void begin();
    EEPROMBuffer(); // Private constructor to prevent instantiation
    EEPROMBuffer(const EEPROMBuffer &) = delete; // Delete copy constructor
    EEPROMBuffer &
    operator=(const EEPROMBuffer &) = delete;    // Delete assignment operator
};

static EEPROMBuffer &eeprom = EEPROMBuffer::getInstance();

#endif // EEPROM_BUFFER_H
