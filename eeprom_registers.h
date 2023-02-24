#include <stdint.h>
#define EEPROM_REGISTER_SIZE 50
extern SemaphoreHandle_t xEEPROMMutex;
extern uint16_t eeprom[];

int readReg(int regNumber);
void writeReg(int regNumber, int val);

/* BLOCK STARTS */
#define _GENERAL 0
