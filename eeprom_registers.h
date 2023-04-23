#include <stdint.h>
#define EEPROM_SIZE 10000
#define EEPROM_REGISTER_SIZE 1000
extern uint16_t modbus_eeprom[];

#define _GENERAL 0
#define _MAX_TEMPERATURE 1
#define _EEPROM_DATA_START 10
