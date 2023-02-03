#define MODBUS_REGISTER_SIZE 100
#include <stdint.h>
extern uint16_t tab_reg[];

/* GENERAL REGISTER ADDRESSES */
#define _GENERAL_REG_START 0
#define _REQ_TIMESTAMP1 (_GENERAL_REG_START)
#define _REQ_TIMESTAMP2 (_GENERAL_REG_START + 1)
#define _REQ_COUNTER (_GENERAL_REG_START + 2)
#define _EVENT_FLAGS (_GENERAL_REG_START + 3)
#define _SLAVE_ID (_GENERAL_REG_START + 4)
#define _MODBUS_FUNCTION (_GENERAL_REG_START + 5)
#define _REGISTER_ADRESS (_GENERAL_REG_START + 6)
#define _REGISTER_NUMBER (_GENERAL_REG_START + 7)

/* LASER TASK REGISTER ADDRESSES */
#define _LASER_REG_START 32
#define _LASER_STATUS (_LASER_REG_START)
#define _LASER_TOTAL_ACTIVATIONS (_LASER_REG_START + 1) // 32bit
#define _LASER_TOTAL_RETRYS (_LASER_REG_START + 3)      // 32bit
#define _LASER_CURRENT_RETRYS (_LASER_REG_START + 5)
#define _LASER_TOTAL_UPTIME (_LASER_REG_START + 6)   // 32bit
#define _LASER_CURRENT_UPTIME (_LASER_REG_START + 8) // 32bit
#define _LASER_TIMESTAMP1 (_LASER_REG_START + 10)
#define _LASER_TIMESTAMP2 (_LASER_REG_START + 11)
#define _LASER_MODE (_LASER_REG_START + 12)
#define _LASER_INTENSITY (_LASER_REG_START + 13)
#define _SYNC_CONTROL (_LASER_REG_START + 14)
#define _HDR_CONTROL (_LASER_REG_START + 15)
#define _FRAME_COUNTER (_LASER_REG_START + 16) // 32bit
#define _LASER_DELAY (_LASER_REG_START + 18)
#define _LASER_REG_END (_LASER_REG_START + 18)

/* TEMPERATURE TASK REGISTER ADDRESSES */
#define _TEMP_REG_START 51
#define _TEMP_TIMESTAMP (_TEMP_REG_START) // 32bit
#define _TEMPERATURE (_TEMP_REG_START + 2)
#define _HUMIDITY (_TEMP_REG_START + 3)
#define _TEMP_REG_END (_TEMP_REG_START + 3)