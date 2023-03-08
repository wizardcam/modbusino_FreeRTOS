#include <stdint.h>
#define EEPROM_REGISTER_SIZE 100
extern SemaphoreHandle_t xEEPROMMutex;
extern uint16_t modbus_eeprom[];

/* BLOCK STARTS */
#define _GENERAL 0
