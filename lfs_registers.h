#include <stdint.h>
#define LFS_REGISTER_SIZE 100
extern SemaphoreHandle_t xLFSMutex;
extern uint16_t modbus_lfs[];

/* BLOCK STARTS */
#define _NAME 0
#define _FUNCTION 1
#define _DATA 2
